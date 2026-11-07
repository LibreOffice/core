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

describe('VectorCompositor', function () {
	describe('a paragraph in the state of its shape', function () {
		it('moves with the shape and after its own transform', function () {
			// The shape moved right by 100, the paragraph is flying in from 50 above.
			const state = VectorCompositor.overOwner(
				{ transform: [1, 0, 0, 1, 100, 0] },
				{ transform: [1, 0, 0, 1, 0, -50] },
			);
			nodeassert.deepStrictEqual(state.transform, [1, 0, 0, 1, 100, -50]);
		});

		it('is as transparent as both together', function () {
			const state = VectorCompositor.overOwner({ alpha: 0.5 }, { alpha: 0.5 });
			nodeassert.strictEqual(state.alpha, 0.25);
		});

		it('keeps the shape alone moving it when it has no transform of its own', function () {
			const state = VectorCompositor.overOwner(
				{ transform: [2, 0, 0, 2, 0, 0], alpha: 0.5 },
				{},
			);
			nodeassert.deepStrictEqual(state.transform, [2, 0, 0, 2, 0, 0]);
			nodeassert.strictEqual(state.alpha, 0.5);
		});

		it('takes the colors of the shape where it has none of its own', function () {
			const state = VectorCompositor.overOwner(
				{ colorOverride: { fill: '#ff0000' }, textOverride: { weight: 8 } },
				{ textOverride: { italic: true } },
			);
			nodeassert.deepStrictEqual(state.colorOverride, { fill: '#ff0000' });
			nodeassert.deepStrictEqual(state.textOverride, { italic: true });
		});

		it('is hidden when the shape is', function () {
			const state = VectorCompositor.overOwner(
				{ hidden: true },
				{ transform: [1, 0, 0, 1, 0, -50] },
			);
			nodeassert.deepStrictEqual(state, { hidden: true });
		});

		it('changes nothing when neither has a state', function () {
			nodeassert.deepStrictEqual(VectorCompositor.overOwner({}, {}), {});
		});
	});
});
