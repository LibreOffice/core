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
	private documentScrollOffset: number = 0;

	constructor() {
		super();
		app.map.on('zoomlevelschange', this.documentZoomCallback, this);
		app.map.on('deleteannotation', this.annotationOperationsCallback, this);
		app.map.on('insertannotation', this.annotationOperationsCallback, this);
		app.map.on('importannotations', this.annotationOperationsCallback, this);
		app.map.on(
			'showannotationschanged',
			this.annotationOperationsCallback,
			this,
		);
	}

	public unselectCommentOnScroll() {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		) as cool.CommentSection;

		if (commentSection && commentSection.sectionProperties.selectedComment) {
			commentSection.unselect();
		}
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

	private documentCanMoveLeft(ignoreDocumentScrollOffset: boolean) {
		const spacingInfo = this.getCommentAndDocumentSpacingInfo();
		const offset = ignoreDocumentScrollOffset ? 0 : this.documentScrollOffset;

		const commentsWiderThanRightMargin =
			spacingInfo.documentMarginsWidth + offset <
			spacingInfo.commentSectionWidth;

		const haveEnoughLeftMarginForMove =
			spacingInfo.commentSectionWidth -
				(spacingInfo.documentMarginsWidth + offset) <=
			spacingInfo.documentMarginsWidth - offset;

		return commentsWiderThanRightMargin && haveEnoughLeftMarginForMove;
	}

	/*
		`cool.CommentSection.shouldCollapse()` doesn't need `documentScrollOffset`
		details to know if it `shouldCollapse` the comments or not.
	*/
	public viewHasEnoughSpaceToShowFullWidthComments() {
		const spacingInfo = this.getCommentAndDocumentSpacingInfo();
		return (
			spacingInfo.documentMarginsWidth * 2 >= spacingInfo.commentSectionWidth
		);
	}

	private documentMoveLeftByOffset(): number {
		const spacingInfo = this.getCommentAndDocumentSpacingInfo();
		return (
			spacingInfo.commentSectionWidth -
			(spacingInfo.documentMarginsWidth + this.documentScrollOffset)
		);
	}

	public getDocumentScrollOffset() {
		if (this.commentsHiddenOrNotPresent()) return 0;
		if (!this.viewHasEnoughSpaceToShowFullWidthComments()) return 0;

		if (this.documentCanMoveLeft(true)) {
			this.documentScrollOffset = 0;
			this.documentScrollOffset = this.documentMoveLeftByOffset();
			return this.documentScrollOffset;
		}

		return 0;
	}

	private recenterDocument() {
		if (this.documentScrollOffset == 0) return;

		this.scrollHorizontal(-this.documentScrollOffset, true);
		this.documentScrollOffset = 0;
		app.sectionContainer.requestReDraw();
	}

	private commentsHiddenOrNotPresent() {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		) as cool.CommentSection;

		if (commentSection.commentsHiddenOrNotPresent()) {
			this.recenterDocument();
			return true;
		}
		return false;
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

	private adjustDocumentMarginsForComments(onZoom: boolean) {
		this.unselectSelectedCommentIfAny();

		if (this.commentsHiddenOrNotPresent()) return;

		if (this.documentCanMoveLeft(onZoom)) {
			if (onZoom) this.documentScrollOffset = 0;
			this.documentScrollOffset = this.documentMoveLeftByOffset();
			/*
			 * we scrollHorizontal by 1 to trigger the layouting tasks in
			 * the `ScrollSection.doMove` function, which calls `map.panBy`
			 * to adjust the document center, and there we add the offset
			 * to the x component to move the document to the left.
			 * we only do it for the zoom events because for resize,
			 * the layouting tasks are scheduled automatically by other
			 * code, like that in CommentListSection.ts (updateDOM)
			 */
			this.scrollHorizontal(1, true);
		}
	}

	private documentZoomCallback() {
		this.adjustDocumentMarginsForComments(true);
	}

	private annotationOperationsCallback() {
		this.adjustDocumentMarginsForComments(false);
	}
}
