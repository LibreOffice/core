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

describe('Collaboration notices in a document window', function () {
	// Permission.js adds its methods to the map, so they run with the map itself as
	// "this". A bare object built on those methods stands in for the map.
	function mapShowing(shown: any[]): any {
		const methods = (globalThis as any).L.Map.included.find(
			(set: any) => set && typeof set._onCollabUserJoined === 'function',
		);
		const map = Object.create(methods);
		map._showTwoCardDialog = (
			id: string,
			title: any,
			subtitle: string,
			cards: any[],
		) => {
			shown.push({ title, choices: cards.map((card) => card.heading), cards });
		};
		map.uiManager = { isAnyDialogOpen: () => false };
		return map;
	}

	afterEach(function () {
		delete (window as any).collabEditingActive;
		delete (window as any).collabUsers;
	});

	it('someone opening the document being edited is named, with a choice of how to go on', function () {
		const shown: any[] = [];
		const map = mapShowing(shown);
		map.isEditMode = () => true;

		map._onCollabUserJoined('Robin');

		nodeassert.equal(shown.length, 1);
		nodeassert.equal(shown[0].title, 'Robin opened this document');
		nodeassert.deepEqual(shown[0].choices, [
			'Edit locally',
			'Collaborative editing',
		]);
	});

	it('choosing to edit together saves the local changes and joins the others', function () {
		const shown: any[] = [];
		const map = mapShowing(shown);
		map.isEditMode = () => true;
		let joined = 0;
		map._saveAndSwitchToServerMode = () => joined++;

		map._onCollabUserJoined('Robin');
		shown[0].cards[1].onClick();

		nodeassert.equal(joined, 1);
	});

	it('someone starting to edit while this window edits is named the same way', function () {
		const shown: any[] = [];
		const map = mapShowing(shown);
		map.isEditMode = () => true;

		map._onOtherUserEditingStarted('Robin');

		nodeassert.equal(shown[0].title, 'Robin started editing');
		nodeassert.deepEqual(shown[0].choices, [
			'Edit locally',
			'Collaborative editing',
		]);
	});
});
