/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
	The classic way of editing, which follows the office. What its page context and its group
	context do is the same so far, apart from the group one stands in, so it is written once here
	and each context says only what differs.
*/
abstract class ClassicContext implements InteractionContext {
	/// The group one stands in, or nothing on the page.
	protected readonly groupStoodIn: number | undefined;

	constructor(groupStoodIn: number | undefined) {
		this.groupStoodIn = groupStoodIn;
	}

	/*
		Answers a press from the geometry the client holds, by naming the objects to the engine
		instead of sending the press for it to hit test. A press that meets no object asks for no
		selection at all, which is how the engine is told to drop the one it holds.

		Not taken here: a modifier other than Shift or Ctrl, or a press that meets nothing while
		nothing is selected.
	*/
	public press(hit: number | undefined, modifier: number): boolean {
		const extending =
			modifier === app.UNOModifier.SHIFT || modifier === app.UNOModifier.CTRL;
		if (modifier !== 0 && !extending) return false;

		/*
			A press inside a group takes hold of the group. What was hit may be an object the
			group holds, and only entering the group reaches that.
		*/
		const objectId =
			hit === undefined
				? undefined
				: RenderGeometrySection.outermostOf(hit, this.groupStoodIn);
		if (objectId === undefined && !GraphicSelection.selectedObjectIDs.length)
			return false;

		GraphicSelection.selectObjects(
			this.enteredBy(
				this.orTheGroup(this.selectionForPress(objectId, extending)),
				hit === undefined ? [] : [hit],
			),
		);
		return true;
	}

	/*
		A diagram is never selected on its own, only among several objects: selecting it alone
		enters it. Of its objects, in the order a walk down it meets them, the one under the press
		is taken, else the first - the shape that spans the whole diagram - or going backwards the
		last.
	*/
	public enteredBy(
		selection: number[],
		hits: number[] = [],
		backwards: boolean = false,
	): number[] {
		if (selection.length !== 1) return selection;
		if (!RenderGeometrySection.objectOf(selection[0])?.isDiagram)
			return selection;

		const objects = RenderGeometrySection.theOnesThatDraw(selection);
		if (!objects.length) return selection;

		const hit = hits.find((objectId: number) => objects.includes(objectId));
		if (hit !== undefined) return [hit];
		return [backwards ? objects[objects.length - 1] : objects[0]];
	}

	/*
		The selection to take for that one: as it is where it holds something, and where it holds
		nothing the group one stands in, since standing in a group with nothing selected is no
		place to be. Leaving a group selects it. On the page nothing is selected then.
	*/
	protected orTheGroup(selection: number[]): number[] {
		if (selection.length || this.groupStoodIn === undefined) return selection;
		return [this.groupStoodIn];
	}

	/*
		A double press on a group that is no diagram enters it: the member hit is selected, which
		puts one in the group's context. Anything else - an object with text, a chart, a diagram,
		empty space - is the engine's, which starts a text edit or activates the object.
	*/
	public doublePress(hit: number | undefined): boolean {
		if (hit === undefined) return false;

		const objectId = RenderGeometrySection.outermostOf(hit, this.groupStoodIn);
		if (objectId === hit) return false;
		if (RenderGeometrySection.objectOf(objectId)?.isDiagram) return false;

		GraphicSelection.selectObjects([
			RenderGeometrySection.outermostOf(hit, objectId),
		]);
		GraphicSelection.updateSelectionSection();
		return true;
	}

	/*
		Enter on one selected group that is no diagram enters it, selecting its first object.
		Anything else is the engine's, which starts the text edit of the object selected.
	*/
	public enterKey(): boolean {
		const selected = GraphicSelection.selectedObjectIDs;
		if (selected.length !== 1) return false;
		if (RenderGeometrySection.objectOf(selected[0])?.isDiagram) return false;

		const held = RenderGeometrySection.heldBy(selected[0]);
		if (!held.length) return false;

		GraphicSelection.selectAndShow([held[0]]);
		return true;
	}

	/*
		The states of the commands the client gives itself, from what is selected where one
		stands:
		- Enter Group on one selected group that is no diagram, as Enter and F3 enter it; Leave
		  Group wherever one stands in a group; Ungroup where groups or diagrams are selected,
		  each of which it takes apart by one level, and otherwise where one stands in a group,
		  which it takes apart.
		- Group with two objects or more; aligning, arranging and the position and size with one
		  or more; naming with exactly one; spreading out with three or more. As the office does,
		  a part inside a 3D scene is neither aligned nor arranged.
		- Bringing to the front and forward where what is selected does not stand at the front
		  of its level already, sending to the back and backward where it does not stand at the
		  back.
		- By the kind of the objects and what they allow: flipping where none is held in place
		  and each may be turned over; changing to curves for one object that may be and is no
		  curve yet, or for several of which one may be; the area unless each object is a line
		  or a table and one at least is a line, a picture having a fill behind what it shows;
		  the line where no table is selected.
		  With nothing selected the area and the line are offered, for the objects drawn next.
	*/
	public commandStates(): { [command: string]: boolean } {
		const selected = GraphicSelection.selectedObjectIDs;
		const count = selected.length;
		const single = count === 1 ? selected[0] : undefined;
		const aGroup =
			single !== undefined &&
			RenderGeometrySection.holdsObjects(single) &&
			!RenderGeometrySection.objectOf(single)?.isDiagram;
		// The kinds of the parts of a 3D scene, which sit between the scene's and the polygon's.
		const inAScene = selected.some((objectId: number) => {
			const kind = RenderGeometrySection.objectOf(objectId)?.objectKind ?? 0;
			return kind > 202 && kind <= 209;
		});

		const states: { [command: string]: boolean } = {
			'.uno:EnterGroup': aGroup,
			'.uno:LeaveGroup': this.groupStoodIn !== undefined,
			'.uno:FormatUngroup':
				this.groupStoodIn !== undefined ||
				selected.some((objectId: number) =>
					RenderGeometrySection.holdsObjects(objectId),
				),
			'.uno:FormatGroup': count >= 2,
		};
		for (const command of Interaction.commandsForOneOrMore)
			states[command] =
				count >= 1 && (command === '.uno:TransformDialog' || !inAScene);

		// The places are counted only where every selected object is found at the level.
		if (count >= 1) {
			const level = this.objectsOfTheLevelOf(selected[0]);
			const places = selected
				.map((objectId: number) => level.indexOf(objectId))
				.filter((place: number) => place >= 0)
				.sort((a: number, b: number) => a - b);
			const atTheBack = places.every(
				(place: number, at: number) => place === at,
			);
			const atTheFront = places.every(
				(place: number, at: number) =>
					place === level.length - places.length + at,
			);
			if (places.length === count) {
				if (atTheFront)
					states['.uno:BringToFront'] = states['.uno:ObjectForwardOne'] = false;
				if (atTheBack)
					states['.uno:SendToBack'] = states['.uno:ObjectBackOne'] = false;
			}
		}

		for (const command of Interaction.commandsForOne)
			states[command] = count === 1;
		for (const command of Interaction.commandsForThreeOrMore)
			states[command] = count >= 3;

		const objects = this.objectsTheCommandsGoTo()
			.map((objectId: number) => RenderGeometrySection.objectOf(objectId))
			.filter((object): object is cool.SlideObject => object !== undefined);
		const kinds = objects.map(
			(object: cool.SlideObject) => object.objectKind ?? 0,
		);

		const flip =
			objects.length >= 1 &&
			objects.every(
				(object: cool.SlideObject) =>
					!object.cannotBeMoved && !object.cannotBeFlipped,
			);
		states['.uno:FlipHorizontal'] = flip;
		states['.uno:FlipVertical'] = flip;

		// The kinds 10 and 11 are the open and the closed curve.
		states['.uno:ChangeBezier'] =
			objects.length === 1
				? !objects[0].cannotBeChangedToCurves &&
					kinds[0] !== 10 &&
					kinds[0] !== 11
				: objects.some(
						(object: cool.SlideObject) => !object.cannotBeChangedToCurves,
					);

		// The kinds with no area: the line, the arc, the polyline, the open curve, the freehand
		// line and the connector. The table, kind 35, fills its cells with an area of their own.
		const withNoArea = [2, 6, 9, 10, 12, 24];
		states['.uno:FormatArea'] = !(
			kinds.some((kind: number) => withNoArea.includes(kind)) &&
			kinds.every((kind: number) => withNoArea.includes(kind) || kind === 35)
		);
		states['.uno:FormatLine'] = !kinds.includes(35);
		return states;
	}

	/// The objects the commands go to, which are the ones selected.
	protected objectsTheCommandsGoTo(): number[] {
		return GraphicSelection.selectedObjectIDs;
	}

	/*
		Ungroup takes apart the groups and diagrams selected, each by one level, which the engine
		does with what it has marked. With none of them selected it takes apart the group one
		stands in.
	*/
	public ungroup(): boolean {
		const anySelected = GraphicSelection.selectedObjectIDs.some(
			(objectId: number) => RenderGeometrySection.holdsObjects(objectId),
		);
		if (anySelected || this.groupStoodIn === undefined) return false;

		GraphicSelection.takeApart(this.groupStoodIn);
		return true;
	}

	/// Ctrl+F3 leaves the group one stands in, selecting it. On the page there is none to leave.
	public leaveKey(): boolean {
		if (this.groupStoodIn === undefined) return false;

		GraphicSelection.selectAndShow([this.groupStoodIn]);
		return true;
	}

	/*
		The objects one travels through where one stands: the ones the group holds, or those of
		the page. In the order the page draws them, from the back to the front, as the office
		travels.
	*/
	protected objectsOfTheLevel(): number[] {
		return this.groupStoodIn === undefined
			? RenderGeometrySection.heldByThePage()
			: RenderGeometrySection.heldBy(this.groupStoodIn);
	}

	/// The objects at the level of that one, in the order the page draws them, from the back to
	/// the front.
	protected objectsOfTheLevelOf(objectId: number): number[] {
		const parent = RenderGeometrySection.parentOf(objectId);
		return parent
			? RenderGeometrySection.heldBy(parent)
			: RenderGeometrySection.heldByThePage();
	}

	/// Selects that one object alone and brings it on screen, for a key going forwards or, with
	/// backwards set, backwards.
	protected selectOnly(objectId: number, backwards: boolean = false): boolean {
		GraphicSelection.selectAndShow(this.enteredBy([objectId], [], backwards));
		return true;
	}

	/// Selects that one object alone where the mouse is, which is on screen already, the objects
	/// under the mouse being those.
	private selectOnlyHere(objectId: number, hits: number[]): boolean {
		GraphicSelection.selectObjects(this.enteredBy([objectId], hits));
		GraphicSelection.updateSelectionSection();
		return true;
	}

	/*
		Tab selects the next object of the level after the selection, Shift+Tab the one before it,
		going round from the last to the first. With nothing selected it starts at the first, or
		with Shift at the last.
	*/
	public tabKey(forwards: boolean): boolean {
		const level = this.objectsOfTheLevel();
		if (!level.length) return false;

		const places = GraphicSelection.selectedObjectIDs
			.map((objectId: number) => level.indexOf(objectId))
			.filter((place: number) => place >= 0);
		if (!places.length)
			return this.selectOnly(level[forwards ? 0 : level.length - 1], !forwards);

		const from = forwards ? Math.max(...places) : Math.min(...places);
		const to = (from + (forwards ? 1 : -1) + level.length) % level.length;
		return this.selectOnly(level[to], !forwards);
	}

	/*
		Alt+press goes through the objects under the press, deep: every object on the way from the
		page down to each object the press meets, in the order a walk from the front to the back
		meets them, a group before what it holds. It selects the one after the selected one, or
		with Shift the one before it, going round at either end. So a group is selected first, the
		next press enters it, selecting the member under the press, and after its last member under
		the press one leaves it for the object behind. A diagram's objects under the press are
		stops of their own, its groups and the diagram itself are none, as a diagram is never
		selected alone. With nothing under the press selected, the frontmost is taken.
	*/
	public altPress(hits: number[], backwards: boolean): boolean {
		const order: number[] = [];
		for (const hit of hits) {
			// The way from the page down to the object met; inside a diagram only the object.
			const way: number[] = [];
			for (
				let objectId: number | undefined = hit, guard = 0;
				objectId && guard < 100;
				objectId = RenderGeometrySection.parentOf(objectId), ++guard
			) {
				if (RenderGeometrySection.objectOf(objectId)?.isDiagram) {
					way.length = 0;
					way.push(hit);
					continue;
				}
				way.unshift(objectId);
			}
			for (const objectId of way)
				if (!order.includes(objectId)) order.push(objectId);
		}
		if (!order.length) return false;

		const selected = GraphicSelection.selectedObjectIDs;
		const isSelected = (objectId: number): boolean =>
			selected.includes(objectId);
		const at = backwards
			? order.map(isSelected).lastIndexOf(true)
			: order.findIndex(isSelected);
		if (at < 0) return this.selectOnlyHere(order[0], hits);

		const next = (at + (backwards ? -1 : 1) + order.length) % order.length;
		return this.selectOnlyHere(order[next], hits);
	}

	/// Ctrl+Home selects the first object of the level, Ctrl+End the last.
	public toAnEnd(first: boolean): boolean {
		const level = this.objectsOfTheLevel();
		if (!level.length) return false;

		return this.selectOnly(level[first ? 0 : level.length - 1], !first);
	}

	/*
		Escape takes the selection away; in a group that selects the group, which leaves it, so
		Escape pressed again and again goes up group by group to the page and to nothing selected.
		With nothing selected it is left to the engine.
	*/
	public escape(): boolean {
		if (!GraphicSelection.selectedObjectIDs.length) return false;

		GraphicSelection.selectAndShow(this.orTheGroup([]));
		return true;
	}

	/*
		The selection a press asks for. A plain press asks for the object it meets alone, and none
		where it meets no object. A press with Shift or Ctrl asks for the object to join what is
		selected, or to leave it when it is in there already.

		The objects of one selection all sit at the same level, so an object from another group, or
		from the page while a group's members are selected, starts a selection of its own instead
		of joining one.
	*/
	protected selectionForPress(
		objectId: number | undefined,
		extending: boolean,
	): number[] {
		const selected = GraphicSelection.selectedObjectIDs;

		if (objectId === undefined) return [];
		if (!extending) return [objectId];

		if (selected.includes(objectId))
			return selected.filter((selectedId: number) => selectedId !== objectId);

		const parent = RenderGeometrySection.parentOf(objectId);
		const sameLevel = selected.every(
			(selectedId: number) =>
				RenderGeometrySection.parentOf(selectedId) === parent,
		);

		return sameLevel ? selected.concat(objectId) : [objectId];
	}

	/*
		A drag that begins on something nobody selected takes hold of the outermost object it
		belongs to, which is the group where it is in one, as long as that may be moved. A diagram
		is taken hold of through its first object, which stands for it, carries the whole of it,
		and is what the engine reports back. A drag with a modifier is the engine's.
	*/
	public takesHoldFor(hit: number, modifier: number): number | undefined {
		if (modifier !== 0) return undefined;

		const objectId = RenderGeometrySection.outermostOf(hit, this.groupStoodIn);
		if (!ObjectHandles.canBeMoved([objectId])) return undefined;

		return this.enteredBy([objectId])[0];
	}

	/// An arrow key moves what is selected by the step EditSteps gives for the modifier held.
	public arrowKey(towards: cool.Point, event: KeyboardEvent): boolean {
		GraphicSelection.selectionSection?.moveByKey(towards, event);
		return true;
	}

	/*
		Takes hold of the first object of the group that was selected, which is what stepping into
		a group leaves behind. Stepping in leaves nothing selected on its own, and with nothing
		selected there is no band to say which group one is standing in, and nothing to add to
		with a second press either.
	*/
	public groupEntered(): void {
		const selected = GraphicSelection.selectedObjectIDs;
		if (selected.length !== 1) return;

		const held = RenderGeometrySection.heldBy(selected[0]);
		if (!held.length) return;

		GraphicSelection.selectAndShow([held[0]]);
	}

	/*
		What is left of the selection where some of its objects were deleted: the ones that still
		stand. Where none does, one stays in the group they stood in and its first object is
		selected; a group holding nothing is deleted with its last member, so the first group up
		that still stands and holds something is taken, and the page with nothing selected where
		none does.
	*/
	public whatIsLeft(selected: number[], groupsAbove: number[]): number[] {
		const standing = (objectId: number): boolean =>
			RenderGeometrySection.objectOf(objectId) !== undefined;

		const left = selected.filter(standing);
		if (left.length) return left;

		for (const group of groupsAbove) {
			if (!standing(group)) continue;
			const held = RenderGeometrySection.heldBy(group);
			if (held.length) return [held[0]];
		}
		return [];
	}
}

/// The page: what a selection of the page's objects does, and what one does with none.
class ClassicPageContext extends ClassicContext {
	constructor() {
		super(undefined);
	}

	/// With nothing selected an arrow key scrolls the view, as the office does.
	public arrowKey(towards: cool.Point, event: KeyboardEvent): boolean {
		if (!GraphicSelection.selectedObjectIDs.length)
			return ClassicPageContext.scroll(towards, event);

		return super.arrowKey(towards, event);
	}

	/*
		Scrolls the view that draws from the objects, whose view is the client's own, by the step
		EditSteps gives for the modifier held.
	*/
	private static scroll(towards: cool.Point, event: KeyboardEvent): boolean {
		const layout = app.activeDocument?.activeLayout;
		if (layout?.type !== 'ViewLayoutVector') return false;

		const view = layout.viewedRectangle;
		const step = EditSteps.scrollStep(event, view.width, view.height);
		layout.scroll(
			towards.x * step.x * app.twipsToPixels,
			towards.y * step.y * app.twipsToPixels,
			true,
		);
		return true;
	}
}

/// A group one stands in, with some of its members selected.
class ClassicGroupContext extends ClassicContext {
	constructor(group: number) {
		super(group);
	}
}

/*
	A diagram one stands in. Whatever groups a diagram holds for its own reasons stay hidden: its
	objects are one list, in the order a walk down it meets them, the first of them the shape that
	spans the whole diagram. One of them is selected at a time. Leaving the diagram goes to where
	it stands, the page or the group holding it, which stands in for the diagram itself, as a
	diagram is never selected alone.
*/
class ClassicDiagramContext extends ClassicGroupContext {
	private readonly diagram: number;

	constructor(diagram: number) {
		super(diagram);
		this.diagram = diagram;
	}

	/// The diagram's objects, one list without its groups.
	protected objectsOfTheLevel(): number[] {
		return RenderGeometrySection.theOnesThatDraw([this.diagram]);
	}

	/// The context of the place the diagram stands at.
	private around(): InteractionContext {
		return Interaction.contextAt(Interaction.placeOf([this.diagram]));
	}

	/// Leaves the diagram: the group holding it is selected, or nothing on the page.
	private leave(): boolean {
		const holder = RenderGeometrySection.parentOf(this.diagram);
		GraphicSelection.selectAndShow(holder ? [holder] : []);
		return true;
	}

	/*
		A press on an object of the diagram selects it; Shift and Ctrl add nothing there, as one
		object is selected in a diagram at a time. Shift or Ctrl on an object outside the diagram
		selects the whole diagram, as an object, together with it, where the two stand at one
		level, or that object alone where they do not. A plain press anywhere else leaves the
		diagram and is answered as the place around it answers it: another object is selected,
		empty space selects nothing, or the group holding the diagram.
	*/
	public press(hit: number | undefined, modifier: number): boolean {
		const extending =
			modifier === app.UNOModifier.SHIFT || modifier === app.UNOModifier.CTRL;
		if (extending) {
			if (hit === undefined || this.objectsOfTheLevel().includes(hit))
				return true;

			const holder = RenderGeometrySection.parentOf(this.diagram) || undefined;
			const objectId = RenderGeometrySection.outermostOf(hit, holder);
			if (objectId === this.diagram) return true;

			const sameLevel =
				RenderGeometrySection.parentOf(objectId) ===
				RenderGeometrySection.parentOf(this.diagram);
			GraphicSelection.selectObjects(
				sameLevel
					? [this.diagram, objectId]
					: this.enteredBy([objectId], [hit]),
			);
			return true;
		}
		if (modifier !== 0) return false;

		if (hit !== undefined && this.objectsOfTheLevel().includes(hit)) {
			GraphicSelection.selectObjects([hit]);
			return true;
		}

		return this.around().press(hit, modifier);
	}

	/// A double press is the engine's here, which starts the text edit of the object.
	public doublePress(_hit: number | undefined): boolean {
		return false;
	}

	/*
		Escape on an object of the diagram selects its first object, which stands for the whole
		diagram; Escape on that one leaves the diagram.
	*/
	public escape(): boolean {
		const first = this.objectsOfTheLevel()[0];
		if (first !== undefined && GraphicSelection.selectedObjectIDs[0] !== first)
			return this.selectOnly(first);

		return this.leave();
	}

	/// Whether the first object is selected, which stands for the whole diagram: then the
	/// diagram is what is selected, at the level around it.
	private wholeSelected(): boolean {
		return (
			GraphicSelection.selectedObjectIDs[0] === this.objectsOfTheLevel()[0]
		);
	}

	/*
		Leave Group, and Ctrl+F3, on an object of the diagram go to its first object, the whole
		diagram, as Escape does. With the whole diagram selected it is an object at the level
		around it: inside a group they leave that group, selecting it; on the page there is
		nothing to leave.
	*/
	public leaveKey(): boolean {
		if (!this.wholeSelected()) return this.escape();

		return RenderGeometrySection.parentOf(this.diagram) ? this.leave() : false;
	}

	/// Enter and F3 are the engine's here, Enter starting the text edit of the object.
	public enterKey(): boolean {
		return false;
	}

	/*
		A diagram gives no hint of being a group but by Ungroup, which is offered wherever one
		stands in it and takes the whole diagram apart. Enter Group is never offered. Leave
		Group is offered inside the diagram, on its other objects, and with the whole diagram
		selected as for any object at the level around it: only where a group holds it. The
		whole diagram is moved and arranged as an object; its other objects take no geometry,
		so nothing that moves, arranges, groups, flips or changes them to curves is offered on
		them, while naming them and their area and line are.
	*/
	public commandStates(): { [command: string]: boolean } {
		const states = super.commandStates();
		const whole = this.wholeSelected();
		states['.uno:EnterGroup'] = false;
		states['.uno:LeaveGroup'] =
			!whole || !!RenderGeometrySection.parentOf(this.diagram);
		states['.uno:FormatUngroup'] = true;
		if (!whole) {
			states['.uno:FormatGroup'] = false;
			for (const command of Interaction.commandsForOneOrMore)
				states[command] = false;
			for (const command of Interaction.commandsForThreeOrMore)
				states[command] = false;
			states['.uno:FlipHorizontal'] = false;
			states['.uno:FlipVertical'] = false;
			states['.uno:ChangeBezier'] = false;
		}
		return states;
	}

	/// The whole diagram, where its first object stands for it, or the object of it selected.
	protected objectsTheCommandsGoTo(): number[] {
		return this.wholeSelected()
			? [this.diagram]
			: GraphicSelection.selectedObjectIDs;
	}

	/// Ungroup takes the diagram apart, whichever of its objects is selected.
	public ungroup(): boolean {
		GraphicSelection.takeApart(this.diagram);
		return true;
	}

	/*
		Tab goes to the next object of the diagram, Shift+Tab to the one before; past the last,
		or before the first, it leaves the diagram for the object after it, or before it, where the
		diagram stands - entering that one where it is a diagram too.
	*/
	public tabKey(forwards: boolean): boolean {
		const objects = this.objectsOfTheLevel();
		const at = objects.indexOf(GraphicSelection.selectedObjectIDs[0]);
		const to = at + (forwards ? 1 : -1);
		if (at >= 0 && to >= 0 && to < objects.length)
			return this.selectOnly(objects[to], !forwards);

		const holder = RenderGeometrySection.parentOf(this.diagram);
		const outside = holder
			? RenderGeometrySection.heldBy(holder)
			: RenderGeometrySection.heldByThePage();
		const place = outside.indexOf(this.diagram);
		if (place < 0) return false;

		const next =
			(place + (forwards ? 1 : -1) + outside.length) % outside.length;
		return this.selectOnly(outside[next], !forwards);
	}
}

class ClassicInteractionSet implements InteractionSet {
	public contextFor(place: InteractionPlace): InteractionContext {
		switch (place.kind) {
			case 'group':
				return new ClassicGroupContext(place.group);
			case 'diagram':
				return new ClassicDiagramContext(place.diagram);
			default:
				return new ClassicPageContext();
		}
	}
}
