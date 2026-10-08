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
	- a diagram context, where the members selected are held by a diagram, however far up it is.

	Exactly one of the three applies at any time. In a group context and a diagram context there is
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
	| { kind: 'diagram'; group: number };

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

	/// An arrow key, pointing that way.
	arrowKey(towards: cool.Point, event: KeyboardEvent): boolean;

	/// Escape, once a drag, a text edit and the handle mode have had it and left it.
	escape(): boolean;

	/// Enter with no modifier.
	enterKey(): boolean;

	/// The engine was asked to enter the group that is selected; selects what that leaves.
	groupEntered(): void;

	/// Objects of the selection were deleted, by this view or another. Answers the selection to
	/// take instead, from what was selected and the groups it stood in, from its parent up.
	whatIsLeft(selected: number[], groupsAbove: number[]): number[];
}

/// A way of editing: the context it gives for each place one can stand at.
interface InteractionSet {
	contextFor(place: InteractionPlace): InteractionContext;
}

class Interaction {
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
				return { kind: 'diagram', group: parent };
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
