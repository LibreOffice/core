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
/* See CanvasSectionContainer.ts for explanations. */

app.definitions.CellFillMarkerSection = class CellFillMarkerSection extends (
	AutoFillBaseSection
) {
	processingOrder: number = app.CSections.CellFillMarker.processingOrder;
	drawingOrder: number = app.CSections.CellFillMarker.drawingOrder;
	zIndex: number = app.CSections.CellFillMarker.zIndex;

	constructor() {
		super(app.CSections.CellFillMarker.name);
	}
};
