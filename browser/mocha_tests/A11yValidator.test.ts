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

describe('A11yValidator', function () {
	before(function () {
		global.getComputedStyle = window.getComputedStyle.bind(window);
	});

	// The validator reports every problem it finds on the console.
	function reportsFor(content: HTMLElement): string[] {
		const dialog = document.createElement('div');
		dialog.className = 'ui-dialog';
		const title = document.createElement('h2');
		title.className = 'ui-dialog-title';
		title.textContent = 'Character';
		dialog.appendChild(title);
		content.classList.add('ui-dialog-content');
		dialog.appendChild(content);

		const reports: string[] = [];
		const realError = console.error;
		console.error = (message: any) => reports.push(String(message));
		try {
			new A11yValidator().validateDialog(dialog);
		} finally {
			console.error = realError;
		}
		return reports.filter((report) =>
			report.includes(A11yValidatorException.PREFIX),
		);
	}

	it('detects an element that plays a button without being one', function () {
		const content = document.createElement('div');
		const fake = document.createElement('span');
		fake.id = 'something';
		fake.setAttribute('role', 'button');
		fake.textContent = 'Bad Button';
		content.appendChild(fake);

		const reports = reportsFor(content);
		nodeassert.strictEqual(reports.length, 1);
		nodeassert.ok(reports[0].includes("In 'Character' at 'something'"));
		nodeassert.ok(reports[0].includes('native <button>'));
	});

	it('accepts a native button', function () {
		const content = document.createElement('div');
		const button = document.createElement('button');
		button.id = 'ok';
		button.textContent = 'OK';
		content.appendChild(button);

		nodeassert.deepStrictEqual(reportsFor(content), []);
	});

	describe('an image in a dialog', function () {
		// jsdom lays nothing out, so an image says it is on screen through its client rects.
		function shownImage(alt?: string): HTMLImageElement {
			const img = document.createElement('img');
			if (alt !== undefined) img.setAttribute('alt', alt);
			img.getClientRects = () => [{}] as any;
			return img;
		}

		function widgetHolding(img: HTMLImageElement, label?: string): HTMLElement {
			const content = document.createElement('div');
			const widget = document.createElement('div');
			widget.id = 'something';
			if (label) widget.setAttribute('aria-label', label);
			widget.appendChild(img);
			content.appendChild(widget);
			return content;
		}

		it('is reported when it has no alt attribute', function () {
			const reports = reportsFor(widgetHolding(shownImage()));
			nodeassert.strictEqual(reports.length, 1);
			nodeassert.ok(reports[0].includes('is missing alt attribute'));
		});

		it('is reported when its alt is empty and its widget has no label', function () {
			const reports = reportsFor(widgetHolding(shownImage('')));
			nodeassert.strictEqual(reports.length, 1);
			nodeassert.ok(reports[0].includes('parent element lacks label'));
		});

		it('is reported when its alt repeats a label the widget has', function () {
			const img = shownImage('Image description');
			const reports = reportsFor(widgetHolding(img, 'Parent Label'));
			nodeassert.strictEqual(reports.length, 1);
			nodeassert.ok(reports[0].includes('should not duplicate'));
		});

		it('is accepted with an empty alt when the widget has a label', function () {
			const img = shownImage('');
			nodeassert.deepStrictEqual(
				reportsFor(widgetHolding(img, 'Parent Label')),
				[],
			);
		});

		it('is left alone when it is not on screen', function () {
			const img = document.createElement('img');
			nodeassert.deepStrictEqual(reportsFor(widgetHolding(img)), []);
		});
	});

	describe('a form control in the sidebar', function () {
		let sidebar: HTMLElement;

		beforeEach(function () {
			sidebar = document.createElement('div');
			document.body.appendChild(sidebar);
			app.map = Object.assign({}, app.map, {
				sidebar: { getContainer: () => sidebar },
			});
		});

		afterEach(function () {
			sidebar.remove();
		});

		function shownInput(id: string): HTMLInputElement {
			const input = document.createElement('input');
			input.id = id;
			input.getClientRects = () => [{}] as any;
			sidebar.appendChild(input);
			return input;
		}

		function missingLabelReports(): string[] {
			const reports: string[] = [];
			const realError = console.error;
			console.error = (message: any) => reports.push(String(message));
			try {
				new A11yValidator().validateSidebar();
			} finally {
				console.error = realError;
			}
			return reports.filter((report) =>
				report.includes('missing a visible label'),
			);
		}

		it('is reported when nothing labels it', function () {
			shownInput('decimalplaces');
			const reports = missingLabelReports();
			nodeassert.strictEqual(reports.length, 1);
			nodeassert.ok(reports[0].includes("'decimalplaces'"));
		});

		it('is accepted with a label element for it', function () {
			shownInput('decimalplaces');
			const label = document.createElement('label');
			label.setAttribute('for', 'decimalplaces');
			label.textContent = 'Decimal places';
			label.getClientRects = () => [{}] as any;
			sidebar.appendChild(label);
			nodeassert.deepStrictEqual(missingLabelReports(), []);
		});

		it('is accepted when it names the element that labels it', function () {
			const label = document.createElement('span');
			label.id = 'decimalplaces-label';
			label.textContent = 'Decimal places';
			label.getClientRects = () => [{}] as any;
			sidebar.appendChild(label);
			shownInput('decimalplaces').setAttribute(
				'aria-labelledby',
				'decimalplaces-label',
			);
			nodeassert.deepStrictEqual(missingLabelReports(), []);
		});
	});
});
