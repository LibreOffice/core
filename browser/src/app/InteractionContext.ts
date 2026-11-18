/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
	How a view that draws from the objects answers the mouse and the keyboard on the objects of a
	page. What an action means depends on where one stands, and that is read from the selection
	alone, with nothing kept beside it:

	- the page context, with nothing selected or with objects of the page selected;
	- a group context, with members of a group selected;
	- a diagram context, where the members selected are held by a diagram, however far up it is;
	- an in-place context, while an object is active in place, a chart being edited. That one is
	  read from the engine's report of it, before the selection.

	Exactly one of the four applies at any time. In a group context and a diagram context there is
	always a selection; only in the page context can there be none. A selected group is a selected
	object and stands in the context of its parent: it is not entered.

	The three come from an interaction set, and the set in use decides how editing behaves as a
	whole. The classic set, which follows the office, is the one there is; another way of editing is
	written as another set of three.
*/

/// Where one stands, as the selection says it.
type InteractionPlace =
	| { kind: 'page' }
	| { kind: 'group'; group: number }
	| { kind: 'diagram'; group: number; diagram: number }
	| { kind: 'inPlace'; object: number }
	// one table selected, standing on the page or in that group
	| { kind: 'table'; table: number; group: number | undefined };

/// What one context answers. Each action answers whether it was taken here; one that is not goes
/// on to the engine.
interface InteractionContext {
	/// A press of the left button on that object, or on no object, with the modifiers as the
	/// engine numbers them.
	press(hit: number | undefined, modifier: number): boolean;

	/// The object a drag that began on that object takes hold of and carries, or nothing where
	/// such a drag is not carried here.
	takesHoldFor(hit: number, modifier: number): number | undefined;

	/// A double press of the left button on that object, or on no object.
	doublePress(hit: number | undefined): boolean;

	/// A press with Alt, or with Alt and Shift for backwards, on those objects, from the front to
	/// the back: all the objects under the press, not only the topmost.
	altPress(hits: number[], backwards: boolean): boolean;

	/// An arrow key, pointing that way.
	arrowKey(towards: cool.Point, event: KeyboardEvent): boolean;

	/// Escape, once a drag, a text edit and the handle mode have had it and left it.
	escape(): boolean;

	/// Enter with no modifier, or F3, which enters a group.
	enterKey(): boolean;

	/// Ctrl+F3, which leaves a group.
	leaveKey(): boolean;

	/// Whether the commands whose state the client gives itself may be used where one stands,
	/// by command name.
	commandStates(): { [command: string]: boolean };

	/// A key went down or up, before anything else handled it, with the modifiers held as they are
	/// after it. Answers whether the key was used here, so nothing else handles it.
	keyEvent(event: KeyboardEvent, modifier: number): boolean;

	/// Whether the groups the selection stands in are marked by bands, with the buttons beside
	/// them. Inside an object active in place its surround marks the level instead.
	showsGroupBands(): boolean;

	/// The objects that have a say in what may be done to these ones: they may be moved, resized,
	/// sheared or turned only where every one of those allows it.
	theOnesThatHaveASay(objectIds: number[]): number[];

	/// The objects a change of these ones is named to the engine by, each with a mapping of its
	/// own.
	theOnesAChangeNames(objectIds: number[]): number[];

	/// How far a move of these objects by that much takes them, in twips, where a part goes a
	/// way of its own.
	heldToItsWay(objectIds: number[], by: cool.Point): cool.Point;

	/// Ungroup, which takes apart what is selected. Answers whether it was run here; one that is
	/// not goes to the engine.
	ungroup(): boolean;

	/// Tab, forwards, or with Shift backwards.
	tabKey(forwards: boolean): boolean;

	/// The selection a frame drawn over that range of the page makes, in twips: the objects of
	/// the level one stands at that lie wholly inside it; with extending those change their
	/// state in the selection there is. Nothing where no frame is drawn here.
	frameSelects(frame: cool.Range2D, extending: boolean): number[] | undefined;

	/// The selection a lasso drawn through those places of the page makes, in twips, closed back
	/// to its start: the objects of the level one stands at that lie wholly inside it; with
	/// extending those change their state in the selection there is. Nothing where no lasso is
	/// drawn here.
	lassoSelects(lasso: cool.Point[], extending: boolean): number[] | undefined;

	/// Ctrl+Home for the first object, Ctrl+End for the last.
	toAnEnd(first: boolean): boolean;

	/// The engine was asked to enter the group that is selected; selects what that leaves.
	groupEntered(): void;

	/// The selection to take for that one where taking it as it is would be no place to stand:
	/// a diagram selected alone, for one. The objects under a press, from the front to the back,
	/// and the way a key goes help to choose.
	enteredBy(
		selection: number[],
		hits?: number[],
		backwards?: boolean,
	): number[];

	/// Objects of the selection were deleted, by this view or another. Answers the selection to
	/// take instead, from what was selected and the groups it stood in, from its parent up.
	whatIsLeft(selected: number[], groupsAbove: number[]): number[];
}

/*
	A context one is put into by choosing a tool, not by what is selected: creating an object by
	a drag, for one. While it is on it takes every press, every movement of the mouse and every
	key, and the selection waits under it.
*/
interface InteractionMode extends InteractionContext {
	/// The mouse is at that point of the page, in twips, with those modifiers, the button up or
	/// down.
	mouseAt(at: cool.Point, modifier: number, pressed: boolean): void;
	/// The left button went down at that point of the page.
	mouseDown(at: cool.Point, modifier: number): void;
	/// The button came up at that point; dragged says whether the mouse moved far enough in
	/// between to count as a drag.
	mouseUp(at: cool.Point, modifier: number, dragged: boolean): void;
	/// Whether the right button is the mode's, so a press of it opens no menu.
	takesTheRightButton(): boolean;
	/// The right button went down at that point of the page.
	rightDown(at: cool.Point, modifier: number): void;
	/// The right button came up at that point of the page.
	rightUp(at: cool.Point, modifier: number): void;
	/// The drag was called off, by the container or by a key.
	dragCancelled(): void;
	/// The mouse left the page.
	mouseLeft(): void;
	/// The pointer to show over the page.
	pointer(): string;
	/// Draws what the mode shows over the page, into the section that lies over it.
	draw(section: CanvasSectionObject): void;
	/// Whether something drawn still waits for the engine after the mode is over.
	stillDraws(): boolean;
	/// The mode is over; what it held is let go.
	ended(): void;
}

/// A way of editing: the context it gives for each place one can stand at.
interface InteractionSet {
	contextFor(place: InteractionPlace): InteractionContext;
}

class Interaction {
	/// The commands that move or arrange what is selected, which need one object or more.
	public static readonly commandsForOneOrMore: string[] = [
		'.uno:ObjectAlignLeft',
		'.uno:AlignCenter',
		'.uno:ObjectAlignRight',
		'.uno:AlignUp',
		'.uno:AlignMiddle',
		'.uno:AlignDown',
		'.uno:ObjectAlign',
		'.uno:BringToFront',
		'.uno:ObjectForwardOne',
		'.uno:ObjectBackOne',
		'.uno:SendToBack',
		'.uno:TransformDialog',
	];

	/// The commands that name one object, which need exactly one.
	public static readonly commandsForOne: string[] = [
		'.uno:NameGroup',
		'.uno:ObjectTitleDescription',
	];

	/// The commands that spread objects out, which need three or more.
	public static readonly commandsForThreeOrMore: string[] = [
		'.uno:DistributeSelection',
		'.uno:DistributeHorzLeft',
		'.uno:DistributeHorzCenter',
		'.uno:DistributeHorzDistance',
		'.uno:DistributeHorzRight',
		'.uno:DistributeVertTop',
		'.uno:DistributeVertCenter',
		'.uno:DistributeVertDistance',
		'.uno:DistributeVertBottom',
	];

	/// The commands that depend on the kind of the objects selected and on what they allow.
	public static readonly commandsByKind: string[] = [
		'.uno:FlipHorizontal',
		'.uno:FlipVertical',
		'.uno:ChangeBezier',
		'.uno:ConvertToEditableShape',
		'.uno:FormatArea',
		'.uno:FormatLine',
	];

	/// Every command whose state a view that draws from the objects gives itself, from where the
	/// selection stands, rather than taking the engine's.
	public static readonly statesGivenHere: string[] = [
		'.uno:EnterGroup',
		'.uno:LeaveGroup',
		'.uno:FormatUngroup',
		'.uno:FormatGroup',
		...Interaction.commandsForOneOrMore,
		...Interaction.commandsForOne,
		...Interaction.commandsForThreeOrMore,
		...Interaction.commandsByKind,
	];

	// the objectKind of a table
	private static readonly tableKind = 35;

	/// The set in use. It is the classic one for now; which set to use is to become a setting.
	private static setInUse: InteractionSet | null = null;

	private static set(): InteractionSet {
		if (!Interaction.setInUse)
			Interaction.setInUse = new ClassicInteractionSet();
		return Interaction.setInUse;
	}

	/*
		Where the selection says one stands: the page where nothing is selected or the parent of
		what is selected is the page, a diagram where that parent or any parent above it is one,
		a group otherwise. The objects of a selection all have the same parent, so the first says
		it for all of them.
	*/
	public static placeOf(selected: number[]): InteractionPlace {
		// An object active in place takes everything while it is, whatever it selects inside.
		if (
			RenderManager.isVectorRendering() &&
			GraphicSelection.inPlaceActive &&
			GraphicSelection.inPlaceArea &&
			GraphicSelection.inPlaceObject !== undefined
		)
			return { kind: 'inPlace', object: GraphicSelection.inPlaceObject };

		const first = selected[0];
		const parent =
			first === undefined ? undefined : RenderGeometrySection.parentOf(first);

		// one table selected is a place of its own, for its cells
		if (
			selected.length === 1 &&
			RenderGeometrySection.objectOf(first)?.objectKind ===
				Interaction.tableKind
		)
			return { kind: 'table', table: first, group: parent || undefined };

		if (!parent) return { kind: 'page' };

		for (
			let above: number | undefined = parent, guard = 0;
			above && guard < 100;
			above = RenderGeometrySection.parentOf(above), ++guard
		) {
			if (RenderGeometrySection.objectOf(above)?.isDiagram)
				return { kind: 'diagram', group: parent, diagram: above };
		}

		return { kind: 'group', group: parent };
	}

	/// The context for the selection as it is now.
	public static context(): InteractionContext {
		// a mode one was put into takes everything while it is on
		if (Interaction.mode) return Interaction.mode;
		return Interaction.contextAt(
			Interaction.placeOf(GraphicSelection.selectedObjectIDs),
		);
	}

	/// Whether the keys are followed, which they are once.
	private static followingTheKeys: boolean = false;

	/*
		Follows every key that goes down or up, ahead of every other listener of the page, so the
		context one stands in has it first, wherever the focus is. A key it uses goes no further.
	*/
	public static followTheKeys(): void {
		if (Interaction.followingTheKeys) return;
		Interaction.followingTheKeys = true;
		const listener = (event: KeyboardEvent) => {
			if (!RenderManager.drawsFromObjects() || !Interaction.keyEvent(event))
				return;
			event.preventDefault();
			event.stopPropagation();
		};
		window.addEventListener('keydown', listener, true);
		window.addEventListener('keyup', listener, true);
	}

	/*
		Hands a key to the context one stands in, before anything else handles it. Answers whether
		the context used it.
	*/
	public static keyEvent(event: KeyboardEvent): boolean {
		if (event.type !== 'keydown' && event.type !== 'keyup') return false;
		return Interaction.context().keyEvent(
			event,
			Interaction.modifiersOf(event),
		);
	}

	/*
		The modifiers held after a key, as the engine numbers them. The key that went down or up
		counts as held or let go by the kind of the event: on some systems the event of a modifier
		key reports the modifiers as they were before it.
	*/
	public static modifiersOf(event: KeyboardEvent): number {
		const held = (key: string, flag: boolean) =>
			event.key === key ? event.type === 'keydown' : flag;
		return (
			(held('Shift', event.shiftKey) ? app.UNOModifier.SHIFT : 0) |
			(held('Control', event.ctrlKey) ? app.UNOModifier.CTRL : 0) |
			(held('Alt', event.altKey) ? app.UNOModifier.ALT : 0) |
			(held('Meta', event.metaKey) ? app.UNOModifier.CTRLMAC : 0)
		);
	}

	/// The mode that is on, or null while one stands where the selection says.
	private static mode: InteractionMode | null = null;

	/// The mode that is on.
	public static modeOn(): InteractionMode | null {
		return Interaction.mode;
	}

	/// Puts one into that mode, ending the one that was on.
	public static enterMode(mode: InteractionMode): void {
		Interaction.leaveMode();
		Interaction.mode = mode;
		InteractionModeSection.follow();
	}

	/// Ends the mode that is on, where one is. Answers whether one was.
	public static leaveMode(): boolean {
		const mode = Interaction.mode;
		if (!mode) return false;
		Interaction.mode = null;
		mode.ended();
		InteractionModeSection.follow();
		return true;
	}

	/// The context for that place, which may be one the selection stood at before.
	public static contextAt(place: InteractionPlace): InteractionContext {
		return Interaction.set().contextFor(place);
	}

	/// The groups above those objects, from their parent up to the one the page holds.
	public static groupsAbove(selected: number[]): number[] {
		const groups: number[] = [];
		const first = selected[0];
		let above =
			first === undefined ? undefined : RenderGeometrySection.parentOf(first);

		for (let guard = 0; above && guard < 100; ++guard) {
			groups.push(above);
			above = RenderGeometrySection.parentOf(above);
		}
		return groups;
	}
}
