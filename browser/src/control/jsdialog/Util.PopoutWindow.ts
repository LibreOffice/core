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
 * Util.PopoutWindow - hosts a jsdialog in its own native shell window.
 *
 * Opens an empty child window whose URL fragment carries the role that
 * names its type to the native shell. The dialog DOM is then built by
 * the opener directly into the child window's document. Same origin
 * means one JavaScript context, so widget code and callbacks work on
 * the child's nodes unchanged.
 */

declare var JSDialog: any;

interface PopoutWindowEntry {
	win: Window;
	// True while a close this code started is tearing the window down.
	closing: boolean;
	// Resolves once every cloned stylesheet has loaded, so measurements see
	// the styled layout.
	stylesReady: Promise<void>;
}

const popoutWindowRegistry = new Map<string, PopoutWindowEntry>();

function clonePageStyleIntoPopout(childDocument: Document): Promise<void> {
	// Relative URLs in the child resolve against the opener's base, not
	// against the child's about:blank URL.
	const base = childDocument.createElement('base');
	base.href = document.baseURI;
	childDocument.head.appendChild(base);

	const loads: Promise<void>[] = [];
	// An input element early in the page's head ends the head as the parser
	// reads it, so most stylesheet links sit in the body. Collecting them from
	// the whole document in document order picks up every sheet.
	document
		.querySelectorAll('link[rel="stylesheet"], style')
		.forEach((node: Element) => {
			if (node.tagName === 'LINK') {
				const link = childDocument.createElement('link');
				link.rel = 'stylesheet';
				link.href = (node as HTMLLinkElement).href;
				loads.push(
					new Promise((resolve) => {
						link.onload = () => resolve();
						link.onerror = () => resolve();
					}),
				);
				childDocument.head.appendChild(link);
			} else {
				childDocument.head.appendChild(node.cloneNode(true));
			}
		});

	// The theme (dark mode, RTL) is keyed off attributes on the root and
	// body elements; copy them so the same selectors match in the child.
	childDocument.documentElement.className = document.documentElement.className;
	for (const key of Object.keys(document.documentElement.dataset)) {
		childDocument.documentElement.dataset[key] =
			document.documentElement.dataset[key];
	}
	childDocument.documentElement.dir = document.documentElement.dir;

	// The dialog window is sized to the dialog it holds, so its own viewport
	// cannot serve as the room the dialog has. The document window's inner
	// size does, and it is the same room an in-page dialog gets.
	childDocument.documentElement.style.setProperty(
		'--dialog-available-width',
		window.innerWidth + 'px',
	);
	childDocument.documentElement.style.setProperty(
		'--dialog-available-height',
		window.innerHeight + 'px',
	);

	childDocument.body.className = document.body.className;
	childDocument.body.classList.add('jsdialog-popout-body');

	return Promise.all(loads).then(() => undefined);
}

JSDialog.OpenPopoutWindow = function (
	dialogId: string,
	isModal: boolean,
	title: string,
	onUserClose: () => void,
): Window | null {
	// The dialog id arrives as a JSON number from core on some routes and as a
	// string built from an object key on others; normalize so both routes hit
	// the same registry entry.
	const key = String(dialogId);
	if (!window.ThisIsTheQtApp || !window.origOpen) return null;

	// The URL fragment names the window's role to the native shell.
	const role = isModal ? 'coda-dialog-modal' : 'coda-dialog';
	const win = window.origOpen.call(
		window,
		'about:blank#' + role,
		'_blank',
		'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=1,popup=true',
	);
	if (!win) return null;

	const stylesReady = clonePageStyleIntoPopout(win.document);
	win.document.title = title || '';

	const entry: PopoutWindowEntry = {
		win: win,
		closing: false,
		stylesReady: stylesReady,
	};
	popoutWindowRegistry.set(key, entry);

	// The shell destroys the page when the user closes the native window;
	// report that so the dialog can be closed server-side too.
	win.addEventListener('pagehide', () => {
		// A new dialog can already be registered under the same key by the
		// time this fires, since the native close reaches this handler
		// several event-loop turns after ClosePopoutWindow deletes the old
		// entry; only remove the entry that still belongs to this window.
		if (popoutWindowRegistry.get(key) === entry)
			popoutWindowRegistry.delete(key);
		if (!entry.closing) onUserClose();
	});

	// Key events inside the dialog land in the child document. The window
	// outlives dialog rebuilds, so the listener is added here, once, rather
	// than with the per-build dialog handlers. An Escape a widget already
	// consumed closes only that widget.
	win.document.addEventListener('keydown', (event: KeyboardEvent) => {
		if (event.defaultPrevented) return;
		if (event.key === 'Escape') onUserClose();
	});

	return win;
};

JSDialog.GetPopoutWindow = function (dialogId: string): Window | null {
	const key = String(dialogId);
	const entry = popoutWindowRegistry.get(key);
	return entry && !entry.win.closed ? entry.win : null;
};

JSDialog.ClosePopoutWindow = function (dialogId: string): void {
	const key = String(dialogId);
	const entry = popoutWindowRegistry.get(key);
	popoutWindowRegistry.delete(key);
	if (entry && !entry.win.closed) {
		entry.closing = true;
		// A fragment-only navigation stays in-page and reaches the native
		// shell through its URL-change hook; the shell then closes the
		// native window itself, so window teardown runs on the same native
		// path as a user-initiated close.
		entry.win.location.hash = 'coda-dialog-close';
	}
};

JSDialog.FitPopoutToContent = function (
	dialogId: string,
	form: HTMLElement,
): void {
	const key = String(dialogId);
	const entry = popoutWindowRegistry.get(key);
	if (!entry || entry.win.closed) return;
	entry.stylesReady.then(() => {
		// A window the native shell has not shown yet receives no compositor
		// frames of its own, so the wait for the next frame runs on the opener's
		// window, which is always visible.
		window.requestAnimationFrame(() => {
			if (entry.win.closed) return;
			const rect = form.getBoundingClientRect();
			entry.win.resizeTo(Math.ceil(rect.width), Math.ceil(rect.height));
		});
	});
};

JSDialog.FindInPopoutWindows = function (id: string): HTMLElement | null {
	for (const entry of popoutWindowRegistry.values()) {
		if (entry.win.closed) continue;
		const found = entry.win.document.getElementById(id);
		if (found) return found;
	}
	return null;
};

JSDialog.ForEachPopoutDocument = function (
	callback: (childDocument: Document) => void,
): void {
	for (const entry of popoutWindowRegistry.values()) {
		if (!entry.win.closed) callback(entry.win.document);
	}
};
