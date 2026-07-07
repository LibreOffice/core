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

/*
 * SpacerSection - a layout-only section that reserves empty space next to the
 * document-anchor (tiles) section.
 *
 * The tiles section expands into all free space. When the document content is
 * smaller than the frame, a spacer sized to the leftover space sits at the
 * given edge; because it shares the tiles section's zIndex and is processed
 * first, the tiles section stops at the spacer and shrinks to the content size.
 * The spacer draws nothing; its size is driven externally (see
 * CalcTileLayer._updateSpacerSizes). When there is no leftover space its size
 * is [0, 0] and it has no effect.
 *
 * The constructor's `side` selects which edge the spacer hugs: 'right' fills
 * horizontal leftover space, 'bottom' fills vertical leftover space.
 */

namespace cool {
	export class SpacerSection extends CanvasSectionObject {
		anchor: string[];
		processingOrder: number;
		drawingOrder: number;
		zIndex: number;
		interactable: boolean = false;

		constructor(side: 'right' | 'bottom') {
			const props =
				side === 'right'
					? app.CSections.RightSpacer
					: app.CSections.BottomSpacer;

			super(props.name);

			// 'right' hugs the right edge and spans full height; 'bottom' hugs the
			// bottom edge and spans full width. The right spacer's anchor is
			// flipped to the left edge for RTL Calc where content is anchored to
			// the right.
			this.anchor = side === 'right' ? ['top', 'right'] : ['bottom', 'left'];
			this.processingOrder = props.processingOrder;
			this.drawingOrder = props.drawingOrder;
			this.zIndex = props.zIndex;
			this.size = [0, 0];
		}
	}
} // namespace cool
