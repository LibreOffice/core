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

type DocumentSpacingInfo = {
	documentMarginsWidth: number;
	commentSectionWidth: number;
};

class ViewLayoutWriter extends ViewLayoutBase {
	public readonly type: string = 'ViewLayoutWriter';

	// Cached comment-margin shift (canvas pixels) and its dirty flag. The shift is
	// recomputed only when the comment layout can actually change (annotation
	// add/remove/import, show/hide, zoom, resize) - never on plain scrolling.
	// Scrolling rebuilds the viewed rectangle too, so without this cache the
	// shift would be recomputed against transient comment state mid-scroll and
	// the page would drift horizontally. Freezing it keeps the post-scroll
	// placement identical to the initial one.
	private commentMarginShift = 0;
	private commentMarginDirty = true;

	constructor() {
		super();

		// On a mid-session swap into this layout (e.g. leaving multi-page or
		// compare-changes view) the file size is already known, so seed the
		// scrollable extent from it - the off-map path needs viewSize for
		// centering and vertical scrolling. On first construction during document
		// load fileSize is not set yet (optional chaining skips it) and
		// WriterTileLayer._setNewSize sets it from the first status.
		if (app.activeDocument?.fileSize?.x)
			this.viewSize = app.activeDocument.fileSize.clone();

		// The comment column shifts the page horizontally, so the view is rebuilt
		// whenever the comment set or its visibility changes.
		app.map.on('zoomlevelschange', this.onCommentLayoutChange, this);
		app.map.on('deleteannotation', this.onCommentLayoutChange, this);
		app.map.on('insertannotation', this.onCommentLayoutChange, this);
		app.map.on('importannotations', this.onCommentLayoutChange, this);
		app.map.on('showannotationschanged', this.onCommentLayoutChange, this);

		// A resize changes both the frame (base centering) and the side margins
		// (comment shift), so the shift must be recomputed on the next rebuild.
		app.events.on('resize', () => {
			this.commentMarginDirty = true;
		});
	}

	// Writer places one continuous page column with the inherited single-window
	// machinery, centred horizontally when the page is narrower than the viewport.
	protected override usesSingleWindowView(): boolean {
		return true;
	}

	// Horizontal placement of the page: centre it using the stable page width,
	// then shift it left by the comment margin so the comment column fits on the
	// right. Vertical centring stays a no-op (the document is taller than the
	// viewport). Folded into the viewed rectangle on rebuild, so drawing
	// (documentToViewX) and hit-testing stay consistent from that one source.
	protected override getCenteringOffset(): number[] {
		Util.ensureValue(app.activeDocument);

		const frame = this.frameSize;
		// Centre on fileSize, NOT viewSize: the comment section inflates viewSize
		// to (page + comment column) so the comment stays scrollable
		// (CommentListSection.update), which makes viewSize.x toggle and the page
		// jump on scroll. fileSize is the stable page extent, and it is also what
		// the comment section anchors the comment column to (fileSize.cX in
		// CommentListSection.update) - so centring on it keeps the page and the
		// comment aligned.
		const content = app.activeDocument.fileSize;
		if (content.pX <= 0) return [0, 0]; // before the first status

		// Centre the page horizontally, then shift it left by the (cached) comment
		// margin. Vertical stays a no-op (the document is taller than the viewport).
		let centerX = Math.max(0, Math.round((frame.pX - content.pX) / 2));

		// Recompute the comment shift only when it may have changed; otherwise
		// reuse the cached value so scrolling never moves the page horizontally.
		if (this.commentMarginDirty) {
			this.commentMarginShift = this.computeDocumentScrollOffset();
			this.commentMarginDirty = false;
		}
		centerX -= this.commentMarginShift;

		const centerY = Math.max(0, Math.round((frame.pY - content.pY) / 2));
		return [centerX, centerY];
	}

	private getCommentAndDocumentSpacingInfo(): DocumentSpacingInfo {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		) as cool.CommentSection;

		return {
			documentMarginsWidth: commentSection.calculateAvailableSpace(),
			commentSectionWidth: commentSection.sectionProperties.commentWidth,
		} as DocumentSpacingInfo;
	}

	private documentCanMoveLeft(): boolean {
		const spacingInfo = this.getCommentAndDocumentSpacingInfo();

		const commentsWiderThanRightMargin =
			spacingInfo.documentMarginsWidth < spacingInfo.commentSectionWidth;

		const haveEnoughLeftMarginForMove =
			spacingInfo.commentSectionWidth - spacingInfo.documentMarginsWidth <=
			spacingInfo.documentMarginsWidth;

		return commentsWiderThanRightMargin && haveEnoughLeftMarginForMove;
	}

	// Whether both side margins together are wide enough to show the comment
	// column at full width without shifting the page. Also read by
	// CommentSection.shouldCollapse().
	public viewHasEnoughSpaceToShowFullWidthComments() {
		const spacingInfo = this.getCommentAndDocumentSpacingInfo();
		return (
			spacingInfo.documentMarginsWidth * 2 >= spacingInfo.commentSectionWidth
		);
	}

	// How far to shift the page left so the comment column fits on the right, in
	// canvas pixels - the unit the comment column width and the side margin are
	// both measured in. Pure: no scrolling, no selection changes. Called by
	// getCenteringOffset only when the comment margin is marked dirty.
	private computeDocumentScrollOffset(): number {
		// Can run during early load (a rebuild before the comment section exists),
		// so tolerate a missing section.
		if (
			!app.sectionContainer.getSectionWithName(app.CSections.CommentList.name)
		)
			return 0;

		if (this.commentsHiddenOrNotPresent()) return 0;
		if (!this.viewHasEnoughSpaceToShowFullWidthComments()) return 0;

		if (this.documentCanMoveLeft()) {
			const spacingInfo = this.getCommentAndDocumentSpacingInfo();
			return spacingInfo.commentSectionWidth - spacingInfo.documentMarginsWidth;
		}

		return 0;
	}

	private commentsHiddenOrNotPresent(): boolean {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		) as cool.CommentSection;

		return commentSection.commentsHiddenOrNotPresent();
	}

	private unselectSelectedCommentIfAny() {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		) as cool.CommentSection;

		if (
			commentSection.sectionProperties.selectedComment &&
			!commentSection.sectionProperties.selectedComment.isEdit()
		) {
			commentSection.unselect();
		}
	}

	private onCommentLayoutChange(): void {
		this.unselectSelectedCommentIfAny();
		// The comment layout changed, so the shift must be recomputed on the next
		// rebuild; then rebuild the viewed rectangle (which folds in the new offset
		// through getCenteringOffset) and refresh the visible area / tiles.
		this.commentMarginDirty = true;
		this.updateViewData();
		app.sectionContainer.requestReDraw();
	}
}
