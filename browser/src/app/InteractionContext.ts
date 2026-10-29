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
	| { kind: 'inPlace'; object: number };

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

	/// Whether the groups the selection stands in are marked by bands, with the buttons beside
	/// them. Inside an object active in place its surround marks the level instead.
	showsGroupBands(): boolean;

	/// Ungroup, which takes apart what is selected. Answers whether it was run here; one that is
	/// not goes to the engine.
	ungroup(): boolean;

	/// Tab, forwards, or with Shift backwards.
	tabKey(forwards: boolean): boolean;

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
		return Interaction.contextAt(
			Interaction.placeOf(GraphicSelection.selectedObjectIDs),
		);
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
