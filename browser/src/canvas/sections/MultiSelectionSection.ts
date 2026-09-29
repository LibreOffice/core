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
	Several selected objects, held together as one thing. One frame of eight handles goes around
	them all and scales every one of them, which is what the office shows for such a selection.
	The objects themselves offer nothing of their own here: a handle that shapes one of them
	would say nothing about the others.
*/
class MultiSelectionSection extends SelectionSection {
	private objectIds: number[];

	constructor(objectIds: number[]) {
		super(MultiSelectionSection.sectionName);

		this.objectIds = objectIds.slice();
	}

	/// What the section of a selection of several objects is called.
	public static readonly sectionName: string = 'selected-objects';

	/// Takes the objects the selection now holds and works its handles out again.
	public setObjects(objectIds: number[]): void {
		this.objectIds = objectIds.slice();
		this.forgetTheSquare();
		this.refresh();
	}

	public selectedObjects(): number[] {
		return this.objectIds;
	}
}

app.definitions.multiSelectionSection = MultiSelectionSection;
