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
 * Definitions.DialogModification - dialog-specific customization callbacks
 */

declare var JSDialog: any;
declare var UNOKey: any;

type DialogModificationCallback = (instance: any) => void;

const dialogModifications = new Map<string, DialogModificationCallback>();

// Find & Replace Dialog keyboard handler
dialogModifications.set('FindReplaceDialog', function (instance: any) {
	if (!instance.container) return;

	// Pre-fill the search field when another component (e.g. Navigator) stored a term.
	const pendingTerm = JSDialog.pendingFindReplaceSearchTerm as
		| string
		| undefined;
	if (pendingTerm) {
		JSDialog.pendingFindReplaceSearchTerm = undefined;
		// The searchterm widget id is defined in srchdlg.cxx. Its DOM input
		// element has the suffix appended, so anchor the selector to the
		// known prefix to avoid matching an unrelated widget.
		const searchInput = instance.container.querySelector(
			'input[id^="searchterm-input-"]',
		) as HTMLInputElement;
		if (searchInput && instance.id !== undefined) {
			// Set the DOM value immediately so the field is filled before
			// the server round-trip completes.
			searchInput.value = pendingTerm;
			app.socket.sendMessage(
				'dialogevent ' +
					instance.id +
					' ' +
					JSON.stringify({
						id: 'searchterm',
						cmd: 'change',
						data: pendingTerm,
						type: 'combobox',
					}),
			);
		}
	}

	instance.container.addEventListener('keydown', function (e: KeyboardEvent) {
		if (e.code !== 'Enter') return; // Only handle Enter key

		const activeElement = document.activeElement as HTMLElement;
		if (!activeElement) return;

		if (activeElement.tagName === 'INPUT') {
			const inputElement = activeElement as HTMLInputElement;
			if (inputElement.type === 'text') {
				const activeId = inputElement.id;
				let pushButtonWrapperId: string | null = null;

				if (activeId.includes('searchterm')) {
					pushButtonWrapperId = e.shiftKey ? 'backsearch' : 'search';
				} else if (activeId.includes('replaceterm')) {
					pushButtonWrapperId = 'replace';
				}

				if (pushButtonWrapperId) {
					const pushButtonWrapper = instance.container.querySelector(
						`#${pushButtonWrapperId}`,
					);
					/* Pushbutton wrapper always has a single child: button */
					const button = pushButtonWrapper
						? (pushButtonWrapper.firstChild as HTMLButtonElement)
						: null;
					if (button && !button.disabled) {
						button.click();
						e.preventDefault();
					}
				}
			} else if (inputElement.type === 'checkbox') {
				inputElement.click();
				e.preventDefault();
			}
		}
	});
});

// TOC Dialog: Delete key and context menu for token buttons
dialogModifications.set('TocDialog', function (instance: any) {
	if (!instance.container) return;

	function sendAction(tokenId: string, cmd: string, data: string) {
		const message =
			'dialogevent ' +
			instance.id +
			' ' +
			JSON.stringify({
				id: tokenId,
				cmd: cmd,
				data: data,
				type: 'pushbutton',
			});
		(window as any).app.socket.sendMessage(message);
	}

	function deleteToken(tokenWrapper: Element) {
		// Select the token first (so core's m_pActiveCtrl points to it),
		// then send Delete keypress.
		if (!tokenWrapper.classList.contains('checked')) {
			sendAction(tokenWrapper.id, 'toggle', '');
		}
		sendAction(tokenWrapper.id, 'keypress', String(UNOKey.DELETE));
	}

	// Forward focus events from edit inputs so core tracks the active control.
	// Without this, clicking an edit between tokens in COKit mode does not
	// update m_pActiveCtrl, and insertion buttons act on the wrong position.
	instance.container.addEventListener('focusin', function (e: FocusEvent) {
		const input = e.target as HTMLElement;
		if (input.tagName !== 'INPUT') return;
		// The edit container has the widget ID; the input has id + '-input'
		const container = input.closest('.ui-edit-container');
		if (!container) return;
		const message =
			'dialogevent ' +
			instance.id +
			' ' +
			JSON.stringify({
				id: container.id,
				cmd: 'grab_focus',
				data: '',
				type: 'edit',
			});
		(window as any).app.socket.sendMessage(message);
	});

	// Delete key removes the selected token
	instance.container.addEventListener('keydown', function (e: KeyboardEvent) {
		if (e.key !== 'Delete') return;

		// Don't intercept Delete in input/textarea fields
		const active = document.activeElement as HTMLElement;
		if (active && (active.tagName === 'INPUT' || active.tagName === 'TEXTAREA'))
			return;

		const checked = instance.container.querySelector('.ui-toggle.checked');
		if (!checked) return;

		deleteToken(checked);
		e.preventDefault();
	});

	// Right-click context menu with Delete option for token buttons
	instance.container.addEventListener('contextmenu', function (e: MouseEvent) {
		const target = e.target as HTMLElement;
		const toggle = target.closest('.ui-toggle');
		if (!toggle) return;

		e.preventDefault();

		// Remove any existing context menu
		const old = document.getElementById('toc-token-context-menu');
		if (old) old.remove();

		const menu = document.createElement('ul');
		menu.id = 'toc-token-context-menu';
		menu.className = 'context-menu-list';
		menu.style.cssText =
			'position:fixed;z-index:10000;display:block;' +
			'font-family:var(--cool-font);min-width:0;';

		const item = document.createElement('li');
		item.className = 'context-menu-item';
		const link = document.createElement('a');
		link.href = '#';
		link.className = 'context-menu-link';
		link.textContent = _('Delete');
		item.appendChild(link);
		item.addEventListener('mouseenter', function () {
			item.classList.add('context-menu-hover');
		});
		item.addEventListener('mouseleave', function () {
			item.classList.remove('context-menu-hover');
		});
		item.addEventListener('click', function () {
			closeMenu();
			deleteToken(toggle);
		});
		menu.appendChild(item);

		menu.style.left = e.clientX + 'px';
		menu.style.top = e.clientY + 'px';
		document.body.appendChild(menu);

		function closeMenu() {
			menu.remove();
			document.removeEventListener('mousedown', onMouseDown);
			document.removeEventListener('keydown', onKeyDown);
		}
		function onMouseDown(ev: MouseEvent) {
			if (!menu.contains(ev.target as Node)) closeMenu();
		}
		function onKeyDown(ev: KeyboardEvent) {
			if (ev.key === 'Escape') closeMenu();
		}
		setTimeout(function () {
			document.addEventListener('mousedown', onMouseDown);
			document.addEventListener('keydown', onKeyDown);
		}, 0);
	});
});

JSDialog.getDialogModificationCallback = function (
	dialogId: string,
): DialogModificationCallback | undefined {
	return dialogModifications.get(dialogId);
};
