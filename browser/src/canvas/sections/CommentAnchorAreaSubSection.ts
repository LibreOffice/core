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
	Outlined "anchor area" rectangle drawn underneath a Draw/Impress comment
	when the comment carries an explicit area (PDF /Rect or the user's
	drag-to-area selection in fileBasedView). The small CommentMarkerSubSection
	icon stays at the rectangle's top-left; this section adds a clickable
	box covering the whole area so the user can activate the comment by
	clicking anywhere on it, not only on the icon.
*/

class CommentAnchorAreaSubSection extends HTMLObjectSection {
	constructor(
		sectionName: string,
		objectWidth: number,
		objectHeight: number,
		documentPosition: cool.SimplePoint,
		showSection: boolean,
		parentSection: any,
		data: any,
	) {
		super(
			sectionName,
			objectWidth,
			objectHeight,
			documentPosition,
			'annotation-anchor-area',
			showSection,
		);
		this.sectionProperties.parentSection = parentSection;
		this.sectionProperties.data = data;

		// Take pointer events directly. The default HTMLObjectSection
		// passes them through to the canvas so the section container can
		// hit-test by section bounds; that path lost clicks here
		// intermittently (likely because the marker icon is added to the
		// same documentObject layer and its hit area shadowed the
		// area's). Listening on the div is simpler and the area is
		// purpose-built to "clicking activates the comment."
		this.sectionProperties.objectDiv.style.pointerEvents = 'auto';
		this.sectionProperties.objectDiv.style.cursor = 'pointer';
		this.sectionProperties.objectDiv.addEventListener(
			'click',
			(e: MouseEvent) => {
				e.stopPropagation();
				this.sectionProperties.parentSection.sectionProperties.commentListSection.selectById(
					this.sectionProperties.data.id,
				);
			},
		);
		// Swallow mousedown so the underlying canvas sections (e.g.
		// MouseControl forwarding to core) don't see the click as a
		// page interaction.
		this.sectionProperties.objectDiv.addEventListener(
			'mousedown',
			(e: MouseEvent) => {
				e.stopPropagation();
			},
		);
	}

	public resize(widthPx: number, heightPx: number): void {
		this.size = [widthPx * app.dpiScale, heightPx * app.dpiScale];
		this.sectionProperties.objectWidth = widthPx;
		this.sectionProperties.objectHeight = heightPx;
		this.sectionProperties.objectDiv.style.width = widthPx + 'px';
		this.sectionProperties.objectDiv.style.height = heightPx + 'px';
	}
}

app.definitions.commentAnchorAreaSubSection = CommentAnchorAreaSubSection;
