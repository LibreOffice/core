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

describe('SlideAvatars', function () {
	const slotCounts = [1, 2, 3, 5];
	const peopleCounts = [
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 400,
	];

	function circles(slots: { faces: number; hidden: number }): number {
		return slots.faces + (slots.hidden > 0 ? 1 : 0);
	}

	slotCounts.forEach(function (slotCount) {
		describe('with ' + slotCount + ' slots', function () {
			it('accounts for everyone on the slide', function () {
				peopleCounts.forEach(function (people) {
					const slots = SlideAvatars.slots(people, slotCount);
					nodeassert.equal(
						slots.faces + slots.hidden,
						people,
						'people: ' + people,
					);
				});
			});

			it('never draws more circles than there are slots', function () {
				peopleCounts.forEach(function (people) {
					nodeassert.ok(
						circles(SlideAvatars.slots(people, slotCount)) <= slotCount,
						'people: ' + people,
					);
				});
			});

			it('counts nobody while everyone fits', function () {
				peopleCounts.forEach(function (people) {
					const slots = SlideAvatars.slots(people, slotCount);
					if (slots.hidden > 0)
						nodeassert.ok(people > slotCount, 'people: ' + people);
				});
			});

			it('never counts a single person', function () {
				peopleCounts.forEach(function (people) {
					const hidden = SlideAvatars.slots(people, slotCount).hidden;
					nodeassert.ok(hidden === 0 || hidden >= 2, 'people: ' + people);
				});
			});

			// The face count is deliberately not monotonic: the last
			// face is traded for the counter once people stop fitting.
			it('trades one face for the counter, and only once', function () {
				let drops = 0;
				for (let people = 1; people <= 20; people++) {
					const before = SlideAvatars.slots(people - 1, slotCount).faces;
					const after = SlideAvatars.slots(people, slotCount).faces;
					if (after < before) {
						drops++;
						nodeassert.equal(before - after, 1, 'people: ' + people);
					}
				}
				nodeassert.equal(drops, 1);
			});

			it('counts more people as more people arrive', function () {
				let previous = 0;
				for (let people = 0; people <= 20; people++) {
					const hidden = SlideAvatars.slots(people, slotCount).hidden;
					nodeassert.ok(hidden >= previous, 'people: ' + people);
					previous = hidden;
				}
			});

			it('never shrinks the row', function () {
				let previous = 0;
				for (let people = 0; people <= 20; people++) {
					const drawn = circles(SlideAvatars.slots(people, slotCount));
					nodeassert.ok(drawn >= previous, 'people: ' + people);
					previous = drawn;
				}
			});
		});
	});

	it('shows nobody for an empty or negative count', function () {
		[0, -1, -400].forEach(function (people) {
			nodeassert.deepEqual(SlideAvatars.slots(people), {
				faces: 0,
				hidden: 0,
			});
		});
	});

	it('writes the counter as a plus and one digit', function () {
		for (let hidden = 2; hidden <= 40; hidden++)
			nodeassert.ok(
				/^\+[2-9]$/.test(SlideAvatars.counterText(hidden)),
				'hidden: ' + hidden,
			);
	});

	it('stops the counter at nine', function () {
		[9, 10, 40, 400].forEach(function (hidden) {
			nodeassert.equal(SlideAvatars.counterText(hidden), '+9');
		});
	});
});
