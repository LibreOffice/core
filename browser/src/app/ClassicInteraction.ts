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
			this.orTheGroup(this.selectionForPress(objectId, extending)),
		);
		return true;
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

	/// Selects that one object alone and brings it on screen, for a key.
	private selectOnly(objectId: number): boolean {
		GraphicSelection.selectAndShow([objectId]);
		return true;
	}

	/// Selects that one object alone where the mouse is, which is on screen already.
	private selectOnlyHere(objectId: number): boolean {
		GraphicSelection.selectObjects([objectId]);
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
			return this.selectOnly(level[forwards ? 0 : level.length - 1]);

		const from = forwards ? Math.max(...places) : Math.min(...places);
		const to = (from + (forwards ? 1 : -1) + level.length) % level.length;
		return this.selectOnly(level[to]);
	}

	/*
		Alt+press goes through the objects under the press, deep: every object on the way from the
		page down to each object the press meets, in the order a walk from the front to the back
		meets them, a group before what it holds. It selects the one after the selected one, or
		with Shift the one before it, going round at either end. So a group is selected first, the
		next press enters it, selecting the member under the press, and after its last member under
		the press one leaves it for the object behind. A diagram is taken whole, until what a
		diagram does is defined. With nothing under the press selected, the frontmost is taken.
	*/
	public altPress(hits: number[], backwards: boolean): boolean {
		const order: number[] = [];
		for (const hit of hits) {
			// The way from the page down to the object met, a diagram ending it.
			const way: number[] = [];
			for (
				let objectId: number | undefined = hit, guard = 0;
				objectId && guard < 100;
				objectId = RenderGeometrySection.parentOf(objectId), ++guard
			) {
				if (RenderGeometrySection.objectOf(objectId)?.isDiagram) way.length = 0;
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
		if (at < 0) return this.selectOnlyHere(order[0]);

		const next = (at + (backwards ? -1 : 1) + order.length) % order.length;
		return this.selectOnlyHere(order[next]);
	}

	/// Ctrl+Home selects the first object of the level, Ctrl+End the last.
	public toAnEnd(first: boolean): boolean {
		const level = this.objectsOfTheLevel();
		if (!level.length) return false;

		return this.selectOnly(level[first ? 0 : level.length - 1]);
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
		belongs to, which is the group where it is in one, as long as that may be moved. A drag
		with a modifier is the engine's.
	*/
	public takesHoldFor(hit: number, modifier: number): number | undefined {
		if (modifier !== 0) return undefined;

		const objectId = RenderGeometrySection.outermostOf(hit, this.groupStoodIn);
		return ObjectHandles.canBeMoved([objectId]) ? objectId : undefined;
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

/// A diagram one stands in. It does what a group does until it is defined on its own.
class ClassicDiagramContext extends ClassicGroupContext {}

class ClassicInteractionSet implements InteractionSet {
	public contextFor(place: InteractionPlace): InteractionContext {
		switch (place.kind) {
			case 'group':
				return new ClassicGroupContext(place.group);
			case 'diagram':
				return new ClassicDiagramContext(place.group);
			default:
				return new ClassicPageContext();
		}
	}
}
