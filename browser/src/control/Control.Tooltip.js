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
 * Class Tooltip - tooltip manager
 */

/* global app */

class Tooltip {
	constructor(options) {
		this._options = window.L.extend({ timeout: 150 }, options);
		let win = this._options.window ? this._options.window : window;
		this._container = this._options.container
			? this._options.container
			: window.L.DomUtil.create('div', 'cooltip-text', win.document.body);
		this._container.id = 'cooltip';
		this._container.addEventListener(
			'mouseenter',
			window.L.bind(this.mouseEnter, this),
		);
		this._container.addEventListener(
			'mouseleave',
			window.L.bind(this.mouseLeave, this),
		);

		win.addEventListener('keydown', window.L.bind(this.keyDown, this), {
			capture: true,
			passive: true,
		});
	}

	beginShow(elem) {
		if (this._cancel || this._disabled) return;

		let win = this._options.window ? this._options.window : window;
		win.clearTimeout(this._showTimeout);
		this._showTimeout = win.setTimeout(
			window.L.bind(this.show, this, elem),
			this._options.timeout,
		);
	}

	beginHide(elem) {
		if (this._cancel || this._disabled || this._locked) return;

		let win = this._options.window ? this._options.window : window;
		win.clearTimeout(this._showTimeout);
		win.clearTimeout(this._hideTimeout);
		if (this._current)
			this._hideTimeout = win.setTimeout(
				window.L.bind(this.hide, this, elem),
				this._options.timeout / 8,
			);
	}

	// Pin the tooltip on a specific element so it stays visible while the
	// caller animates it (e.g. live value during a drag). Cancels any pending
	// hide and re-runs show() so the icon/label DOM is correct.
	lock(elem, textContent) {
		if (this._disabled) return;
		let win = this._options.window ? this._options.window : window;
		win.clearTimeout(this._hideTimeout);
		win.clearTimeout(this._showTimeout);
		this._locked = true;
		this.show(elem, textContent);
	}

	// Update just the label text on an already-locked tooltip, without
	// recomputing the position. Cheap enough to call from a mousemove.
	updateLabel(elem, textContent) {
		if (this._current !== elem) return;
		let label = this._container.querySelector('.cooltip-label');
		if (label) label.textContent = textContent;
	}

	unlock() {
		this._locked = false;
		this.hide();
	}

	// Switch the tooltip subsystem off entirely and hide live tooltip if any.
	disable() {
		if (this._disabled) return;
		let win = this._options.window ? this._options.window : window;
		this._disabled = true;
		win.clearTimeout(this._showTimeout);
		win.clearTimeout(this._hideTimeout);
		this._container.style.visibility = 'hidden';
		this._current = null;
		this._cancel = false;
	}

	enable() {
		this._disabled = false;
	}

	/**
	 * Calculate one of the 8 different position
	 * of the tooltip, around the elem parameter
	 *
	 * - bottom-right, bottom-left, top-right, top-left
	 * - and their left/right aligned versions
	 *
	 * @param elem - element that the cursor is over
	 * @param popup - tooltip rectangle
	 * @param index - used to determine one of the 8 tooltip location
	 * @returns tooltip rectangle with its location calculated
	 */
	position(elem, popup, index) {
		let rect = new DOMRect();
		switch (index) {
			case 0: // below cursor, bottom-right (aligned to left)
				rect.x = elem.left;
				rect.y = elem.bottom + 12;
				break;
			case 1: // below cursor, bottom-left (aligned to right)
				rect.x = elem.right - popup.width;
				rect.y = elem.bottom + 12;
				break;
			case 2: // above cursor, top-right (aligned to left)
				rect.x = elem.left;
				rect.y = elem.top - popup.height - 8;
				break;
			case 3: // above cursor, top-left (aligned to right)
				rect.x = elem.right - popup.width;
				rect.y = elem.top - popup.height - 8;
				break;
			case 4: // below cursor, bottom-right
				rect.x = elem.right;
				rect.y = elem.bottom + 4;
				break;
			case 5: // below cursor, bottom-left
				rect.x = elem.left - popup.width;
				rect.y = elem.bottom + 4;
				break;
			case 6: // above cursor, top-right
				rect.x = elem.right;
				rect.y = elem.top - popup.height - 4;
				break;
			case 7: // above cursor, top-left
				rect.x = elem.left - popup.width;
				rect.y = elem.top - popup.height - 4;
				break;
			default:
				break;
		}

		rect.width = popup.width;
		rect.height = popup.height;

		return rect;
	}

	show(elem, textContent) {
		if (this._disabled) return;
		// `textContent` adds flexibility, enabling custom messages like document "Saved" instead of the fixed "cool-tooltip."
		let content = textContent ? textContent : elem.dataset.cooltip,
			iconUrl = elem.dataset.cooltipIcon,
			rectView = new DOMRect(0, 0, window.innerWidth, window.innerHeight),
			rectElem = elem.getBoundingClientRect(),
			rectCont,
			rectTooltip,
			index = 0;

		// Clear any prior content (built safely via DOM nodes to support
		// an optional icon without resorting to innerHTML).
		while (this._container.firstChild)
			this._container.removeChild(this._container.firstChild);

		if (!content && !iconUrl) return;

		this._container.classList.toggle('cooltip-with-icon', !!iconUrl);

		if (iconUrl) {
			let icon = document.createElement('img');
			icon.src = iconUrl;
			icon.alt = '';
			icon.className = 'cooltip-icon';
			this._container.appendChild(icon);
		}
		if (content) {
			let label = document.createElement('span');
			label.className = 'cooltip-label';
			label.textContent = content;
			this._container.appendChild(label);
		}

		rectCont = this._container.getBoundingClientRect();

		do {
			rectTooltip = this.position(rectElem, rectCont, index++);
		} while (index < 8 && !app.LOUtil.containsDOMRect(rectView, rectTooltip));
		// containsDOMRect() checks if the tooltip box(rectTooltip) is inside the boundaries of the window(rectView)

		this._container.style.left = rectTooltip.left + 'px';
		this._container.style.top = rectTooltip.top + 'px';
		this._container.style.visibility = 'visible';
		this._current = elem;
	}

	hide() {
		if (this._cancel || this._locked) return;

		this._container.style.visibility = 'hidden';
		this._current = null;
	}

	mouseEnter() {
		if (this._disabled) return;
		if (this._current) {
			let win = this._options.window ? this._options.window : window;
			this._cancel = true;
			win.clearTimeout(this._hideTimeout);
			win.clearTimeout(this._showTimeout);
		}
	}

	mouseLeave() {
		if (this._disabled) return;
		this._cancel = false;
		this.beginHide();
	}

	keyDown(e) {
		let key = e.key.toUpperCase();
		if (key === 'ESCAPE') {
			this.mouseLeave();
		}
	}

	static attachEventListener(elem, map) {
		if (!map.tooltip) {
			return;
		}

		elem.addEventListener('mouseenter', function () {
			map.tooltip.beginShow(elem);
		});
		elem.addEventListener('mouseleave', function () {
			map.tooltip.beginHide(elem);
		});
		elem.addEventListener('click', function () {
			map.tooltip.mouseLeave();
		});
	}
}

window.L.control.tooltip = function (options) {
	return new Tooltip(options);
};

window.L.control.attachTooltipEventListener = Tooltip.attachEventListener;
