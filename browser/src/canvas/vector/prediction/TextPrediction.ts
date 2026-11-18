/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
 * The typing shown here before the engine answers. A change is applied at the caret in a tree of
 * primitives and kept, under the key its owner gives it, until an answer of the engine includes
 * it. The changes an answer does not include are applied again to what the answer brought, and a
 * caret update of the engine that predates the newest change leaves the caret where the typing
 * put it. The owner says which tree the caret is in and where the tree lies in the document.
 */

namespace cool {
	/// The caret of this view that a change is typed at. logical is its paragraph and index,
	/// or null when the engine did not report them. anchor is the other end of a selection, or
	/// null. selected is true when selected text is drawn.
	export interface TypingCaret {
		logical: LogicalCaret | null;
		anchor: LogicalCaret | null;
		selected: boolean;
	}

	/// The tree a change goes into, and the position of the caret in it.
	export interface TypingPlace {
		tree: PredictionTree;
		position: CaretPosition;
	}

	/// A change kept until the engine answers it, with the key that names what it went into.
	export type KeyedEdit<Key> = UnansweredEdit & Key;

	/// A change that was applied at the caret.
	export interface TypedChange<Place extends TypingPlace> {
		/// The place the change went into.
		place: Place;
		/// The caret after the change, or null for a caret without paragraph and index.
		caretAfter: LogicalCaret | null;
		/// How far the change moved the caret along its line, in the twips of the tree.
		caretShift: number;
		/// True when the change replaced selected text.
		replacedSelection: boolean;
	}

	/// What a caret update of the engine is compared with.
	export interface CaretUpdateCheck<Edit> {
		/// True when the object the update names is the one the newest change went into.
		inEdit: (objectId: number, newest: Edit) => boolean;
		/// Picks the changes the update is about. Without it every change counts.
		matches?: (edit: Edit) => boolean;
		/// Where the engine puts the caret once it has the newest change. Without it, the
		/// caret after that change.
		expected?: (newest: Edit) => LogicalCaret;
		/// Runs when the update includes the newest change and puts the caret elsewhere,
		/// so the typing no longer matches the engine.
		elsewhere: () => void;
	}

	export class TextPrediction<Key> {
		/// The changes the engine has not answered, oldest first.
		private edits = new UnansweredEdits<KeyedEdit<Key>>();

		/// The other end of a selection that lies in the paragraph of the caret, or null
		/// when there is no selection or it ends in another paragraph.
		static anchorInParagraph(caret: TypingCaret): LogicalCaret | null {
			const anchor = caret.anchor;
			return anchor &&
				caret.logical &&
				anchor.paragraph === caret.logical.paragraph
				? anchor
				: null;
		}

		/// True when a change at the caret shows here. With replacing, the change replaces a
		/// selection, which must lie in one paragraph with both ends known. A selection
		/// over a paragraph break alone draws no rectangle, so its anchor counts as a
		/// selection too.
		static covers(caret: TypingCaret, replacing: boolean): boolean {
			const selected = caret.selected || caret.anchor !== null;
			if (!selected) return true;
			return replacing && TextPrediction.anchorInParagraph(caret) !== null;
		}

		/// The position of a caret drawn at the rectangle, or null. The rectangle picks the
		/// object, and the logical caret the place in its text.
		static caretAt(
			tree: PredictionTree,
			rectangle: SimpleRectangle,
			logical: LogicalCaret | null,
		): CaretPosition | null {
			return tree.caretAt(
				rectangle.x1,
				rectangle.y1,
				rectangle.y2,
				logical ?? undefined,
			);
		}

		/// True when a keystroke at the caret would show here. find gives the place of the
		/// caret, or null. Nothing changes.
		canTypeAt(caret: TypingCaret, find: () => TypingPlace | null): boolean {
			return TextPrediction.covers(caret, false) && find() !== null;
		}

		/// Applies a change at the caret and keeps it until the engine answers. find gives
		/// the place of the caret, or null. build gets the code points selected around the
		/// caret in its paragraph, or null without a selection, and returns the change.
		/// recordedAs gives the ticket of the input message and the key of the change, or
		/// null for a change that is shown and not kept. A change at a caret without
		/// paragraph and index is not kept either. Null when nothing changed.
		type<Place extends TypingPlace>(
			caret: TypingCaret,
			find: () => Place | null,
			build: (selected: SelectedAround | null) => PredictedChange,
			recordedAs: (place: Place) => (Key & { ticket: InputTicket }) | null,
		): TypedChange<Place> | null {
			if (!TextPrediction.covers(caret, true)) return null;
			const place = find();
			if (!place) return null;
			const position = place.position;
			const anchor = TextPrediction.anchorInParagraph(caret);
			const selected = anchor ? selectedAround(position, anchor) : null;
			const applied = applyPredictedChange(
				place.tree,
				position,
				caret.logical,
				build(selected),
			);
			if (!applied) return null;

			let caretAfter: LogicalCaret | null = null;
			if (caret.logical) {
				caretAfter = {
					paragraph: caret.logical.paragraph,
					index: Math.max(0, caret.logical.index + applied.indexShift),
					atEnd: applied.position.atEnd,
				};
				const key = recordedAs(place);
				if (key)
					this.edits.record({
						...key,
						caretBefore: { ...caret.logical, atEnd: position.atEnd },
						caretAfter,
						...applied.change,
					});
			}
			return {
				place,
				caretAfter,
				caretShift: applied.caretShift,
				replacedSelection: selected !== null,
			};
		}

		/// Applies kept changes again, oldest first. placeOf gives the tree and the object
		/// a change goes into, or null when it has no place any more. False as soon as one
		/// change does not apply, which leaves the ones before it applied.
		static applyAgain<Edit extends UnansweredEdit>(
			edits: Edit[],
			placeOf: (
				edit: Edit,
			) => { tree: PredictionTree; objectId: number } | null,
		): boolean {
			return UnansweredEdits.applyAgain(edits, (edit) => {
				const place = placeOf(edit);
				if (!place) return false;
				const position = place.tree.caretIn(place.objectId, edit.caretBefore);
				if (!position) return false;
				return (
					applyPredictedChange(place.tree, position, edit.caretBefore, edit) !==
					null
				);
			});
		}

		/// True when the update is older than the newest change, so the caret typed here
		/// stays.
		keepsCaret(
			update: CaretUpdate,
			check: CaretUpdateCheck<KeyedEdit<Key>>,
		): boolean {
			const verdict = this.edits.verdictOn(
				update,
				check.inEdit,
				check.matches,
				check.expected,
			);
			if (verdict === CaretUpdateVerdict.Elsewhere) check.elsewhere();
			return verdict === CaretUpdateVerdict.Older;
		}

		/// True while changes that match wait for the engine.
		has(matches?: (edit: KeyedEdit<Key>) => boolean): boolean {
			return this.edits.has(matches);
		}

		/// The newest change that matches, or undefined.
		newest(
			matches?: (edit: KeyedEdit<Key>) => boolean,
		): KeyedEdit<Key> | undefined {
			return this.edits.newest(matches);
		}

		/// Drops the changes that match.
		forget(matches?: (edit: KeyedEdit<Key>) => boolean): void {
			this.edits.forget(matches);
		}

		/// Drops the changes that match and that an answer of the engine includes: those up
		/// to inputSequence, or all of them when it is undefined. Returns the matching
		/// changes left, oldest first.
		answered(
			inputSequence: number | undefined,
			matches?: (edit: KeyedEdit<Key>) => boolean,
		): KeyedEdit<Key>[] {
			return this.edits.answered(inputSequence, matches);
		}
	}

	/// Removes the selection of this view, which is gone with the text a change replaced.
	export function dropReplacedSelection(): void {
		app.file.textCursor.anchor = null;
		if (TextSelections.isActive())
			(
				app.map._docLayer as unknown as CanvasTileLayerInterface
			).showTextSelection([]);
	}
}
