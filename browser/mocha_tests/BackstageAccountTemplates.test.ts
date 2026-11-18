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

describe('Backstage account templates', function () {
	function template(id: string, name: string): TemplateData {
		return {
			id: id,
			name: name,
			type: 'impress',
			path: '/accounts/work/presets/shared/template/' + name + '.otp',
			searchText: name.toLowerCase(),
		};
	}

	function sections(
		accountSections: AccountTemplateSection[],
		onClick: (template: TemplateData) => void = () => {},
	): HTMLElement {
		return BackstageTemplates.templateSections({
			userTemplates: [],
			accountSections: accountSections,
			builtInTemplates: [],
			searchQuery: '',
			onClick: onClick,
			onAddTemplate: () => {},
			onManageTemplates: () => {},
			favoriteState: () => ({ isFavorite: false, onToggle: () => {} }),
		});
	}

	function headings(element: HTMLElement): string[] {
		return Array.from(
			element.querySelectorAll('.backstage-template-section-title'),
		).map((title) => title.textContent);
	}

	it('shows the templates of an account between the user templates and the built-in ones', function () {
		const card = template('account-template-work-shared/Report.otp', 'Report');
		const element = sections([
			{ heading: 'From Work cloud', templates: [card] },
		]);

		nodeassert.deepStrictEqual(headings(element), [
			'My templates',
			'From Work cloud',
			'Built-in templates',
		]);
		const cards = element.querySelectorAll(
			'.backstage-account-templates-grid .backstage-template-card',
		);
		nodeassert.strictEqual(cards.length, 1);
		nodeassert.strictEqual(cards[0].id, card.id);
		nodeassert.strictEqual(
			cards[0].querySelector('.template-name').textContent,
			'Report',
		);
	});

	it('opens the template when its card is clicked', function () {
		const card = template('account-template-work-shared/Report.otp', 'Report');
		const clicked: TemplateData[] = [];
		const element = sections(
			[{ heading: 'From Work cloud', templates: [card] }],
			(clickedTemplate) => clicked.push(clickedTemplate),
		);

		(
			element.querySelector(
				'.backstage-account-templates-grid .backstage-template-card',
			) as HTMLElement
		).click();
		nodeassert.deepStrictEqual(clicked, [card]);
	});

	it('shows no heading for an account with no template to show', function () {
		const element = sections([{ heading: 'From Work cloud', templates: [] }]);

		nodeassert.deepStrictEqual(headings(element), [
			'My templates',
			'Built-in templates',
		]);
		nodeassert.strictEqual(
			element.querySelector('.backstage-account-templates-grid'),
			null,
		);
	});

	describe('sections of the stored templates', function () {
		function stored(account: string, name: string): AccountTemplates {
			return {
				account: account,
				templates: [
					{
						id: 'shared/' + name + '.otp',
						name: name,
						type: 'impress',
						path: '/accounts/' + account + '/' + name + '.otp',
					},
				],
			};
		}

		function account(id: string, name: string): CloudProvider {
			return { id: id, kind: 'other', name: name, url: 'https://example.org' };
		}

		it('names each section after its account', function () {
			const result = BackstageTemplates.accountTemplateSections(
				[stored('work', 'Report'), stored('home', 'Letter')],
				[account('home', 'Home cloud'), account('work', 'Work cloud')],
			);

			nodeassert.deepStrictEqual(
				result.map((section) => section.heading),
				['From Work cloud', 'From Home cloud'],
			);
			nodeassert.deepStrictEqual(
				result.map((section) => section.templates.map((t) => t.name)),
				[['Report'], ['Letter']],
			);
		});

		it('gives no section for an account that was removed', function () {
			const result = BackstageTemplates.accountTemplateSections(
				[stored('gone', 'Report'), stored('home', 'Letter')],
				[account('home', 'Home cloud')],
			);

			nodeassert.deepStrictEqual(
				result.map((section) => section.heading),
				['From Home cloud'],
			);
		});

		it('shows an account name with a dollar sign as it is', function () {
			const result = BackstageTemplates.accountTemplateSections(
				[stored('work', 'Report')],
				[account('work', "Cash $& $' cloud")],
			);

			nodeassert.strictEqual(result[0].heading, "From Cash $& $' cloud");
		});

		it('finds a template by the name of its account', function () {
			const result = BackstageTemplates.accountTemplateSections(
				[stored('work', 'Report')],
				[account('work', 'Work cloud')],
			);

			nodeassert.strictEqual(
				result[0].templates[0].searchText,
				'report impress work cloud',
			);
		});
	});
});
