// @ts-strict-ignore
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
/* See CanvasSectionContainer.ts for explanations. */

declare var Autolinker: any;
declare var DOMPurify : any;

// By default DOMPurify will strip all targets, so set everything
// as target=_blank with rel=noopener
DOMPurify.addHook('afterSanitizeAttributes', function (node: HTMLElement) {
	if (node.tagName === 'A' && !node.classList.contains('context-menu-link')) {
		node.setAttribute('target', '_blank');
		node.setAttribute('rel', 'noopener');
	}
});

namespace cool {

/*
	data.layoutStatus: Enumartion sent from the core side.
	0: INVISIBLE, 1: VISIBLE, 2: INSERTED, 3: DELETED, 4: NONE, 5: HIDDEN
	Ex: "DELETED" means that the comment is deleted while the "track changes" is on.
*/
export enum CommentLayoutStatus {
	INVISIBLE,
	VISIBLE,
	INSERTED,
	DELETED,
	NONE,
	HIDDEN
}

export class Comment extends CanvasSectionObject {
	// Cache the expensive to localize frequently created strings
	static readonly editCommentLabel = _('Edit comment');
	static readonly replyCommentLabel = _('Reply comment');
	static readonly openMenuLabel = _('Open menu');

	processingOrder: number = app.CSections.Comment.processingOrder;
	drawingOrder: number = app.CSections.Comment.drawingOrder;
	zIndex: number = app.CSections.Comment.zIndex;

	valid: boolean = true;
	map: any;
	pendingInit: boolean = true;

	cachedCommentHeight: number | null = null;
	cachedIsEdit: boolean = false;
	hidden: boolean | null = null;

	containerPosX: number = 0;
	containerPosY: number = 0;
	canvasContainerBounds: DOMRect = new DOMRect();

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public static makeName(data: any): string {
		return data.id === 'new' ? 'new comment' : 'comment ' + data.id;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	constructor (name: string, data: any, options: any, commentListSectionPointer: cool.CommentSection) {
		super(name);

		this.myTopLeft = [0, 0];
		this.documentObject = true;
		this.map = window.L.Map.THIS;

		if (!options)
			options = {};

		this.sectionProperties.commentListSection = commentListSectionPointer;
		this.sectionProperties.docLayer = this.map._docLayer;

		this.sectionProperties.selectedAreaPoint = null;
		this.sectionProperties.cellCursorPoint = null;

		this.sectionProperties.draggingStarted = false;
		this.sectionProperties.dragStartPosition = null;

		this.sectionProperties.minWidth = options.minWidth ? options.minWidth : 160;
		this.sectionProperties.maxHeight = options.maxHeight ? options.maxHeight : 50;
		this.sectionProperties.imgSize = options.imgSize ? options.imgSize : [32, 32];
		this.sectionProperties.margin = options.margin ? options.margin : [40, 40];
		this.sectionProperties.noMenu = options.noMenu ? options.noMenu : false;

		if (data.parent === undefined)
			data.parent = '0';

		this.setData(data);

		/*
			possibleParentCommentId:
				* User deletes a parent comment.
				* User deletes also its child comment.
				* User reverts the last change (deletion of child comment).
				* A comment "Add" action is sent from the core side.
				* The child comment has also its parent id.
				* But there is no such parent at the moment.
				* So we will remember its possible parent comment in case user also reverts the deletion of parent comment.
				* In that case, parent comment will come with its old id.
				* Child comment can now find its parent.
				* We will check child comment to see if its parent has also been revived.
		*/
		this.sectionProperties.possibleParentCommentId = null;
		this.sectionProperties.wrapper = null;
		this.sectionProperties.container = null;
		this.sectionProperties.author = null;
		this.sectionProperties.resolvedTextElement = null;
		this.sectionProperties.removedTextElement = null;
		this.sectionProperties.authorAvatarImg = null;
		this.sectionProperties.authorAvatartdImg = null;
		this.sectionProperties.contentAuthor = null;
		this.sectionProperties.contentDate = null;
		this.sectionProperties.acceptButton = null;
		this.sectionProperties.rejectButton = null;
		this.sectionProperties.menu = null;
		this.sectionProperties.captionNode = null;
		this.sectionProperties.captionText = null;

		this.sectionProperties.contentNode = null;
		this.sectionProperties.nodeModify = null;
		this.sectionProperties.nodeModifyText = null;
		this.sectionProperties.saveButton = null;
		this.sectionProperties.cancelButton = null;
		this.sectionProperties.replyButton = null;
		this.sectionProperties.cancelReplyButton = null;
		this.sectionProperties.contentText = null;
		this.sectionProperties.nodeReply = null;
		this.sectionProperties.nodeReplyText = null;
		this.sectionProperties.contextMenu = false;

		this.sectionProperties.highlightedTextColor = '#777777'; // Writer.
		this.sectionProperties.usedTextColor = this.sectionProperties.data.color; // Writer.
		this.sectionProperties.showSelectedCoordinate = true; // Writer.

		if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') {
			this.sectionProperties.parthash = parseInt(this.sectionProperties.data.parthash);
			this.sectionProperties.partIndex = app.impress.getIndexFromSlideHash(this.sectionProperties.parthash);
		}

		this.sectionProperties.isHighlighted = false;

		this.sectionProperties.commentContainerRemoved = false;
		this.sectionProperties.children = []; // This is used for Writer comments. There is parent / child relationship between comments in Writer files.
		this.sectionProperties.childLinesNode = null;
		this.sectionProperties.childLines = [];
		this.sectionProperties.childCommentOffset = 8;
		this.sectionProperties.commentMarkerSubSection = null; // For Impress and Draw documents.
		this.sectionProperties.calcCommentAreaWidth = 40; // Calc comment area doesn't cover the whole cell, in order to allow multi-cell selections.

		app.map.on('sheetgeometrychanged', this.setPositionAndSize.bind(this));
	}

	// Comments import can be costly if the document has a lot of them. If they are all imported/initialized
	// when online gets comments message from core, the initial doc render is delayed. To avoid that we do
	// lazy import of each comment when it needs to be shown (based on its coordinates).
	private doPendingInitializationInView (force: boolean = false): void {
		if (!this.pendingInit)
			return;

		if (!force) {
			if (!this.convertRectanglesToViewCoordinates())
				return;

			// skip comments on other tabs than the current
			if (app.map._docLayer._docType === 'spreadsheet' && parseInt(this.sectionProperties.data.tab) !== app.map._docLayer._selectedPart)
				return;
		}

		var button = window.L.DomUtil.create('div', 'annotation-btns-container', this.sectionProperties.nodeModify);
		window.L.DomEvent.on(this.sectionProperties.nodeModifyText, 'focus', this.onFocus, this);
		window.L.DomEvent.on(this.sectionProperties.nodeReplyText, 'focus', this.onFocusReply, this);
		window.L.DomEvent.on(this.sectionProperties.nodeModifyText, 'input', this.textAreaInput, this);
		window.L.DomEvent.on(this.sectionProperties.nodeReplyText, 'input', this.textAreaInput, this);
		window.L.DomEvent.on(this.sectionProperties.nodeModifyText, 'keydown', this.textAreaKeyDown, this);
		window.L.DomEvent.on(this.sectionProperties.nodeReplyText, 'keydown', this.textAreaKeyDown, this);
		this.sectionProperties.cancelButton = this.createButton(button, 'annotation-cancel-' + this.sectionProperties.data.id, 'annotation-button button-secondary', _('Cancel'), this.handleCancelCommentButton);
		this.sectionProperties.saveButton = this.createButton(button, 'annotation-save-' + this.sectionProperties.data.id, 'annotation-button button-primary',_('Save'), this.handleSaveCommentButton);
		button = window.L.DomUtil.create('div', '', this.sectionProperties.nodeReply);
		this.sectionProperties.cancelReplyButton = this.createButton(button, 'annotation-cancel-reply-' + this.sectionProperties.data.id, 'annotation-button button-secondary', _('Cancel'), this.handleCancelCommentButton);
		this.sectionProperties.replyButton = this.createButton(button, 'annotation-reply-' + this.sectionProperties.data.id, 'annotation-button button-primary', _('Reply'), this.handleReplyCommentButton);
		window.L.DomEvent.disableScrollPropagation(this.sectionProperties.container);

		// Since this is a late called function, if the width is enough, we shouldn't collapse the comments.
		if (app.map._docLayer._docType !== 'text' || this.sectionProperties.commentListSection.isCollapsed === true)
			this.sectionProperties.container.style.visibility = 'hidden';

		this.sectionProperties.nodeModify.style.display = 'none';
		this.sectionProperties.nodeReply.style.display = 'none';

		var events = ['click', 'dblclick', 'mousedown', 'mouseup', 'mouseover', 'mouseout', 'keydown', 'keypress', 'keyup', 'touchstart', 'touchmove', 'touchend'];
		window.L.DomEvent.on(this.sectionProperties.container, 'click', this.onMouseClick, this);
		window.L.DomEvent.on(this.sectionProperties.container, 'keydown', this.onEscKey, this);

		for (var it = 0; it < events.length; it++) {
			window.L.DomEvent.on(this.sectionProperties.container, events[it], window.L.DomEvent.stopPropagation, this);
		}

		window.L.DomEvent.on(this.sectionProperties.container, 'touchstart',
			function (e: TouchEvent) {
				if (e && e.touches.length > 1) {
					window.L.DomEvent.preventDefault(e);
				}
			},
			this);

		this.update();

		this.pendingInit = false;
	}

	public onInitialize (): void {
		this.createContainerAndWrapper();

		this.createAuthorTable();

		if (this.sectionProperties.data.trackchange && !this.map.isReadOnlyMode() && !app.map['wopi'].HideChangeTrackingControls) {
			this.createTrackChangeButtons();
		}

		if (this.sectionProperties.noMenu !== true && app.isCommentEditingAllowed()) {
			this.createMenu();
		}

		if (this.sectionProperties.data.trackchange) {
			this.sectionProperties.captionNode = window.L.DomUtil.create('div', 'cool-annotation-caption', this.sectionProperties.wrapper);
			this.sectionProperties.captionText = window.L.DomUtil.create('div', '', this.sectionProperties.captionNode);
		}

		this.sectionProperties.contentNode = window.L.DomUtil.create('div', 'cool-annotation-content cool-dont-break', this.sectionProperties.wrapper);
		this.sectionProperties.contentNode.id = 'annotation-content-area-' + this.sectionProperties.data.id;
		this.sectionProperties.contentNode.setAttribute('tabindex', '-1');

		const commentFooter = window.L.DomUtil.create('div', 'cool-annotation-footer', this.sectionProperties.wrapper);
		this.sectionProperties.contentDate = window.L.DomUtil.create('div', 'cool-annotation-date', commentFooter);
		const resolvedEl = window.L.DomUtil.create('div', 'cool-annotation-content-resolved', commentFooter);
		this.sectionProperties.resolvedTextElement = resolvedEl;
		this.updateResolvedField(this.sectionProperties.data.resolved);


		this.sectionProperties.nodeModify = window.L.DomUtil.create('div', 'cool-annotation-edit' + ' modify-annotation', this.sectionProperties.wrapper);
		this.sectionProperties.nodeModifyText = window.L.DomUtil.create('div', 'cool-annotation-textarea', this.sectionProperties.nodeModify);
		this.createReplyHint(this.sectionProperties.nodeModify);
		this.sectionProperties.nodeModifyText.setAttribute('contenteditable', 'true');
		this.sectionProperties.nodeModifyText.setAttribute('role', 'textbox');
		this.sectionProperties.nodeModifyText.setAttribute('aria-label', Comment.editCommentLabel);
		this.sectionProperties.nodeModifyText.id = 'annotation-modify-textarea-' + this.sectionProperties.data.id;
		this.sectionProperties.contentText = window.L.DomUtil.create('div', '', this.sectionProperties.contentNode);
		this.sectionProperties.nodeReply = window.L.DomUtil.create('div', 'cool-annotation-edit' + ' reply-annotation', this.sectionProperties.wrapper);
		this.sectionProperties.nodeReplyText = window.L.DomUtil.create('div', 'cool-annotation-textarea', this.sectionProperties.nodeReply);
		this.createReplyHint(this.sectionProperties.nodeReply);
		this.sectionProperties.nodeReplyText.setAttribute('contenteditable', 'true');
		this.sectionProperties.nodeReplyText.setAttribute('role', 'textbox');
		this.sectionProperties.nodeReplyText.setAttribute('aria-label', Comment.replyCommentLabel);
		this.sectionProperties.nodeReplyText.id = 'annotation-reply-textarea-' + this.sectionProperties.data.id;
		this.createChildLinesNode();

		this.sectionProperties.container.style.visibility = 'hidden';

		if (this.sectionProperties.commentMarkerSubSection === null && app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing')
			this.createMarkerSubSection();

		this.doPendingInitializationInView();

		if (!(<any>window).mode.isSmallScreenDevice())
			document.getElementById('document-container').appendChild(this.sectionProperties.container);
	}

	private createContainerAndWrapper (): void {
		var isRTL = document.documentElement.dir === 'rtl';
		this.sectionProperties.container = window.L.DomUtil.create('div', 'cool-annotation' + (isRTL ? ' rtl' : ''));
		this.sectionProperties.container.id = 'comment-container-' + this.sectionProperties.data.id;
		window.L.DomEvent.on(this.sectionProperties.container, 'focusout', this.onLostFocus, this);

		var mobileClass = (<any>window).mode.isSmallScreenDevice() ? ' wizard-comment-box': '';

		if (this.sectionProperties.data.trackchange) {
			this.sectionProperties.wrapper = window.L.DomUtil.create('div', 'cool-annotation-redline-content-wrapper' + mobileClass, this.sectionProperties.container);
		} else {
			this.sectionProperties.wrapper = window.L.DomUtil.create('div', 'cool-annotation-content-wrapper' + mobileClass, this.sectionProperties.container);
		}

		this.sectionProperties.wrapper.style.marginLeft = this.sectionProperties.childCommentOffset*this.getChildLevel() + 'px';

		if (document.documentElement.dir === 'rtl')
			this.sectionProperties.wrapper.dir = 'rtl';

		// We make comment directly visible when its transitioned to its determined position
		if (cool.CommentSection.autoSavedComment)
			this.sectionProperties.container.style.visibility = 'hidden';
	}

	private createAuthorTable (): void {
		this.sectionProperties.author = window.L.DomUtil.create('table', 'cool-annotation-table', this.sectionProperties.wrapper);

		var tbody = window.L.DomUtil.create('tbody', '', this.sectionProperties.author);

		var tr = window.L.DomUtil.create('tr', 'cool-annotation-author-header', tbody);
		this.sectionProperties.authorRow = tr;
		tr.id = 'author table row ' + this.sectionProperties.data.id;
		var tdImg = window.L.DomUtil.create('td', 'cool-annotation-img', tr);
		var tdAuthor = window.L.DomUtil.create('td', 'cool-annotation-author', tr);
		var imgAuthor = window.L.DomUtil.create('img', 'avatar-img', tdImg);
		imgAuthor.setAttribute('alt', this.sectionProperties.data.author);
		var viewId = this.map.getViewId(this.sectionProperties.data.author);
		if (this.map['wopi'] && this.map['wopi'].CommentAvatarUrl)
			imgAuthor.setAttribute('src', this.map['wopi'].CommentAvatarUrl);
		else
			app.LOUtil.setUserImage(imgAuthor, this.map, viewId);
		imgAuthor.setAttribute('width', this.sectionProperties.imgSize[0]);
		imgAuthor.setAttribute('height', this.sectionProperties.imgSize[1]);

		if (app.map._docLayer._docType !== 'spreadsheet') {
			this.sectionProperties.collapsedInfoNode = window.L.DomUtil.create('div', 'cool-annotation-info-collapsed', tdImg);
			this.sectionProperties.collapsedInfoNode.style.display = 'none';
		}

		this.sectionProperties.authorAvatarImg = imgAuthor;
		this.sectionProperties.authorAvatartdImg = tdImg;
		this.sectionProperties.contentAuthor = window.L.DomUtil.create('div', 'cool-annotation-content-author', tdAuthor);
	}

	private createMenu (): void {
		var tdMenu = window.L.DomUtil.create('td', 'cool-annotation-menubar', this.sectionProperties.authorRow);
		const edit = window.L.DomUtil.create('div', 'cool-annotation-menu-edit', tdMenu);
		edit.id = 'comment-annotation-menu-edit-' + this.sectionProperties.data.id;
		edit.tabIndex = 0;
		edit.onclick = this.onEditComment.bind(this);
		edit.onkeypress = this.editOnKeyPress.bind(this);
		edit.dataset.title = Comment.editCommentLabel;
		edit.setAttribute('aria-label', Comment.editCommentLabel);

		this.sectionProperties.menu = window.L.DomUtil.create('div', this.sectionProperties.data.trackchange ? 'cool-annotation-menu-redline' : 'cool-annotation-menu', tdMenu);
		this.sectionProperties.menu.id = 'comment-annotation-menu-' + this.sectionProperties.data.id;
		this.sectionProperties.menu.tabIndex = 0;
		this.sectionProperties.menu.onclick = this.menuOnMouseClick.bind(this);
		this.sectionProperties.menu.onkeypress = this.menuOnKeyPress.bind(this);
		this.sectionProperties.menu.dataset.title = Comment.openMenuLabel;
		this.sectionProperties.menu.setAttribute('aria-label', Comment.openMenuLabel);
	}

	public setContainerPos(forceUpdate: boolean, canvasContainerBounds?: DOMRect, left?: number, top?: number): void {
		if ((<any>window).mode.isSmallScreenDevice()) {
			return;
		}

		if (canvasContainerBounds === undefined) {
			canvasContainerBounds = this.canvasContainerBounds;
		}

		if (left === undefined) {
			left = this.containerPosX;
		}

		if (top === undefined) {
			top = this.containerPosY;
		}

		if (this.containerPosX === left
				&& this.containerPosY === top
				&& this.canvasContainerBounds.left === canvasContainerBounds.left
				&& this.canvasContainerBounds.right === canvasContainerBounds.right
				&& this.canvasContainerBounds.top === canvasContainerBounds.top
				&& this.canvasContainerBounds.bottom === canvasContainerBounds.bottom
				&& !forceUpdate
		) {
			return;
		}

		this.containerPosX = left;
		this.containerPosY = top;
		this.canvasContainerBounds = canvasContainerBounds;

		left += canvasContainerBounds.left;
		top += canvasContainerBounds.top;

		if (this.isSelected() || this.isEdit()) {
			if (left < canvasContainerBounds.left) {
				left = canvasContainerBounds.left;
			}

			if (top < canvasContainerBounds.top) {
				top = canvasContainerBounds.top;
			}

			const width = this.getCommentWidth() / app.dpiScale;
			if (left + width > canvasContainerBounds.right) {
				left = canvasContainerBounds.right - width;
			}

			const height = this.getCommentHeight();
			if (top + height > canvasContainerBounds.bottom) {
				top = canvasContainerBounds.bottom - height;
			}
		}

		if (this.isSelected()) {
			this.sectionProperties.container.style.zIndex = 14;
		} else if (this.isEdit()) {
			this.sectionProperties.container.style.zIndex = 13;
		} else {
			this.sectionProperties.container.style.zIndex = ''; // Default for .cool-annotation is 12
		}

		this.sectionProperties.container.style.left = Math.round(left) + 'px';
		this.sectionProperties.container.style.top = Math.round(top) + 'px';
	}

	private createReplyHint (commentType: HTMLElement): void {
		this.sectionProperties.replyHint = window.L.DomUtil.create('p', '', commentType);
		var small = document.createElement('small');
		small.classList.add('cool-font');
		small.innerText = _('Press Ctrl + Enter to post');
		this.sectionProperties.replyHint.appendChild(small);
	}

	private createChildLinesNode (): void {
		this.sectionProperties.childLinesNode = window.L.DomUtil.create('div', '', this.sectionProperties.container);
		this.sectionProperties.childLinesNode.id = 'annotation-child-lines-' + this.sectionProperties.data.id;
		this.sectionProperties.childLinesNode.style.width = this.sectionProperties.childCommentOffset*(this.getChildLevel() + 1) + 'px';
	}

	public getContainerPosX(): number {
		return this.containerPosX;
	}

	public getContainerPosY(): number {
		return this.containerPosY;
	}

	public updateChildLines (): void {
		if (!this.isContainerVisible())
			return;
		this.sectionProperties.wrapper.style.marginLeft =  this.sectionProperties.childCommentOffset*this.getChildLevel() + 'px';
		this.sectionProperties.childLinesNode.style.width = this.sectionProperties.childCommentOffset*(this.getChildLevel() + 1) + 'px';

		const childPositions = [];
		for (let i = 0; i < this.sectionProperties.children.length; i++) {
			if (this.sectionProperties.children[i].isContainerVisible())
				childPositions.push({ id: this.sectionProperties.children[i].sectionProperties.data.id,
					posY: this.sectionProperties.children[i].getContainerPosY()
				});
		}
		childPositions.sort((a, b) => { return a.posY - b.posY; });
		let lastPosY = this.getContainerPosY() + this.getCommentHeight(false);
		let i = 0;
		for (; i < childPositions.length; i++) {
			if (this.sectionProperties.childLines[i] === undefined) {
				this.sectionProperties.childLines[i] = window.L.DomUtil.create('div', 'cool-annotation-child-line', this.sectionProperties.childLinesNode);
				this.sectionProperties.childLines[i].id = 'annotation-child-line-' + this.sectionProperties.data.id + '-' + i;
				this.sectionProperties.childLines[i].style.width = this.sectionProperties.childCommentOffset/2 + 'px';
			}
			this.sectionProperties.childLines[i].style.marginLeft =  (this.sectionProperties.childCommentOffset*this.getChildLevel() + 4) + 'px';
			this.sectionProperties.childLines[i].style.height = (childPositions[i].posY + 24 - lastPosY) + 'px';
			lastPosY = childPositions[i].posY + 24;
		}
		if (i < this.sectionProperties.childLines.length) {
			for (let j = i; j < this.sectionProperties.childLines.length; j++) {
				this.sectionProperties.childLinesNode.removeChild(this.sectionProperties.childLines[i]);
				this.sectionProperties.childLines.splice(i);
			}
		}

	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public setData (data: any): void {
		this.sectionProperties.data = data;
		const rectangles = this.sectionProperties.data.rectangles;

		if (rectangles) {
			// Convert the rectangles into simple rectangles as soon as possible.
			for (let i = 0; i < rectangles.length; i++) {
				rectangles[i] = new cool.SimpleRectangle(rectangles[i][0], rectangles[i][1], rectangles[i][2], rectangles[i][3]);
			}
		}
	}

	private createTrackChangeButtons (): void {
		var tdAccept = window.L.DomUtil.create('td', 'cool-annotation-menubar', this.sectionProperties.authorRow);
		var acceptButton = this.sectionProperties.acceptButton = window.L.DomUtil.create('button', 'cool-redline-accept-button', tdAccept);

		var tdReject = window.L.DomUtil.create('td', 'cool-annotation-menubar', this.sectionProperties.authorRow);
		var rejectButton = this.sectionProperties.rejectButton = window.L.DomUtil.create('button', 'cool-redline-reject-button', tdReject);

		acceptButton.dataset.title = _('Accept change');
		acceptButton.setAttribute('aria-label', _('Accept change'));

		window.L.DomEvent.on(acceptButton, 'click', function() {
			this.map.fire('RedlineAccept', {id: this.sectionProperties.data.id});
		}, this);

		rejectButton.dataset.title = _('Reject change');
		rejectButton.setAttribute('aria-label', _('Reject change'));

		window.L.DomEvent.on(rejectButton, 'click', function() {
			this.map.fire('RedlineReject', {id: this.sectionProperties.data.id});
		}, this);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private createButton (container: any, id: any, cssClass: string, value: any, handler: any): HTMLButtonElement {
		var button = window.L.DomUtil.create('input', cssClass, container);
		button.id = id;
		button.type = 'button';
		button.value = value;
		window.L.DomEvent.on(button, 'mousedown', window.L.DomEvent.preventDefault);
		window.L.DomEvent.on(button, 'click', handler, this);
		return button;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public parentOf (comment: any): boolean {
		return this.sectionProperties.data.id === comment.sectionProperties.data.parent;
	}

	public updateResolvedField (state: string): void {
		this.sectionProperties.resolvedTextElement.innerText = state === 'true' ? _('Resolved') : '';
	}

	public updateRemovedField (): void {
		var isDeleted = this.sectionProperties.data.layoutStatus === CommentLayoutStatus.DELETED;
		if (isDeleted && !this.sectionProperties.removedTextElement) {
			var commentFooter = this.sectionProperties.contentDate.parentNode;
			this.sectionProperties.removedTextElement = window.L.DomUtil.create('div', 'cool-annotation-content-removed', commentFooter);
		}
		if (this.sectionProperties.removedTextElement) {
			this.sectionProperties.removedTextElement.innerText = isDeleted ? _('Removed') : '';
		}
		if (isDeleted) {
			this.sectionProperties.resolvedTextElement.innerText = '';
		} else {
			this.updateResolvedField(this.sectionProperties.data.resolved);
		}
	}

	private isNewPara(): boolean {
		const selection = window.getSelection();
		if (!selection.rangeCount) return;

		const range = selection.getRangeAt(0);
		const cursorPosition = range.startOffset;
		const node = range.startContainer;

		const beforeCursor = node.textContent.slice(0, cursorPosition);
		return /^\s*$/.test(beforeCursor.slice(0, -1));
	}

	private textAreaInput(ev: any): void {
		if (ev && app.map._docLayer._docType === 'text') {
			// special handling for mentions
			this.map?.mention.handleMentionInput(ev, this.isNewPara());
		}
	}

	private handleKeyDownForPopup (ev: any, id: string): void {
		var popup = this.map._textInput._handleKeyDownForPopup(ev, id);
		// Block Esc from propogating if it closes the comment mention Popup
		if (popup && id === 'mentionPopup' && ev.key === 'Escape') {
			ev.preventDefault();
			ev.stopPropagation();
		}
	}

	private textAreaKeyDown (ev: any): void {
		if (window.KeyboardShortcuts.processEvent(app.UI.language.fromURL, ev)) {
			return;
		}

		if (ev && ev.ctrlKey && ev.key === "Enter") {
			this.map.mention?.closeMentionPopup(false);

			if (this.sectionProperties.nodeReplyText.id == ev.srcElement.id) {
				this.handleReplyCommentButton(ev);
			} else {
				this.handleSaveCommentButton(ev);
			}
			return;
		}

		this.handleKeyDownForPopup(ev, 'mentionPopup');
	}

	private onFocus() {
		this.resetControl(this.sectionProperties.saveButton, _('Save'), 'annotation-button-autosaved');
	}

	private onFocusReply() {
		this.resetControl(this.sectionProperties.replyButton, _('Reply'), 'annotation-button-autosaved');
	}

	private updateContent (): void {
		if(this.sectionProperties.data.html)
			this.sectionProperties.contentText.innerHTML = app.LOUtil.sanitize(this.sectionProperties.data.html);
		else
			this.sectionProperties.contentText.innerText = this.sectionProperties.data.text ? this.sectionProperties.data.text: '';
		// Get the escaped HTML out and find for possible, useful links
		var linkedText = Autolinker.link(this.sectionProperties.contentText.outerHTML);
		this.sectionProperties.contentText.innerHTML = app.LOUtil.sanitize(linkedText);
		// Original unlinked text
		this.sectionProperties.contentText.origText = this.sectionProperties.data.text ? this.sectionProperties.data.text: '';
		this.sectionProperties.contentText.origHTML = this.sectionProperties.data.html ? this.sectionProperties.data.html: '';
		this.sectionProperties.nodeModifyText.innerText = this.sectionProperties.data.text ? this.sectionProperties.data.text: '';
		if (this.sectionProperties.data.html) {
			this.sectionProperties.nodeModifyText.innerHTML = app.LOUtil.sanitize(this.sectionProperties.data.html);
		}
		this.sectionProperties.contentAuthor.innerText = this.sectionProperties.data.author;

		this.updateResolvedField(this.sectionProperties.data.resolved);
		this.updateRemovedField();
		if (this.map['wopi'] && this.map['wopi'].CommentAvatarUrl) {
			this.sectionProperties.authorAvatarImg.setAttribute('src', this.map['wopi'].CommentAvatarUrl);
		}
		else if (this.sectionProperties.data.avatar) {
			this.sectionProperties.authorAvatarImg.setAttribute('src', this.sectionProperties.data.avatar);
		}
		else {
			$(this.sectionProperties.authorAvatarImg).css('padding', '1.8px');
		}
		var user = this.map.getViewId(this.sectionProperties.data.author);
		if (user >= 0) {
			var color = app.LOUtil.rgbToHex(this.map.getViewColor(user));
			this.sectionProperties.authorAvatartdImg.style.borderColor = color;
		}

		// dateTime is already in UTC, so we will not append Z that will create issues while converting date
		var d = new Date(this.sectionProperties.data.dateTime.replace(/,.*/, ''));
		var dateOptions: any = { weekday: 'short', year: 'numeric', month: 'short', day: 'numeric', hour: 'numeric', minute: 'numeric'};
		this.sectionProperties.contentDate.innerText = isNaN(d.getTime()) ? this.sectionProperties.data.dateTime: d.toLocaleDateString((<any>String).locale, dateOptions);

		if (this.sectionProperties.data.trackchange) {
			this.sectionProperties.captionText.innerText = this.sectionProperties.data.description;
		}
	}

	private updateLayout (): void {
		var style = this.sectionProperties.wrapper.style;
		style.width = '';
		style.whiteSpace = 'nowrap';

		style.whiteSpace = '';
	}

	private setPositionAndSize (): void {
		var rectangles = this.sectionProperties.data.rectanglesOriginal;
		if (rectangles && app.map._docLayer._docType === 'text') {
			var xMin: number = Infinity, yMin: number = Infinity, xMax: number = 0, yMax: number = 0;
			for (var i = 0; i < rectangles.length; i++) {
				if (rectangles[i][0] < xMin)
					xMin = rectangles[i][0];

				if (rectangles[i][1] < yMin)
					yMin = rectangles[i][1];

				if (rectangles[i][0] + rectangles[i][2] > xMax)
					xMax = rectangles[i][0] + rectangles[i][2];

				if (rectangles[i][1] + rectangles[i][3] > yMax)
					yMax = rectangles[i][1] + rectangles[i][3];
			}
			// Rectangles are in twips. Convert them to core pixels.
			xMin = Math.round(xMin * app.twipsToPixels);
			yMin = Math.round(yMin * app.twipsToPixels);
			xMax = Math.round(xMax * app.twipsToPixels);
			yMax = Math.round(yMax * app.twipsToPixels);

			this.setPosition(xMin, yMin); // This function is added by section container.
			this.size = [xMax - xMin, yMax - yMin];
			if (this.size[0] < 5)
				this.size[0] = 5;
		}
		else if (this.sectionProperties.data.cellRange && app.map._docLayer._docType === 'spreadsheet') {
			this.size = this.calcOptimumSizeForCalc();
			var cellPos = app.map._docLayer._cellRangeToTwipRect(this.sectionProperties.data.cellRange).toRectangle();
			let startX = cellPos[0];
			if (this.isCalcRTL()) { // Mirroring is done in setPosition
				const sizeX = cellPos[2];
				startX += sizeX;  // but adjust for width of the cell.
			}
			this.setShowSection(true);
			var position: Array<number> = [Math.round((cellPos[0] + cellPos[2]) * app.twipsToPixels - this.size[0]), Math.round(cellPos[1] * app.twipsToPixels)];

			this.setPosition(position[0], position[1]);
		}
		else if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') {
			this.size = [Math.round(this.sectionProperties.imgSize[0] * app.dpiScale), Math.round(this.sectionProperties.imgSize[1] * app.dpiScale)];
			this.setPosition(Math.round(this.sectionProperties.data.rectangle[0] * app.twipsToPixels), Math.round(this.sectionProperties.data.rectangle[1] * app.twipsToPixels));
		}
	}

	public removeHighlight (): void {
		if (app.map._docLayer._docType === 'text') {
			this.sectionProperties.usedTextColor = this.sectionProperties.data.color;

			this.sectionProperties.isHighlighted = false;
		}
		else if (app.map._docLayer._docType === 'spreadsheet') {
			this.backgroundColor = null;
			this.backgroundOpacity = 1;
		}
	}

	public highlight (): void {
		if (app.map._docLayer._docType === 'text') {
			this.sectionProperties.usedTextColor = this.sectionProperties.highlightedTextColor;

			var x: number = Math.round(this.position[0] / app.dpiScale);
			var y: number = Math.round(this.position[1] / app.dpiScale);
			(this.containerObject.getSectionWithName(app.CSections.Scroll.name) as any as cool.ScrollSection).onScrollTo({x: x, y: y});
		}
		else if (app.map._docLayer._docType === 'spreadsheet') {
			this.backgroundColor = '#777777'; //background: rgba(119, 119, 119, 0.25);
			this.backgroundOpacity = 0.25;

			var x: number = Math.round(this.position[0] / app.dpiScale);
			var y: number = Math.round(this.position[1] / app.dpiScale);
			(this.containerObject.getSectionWithName(app.CSections.Scroll.name) as any as cool.ScrollSection).onScrollTo({x: x, y: y});
		}
		else if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') {
			var x: number = Math.round(this.position[0] / app.dpiScale);
			var y: number = Math.round(this.position[1] / app.dpiScale);
			(this.containerObject.getSectionWithName(app.CSections.Scroll.name) as any as cool.ScrollSection).onScrollTo({x: x, y: y});
		}

		this.containerObject.requestReDraw();
		this.sectionProperties.isHighlighted = true;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private static doesRectIntersectView(pos: number[], size: number[], viewContext: any): boolean {
		var paneBoundsList = <any[]>viewContext.paneBoundsList;
		var endPos = [pos[0] + size[0], pos[1] + size[1]];
		for (var i = 0; i < paneBoundsList.length; ++i) {
			var paneBounds = paneBoundsList[i];
			var rectInvisible = (endPos[0] < paneBounds.min.x || endPos[1] < paneBounds.min.y ||
				pos[0] > paneBounds.max.x || pos[1] > paneBounds.max.y);
			if (!rectInvisible)
				return true;
		}
		return false;
	}

	// This is for svg elements that will be bound to document-container.
	// This also returns whether any rectangle has an intersection with the visible area/panes.
	// This function calculates the core pixel coordinates then converts them into view coordinates.
	private convertRectanglesToViewCoordinates () : boolean {
		var rectangles = this.sectionProperties.data.rectangles;
		var viewContext = this.map.getTileSectionMgr()._paintContext();
		var intersectsVisibleArea = false;

		if (rectangles) {
			for (let i = 0; i < rectangles.length; i++) {
				if (app.activeDocument.activeLayout.viewedRectangle.intersectsRectangle(rectangles[i].toArray())) {
					intersectsVisibleArea = true;
					break;
				}
			}
		} else if (this.sectionProperties.data.trackchange && this.sectionProperties.data.anchorPos) {
			// For redline comments there are no 'rectangles' or 'rectangleOriginal' properties in sectionProperties.data
			// So use the comment rectangle stored in anchorPos (in display? twips).
			intersectsVisibleArea = Comment.doesRectIntersectView(this.getPosition(), this.getSize(), viewContext);
		}

		return intersectsVisibleArea;
	}

	public getPosition (): number[] {
		// For redline comments there are no 'rectangles' or 'rectangleOriginal' properties in sectionProperties.data
		// So use the comment rectangle stored in anchorPos (in display? twips).
		if (this.sectionProperties.data.trackchange && this.sectionProperties.data.anchorPos) {
			var anchorPos = this.sectionProperties.data.anchorPos;
			return [
				Math.round(anchorPos[0] * app.twipsToPixels),
				Math.round(anchorPos[1] * app.twipsToPixels)
			];
		} else {
			return this.position;
		}
	}

	public getSize(): number[] {
		// For redline comments there are no 'rectangles' or 'rectangleOriginal' properties in sectionProperties.data
		// So use the comment rectangle stored in anchorPos (in display? twips).
		if (this.sectionProperties.data.trackchange && this.sectionProperties.data.anchorPos) {
			var anchorPos = this.sectionProperties.data.anchorPos;
			return [
				Math.round(anchorPos[2] * app.twipsToPixels),
				Math.round(anchorPos[3] * app.twipsToPixels)
			];
		} else {
			return this.size;
		}
	}

	private updatePosition (): void {
		this.setPositionAndSize();
		if (app.map._docLayer._docType === 'spreadsheet')
			this.positionCalcComment();
		else if (app.map._docLayer._docType === "presentation" || app.map._docLayer._docType === "drawing") {
			if (this.sectionProperties.commentMarkerSubSection !== null) {
				this.sectionProperties.commentMarkerSubSection.sectionProperties.data = this.sectionProperties.data;
				this.sectionProperties.commentMarkerSubSection.setPosition(
					this.sectionProperties.data.anchorPos[0] * app.twipsToPixels,
					this.sectionProperties.data.anchorPos[1] * app.twipsToPixels
				);
			}
		}
	}

	private createMarkerSubSection() {
		if (this.sectionProperties.data.rectangle === null)
			return;

		const showMarker = app.impress.partList[app.map._docLayer._selectedPart].hash === this.sectionProperties.data.parthash ||
							app.file.fileBasedView;

		this.sectionProperties.commentMarkerSubSection = new CommentMarkerSubSection(
			this.name + this.sectionProperties.data.id + String(Math.random()), // Section name - only as a placeholder.
			28, 28, // Width and height.
			new SimplePoint(this.sectionProperties.data.anchorPos[0], this.sectionProperties.data.anchorPos[1]), // Document position.
			'annotation-marker', // Extra class.
			showMarker, // Show section.
			this, // Parent section.
			this.sectionProperties.data
		);

		app.sectionContainer.addSection(this.sectionProperties.commentMarkerSubSection);
	}

	public isContainerVisible (): boolean {
		return this.sectionProperties.container.style &&
			this.sectionProperties.container.style.display !== 'none' &&
			(
				this.sectionProperties.container.style.visibility === 'visible' ||
				this.sectionProperties.container.style.visibility === ''
			);
	}

	public update (): void {
		this.updateContent();
		this.updateLayout();
		this.updatePosition();
	}

	private showMarker (): void {
		if (this.sectionProperties.commentMarkerSubSection != null) {
			this.sectionProperties.commentMarkerSubSection.showSection = true;
			this.sectionProperties.commentMarkerSubSection.onSectionShowStatusChange();
		}
	}

	private hideMarker (): void {
		if (this.sectionProperties.commentMarkerSubSection != null) {
			this.sectionProperties.commentMarkerSubSection.showSection = false;
			this.sectionProperties.commentMarkerSubSection.onSectionShowStatusChange();
		}
	}

	private showWriter() {
		if (!this.isCollapsed || this.isSelected()) {
			this.sectionProperties.container.style.visibility = '';
			this.sectionProperties.container.style.display = '';
		}
		if (this.sectionProperties.data.resolved !== 'true' || this.sectionProperties.commentListSection.sectionProperties.showResolved) {
			window.L.DomUtil.addClass(this.sectionProperties.container, 'cool-annotation-collapsed-show');
			this.sectionProperties.showSelectedCoordinate = true;
		}
		this.sectionProperties.contentNode.style.display = '';
		this.sectionProperties.nodeModify.style.display = 'none';
		this.sectionProperties.nodeReply.style.display = 'none';
		this.sectionProperties.collapsedInfoNode.style.visibility = '';
		this.cachedIsEdit = false;
		this.setContainerPos(true);
	}

	private showCalc() {
		this.sectionProperties.container.style.display = '';
		this.sectionProperties.contentNode.style.display = '';
		this.sectionProperties.nodeModify.style.display = 'none';
		this.sectionProperties.nodeReply.style.display = 'none';
		this.cachedIsEdit = false;

		this.positionCalcComment();
		if (!(<any>window).mode.isSmallScreenDevice()) {
			this.sectionProperties.commentListSection.select(this);
		}
		this.sectionProperties.container.style.visibility = '';
	}

	private getCommentWidth() {
		// note: getComputedStyle can be an exceptional bottle-neck with many comments
		return parseFloat(getComputedStyle(this.sectionProperties.container).width) * app.dpiScale;
	}

	public positionCalcComment(): void {
		if (!(<any>window).mode.isSmallScreenDevice()) {
			const startX = this.isCalcRTL() ? this.myTopLeft[0] - this.getCommentWidth() : this.myTopLeft[0] + this.calcOptimumSizeForCalc()[0] - 3;

			var pos: Array<number> = [Math.round(startX / app.dpiScale), Math.round(this.myTopLeft[1] / app.dpiScale)];
			this.sectionProperties.container.style.transform = 'translate3d(' + pos[0] + 'px, ' + pos[1] + 'px, 0px)';
		}
	}

	private showImpressDraw() {
		if (this.isInsideActivePart()) {
			this.sectionProperties.container.style.display = '';
			this.sectionProperties.nodeModify.style.display = 'none';
			this.sectionProperties.nodeReply.style.display = 'none';
			this.sectionProperties.contentNode.style.display = '';
			this.cachedIsEdit = false;
			this.setContainerPos(true);
			if (this.isSelected() || !this.isCollapsed) {
				this.sectionProperties.container.style.visibility = '';
			}
			else {
				this.sectionProperties.container.style.visibility = 'hidden';
			}
			window.L.DomUtil.addClass(this.sectionProperties.container, 'cool-annotation-collapsed-show');
		}
	}

	public setLayoutClass(): void {
		this.sectionProperties.container.classList.remove('tracked-deleted-comment-show');
		this.sectionProperties.container.classList.remove('tracked-deleted-comment-hide');

		var showTrackedChanges: boolean = this.map['stateChangeHandler'].getItemValue('.uno:ShowTrackedChanges') === 'true';
		var layoutClass: string = showTrackedChanges ? 'tracked-deleted-comment-show' : 'tracked-deleted-comment-hide';
		if (this.sectionProperties.data.layoutStatus === CommentLayoutStatus.DELETED) {
			this.sectionProperties.container.classList.add(layoutClass);
		}
		this.updateRemovedField();
	}

	public show(): void {
		this.doPendingInitializationInView(true /* force */);

		if (this.hidden === false && !this.isEdit()) return;

		this.showMarker();

		// On mobile, container shouldn't be 'document-container', but it is 'document-container' on initialization. So we hide the comment until comment wizard is opened.
		if ((<any>window).mode.isSmallScreenDevice() && this.sectionProperties.container.parentElement === document.getElementById('document-container'))
			this.sectionProperties.container.style.visibility = 'hidden';

		if (cool.CommentSection.commentWasAutoAdded)
			return;

		// We don't cache the hidden state for spreadsheets. Only one comment can be
		// visible and they're hidden when scrolling, so it's easier this way.
		if (app.map._docLayer._docType === 'text') {
			this.showWriter();
			this.hidden = false;
		} else if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') {
			this.showImpressDraw();
			this.hidden = false;
		} else if (app.map._docLayer._docType === 'spreadsheet')
			this.showCalc();

		this.setLayoutClass();
	}

	private hideWriter() {
		this.sectionProperties.container.style.visibility = 'hidden';
		this.sectionProperties.nodeModify.style.display = 'none';
		this.sectionProperties.nodeReply.style.display = 'none';
		this.sectionProperties.showSelectedCoordinate = false;
		window.L.DomUtil.removeClass(this.sectionProperties.container, 'cool-annotation-collapsed-show');
		this.cachedIsEdit = false;
		this.setContainerPos(true);
		this.hidden = true;
	}

	private hideCalc() {
		this.sectionProperties.container.style.visibility = 'hidden';
		this.sectionProperties.nodeModify.style.display = 'none';
		this.sectionProperties.nodeReply.style.display = 'none';
		this.cachedIsEdit = false;

		if (this.sectionProperties.commentListSection.sectionProperties.selectedComment === this)
			this.sectionProperties.commentListSection.sectionProperties.selectedComment = null;
	}

	private hideImpressDraw() {
		if (!this.isInsideActivePart()) {
			this.sectionProperties.container.style.display = 'none';
			this.hideMarker();
		}
		else {
			this.sectionProperties.container.style.display = '';
			if (this.isCollapsed)
				this.sectionProperties.container.style.visibility = 'hidden';

			this.sectionProperties.nodeModify.style.display = 'none';
			this.sectionProperties.nodeReply.style.display = 'none';
			this.cachedIsEdit = false;
			this.setContainerPos(true);
		}
		window.L.DomUtil.removeClass(this.sectionProperties.container, 'cool-annotation-collapsed-show');
		this.hidden = true;
	}

	// check if this is "our" autosaved comment
	// core is not aware it's autosaved one so use this simplified detection based on content
	public isAutoSaved (): boolean {
		var autoSavedComment = cool.CommentSection.autoSavedComment;
		if (!autoSavedComment)
			return false;

		var authorMatch = this.sectionProperties.data.author === this.map.getViewName(app.map._docLayer._viewId);
		return authorMatch;
	}

	public hide (): void {
		if (this.hidden === true || this.isEdit()) {
			return;
		}

		if (this.sectionProperties.data.id === 'new') {
			this.sectionProperties.commentListSection.removeItem(this.sectionProperties.data.id);
			return;
		}

		if (app.map._docLayer._docType === 'text')
			this.hideWriter();
		else if (app.map._docLayer._docType === 'spreadsheet')
			this.hideCalc();
		else if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing')
			this.hideImpressDraw();
	}

	private isInsideActivePart() {
		// Impress and Draw only.
		return this.sectionProperties.partIndex === app.map._docLayer._selectedPart;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private menuOnMouseClick (e: any): void {
		this.openContextMenu();
		window.L.DomEvent.stopPropagation(e);
	}

	private onEditComment (e: any): void {
		this.sectionProperties.commentListSection.modify(this);
		window.L.DomEvent.stopPropagation(e);
	}

	private editOnKeyPress (e: any): void {
		if (e.code === 'Space' || e.code === 'Enter')
		{
			this.onEditComment(e);
			window.L.DomEvent.stopPropagation(e);
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private menuOnKeyPress (e: any): void {
		if (e.code === 'Space' || e.code === 'Enter')
			this.openContextMenu();
		window.L.DomEvent.stopPropagation(e);
	}

	private openContextMenu (): void {
		const listSection = this.sectionProperties.commentListSection;
		const data = this.sectionProperties.data;
		const docLayer = app.map._docLayer;
		const entries: Array<any> = [];
		let pos = 0;

		if (data.trackchange) {
			entries.push({ text: _('Comment'), type: 'action', id: 'modify', pos: String(pos++) });
		} else {
			const blockChangeFromDifferentAuthor = this.map.isReadOnlyMode()
				&& docLayer._docType === 'text'
				&& this.map.getViewName(docLayer._viewId) !== data.author;

			if (!blockChangeFromDifferentAuthor)
				entries.push({ text: _('Modify'), type: 'action', id: 'modify', pos: String(pos++) });

			if (docLayer._docType === 'text')
				entries.push({ text: _('Reply'), type: 'action', id: 'reply', pos: String(pos++) });

			if (!blockChangeFromDifferentAuthor)
				entries.push({ text: _('Remove'), type: 'action', id: 'remove', pos: String(pos++) });

			if (docLayer._docType === 'text' && this.isRootComment() && !blockChangeFromDifferentAuthor)
				entries.push({ text: _('Remove Thread'), type: 'action', id: 'removeThread', pos: String(pos++) });

			if (docLayer._docType === 'text'
				|| (docLayer._docType === 'spreadsheet' && data.threaded))
				entries.push({
					text: data.resolved === 'false' ? _('Resolve') : _('Unresolve'),
					type: 'action', id: 'resolve', pos: String(pos++),
				});

			if (docLayer._docType === 'text' && this.isRootComment())
				entries.push({
					text: listSection.isThreadResolved(this) ? _('Unresolve Thread') : _('Resolve Thread'),
					type: 'action', id: 'resolveThread', pos: String(pos++),
				});

			if (docLayer._docType === 'text' && !this.isRootComment() && !blockChangeFromDifferentAuthor)
				entries.push({ text: _('Promote to top comment'), type: 'action', id: 'promote', pos: String(pos++) });

			if (docLayer._docType === 'text' && !window.mode.isSmallScreenDevice()) {
				const isShownBig = listSection.isShownBig(this);
				entries.push({
					text: isShownBig ? _('Show on the side') : _('Open in full view'),
					type: 'action', id: 'showBigger', pos: String(pos++),
				});
			}

			if ((docLayer._docType === 'text' || docLayer._docType === 'spreadsheet')
				&& !window.mode.isSmallScreenDevice())
				entries.push({ text: _('Show in navigator'), type: 'action', id: 'showInNavigator', pos: String(pos++) });
		}

		if (entries.length === 0)
			return;

		const menuEl = this.sectionProperties.menu;
		menuEl._onDropDown = function (open: boolean) {
			this.sectionProperties.contextMenu = open;
		}.bind(this);

		if (window.mode.isSmallScreenDevice()) {
			const menu: any[] = [];
			entries.forEach((entry: any) => {
				menu.push({
					name: entry.text,
					command: entry.text,
					callback: () => {
						this.handleMenuAction(entry.id);
						if (entry.id !== 'reply' && entry.id !== 'modify')
							app.map.fire('mobilewizardback');
					},
				});
			});
			const menuData =
				window.L.Control.JSDialogBuilder.getMenuStructureForMobileWizard(
					menu,
					true,
					'',
				);
			app.map.fire('mobilewizard', { data: menuData });
			return;
		}

		const callback = function (_objectType: string, eventType: string, _object: any, _data: any, entry: any) {
			if (eventType !== 'selected')
				return false;
			this.handleMenuAction(entry?.id);
			JSDialog.CloseAllDropdowns();
			return true;
		}.bind(this);

		JSDialog.OpenDropdown(
			'comment-menu-' + data.id,
			menuEl,
			entries,
			callback,
			'',
			false,
			false,
			true,
		);
	}

	private handleMenuAction (id: string): void {
		const listSection = this.sectionProperties.commentListSection;
		switch (id) {
		case 'modify': listSection.modify(this); break;
		case 'reply': listSection.reply(this); break;
		case 'remove': listSection.remove(this.sectionProperties.data.id); break;
		case 'removeThread': listSection.removeThread(this.sectionProperties.data.id); break;
		case 'resolve': listSection.resolve(this); break;
		case 'resolveThread': listSection.resolveThread(this); break;
		case 'promote': listSection.promote(this); break;
		case 'showBigger': listSection.toggleShowBigger(this); break;
		case 'showInNavigator': listSection.showInNavigator(this); break;
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private onMouseClick (e: any): void {
		if (((<any>window).mode.isSmallScreenDevice() || (<any>window).mode.isTablet())
			&& this.map.getDocType() == 'spreadsheet'
			&& !this.map.uiManager.mobileWizard.isOpen()) {
			this.hide();
		}
		window.L.DomEvent.stopPropagation(e);
		this.sectionProperties.commentListSection.click(this);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private onEscKey (e: any): void {
		if ((<any>window).mode.isDesktop()) {
			// When a comment is being edited and focus is in comment textbox,
			// Esc should not close the comment being edited, but should just mark it with an attention.
			if (e.keyCode === 27) {
				const editingComment = Comment.isAnyEdit();
				if (editingComment) {
					this.sectionProperties.commentListSection.addCommentAttention(editingComment);
					return;
				}
			} else if (e.keyCode === 33 /*PageUp*/ || e.keyCode === 34 /*PageDown*/) {
				// work around for a chrome issue https://issues.chromium.org/issues/41417806
				window.L.DomEvent.preventDefault(e);
				var pos = e.keyCode === 33 ? 0 : e.target.textLength;
				var currentPos = e.target.selectionStart;
				if (e.shiftKey) {
					var [start, end] = currentPos <= pos ? [currentPos, pos] : [pos, currentPos];
					e.target.setSelectionRange(start, end, currentPos > pos ? 'backward' : 'forward');
				} else {
					e.target.setSelectionRange(pos, pos);
				}
			}
		}

	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public handleReplyCommentButton (e: any): void {
		cool.CommentSection.autoSavedComment = null;
		cool.CommentSection.commentWasAutoAdded = false;
		this.textAreaInput(null);
		this.onReplyClick(e);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onReplyClick (e: any): void {
		window.L.DomEvent.stopPropagation(e);
		if ((<any>window).mode.isSmallScreenDevice()) {
			this.sectionProperties.data.reply = this.sectionProperties.data.text;
			this.sectionProperties.commentListSection.saveReply(this);
		} else {
			this.removeLastBRTag(this.sectionProperties.nodeReplyText);
			this.sectionProperties.data.reply = this.sectionProperties.nodeReplyText.innerText;
			this.sectionProperties.data.html = this.sectionProperties.nodeReplyText.innerHTML;
			// Assigning an empty string to .innerHTML property in some browsers will convert it to 'null'
			// While in browsers like Chrome and Firefox, a null value is automatically converted to ''
			// Better to assign '' here instead of null to keep the behavior same for all
			this.sectionProperties.nodeReplyText.innerText = '';
			this.show();
			this.sectionProperties.commentListSection.saveReply(this);
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public handleCancelCommentButton (e: any): void {
		if (cool.CommentSection.commentWasAutoAdded) {
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).remove(this.sectionProperties.data.id);
		}

		if (cool.CommentSection.autoSavedComment) {
			this.sectionProperties.contentText.origText = this.sectionProperties.contentText.uneditedText;
			this.sectionProperties.contentText.uneditedText = null;
			this.sectionProperties.contentText.origHTML = this.sectionProperties.contentText.uneditedHTML;
			this.sectionProperties.contentText.uneditedHTML = null;
		}

		// These lines are repeated in onCancelClick,
		// it makes things simple by not adding so many condition for different apps and different situation
		// It is mandatory to change these values before handleSaveCommentButton is called
		// calling handleSaveCommentButton in onCancelClick causes problem because that is also called from many other events/function (i.e: onPartChange)
		if (this.sectionProperties.contentText.origHTML) {
			this.sectionProperties.nodeModifyText.innerHTML = app.LOUtil.sanitize(this.sectionProperties.contentText.origHTML);
		}
		else {
			this.sectionProperties.nodeModifyText.innerText = this.sectionProperties.contentText.origText;
		}
		this.sectionProperties.nodeReplyText.innerText = '';

		if (cool.CommentSection.autoSavedComment)
			this.handleSaveCommentButton(e);

		this.onCancelClick(e);
		if (app.map._docLayer._docType === 'spreadsheet')
			this.hideCalc();
		cool.CommentSection.commentWasAutoAdded = false;
		cool.CommentSection.autoSavedComment = null;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onCancelClick (e: any): void {
		if (e)
			window.L.DomEvent.stopPropagation(e);
		if (this.sectionProperties.contentText.origHTML) {
			this.sectionProperties.nodeModifyText.innerHTML = app.LOUtil.sanitize(this.sectionProperties.contentText.origHTML);
		}
		else {
			this.sectionProperties.nodeModifyText.innerText = this.sectionProperties.contentText.origText;
		}
		this.sectionProperties.nodeReplyText.innerText = '';
		if (app.map._docLayer._docType !== 'spreadsheet')
			this.show();
		this.sectionProperties.commentListSection.cancel(this);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public handleSaveCommentButton (e: any): void {
		cool.CommentSection.autoSavedComment = null;
		cool.CommentSection.commentWasAutoAdded = false;
		this.sectionProperties.contentText.uneditedText = null;
		this.sectionProperties.contentText.uneditedHTML = null;
		this.textAreaInput(null);
		this.onSaveComment(e);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onSaveComment (e: any): void {
		this.sectionProperties.commentContainerRemoved = true;
		window.L.DomEvent.stopPropagation(e);
		this.removeLastBRTag(this.sectionProperties.nodeModifyText);
		this.sectionProperties.data.text = this.sectionProperties.nodeModifyText.innerText;
		this.sectionProperties.data.html = this.sectionProperties.nodeModifyText.innerHTML;
		this.updateContent();
		if (!cool.CommentSection.autoSavedComment)
			this.show();
		this.sectionProperties.commentListSection.save(this);
	}

	// for some reason firefox adds <br> at of the end of text in contenteditable div
	// there have been similar reports: https://bugzilla.mozilla.org/show_bug.cgi?id=1615852
	private removeLastBRTag(element: HTMLElement) {
		if (!window.L.Browser.gecko)
			return;
		const brElements = element.querySelectorAll('br');
		if (brElements.length > 0)
			brElements[brElements.length-1].remove();
	}

	private isNodeEmpty(): boolean {
		this.removeLastBRTag(this.sectionProperties.nodeModifyText);
		if (this.sectionProperties.nodeModifyText.innerText == "" &&
			this.sectionProperties.nodeModifyText.innerHTML == "")
			return true;
		return false;
	}

	private updateControl(
		button: HTMLButtonElement | null,
		label: string,
		className: string
	): void {
		if (button) {
			button.value = label;
			button.classList.add(className);
		}
	}

	private updateSaveControls() {
		this.updateControl(this.sectionProperties.saveButton, _('Saved'), 'annotation-button-autosaved');
		this.updateControl(this.sectionProperties.cancelButton, _('Delete'), 'annotation-button-delete');
	}

	private updateReplyControls() {
		this.updateControl(this.sectionProperties.replyButton, _('Saved'), 'annotation-button-autosaved');
		this.updateControl(this.sectionProperties.cancelReplyButton, _('Delete'), 'annotation-button-delete');
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onLostFocus (e: any): void {

		if (!this.isEdit() || this.sectionProperties.container.contains(e.relatedTarget))
			return;
		if (this.sectionProperties.nodeReply.contains(e.target)) {
			this.onLostFocusReply(e);
			return;
		}

		if (app.map._docLayer._docType === 'text' && this.map.mention?.isTypingMention()) {
			return;
		}
		if (!this.sectionProperties.commentContainerRemoved) {
			$(this.sectionProperties.container).removeClass('annotation-active reply-annotation-container modify-annotation-container');
			this.removeLastBRTag(this.sectionProperties.nodeModifyText);
			if (this.sectionProperties.contentText.origText !== this.sectionProperties.nodeModifyText.innerText ||
			    this.sectionProperties.contentText.origHTML !== this.sectionProperties.nodeModifyText.innerHTML) {
				if(!document.hasFocus())
					app.definitions.CommentSection.needFocus = this;
				if (!this.sectionProperties.contentText.uneditedHTML)
					this.sectionProperties.contentText.uneditedHTML = this.sectionProperties.contentText.origHTML;
				if (!this.sectionProperties.contentText.uneditedText)
					this.sectionProperties.contentText.uneditedText = this.sectionProperties.contentText.origText;
				cool.CommentSection.autoSavedComment = this;
				this.onSaveComment(e);
			}
			else if (this.containerObject.testing) {
				var insertButton = document.getElementById('menu-insertcomment');
				if (insertButton) {
					if (window.getComputedStyle(insertButton).display === 'none') {
						this.onCancelClick(e);
					}
				}
			}
			else if (app.map._docLayer._docType === 'text' && this.isNodeEmpty())
				this.onCancelClick(e);
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onLostFocusReply (e: any): void {
		if (app.map._docLayer._docType === 'text' && this.map.mention?.isTypingMention()) {
			return;
		}
		if (this.sectionProperties.nodeReplyText.innerText !== '') {
			if(!document.hasFocus())
				app.definitions.CommentSection.needFocus = this;
			if (!this.sectionProperties.contentText.uneditedHTML)
				this.sectionProperties.contentText.uneditedHTML = this.sectionProperties.contentText.origHTML;
			if (!this.sectionProperties.contentText.uneditedText)
				this.sectionProperties.contentText.uneditedText = this.sectionProperties.contentText.origText;
			cool.CommentSection.autoSavedComment = this;
			this.onReplyClick(e);
		}
		else {
			this.sectionProperties.nodeReply.style.display = 'none';
			if (!this.sectionProperties.nodeModify || this.sectionProperties.nodeModify.style.display === 'none') {
				this.cachedIsEdit = false;
				if (app.map._docLayer._docType !== 'spreadsheet') {
					this.setContainerPos(true);
				}
			}
		}
	}

	private resetControl(
		button: HTMLButtonElement | null,
		label: string,
		className: string
	): void {
		if (button) {
			button.value = label;
			button.classList.remove(className);
		}
	}

	private resetSaveControls(): void {
		this.resetControl(this.sectionProperties.saveButton, _('Save'), 'annotation-button-autosaved');
		this.resetControl(this.sectionProperties.cancelButton, _('Cancel'), 'annotation-button-delete');
	}

	private resetReplyControls(): void {
		this.resetControl(this.sectionProperties.replyButton, _('Reply'), 'annotation-button-autosaved');
		this.resetControl(this.sectionProperties.cancelReplyButton, _('Cancel'), 'annotation-button-delete');
	}

	public focus (): void {
		this.sectionProperties.container.classList.add('annotation-active');
		this.sectionProperties.nodeModifyText.focus({ focusVisible: true });
		this.sectionProperties.nodeReplyText.focus({ focusVisible: true });

		// set cursor at the last position on refocus after autosave
		if (this.isModifying() && this.sectionProperties.nodeModifyText.childNodes.length > 0) {
			var range = document.createRange()
			var sel = document.getSelection()
			range.setStartAfter(this.sectionProperties.nodeModifyText.lastChild)
			range.collapse(true)
			sel.removeAllRanges()
			sel.addRange(range)
		}

		this.resetSaveControls();
		this.resetReplyControls();
	}

	public focusLost (): void {
		this.updateSaveControls();
		this.updateReplyControls();
	}

	public reply (): Comment {
		this.sectionProperties.container.classList.add('reply-annotation-container');
		this.sectionProperties.container.style.visibility = '';
		this.sectionProperties.contentNode.style.display = '';
		this.sectionProperties.nodeModify.style.display = 'none';
		this.sectionProperties.nodeReply.style.display = '';
		this.cachedIsEdit = true;
		if (app.map._docLayer._docType !== 'spreadsheet') {
			this.setContainerPos(true);
		}
		return this;
	}

	public edit (): Comment {
		this.doPendingInitializationInView(true /* force */);
		this.sectionProperties.container.classList.add('modify-annotation-container');
		this.sectionProperties.nodeModify.style.display = '';
		this.sectionProperties.nodeReply.style.display = 'none';
		this.sectionProperties.container.style.visibility = '';
		this.sectionProperties.contentNode.style.display = 'none';
		this.cachedIsEdit = true;
		if (app.map._docLayer._docType !== 'spreadsheet') {
			this.setContainerPos(true);
		}
		return this;
	}

	public isEdit (): boolean {
		return this.cachedIsEdit;
	}

	public isModifying(): boolean {
		return !this.pendingInit && this.sectionProperties.nodeModify && this.sectionProperties.nodeModify.style.display !== 'none';
	}

	public static isAnyEdit (): Comment {
		var section = app.sectionContainer && app.sectionContainer instanceof CanvasSectionContainer ?
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name) : null;
		if (!section) {
			return null;
		}

		var commentList = section.sectionProperties.commentList;
		for (var i in commentList) {
			var modifyNode = commentList[i].sectionProperties.nodeModify;
			var replyNode = commentList[i].sectionProperties.nodeReply;
			if (!commentList[i].pendingInit &&
				((modifyNode && modifyNode.style.display !== 'none') ||
				(replyNode && replyNode.style.display !== 'none')))
					return commentList[i];
		}
		return null;
	}

	public static isAnyFocus(): boolean {
		const comment_: Comment = Comment.isAnyEdit();

		// We have a comment in edit mode. Is it focused?
		if (comment_ && (document.activeElement === comment_.sectionProperties.nodeModifyText || document.activeElement === comment_.sectionProperties.nodeReplyText))
			return true;

		return false;
	}

	public isDisplayed (): boolean {
		return (this.sectionProperties.container.style && this.sectionProperties.container.style.visibility === '');
	}

	public onResize (): void {
		this.updatePosition();
	}

	private isSelected(): boolean {
		return this.sectionProperties.commentListSection.sectionProperties.selectedComment === this;
	}

	/*
		point is the core pixel coordinate of the cursor.
		Not adjusted according to the view.
		For adjusting, we need to take document top left and documentAnchor top left into account.
		No need to do that for now.
	*/
	private checkIfCursorIsOnThisCommentWriter(newPosition: cool.SimpleRectangle) {
		if (this.sectionProperties.commentListSection.sectionProperties.show == false)
			return;

		const rectangles: cool.SimpleRectangle[] = this.sectionProperties.data.rectangles;

		for (var i: number = 0; i < rectangles.length; i++) {
			if (rectangles[i].containsPoint([newPosition.x1, newPosition.y1])) {
				if (!this.isSelected())
					this.sectionProperties.commentListSection.selectById(this.sectionProperties.data.id);

				this.stopPropagating();
				return;
			}
		}

		// If we are here, this comment is not selected.
		if (this.isSelected()) {
			if (this.isCollapsed)
				this.setCollapsed();
			this.sectionProperties.commentListSection.unselect();
		}
	}

	/// This event is Writer-only. Fired by CanvasSectionContainer.
	public onCursorPositionChanged(newPosition: cool.SimpleRectangle): void {
		if (this.sectionProperties.data.rectangles) {
			this.checkIfCursorIsOnThisCommentWriter(newPosition);
		}
	}

	/// This event is Calc-only. Fired by CanvasSectionContainer.
	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onCellAddressChanged(): void {
		if (this.sectionProperties.data.rectangles) {
			if (this.sectionProperties.data.rectangles[0].containsPoint(app.calc.cellCursorRectangle.center))
				this.sectionProperties.commentListSection.sectionProperties.calcCurrentComment = this;
			else if (this.isSelected()) {
				if (!this.sectionProperties.commentListSection.sectionProperties.doNotHideCommentTimer) {
					this.hide();
					this.sectionProperties.commentListSection.sectionProperties.calcCurrentComment = null;
				}
			}
			else if (this.sectionProperties.commentListSection.sectionProperties.calcCurrentComment == this)
				this.sectionProperties.commentListSection.sectionProperties.calcCurrentComment = null;
		}
	}

	private sendClickToCore(point: cool.SimplePoint, count: number) {
		/*
			On Calc, comments are shown when user focuses to the cell or moves the mouse pointer over the cell.
			Comment class (this class) covers the cell area. So the document is blocked now.
			We need to pass the click and double click events to the document.
		*/
		app.map._docLayer._postMouseEvent('buttondown', point.x, point.y, count, 1, 0);
		app.map._docLayer._postMouseEvent('buttonup', point.x, point.y, count, 1, 0);
		app.map.focus();
	}

	public onClick (point: cool.SimplePoint, e: MouseEvent): void {
		const docType = app.map._docLayer._docType;

		if (['presentation', 'drawing'].includes(docType)) {
			this.sectionProperties.commentListSection.selectById(this.sectionProperties.data.id);
		}
		else if (docType === 'text') {
			const mousePoint = point.clone();
			mousePoint.pX += this.myTopLeft[0];
			mousePoint.pY += this.myTopLeft[1];
			app.activeDocument.mouseControl.onClick(mousePoint, e);
		}
		else if (docType === 'spreadsheet') {
			point.pX += this.position[0];
			point.pY += this.position[1];
			this.sendClickToCore(point, 1);
		}
	}

	public onDoubleClick(point: cool.SimplePoint, e: MouseEvent): void {
		if ('spreadsheet' === app.map._docLayer._docType) {
			point.pX += this.position[0];
			point.pY += this.position[1];
			this.sendClickToCore(point, 2);
		}
	}

	public onDraw (): void {
		if (this.sectionProperties.showSelectedCoordinate) {
			if (app.map._docLayer._docType === 'text') {
				const rectangles: Array<cool.SimpleRectangle> = this.sectionProperties.data.rectangles;
				if (rectangles) {
					// We are using view coordinates and it ignores myTopLeft values.
					// We set the pen position to canvas origin here. Then we set it back at the end of this block, after drawing.
					this.context.translate(-this.myTopLeft[0], -this.myTopLeft[1]);

					this.context.fillStyle = this.sectionProperties.usedTextColor;
					this.context.globalAlpha = 0.25;

					for (let i = 0; i < rectangles.length;i ++) {
						const height = Math.max(rectangles[i].v4Y - rectangles[i].v1Y, 3);
						const width = Math.max(rectangles[i].v4X - rectangles[i].v1X, 3);
						this.context.fillRect(rectangles[i].v1X, rectangles[i].v1Y, width, height);
					}

					this.context.globalAlpha = 1;
					this.context.translate(this.myTopLeft[0], this.myTopLeft[1]);
				}
			}
			else if (app.map._docLayer._docType === 'spreadsheet' &&
				 parseInt(this.sectionProperties.data.tab) === app.map._docLayer._selectedPart &&
				 // Don't draw with stale section myTopLeft after a tab-switch.
				 // Wait for a 'commandstatechanged'.
				 this.sectionProperties.commentListSection.sectionProperties.calcCommandStateChanged === true) {

				var cellSize = this.calcOptimumSizeForCalc();
				if (cellSize[0] !== 0 && cellSize[1] !== 0) { // don't draw notes in hidden cells
					// `zoom` represents the current zoom level of the map, retrieved from `this.map.getZoom()`.
					// `baseSize` is a constant that defines the base size of the square at the initial zoom level.
					// `squareDim` calculates the dimension of the square, which dynamically adjusts based on the current zoom level.
					// The dimension increases proportionally to the zoom level by adding `zoom` to `baseSize`.
					var margin = 1;
					var baseSize = 2;
					var zoom = this.map.getZoom();
					var squareDim = baseSize + zoom;

					const isRTL = this.isCalcRTL();

					// this.size may currently have an artificially wide size if mouseEnter without moveLeave seen
					// so fetch the real size
					var x = isRTL ? margin : cellSize[0] - squareDim - margin;
					var commentColor = getComputedStyle(document.body).getPropertyValue('--color-calc-comment');
					this.context.fillStyle = commentColor;
					var region = new Path2D();
					region.moveTo(x, 0);
					region.lineTo(x + squareDim, 0);
					region.lineTo(x + (isRTL ? 0 : squareDim), squareDim);
					region.closePath();
					this.context.fill(region);
				}
			}
		}
	}

	public onMouseUp (point: cool.SimplePoint, e: MouseEvent): void {
		// Hammer.js doesn't fire onClick event after touchEnd event.
		// CanvasSectionContainer fires the onClick event. But since Hammer.js is used for map, it disables the onClick for SectionContainer.
		// We will use this event as click event on touch devices, until we remove Hammer.js (then this code will be removed from here).
		// Control.ColumnHeader.js file is not affected by this situation, because map element (so Hammer.js) doesn't cover headers.
		if (!this.containerObject.isDraggingSomething() && (<any>window).mode.isSmallScreenDevice() || (<any>window).mode.isTablet()) {
			if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing')
				app.map._docLayer._openCommentWizard(this);
			this.onMouseEnter();
			this.onClick(point, e);
		}
	}

	private calcContinueWithMouseEvent (): boolean {
		if (app.map._docLayer._docType === 'spreadsheet') {
			var conditions: boolean = !this.isEdit();
			if (conditions) {
				var sc = this.sectionProperties.commentListSection.sectionProperties.selectedComment;
				if (sc)
					conditions = sc.sectionProperties.data.id !== 'new';
			}
			return conditions;
		}
		else {
			return false;
		}
	}

	private calcCellSize(): number[] {
		var cellPos = app.map._docLayer._cellRangeToTwipRect(this.sectionProperties.data.cellRange).toRectangle();
		return [Math.round((cellPos[2]) * app.twipsToPixels), Math.round((cellPos[3]) * app.twipsToPixels)];
	}

	private calcOptimumSizeForCalc(): number[] {
		const size = this.calcCellSize();

		if (size[0] > this.sectionProperties.calcCommentAreaWidth) size[0] = this.sectionProperties.calcCommentAreaWidth;

		return size;
	}

	public onMouseEnter(): void {
		if (this.calcContinueWithMouseEvent()) {
			// When mouse is above this section, comment's HTML element will be shown.
			// If mouse pointer goes to HTML element, onMouseLeave event shouldn't be fired.
			// But mouse pointer will have left the borders of this section and onMouseLeave event will be fired.
			// Let's do it properly, when mouse is above this section, we will make this section's size bigger and onMouseLeave event will not be fired.
			if (parseInt(this.sectionProperties.data.tab) === app.map._docLayer._selectedPart) {
				var sc = this.sectionProperties.commentListSection.sectionProperties.selectedComment;
				if (sc) {
					if (!sc.isEdit())
						sc.hide();
					else
						return; // Another comment is being edited. Return.
				}

				var containerWidth: number = this.sectionProperties.container.getBoundingClientRect().width;
				var cellPos = app.map._docLayer._cellRangeToTwipRect(this.sectionProperties.data.cellRange).toRectangle();
				this.size = [Math.round((cellPos[2]) * app.twipsToPixels + containerWidth), Math.round((cellPos[3]) * app.twipsToPixels)];
				this.sectionProperties.commentListSection.selectById(this.sectionProperties.data.id);
				this.show();
			}
		}
	}

	public onMouseLeave(point: cool.SimplePoint): void {
		if (this.calcContinueWithMouseEvent()) {
			if (parseInt(this.sectionProperties.data.tab) === app.map._docLayer._selectedPart) {
				// Revert the changes we did on "onMouseEnter" event.
				this.size = this.calcOptimumSizeForCalc();
				if (point) {
					this.hide();
				}
			}
		}
	}

	public onNewDocumentTopLeft (): void {
		this.doPendingInitializationInView();
		this.updatePosition();
	}

	public onCommentDataUpdate(): void {
		this.doPendingInitializationInView();
		this.updatePosition();
	}

	public onRemove (): void {
		this.sectionProperties.commentContainerRemoved = true;

		if (this.sectionProperties.commentListSection.sectionProperties.selectedComment === this)
			this.sectionProperties.commentListSection.sectionProperties.selectedComment = null;

		this.sectionProperties.commentListSection.hideArrow();
		var container = this.sectionProperties.container;

		if (this.sectionProperties.commentMarkerSubSection !== null)
			app.sectionContainer.removeSection(this.sectionProperties.commentMarkerSubSection.name);

		if (container && container.parentElement) {
			var c: number = 0;
			while (c < 10) {
				try {
					container.parentElement.removeChild(container);
					break;
				}
				catch (e) {
					c++;
				}
			}
		}
	}

	public isRootComment(): boolean {
		return this.sectionProperties.data.parent === '0';
	}

	public setAsRootComment(): void {
		this.sectionProperties.data.parent = '0';
		if (app.map._docLayer._docType === 'text')
			this.sectionProperties.data.parentId = '0';
	}

	public getChildrenLength(): number {
		return this.sectionProperties.children.length;
	}

	public getChildByIndex(index: number): Comment {
		if (this.sectionProperties.children.length > index)
			return this.sectionProperties.children[index];
		else
			return null;
	}

	public removeChildByIndex(index: number): void {
		if (this.sectionProperties.children.length > index)
			this.sectionProperties.children.splice(index, 1);
	}

	public getParentCommentId(): string {
		if (this.sectionProperties.data.parent && this.sectionProperties.data.parent !== '0')
			return this.sectionProperties.data.parent;
		else
			return null;
	}

	public getIndexOfChild(comment: Comment): number {
		return this.sectionProperties.children.indexOf(comment);
	}

	public getChildLevel(): number {
		if (this.isRootComment()) return 0;
		const parentComment = this.sectionProperties.commentListSection.getComment(this.getParentCommentId());
		if (parentComment) return parentComment.getChildLevel() + 1;
		return 1; // Comment list not fully initialized but we know we are not root
	}

	public getCommentHeight(invalidateCache: boolean = true): number {
		if (invalidateCache)
			this.cachedCommentHeight = null;
		if (this.cachedCommentHeight === null)
			this.cachedCommentHeight = this.sectionProperties.container.getBoundingClientRect().height
			- this.sectionProperties.childLinesNode.getBoundingClientRect().height;
		return this.cachedCommentHeight;
	}

	public setCollapsed(): void {
		this.isCollapsed = true;

		if (this.sectionProperties.commentListSection.sectionProperties.show != false && !this.isEdit()) {
			this.show();
			this.sectionProperties.commentListSection.removeCommentAttention(this);
		}

		if (this.isRootComment() || app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') {
			this.sectionProperties.container.style.display = '';
			this.sectionProperties.container.style.visibility = 'hidden';
		}
		this.updateThreadInfoIndicator();
		if (this.sectionProperties.data.resolved === 'false'
		|| this.sectionProperties.commentListSection.sectionProperties.showResolved
		|| app.map._docLayer._docType === 'presentation'
		|| app.map._docLayer._docType === 'drawing')
			window.L.DomUtil.addClass(this.sectionProperties.container, 'cool-annotation-collapsed-show');
	}

	public updateThreadInfoIndicator(replycount:number | string = -1): void {
		if (app.map._docLayer._docType === 'spreadsheet')
			return;

		var innerText;
		if (this.isEdit())
			innerText = '!';
		else if (replycount === '!' || typeof replycount === "number" && replycount > 0)
			innerText = replycount;
		else
			innerText = '';

		if (this.sectionProperties.collapsedInfoNode.innerText != innerText)
			this.sectionProperties.collapsedInfoNode.innerText = innerText;

		if (innerText === '' || this.isContainerVisible())
			this.sectionProperties.collapsedInfoNode.style.display = 'none';
		else if ((!this.isContainerVisible() && this.sectionProperties.collapsedInfoNode.innerText !== ''))
			this.sectionProperties.collapsedInfoNode.style.display = '';
	}

	public setExpanded(): void {
		if (!this.isCollapsed)
			return;
		this.isCollapsed = false;
		if (app.map.getDocType() !== 'text' // Comments are resolved only in writer, always show in other apps
		|| this.sectionProperties.data.resolved === 'false'
		|| this.sectionProperties.commentListSection.sectionProperties.showResolved) {
			this.sectionProperties.container.style.display = '';
			// For presentations, only expand if the comment is on the active slide.
			if ((app.map.getDocType() !== 'presentation' && app.map.getDocType() !== 'drawing') || this.isInsideActivePart()) {
				this.sectionProperties.container.style.visibility = '';
			}
		}
		if (app.map._docLayer._docType === 'text')
			this.sectionProperties.collapsedInfoNode.style.display = 'none';
		window.L.DomUtil.removeClass(this.sectionProperties.container, 'cool-annotation-collapsed-show');
	}

	public selectText(startParagraph: number, startIndex: number, endParagraph: number, endIndex: number): void {
		const selection = window.getSelection();
		selection.removeAllRanges();

		const paragraphElements = Array.from(this.sectionProperties.contentText.firstChild.children);
		if (paragraphElements.length === 0) {
			return;
		}
		if (startParagraph > paragraphElements.length - 1 || endParagraph > paragraphElements.length - 1) {
			return;
		}

		// Find start position
		const startElement = paragraphElements[startParagraph] as HTMLElement;
		const startWalker = document.createTreeWalker(
			startElement,
			NodeFilter.SHOW_TEXT,
			null
		);
		const startTextNode = startWalker.nextNode();
		if (!startTextNode) {
			return;
		}

		// Find end position
		const endElement = paragraphElements[endParagraph] as HTMLElement;
		const endWalker = document.createTreeWalker(
			endElement,
			NodeFilter.SHOW_TEXT,
			null
		);
		const endTextNode = endWalker.nextNode();
		if (!endTextNode)
			return;

		// Create and apply the selection range
		const range = document.createRange();
		range.setStart(startTextNode, startIndex);
		range.setEnd(endTextNode, endIndex);

		selection.addRange(range);

		// Ensure the selection is visible
		this.sectionProperties.contentText.focus();
	}

	public autoCompleteMention(username: string, profileLink: string, replacement: string): void {
		const selection = window.getSelection();
		if (!selection)
			return;

		const editorElement = this.getActiveEditorElement();
		let range: Range | null = null;
		if (selection.rangeCount) {
			const candidateRange = selection.getRangeAt(0);
			if (
				!editorElement ||
				editorElement.contains(candidateRange.startContainer)
			)
				range = candidateRange;
		}
		const container = range ? range.startContainer : null;
		let textNode: Text | null = null;
		let cursorPosition = range ? range.endOffset : 0;

		if (container && container.nodeType === Node.TEXT_NODE) {
			textNode = container as Text;
		} else if (!range) {
			if (!editorElement) return;
			const walker = document.createTreeWalker(
				editorElement,
				NodeFilter.SHOW_TEXT,
				null,
			);
			textNode = walker.nextNode() as Text | null;
			if (!textNode) {
				textNode = document.createTextNode('');
				editorElement.appendChild(textNode);
			}
			cursorPosition = textNode.textContent?.length ?? 0;
		}

		const containerText = textNode?.textContent || container?.textContent || editorElement?.textContent || '';
		const mentionStart = containerText.lastIndexOf(replacement, cursorPosition);

		if (mentionStart !== -1) {
			const mentionEnd = mentionStart + replacement.length;

			const beforeMention = containerText.substring(0, mentionStart);
			const afterMention = containerText.substring(mentionEnd);

			const hyperlink = document.createElement('a');
			hyperlink.href = profileLink;
			hyperlink.textContent = `@${username}`;

			if (!textNode || !textNode.parentNode) {
				return;
			}

			textNode.textContent = beforeMention;
			textNode.parentNode?.insertBefore(hyperlink, textNode.nextSibling);

			const afterTextNode = document.createTextNode(afterMention);
			const extraSpaceNode = document.createTextNode('\u00A0');
			hyperlink.parentNode?.insertBefore(extraSpaceNode, hyperlink.nextSibling);
			hyperlink.parentNode?.insertBefore(afterTextNode, extraSpaceNode.nextSibling);

			const newRange = document.createRange();
			newRange.setStartAfter(extraSpaceNode);
			newRange.setEndAfter(extraSpaceNode);

			selection.removeAllRanges();
			selection.addRange(newRange);
		}
	}

	private getActiveEditorElement(): HTMLElement | null {
		if ((<any>window).mode.isSmallScreenDevice()) {
			const commentSection = app.sectionContainer.getSectionWithName(
				app.CSections.CommentList.name,
			);
			const isMobileCommentActive = commentSection?.isMobileCommentActive();
			if (!isMobileCommentActive) return null;
			const mobileCommentModalId = commentSection?.getMobileCommentModalId();
			return document.getElementById(mobileCommentModalId);
		}
		if (
			this.sectionProperties.nodeModify &&
			this.sectionProperties.nodeModify.style.display !== 'none'
		)
			return this.sectionProperties.nodeModifyText;
		if (
			this.sectionProperties.nodeReply &&
			this.sectionProperties.nodeReply.style.display !== 'none'
		)
			return this.sectionProperties.nodeReplyText;
		return (
			this.sectionProperties.nodeModifyText ||
			this.sectionProperties.nodeReplyText ||
			null
		);
	}
}

}

app.definitions.Comment = cool.Comment;
