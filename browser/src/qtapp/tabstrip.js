/* -*- js-indent-level: 4 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* global QWebChannel, qt */

(function () {
	const ICON_PX = 16;

	const docTypeIcon = {
		writer: 'images/x-office-document.svg',
		calc: 'images/x-office-spreadsheet.svg',
		impress: 'images/x-office-presentation.svg',
		draw: 'images/x-office-drawing.svg',
		starter: 'images/lc_home.svg',
		welcome: 'images/lc_home.svg',
		other: 'images/x-office-document.svg',
	};

	let bridge = null;
	let tabs = [];
	let activeId = -1;
	let tabEls = [];
	const lastMarker = { el: null, cls: '' };
	// Set by a strip's own drop so the source's dragend knows the drop was
	// handled here and must not detach the tab into a new window.
	const dragState = { inStripDropHandled: false };

	// Report a hover to C++ only when the insert index changes.
	let lastOverInsert = -2;

	function el(tag, props) {
		const e = document.createElement(tag);
		if (props) for (const k in props) e[k] = props[k];
		return e;
	}

	function clearDropMarkers() {
		if (!lastMarker.el) return;
		lastMarker.el.classList.remove(lastMarker.cls);
		lastMarker.el = null;
		lastMarker.cls = '';
	}

	function setDropMarker(target, before) {
		const cls = before ? 'drop-before' : 'drop-after';
		if (lastMarker.el === target && lastMarker.cls === cls) return;
		clearDropMarkers();
		target.classList.add(cls);
		lastMarker.el = target;
		lastMarker.cls = cls;
	}

	// The tabs run from right to left in a language written that way.
	function isRightToLeft() {
		return document.documentElement.dir === 'rtl';
	}

	// For a viewport X inside the strip, return the tab under it, whether the
	// drop goes before or after, and the resulting insert index (null if empty).
	function dropTargetAt(clientX) {
		if (!tabEls.length) return null;
		for (let i = 0; i < tabEls.length; i++) {
			const tabEl = tabEls[i];
			const rect = tabEl.getBoundingClientRect();
			// How far X is from the edge of the tab where the tab starts.
			const fromStart = isRightToLeft()
				? rect.right - clientX
				: clientX - rect.left;
			if (fromStart < rect.width) {
				const before = fromStart < rect.width / 2;
				return {
					el: tabEl,
					before: before,
					insertAt: i + (before ? 0 : 1),
				};
			}
		}
		const last = tabEls[tabEls.length - 1];
		return { el: last, before: false, insertAt: tabEls.length };
	}

	// No markers or slot reports for OS file or text drags.
	function isTabDrag(ev) {
		return ev.dataTransfer && ev.dataTransfer.types.includes('text/x-coda-tab');
	}

	// Bound to the whole strip, not per tab, so gaps, padding and the `+`
	// button all accept a drop. C++ tracks the hovered strip from native
	// drag events; dragover only refines the insert slot. No dragleave
	// handler: its position cannot tell "left the strip" from "moved
	// between tabs", so C++ signals dragExited from the native leave.
	function installStripDropTarget(strip) {
		strip.addEventListener('dragenter', (ev) => {
			if (!isTabDrag(ev)) return;
			ev.preventDefault();
			// C++ reset its slot on the native enter; push the next
			// dragover past the dedup even if the slot is unchanged.
			lastOverInsert = -2;
		});
		strip.addEventListener('dragover', (ev) => {
			if (!isTabDrag(ev)) return;
			ev.preventDefault();
			ev.dataTransfer.dropEffect = 'move';
			const hit = dropTargetAt(ev.clientX);
			if (hit) setDropMarker(hit.el, hit.before);
			else clearDropMarkers();
			if (bridge && hit && hit.insertAt !== lastOverInsert) {
				lastOverInsert = hit.insertAt;
				bridge.targetDragOver(hit.insertAt);
			}
		});
		strip.addEventListener('drop', (ev) => {
			if (!isTabDrag(ev)) return;
			ev.preventDefault();
			clearDropMarkers();
			lastOverInsert = -2;
			dragState.inStripDropHandled = true;
			const srcId = parseInt(ev.dataTransfer.getData('text/x-coda-tab'), 10);
			const hit = dropTargetAt(ev.clientX);
			if (!Number.isFinite(srcId) || !hit) return;
			const srcIdx = tabs.findIndex((x) => x.id === srcId);
			if (srcIdx >= 0) {
				// Our own tab: reorder in place.
				let insertAt = hit.insertAt;
				if (srcIdx < insertAt) insertAt--;
				if (bridge && srcIdx !== insertAt)
					bridge.tabReordered(srcIdx, insertAt);
			} else if (bridge) {
				// Another window's tab (ids are unique): adopt it here.
				bridge.tabAdoptFromOtherWindow(srcId, hit.insertAt);
			}
		});
	}

	// Ask the app for the tab's hover card, placed below the tab and drawn in the strip's
	// theme colours. The card lines up with the edge of the tab where the tab starts.
	function startHover(tab, id) {
		if (!bridge) return;
		const rect = tab.getBoundingClientRect();
		const style = getComputedStyle(document.documentElement);
		const color = (name) => style.getPropertyValue(name).trim();
		const rightToLeft = isRightToLeft();
		bridge.tabHoverStarted(
			id,
			Math.round(rightToLeft ? rect.right : rect.left),
			Math.round(rect.bottom),
			color('--color-main-background'),
			color('--color-main-text'),
			color('--color-border-dark'),
			rightToLeft,
		);
	}

	// Rebuilt whole on every tabsChanged: matches the Qt-side push rate and
	// keeps no stale drag state.
	function render() {
		const strip = document.getElementById('strip');
		strip.replaceChildren();
		tabEls = [];
		clearDropMarkers();
		tabs.forEach((t, idx) => {
			const tab = el('div');
			tab.className = 'tab' + (t.id === activeId ? ' active' : '');
			tab.draggable = true;
			tab.dataset.id = String(t.id);
			tab.dataset.index = String(idx);
			tab.setAttribute('role', 'tab');
			// A background tab with a preview gets the app's hover card, which shows the name too.
			// The active tab keeps the plain title tooltip.
			if (t.hasThumbnail && t.id !== activeId) {
				tab.addEventListener('mouseenter', () => startHover(tab, t.id));
				tab.addEventListener('mouseleave', () => {
					if (bridge) bridge.tabHoverEnded();
				});
			} else {
				// The tooltip shows where the document is, the tab itself
				// only its name.
				tab.title = t.path || t.title || '';
			}

			const icon = el('span');
			icon.className = 'icon';
			const iconSrc = docTypeIcon[t.docType] || docTypeIcon.other;
			icon.appendChild(
				el('img', {
					src: iconSrc,
					width: ICON_PX,
					height: ICON_PX,
					alt: '',
				}),
			);

			const title = el('span');
			title.className = 'title' + (t.modified ? ' modified' : '');
			title.textContent = t.title || '(untitled)';

			const close = el('span');
			close.className = 'close';
			close.textContent = '×';
			close.title = 'Close tab';
			close.addEventListener('click', (ev) => {
				ev.stopPropagation();
				if (bridge) bridge.tabCloseRequested(t.id);
			});

			tab.addEventListener('click', () => {
				if (bridge) bridge.tabActivated(t.id);
			});
			tab.addEventListener('auxclick', (ev) => {
				if (ev.button === 1 && bridge) bridge.tabCloseRequested(t.id);
			});
			tab.addEventListener('contextmenu', (ev) => {
				ev.preventDefault();
				if (bridge)
					bridge.tabContextMenuRequested(t.id, ev.clientX, ev.clientY);
			});

			tab.addEventListener('dragstart', (ev) => {
				ev.dataTransfer.effectAllowed = 'move';
				ev.dataTransfer.setData('text/x-coda-tab', String(t.id));
				tab.classList.add('dragging');
				dragState.inStripDropHandled = false;
				if (bridge) bridge.tabDragStarted(t.id);
			});
			tab.addEventListener('dragend', () => {
				tab.classList.remove('dragging');
				clearDropMarkers();
				// C++ decides the cross-window move; we only report whether
				// our own drop handled it.
				if (bridge) bridge.tabDragEnded(t.id, dragState.inStripDropHandled);
				dragState.inStripDropHandled = false;
			});

			const content = el('div');
			content.className = 'content';
			content.appendChild(icon);
			content.appendChild(title);
			tab.appendChild(content);
			tab.appendChild(close);
			strip.appendChild(tab);
			tabEls.push(tab);
		});

		const add = el('div');
		add.id = 'new-tab';
		add.textContent = '+';
		add.title = 'New';
		add.addEventListener('click', () => {
			if (bridge) bridge.newTabRequested();
		});
		strip.appendChild(add);
	}

	function onTabsChanged(json) {
		try {
			tabs = JSON.parse(json) || [];
		} catch (e) {
			tabs = [];
		}
		const active = tabs.find((t) => t.active);
		if (active) activeId = active.id;
		render();
	}

	function onThemeChanged(theme) {
		document.documentElement.setAttribute('data-theme', theme);
	}

	function onDragExited() {
		clearDropMarkers();
		lastOverInsert = -2;
	}

	// The calls the strip makes into the app, and the notifications the app
	// sends back. The bridge below offers a method per call and an object with
	// a connect() per notification, whichever host the strip is running in.
	const CALLS = [
		'tabActivated',
		'tabCloseRequested',
		'tabContextMenuRequested',
		'newTabRequested',
		'tabReordered',
		'tabDragStarted',
		'tabDragEnded',
		'tabAdoptFromOtherWindow',
		'targetDragOver',
		'tabHoverStarted',
		'tabHoverEnded',
		'requestSync',
		'debug',
	];
	const NOTIFICATIONS = ['tabsChanged', 'themeChanged', 'dragExited'];

	// The WebView2 message channel carries structured data both ways: a call
	// goes out as {call, args} and a notification arrives as {signal, args}.
	function webView2Bridge() {
		const bridgeObject = {};
		const handlers = {};

		NOTIFICATIONS.forEach((name) => {
			handlers[name] = [];
			bridgeObject[name] = {
				connect: (handler) => handlers[name].push(handler),
			};
		});
		CALLS.forEach((name) => {
			bridgeObject[name] = (...args) =>
				window.chrome.webview.postMessage({ call: name, args: args });
		});

		window.chrome.webview.addEventListener('message', (ev) => {
			const message = ev.data;
			if (!message || !handlers[message.signal]) return;
			handlers[message.signal].forEach((handler) =>
				handler(...(message.args || [])),
			);
		});

		return bridgeObject;
	}

	// The Windows app hosts the strip in a WebView2, the Linux one in a
	// QWebEngineView whose QWebChannel exposes the app object directly.
	function connectTabBridge(onReady) {
		if (window.chrome && window.chrome.webview) {
			onReady(webView2Bridge());
			return;
		}
		new QWebChannel(qt.webChannelTransport, (channel) =>
			onReady(channel.objects.tabBridge),
		);
	}

	installStripDropTarget(document.getElementById('strip'));

	connectTabBridge(function (tabBridge) {
		bridge = tabBridge;
		bridge.tabsChanged.connect(onTabsChanged);
		bridge.themeChanged.connect(onThemeChanged);
		bridge.dragExited.connect(onDragExited);
		bridge.debug('tabstrip ready');
		// Slots are connected now; ask the app for the current state.
		bridge.requestSync();
	});
})();
