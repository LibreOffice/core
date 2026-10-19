/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('StateMachine', function () {
	// A document goes through review: the author submits a draft, the reviewer reads it and
	// either approves it or sends it back for another round.
	enum ReviewState {
		Draft,
		InReview,
		Approved,
	}

	enum ReviewTrigger {
		Author,
		Reviewer,
	}

	let warnings: string[];
	let originalWarn: (...data: any[]) => void;

	beforeEach(function () {
		warnings = [];
		originalWarn = app.console.warn;
		app.console.warn = function (message: string) {
			warnings.push(message);
		};
	});

	afterEach(function () {
		app.console.warn = originalWarn;
	});

	function makeReview(
		extra: Partial<StateMachineOptions<ReviewState, ReviewTrigger>> = {},
		actions: string[] = [],
	): StateMachine<ReviewState, ReviewTrigger> {
		return new StateMachine<ReviewState, ReviewTrigger>({
			name: 'DocumentReview',
			initial: ReviewState.Draft,
			stateNames: ReviewState,
			triggerNames: ReviewTrigger,
			transitions: [
				{
					from: ReviewState.Draft,
					to: ReviewState.InReview,
					triggers: [ReviewTrigger.Author],
					action: () => actions.push('reviewer notified'),
				},
				{
					from: ReviewState.InReview,
					to: ReviewState.InReview,
					action: () => actions.push('reviewer notified again'),
				},
				{ from: ReviewState.InReview, to: ReviewState.Approved },
				{ from: ReviewState.Approved, to: ReviewState.Draft },
			],
			...extra,
		});
	}

	it('starts in the initial state', function () {
		const review = makeReview();
		nodeassert.equal(review.state, ReviewState.Draft);
		nodeassert.ok(review.is(ReviewState.Draft));
		nodeassert.ok(review.is(ReviewState.Approved, ReviewState.Draft));
		nodeassert.ok(!review.is(ReviewState.Approved, ReviewState.InReview));
	});

	it('takes a listed move and runs its action after the state changed', function () {
		const seen: ReviewState[] = [];
		const review = makeReview({
			transitions: [
				{
					from: ReviewState.Draft,
					to: ReviewState.Approved,
					action: () => seen.push(review.state),
				},
			],
		});
		nodeassert.ok(
			review.transition(
				ReviewState.Draft,
				ReviewState.Approved,
				ReviewTrigger.Author,
			),
		);
		nodeassert.equal(review.state, ReviewState.Approved);
		nodeassert.deepEqual(seen, [ReviewState.Approved]);
		nodeassert.deepEqual(warnings, []);
	});

	it('refuses a move that is not listed and keeps the state', function () {
		const review = makeReview();
		nodeassert.ok(
			!review.transition(
				ReviewState.Draft,
				ReviewState.Approved,
				ReviewTrigger.Author,
			),
		);
		nodeassert.equal(review.state, ReviewState.Draft);
		nodeassert.equal(warnings.length, 1);
		nodeassert.ok(warnings[0].includes('DocumentReview'));
		nodeassert.ok(warnings[0].includes('Draft'));
		nodeassert.ok(warnings[0].includes('Approved'));
		nodeassert.ok(warnings[0].includes('Author'));
	});

	it('refuses a move when the caller names the wrong current state', function () {
		const actions: string[] = [];
		const review = makeReview({}, actions);
		nodeassert.ok(
			!review.transition(
				ReviewState.Approved,
				ReviewState.Draft,
				ReviewTrigger.Author,
			),
		);
		nodeassert.equal(review.state, ReviewState.Draft);
		nodeassert.deepEqual(actions, []);
		nodeassert.equal(warnings.length, 1);
		nodeassert.ok(warnings[0].includes('current state is Draft'));
	});

	it('limits a move to its listed triggers', function () {
		const actions: string[] = [];
		const review = makeReview({}, actions);
		nodeassert.ok(
			!review.transition(
				ReviewState.Draft,
				ReviewState.InReview,
				ReviewTrigger.Reviewer,
			),
		);
		nodeassert.equal(review.state, ReviewState.Draft);
		nodeassert.deepEqual(actions, []);
		nodeassert.ok(
			review.transition(
				ReviewState.Draft,
				ReviewState.InReview,
				ReviewTrigger.Author,
			),
		);
		nodeassert.deepEqual(actions, ['reviewer notified']);
	});

	it('reports a change to a different state only', function () {
		const changes: string[] = [];
		const actions: string[] = [];
		const review = makeReview(
			{
				onChanged: (from, to, trigger) =>
					changes.push(
						ReviewState[from] +
							'>' +
							ReviewState[to] +
							':' +
							ReviewTrigger[trigger],
					),
			},
			actions,
		);
		review.transition(
			ReviewState.Draft,
			ReviewState.InReview,
			ReviewTrigger.Author,
		);
		review.transition(
			ReviewState.InReview,
			ReviewState.InReview,
			ReviewTrigger.Reviewer,
		);
		review.transition(
			ReviewState.InReview,
			ReviewState.Approved,
			ReviewTrigger.Reviewer,
		);
		nodeassert.deepEqual(changes, [
			'Draft>InReview:Author',
			'InReview>Approved:Reviewer',
		]);
		nodeassert.deepEqual(actions, [
			'reviewer notified',
			'reviewer notified again',
		]);
		nodeassert.deepEqual(warnings, []);
	});
});
