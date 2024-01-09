---
tags:
  - Class
---
# Class "Beam"

This class provides more streamlined access to the `BeamRenderer` used internally for rendering cords, ie Evis, Gello, Vis Fatty, etc.
Note that this is a low-level class that strictly handles rendering. We hope to later provide an extension of this class capable of handling the physics calculations and automatic point adjustment required for cords, but this is a complex system that will require a non-trivial amount of effort to implement.

???+ Info "Info"
    A better tutorial on how to use this class is pending.

## Constructors
### Beam () {: aria-label='Constructors' }
#### [Beam](Beam.md) Beam ( [Sprite](../Sprite.md) Sprite, int Layer, boolean UseOverlay, boolean UnkBool, int PointsPreallocateSize = 8) {: .copyable aria-label='Constructors' }
#### [Beam](Beam.md) Beam ( [Sprite](../Sprite.md) Sprite, string LayerName, boolean UseOverlay, boolean UnkBool int PointsPreallocateSize = 8) {: .copyable aria-label='Constructors' }

???- note "Notes"
	`PointsPreallocateSize` is the amount of space that `Beam` will reserve to hold points. This is not a hard limit, but going past this amount will incur a performance cost as more memory must be allocated to hold new ones.

???- example "Example Code"
	Here is an example of how you would use this class:

    ```lua
	local sprite = Sprite()
	sprite:Load("gfx/1000.193_anima chain.anm2", true)
	local chain = Beam(sprite, "chain", false, false)
	chain:GetSprite():Play("Idle", false)
	local layer = chain:GetSprite():GetLayer("chain")
	layer:SetWrapSMode(1) -- these are critical, otherwise
	layer:SetWrapTMode(0) -- the beam sprite won't wrap!
	local spritesheetHeight = 64

	mod:AddCallback(ModCallbacks.MC_PRE_PLAYER_RENDER, function(_, player)
		local origin = Isaac.WorldToScreen(Game():GetRoom():GetCenterPos())
		local target = Isaac.WorldToScreen(player.Position)
		local coord = target:Distance(origin)
		chain:Add(origin,0)
		chain:Add(target,coord)
		chain:Render()
	end)
    ```

## Functions

### Add () {: aria-label='Functions' }
#### void Add ( [Vector](../Vector.md) Position, float SpritesheetCoordinate, float Width = 1.0, [Color](../Color.md) PointColor = Default ) {: .copyable aria-label='Functions' }
#### void Add ( [Point](Point.md) Point ) {: .copyable aria-label='Functions' }   
Adds a point to the beam. Points are stored in order of adding.

???+ info "Info"
    `SpritesheetCoordinate` is, to our current understanding, the `Y` position of the spritesheet that should be drawn by the time this Point is reached. For example, two points of `0` and `64` SpritesheetCoordinate will render the spritesheet starting from `y 0` to `y 64`, while an additional third point of `0` will draw it in reverse from `y 64` to `y 0`.
	`Width` acts as a multiplier for how wide the beam should be. A non-zero value will scale the spritesheet width accordingly. This is interpolated between points.

___
### GetLayer () {: aria-label='Functions' }
#### int GetLayer ( ) {: .copyable aria-label='Functions' }   

___

### GetPoints () {: aria-label='Functions' }
#### [Point](Point.md)[] GetPoints ( ) {: .copyable aria-label='Functions' }   
Returns a table of the [Points](Point.md) currently stored.

___

### GetSprite () {: aria-label='Functions' }
#### [Sprite](../Sprite.md) GetSprite ( ) {: .copyable aria-label='Functions' }   

___
### GetUnkBool () {: aria-label='Functions' }
#### boolean GetUnkBool ( ) {: .copyable aria-label='Functions' }   

___
### GetUseOverlay () {: aria-label='Functions' }
#### boolean GetUseOverlay ( ) {: .copyable aria-label='Functions' }   

___
### Render () {: aria-label='Functions' }
#### void Render ( boolean ClearPoints = true ) {: .copyable aria-label='Functions' }

___
### SetLayer () {: aria-label='Functions' }
#### void SetLayer ( int LayerID ) {: .copyable aria-label='Functions' }   
#### void SetLayer ( string LayerName ) {: .copyable aria-label='Functions' } 
  
___
### SetPoints () {: aria-label='Functions' }
#### void SetPoints ( [Point](Point.md)[] Points ) {: .copyable aria-label='Functions' }   
Sets the [Points](Point.md) used by this.

___
### SetSprite () {: aria-label='Functions' }
#### void SetSprite ( [Sprite](../Sprite.md) Sprite ) {: .copyable aria-label='Functions' }   

___
### SetUnkBool () {: aria-label='Functions' }
#### void SetUnkBool ( boolean UnkBool ) {: .copyable aria-label='Functions' }   

___
### SetUseOverlay () {: aria-label='Functions' }
#### void SetUseOverlay ( boolean UseOverlay ) {: .copyable aria-label='Functions' }   

___