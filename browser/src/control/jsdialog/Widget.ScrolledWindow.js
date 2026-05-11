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
 * JSDialog.ScrolledWindow - container with scrollbars
 *
 * Example JSON:
 * {
 *     id: 'id',
 *     type: 'scrollwindow',
 *     vertical: {
 * 	       policy: always,
 *         lower: 0,
 *         upper: 5,
 *         page_size: 4
 *     },
 *     horizontal: {
 * 	       policy: always,
 *         lower: 0,
 *         upper: 5,
 *         page_size: 4
 *     },
 *     children: [...]
 * }
 */

/* global JSDialog */

function _hasDrawingAreaInside(children) {
	if (!children)
		return false;

	for (var i in children) {
		if (children[i].type === 'drawingarea')
			return true;
		if (_hasDrawingAreaInside(children[i].children))
			return true;
	}

	return false;
}

function _scrolledWindowControl(parentContainer, data, builder) {
	var scrollwindow = window.L.DomUtil.create('div', builder.options.cssClass + ' ui-scrollwindow', parentContainer);
	if (data.id)
		scrollwindow.id = data.id;

	// drawing areas inside scrollwindows should be not cropped so we add special class
	if (_hasDrawingAreaInside(data.children)) {
		window.L.DomUtil.addClass(scrollwindow, 'has-ui-drawing-area');
		scrollwindow.setAttribute('tabindex', '-1');
	}

	var content = window.L.DomUtil.create('div', builder.options.cssClass + ' ui-scrollwindow-content', scrollwindow);

	builder.build(content, data.children, false);

	if (!data.vertical && !data.horizontal)
		return false;

	var noVertical = data.vertical.policy === 'never';
	if (noVertical)
		scrollwindow.style.overflowY = 'hidden';
	if (data.vertical.policy === 'always')
		scrollwindow.style.overflowY = 'scroll';

	var noHorizontal = data.horizontal.policy === 'never';
	if (noHorizontal)
		scrollwindow.style.overflowX = 'hidden';
	if (data.horizontal.policy === 'always')
		scrollwindow.style.overflowX = 'scroll';

	// "External" horizontal scrolling: policy is "never" (no scrollbar) but content
	// may be wider than the viewport. The browser determines actual overflow.
	// Allow content to extend and handle scrolling via sibling scroll buttons.
	if (noHorizontal) {
		content.style.width = 'max-content';
		content.style.minWidth = '100%';
		scrollwindow.style.overflowX = 'hidden';

		var setupExternalScroll = function() {
			if (scrollwindow._externalScrollSetup)
				return;

			var hasOverflow = scrollwindow.scrollWidth > scrollwindow.clientWidth;
			if (!hasOverflow)
				return;

			// Find sibling scroll buttons - deferred so all siblings exist in DOM.
			// A scroll-button container is a simple wrapper whose only child
			// is the button (e.g. a pushbutton widget).  Tab headers, toolbars
			// and other complex siblings also contain <button> elements but
			// have more than one child — skip those.
			var prevSibling = scrollwindow.previousElementSibling;
			var nextSibling = scrollwindow.nextElementSibling;
			var isScrollBtn = function(sibling) {
				if (!sibling)
					return false;
				var btn = sibling.querySelector('button');
				return btn && btn.parentElement === sibling
					&& sibling.children.length === 1;
			};
			var isLeftScrollBtn = isScrollBtn(prevSibling);
			var isRightScrollBtn = isScrollBtn(nextSibling);

			// Only swap to a native horizontal scrollbar when this scrolled
			// window is the one driving sibling scroll buttons (the
			// "external scroll" pattern, e.g. the TOC tokens row). If no
			// such buttons exist this is a regular panel that just asked
			// for hscrollbar-policy="never"; drop the max-content
			// expansion so content fits the container instead of being
			// clipped on the right (e.g. the Animation sidebar).
			if (!isLeftScrollBtn && !isRightScrollBtn) {
				content.style.width = '';
				content.style.minWidth = '';
				scrollwindow._externalScrollSetup = true;
				return;
			}

			scrollwindow._externalScrollSetup = true;

			// Hide sibling scroll buttons — only simple single-button
			// wrappers, not tab headers or other complex widgets.
			if (isLeftScrollBtn)
				prevSibling.style.display = 'none';
			if (isRightScrollBtn)
				nextSibling.style.display = 'none';

			// Show a native browser scrollbar in place of the hidden buttons.
			scrollwindow.style.overflowX = 'auto';
		};

		// Use ResizeObserver to detect when content overflows the
		// viewport and enable a native horizontal scrollbar.
		var observer = new ResizeObserver(function() {
			setupExternalScroll();
		});
		observer.observe(content);
	}

	var realContentHeight = scrollwindow.scrollHeight;
	var realContentWidth = scrollwindow.scrollwidth;

	var margin = 15;

	var verticalSteps = (data.vertical.upper - data.vertical.lower - data.vertical.page_size) * 10;
	if (verticalSteps < 0 || noVertical)
		verticalSteps = 0;

	var horizontalSteps = (data.horizontal.upper - data.horizontal.lower - data.horizontal.page_size) * 10;
	if (horizontalSteps < 0 || noHorizontal)
		horizontalSteps = 0;

	var timeoutLimit = 2;
	var updateSize = function () {
		realContentHeight = scrollwindow.scrollHeight;
		realContentWidth = scrollwindow.scrollwidth;
		if (realContentHeight === 0 || realContentWidth === 0) {
			if (timeoutLimit--)
				setTimeout(updateSize, 100);
			return;
		}

		if (!noVertical) {
			content.style.height = (realContentHeight + verticalSteps) + 'px';
			scrollwindow.style.height = (realContentHeight + margin) + 'px';
		}
		if (!noHorizontal) {
			content.style.width = (realContentWidth + horizontalSteps) + 'px';
			scrollwindow.style.width = (realContentWidth + margin) + 'px';
		}

		content.scrollTop = data.vertical.value * 10;
		content.scrollLeft = data.horizontal.value * 10;

		content.style.margin = content.scrollTop + 'px ' + margin + 'px ' + margin + 'px ' + content.scrollLeft + 'px';
	};

	if (data.user_managed_scrolling !== false)
		setTimeout(updateSize, 0);

	var sendTimer = null;

	if ((!noVertical && verticalSteps) || (!noHorizontal && horizontalSteps)) {
		scrollwindow.addEventListener('scroll', function () {
			// keep content at the same place on the screen
			var scrollTop = scrollwindow.scrollTop;
			var scrollLeft = scrollwindow.scrollLeft;

			if (data.user_managed_scrolling !== false) {
				content.style.margin = scrollTop + 'px ' + margin + 'px ' + margin + 'px ' + scrollLeft + 'px';
				content.style.height = (realContentHeight - scrollTop + verticalSteps) + 'px';
				content.style.width = (realContentWidth - scrollLeft + horizontalSteps) + 'px';
			}

			if (sendTimer)
				clearTimeout(sendTimer);
			sendTimer = setTimeout(function () {
				builder.callback('scrolledwindow', 'scrollv', scrollwindow, Math.round(scrollTop / 10), builder);
				builder.callback('scrolledwindow', 'scrollh', scrollwindow, Math.round(scrollLeft / 10), builder); }, 50);
		});
	}

	return false;
}

JSDialog.scrolledWindow = function (parentContainer, data, builder) {
	var buildInnerData = _scrolledWindowControl(parentContainer, data, builder);
	return buildInnerData;
};
