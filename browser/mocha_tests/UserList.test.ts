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

describe('UserList with linked documents', function () {
	// Our own view, a colleague, and the view through which a presentation of
	// that colleague reads the document.
	const myView = 1;
	const aliceView = 2;
	const aliceLinkedView = 3;
	const aliceSecondLinkedView = 4;
	const carolLinkedView = 5;

	let list: any;
	let messages: string[];
	let followedViews: number[];
	let savedFollowing: any;

	function addView(
		viewId: number,
		userId: string,
		username: string,
		remoteLink: boolean,
		parked: boolean = false,
	) {
		list.onAddView({
			viewId: viewId,
			userId: userId,
			username: username,
			extraInfo: {},
			readonly: remoteLink,
			parked: parked,
			remoteLink: remoteLink,
		});
	}

	beforeEach(function () {
		messages = [];
		followedViews = [];
		savedFollowing = app.following;
		app.following = { mode: 'none', viewId: -1 } as any;

		list = new UserList();
		list.map = {
			_docLayer: { _viewId: myView },
			getViewColor: () => 0,
			_setFollowing: (_on: boolean, viewId: number) =>
				followedViews.push(viewId),
		};
		list.renderAll = () => {};
		list.showJoinLeaveMessage = (
			type: string,
			_viewId: number,
			username: string,
		) => messages.push(type + ' ' + username);

		addView(myView, 'me', 'Me', false);
		addView(aliceView, 'alice', 'Alice', false);
		messages = [];
	});

	afterEach(function () {
		app.following = savedFollowing;
	});

	it('a linked document is no person: it has no row and is not counted', function () {
		addView(aliceLinkedView, 'alice', 'Alice', true);
		addView(carolLinkedView, 'carol', 'Carol', true);

		const rows = Array.from(list.getSortedUsers()).map(
			(entry: any) => entry[0],
		);
		nodeassert.deepEqual(rows, [myView, aliceView]);
		nodeassert.equal(list.getListedUserCount(), 2);
	});

	it('the linked presentations of one user make one entry', function () {
		addView(aliceLinkedView, 'alice', 'Alice', true);
		addView(carolLinkedView, 'carol', 'Carol', true);
		addView(aliceSecondLinkedView, 'alice', 'Alice', true);

		nodeassert.deepEqual(list.getLinkedViewUsers(), ['Alice', 'Carol']);
	});

	it('a linked document is not followed', function () {
		addView(aliceLinkedView, 'alice', 'Alice', true);

		list.followUser(aliceLinkedView);

		nodeassert.equal(app.isFollowingUser(), false);
		nodeassert.deepEqual(followedViews, []);
	});

	it('following a user stays on their own view when their linked document acts', function () {
		addView(aliceLinkedView, 'alice', 'Alice', true);
		list.followUser(aliceView);
		followedViews = [];

		list.moveFollowingToActiveView(aliceLinkedView);

		nodeassert.equal(app.getFollowedViewId(), aliceView);
		nodeassert.deepEqual(followedViews, []);
	});

	it('a user whose browser is away shows away while their linked document reads', function () {
		list.users.get(aliceView).parked = true;
		addView(aliceLinkedView, 'alice', 'Alice', true);

		nodeassert.equal(list.isUserParked(list.users.get(aliceView)), true);
	});

	it('a linked document coming and going says nothing', function () {
		addView(carolLinkedView, 'carol', 'Carol', true);
		list.onRemoveView({ viewId: carolLinkedView });

		nodeassert.deepEqual(messages, []);
	});

	it('a user leaves when their last own view closes, though their linked document stays', function () {
		addView(aliceLinkedView, 'alice', 'Alice', true);
		list.followUser(aliceView);

		list.onRemoveView({ viewId: aliceView });

		nodeassert.deepEqual(messages, ['leave Alice']);
		nodeassert.equal(app.isFollowingUser(), false);
	});
});
