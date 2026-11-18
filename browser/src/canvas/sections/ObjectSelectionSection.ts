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
	One selected object. It names the object by its id and works everything it draws out of the
	geometry the client holds for it: the eight handles that frame it and the ones that shape
	it. A path that belongs to the object, the one an animation follows, becomes a further
	source of handles here.
*/
class ObjectSelectionSection extends SelectionSection {
	private objectId: number;

	constructor(objectId: number) {
		super(ObjectSelectionSection.nameFor(objectId));

		this.objectId = objectId;
	}

	/// What the section of that object is called among the sections of the document.
	public static nameFor(objectId: number): string {
		return 'selected-object-' + String(objectId);
	}

	public selectedObjects(): number[] {
		return [this.objectId];
	}
}

app.definitions.objectSelectionSection = ObjectSelectionSection;
