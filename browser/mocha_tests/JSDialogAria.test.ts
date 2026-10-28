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

describe('JSDialog aria attributes', function () {
	describe('the description of a widget', function () {
		const purpose = 'Lets a screen reader follow the cursor';

		it('is written when there is one to write', function () {
			const button = document.createElement('button');
			JSDialog.AddAriaDescription(button, { aria: { description: purpose } });
			nodeassert.strictEqual(button.getAttribute('aria-description'), purpose);
		});

		it('is left out when it is blank or missing', function () {
			const button = document.createElement('button');
			JSDialog.AddAriaDescription(button, { aria: { description: '   ' } });
			nodeassert.strictEqual(button.getAttribute('aria-description'), null);

			JSDialog.AddAriaDescription(button, {});
			nodeassert.strictEqual(button.getAttribute('aria-description'), null);
		});
	});

	describe('a greyed-out toolbar item', function () {
		const realLayoutingService = app.layoutingService;

		before(function () {
			app.layoutingService = new LayoutingService();
		});

		after(function () {
			app.layoutingService = realLayoutingService;
		});

		// The item mirrors its disabled attribute from a mutation observer, which runs after
		// the current task, and then from a layouting task.
		async function settle(): Promise<void> {
			await Promise.resolve();
			while (app.layoutingService.hasTasksPending())
				app.layoutingService.runTheTopTask();
		}

		function toolItem(): { item: HTMLElement; button: HTMLElement } {
			const item = document.createElement('div');
			item.className = 'unotoolbutton';
			const button = document.createElement('button');
			item.appendChild(button);
			JSDialog.SynchronizeDisabledState(item, [button]);
			return { item, button };
		}

		it('says it is greyed out when the state arrives', async function () {
			const { item, button } = toolItem();
			item.setAttribute('disabled', '');
			await settle();
			nodeassert.strictEqual(button.getAttribute('aria-disabled'), 'true');
			nodeassert.ok(button.hasAttribute('disabled'));
		});

		it('built by the notebookbar builder says it is greyed out when the state arrives', async function () {
			const map = setUpDialogPage();
			const container = document.createElement('div');
			document.body.appendChild(container);
			try {
				const builder = window.L.control.notebookbarBuilder({
					windowId: WindowId.Notebookbar,
					mobileWizard: {},
					map: map,
					cssClass: 'notebookbar',
					suffix: 'notebookbar',
					callback: () => {},
				});
				builder.build(container, [
					{
						id: 'toolbox',
						type: 'toolbox',
						children: [
							{
								id: 'bold',
								type: 'toolitem',
								text: 'Bold',
								command: '.uno:Bold',
							},
						],
					},
				]);
				drainLayout();
				const item = container.querySelector('.unotoolbutton');
				const button = item.querySelector('button');
				nodeassert.strictEqual(button.getAttribute('aria-disabled'), null);

				item.setAttribute('disabled', '');
				await settle();

				nodeassert.strictEqual(button.getAttribute('aria-disabled'), 'true');
				nodeassert.ok(button.hasAttribute('disabled'));
			} finally {
				container.remove();
				closeAllDialogs();
			}
		});

		it('stops saying so when it is enabled again', async function () {
			const { item, button } = toolItem();
			item.setAttribute('disabled', '');
			await settle();
			item.removeAttribute('disabled');
			await settle();
			nodeassert.strictEqual(button.getAttribute('aria-disabled'), null);
			nodeassert.ok(!button.hasAttribute('disabled'));
		});
	});
});
