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
/* eslint-disable @typescript-eslint/explicit-module-boundary-types */

/* See CanvasSectionContainer.ts for explanations. */

window.L.Map.include({
	insertComment: function() {
		if (this.stateChangeHandler.getItemValue('InsertAnnotation') === 'disabled')
			return;
		const editingComment = cool.Comment.isAnyEdit();
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name
		) as cool.CommentSection;
		if (commentSection && editingComment) {
			commentSection.navigateAndFocusComment(editingComment);
			return;
		}

		/*
			if the user inserts a new comment when all the comments
			are hidden, the new comment also goes into hiding as it
			is saved. so we show all the comments instead of hiding
			the newly inserted one.

			comments in a 'spreadsheet' are hidden by default and
			only visible on hover. therefore we wouldn't want to
			show all the comments in a spreadsheet, this jumps the
			view as all the comments are quickly shown and hidden
			again.
		*/
		if (app.map._docLayer._docType !== 'spreadsheet')
			app.map.showComments(true);

		var avatar = undefined;
		var author = this.getViewName(this._docLayer._viewId);
		if (author in this._viewInfoByUserName) {
			avatar = this._viewInfoByUserName[author].userextrainfo.avatar;
		}
		const commentData = {
			text: '',
			textrange: '',
			author: author,
			dateTime: new Date().toISOString(),
			id: 'new', // 'new' only when added by us
			avatar: avatar
		};
		// In PDF, enter a click-to-place mode and let the user pick the spot.
		if (app.file.fileBasedView && commentSection) {
			commentSection.startCommentPlacement(commentData);
			return;
		}
		this._docLayer.newAnnotation(commentData);
	},

	insertThreadedComment: function() {
		if (this.stateChangeHandler.getItemValue('InsertAnnotation') === 'disabled')
			return;
		const editingComment = cool.Comment.isAnyEdit();
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name
		) as cool.CommentSection;
		if (commentSection && editingComment) {
			commentSection.navigateAndFocusComment(editingComment);
			return;
		}

		var avatar = undefined;
		var author = this.getViewName(this._docLayer._viewId);
		if (author in this._viewInfoByUserName) {
			avatar = this._viewInfoByUserName[author].userextrainfo.avatar;
		}
		this._docLayer.newAnnotation({
			text: '',
			textrange: '',
			author: author,
			dateTime: new Date().toISOString(),
			id: 'new',
			avatar: avatar,
			threaded: 'true',
		});
	},

	showResolvedComments: function(on: boolean = false) {
		const unoCommand = '.uno:ShowResolvedAnnotations';
		this.sendUnoCommand(unoCommand);
		app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).setViewResolved(on);
		this.uiManager.setDocTypePref('ShowResolved', on);
	},

	showComments: function(on: boolean = false) {
		app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).setView(on);
		this.uiManager.setDocTypePref('showannotations', on);
		this.fire('commandstatechanged', {commandName : 'showannotations', state : on ? 'true': 'false'});
		this.fire('showannotationschanged', {state: on ? 'true': 'false'});
	}
});


declare var JSDialog: any;

namespace cool {

export class CommentSection extends CanvasSectionObject {
	backgroundColor: string = app.sectionContainer.getClearColor();
	expand: string[] = ['bottom'];
	processingOrder: number = app.CSections.CommentList.processingOrder;
	drawingOrder: number = app.CSections.CommentList.drawingOrder;
	zIndex: number = app.CSections.CommentList.zIndex;
	interactable: boolean = false;
	sectionProperties: {
		// remember the tab when importComments() was called.
		calcLastTab: number;
		// Keep a reference to the original set of comments received.
		calcMasterList: Array<any>;
		// Has the 'commandstatechanged' msg arrived after a tab switch.
		// This a precondition for drawing spreadsheet comment marker.
		calcCommandStateChanged: boolean;
		commentList: Array<Comment>;
		selectedComment: Comment | null;
		calcCurrentComment: Comment | null;
		marginY: number;
		offset: number;
		width: number;
		commentWidth: number;
		collapsedMarginToTheEdge: number;
		deflectionOfSelectedComment: number;
		collapsedCommentWidth: number;
		showSelectedBigger: boolean;
		commentsAreListed: boolean;
		show: boolean;
		showResolved: boolean;
		[key: string]: any;
		canvasContainerBounds: DOMRect;
	};
	disableLayoutAnimation: boolean = false;

	mobileCommentId: string = 'new-annotation-dialog';
	mobileCommentModalId: string;

	map: any;
	static autoSavedComment: cool.Comment;
	static needFocus: cool.Comment;
	static commentWasAutoAdded: boolean = false;
	static pendingImport: boolean = false;
	static importingComments: boolean = false; // active during comments insertion, disable scroll

	// To associate comment id with its index in commentList array.
	private idIndexMap: Map<any, number>;

	// Dirty flag: set by onNewDocumentTopLeft, consumed by onDraw.
	// Collapses multiple onNewDocumentTopLeft calls into a single layout
	// pass and avoids the extra requestReDraw that updateDOM would trigger.
	private _commentPositionDirty: boolean = false;

	private annotationMinSize: number;
	private annotationMaxSize: number;
	escapeListener: (e: KeyboardEvent) => void;

	// Active "click anywhere on the page to place a new comment" mode for PDF.
	// When non-null, the next document mousedown is consumed to position a new
	// comment at that point instead of being forwarded to core. A click+drag
	// instead of a plain click captures an anchor area; if the drag stays
	// below DRAG_THRESHOLD_PX the click is treated as a point placement.
	private placementCommentData: any = null;
	private placementSavedCursor: string | null = null;
	private placementStart: { cX: number; cY: number } | null = null;
	private placementOverlay: HTMLDivElement | null = null;
	private static readonly DRAG_THRESHOLD_PX = 5;

	constructor () {
		super(app.CSections.CommentList.name);

		this.map = window.L.Map.THIS;
		this.anchor = ['top', 'right'];
		this.sectionProperties.docLayer = this.map._docLayer;
		this.sectionProperties.calcLastTab = -1;
		this.sectionProperties.calcMasterList = [];
		this.sectionProperties.calcCommandStateChanged = true;
		this.sectionProperties.doNotHideCommentTimer = null; // For _goToCalcComment, where comment needs to show, despite async events trying to hide it
		this.sectionProperties.commentList = new Array(0);
		this.sectionProperties.selectedComment = null;
		this.sectionProperties.arrow = null;
		this.sectionProperties.show = null;
		this.sectionProperties.showResolved = false;
		this.sectionProperties.marginY = 20 * app.dpiScale;
		this.sectionProperties.offset = 5 * app.dpiScale;
		this.sectionProperties.width = Math.round(1 * app.dpiScale); // Configurable variable.
		this.sectionProperties.scrollAnnotation = null; // For impress, when 1 or more comments exist.
		this.sectionProperties.commentWidth = CommentSection.getCommentWidth();
		this.sectionProperties.commentWidthBigger =  588 * app.dpiScale;
		this.sectionProperties.collapsedCommentWidth = 32 * 1.5 * app.dpiScale;
		this.sectionProperties.collapsedMarginToTheEdge = 120; // CSS pixels.
		this.sectionProperties.deflectionOfSelectedComment = 160; // CSS pixels.
		this.sectionProperties.showSelectedBigger = false;
		this.sectionProperties.calcCurrentComment = null; // We don't automatically show a Calc comment when cursor is on its cell. But we remember it to show if user presses Alt+C keys.
		this.sectionProperties.reLayout = true;

		// This (commentsAreListed) variable means that comments are shown as a list on the right side of the document.
		this.sectionProperties.commentsAreListed = (app.map._docLayer._docType === 'text' || app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') && !(<any>window).mode.isSmallScreenDevice();
		this.idIndexMap = new Map<any, number>();
		this.mobileCommentModalId = this.map.uiManager.generateModalId(this.mobileCommentId);
		this.annotationMinSize = Number(getComputedStyle(document.documentElement).getPropertyValue('--annotation-min-size'));
		this.annotationMaxSize = Number(getComputedStyle(document.documentElement).getPropertyValue('--annotation-max-size'));

		// PDF placement-mode handlers are added to / removed from the DOM via
		// addEventListener/removeEventListener; bind once here so the same
		// reference is used for both calls.
		this.onPlacementMouseDown = this.onPlacementMouseDown.bind(this);
		this.onPlacementMouseMove = this.onPlacementMouseMove.bind(this);
		this.onPlacementMouseUp = this.onPlacementMouseUp.bind(this);
		this.onPlacementKeyDown = this.onPlacementKeyDown.bind(this);
	}

	public onInitialize (): void {
		this.checkCollapseState();

		this.map.on('RedlineAccept', this.onRedlineAccept, this);
		this.map.on('RedlineReject', this.onRedlineReject, this);
		this.map.on('updateparts', this.importPartSpecificComments, this);
		this.map.on('updateparts', this.showHideComments, this);
		this.map.on('AnnotationScrollUp', this.onAnnotationScrollUp, this);
		this.map.on('AnnotationScrollDown', this.onAnnotationScrollDown, this);

		this.map.on('commandstatechanged', function (event: any) {
			this.sectionProperties.calcCommandStateChanged = true;
			if (event.commandName === '.uno:ShowResolvedAnnotations')
				this.setViewResolved(event.state === 'true');
			else if (event.commandName === 'showannotations')
				this.setView(event.state === 'true');
			else if (event.commandName === '.uno:ShowTrackedChanges' && event.state === 'true')
				app.socket.sendMessage('commandvalues command=.uno:ViewAnnotations');
		}, this);

		this.map.on('zoomend', function() {
			this.checkCollapseState();
			this.layout(true);
		}, this);

		this.backgroundColor = this.containerObject.getClearColor();

		if ((<any>window).mode.isSmallScreenDevice()) {
			this.setShowSection(false);
			this.size[0] = 0;
		}

		this.escapeSelectedComment();
	}

	public navigateAndFocusComment(annotation: any): void {
		if (!annotation) return;

		this.scrollCommentIntoView(annotation);
		this.highlightComment(annotation);

		if (this.sectionProperties.selectedComment !== annotation) {
			this.select(annotation, true);
		}

		annotation.focus();
		this.addCommentAttention(annotation);
	}

	private addCommentAttention(annotation: any): void {
		if (!annotation) return;

		const wrapper = annotation.sectionProperties.wrapper;
		if (wrapper) {
			wrapper.classList.add('cool-annotation-inedit');
		}
	}

	private removeCommentAttention(annotation: any): void {
		if (!annotation) return;

		const wrapper = annotation.sectionProperties.wrapper;
		if (wrapper) {
			wrapper.classList.remove('cool-annotation-inedit');
		}
	}

	private checkCollapseState(): void {
		if (!(<any>window).mode.isSmallScreenDevice() && app.map._docLayer._docType !== 'spreadsheet') {
			if (this.shouldCollapse()) {
				this.sectionProperties.deflectionOfSelectedComment = 180;
				this.setCollapsed();
			}
			else {
				this.sectionProperties.deflectionOfSelectedComment = 70;
				this.setExpanded();
			}

			if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing')
				this.showHideComments();
		}
	}

	private findNextPartWithComment (currentPart: number): number {
		for (var i = 0;  i < this.sectionProperties.commentList.length; i++) {
			if (this.sectionProperties.commentList[i].sectionProperties.partIndex > currentPart) {
				return this.sectionProperties.commentList[i].sectionProperties.partIndex;
			}
		}
		return -1;
	}

	private findPreviousPartWithComment (currentPart: number): number {
		for (var i = this.sectionProperties.commentList.length - 1;  i > -1; i--) {
			if (this.sectionProperties.commentList[i].sectionProperties.partIndex < currentPart) {
				return this.sectionProperties.commentList[i].sectionProperties.partIndex;
			}
		}
		return -1;
	}

	public onAnnotationScrollDown (): void {
		var index = this.findNextPartWithComment(app.map._docLayer._selectedPart);
		if (index >= 0) {
			this.map.setPart(index);
		}
	}

	public onAnnotationScrollUp (): void {
		var index = this.findPreviousPartWithComment(app.map._docLayer._selectedPart);
		if (index >= 0) {
			this.map.setPart(index);
		}
	}

	private checkSize (): void {
		// When there is no comment || file is a spreadsheet || view type is mobile, we set this section's size to [0, 0].
		if (app.map._docLayer._docType === 'spreadsheet' || (<any>window).mode.isSmallScreenDevice() || this.sectionProperties.commentList.length === 0)
		{
			if (app.map._docLayer._docType === 'presentation' && this.sectionProperties.scrollAnnotation) {
				this.map.removeControl(this.sectionProperties.scrollAnnotation);
				this.sectionProperties.scrollAnnotation = null;
			}
		}
		else if (app.map._docLayer._docType === 'presentation') { // If there are comments but none of them are on the selected part.
			if (!this.sectionProperties.scrollAnnotation) {
				this.sectionProperties.scrollAnnotation = window.L.control.scrollannotation();
				this.sectionProperties.scrollAnnotation.addTo(this.map);
			}
		}
	}

	private isEditing(): boolean {
		const textBoxes = this.sectionProperties.commentList
			.flatMap((section: any) => [section.sectionProperties.nodeModifyText, section.sectionProperties.nodeReplyText])
			.filter((textBox: any) => textBox !== undefined);

		return textBoxes.includes(document.activeElement);
	}

	public getActiveEdit(): Comment {
		if (!this.sectionProperties.selectedComment) {
			return null;
		}
		if (this.sectionProperties.selectedComment.isEdit()) {
			return this.sectionProperties.selectedComment;
		}
		var openArray: Comment[] = [];
		this.getChildren(this.sectionProperties.selectedComment, openArray);
		for (var i = 0; i < openArray.length; i++) {
			if (openArray[i].isEdit()) {
				return openArray[i];
			}
		}
		return null;
	}

	public commentsHiddenOrNotPresent(): boolean {
		if (this.sectionProperties.show != true || this.sectionProperties.commentList.length == 0)
			return true;
		return false;
	}

	public setCollapsed(): void {
		if (this.sectionProperties.show != true || this.isEditing()) {
			return;
		}
		this.isCollapsed = true;
		this.unselect();
		for (var i: number = 0; i < this.sectionProperties.commentList.length; i++) {
			if (this.sectionProperties.commentList[i].sectionProperties.data.id !== 'new')
				this.sectionProperties.commentList[i].setCollapsed();
			$(this.sectionProperties.commentList[i].sectionProperties.container).addClass('collapsed-comment');
			if (this.sectionProperties.commentList[i].isRootComment())
				this.collapseReplies(i, this.sectionProperties.commentList[i].sectionProperties.data.id);
		}
	}

	public setExpanded(): void {
		this.isCollapsed = false;
		for (var i: number = 0; i < this.sectionProperties.commentList.length; i++) {
			this.sectionProperties.commentList[i].setExpanded();
			$(this.sectionProperties.commentList[i].sectionProperties.container).removeClass('collapsed-comment');
		}
	}

	public static getCommentWidth(): number {
		return 200 * 1.3 * app.dpiScale;
	}

	/// If the current layout has more than one pages in a row, so the comment should be next to
	/// the document content instead of next to the page.
	private static isMultiColumnLayout(): boolean {
		const type = app.activeDocument.activeLayout.type;
		return type === 'ViewLayoutMultiPage' || type === 'ViewLayoutCompareChanges';
	}

	public calculateAvailableSpace() {
		if (CommentSection.isMultiColumnLayout()) {
			const layout = app.activeDocument.activeLayout as ViewLayoutMultiPage | ViewLayoutCompareChanges;
			const availableSpace = layout.getTotalSideSpace();
			return Math.round(availableSpace * 0.5 / app.dpiScale);
		}
		else {
			let availableSpace = (this.containerObject.getDocumentAnchorSection().size[0] - app.activeDocument.fileSize.pX) * 0.5;
			availableSpace = Math.round(availableSpace / app.dpiScale);
			return availableSpace;
		}
	}

	public shouldCollapse (): boolean {
		if (app.map._docLayer._docType === 'text')
			return false;
		if (!this.containerObject.getDocumentAnchorSection() || app.map._docLayer._docType === 'spreadsheet' || (<any>window).mode.isSmallScreenDevice())
			return false;
		const availableSpace = this.calculateAvailableSpace();
		/*
			in case the comment section is half hidden and there
			is some space on the left side of the document (since
			the document is centered), we don't collapse the comments.
			the comments section doesn't end up in such layout normally,
			either the user resized the window, or zoomed in. both of
			those events are being listened to in ViewLayoutWriter and
			when that happens, `ViewLayoutWriter` moves the document to
			the left in function `adjustDocumentMarginsForComments`.
		*/
		if (app.activeDocument.activeLayout.viewHasEnoughSpaceToShowFullWidthComments())
			return false;
		return availableSpace < this.sectionProperties.commentWidth && availableSpace >= 0;
	}

	public hideAllComments (): void {
		for (var i: number = 0; i < this.sectionProperties.commentList.length; i++) {
			if (!this.sectionProperties.doNotHideCommentTimer
				|| this.sectionProperties.commentList[i] !== this.sectionProperties.selectedComment)
				this.sectionProperties.commentList[i].hide();
			const part = app.map._docLayer._selectedPart;
			if (app.map._docLayer._docType === 'spreadsheet') {
				// Change drawing order so they don't prevent each other from being shown.
				if (parseInt(this.sectionProperties.commentList[i].sectionProperties.data.tab) === part) {
					this.sectionProperties.commentList[i].drawingOrder = 2;
				}
				else {
					this.sectionProperties.commentList[i].drawingOrder = 1;
				}
			}
		}

		if (app.map._docLayer._docType === 'spreadsheet')
			this.containerObject.applyDrawingOrders();
	}

	// Mobile.
	private getChildren(comment: any, array: Array<any>) {
		for (var i = 0; i < comment.sectionProperties.children.length; i++) {
			array.push(comment.sectionProperties.children[i]);
			if (comment.sectionProperties.children[i].sectionProperties.children.length > 0)
				this.getChildren(comment.sectionProperties.children[i], array);
		}
	}

	// Mobile.
	private getCommentListOneDimensionalArray() {
		// 1 dimensional array of ordered comments.
		var openArray = [];

		for (var i = 0; i < this.sectionProperties.commentList.length; i++) {
			if (this.sectionProperties.commentList[i].isRootComment()) {
				openArray.push(this.sectionProperties.commentList[i]);
				if (this.sectionProperties.commentList[i].sectionProperties.children.length > 0)
					this.getChildren(this.sectionProperties.commentList[i], openArray);
			}
		}
		return openArray;
	}

	private createCommentStructureWriter (menuStructure: any, threadOnly: any): void {
		var rootComment, comment;
		const commentList = this.getCommentListOneDimensionalArray();
		const showResolved = this.sectionProperties.showResolved;

		if (threadOnly) {
			if (!threadOnly.sectionProperties.data.trackchange && threadOnly.sectionProperties.data.parent !== '0')
				threadOnly = commentList[this.getIndexOf(threadOnly.sectionProperties.data.parent)];
		}

		for (var i = 0; i < commentList.length; i++) {
			if (commentList[i].isRootComment() || commentList[i].sectionProperties.data.trackchange) {
				var commentThread = [];
				do {
					comment = {
						id: 'comment' + commentList[i].sectionProperties.data.id,
						enable: true,
						data: commentList[i].sectionProperties.data,
						type: 'comment',
						text: commentList[i].sectionProperties.data.text,
						annotation: commentList[i],
						children: []
					};

					if (showResolved || comment.data.resolved !== 'true') {
						commentThread.unshift(comment);
					}
					i++;
				} while (commentList[i] && commentList[i].sectionProperties.data.parent !== '0');
				i--;

				if (commentThread.length > 0)
				{
					rootComment = {
						id: commentThread[commentThread.length - 1].id,
						enable: true,
						data: commentThread[commentThread.length - 1].data,
						type: 'rootcomment',
						text: commentThread[commentThread.length - 1].data.text,
						annotation: commentThread[commentThread.length - 1].annotation,
						children: commentThread
					};

					var matchingThread = threadOnly && threadOnly.sectionProperties.data.id === commentThread[0].data.id;
					if (matchingThread)
						menuStructure['children'] = commentThread;
					else if (!threadOnly)
						menuStructure['children'].push(rootComment);
				}
			}
		}
	}

	public createCommentStructureImpress (menuStructure: any, threadOnly: any): void {
		var rootComment;

		for (var i in this.sectionProperties.commentList) {
			const matchingThread = !threadOnly || (threadOnly && threadOnly.sectionProperties.data.id === this.sectionProperties.commentList[i].sectionProperties.data.id);
			if (matchingThread && (this.sectionProperties.commentList[i].sectionProperties.partIndex === app.map._docLayer._selectedPart || app.file.fileBasedView)) {
				rootComment = {
					id: 'comment' + this.sectionProperties.commentList[i].sectionProperties.data.id,
					enable: true,
					data: this.sectionProperties.commentList[i].sectionProperties.data,
					type: threadOnly ? 'comment' : 'rootcomment',
					text: this.sectionProperties.commentList[i].sectionProperties.data.text,
					annotation: this.sectionProperties.commentList[i],
					children: []
				};
				menuStructure['children'].push(rootComment);
			}
		}
	}

	public createCommentStructureCalc (menuStructure: any, threadOnly: any): void {
		var rootComment;
		const commentList = this.sectionProperties.commentList;
		const selectedTab = app.map._docLayer._selectedPart;

		for (var i: number = 0; i < commentList.length; i++) {
			var matchingThread = !threadOnly || (threadOnly && threadOnly.sectionProperties.data.id === commentList[i].sectionProperties.data.id);
			if (parseInt(commentList[i].sectionProperties.data.tab) === selectedTab && matchingThread) {
				rootComment = {
					id: 'comment' + commentList[i].sectionProperties.data.id,
					enable: true,
					data: commentList[i].sectionProperties.data,
					type: threadOnly ? 'comment' : 'rootcomment',
					text: commentList[i].sectionProperties.data.text,
					annotation: commentList[i],
					children: []
				};
				menuStructure['children'].push(rootComment);
			}
		}
	}

	// threadOnly - takes annotation indicating which thread will be generated
	public createCommentStructure (menuStructure: any, threadOnly: any): void {
		if (app.map._docLayer._docType === 'text') {
			this.createCommentStructureWriter(menuStructure, threadOnly);
		}
		else if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') {
			this.createCommentStructureImpress(menuStructure, threadOnly);
		}
		else if (app.map._docLayer._docType === 'spreadsheet') {
			this.createCommentStructureCalc(menuStructure, threadOnly);
		}
	}

	public isMobileCommentActive (): boolean {
		const newComment = document.getElementById(this.mobileCommentId);
		if (!newComment)
			return false;
		return newComment.style.display !== 'none'
	}

	public getMobileCommentModalId (): string {
		return this.mobileCommentModalId;
	}

	public newAnnotationMobile (comment: any, addCommentFn: any, isMod: any): void {
		const commentData = comment.sectionProperties.data;

		const callback = function(div: HTMLDivElement) {
			if (div.textContent || div.innerHTML) {
				var annotation = comment;

				annotation.sectionProperties.data.text = div.textContent;
				annotation.sectionProperties.data.html = div.innerHTML;
				comment.text = div.textContent;

				addCommentFn.call(annotation, annotation, comment);
				if (!isMod)
					this.containerObject.removeSection(annotation);
			}
			else {
				this.cancel(comment);
			}
		}.bind(this);

		let listId = 'mentionPopupList';
		if (this.map.mention)
			listId = this.map.mention.getPopupId() + 'List';
		const json = this.map.uiManager._modalDialogJSON(this.mobileCommentId, '', true, [
			{
				id: 'input-modal-input',
				type: 'multilineedit',
				text: (commentData.text && isMod ? commentData.text: ''),
				html: (commentData.html && isMod ? commentData.html: ''),
				contenteditable: true
			},
			{
				id: listId,
				type: 'treelistbox',
				text: '',
				enabled: true,
				singleclickactivate: false,
				fireKeyEvents: true,
				hideIfEmpty: true,
				entries: [] as Array<TreeEntryJSON>,
				noSearchField: true,
			},
			{
				id: '',
				type: 'buttonbox',
				text: '',
				enabled: true,
				children: [
					{
						id: 'response-cancel',
						type: 'pushbutton',
						text: _('Cancel'),
					},
					{
						id: 'response-ok',
						type: 'pushbutton',
						text: _('Save'),
						'has_default': true,
					}
				],
				vertical: false,
				layoutstyle: 'end'
			},
		]);

		const cancelFunction = function() {
			this.cancel(comment);
			this.map.uiManager.closeModal(this.mobileCommentModalId);
		}.bind(this);

		const mentionListCallback = function(objectType: any, eventType: any, object: any, index: number) {
				const mention = this.map.mention;
				if (eventType === 'close')
					mention.closeMentionPopup(false);
				else if (eventType === 'select' || eventType === 'activate') {
					const item = mention.getMentionUserData(index);
					const replacement = '@' + mention.getPartialMention();
					const uid = item.label ?? item.username
					if (uid !== '' && item.profile !== '')
						comment.autoCompleteMention(uid, item.profile, replacement)
					mention.closeMentionPopup(false);
				}
		}.bind(this);

		this.map.uiManager.showModal(json, [
			{id: 'response-ok', func: function() {
				if (typeof callback === 'function') {
					const input = document.getElementById('input-modal-input') as HTMLDivElement;
					callback(input);
				}
				this.map.uiManager.closeModal(this.mobileCommentModalId);
			}.bind(this)},
			{id: 'response-cancel', func: cancelFunction},
			{id: '__POPOVER__', func: cancelFunction},
			{id: '__DIALOG__', func: cancelFunction},
			{id: listId, func: mentionListCallback}
		]);

		app.layoutingService.appendLayoutingTask(() => {
			const multilineEditDiv = document.getElementById('input-modal-input');
			multilineEditDiv.addEventListener('input', function(ev: any){
				if (ev && app.map._docLayer._docType === 'text') {
					// special handling for mentions
					this.map?.mention.handleMentionInput(ev, comment.isNewPara());
				}
			}.bind(this));

			const tagTd = 'td',
			empty = '',
			tagDiv = 'div';
			const author = window.L.DomUtil.create('table', 'cool-annotation-table');
			const tbody = window.L.DomUtil.create('tbody', empty, author);
			const tr = window.L.DomUtil.create('tr', empty, tbody);
			const tdImg = window.L.DomUtil.create(tagTd, 'cool-annotation-img', tr);
			const tdAuthor = window.L.DomUtil.create(tagTd, 'cool-annotation-author', tr);
			const imgAuthor = window.L.DomUtil.create('img', 'avatar-img', tdImg);
			const user = this.map.getViewId(commentData.author);
			if (this.map['wopi'] && this.map['wopi'].CommentAvatarUrl)
				imgAuthor.setAttribute('src', this.map['wopi'].CommentAvatarUrl);
			else
				app.LOUtil.setUserImage(imgAuthor, this.map, user);
			imgAuthor.setAttribute('width', 32);
			imgAuthor.setAttribute('height', 32);
			const authorAvatarImg = imgAuthor;
			const contentAuthor = window.L.DomUtil.create(tagDiv, 'cool-annotation-content-author', tdAuthor);
			const contentDate = window.L.DomUtil.create(tagDiv, 'cool-annotation-date', tdAuthor);

			$(contentAuthor).text(commentData.author);
			if (!this.map['wopi'] || !this.map['wopi'].CommentAvatarUrl)
				$(authorAvatarImg).attr('src', commentData.avatar);
			if (user >= 0) {
				const color = app.LOUtil.rgbToHex(this.map.getViewColor(user));
				$(authorAvatarImg).css('border-color', color);
			}

			if (commentData.dateTime) {
				const d = new Date(commentData.dateTime.replace(/,.*/, 'Z'));
				const dateOptions = { weekday: 'short', year: 'numeric', month: 'short', day: 'numeric' };
				$(contentDate).text(isNaN(d.getTime()) ? comment.dateTime: d.toLocaleDateString((<any>String).locale, <any>dateOptions));
			}

			const newAnnotationDialog = document.getElementById(this.mobileCommentId);
			$(newAnnotationDialog).css('width', '100%');
			const dialogInput = newAnnotationDialog.children[0];
			$(dialogInput).css('height', '30vh');
			const parent = newAnnotationDialog.parentElement;
			parent.insertBefore(author, parent.childNodes[0]);
			document.getElementById('input-modal-input').focus();
		});
	}

	public highlightComment (comment: any): void {
		this.removeHighlighters();

		const commentList = this.sectionProperties.commentList;
		let lastChild: any = this.getLastChildIndexOf(comment.sectionProperties.data.id);

		while (true && lastChild >= 0) {
			commentList[lastChild].highlight();
			if (commentList[lastChild].isRootComment())
				break;

			lastChild = this.getIndexOf(commentList[lastChild].sectionProperties.data.parent);
		}
	}

	public removeHighlighters (): void {
		var commentList = this.sectionProperties.commentList;
		for (var i: number = 0; i < commentList.length; i++) {
			if (commentList[i].sectionProperties.isHighlighted) {
				commentList[i].removeHighlight();
			}
		}
	}

	public removeItem (id: any): void {
		var annotation;
		for (var i = 0; i < this.sectionProperties.commentList.length; i++) {
			annotation = this.sectionProperties.commentList[i];
			if (annotation.sectionProperties.data.id === id) {
				this.containerObject.removeSection(annotation.name);
				this.sectionProperties.commentList.splice(i, 1);
				this.updateIdIndexMap();
				break;
			}
		}
		// Synchronize calcMasterList
		var masterElementIndex = this.sectionProperties.calcMasterList.findIndex(el => el.id == id);
		if (masterElementIndex >= 0) {
			this.sectionProperties.calcMasterList.splice(masterElementIndex, 1);
		}
		this.checkSize();
		app.map.fire('deleteannotation');
	}

	public startCommentPlacement(commentData: any): void {
		// An additional Insert Comment trigger while placement is already
		// pending is a no-op.
		if (this.placementCommentData)
			return;

		const canvas = document.getElementById('document-canvas') as HTMLCanvasElement | null;
		if (!canvas)
			return;

		this.placementCommentData = commentData;
		this.placementSavedCursor = canvas.style.cursor;
		canvas.style.cursor = 'crosshair';

		// Capture phase, before CanvasSectionContainer's onmousedown property
		// handler dispatches to MouseControl/etc.
		canvas.addEventListener('mousedown', this.onPlacementMouseDown, true);
		document.addEventListener('keydown', this.onPlacementKeyDown, true);
	}

	private exitCommentPlacement(): void {
		const canvas = document.getElementById('document-canvas') as HTMLCanvasElement | null;
		if (canvas) {
			canvas.removeEventListener('mousedown', this.onPlacementMouseDown, true);
			canvas.removeEventListener('mousemove', this.onPlacementMouseMove, true);
			canvas.removeEventListener('mouseup', this.onPlacementMouseUp, true);
			if (this.placementSavedCursor !== null)
				canvas.style.cursor = this.placementSavedCursor;
		}
		document.removeEventListener('keydown', this.onPlacementKeyDown, true);
		this.removePlacementOverlay();
		this.placementCommentData = null;
		this.placementSavedCursor = null;
		this.placementStart = null;
	}

	private onPlacementMouseDown(e: MouseEvent): void {
		if (e.button !== 0) return; // ignore right/middle clicks
		e.stopImmediatePropagation();
		e.preventDefault();
		const canvas = e.currentTarget as HTMLCanvasElement;
		const rect = canvas.getBoundingClientRect();
		this.placementStart = {
			cX: e.clientX - rect.left,
			cY: e.clientY - rect.top,
		};
		// Track move/up on the canvas so a click-and-drag draws a rectangle.
		// A plain click without movement is handled by mouseup with the same
		// start/end point and falls through to point placement.
		canvas.addEventListener('mousemove', this.onPlacementMouseMove, true);
		canvas.addEventListener('mouseup', this.onPlacementMouseUp, true);
	}

	private onPlacementMouseMove(e: MouseEvent): void {
		if (!this.placementStart) return;
		e.stopImmediatePropagation();
		e.preventDefault();
		const canvas = e.currentTarget as HTMLCanvasElement;
		const rect = canvas.getBoundingClientRect();
		const cur = { cX: e.clientX - rect.left, cY: e.clientY - rect.top };
		const dx = cur.cX - this.placementStart.cX;
		const dy = cur.cY - this.placementStart.cY;
		if (!this.placementOverlay
			&& Math.hypot(dx, dy) < CommentSection.DRAG_THRESHOLD_PX)
			return;
		this.updatePlacementOverlay(canvas, rect, cur);
	}

	private onPlacementMouseUp(e: MouseEvent): void {
		if (e.button !== 0 || !this.placementStart) return;
		e.stopImmediatePropagation();
		e.preventDefault();
		const canvas = e.currentTarget as HTMLCanvasElement;
		const rect = canvas.getBoundingClientRect();
		const end = { cX: e.clientX - rect.left, cY: e.clientY - rect.top };
		this.finishCommentPlacement(canvas, end);
	}

	private updatePlacementOverlay(canvas: HTMLCanvasElement, rect: DOMRect,
		cur: { cX: number; cY: number }): void {
		if (!this.placementOverlay) {
			this.placementOverlay = document.createElement('div');
			this.placementOverlay.className = 'comment-placement-overlay';
			canvas.parentElement.appendChild(this.placementOverlay);
		}
		const x1 = Math.min(this.placementStart.cX, cur.cX);
		const y1 = Math.min(this.placementStart.cY, cur.cY);
		const w = Math.abs(cur.cX - this.placementStart.cX);
		const h = Math.abs(cur.cY - this.placementStart.cY);
		// rect.left/top are viewport-relative; the overlay is parented to the
		// canvas's parent, which uses the same offset, so add canvas.offsetLeft
		// /Top to align with the canvas position inside that parent.
		this.placementOverlay.style.left = (canvas.offsetLeft + x1) + 'px';
		this.placementOverlay.style.top = (canvas.offsetTop + y1) + 'px';
		this.placementOverlay.style.width = w + 'px';
		this.placementOverlay.style.height = h + 'px';
	}

	private removePlacementOverlay(): void {
		if (this.placementOverlay && this.placementOverlay.parentNode)
			this.placementOverlay.parentNode.removeChild(this.placementOverlay);
		this.placementOverlay = null;
	}

	private onPlacementKeyDown(e: KeyboardEvent): void {
		if (e.key !== 'Escape') return;
		e.stopImmediatePropagation();
		e.preventDefault();
		this.exitCommentPlacement();
	}

	private finishCommentPlacement(canvas: HTMLCanvasElement,
		end: { cX: number; cY: number }): void {
		const commentData = this.placementCommentData;
		const start = this.placementStart;
		// Treat the gesture as a drag-to-area when the user moved past the
		// threshold; otherwise it's a plain click and we anchor a 5x5 mm
		// marker at the click point (current behaviour, preserved by the
		// engine when no Width/Height args reach .uno:InsertAnnotation).
		const isDrag = Math.hypot(end.cX - start.cX, end.cY - start.cY)
			>= CommentSection.DRAG_THRESHOLD_PX;
		// canvasToDocumentPoint expects coordinates relative to the canvas DOM
		// element. Use the top-left corner of the dragged box as the anchor;
		// cap the bottom-right at the canvas in case the drag escaped it.
		const layout = app.activeDocument.activeLayout;
		const tlPoint = new cool.SimplePoint(0, 0);
		tlPoint.cX = isDrag ? Math.min(start.cX, end.cX) : end.cX;
		tlPoint.cY = isDrag ? Math.min(start.cY, end.cY) : end.cY;
		const docTL = layout.canvasToDocumentPoint(tlPoint);
		if (Number.isNaN(docTL.x) || Number.isNaN(docTL.y))
			return;

		let docBR = null;
		if (isDrag) {
			const brPoint = new cool.SimplePoint(0, 0);
			brPoint.cX = Math.max(start.cX, end.cX);
			brPoint.cY = Math.max(start.cY, end.cY);
			docBR = layout.canvasToDocumentPoint(brPoint);
			if (Number.isNaN(docBR.x) || Number.isNaN(docBR.y))
				return;
		}

		// docTL.x/.y are stacked-document twips. fileBasedView lays pages
		// out vertically with a fixed _spaceBetweenParts gap. Compute the
		// containing page and reject clicks that fell in a horizontal margin,
		// in an inter-page gap, or past the last page - the user should stay
		// in placement mode and try again on a real page.
		const docLayer = app.map._docLayer;
		const additionPerPart = docLayer._partHeightTwips + docLayer._spaceBetweenParts;
		const partIndex = Math.floor(docTL.y / additionPerPart);
		const yInPart = docTL.y - partIndex * additionPerPart;
		if (partIndex < 0 || partIndex >= docLayer._parts
			|| docTL.x < 0 || docTL.x > docLayer._partWidthTwips
			|| yInPart > docLayer._partHeightTwips)
			return;

		// For a drag, clamp the bottom-right to the same page so a comment
		// that overshot the page boundary still produces a valid in-page area.
		if (docBR) {
			const partTop = partIndex * additionPerPart;
			docBR.x = Math.min(docBR.x, docLayer._partWidthTwips);
			docBR.y = Math.min(docBR.y, partTop + docLayer._partHeightTwips);
		}

		// Switching the active part keeps save()'s setPart wrapper consistent
		// and lets newAnnotation pick up the correct parthash; yAddition lets
		// save() back the per-page offset out before sending PositionY.
		commentData.yAddition = partIndex * additionPerPart;
		this.map.setPart(partIndex, false);

		commentData.position = [docTL.x, docTL.y];
		if (docBR)
			commentData.size = [docBR.x - docTL.x, docBR.y - docTL.y];
		this.exitCommentPlacement();
		this.sectionProperties.docLayer.newAnnotation(commentData);
	}

	public click (annotation: any): void {
		this.select(annotation);
		app.map.fire('postMessage', {
			msgId: 'Clicked_Comment',
			args: { Id: annotation.sectionProperties.data.id }
		});
	}

	public save (annotation: any): void {
		var comment;
		if (annotation.sectionProperties.data.id === 'new') {
			// PDF click-to-place: when the user picked a position before
			// opening the editor, ship it as twips so core can place the
			// annotation there instead of (0, 0). Mirrors the EditAnnotation
			// pattern in CommentMarkerSubSection.sendAnnotationPositionChange.
			// PDF drag-to-area: when the user dragged out a rectangle,
			// also ship Width/Height so core stores the area as the PDF /Rect.
			let positionArgs: any = {};
			if (annotation.sectionProperties.data.position) {
				const pos = annotation.sectionProperties.data.position;
				let py = pos[1];
				if (app.file.fileBasedView)
					py -= annotation.sectionProperties.data.yAddition || 0;
				positionArgs = {
					PositionX: { type: 'int32', value: pos[0] },
					PositionY: { type: 'int32', value: py },
				};
				const size = annotation.sectionProperties.data.size;
				if (size) {
					positionArgs.Width = { type: 'int32', value: size[0] };
					positionArgs.Height = { type: 'int32', value: size[1] };
				}
			}
			comment = {
				Author: {
					type: 'string',
					value: annotation.sectionProperties.data.author
				},
				// send if html exists, and it's writer send just html, otherwise text
				... (app.map._docLayer._docType === 'text' &&
				     annotation.sectionProperties.data.html) ?
					{ Html: {
						type: 'string',
						value: annotation.sectionProperties.data.html
					} } :
					{ Text: {
						type: 'string',
						value: annotation.sectionProperties.data.text
					} },
				...positionArgs
			};
			var unoCommand = annotation.sectionProperties.data.threaded
				? '.uno:InsertThreadedComment' : '.uno:InsertAnnotation';
			if (app.file.fileBasedView) {
				this.map.setPart(app.map._docLayer._selectedPart, false);
				this.map.sendUnoCommand(unoCommand, comment, true /* force */);
				this.map.setPart(0, false);
			}
			else {
				this.map.sendUnoCommand(unoCommand, comment, true /* force */);
			}

			// Object is later removed in onACKComment when newly inserted comment object is available
			// It's to reduce the flicker when using comment autosave
			if (!CommentSection.autoSavedComment)
				this.removeItem(annotation.sectionProperties.data.id);
		} else if (annotation.sectionProperties.data.trackchange) {
			comment = {
				ChangeTrackingId: {
					type: 'long',
					value: annotation.sectionProperties.data.index
				},
				Text: {
					type: 'string',
					value: annotation.sectionProperties.data.text
				}
			};
			this.map.sendUnoCommand('.uno:CommentChangeTracking', comment, true /* force */);
		} else {
			comment = {
				Id: {
					type: 'string',
					value: annotation.sectionProperties.data.id
				},
				Author: {
					type: 'string',
					value: annotation.sectionProperties.data.author
				},
				// send if html exists, and it's writer send just html, otherwise text
				... (app.map._docLayer._docType === 'text' &&
				     annotation.sectionProperties.data.html) ?
					{ Html: {
						type: 'string',
						value: annotation.sectionProperties.data.html
					} } :
					{ Text: {
						type: 'string',
						value: annotation.sectionProperties.data.text
					} }
			};
			this.map.sendUnoCommand('.uno:EditAnnotation', comment, true /* force */);
		}
		this.unselect();
		this.removeCommentAttention(annotation);
		this.map.focus();
	}

	public reply (annotation: any): void {
		if (cool.Comment.isAnyEdit()) {
			this.navigateAndFocusComment(cool.Comment.isAnyEdit());
			return;
		}
		if ((<any>window).mode.isSmallScreenDevice()) {
			var avatar = undefined;
			var author = this.map.getViewName(app.map._docLayer._viewId);
			if (author in this.map._viewInfoByUserName) {
				avatar = this.map._viewInfoByUserName[author].userextrainfo.avatar;
			}

			if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') {
				this.newAnnotationMobile(annotation, annotation.onReplyClick, /* isMod */ false);
			}
			else {
				var replyAnnotation = {
					text: '',
					textrange: '',
					author: author,
					dateTime: new Date().toDateString(),
					id: annotation.sectionProperties.data.id,
					avatar: avatar,
					parent: annotation.sectionProperties.data.parent,
					anchorPos: [annotation.sectionProperties.data.anchorPos[0], annotation.sectionProperties.data.anchorPos[1]],
				};

				const name = cool.Comment.makeName(replyAnnotation) + '-reply';
				var replyAnnotationSection = new cool.Comment(name, replyAnnotation, replyAnnotation.id === 'new' ? {noMenu: true} : {}, this);

				this.newAnnotationMobile(replyAnnotationSection, annotation.onReplyClick, /* isMod */ false);
			}
		}
		else {
			annotation.reply();
			this.select(annotation, true);
			annotation.focus();
		}
	}

	public modify (annotation: any): void {
		if (cool.Comment.isAnyEdit()) {
			this.navigateAndFocusComment(cool.Comment.isAnyEdit());
			return;
		}
		if ((<any>window).mode.isSmallScreenDevice()) {
			this.newAnnotationMobile(annotation, function(annotation: any) {
				this.save(annotation);
			}.bind(this), /* isMod */ true);
		}
		else {
			// Make sure that comment is not transitioning and comment menu is not open.
			var tempFunction = function() {
				setTimeout(function() {
					if (annotation.sectionProperties.container && annotation.sectionProperties.contextMenu === true
					) {
						tempFunction();
					}
					else {
						annotation.edit();
						this.select(annotation, true);
						annotation.focus();
					}
				}.bind(this), 1);
			}.bind(this);
			tempFunction();
		}
	}

	private showCollapsedReplies(rootIndex: number) {
		if (!this.sectionProperties.commentList.length)
			return;
		var lastIndex = this.getLastChildIndexOf(this.sectionProperties.commentList[rootIndex].sectionProperties.data.id);
		var rootComment = this.sectionProperties.commentList[rootIndex];

		while (rootIndex <= lastIndex) {
			this.sectionProperties.commentList[rootIndex].sectionProperties.container.style.display = '';
			this.sectionProperties.commentList[rootIndex].sectionProperties.container.style.visibility = '';
			$(this.sectionProperties.commentList[rootIndex].sectionProperties.container).removeClass('collapsed-comment');
			rootIndex++;
		}
		rootComment.updateThreadInfoIndicator();
	}

	private collapseReplies(rootIndex: number, rootId: number) {
		var lastChild = this.getLastChildIndexOf(rootId);

		$(this.sectionProperties.commentList[rootIndex].sectionProperties.container).addClass('collapsed-comment');
		for (var i = lastChild; i > rootIndex; i--) {
			this.sectionProperties.commentList[i].sectionProperties.container.style.display = 'none';
			$(this.sectionProperties.commentList[i].sectionProperties.container).addClass('collapsed-comment');
		}
		this.sectionProperties.commentList[i].updateThreadInfoIndicator();
	}

	private cssToCorePixels(cssPixels: number) {
		return cssPixels * app.dpiScale;
	}

	public select (annotation: Comment, force: boolean = false): void {
		if (force
			|| (annotation && !annotation.pendingInit && annotation !== this.sectionProperties.selectedComment
			&& (annotation.sectionProperties.data.resolved !== 'true' || this.sectionProperties.showResolved)
			)) {
			// Select the root comment
			var idx = this.getRootIndexOf(annotation.sectionProperties.data.id);

			// no need to reselect comment, it will cause to scroll to root comment unnecessarily
			if (this.sectionProperties.selectedComment === this.sectionProperties.commentList[idx]) {
				this.update();
				return;
			}

			// Unselect first if there anything selected
			if (this.sectionProperties.selectedComment)
				this.unselect();

			this.sectionProperties.selectedComment = this.sectionProperties.commentList[idx];

			if (this.sectionProperties.selectedComment && !$(this.sectionProperties.selectedComment.sectionProperties.container).hasClass('annotation-active')) {
				$(this.sectionProperties.selectedComment.sectionProperties.container).addClass('annotation-active');
			}

			if (app.map._docLayer._docType === 'text' && this.sectionProperties.showSelectedBigger) {
				this.setThreadPopup(this.sectionProperties.selectedComment, true);
			}

			this.scrollCommentIntoView(annotation);

			const selectedComment = this.sectionProperties.selectedComment;
			if (this.isCollapsed) {
				this.showCollapsedReplies(idx);
				selectedComment.updateThreadInfoIndicator();
			}

			if (app.map._docLayer._docType !== 'spreadsheet') {
				this.sectionProperties.selectedComment.setContainerPos(true, this.sectionProperties.canvasContainerBounds);
			}

			this.update();
		}
	}

	// Focus the edit icon of the given comment
	private focusEditIcon(comment: Comment): void {
		var container = comment.sectionProperties.container;
		app.layoutingService.appendLayoutingTask(() => {
			var editIcon = container?.querySelector(
				'.cool-annotation-menu-edit',
			) as HTMLElement;
			if (editIcon) editIcon.focus();
		});
	}

	private scrollCommentIntoView (comment: Comment) {
		if (CommentSection.importingComments || !comment)
			return;

		const rootComment = this.sectionProperties.commentList[this.getRootIndexOf(comment.sectionProperties?.data.id)];

		if (!rootComment.sectionProperties.data.anchorSPoint) {
			const anchorPos = rootComment.sectionProperties.data.anchorPos;
			rootComment.sectionProperties.data.anchorSPoint = new cool.SimplePoint(anchorPos[0], anchorPos[1]);
		}

		const anchorPos = rootComment.sectionProperties.data.anchorSPoint;

		let topTwips: number = anchorPos.toArray()[1];
		if (app.map._docLayer._docType === 'spreadsheet' && rootComment.sectionProperties.data.id !== 'new') {
			// anchorPos is in display twips but
			// app.isYVisibleInTheDisplayedArea() expects print-twips.
			topTwips = (app.map._docLayer.sheetGeometry as cool.SheetGeometry)
				.getPrintTwipsPointFromTile(new cool.Point(0, topTwips)).y;
		}
		const topVisible = app.isYVisibleInTheDisplayedArea(topTwips);
		const commentHeight = this.cssToCorePixels(rootComment.getCommentHeight());
		const bottomVisible = app.isYVisibleInTheDisplayedArea(
			topTwips + Math.round(commentHeight * app.pixelsToTwips)
		);

		if (!topVisible || !bottomVisible) {
			const topPixels = topTwips * app.twipsToPixels;
			const topBottom = this.getScreenTopBottom();
			const viewHeight = topBottom[1] - topBottom[0];
			const needsScrollDown = topPixels > topBottom[0];
			// Have a margin of 5% of view height, to avoid edge effects
			// (e.g. when top of comment aligned with anchor will not fit to view)
			let scrollPos = topPixels - viewHeight * 0.05;
			if (needsScrollDown) {
				// Only consider when the comment can fit into view
				const bottomViewPos = topPixels + commentHeight - viewHeight * 0.95;
				if (bottomViewPos < scrollPos)
					scrollPos = bottomViewPos;
			}
			app.activeDocument.activeLayout.scrollTo(0, scrollPos);

			if (app.map._docLayer._docType === 'spreadsheet' && rootComment) {
				rootComment.positionCalcComment();
				rootComment.focus();
			}
		}
	}

	/// returns canvas top and bottom position in core pixels
	public getScreenTopBottom(): Array<number> {
		const screenTop = app.activeDocument.activeLayout.viewedRectangle.pY1;
		const screenBottom = screenTop + this.cssToCorePixels($('#map').height());

		return [screenTop, screenBottom];
	}

	public unselect (): void {
		if (this.sectionProperties.selectedComment && this.sectionProperties.selectedComment.sectionProperties.data.id != 'new') {
			for (const comment of this.sectionProperties.commentList) {
				if (window.L.DomUtil.hasClass(comment.sectionProperties.container, 'annotation-active'))
					window.L.DomUtil.removeClass(comment.sectionProperties.container, 'annotation-active');
			}

			if (app.map._docLayer._docType === 'spreadsheet')
				this.sectionProperties.selectedComment.hide();

			if (this.sectionProperties.commentsAreListed && this.isCollapsed) {
				this.sectionProperties.selectedComment.setCollapsed();
				this.collapseReplies(this.getRootIndexOf(this.sectionProperties.selectedComment.sectionProperties.data.id), this.sectionProperties.selectedComment.sectionProperties.data.id);
			}
			if (app.map._docLayer._docType === 'text' && this.sectionProperties.showSelectedBigger) {
				this.setThreadPopup(this.sectionProperties.selectedComment, false);
				this.sectionProperties.showSelectedBigger = false;
			}
			
			const previouslySelectedComment = this.sectionProperties.selectedComment;
			this.sectionProperties.selectedComment = null;
			if (app.map._docLayer._docType !== 'spreadsheet') {
				previouslySelectedComment.setContainerPos(true, this.sectionProperties.canvasContainerBounds); // Must be done after we clear the selection since as it resets the z-index based on this...
			}

			this.update();
		}
	}


	// Escape selected comment and also comment in full view mode.
	private escapeSelectedComment() {
		this.escapeListener = (e: KeyboardEvent) => {
			if (e.key !== 'Escape') return;
			this.unselect();
			this.map.focus();
		};

		document.addEventListener('keydown', this.escapeListener);
	}

	private setThreadPopup (comment: Comment, popup: boolean) {
		if (popup && !$(comment.sectionProperties.container).hasClass('annotation-pop-up'))
			$(comment.sectionProperties.container).addClass('annotation-pop-up');
		else if (!popup && $(comment.sectionProperties.container).hasClass('annotation-pop-up'))
			$(comment.sectionProperties.container).removeClass('annotation-pop-up');

		for (const childComment of comment.sectionProperties.children) {
			this.setThreadPopup(childComment, popup);
		}
	}

	public toggleShowBigger (comment: Comment) {
		const rootComment = this.sectionProperties.commentList[this.getRootIndexOf(comment.sectionProperties.data.id)];
		const isSelected = this.sectionProperties.selectedComment === rootComment;
		if (this.sectionProperties.showSelectedBigger && isSelected) {
			this.sectionProperties.showSelectedBigger = false;
			this.setThreadPopup(this.sectionProperties.selectedComment, false);
		}
		else if (!isSelected) {
			if (this.sectionProperties.selectedComment)
				this.unselect();
			this.sectionProperties.showSelectedBigger = true;
			this.select(comment);
		}
		else {
			this.sectionProperties.showSelectedBigger = true;
			this.setThreadPopup(rootComment, true);
			this.scrollCommentIntoView(comment);
		}
		this.update();
	}

	public showInNavigator(comment: Comment) {
		if (app.map.getDocType() === 'text' || app.map.getDocType() === 'spreadsheet') {
			app.dispatcher.dispatch('showcommentsnavigator', comment.sectionProperties.data.id);
		}
	}

	public saveReply (annotation: any): void {
		const comment = {
			Id: {
				type: 'string',
				value: annotation.sectionProperties.data.id
			},
			// send if html exists, and it's writer send just html, otherwise text
			... (app.map._docLayer._docType === 'text' &&
			     annotation.sectionProperties.data.html) ?
				{ Html: {
					type: 'string',
					value: annotation.sectionProperties.data.html
				} } :
				{ Text: {
					type: 'string',
					value: annotation.sectionProperties.data.reply
				} }
		};

		if (app.map._docLayer._docType === 'text' || app.map._docLayer._docType === 'spreadsheet')
			this.map.sendUnoCommand('.uno:ReplyComment', comment);
		else if (app.map._docLayer._docType === 'presentation')
			this.map.sendUnoCommand('.uno:ReplyToAnnotation', comment);

		this.unselect();
		this.map.focus();
	}

	public cancel (annotation: any): void {
		this.removeCommentAttention(annotation);

		if (annotation.sectionProperties.data.id === 'new') {
			this.removeItem(annotation.sectionProperties.data.id);
		}
		if (this.sectionProperties.selectedComment === annotation) {
			this.unselect();
		} else {
			this.update();
		}
		this.map.focus();
	}

	public onRedlineAccept (e: any): void {
		const command = {
			AcceptTrackedChange: {
				type: 'unsigned short',
				value: e.id.substring('change-'.length)
			}
		};
		this.map.sendUnoCommand('.uno:AcceptTrackedChange', command);
		this.unselect();
		this.map.focus();
	}

	public onRedlineReject (e: any): void {
		const command = {
			RejectTrackedChange: {
				type: 'unsigned short',
				value: e.id.substring('change-'.length)
			}
		};
		this.map.sendUnoCommand('.uno:RejectTrackedChange', command);
		this.unselect();
		this.map.focus();
	}

	public remove (id: any): void {
		const comment = {
			Id: {
				type: 'string',
				value: id
			}
		};

		var removedComment = this.getComment(id);
		if (removedComment) {
			removedComment.sectionProperties.selfRemoved = true;
		}
		if (app.file.fileBasedView) // We have to set the part from which the comment will be removed as selected part before the process.
			this.map.setPart(app.map._docLayer._selectedPart, false);

		if (app.map._docLayer._docType === 'text')
			this.map.sendUnoCommand('.uno:DeleteComment', comment);
		else if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing')
			this.map.sendUnoCommand('.uno:DeleteAnnotation', comment);
		else if (app.map._docLayer._docType === 'spreadsheet')
			this.map.sendUnoCommand('.uno:DeleteNote', comment);

		if (app.file.fileBasedView)
			this.map.setPart(0, false);

		this.unselect();
		this.map.focus();
	}

	public removeThread (id: any): void {
		const comment = {
			Id: {
				type: 'string',
				value: id
			}
		};
		this.map.sendUnoCommand('.uno:DeleteCommentThread', comment);
		this.unselect();
		this.map.focus();
	}

	public resolve (annotation: any): void {
		const comment = {
			Id: {
				type: 'string',
				value: annotation.sectionProperties.data.id
			}
		};
		this.map.sendUnoCommand('.uno:ResolveComment', comment);
	}

	public resolveThread (annotation: any): void {
		const comment = {
			Id: {
				type: 'string',
				value: annotation.sectionProperties.data.id
			}
		};
		this.map.sendUnoCommand('.uno:ResolveCommentThread', comment);
	}

	public promote(annotation: any): void {
		const comment = {
			Id: {
				type: 'string',
				value: annotation.sectionProperties.data.id
			}
		};
		this.map.sendUnoCommand('.uno:PromoteComment', comment);
	}

	public getIndexOf (id: any): number {
		const index = this.idIndexMap.get(id);
		return (index === undefined) ? -1 : index;
	}

	private isSubThreadResolved(annotation: Comment): boolean {
		if (annotation.sectionProperties.data.resolved !== 'true')
			return false;
		for (var i = 0; i < annotation.sectionProperties.children.length; i++) {
			if (!this.isSubThreadResolved(annotation.sectionProperties.children[i]))
				return false;
		}
		return true;
	}

	public isThreadResolved(annotation: Comment): boolean {
		if (annotation.isRootComment())
			return this.isSubThreadResolved(annotation);
		const index = this.getRootIndexOf(annotation.sectionProperties.data.parent);
		const top_comment = this.sectionProperties.commentList[index];
		return this.isSubThreadResolved(top_comment);
	}

	public isShownBig (annotation: any): boolean {
		return this.sectionProperties.showSelectedBigger
			&& this.sectionProperties.selectedComment === this.sectionProperties.commentList[this.getRootIndexOf(annotation.sectionProperties.data.id)];
	}

	public onResize (): void {
		this.checkCollapseState();
		// When window is resized, it may mean that comment wizard is closed. So we hide the highlights.
		this.removeHighlighters();
		this.update();
	}

	public onNewDocumentTopLeft (): void {
		if (app.map._docLayer._docType === 'spreadsheet') {
			if (this.sectionProperties.selectedComment && !this.sectionProperties.doNotHideCommentTimer)
				this.sectionProperties.selectedComment.hide();
		}

		this._commentPositionDirty = true;
	}

	public onDraw (frameCount?: number, elapsedTime?: number): void {
		if (this._commentPositionDirty) {
			this._commentPositionDirty = false;
			var previousAnimationState = this.disableLayoutAnimation;
			this.disableLayoutAnimation = true;
			this.update(false);
			this.disableLayoutAnimation = previousAnimationState;
		}
	}

	private showHideComments (): void {
		for (var i: number = 0; i < this.sectionProperties.commentList.length; i++) {
			this.showHideComment(this.sectionProperties.commentList[i]);
		}
	}

	public showHideComment (annotation: Comment): void {
		// This manually shows/hides comments
		if (!this.sectionProperties.showResolved && app.map._docLayer._docType === 'text') {
			let hide = annotation.isContainerVisible() && annotation.sectionProperties.data.resolved === 'true';
			hide = hide || (CommentSection.isMultiColumnLayout() && this.calculateAvailableSpace() < this.sectionProperties.collapsedCommentWidth);

			if (hide && annotation.isContainerVisible()) {
				if (this.sectionProperties.selectedComment == annotation) {
					this.unselect();
				}
				annotation.hide();
				annotation.update();
			}
			else if (!hide && !annotation.isContainerVisible() && annotation.sectionProperties.data.resolved === 'false') {
				annotation.show();
				annotation.update();
			}
			this.update();
		}
		else if (CommentSection.isMultiColumnLayout()) {
			const hide = this.calculateAvailableSpace() < this.sectionProperties.collapsedCommentWidth;

			if (hide && annotation.isContainerVisible()) {
				annotation.show();
				annotation.update();
			}
			else if (!hide && !annotation.isContainerVisible()) {
				annotation.show();
				annotation.update();
			}
		}
		else if (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') {
			if (annotation.sectionProperties.partIndex === app.map._docLayer._selectedPart || app.file.fileBasedView) {
				if (!annotation.isContainerVisible()) {
					annotation.show();
					annotation.update();
					this.update();
				}
			}
			else {
				annotation.hide();
				annotation.update();
				this.update();
			}
		}
	}

	public add (comment: any): cool.Comment {
		if (!comment.sectionProperties) {
			const name = cool.Comment.makeName(comment);
			const temp = new cool.Comment(name, comment, comment.id === 'new' ? {noMenu: true} : {}, this);
			temp.sectionProperties.data = comment;
			comment = temp;
		}

		comment.sectionProperties.noMenu  = comment.sectionProperties.data.id === 'new' ? true : false;

		/*
			Remove if a comment with the same id exists.
			When user deletes a parent and a child of that parent and undoes the operation respectively:
				* The first undo: Core side sends the deleted child - this is fine.
				* The second undo: Core side sends parent and child together - which is not fine. We already had the child with the first undo command.
			So, delete if a comment already exists and trust core side about the ids of the comments.
		*/
		if (this.containerObject.doesSectionExist(comment.name))
			this.removeItem(comment.name);

		this.containerObject.addSection(comment);
		this.sectionProperties.commentList.push(comment);

		this.adjustParentAdd(comment);
		this.orderCommentList(); // Also updates the index map.
		this.checkSize();

		if (this.isCollapsed && comment.sectionProperties.data.id !== 'new')
			comment.setCollapsed();
		else
		comment.setExpanded();

		// check if we are the author
		// then select it so it does not get lost in a long list of comments and replies.
		const authorName = this.map.getViewName(app.map._docLayer._viewId);
		const newComment = comment.sectionProperties.data.id === 'new';
		if (!newComment && (authorName === comment.sectionProperties.data.author)) {
			this.select(comment);
		}

		app.map.fire('insertannotation');
		return comment;
	}

	public adjustRedLine (redline: any): boolean {
		// All sane values ?
		if (!redline.textRange) {
			console.warn('Redline received has invalid textRange');
			return false;
		}

		// transform change tracking index into an id
		redline.id = 'change-' + redline.index;
		redline.parent = '0'; // Redlines don't have parents, we need to specify this for consistency.
		redline.anchorPos = this.stringToRectangles(redline.textRange)[0];
		redline.anchorSPoint = new cool.SimplePoint(redline.anchorPos[0], redline.anchorPos[1]);
		redline.trackchange = true;
		redline.text = redline.comment;
		redline.rectanglesOriginal = this.stringToRectangles(redline.textRange || redline.anchorPos || redline.rectangle); // This unmodified version will be kept for re-calculations.

		return true;
	}

	public getComment (id: any): any {
		const index = this.getIndexOf(id);
		return index == -1 ? null : this.sectionProperties.commentList[index];
	}

	private checkIfCommentHasPreAssignedChildren(comment: CommentSection) {
		for (var i = 0; i < this.sectionProperties.commentList.length; i++) {
			var possibleChild: Comment = this.sectionProperties.commentList[i];
			if (possibleChild.sectionProperties.possibleParentCommentId !== null) {
				if (possibleChild.sectionProperties.possibleParentCommentId === comment.sectionProperties.data.id) {
					if (!comment.sectionProperties.children.includes(possibleChild))
						comment.sectionProperties.children.push(possibleChild);
				}
			}
		}
	}

	// Adjust parent-child relationship, if required, after `comment` is added
	public adjustParentAdd (comment: any): void {
		if (comment.sectionProperties.data.parent === undefined)
			comment.sectionProperties.data.parent = '0';

		if (comment.sectionProperties.data.parent !== '0') {
			var parentIdx = this.getIndexOf(comment.sectionProperties.data.parent);
			if (parentIdx === -1) {
				console.warn('adjustParentAdd: No parent comment to attach received comment to. ' +
					'Parent comment ID sought is :' + comment.sectionProperties.data.parent + ' for current comment with ID : ' + comment.sectionProperties.data.id);
				comment.sectionProperties.possibleParentCommentId = comment.sectionProperties.data.parent; // Save the proposed parentId so we can remember if such parent appears.
				comment.setAsRootComment(); // Set this to default since there is no such parent at the moment.
			}
			else {
				var parentComment = this.sectionProperties.commentList[parentIdx];
				if (parentComment && !parentComment.sectionProperties.children.includes(comment))
					parentComment.sectionProperties.children.unshift(comment);
			}
		}

		// Check if any of the child comments targets the newly added comment as parent.
		this.checkIfCommentHasPreAssignedChildren(comment);
	}

	// Adjust parent-child relationship, if required, after `comment` is removed
	public adjustParentRemove (comment: any): void {
		var parentIdx = this.getIndexOf(comment.getParentCommentId());

		// If a child comment is removed.
		var parentComment = this.sectionProperties.commentList[parentIdx];
		if (parentComment) {
			var index = parentComment.getIndexOfChild(comment);
			if (index >= 0)
				parentComment.removeChildByIndex(index); // Removed comment has a parent. Remove the comment also from its parent's list.
		}

		// If a parent comment is removed.
		for (var i = 0; i < comment.getChildrenLength(); i++) { // Loop over removed comment's children.
			var childComment = comment.getChildByIndex(i);
			if (childComment)
				childComment.setAsRootComment(); // The children have no parent comment any more.
		}
	}

	public overWriteCommentChanges(obj: any, editComment: Comment) {
		this.clearAutoSaveStatus();
		editComment.onCancelClick(null);
		this.onACKComment(obj);
	}

	public handleCommentConflict(obj: any, editComment: Comment) {
		if (document.getElementById(this.map.uiManager.generateModalId('comments-update')))
			return;

		if (obj.comment.action === 'Remove' || obj.comment.action === 'RedlinedDeletion') {
			JSDialog.showInfoModalWithOptions(
				'comments-update', {
				'title':_('Comments Updated'),
				'messages': [_('Another user has removed this comment.')],
				'buttons': [{'text': _('OK'),
				'callback': () => {
					this.overWriteCommentChanges(obj, editComment);
				}}],
				'withCancel': false}
			);
			return;
		}

		this.map.uiManager.showYesNoButton(
			'comments-update',
			_('Comments Updated'),
			_('Another user has updated the comment. Would you like to overwrite those changes?'),
			_('Overwrite'),
			_('Update'),
			null,
			() => {
				this.overWriteCommentChanges(obj, editComment);
			}, false
		);
	}

	public checkIfOnlyAnchorPosChanged(obj: any, editComment: Comment): boolean {
		if (obj.comment.action !== 'Modify')
			return false;

		var newComment = obj.comment;
		var editCommentData = editComment.sectionProperties.data;

		if (newComment.author !== editCommentData.author
		|| newComment.dateTime !== editCommentData.dateTime
		|| newComment.html !== editCommentData.html
		|| newComment.layoutStatus !== editCommentData.layoutStatus.toString()
		|| newComment.parentId !== editCommentData.parentId
		|| newComment.resolved !== editCommentData.resolved
		|| newComment.textRange !== editCommentData.textRange)
			return false;

		if (newComment.anchorPos.replaceAll(" ", '') !== editCommentData.anchorPos.toString())
			return true;
		return false;
	}

	private actionPerformedByCurrentUser(obj: any): boolean {
		return obj.comment.author === this.map._viewInfo[this.map._docLayer._viewId].username;
	}

	public onACKComment (obj: any): void {
		var id;
		const anyEdit = Comment.isAnyEdit();
		if (anyEdit
			&& !this.checkIfOnlyAnchorPosChanged(obj, anyEdit)
			&& !anyEdit.sectionProperties.selfRemoved
			&& anyEdit.sectionProperties.data.id === obj.comment.id
			&& CommentSection.autoSavedComment !== anyEdit
			&& !this.actionPerformedByCurrentUser(obj)) {
			this.handleCommentConflict(obj, anyEdit);
			return;
		}
		var changetrack = obj.redline ? true : false;
		var dataroot = changetrack ? 'redline' : 'comment';
		if (changetrack) {
			obj.redline.id = 'change-' + obj.redline.index;
		}
		var action = changetrack ? obj.redline.action : obj.comment.action;

		if (action === 'Focus') {
			var comment = this.getComment(obj[dataroot].id);
			if (comment)
				this.focusEditIcon(comment);
			return;
		}

		if (!changetrack && obj.comment.parent === undefined) {
			if (obj.comment.parentId)
				obj.comment.parent = String(obj.comment.parentId);
			else
				obj.comment.parent = '0';
		}

		if (changetrack && obj.redline.author in this.map._viewInfoByUserName) {
			obj.redline.avatar = this.map._viewInfoByUserName[obj.redline.author].userextrainfo.avatar;
		}
		else if (!changetrack && obj.comment.author in this.map._viewInfoByUserName) {
			obj.comment.avatar = this.map._viewInfoByUserName[obj.comment.author].userextrainfo.avatar;
		}

		if ((<any>window).mode.isSmallScreenDevice()) {
			var annotation = this.sectionProperties.commentList[this.getRootIndexOf(obj[dataroot].id)];
			if (!annotation)
				annotation = this.sectionProperties.commentList[this.getRootIndexOf(obj[dataroot].parent)]; //this is required for reload after reply in writer
		}
		if (action === 'Add') {
			if (changetrack) {
				if (!this.adjustRedLine(obj.redline)) {
					// something wrong in this redline
					return;
				}
				this.add(obj.redline);
			} else {
				const currentComment = this.getComment(obj[dataroot].id);
				if (currentComment !== null) {
					if (obj[dataroot].layoutStatus !== undefined) {
						currentComment.sectionProperties.data.layoutStatus = parseInt(obj[dataroot].layoutStatus);
						currentComment.setLayoutClass();
					}
					return;
				}

				// Synchronize calcMasterList
				if (app.map._docLayer._docType === 'spreadsheet' && obj.comment.id !== 'new')
					this.sectionProperties.calcMasterList.push(structuredClone(obj.comment));

				this.adjustComment(obj.comment);
				annotation = this.add(obj.comment);
				if (app.map._docLayer._docType === 'spreadsheet')
					annotation.hide();

				var autoSavedComment = CommentSection.autoSavedComment;
				if (autoSavedComment) {
					var isOurComment = annotation.isAutoSaved();
					if (isOurComment) {
						if (app.definitions.CommentSection.needFocus) {
							app.definitions.CommentSection.needFocus = annotation;
						}
						annotation.sectionProperties.container.style.visibility = 'visible';
						annotation.focusLost();
						if (app.map._docLayer._docType === 'spreadsheet')
							annotation.show();
						if (autoSavedComment.sectionProperties.data.id === 'new')
							this.removeItem(autoSavedComment.sectionProperties.data.id);
						annotation.edit();
						if(this.shouldCollapse())
							annotation.setCollapsed();
						CommentSection.autoSavedComment = null;
						CommentSection.commentWasAutoAdded = true;
					}
				}
			}
			if (this.sectionProperties.selectedComment && !this.sectionProperties.selectedComment.isEdit()) {
				this.map.focus();
			}
		} else if (action === 'Remove') {
			id = obj[dataroot].id;
			var removed = this.getComment(id);
			if (removed) {
				this.adjustParentRemove(removed);
				if (this.sectionProperties.selectedComment === removed) {
					this.unselect();
					this.removeItem(id);
				}
				else {
					this.removeItem(id);
					this.update();
				}
			}
		} else if (action === 'RedlinedDeletion') {
			id = obj[dataroot].id;
			var _redlined = this.getComment(id);
			if (_redlined && _redlined.sectionProperties.data.layoutStatus === CommentLayoutStatus.INSERTED) {
				// Do normal removal if comment was added while recording was on
				// No need to keep the deleted comment
				obj[dataroot].action = 'Remove';
				this.onACKComment(obj);
				return;
			}
			if (_redlined) {
				_redlined.sectionProperties.data.layoutStatus = CommentLayoutStatus.DELETED;
				_redlined.setLayoutClass();
			}
		} else if (action === 'Modify') {
			id = obj[dataroot].id;
			var modified = this.getComment(id);
			if (modified) {
				var modifiedObj;
				if (changetrack) {
					if (!this.adjustRedLine(obj.redline)) {
						// something wrong in this redline
						return;
					}
					modifiedObj = obj.redline;
				} else {
					this.adjustComment(obj.comment);
					modifiedObj = obj.comment;
				}
				const oldParent = modified.getParentCommentId();
				modified.setData(modifiedObj);
				modified.update();
				if (oldParent !== null && modified.isRootComment()) {
					const parentIdx = this.getIndexOf(oldParent);
					const parentComment = this.sectionProperties.commentList[parentIdx];
					if (parentComment) {
						const index = parentComment.getIndexOfChild(modified);
						if (index >= 0)
							parentComment.removeChildByIndex(index);
					}
				}
				this.update();

				if (CommentSection.autoSavedComment) {
					modified.focusLost();
					if (app.map._docLayer._docType === 'spreadsheet')
						modified.show();
					modified.edit();
					if(this.shouldCollapse())
						modified.setCollapsed();
				}
			}
		} else if (action === 'Resolve') {
			id = obj[dataroot].id;
			var resolved = this.getComment(id);
			if (resolved) {
				var resolvedObj;
				if (changetrack) {
					if (!this.adjustRedLine(obj.redline)) {
						// something wrong in this redline
						return;
					}
					resolvedObj = obj.redline;
				} else {
					this.adjustComment(obj.comment);
					resolvedObj = obj.comment;
				}
				resolved.setData(resolvedObj);
				resolved.update();
				this.showHideComment(resolved);
				this.update();
			}
		}
		else if (action === 'SearchHighlight') {
			id = obj[dataroot].id;
			const comment = this.getComment(id);
			if (comment) {
				const selection = obj[dataroot].searchSelection.split(",");
				comment.selectText(parseInt(selection[0]), parseInt(selection[1]), parseInt(selection[2]), parseInt(selection[3]));
				this.showHideComment(comment);
			}
		}
		if ((<any>window).mode.isSmallScreenDevice()) {
			var shouldOpenWizard = false;
			var wePerformedAction = obj.comment.author === this.map.getViewName(app.map._docLayer._viewId);

			if ((<any>window).commentWizard || (action === 'Add' && wePerformedAction))
				shouldOpenWizard = true;

			if (shouldOpenWizard) {
				app.map._docLayer._openCommentWizard(annotation);
			}
		}

		if (app.map._docLayer._docType === 'text') {
			this.updateThreadInfoIndicator();
		}

		if (CommentSection.pendingImport) {
			app.socket.sendMessage('commandvalues command=.uno:ViewAnnotations');
			CommentSection.pendingImport = false;
		}
	}

	public selectById (commentId: any): void {
		const idx = this.getRootIndexOf(commentId);
		const annotation = this.sectionProperties.commentList[idx];
		this.select(annotation);
	}

	public stringToRectangles (str: string): number[][] {
		const strString = typeof str !== 'string' ? String(str) : str;
		const matches = strString.match(/\d+/g);
		const rectangles: number[][] = [];
		if (matches !== null) {
			for (var i: number = 0; i < matches.length; i += 4) {
				rectangles.push([parseInt(matches[i]), parseInt(matches[i + 1]), parseInt(matches[i + 2]), parseInt(matches[i + 3])]);
			}
		}
		return rectangles;
	}

	public onPartChange (): void {
		for (var i: number = 0; i < this.sectionProperties.commentList.length; i++) {
			this.showHideComment(this.sectionProperties.commentList[i]);
		}
		// Honor the doNotHideCommentTimer guard: when Action_GoToComment has
		// just navigated to a comment on another part, a status-msg-triggered
		// 'updateparts' can fire right after the select() and would otherwise
		// cancel the freshly-selected comment.
		if (this.sectionProperties.selectedComment && !this.sectionProperties.doNotHideCommentTimer)
			this.sectionProperties.selectedComment.onCancelClick(null);

		this.checkSize();
	}

	// In file based view, we need to move comments to their part's position.
	// Because all parts are drawn on the screen. Core side doesn't have this feature.
	// Core side sends the information in part coordinates.
	// When a coordinate like [0, 0] is inside 2nd part for example, that coordinate should correspond to a value like (just guessing) [0, 45646].
	// See that y value is different. Because there is 1st part above the 2nd one in the view.
	// We will add their part's position to comment's variables.
	// When we are saving their position, we will remove the additions before sending the information.
	private adjustCommentFileBasedView (comment: any): void {
		// Below calculations are the same with the ones we do while drawing tiles in fileBasedView.
		var partHeightTwips = app.map._docLayer._partHeightTwips + app.map._docLayer._spaceBetweenParts;
		var index = app.impress.getIndexFromSlideHash(parseInt(comment.parthash));
		var yAddition = index * partHeightTwips;
		comment.yAddition = yAddition; // We'll use this while we save the new position of the comment.

		comment.trackchange = false;

		comment.rectangles = this.stringToRectangles(comment.textRange || comment.anchorPos || comment.rectangle); // Simple array of point arrays [x1, y1, x2, y2].
		comment.rectangles[0][1] += yAddition; // There is only one rectangle for our case.

		comment.rectanglesOriginal = this.stringToRectangles(comment.textRange || comment.anchorPos || comment.rectangle); // This unmodified version will be kept for re-calculations.
		comment.rectanglesOriginal[0][1] += yAddition;

		comment.anchorPos = this.stringToRectangles(comment.anchorPos || comment.rectangle)[0];
		comment.anchorPos[1] += yAddition;

		if (comment.rectangle) {
			comment.rectangle = this.stringToRectangles(comment.rectangle)[0]; // This is the position of the marker.
			comment.rectangle[1] += yAddition;
		}

		comment.anchorSPoint = new cool.SimplePoint(comment.anchorPos[0], comment.anchorPos[1]);

		comment.parthash = comment.parthash ? comment.parthash: null;

		if (comment.parentId)
			comment.parent = String(comment.parentId);

		var viewId = this.map.getViewId(comment.author);
		var color = viewId >= 0 ? app.LOUtil.rgbToHex(this.map.getViewColor(viewId)) : '#43ACE8';
		comment.color = color;
	}

	// Normally, a comment's position information is the same with the desktop version.
	// So we can use it directly.
	private adjustCommentNormal (comment: any): void {
		comment.trackchange = false;

		if (comment.cellRange) {
			// turn cell range string into cell bounds
			comment.cellRange = app.map._docLayer._parseCellRange(comment.cellRange);
		}

		const cellPos = comment.cellRange ? app.map._docLayer._cellRangeToTwipRect(comment.cellRange).toRectangle() : null;
		const rectangles = this.stringToRectangles(comment.textRange || comment.anchorPos || comment.rectangle); // Simple array of point arrays [x1, y1, x2, y2].
		if (rectangles.length === 0 && cellPos?.length) {
			rectangles.push(cellPos);
		}
		console.assert(rectangles.length, 'Found no rectangles in comment!');
		comment.rectangles = rectangles;
		comment.rectanglesOriginal = structuredClone(rectangles);
		comment.anchorPos = rectangles[0];
		comment.anchorSPoint = new cool.SimplePoint(comment.anchorPos[0], comment.anchorPos[1]);

		if (app.map._docLayer._docType === 'spreadsheet' && app.map._docLayer.sheetGeometry)
			app.map._docLayer.sheetGeometry.convertToTileTwips(comment.anchorSPoint);

		comment.parthash = comment.parthash ? comment.parthash: null;
		comment.tab = (comment.tab || comment.tab === 0) ? comment.tab: null;
		comment.layoutStatus = comment.layoutStatus !== undefined ? parseInt(comment.layoutStatus): null;

		if (comment.parentId)
			comment.parent = String(comment.parentId);

		if (comment.rectangle) {
			comment.rectangle = this.stringToRectangles(comment.rectangle)[0]; // This is the position of the marker (Impress & Draw).
		}

		var viewId = this.map.getViewId(comment.author);
		var color = viewId >= 0 ? app.LOUtil.rgbToHex(this.map.getViewColor(viewId)) : '#43ACE8';
		comment.color = color;
	}

	private adjustComment (comment: any): void {
		if (!app.file.fileBasedView)
			this.adjustCommentNormal(comment);
		else
			this.adjustCommentFileBasedView(comment);
	}

	// Returns the last comment id of comment thread containing the given id
	private getLastChildIndexOf (id: any): number {
		var index = this.getIndexOf(id);
		index = this.getRootIndexOf(this.sectionProperties.commentList[index].sectionProperties.data.id);

		while
		(
			this.sectionProperties.commentList[index + 1] &&
			index + 1 < this.sectionProperties.commentList.length &&
			this.sectionProperties.commentList[index + 1].sectionProperties.data.parent !== '0'
		) {
			index++;
		}

		return index;
	}

	// If the file type is presentation or drawing then we shall check the selected part in order to hide comments from other parts.
	// But if file is in fileBasedView, then we will not hide any comments from not-selected/viewed parts.
	private mustCheckSelectedPart (): boolean {
		return (app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing') && !app.file.fileBasedView;
	}

	private layoutUp (subList: Array<Comment>, actualPosition: Array<number>, lastY: number, relayout: boolean = true): number {
		var height: number;
		for (var i = 0; i < subList.length; i++) {
			height = subList[i].getCommentHeight(relayout);
			lastY = subList[i].sectionProperties.data.anchorSPoint.vY + height < lastY ? subList[i].sectionProperties.data.anchorSPoint.vY: lastY - (height * app.dpiScale);

			subList[i].setContainerPos(
				false,
				this.sectionProperties.canvasContainerBounds,
				actualPosition[0] / app.dpiScale,
				lastY / app.dpiScale
			);

			if (this.sectionProperties.show != false && !subList[i].isEdit())
				subList[i].show();
		}
		return lastY;
	}

	private loopUp (startIndex: number, x: number, startY: number, relayout: boolean = true): number {
		var tmpIdx = 0;
		var checkSelectedPart: boolean = this.mustCheckSelectedPart();
		startY -= this.sectionProperties.marginY;
		// Pass over all comments present
		for (var i = startIndex; i > -1;) {
			var subList = [];
			tmpIdx = i;
			do {
				// Add this item to the list of comments.
				if (this.sectionProperties.commentList[tmpIdx].sectionProperties.data.resolved !== 'true' || this.sectionProperties.showResolved) {
					if (!checkSelectedPart || app.map._docLayer._selectedPart === this.sectionProperties.commentList[tmpIdx].sectionProperties.partIndex)
						subList.push(this.sectionProperties.commentList[tmpIdx]);
				}
				tmpIdx = tmpIdx - 1;
				// Continue this loop, until we reach the last item, or an item which is not a direct descendant of the previous item.
			} while (tmpIdx > -1 && this.sectionProperties.commentList[tmpIdx].sectionProperties.data.parent === this.sectionProperties.commentList[tmpIdx + 1].sectionProperties.data.id);

			if (subList.length > 0) {
				if (!subList[0].sectionProperties.data.anchorSPoint)
					subList[0].sectionProperties.data.anchorSPoint = new cool.SimplePoint(subList[0].sectionProperties.data.anchorPos[0], subList[0].sectionProperties.data.anchorPos[1]);

				startY = this.layoutUp(subList, [x, subList[0].sectionProperties.data.anchorSPoint.vY], startY, relayout);
				i = i - subList.length;
			} else {
				i = tmpIdx;
			}
			startY -= this.sectionProperties.marginY;
		}
		return startY;
	}

	private layoutDown (subList: any, actualPosition: Array<number>, lastY: number, relayout: boolean = true): number {
		const selectedComment = subList[0] === this.sectionProperties.selectedComment;
		for (var i = 0; i < subList.length; i++) {
			lastY = subList[i].sectionProperties.data.anchorSPoint.vY > lastY ? subList[i].sectionProperties.data.anchorSPoint.vY: lastY;

			var isRTL = document.documentElement.dir === 'rtl';

			if (selectedComment) {
				const commentWidth = (this.sectionProperties.showSelectedBigger ? this.sectionProperties.commentWidthBigger: this.sectionProperties.commentWidth) / app.dpiScale;
				const documentCanvasWidth = parseInt((document.getElementById('document-canvas') as any).style.width);
				let posX = (this.sectionProperties.showSelectedBigger ?
								Math.round((documentCanvasWidth - commentWidth)/2) :
								Math.round(actualPosition[0] / app.dpiScale) - this.sectionProperties.deflectionOfSelectedComment * (isRTL ? -1 : 1));
				// if on selection full comment is not visible bring it fully inside view, helps in narrow windows and tablets
				if (isRTL && posX < 0)
					posX = 0;
				else if (posX + commentWidth > documentCanvasWidth)
				{
					posX = documentCanvasWidth - commentWidth;
				}

				subList[i].setContainerPos(false, this.sectionProperties.canvasContainerBounds, posX, lastY / app.dpiScale);
			}
			else {
				subList[i].setContainerPos(false, this.sectionProperties.canvasContainerBounds, actualPosition[0] / app.dpiScale, lastY / app.dpiScale);
			}

			lastY += (subList[i].getCommentHeight(relayout) * app.dpiScale);
			if (this.sectionProperties.show != false && !subList[i].isEdit())
				subList[i].show();
		}
		return lastY;
	}

	private loopDown (startIndex: number, x: number, startY: number, relayout: boolean = true): number {
		var tmpIdx = 0;
		var checkSelectedPart: boolean = this.mustCheckSelectedPart();
		// Pass over all comments present
		for (var i = startIndex; i < this.sectionProperties.commentList.length;) {
			var subList = [];
			tmpIdx = i;
			do {
				// Add this item to the list of comments.
				if (this.sectionProperties.commentList[tmpIdx].sectionProperties.data.resolved !== 'true' || this.sectionProperties.showResolved) {
					if (!checkSelectedPart || app.map._docLayer._selectedPart === this.sectionProperties.commentList[tmpIdx].sectionProperties.partIndex)
						subList.push(this.sectionProperties.commentList[tmpIdx]);
				}
				tmpIdx = tmpIdx + 1;
				// Continue this loop, until we reach the last item, or an item which is not a direct descendant of the previous item.
			} while (tmpIdx < this.sectionProperties.commentList.length && this.sectionProperties.commentList[tmpIdx].sectionProperties.data.parent !== '0');

			if (subList.length > 0) {
				if (!subList[0].sectionProperties.data.anchorSPoint)
					subList[0].sectionProperties.data.anchorSPoint = new cool.SimplePoint(subList[0].sectionProperties.data.anchorPos[0], subList[0].sectionProperties.data.anchorPos[1]);

				startY = this.layoutDown(subList, [x, subList[0].sectionProperties.data.anchorSPoint.vY], startY, relayout);
				i = i + subList.length;
			} else {
				i = tmpIdx;
			}
			startY += this.sectionProperties.marginY;
		}
		return startY;
	}

	public hideArrow (): void {
		if (this.sectionProperties.arrow) {
			document.getElementById('document-container').removeChild(this.sectionProperties.arrow);
			this.sectionProperties.arrow = null;
			app.sectionContainer.requestReDraw();
		}
	}

	private showArrow (startPoint: Array<number>, endPoint: Array<number>): void {
		startPoint[0] = Math.floor(startPoint[0] / app.dpiScale);
		startPoint[1] = Math.floor(startPoint[1] / app.dpiScale);
		endPoint[0] = Math.floor(endPoint[0] / app.dpiScale);
		endPoint[1] = Math.floor(endPoint[1] / app.dpiScale);

		if (this.sectionProperties.arrow !== null) {
			var line: SVGLineElement = <SVGLineElement>(this.sectionProperties.arrow.firstElementChild);
			line.setAttribute('x1', String(startPoint[0]));
			line.setAttribute('y1', String(startPoint[1]));
			line.setAttribute('x2', String(endPoint[0]));
			line.setAttribute('y2', String(endPoint[1]));
		}
		else {
			var svg: SVGElement = (<any>document.createElementNS('http://www.w3.org/2000/svg', 'svg'));
			svg.setAttribute('version', '1.1');
			svg.style.zIndex = '12';
			svg.style.pointerEvents = 'none';
			svg.id = 'comment-arrow-container';
			svg.style.position = 'absolute';
			svg.style.top = svg.style.left = svg.style.right = svg.style.bottom = '0';
			svg.setAttribute('width', String(this.context.canvas.width));
			svg.setAttribute('height', String(this.context.canvas.height));
			var line  = document.createElementNS('http://www.w3.org/2000/svg','line');
			line.id = 'comment-arrow-line';
			line.setAttribute('x1', String(startPoint[0]));
			line.setAttribute('y1', String(startPoint[1]));
			line.setAttribute('x2', String(endPoint[0]));
			line.setAttribute('y2', String(endPoint[1]));
			line.setAttribute('stroke', 'darkblue');
			line.setAttribute('stroke-width', '1');
			svg.appendChild(line);
			document.getElementById('document-container').appendChild(svg);
			this.sectionProperties.arrow = svg;
		}
	}

	private layout (relayout: boolean = true): void {
		if ((<any>window).mode.isSmallScreenDevice() || app.map._docLayer._docType === 'spreadsheet') {
			if (this.sectionProperties.commentList.length > 0)
				this.orderCommentList();
			return; // No adjustments for Calc, since only one comment can be shown at a time and that comment is shown at its belonging cell.
		}

		this.sectionProperties.canvasContainerBounds = document.getElementById('document-container').getBoundingClientRect();

		const availableSpace = this.calculateAvailableSpace();
		if (!this.commentsHiddenOrNotPresent()) {
			this.orderCommentList();
			if (relayout)
				this.resetCommentsSize();

			var isRTL = document.documentElement.dir === 'rtl';

			var topRight: Array<number> = [this.myTopLeft[0], this.myTopLeft[1] + this.sectionProperties.marginY + (new cool.SimplePoint(0, 0)).vY];
			var yOrigin = 0;
			var selectedIndex = null;
			var x = isRTL ? 0 : topRight[0];

			if (CommentSection.isMultiColumnLayout()) {
				x = topRight[0] - availableSpace;
			}
			else if (isRTL)
				x = availableSpace - this.sectionProperties.commentWidth;
			else {
				x = (app.activeDocument.fileSize.cX - app.activeDocument.activeLayout.viewedRectangle.cX1 - app.sectionContainer.getCanvasBoundingClientRect().x) * app.dpiScale;
				x += app.map.navigator ? app.map.navigator.navigationPanel.offsetWidth * app.dpiScale : 0;
			}

			if (this.sectionProperties.selectedComment) {
				selectedIndex = this.getRootIndexOf(this.sectionProperties.selectedComment.sectionProperties.data.id);

				yOrigin = this.sectionProperties.commentList[selectedIndex].sectionProperties.data.anchorSPoint.vY;
				var tempCrd: Array<number> = this.sectionProperties.commentList[selectedIndex].sectionProperties.data.anchorSPoint.vToArray();
				var resolved:string = this.sectionProperties.commentList[selectedIndex].sectionProperties.data.resolved;
				if (!resolved || resolved === 'false' || this.sectionProperties.showResolved) {
					var posX = isRTL ? (this.containerObject.getDocumentAnchorSection().size[0] + x + 15) : x;
					this.showArrow([tempCrd[0], tempCrd[1]], [posX, tempCrd[1]]);
				}
			}
			else
				this.hideArrow();

			if (relayout)
				this.resizeLastComment();
			var lastY = 0;
			if (selectedIndex) {
				this.loopUp(selectedIndex - 1, x, yOrigin, relayout);
				lastY = this.loopDown(selectedIndex, x, yOrigin, relayout);
			}
			else {
				lastY = this.loopDown(0, x, topRight[1], relayout);
			}
		} else {
			for (const comment of this.sectionProperties.commentList) {
				comment.setContainerPos(false, this.sectionProperties.canvasContainerBounds);
			}
		}
		if (relayout)
			this.resizeComments();

		let horizontalScroll = app.activeDocument.fileSize.x;
		if (availableSpace < this.sectionProperties.commentWidth && !this.isCollapsed)
			horizontalScroll = (app.activeDocument.fileSize.cX + this.sectionProperties.commentWidth) * app.pixelsToTwips * app.dpiScale;

		const checkY = lastY + app.activeDocument.activeLayout.viewedRectangle.pY1;

		if (checkY > app.activeDocument.fileSize.pY) {
			app.activeDocument.activeLayout.viewSize = new cool.SimplePoint(horizontalScroll, checkY * app.pixelsToTwips);
			this.containerObject.requestReDraw();
		}
		else
			app.activeDocument.activeLayout.viewSize = new cool.SimplePoint(horizontalScroll, app.activeDocument.fileSize.y);

		this.disableLayoutAnimation = false;
	}

	private update (reLayout: boolean = true): void {
		this.sectionProperties.reLayout = reLayout;
		this.updateDOM();
	}

	private updateDOM(): void {
		if (!this.sectionProperties.firstImport) return;

		app.layoutingService.appendLayoutingTask(() => {
			if (this.sectionProperties.reLayout && app.map._docLayer._docType === 'text')
				this.updateThreadInfoIndicator();

			this.layout(this.sectionProperties.reLayout);
		});

		app.sectionContainer.requestReDraw();
	}

	private updateThreadInfoIndicator(): void {
		for (var i = 0; i < this.sectionProperties.commentList.length; i++) {
			var comment = this.sectionProperties.commentList[i];
			var replyCount = 0;
			var anyEdit = false;

			if (comment && comment.isRootComment()) {
				var lastIndex = this.getLastChildIndexOf(comment.sectionProperties.data.id);
				var j = i;
				while (this.sectionProperties.commentList[j] && j <= lastIndex) {
					anyEdit = this.sectionProperties.commentList[j].isEdit() || anyEdit;
					if (this.sectionProperties.commentList[j].sectionProperties.data.parent !== '0') {
						if ((this.sectionProperties.commentList[j].sectionProperties.data.layoutStatus !== CommentLayoutStatus.DELETED ||
							this.map['stateChangeHandler'].getItemValue('.uno:ShowTrackedChanges') === 'true') &&
							this.sectionProperties.commentList[j].sectionProperties.data.resolved !== 'true') {
							replyCount++;
						}
					}
					j++;
				}
			}
			if (anyEdit)
				comment.updateThreadInfoIndicator('!');
			else
				comment.updateThreadInfoIndicator(replyCount);
		}
	}

	private updateChildLines () : void {
		for (let i = 0; i < this.sectionProperties.commentList.length; i++) {
			this.sectionProperties.commentList[i].updateChildLines();
		}
	}

	// Returns the root comment index of given id
	private getRootIndexOf (id: any): number {
		var index = this.getIndexOf(id);

		while (index >= 0) {
			if (this.sectionProperties.commentList[index].sectionProperties.data.parent !== '0')
				index--;
			else
				break;
		}

		return index;
	}

	public setViewResolved (state: boolean): void {
		this.sectionProperties.showResolved = state;

		for (var idx = 0; idx < this.sectionProperties.commentList.length;idx++) {
			if (this.sectionProperties.commentList[idx].sectionProperties.data.resolved === 'true') {
				if (state==false) {
					if (this.sectionProperties.selectedComment == this.sectionProperties.commentList[idx]) {
						this.unselect();
					}
					this.sectionProperties.commentList[idx].hide();
				} else {
					this.sectionProperties.commentList[idx].show();
				}
			}
			this.sectionProperties.commentList[idx].update();
		}
		this.update();
	}

	/*
		also consider whether the comment to be shown is a resolved
		comment and if so then check if the `showResolved` toggle
		is on or off.
	*/
	public setView(state: boolean): void {
		this.sectionProperties.show = state;
		const commentShouldCollapse = this.shouldCollapse();

		for (var idx = 0; idx < this.sectionProperties.commentList.length; idx++) {
			if (state == false) {
				this.sectionProperties.commentList[idx].hide();
			} else if (this.sectionProperties.commentList[idx].sectionProperties.data.resolved != 'true' || this.sectionProperties.showResolved == true) {
				this.sectionProperties.commentList[idx].show();
				if (commentShouldCollapse) {
					this.sectionProperties.commentList[idx].setCollapsed();
				}
			}
		}
		this.update();
	}

	private orderCommentList (): void {
		this.sectionProperties.commentList.sort(function(a: any, b: any) {
			return Math.abs(a.sectionProperties.data.anchorPos[1]) - Math.abs(b.sectionProperties.data.anchorPos[1]) ||
				Math.abs(a.sectionProperties.data.anchorPos[0]) - Math.abs(b.sectionProperties.data.anchorPos[0]);
		});

		if (app.map._docLayer._docType === 'text')
			this.orderTextComments();

		// idIndexMap is now invalid, update it.
		this.updateIdIndexMap();
	}

	// reset theis size to default (100px text)
	private resetCommentsSize (): void {
		if (app.map._docLayer._docType === 'text') {
			for (var i = 0; i < this.sectionProperties.commentList.length;i++) {
				if (this.sectionProperties.commentList[i].sectionProperties.contentNode.style.display !== 'none') {
					const maxHeight = (this.sectionProperties.commentList[i] === this.sectionProperties.selectedComment) ?
						this.annotationMaxSize : this.annotationMinSize;
					this.sectionProperties.commentList[i].sectionProperties.contentNode.style.maxHeight = maxHeight + 'px';
				}
			}
		}
	}

	// grow comments size if they have more text, and there is enough space between other comments
	private resizeComments (): void {
		// Change it true, if comments are allowed to grow up direction.
		// Now it is disabled, because without constant indicator of the comments anchor, it can be confusing.
		var growUp = false;
		if (app.map._docLayer._docType === 'text') {
			const minMaxHeight = Number(getComputedStyle(document.documentElement).getPropertyValue('--annotation-min-size'));
			const maxMaxHeight = Number(getComputedStyle(document.documentElement).getPropertyValue('--annotation-max-size'));
			for (var i = 0; i < this.sectionProperties.commentList.length;i++) {
				// Only if ContentNode is displayed.
				if (this.sectionProperties.commentList[i].sectionProperties.contentNode.style.display !== 'none'
				&& !this.sectionProperties.commentList[i].isEdit()) {
					// act commentText height
					var actHeight = this.sectionProperties.commentList[i].sectionProperties.contentText.getBoundingClientRect().height;
					// if the comment is taller then minimal, we may want to make it taller
					if (actHeight > minMaxHeight) {
						// but we don't want to make it taller then the maximum
						if (actHeight > maxMaxHeight) {
							actHeight = maxMaxHeight;
						}

						// check if there is more space after this comment.
						var maxSize = maxMaxHeight;
						if (i + 1 < this.sectionProperties.commentList.length)
							// max size of text should be the space between comments - size of non text parts
							maxSize = this.sectionProperties.commentList[i + 1].getContainerPosY()
								- this.sectionProperties.commentList[i].getContainerPosY()
								- this.sectionProperties.commentList[i].sectionProperties.author.getBoundingClientRect().height
								- 3 * this.sectionProperties.marginY //top/bottom of comment window + space between comments
								- 2; // not sure why

						if (maxSize > maxMaxHeight) {
							maxSize = maxMaxHeight;
						} else if (growUp && actHeight > maxSize) {
							// if more space needed as we have after the comment
							// check it there is any space before the comment
							var spaceBefore = this.sectionProperties.commentList[i].getContainerPosY();
							if (i > 0) {
								spaceBefore -= this.sectionProperties.commentList[i - 1].getContainerPosY()
									+ this.sectionProperties.commentList[i - 1].getCommentHeight()
									+ this.sectionProperties.marginY;
							} else {
								spaceBefore += app.activeDocument.activeLayout.viewedRectangle.pY1;
							}
							// if there is more space
							if (spaceBefore > 0) {
								var moveUp = 0;
								if (actHeight - maxSize < spaceBefore) {
									// there is enough space, move up as much as we can;
									moveUp = actHeight - maxSize;
								} else {
									// there is not enough space
									moveUp = spaceBefore;
								}
								// move up
								const posX = this.sectionProperties.commentList[i].getContainerPosX();
								const posY = this.sectionProperties.commentList[i].getContainerPosY() - moveUp;
								this.sectionProperties.commentList[i].setContainerPos(false, this.sectionProperties.canvasContainerBounds, posX, posY);
								// increase comment height
								maxSize += moveUp;
							}
						}
						if (maxSize > minMaxHeight)
							this.sectionProperties.commentList[i].sectionProperties.contentNode.style.maxHeight = Math.round(maxSize) + 'px';
					}
				}
			}
			this.updateChildLines();
		}
	}

	private resizeLastComment (): void {
		if (app.map._docLayer._docType === 'text' && !this.commentsHiddenOrNotPresent()) {
			const minMaxHeight = Number(getComputedStyle(document.documentElement).getPropertyValue('--annotation-min-size'));
			const maxMaxHeight = Number(getComputedStyle(document.documentElement).getPropertyValue('--annotation-max-size'));
			//last comment
			var i = this.sectionProperties.commentList.length-1;
			// Only if ContentNode is displayed.
			if (this.sectionProperties.commentList[i].sectionProperties.contentNode.style.display !== 'none'
				&& !this.sectionProperties.commentList[i].isEdit()) {
				// act commentText height
				var actHeight = this.sectionProperties.commentList[i].sectionProperties.contentText.getBoundingClientRect().height;
				// if the comment is taller then minimal, we may want to make it taller
				if (actHeight > minMaxHeight) {
				// but we don't want to make it taller then the maximum
					if (actHeight > maxMaxHeight) {
						actHeight = maxMaxHeight;
					}
					this.sectionProperties.commentList[i].sectionProperties.contentNode.style.maxHeight = Math.round(actHeight) + 'px';
				}
			}
		}
	}

	private updateIdIndexMap(): void {
		this.idIndexMap.clear();
		const commentList = this.sectionProperties.commentList;
		for (var idx = 0; idx < commentList.length; idx++) {
			const comment = commentList[idx];
			console.assert(comment.sectionProperties && comment.sectionProperties.data, 'no sectionProperties.data!');
			this.idIndexMap.set(comment.sectionProperties.data.id, idx);
		}
	}

	private turnIntoAList (commentList: any): any[] {
		var newArray;
		if (!Array.isArray(commentList)) {
			newArray = new Array(0);
			for (var prop in commentList) {
				if (Object.prototype.hasOwnProperty.call(commentList, prop)) {
					newArray.push(commentList[prop]);
				}
			}
		}
		else {
			newArray = commentList;
		}
		return newArray;
	}

	private addUpdateChildGroups() {
		var parentCommentList: Array<any> = [];
		for (var i = 0; i < this.sectionProperties.commentList.length; i++) {
			var comment = this.sectionProperties.commentList[i];
			comment.sectionProperties.children = [];
			if (comment.sectionProperties.data.parent !== '0')
			{
				if (!parentCommentList.includes(comment.sectionProperties.data.parent))
					parentCommentList.push(comment.sectionProperties.data.parent);
			}
		}

		for (var i = 0; i < parentCommentList.length; i++) {
			var parentComment;
			for (var j = 0; j < this.sectionProperties.commentList.length; j++) {
				if (this.sectionProperties.commentList[j].sectionProperties.data.id === parentCommentList[i]) {
					parentComment = this.sectionProperties.commentList[j];
					break;
				}
			}

			if (parentComment) {
				for (var j = 0; j < this.sectionProperties.commentList.length; j++) {
					if (this.sectionProperties.commentList[j].sectionProperties.data.parent === parentCommentList[i])
						parentComment.sectionProperties.children.push(this.sectionProperties.commentList[j]);
				}
			}
			else
				console.warn('Couldn\'t find parent comment.');
		}
	}

	private addChildrenCommentsToList(comment: any, newOrder: Array<any>) {
		comment.sectionProperties.children.forEach(function(element: any) {
			newOrder.push(element);
			if (element.sectionProperties.children.length > 0)
				this.addChildrenCommentsToList(element, newOrder);
		}.bind(this));
	}

	private orderTextComments() {
		var newOrder = [];

		for (var i = 0; i < this.sectionProperties.commentList.length; i++) {
			var comment = this.sectionProperties.commentList[i];

			if (comment.isRootComment()) {
				newOrder.push(comment);

				if (comment.sectionProperties.children.length > 0)
					this.addChildrenCommentsToList(comment, newOrder);
			}
		}

		this.sectionProperties.commentList = newOrder;
	}

	// Needed in the case of a document with lots of comments(30K)
	// filters out comments that don't below to current tab.
	// Doing this at this outer level makes a big difference.
	private spreadSheetFilteredComments(newCommentList?: any[]): any[] {
		const out: any[] = [];
		if (newCommentList) {
			this.sectionProperties.calcMasterList = newCommentList;
		}
		for (let i = 0; i < this.sectionProperties.calcMasterList.length; i++) {
			const comment = this.sectionProperties.calcMasterList[i];
			if (comment.tab == app.map._docLayer._selectedPart) {
				out.push(structuredClone(comment));
			}
		}

		this.sectionProperties.calcLastTab = app.map._docLayer._selectedPart;

		return out;
	}

	// Needed for calc because of spreadSheetFilteredComments().
	// Gets called every updateparts event.
	private importPartSpecificComments(): void {
		if (app.map._docLayer._docType !== 'spreadsheet') {
			return;
		}

		if (this.sectionProperties.calcLastTab == app.map._docLayer._selectedPart) {
			// To ignore updateparts events without actual change in sheet.
			return;
		}

		this.sectionProperties.calcCommandStateChanged = false;

		this.importComments();
	}

	public importComments (commentList?: any): void {
		this.disableLayoutAnimation = true;
		var comment;
		if (Comment.isAnyEdit()) {
			CommentSection.pendingImport = true;
			return;
		}

		// If a navigation (e.g. Action_GoToComment) just selected a comment
		// and set the doNotHideCommentTimer guard, remember the id so we can
		// re-select the rebuilt comment at the end of the import. Without
		// this, a status-msg-triggered annotations refresh arriving right
		// after the navigation would silently drop the selection.
		const preserveSelectedId = this.sectionProperties.doNotHideCommentTimer
				&& this.sectionProperties.selectedComment
			? this.sectionProperties.selectedComment.sectionProperties.data.id
			: null;

		CommentSection.importingComments = true;
		let drawPaused = false;
		if (app.map._docLayer._docType === 'spreadsheet') {
			if (!commentList) {
				this.containerObject.pauseDrawing();
				drawPaused = true;
				commentList = this.spreadSheetFilteredComments();
			} else {
				commentList = this.turnIntoAList(commentList);
				commentList = this.spreadSheetFilteredComments(commentList);
			}
		} else {
			// Non calc document commentList must never be undefined or null.
			console.assert(commentList);
			commentList = this.turnIntoAList(commentList);
		}
		this.clearList();

		if (commentList.length > 0) {
			for (var i = 0; i < commentList.length; i++) {
				comment = commentList[i];

				this.adjustComment(comment);
				if (comment.author in this.map._viewInfoByUserName) {
					comment.avatar = this.map._viewInfoByUserName[comment.author].userextrainfo.avatar;
				}
				const name = cool.Comment.makeName(comment);
				var commentSection = new cool.Comment(name, comment, {}, this);
				if (!this.containerObject.addSection(commentSection))
					continue;
				this.sectionProperties.commentList.push(commentSection);
				this.idIndexMap.set(commentSection.sectionProperties.data.id, i);
			}

			if (app.map._docLayer._docType === 'text')
				this.addUpdateChildGroups();

			this.orderCommentList();
			this.checkSize();
			this.update();
		}

		const show = this.map.stateChangeHandler.getItemValue('showannotations');
		this.setView(show === true || show === 'true');

		const showResolved = this.map.stateChangeHandler.getItemValue('ShowResolvedAnnotations');
		this.setViewResolved(showResolved === true || showResolved === 'true');

		if (app.map._docLayer._docType === 'spreadsheet')
			this.hideAllComments(); // Apply drawing orders.

		if ((app.map._docLayer._docType === 'presentation' || app.map._docLayer._docType === 'drawing'))
			this.showHideComments();

		this.sectionProperties.reLayout = true;
		this.sectionProperties.firstImport = true;
		this.updateDOM();
		CommentSection.importingComments = false;
		if (drawPaused) {
			this.containerObject.resumeDrawing();
		}
		if (preserveSelectedId !== null)
			this.selectById(preserveSelectedId);
	}

	// Accepts redlines/changes comments.
	public importChanges (changesList: any): void {
		var changeComment;
		this.clearChanges();
		changesList = this.turnIntoAList(changesList);

		if (changesList.length > 0) {
			for (let i = 0; i < changesList.length; i++) {
				changeComment = changesList[i];
				if (!this.adjustRedLine(changeComment))
					// something wrong in this redline, skip this one
					continue;

				if (changeComment.author in this.map._viewInfoByUserName) {
					changeComment.avatar = this.map._viewInfoByUserName[changeComment.author].userextrainfo.avatar;
				}
				const name = cool.Comment.makeName(changeComment);
				const commentSection = new cool.Comment(name, changeComment, {}, this);
				if (!this.containerObject.addSection(commentSection))
					continue;
				this.sectionProperties.commentList.push(commentSection);
			}

			this.orderCommentList();
			this.checkSize();
			this.update();
		}

		if (app.map._docLayer._docType === 'spreadsheet')
			this.hideAllComments(); // Apply drawing orders.
	}

	// Remove redline comments.
	private clearChanges(): void {
		this.containerObject.pauseDrawing();
		for (var i: number = this.sectionProperties.commentList.length -1; i > -1; i--) {
			if (this.sectionProperties.commentList[i].sectionProperties.data.trackchange) {
				this.containerObject.removeSection(this.sectionProperties.commentList[i].name);
				this.sectionProperties.commentList.splice(i, 1);
			}
		}
		this.updateIdIndexMap();
		this.containerObject.resumeDrawing();

		this.sectionProperties.selectedComment = null;
		this.checkSize();
	}

	private clearAutoSaveStatus () {
		CommentSection.autoSavedComment = null;
		CommentSection.commentWasAutoAdded = false;
	}

	// Remove only text comments from the document (excluding change tracking comments)
	private clearList (): void {
		this.containerObject.pauseDrawing();
		for (var i: number = this.sectionProperties.commentList.length -1; i > -1; i--) {
			if (!this.sectionProperties.commentList[i].sectionProperties.data.trackchange) {
				this.containerObject.removeSection(this.sectionProperties.commentList[i].name);
				this.sectionProperties.commentList.splice(i, 1);
			}
		}
		this.updateIdIndexMap();
		this.containerObject.resumeDrawing();

		this.sectionProperties.selectedComment = null;
		this.checkSize();
		this.clearAutoSaveStatus();
	}

	public onCommentsDataUpdate(): void {
		for (var i: number = this.sectionProperties.commentList.length -1; i > -1; i--) {
			const comment = this.sectionProperties.commentList[i];
			if (!comment.valid && comment.sectionProperties.data.id !== 'new') {
				comment.sectionProperties.commentListSection.removeItem(comment.sectionProperties.data.id);
			}
			comment.onCommentDataUpdate();
		}
	}

	public rejectAllTrackedCommentChanges(): void {
		for (var i = 0; i < this.sectionProperties.commentList.length; i++) {
			const comment = this.sectionProperties.commentList[i];
			if (comment.sectionProperties.data.layoutStatus === CommentLayoutStatus.DELETED) {
				comment.sectionProperties.data.layoutStatus = CommentLayoutStatus.VISIBLE;
				comment.sectionProperties.container.classList.remove('tracked-deleted-comment-show');
				comment.updateRemovedField();
			}
		}
	}

	public hasAnyComments(): boolean {
		return this.sectionProperties.commentList.length > 0;
	}
}

}

app.definitions.CommentSection = cool.CommentSection;
