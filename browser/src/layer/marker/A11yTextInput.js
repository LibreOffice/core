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
 * window.L.A11yTextInput is the hidden textarea, which handles text input events
 *
 * This is made significantly more difficult than expected by such a
 * mess of browser, and mobile IME quirks that it is not possible to
 * follow events, but we have to re-construct input from a browser
 * text area itself.
 */

/* global app _ _n cool */

window.L.A11yTextInput = window.L.TextInput.extend({
	initialize: function() {
		window.L.TextInput.prototype.initialize.call(this);

		this._className = 'A11yTextInput';

		// Used for signaling when in a mobile device the user tapped the edit button
		this._justSwitchedToEditMode = false;

		// Used when editing a shape text content.
		this._isEditingInSelection = false;
		this._hasAnySelection = false;


		// In core text selection exists even if it's empty and <backspace> deletes the empty selection
		// instead of the previous character.
		this._hasSelection = false;
		this._lastCursorPosition = 0;
		this._lastSelectionStart = 0;
		this._lastSelectionEnd = 0;
		this._listPrefixLength = 0;
		this._isLeftRightArrow = 0;
		this._pendingFocusedParagraph = null;
		this._lineNavigation = false;

		// pending macOS live region update
		this._a11yLiveRegionUpdate = 0;

		// what this element stands for while it is not a text box
		this._a11yContext = '';

		this._updateA11yEditableStateBound = this._updateA11yEditableState.bind(this);
		this._onStrayKeyDownBound = this._onStrayKeyDown.bind(this);
	},

	onAdd: function() {
		window.L.TextInput.prototype.onAdd.call(this);
		this._initContextRegions();
		// the canvas only exists once the doc layer has been built
		this._map.on('doclayerinit', this._bindCanvasFocusGuard, this);
		this._map.on('doclayerinit', this._updateA11yEditableState, this);
		this._map.on('updateparts', this._onA11yPartChanged, this);
		app.events.on('updatepermission', this._updateA11yEditableStateBound);
		document.addEventListener('keydown', this._onStrayKeyDownBound, true);
		window.L.DomEvent.on(this._textArea, 'click auxclick contextmenu dragstart',
			this._onLinkEvent, this);
	},

	onRemove: function() {
		this._map.off('doclayerinit', this._bindCanvasFocusGuard, this);
		this._map.off('doclayerinit', this._updateA11yEditableState, this);
		this._map.off('updateparts', this._onA11yPartChanged, this);
		app.events.off('updatepermission', this._updateA11yEditableStateBound);
		document.removeEventListener('keydown', this._onStrayKeyDownBound, true);
		window.L.DomEvent.off(this._textArea, 'click auxclick contextmenu dragstart',
			this._onLinkEvent, this);
		var canvas = document.getElementById('document-canvas');
		if (canvas)
			window.L.DomEvent.off(canvas, 'mousedown', this._keepFocusOnCanvasClick, this);
		window.L.TextInput.prototype.onRemove.call(this);
	},

	// What a screen reader says about this element while it is the idle
	_setA11yContext: function(text) {
		this._a11yContext = text;
		if (this._textArea.isContentEditable)
			return;
		this._textArea.setAttribute('role', 'group');
		this._textArea.setAttribute('aria-roledescription', text);
	},

	_slideContext: function() {
		return _('Slide');
	},

	// A part change drops the selection with it, so the context goes back to
	// naming the slide.
	_onA11yPartChanged: function() {
		this._setA11yContext(this._slideContext());
	},

	// macOS only: this hands the announcing over to the assertive live region,
	// which is itself macOS only, since a screen reader reads aria-description
	// as part of announcing a form field and stops doing so once this is a plain div.
	_updateA11yEditableState: function() {
		if (!window.L.Browser.mac)
			return;
		// a read-only document has its own contenteditable handling, see
		// TextInput._onPermission
		if (!this._map.isEditMode())
			return;

		var idleSurface = this._map.getDocType() === 'presentation' && !this._isEditingInSelection;
		// already in the wanted state?
		if (this._textArea.isContentEditable === !idleSurface)
			return;

		var hadFocus = document.activeElement === this._textArea;
		if (idleSurface) {
			// a div that is not editable needs a tabindex to stay focusable, so
			// give it one before taking the contenteditable away
			this._textArea.setAttribute('tabindex', '0');
			this._textArea.setAttribute('contenteditable', 'false');
			this._setA11yContext(this._a11yContext || this._slideContext());
		} else {
			this._textArea.setAttribute('contenteditable', 'true');
			this._textArea.removeAttribute('tabindex');
			// a text box announces itself as one again
			this._textArea.removeAttribute('role');
			this._textArea.removeAttribute('aria-roledescription');
		}
		// dropping contenteditable can drop the focus with it
		if (hadFocus && document.activeElement !== this._textArea)
			this._textArea.focus({ preventScroll: true });
	},

	// a link is there to be read, not followed
	_onLinkEvent: function(ev) {
		const link = ev.target.closest ? ev.target.closest('a') : null;
		if (!link)
			return;
		ev.preventDefault();
		// activating a link around the caret takes the caret to its paragraph
		const paragraph = link.closest('.a11y-context > span');
		if (ev.type === 'click' && paragraph && paragraph.dataset.twips && !this._contextJump)
			this._jumpToContextParagraph(paragraph.dataset.twips);
	},

	_bindCanvasFocusGuard: function() {
		var canvas = document.getElementById('document-canvas');
		if (canvas)
			window.L.DomEvent.on(canvas, 'mousedown', this._keepFocusOnCanvasClick, this);
	},

	_keepFocusOnCanvasClick: function(ev) {
		if (ev.button === 0 && this._map.getDocType() === 'presentation'
			&& document.activeElement === this._textArea)
			ev.preventDefault();
	},

	hasAccessibilitySupport: function() {
		return true;
	},

	setHTML: function(content, links, images, comments) {
		// eslint-disable-next-line no-restricted-syntax -- markup around escaped text
		this._textArea.innerHTML = this._wrapContent(content, links, images, comments);
		this._linkCommentDetails();
	},

	_prependSpace: function() {
		// eslint-disable-next-line no-restricted-syntax -- spacer markup around escaped text
		this._textArea.innerHTML = this._preSpaceChar + this._textArea.innerHTML;
	},

	_appendSpace: function() {
		// eslint-disable-next-line no-restricted-syntax -- spacer markup around escaped text
		this._textArea.innerHTML = this._textArea.innerHTML + this._postSpaceChar;
	},

	_getLastCursorPosition: function() {
		return this._lastCursorPosition;
	},

	_getCaretOffset: function() {
		return this._getTextOffset(this._getLastCursorPosition());
	},

	// the given position in the text, from the editable's top left corner
	_getTextOffset: function(nPos) {
		let offset = Math.min(Math.max(nPos, 0), this.getPlainTextContent().length);
		const walker = document.createTreeWalker(this._textArea, NodeFilter.SHOW_TEXT);
		for (let node = walker.nextNode(); node; node = walker.nextNode()) {
			if (offset <= node.length) {
				const range = document.createRange();
				range.setStart(node, offset);
				const caret = range.getBoundingClientRect();
				if (caret.height === 0)
					return { x: 0, y: 0 }; // not laid out
				const area = this._textArea.getBoundingClientRect();
				// a line break in the paragraph puts the caret below the first line
				return { x: caret.left - area.left, y: caret.top - area.top };
			}
			offset -= node.length;
		}
		return { x: 0, y: 0 };
	},

	_setLastCursorPosition: function(nPos) {
		this._lastCursorPosition = nPos;
		this._lastSelectionStart = this._lastSelectionEnd = nPos;
	},

	_setLastSelection: function(nStart, nEnd) {
		this._lastSelectionStart = nStart;
		this._lastSelectionEnd = this._lastCursorPosition = nEnd;
	},

	_isLastSelectionEmpty: function() {
		return this._lastSelectionStart === this._lastSelectionEnd;
	},

	_isLastSelection: function(start, end) {
		return this._hasSelection &&  this._lastSelectionStart === start && this._lastSelectionEnd === end;
	},

	_updateCursorPosition: function(pos) {
		if (typeof pos !== 'number')
			return;
		// Normalize input parameters
		var l = this.getPlainTextContent().length;
		if (pos < 0)
			pos = 0;
		if (pos > l)
			pos = l;

		this._setLastCursorPosition(pos);
		this._setCursorPosition(pos);
		this.update();
	} ,

	_updateSelection: function(pos, start, end, forced) {
		window.app.console.log('_updateSelection: pos: ' + pos + ', start: ' + start + ', end: ' + end);
		if (typeof pos !== 'number' || typeof start !== 'number' || typeof end !== 'number')
			return;

		var hasSelection= !(start === -1 && end === -1);
		if (!hasSelection) {
			this._updateCursorPosition(pos);
		} else if (forced || !this._isLastSelection(start, end)) {
			if (forced || start !== end || !this._hasSelection || !this._isLastSelectionEmpty()) {
				// When the new selection is empty (start == end). the cursor position is updated
				// only if there was no previous selection, or previous selection was not empty.
				// In fact when both old and new selection are empty, it means that the old selection
				// has been moved to a new position by some typing. Anyway changing cursor position
				// while typing can mess up editable area content.
				this._setLastSelection(start, end);
				this._setSelectionRange(start, end);
				this.update();
			}
		}
		this._setSelectionFlag(hasSelection);
	},

	_setSelectionFlag: function(flag) {
		this._hasSelection = flag;
		if (window.L.Browser.cypressTest)
			this._textArea.isSelectionNull = !flag;
	},

	_setFocusedParagraph: function(content, pos, start, end, links, images, comments) {
		window.app.console.log('_setFocusedParagraph:'
			+ '\n    content "' + content + '"'
			+ '\n    pos: ' + pos
			+ '\n    start: ' + start + ', end: ' + end);

		this._isComposing = false;
		this._isLeftRightArrow = 0;
		if (this._hasFormulaBarFocus()) {
			this._placeContextRegions();
			return;
		}

		const current = this.getPlainTextContent();
		this._pendingFocusedParagraph = null;
		if (window.L.Browser.win || !this._lineNavigation || current === '' || content === '' || content === current) {
			this._fillFocusedParagraph(content, pos, start, end, links, images, comments);
			return;
		}

		// Emptied first so Orca hears the new paragraph whole; NVDA would catch the editable empty.
		const pending = { content: content, pos: pos, start: start, end: end, links: links,
			images: images, comments: comments };
		this._pendingFocusedParagraph = pending;
		this.resetContent();
		requestAnimationFrame(() => requestAnimationFrame(() => {
			if (this._pendingFocusedParagraph === pending)
				this._flushFocusedParagraph();
		}));
	},

	_fillFocusedParagraph: function(content, pos, start, end, links, images, comments) {
		this.setHTML(content, links, images, comments);
		this.updateLastContent();
		this._updateSelection(pos, start, end, true);
		this._placeContextRegions();
	},

	_flushFocusedParagraph: function() {
		const pending = this._pendingFocusedParagraph;
		if (!pending)
			return;
		this._pendingFocusedParagraph = null;
		this._fillFocusedParagraph(pending.content, pending.pos, pending.start, pending.end,
			pending.links, pending.images, pending.comments);
	},

	_onKeyDown: function(ev) {
		this._flushFocusedParagraph();
		this._lineNavigation = ['ArrowUp', 'ArrowDown', 'PageUp', 'PageDown'].includes(ev.key)
			|| ((ev.key === 'Home' || ev.key === 'End') && ev.ctrlKey);
		window.L.TextInput.prototype._onKeyDown.call(this, ev);
	},

	_updateFocusedParagraph: function() {
		this._log('_updateFocusedParagraph');
		if (this._remoteContent !== undefined) {
			this._setFocusedParagraph(this._remoteContent, this._remotePosition,
				this._remoteSelectionStart, this._remoteSelectionEnd, this._remoteLinks,
				this._remoteImages, this._remoteComments);
		} else if (this._remoteSelectionEnd !== undefined) {
			this._updateSelection(this._remotePosition, this._remoteSelectionStart, this._remoteSelectionEnd);
		} else if (this._remotePosition !== undefined) {
			this._updateCursorPosition(this._remotePosition);
		}
		this._remoteContent = undefined;
		this._remoteLinks = undefined;
		this._remoteImages = undefined;
		this._remoteComments = undefined;
		this._remotePosition = undefined;
		this._remoteSelectionStart = undefined;
		this._remoteSelectionEnd = undefined;
	},

	// One description carries every piece, so setting it alone would drop the others.
	_caretDescription: function() {
		const parts = [];
		if (this._headingLevel)
			parts.push(_('Heading level {0}').replace('{0}', this._headingLevel));
		return parts.join('. ');
	},

	_describeCaretState: function() {
		const description = this._caretDescription();
		if (!description && !this._caretDescribed)
			return;

		this._caretDescribed = !!description;
		this._setDescription(description);
	},

	// TextInput empties the description on blur.
	_onFocusBlur: function(ev) {
		if (ev.type === 'blur')
			this._caretDescribed = false;
		else
			this._describeCaretState();
		window.L.TextInput.prototype._onFocusBlur.call(this, ev);
	},

	_setHeadingLevel: function(level) {
		const parsed = parseInt(level) || 0;
		if (parsed === (this._headingLevel || 0))
			return;
		this._headingLevel = parsed;
		this._describeCaretState();
	},

	onAccessibilityFocusChanged: function(content, pos, start, end, listPrefixLength, force, before, after,
		beforeRects, afterRects, headingLevel, beforeLevels, afterLevels, links, beforeLinks,
		afterLinks, images, beforeImages, afterImages, comments, beforeComments, afterComments) {
		this._listPrefixLength = listPrefixLength;
		this._setHeadingLevel(headingLevel);
		this._endContextJump();
		this._keepCommentDetails(comments, beforeComments, afterComments);
		this._setContextParagraphs(before, after, beforeRects, afterRects, beforeLevels,
			afterLevels, beforeLinks, afterLinks, beforeImages, afterImages, beforeComments,
			afterComments);
		this._requestHeadings();
		if (!this.hasFocus() || (this._isComposing && !force)) {
			this._log('onAccessibilityFocusChanged: skipped updating: '
				+ '\n  hasFocus: ' + this.hasFocus()
				+ '\n  _isComposing: ' + this._isComposing
				+ '\n  force: ' + force);
			this._remoteContent = content;
			this._remoteLinks = links;
			this._remoteImages = images;
			this._remoteComments = comments;
			this._remotePosition = pos;
			this._remoteSelectionStart = start;
			this._remoteSelectionEnd = end;
		} else {
			this._setFocusedParagraph(content, pos, start, end, links, images, comments);
		}
	},

	setA11yFocusedParagraph: function(content, pos, start, end, before, after, beforeRects, afterRects, headingLevel,
		beforeLevels, afterLevels, links, beforeLinks, afterLinks, images, beforeImages,
		afterImages, comments, beforeComments, afterComments) {
		this._contextPending = Math.max(0, (this._contextPending || 0) - 1);
		this._setHeadingLevel(headingLevel);
		this._keepCommentDetails(comments, beforeComments, afterComments);
		if (this._isComposing) {
			this._remoteContent = content;
			this._remoteLinks = links;
			this._remoteImages = images;
			this._remoteComments = comments;
			this._remotePosition = pos;
			this._remoteSelectionStart = start;
			this._remoteSelectionEnd = end;
		} else {
			this._setFocusedParagraph(content, pos, start, end, links, images, comments);
		}
		this._setContextParagraphs(before, after, beforeRects, afterRects, beforeLevels,
			afterLevels, beforeLinks, afterLinks, beforeImages, afterImages, beforeComments,
			afterComments);
	},

	// getPlainTextContent() reads the whole editable, and every caret offset is
	// measured against it.
	_initContextRegions: function() {
		if (this._contextBefore || !this._container)
			return;

		const createContextRegion = function (id) {
			const region = document.createElement('div');
			region.id = id;
			region.className = 'a11y-context';
			region.setAttribute('aria-hidden', 'false');
			region.setAttribute('contenteditable', 'false');
			return region;
		};

		this._headingsAbove = createContextRegion('a11y-headings-above');
		this._contextBefore = createContextRegion('a11y-context-before');
		this._contextAfter = createContextRegion('a11y-context-after');
		this._headingsBelow = createContextRegion('a11y-headings-below');
		for (const region of [this._contextBefore, this._contextAfter]) {
			window.L.DomEvent.on(region, 'focusin', this._onContextFocus, this);
			window.L.DomEvent.on(region, 'click auxclick contextmenu dragstart', this._onLinkEvent,
				this);
		}
		this._container.insertBefore(this._headingsAbove, this._textArea);
		this._container.insertBefore(this._contextBefore, this._textArea);
		this._container.insertBefore(this._contextAfter, this._textArea.nextSibling);
		this._container.insertBefore(this._headingsBelow, this._contextAfter.nextSibling);
	},

	_requestHeadings: function() {
		if (this._map.getDocType() !== 'text')
			return;

		this._headingsStale = true;
		clearTimeout(this._headingsRequestTimer);
		this._headingsRequestTimer = setTimeout(function () {
			this._headingsRequestTimer = null;
			this._headingsPending = (this._headingsPending || 0) + 1;
			app.socket.sendMessage('commandvalues command=.uno:Headings');
		}.bind(this), 250);
	},

	// A heading's side is measured against the caret of its request; only the last answer is current.
	setA11yHeadings: function(values) {
		this._headingsPending = Math.max(0, (this._headingsPending || 0) - 1);
		if (this._headingsPending || this._headingsRequestTimer)
			return;
		this._headingsStale = false;
		this._headings = values && Array.isArray(values.headings) ? values.headings : [];
		this._initContextRegions();
		this._fillHeadingRegions();
	},

	// A heading given around the caret, or the one being edited, is left out: the
	// reader would list it twice.
	_fillHeadingRegions: function() {
		if (!this._headingsAbove || !this._headings || this._headingsStale)
			return;

		const toRect = function (twips) {
			return typeof twips === 'string' ? twips.split(',').map(Number) : null;
		};
		const holds = function (rect, x, y) {
			return rect && x >= rect[0] && x <= rect[0] + rect[2] && y >= rect[1] && y <= rect[1] + rect[3];
		};
		const given = Array.from(this._contextBefore.children).concat(Array.from(this._contextAfter.children))
			.map(function (span) { return toRect(span.dataset.twips); }).filter(Boolean);

		const above = [];
		const below = [];
		this._headings.forEach(function (heading) {
			if (heading.side === 'caret')
				return;
			const rect = toRect(heading.rect);
			if (rect && given.some(function (span) { return holds(span, rect[0] + rect[2] / 2, rect[1] + rect[3] / 2); }))
				return;
			(heading.side === 'above' ? above : below).push(heading);
		});

		const key = JSON.stringify([above, below]);
		if (key === this._headingsKey)
			return;
		this._headingsKey = key;

		const fill = function (region, headings) {
			region.replaceChildren();
			headings.forEach(function (heading) {
				const element = document.createElement('div');
				element.setAttribute('role', 'heading');
				element.setAttribute('aria-level', heading.level);
				const link = document.createElement('a');
				link.href = '#';
				link.textContent = heading.text;
				link.dataset.target = heading.target;
				link.dataset.index = heading.index;
				link.addEventListener('click', function (event) {
					event.preventDefault();
					this._jumpToHeading(heading.index, heading.target);
				}.bind(this));
				element.appendChild(link);
				region.appendChild(element);
			}.bind(this));
		}.bind(this);
		const focused = document.activeElement;
		const focusedTarget = focused && (this._headingsAbove.contains(focused) || this._headingsBelow.contains(focused))
			? focused.dataset.target : undefined;
		fill(this._headingsAbove, above);
		fill(this._headingsBelow, below);
		if (focusedTarget !== undefined) {
			const links = Array.from(this._headingsAbove.querySelectorAll('a'))
				.concat(Array.from(this._headingsBelow.querySelectorAll('a')));
			const same = links.find(function (link) { return link.dataset.target === focusedTarget; });
			if (same)
				same.focus();
			else
				this.focus();
		}
		this._placeContextRegions();
	},

	// NVDA puts its browse cursor where the focus lands, so the paragraphs around must be there first.
	// Headings without numbering share their target, so the index tells them apart.
	_jumpToHeading: function(index, target) {
		app.map.sendUnoCommand('.uno:GotoOutlineIndex', {
			Index: { type: 'unsigned long', value: index },
			Name: { type: 'string', value: target },
		});
		clearTimeout(this._headingJump);
		this._headingJump = setTimeout(this._endHeadingJump.bind(this), 1000);
	},

	_endHeadingJump: function() {
		if (!this._headingJump)
			return;
		clearTimeout(this._headingJump);
		this._headingJump = null;
		this.focus();
	},

	update: function() {
		window.L.TextInput.prototype.update.call(this);
		this._placeContextRegions();
	},

	onVisibleAreaChanged: function() {
		if (!this._map || !this._map._docLoaded)
			return;
		this.update();
		if (!this.hasAccessibilitySupport() || !this.hasFocus())
			return;

		clearTimeout(this._contextRequestTimer);
		this._contextRequestTimer = setTimeout(function () {
			this._contextRequestTimer = null;
			this._requestFocusedParagraph();
		}.bind(this), 250);
	},

	_setContextParagraphs: function(before, after, beforeRects, afterRects, beforeLevels,
		afterLevels, beforeLinks, afterLinks, beforeImages, afterImages, beforeComments,
		afterComments) {
		this._initContextRegions();
		if (!this._contextBefore)
			return;

		// The spans are reused: a reader whose position is on one that goes away looks for
		// another nearby and leaves focus mode.
		const fillContextRegion = (region, paragraphs, rects, levels, links, images, comments) => {
			const texts = Array.isArray(paragraphs) ? paragraphs : [];
			while (region.children.length > texts.length)
				region.lastElementChild.remove();
			texts.forEach((text, index) => {
				const level = Array.isArray(levels) ? parseInt(levels[index]) || 0 : 0;
				const role = level > 0 ? 'heading' : 'paragraph';
				let span = region.children[index];
				// Orca keeps the level a node had when it first met it.
				if (span && span.getAttribute('role') !== role) {
					const fresh = document.createElement('span');
					span.replaceWith(fresh);
					span = fresh;
				}
				if (!span) {
					span = document.createElement('span');
					region.appendChild(span);
				}
				// NVDA and JAWS focus it on leaving browse mode; Orca would focus every one it reads.
				if (window.L.Browser.win)
					span.tabIndex = -1;
				else
					span.removeAttribute('tabindex');
				const spanLinks = Array.isArray(links) && Array.isArray(links[index])
					? links[index] : [];
				const linksKey = JSON.stringify(spanLinks);
				const spanImages = Array.isArray(images) && Array.isArray(images[index])
					? images[index] : [];
				const imagesKey = JSON.stringify(spanImages);
				const spanComments = Array.isArray(comments) && Array.isArray(comments[index])
					? comments[index] : [];
				const commentsKey = JSON.stringify(spanComments);
				if (span.textContent !== text || (span.dataset.links || '[]') !== linksKey
					|| (span.dataset.images || '[]') !== imagesKey
					|| (span.dataset.comments || '[]') !== commentsKey) {
					span.replaceChildren(...this._createLinkedNodes(text, spanLinks, spanImages,
						spanComments));
					// a reader still reaches them; Tab does not
					for (const link of span.querySelectorAll('a'))
						link.tabIndex = -1;
					if (spanLinks.length)
						span.dataset.links = linksKey;
					else
						delete span.dataset.links;
					if (spanImages.length)
						span.dataset.images = imagesKey;
					else
						delete span.dataset.images;
					if (spanComments.length)
						span.dataset.comments = commentsKey;
					else
						delete span.dataset.comments;
				}
				if (span.getAttribute('role') !== role)
					span.setAttribute('role', role);
				if (level > 0 && span.getAttribute('aria-level') !== String(level))
					span.setAttribute('aria-level', level);
				else if (level === 0 && span.hasAttribute('aria-level'))
					span.removeAttribute('aria-level');
				if (Array.isArray(rects) && typeof rects[index] === 'string')
					span.dataset.twips = rects[index];
				else
					delete span.dataset.twips;
			});
		};

		fillContextRegion(this._contextBefore, before, beforeRects, beforeLevels, beforeLinks,
			beforeImages, beforeComments);
		fillContextRegion(this._contextAfter, after, afterRects, afterLevels, afterLinks,
			afterImages, afterComments);
		this._linkCommentDetails();
		this._fillHeadingRegions();
		this._placeContextRegions();
		this._endHeadingJump();
	},

	// the marks shown keep their details until the paragraphs given with these replace them
	_keepCommentDetails: function(comments, beforeComments, afterComments) {
		const previous = this._commentDetails || new Map();
		this._commentDetails = new Map();
		const keep = (id) => {
			if (previous.has(id))
				this._commentDetails.set(id, previous.get(id));
		};
		if (this._container) {
			for (const mark of this._container.querySelectorAll('[data-comment-id]'))
				keep(mark.dataset.commentId);
		}
		// a paragraph not shown yet
		const pending = this._pendingFocusedParagraph ? this._pendingFocusedParagraph.comments : null;
		for (const list of [pending, this._remoteComments]) {
			for (const comment of Array.isArray(list) ? list : [])
				keep(String(comment.id));
		}
		const add = (list) => {
			for (const comment of Array.isArray(list) ? list : [])
				this._commentDetails.set(String(comment.id), comment);
		};
		add(comments);
		for (const paragraphs of [beforeComments, afterComments]) {
			for (const list of Array.isArray(paragraphs) ? paragraphs : [])
				add(list);
		}
	},

	// aria-details gives the author and the text of a comment on demand (NVDA+D), and not inline at
	// each mark. NVDA reads only the first of several targets, where the comment panel has
	// the author and the text in two nodes, and a target hidden in any way drops the link; so the
	// target is a copy of both off the screen.
	_linkCommentDetails: function() {
		if (!this._container)
			return;
		const copies = new Map();
		for (const mark of this._container.querySelectorAll('[data-comment-id]')) {
			const id = mark.dataset.commentId;
			if (!this._commentDetails || !this._commentDetails.has(id)) {
				mark.removeAttribute('aria-details');
				continue;
			}
			const details = 'a11y-comment-' + id;
			copies.set(details, this._commentDetails.get(id));
			if (mark.getAttribute('aria-details') !== details)
				mark.setAttribute('aria-details', details);
		}
		this._fillCommentCopies(copies);
	},

	_fillCommentCopies: function(copies) {
		// not the positions, which change on typing
		const key = JSON.stringify(Array.from(copies, ([id, comment]) => [id, comment.author,
			comment.text]));
		if (key === this._commentCopiesKey)
			return;
		this._commentCopiesKey = key;
		if (!this._commentCopies) {
			this._commentCopies = document.createElement('div');
			this._commentCopies.className = 'visuallyhidden';
			this._container.appendChild(this._commentCopies);
		}
		this._commentCopies.replaceChildren();
		for (const [id, comment] of copies) {
			const copy = document.createElement('div');
			copy.id = id;
			copy.setAttribute('role', 'comment');
			const author = document.createElement('div');
			author.textContent = comment.author;
			const text = document.createElement('div');
			text.textContent = comment.text;
			copy.append(author, text);
			this._commentCopies.appendChild(copy);
		}
	},

	_onContextFocus: function(ev) {
		// off Windows only a link can take the focus, and Orca focuses each one it reads
		if (!window.L.Browser.win)
			return;
		// NVDA focuses a link before activating it, and a jump now would replace the link
		if (ev.target.closest && ev.target.closest('a'))
			return;
		const paragraph = ev.target.closest ? ev.target.closest('.a11y-context > span') : null;
		if (paragraph && paragraph.dataset.twips)
			this._jumpToContextParagraph(paragraph.dataset.twips);
	},

	// Orca leaves the focus on the outline link, or on the page with the selection on the
	// paragraph, it was reading when it enters focus mode, so the first key decides where the
	// caret goes.
	_onStrayKeyDown: function(ev) {
		if (!this._contextBefore)
			return;
		if (['Tab', 'Shift', 'Control', 'Alt', 'Meta', 'Insert', 'CapsLock'].includes(ev.key))
			return;
		const active = document.activeElement;
		if (ev.key !== 'Enter' && active && active.dataset && active.dataset.target
			&& (this._headingsAbove.contains(active) || this._headingsBelow.contains(active))) {
			window.L.DomEvent.stop(ev);
			this._jumpToHeading(parseInt(active.dataset.index), active.dataset.target);
			return;
		}
		const inContext = active
			&& (this._contextBefore.contains(active) || this._contextAfter.contains(active));
		if (active !== document.body && !inContext)
			return;
		const anchor = window.getSelection() ? window.getSelection().anchorNode : null;
		const element = anchor && anchor.nodeType === Node.TEXT_NODE ? anchor.parentNode : anchor;
		let paragraph = element && element.closest ? element.closest('.a11y-context > span') : null;
		if (!paragraph && inContext) // the link NVDA left the focus on
			paragraph = active.closest('.a11y-context > span');
		if (!paragraph || !paragraph.dataset.twips)
			return;

		window.L.DomEvent.stop(ev);
		let target = paragraph;
		if (ev.key === 'ArrowDown' || ev.key === 'ArrowUp') {
			const down = ev.key === 'ArrowDown';
			const sibling = down ? paragraph.nextElementSibling : paragraph.previousElementSibling;
			if (sibling)
				target = sibling;
			else if ((paragraph.parentNode === this._contextBefore) === down) {
				this._textArea.focus({ preventScroll: true });
				return;
			}
		}
		this._jumpToContextParagraph(target.dataset.twips);
	},

	_jumpToContextParagraph: function(twips) {
		if (!this._map._docLayer)
			return;

		this._contextJump = setTimeout(this._endContextJump.bind(this), 1000);
		const rect = twips.split(',').map(Number);
		const x = rect[0] + 1;
		const y = rect[1] + Math.min(rect[3] / 2, 120);
		this._map._docLayer._postMouseEvent('buttondown', x, y, 1, app.LOButtons.left, 0);
		this._map._docLayer._postMouseEvent('buttonup', x, y, 1, app.LOButtons.left, 0);
	},

	// Given back once the caret is there, and before the regions are refilled: the focused
	// paragraph going away with them makes the reader announce the page again.
	_endContextJump: function() {
		if (!this._contextJump)
			return;
		clearTimeout(this._contextJump);
		this._contextJump = null;
		this._textArea.focus({ preventScroll: true });
	},

	// The editable is 1px tall and its text overflows below it: a region drawn over that
	// text is read by a screen reader as part of the same line.
	_placeContextRegions: function() {
		if (!this._container || !this._contextBefore)
			return;

		const textArea = this._textArea;
		const regions = Array.from(this._container.children).filter(function (child) {
			return child.classList.contains('a11y-context');
		});
		const isBefore = function (region) {
			return !!(region.compareDocumentPosition(textArea) & Node.DOCUMENT_POSITION_FOLLOWING);
		};

		// Each paragraph goes down to where the document draws it, but not over another one.
		// Only Writer sends the rectangles in document coordinates.
		const isText = this._map.getDocType() === 'text';
		const containerRect = this._container.getBoundingClientRect();
		const canvasRect = app.sectionContainer.getCanvasBoundingClientRect();
		const getDocumentRect = function (span) {
			if (!isText || !span.dataset.twips)
				return null;
			const twips = span.dataset.twips.split(',').map(Number);
			return new cool.SimpleRectangle(twips[0], twips[1], twips[2], twips[3]);
		};
		const getDocumentTop = function (span) {
			const rect = getDocumentRect(span);
			return rect ? canvasRect.top + rect.v1Y / app.dpiScale - containerRect.top : NaN;
		};
		const getDocumentLeft = function (span) {
			const rect = getDocumentRect(span);
			return rect ? canvasRect.left + rect.v1X / app.dpiScale - containerRect.left : NaN;
		};
		const spread = function (region, top) {
			region.style.top = top + 'px';
			const spans = Array.from(region.children);
			const heights = spans.map(function (span) { return span.offsetHeight; });
			let bottom = top;
			spans.forEach(function (span, index) {
				const gap = Math.max(0, getDocumentTop(span) - bottom) || 0;
				span.style.marginTop = gap + 'px';
				bottom += gap + heights[index];
			});
			return bottom;
		};

		// not left of the paragraphs: the text starts far left with the caret on a wrapped line
		const anySpan = this._contextBefore.firstElementChild
			|| this._contextAfter.firstElementChild;
		const pageLeft = anySpan ? getDocumentLeft(anySpan) : NaN;
		const textLeft = textArea.offsetLeft + this._getTextOffset(0).x;
		const left = (isNaN(pageLeft) ? textLeft : Math.max(textLeft, pageLeft)) + 'px';
		regions.forEach(function (region) { region.style.left = left; });

		let above = textArea.offsetTop;
		regions.filter(isBefore).reverse().forEach(function (region) {
			const first = region.firstElementChild;
			const firstTop = first ? getDocumentTop(first) : NaN;
			const top = isNaN(firstTop) ? above : firstTop;
			const overlap = Math.max(0, spread(region, top) - above);
			above = top - overlap;
			region.style.top = above + 'px';
		});

		let below = textArea.offsetTop + textArea.scrollHeight;
		regions.filter(function (region) { return !isBefore(region); }).forEach(function (region) {
			below = spread(region, below);
		});
	},

	onAccessibilityCaretChanged: function(nPos) {
		this._log('onAccessibilityCaretChanged: \n' +
			'    position: ' + nPos + '\n' +
			'    _isComposing: ' + this._isComposing);
		if (this._isLeftRightArrow || !this.hasFocus() || this._isComposing) {
			this._log('onAccessibilityCaretChanged: skip updating');
			this._remotePosition = nPos;
		}
		else if (!this._hasFormulaBarFocus()) {
			this._updateCursorPosition(nPos);
		}
	},

	setA11yCaretPosition: function(nPos) {
		if (this._isLastSelectionEmpty()) {
			this.onAccessibilityCaretChanged(nPos);
		}
	},

	onAccessibilityTextSelectionChanged: function(start, end) {
		if (this._isLeftRightArrow || !this.hasFocus() || this._isComposing) {
			this._remoteSelectionStart = start;
			this._remoteSelectionEnd = end;
			var hasSelection = !(start === -1 && end === -1);
			this._setSelectionFlag(hasSelection);
			if (hasSelection) {
				this._remotePosition = end;
			}
			this._statusLog('onAccessibilityTextSelectionChanged: skip updating');
		} else {
			this._updateSelection(this._lastCursorPosition, start, end);
		}
	},

	A11Y_FOCUS_ANNOUNCEMENT_MS: 500,

	_setDescription: function(text) {
		this._log('setDescription: ' + text);
		this._textArea.setAttribute('aria-description', text);

		if (window.L.Browser.mac) {
			// avoid duplicate cell announcement in Chrome
			var sinceFocus = Date.now() - (this._a11yFocusTime || 0);
			if (this._textArea.isContentEditable && text
				&& sinceFocus < this.A11Y_FOCUS_ANNOUNCEMENT_MS
				&& text === this._a11yFocusDescription)
				return;

			// required on macOS as VoiceOver is not triggered by description change only
			var region = this._a11yLiveRegion;
			if (region) {
				if (this._a11yLiveRegionUpdate)
					cancelAnimationFrame(this._a11yLiveRegionUpdate);
				region.textContent = '';
				var t = text;
				var that = this;
				this._a11yLiveRegionUpdate = requestAnimationFrame(function () {
					that._a11yLiveRegionUpdate = 0;
					region.textContent = t;
				});
			}
		}
	},

	_updateTable: function(outCount, inList, row, col, rowSpan, colSpan) {
		this._log('_updateTable: '
			+ '\n outCount: ' + outCount
			+ '\n inList: ' + inList.toString()
			+ '\n row: ' + row + ', rowSpan: ' + rowSpan
			+ '\n col: ' + col + ', colSpan: ' + colSpan
		);

		if (this._timeoutForA11yDescription)
			clearTimeout(this._timeoutForA11yDescription);

		var eventDescription = '';
		if (outCount > 0 || inList.length > 0) {
			this._lastRowIndex = 0;
			this._lastColIndex = 0;
			this._lastRowSpan = 1;
			this._lastColSpan = 1;
		}
		for (var i = 0; i < outCount; i++) {
			eventDescription += _('Out of table') + '. ';
		}
		for (i = 0; i < inList.length; i++) {
			// One msgid can carry only one plural count, so the row and the column
			// count are separate plural entries that the sentence below composes.
			const rows = _n('%n row', '%n rows', inList[i].rowCount);
			const columns = _n('%n column', '%n columns', inList[i].colCount);
			// TRANSLATORS: {0} is a row count, {1} a column count, both already
			// TRANSLATORS: in their plural form, e.g. "3 rows" and "1 column"
			eventDescription += _('Table with {0} and {1}').replace('{0}', rows).replace('{1}', columns) + '. ';
		}
		if (this._lastRowIndex !== row || this._lastRowSpan !== rowSpan) {
			this._lastRowIndex = row;
			if (this._lastRowSpan !== rowSpan && rowSpan > 1) {
				eventDescription += _('Row {0} through {1}').replace('{0}', row).replace('{1}', row + rowSpan - 1);
			}
			else {
				eventDescription += _('Row {0}').replace('{0}', row);
			}
			eventDescription += '. ';
			this._lastRowSpan = rowSpan;
		}
		if (this._lastColIndex !== col || this._lastColSpan !== colSpan) {
			this._lastColIndex = col;
			if (this._lastColSpan !== colSpan && colSpan > 1) {
				eventDescription += _('Column {0} through {1}').replace('{0}', col).replace('{1}', col + colSpan - 1);
			}
			else {
				eventDescription += _('Column {0}').replace('{0}', col);
			}
			eventDescription += '. ';
			this._lastColSpan = colSpan;
		}
		this._setDescription(eventDescription + this._caretDescription());
		this._caretDescribed = true;

		this._timeoutForA11yDescription = setTimeout(this._describeCaretState.bind(this), 1000);
	},

	onAccessibilityFocusedCellChanged: function(outCount, inList, row, col, rowSpan, colSpan, paragraph) {
		this._endContextJump();
		this._keepCommentDetails(paragraph.comments, paragraph.beforeComments,
			paragraph.afterComments);
		this._requestHeadings();
		this._setContextParagraphs(paragraph.before, paragraph.after, paragraph.beforeRects, paragraph.afterRects,
			paragraph.beforeLevels, paragraph.afterLevels, paragraph.beforeLinks,
			paragraph.afterLinks, paragraph.beforeImages, paragraph.afterImages,
			paragraph.beforeComments, paragraph.afterComments);
		this._setFocusedParagraph(paragraph.content, parseInt(paragraph.position),
			parseInt(paragraph.start), parseInt(paragraph.end), paragraph.links, paragraph.images,
			paragraph.comments);
		this._setHeadingLevel(paragraph.headingLevel);
		this._updateTable(outCount, inList, row + 1, col + 1, rowSpan, colSpan);
	},

	onAccessibilityEditingInSelectionState: function(cell, enabled, selectionDescr, paragraph) {
		this._log('onAccessibilityEditingInSelectionState: cell: ' + cell + ', enabled: ' + enabled);
		if (!cell) {
			this._isEditingInSelection = enabled;
			// entering shape text editing makes this a real text box again
			this._updateA11yEditableState();
		}
		if (enabled) {
			clearTimeout(this._timeoutForA11yDescription);
			var eventDescription = '';
			if (typeof selectionDescr === 'string' && selectionDescr.length > 0)
				eventDescription += selectionDescr + '. ';
			eventDescription += _('Editing activated. ');
			if (typeof paragraph === 'string' && paragraph.length > 0)
				eventDescription += paragraph;
			this._setDescription(eventDescription);
			this._timeoutForA11yDescription = setTimeout(function () {
				this._setDescription('');
			}.bind(this), 1000);
		}
	},

	onAccessibilitySelectionChanged: function(cell, action, name, textContent) {
		this._log('onAccessibilitySelectionChanged: cell: ' + cell + ', action: ' + action + ', name: ' + name);
		if (this._timeoutForA11yDescription)
			clearTimeout(this._timeoutForA11yDescription);
		if (!this._isFormula())
			this._emptyArea();
		var eventDescription = '';
		if (action === 'create' || action === 'add') {
			this._hasAnySelection = true;
			// the selected object is now what this element stands for, so that is
			// what a screen reader reads when the focus next enters the document
			this._setA11yContext(name);
			eventDescription =  _('{0} selected').replace('{0}', name) + '. ';
			if (typeof textContent === 'string' && textContent.length > 0) {
				eventDescription += (cell ? '' : _('Has text: ')) + textContent;
			}
		}
		else if (action === 'remove') {
			this._hasAnySelection = false;
			this._setA11yContext(this._slideContext());
			eventDescription = _('{0} unselected').replace('{0}', name);
		}
		else if (action === 'delete') {
			this._hasAnySelection = false;
			this._setA11yContext(this._slideContext());
			eventDescription = _('{0} deleted').replace('{0}', name);
		}
		this._setDescription(eventDescription);
		if (action !== 'create' && action !== 'add') {
			this._timeoutForA11yDescription = setTimeout(function () {
				this._setDescription('');
			}.bind(this), 1000);
		}
	},

	// Check if a UTF-16 pair represents a Unicode code point
	_isSurrogatePair: function(hi, lo) {
		return 	hi >= 0xd800 && hi <= 0xdbff && lo >= 0xdc00 && lo <= 0xdfff;
	},

	// Backspaces and deletes at the beginning / end are filtered out, so
	// we get a beforeinput, but no input for them. Sometimes we can end up
	// in a state where we lost our leading / terminal chars and can't recover
	_onBeforeInput: function(ev) {
		if (this._map.uiManager.isUIBlocked())
			return;
		this._statusLog('_onBeforeInput [');
		this._ignoreNextBackspace = false;
		if (!this._isSelectionValid()) {
			this._setCursorPosition(this._getLastCursorPosition());
		}
		else if (this._isCursorAtBeginning()) {
			this._handleMisplacedCursorAtBeginning(ev);
		}
		else if (!this._isLastSelectionEmpty() && !this._hasFormulaBarFocus() && this._isFormula()) {
			// A cell address is selected in formula input mode,
			// before inserting a new input we need to clear selection
			this._updateCursorPosition(this._lastSelectionEnd);
		}

		if (!this._isComposing && !this._isLeftRightArrow && this._remotePosition !== undefined) {
			this._updateFocusedParagraph();
		}

		// Firefox is not able to delete the <img> post space. Since no 'input' event is generated,
		// we need to handle a <delete> at the end of the paragraph, here.
		if (window.L.Browser.gecko && (!this._hasSelection || this._isLastSelectionEmpty()) &&
			this._getLastCursorPosition() === this.getPlainTextContent().length &&
			this._deleteHint === 'delete') {
			if (this._map._debug.logKeyboardEvents) {
				window.app.console.log('Sending delete');
			}
			this._removeEmptySelectionIfAny();
			this._removeTextContent(0, 1);
		}
		this._statusLog('_onBeforeInput ]');
	},

	updateLastContent: function() {
		var value = this.getValue();
		this._lastContent = this.getValueAsCodePoints(value);
	},

	_isFormula: function() {
		var content = this.getValue();
		return this._map._docLoaded && this._map.getDocType() === 'spreadsheet'
			&& content.length > 0 && content[0] === '=';
	},

	_requestFocusedParagraph: function() {
		this._contextPending = (this._contextPending || 0) + 1;
		app.socket.sendMessage('geta11yfocusedparagraph');
	},

	_restoreSpanWrapper: function() {
		var children = this._textArea.childNodes;
		if (children.length >= 3 && children[1].nodeName === '#text') {
			if (children.length === 3) {
				// When typing in an empty paragraph, we get <img>H<img>
				this.setHTML(children[1].textContent);
			}
			else if (children.length === 4 && children[2].id === 'readable-content') {
				// When typing, let's say 'k', at beginning of a not empty paragraph,
				// we get: <img>k<span>Hello World</span><img>
				// moved as a node, which keeps any link of the paragraph
				children[2].insertBefore(children[1], children[2].firstChild);
			}
		}
	},

	// Fired when text has been inputted, *during* and after composing/spellchecking
	_onInput: function(ev) {
		if (this._map.uiManager.isUIBlocked())
			return;
		this._statusLog('_onInput [');
		app.idleHandler.notifyActive();

		if (this._ignoreInputCount > 0) {
			window.app.console.log('ignoring synthetic input ' + this._ignoreInputCount);
			return;
		}

		if (this._deleteHint === '' && ev.inputType) {
			if (ev.inputType === 'deleteContentForward')
				this._deleteHint = 'delete';
			else if (ev.inputType === 'deleteContentBackward')
				this._deleteHint = 'backspace';
		}

		var ignoreBackspace = this._ignoreNextBackspace;
		this._ignoreNextBackspace = false;

		if (this._newlineHint) {
			this._sendNewlineEvent();
			return;
		}

		// We use a different leading and terminal space character
		// to differentiate backspace from delete, then replace the character.
		if (!this._hasPreSpace()) { // missing initial space
			if (this._map._debug.logKeyboardEvents) {
				window.app.console.log('Sending backspace');
			}
			if (!ignoreBackspace) {
				this._removeEmptySelectionIfAny();
				this._removeTextContent(1, 0);
			}
			// Lately we receive the new paragraph == above paragraph + current paragraph,
			// except current paragraph is the first one.
			// In this last case we need to restore the pre space.
			this._prependSpace();
			this._updateCursorPosition(0);
			return;
		}
		if (!this._hasPostSpace()) { // missing trailing space.
			if (this._map._debug.logKeyboardEvents) {
				window.app.console.log('Sending delete');
			}
			this._removeTextContent(0, this._hasSelection && this._isLastSelectionEmpty() ? 2 : 1);
			this._appendSpace();
			var pos = this._getLastCursorPosition();
			this._updateCursorPosition(pos);
			return;
		}

		// We assume that what is on the right of the new cursor position has not been modified
		// We also assume that lastCursorPosition is synchronized with the cursor position in core
		var cursorPosition = this._getSelectionEnd();
		var lastCursorPosition = this._getLastCursorPosition();
		var value = this.getValue();

		// In the android keyboard when you try to erase the pre-space
		// and then enter some character,
		// The first character will likely travel with the cursor,
		// And that is caused because after entering the first character
		// cursor position is never updated by keyboard (I know it is strange)
		// so here we manually correct the position
		if (lastCursorPosition === 0 && cursorPosition < 1 && value.length - this._lastContent.length === 1) {
			cursorPosition = 1;
			if (!this._isComposing)
				this._setCursorPosition(1);
		}

		// We need to take into account the case that lastCursorPosition is beyond the new cursor position.
		// For instance that can happen when after entering a word, several spaces are typed:
		// a '.' is appended automatically.
		var contentTailLength = value.length - cursorPosition;
		var lastContentTailLength = this._lastContent.length - lastCursorPosition;
		var guessedBackMatchTo = Math.min(lastContentTailLength, contentTailLength);
		var contentEnd = value.length - guessedBackMatchTo;
		var content = this.getValueAsCodePoints(value.slice(0, contentEnd));
		// Note that content is an array of Unicode code points
		var lastContentEnd = this._lastContent.length - guessedBackMatchTo;
		var lastContent = this._lastContent.slice(0, lastContentEnd);

		window.app.console.log('_onInput: cursorPosition: ' + cursorPosition + ', lastContentEnd: ' + lastContentEnd);

		var matchTo = 0;
		var compareUpTo = Math.min(content.length, lastContent.length);
		if (!this._isLastSelectionEmpty()) {
			// Selected text has always to be removed, so there is no need for comparing old and new content
			// over selection start. Moreover, if selection content starts with the typed key, it would lead to
			// an empty new content and the input would never be forwarded to core.
			compareUpTo = Math.min(compareUpTo, this._lastSelectionStart);
		}
		while (matchTo < compareUpTo && content[matchTo] === lastContent[matchTo])
			matchTo++;

		if (this._map._debug.logKeyboardEvents) {
			window.app.console.log('Comparison matchAt ' + matchTo + '\n' +
				'\tnew "' + this.codePointsToString(content) + '" (' + content.length + ')' + '\n' +
				'\told "' + this.codePointsToString(lastContent) + '" (' + lastContent.length + ')');
		}

		// no new content
		if (matchTo === content.length && matchTo === lastContent.length)
			return;

		// matchTo <= lastCursorPosition <= lastContent.length
		matchTo = Math.min(matchTo, lastCursorPosition);

		var removeAfter = 0;
		var removeBefore = 0;
		if (!this._hasSelection) {
			removeAfter = lastContent.length - lastCursorPosition;
			removeBefore = (lastContent.length - matchTo) - removeAfter;
		}
		else if (this._deleteHint === 'backspace') {
			this._removeEmptySelectionIfAny();
			this._setSelectionFlag(false);
			removeBefore = 1;
		}
		else if (this._deleteHint === 'delete') {
			// when in core there is an empty selection the first <delete> deletes
			// the selection instead of the next char
			this._setSelectionFlag(false);
			removeAfter = this._isLastSelectionEmpty() ? 2 : 1;
		}

		// A browser selection range counts a surrogate UTF-16 pair as 2 chars.
		// The same occurs in core for the text cursor position reported by the caret changed accessibility event.
		// However, in core a single <backspace> or <delete> is needed for deleting a surrogate pair.
        if (removeBefore > 1) {
			var start = lastCursorPosition - removeBefore;
			for (var i = start; i < lastCursorPosition; i++) {
				if (this._isSurrogatePair(lastContent[i], lastContent[i+1])) {
					removeBefore--;
					i++;
				}
			}
		}
		if (removeAfter > 1) {
			var end = lastCursorPosition + removeAfter;
			for (var j = lastCursorPosition; j < end; j++) {
				if (this._isSurrogatePair(lastContent[j], lastContent[j+1])) {
					removeAfter--;
					j++;
				}
			}
		}

		if (removeBefore > 0 || removeAfter > 0)
			this._removeTextContent(removeBefore, removeAfter);

		var newText = content;
		if (matchTo > 0)
			newText = newText.slice(matchTo);

		var head = this._lastContent.slice(0, matchTo);
		var tail = this._lastContent.slice(lastContentEnd);
		this._lastContent = head.concat(newText, tail);
		window.app.console.log('_onInput: \n'
			+ 'head: "' + this.codePointsToString(head) + '"\n'
			+ 'newText: "' + this.codePointsToString(newText) + '"\n'
			+ 'tail: "' + this.codePointsToString(tail) + '"');

		this._setLastCursorPosition(cursorPosition);

		if (newText.length > 0) {
			if (!this._isComposing && !this._isWrappedBySpan()) {
				this._restoreSpanWrapper();
				this._setCursorPosition(cursorPosition);
			}
			this._sendNewText(ev, content, newText);
		}

		// special handling for formula bar
		this._finishFormulabarEditing(content, matchTo);

		// special handling for mentions
		if (this._map.getDocType() === 'text')
			this._map.mention.handleMentionInput(ev);

		this._statusLog('_onInput ]');
	},

	_removeEmptySelectionIfAny: function() {
		if (this._hasSelection && this._isLastSelectionEmpty()) {
			// when in core there is an empty selection a <backspace> or a <delete> removes
			// the selection instead of the previous or next char, so we send a fake <delete>
			// in order to remove the empty selection.
			this._sendDelete();
		}
	},

	_sendDelete: function() {
		this._sendKeyEvent(46, 1286, 'input');
	},

	_hasPreSpace: function() {
		var child = this._textArea.firstChild;
		while (child && child.tagName !== 'img') {
			if (child.id === 'pre-space')
				return true;
			child = child.firstChild;
		}
		return false;
	},

	_hasPostSpace: function() {
		var child = this._textArea.lastChild;
		while (child) {
			if (child.id === 'post-space')
				return true;
			child = child.lastChild;
		}
		return false;
	},

	_isWrappedBySpan: function() {
		var children = this._textArea.childNodes;
		return children.length === 3 && children[1].nodeName === 'SPAN';
	}
});

L.a11yTextInput = function() {
	return new window.L.A11yTextInput();
};
