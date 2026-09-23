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
 * Notebookbar.AIAssistantTab.ts - the AI Assistant tab of the notebookbar.
 */

interface AIQuickAction {
	id: string;
	label: string;
	icon: string;
	combination: string;
	prompt?: string; // runs against the selection
	command?: string; // or drives a flow of the sidebar's own
}

interface AIQuickActionGroup {
	id: string;
	actions: AIQuickAction[];
	name?: string;
	combination?: string;
}

class AIAssistantTab implements NotebookbarTab {
	public getName(): string {
		return 'AIAssistant';
	}

	public getEntry(): NotebookbarTabEntry {
		return {
			id: this.getName() + '-tab-label',
			text: _('AI Assistant'),
			name: this.getName(),
			// No context of its own, so a context change must not switch away.
			keepSelected: true,
			accessibility: {
				focusBack: true,
				combination: 'Q',
			},
		} as NotebookbarTabEntry;
	}

	public getQuickActions(): AIQuickAction[] {
		return this.getGroups().reduce(function (all: AIQuickAction[], group) {
			return all.concat(group.actions);
		}, []);
	}

	private getGroups(): AIQuickActionGroup[] {
		const action = this.getActions();

		switch (app.map.getDocType()) {
			case 'text':
				return [
					{
						id: 'aiassistant-text',
						actions: [
							action.shorten,
							action.formalize,
							action.casualize,
							action.summarize,
							action.expand,
							action.fixgrammar,
						],
					},
				];
			case 'presentation':
				return [
					{
						id: 'aiassistant-text',
						name: _('Text Tools'),
						combination: 'TT',
						actions: [
							action.shorten,
							action.formalize,
							action.casualize,
							action.summarize,
							action.expand,
							action.fixgrammar,
							action.makebullets,
						],
					},
					{
						id: 'aiassistant-slide',
						name: _('Slide Tools'),
						combination: 'ST',
						actions: [
							action.createslides,
							action.generateimage,
							action.speakernotes,
						],
					},
				];
			case 'spreadsheet':
				return [
					{
						id: 'aiassistant-formula',
						name: _('Formula Tools'),
						combination: 'FT',
						actions: [
							action.createformula,
							action.explainformula,
							action.fixformulaerror,
						],
					},
					{
						id: 'aiassistant-data',
						name: _('Data Tools'),
						combination: 'DT',
						actions: [action.cleanupdata, action.suggestchart],
					},
					{
						id: 'aiassistant-text',
						name: _('Text Tools'),
						combination: 'TT',
						actions: [
							action.formalize,
							action.summarize,
							action.expand,
							action.fixgrammar,
						],
					},
				];
			default:
				return [];
		}
	}

	// One definition per action, so the apps that share one share its label,
	// icon and prompt.
	private getActions(): { [id: string]: AIQuickAction } {
		return {
			shorten: {
				id: 'shorten',
				label: _('Shorten'),
				icon: 'lc_aichat_shorten.svg',
				prompt: _('Rewrite the selected text so it is shorter.'),
				combination: 'SH',
			},
			formalize: {
				id: 'formalize',
				label: _('Formalize'),
				icon: 'lc_aichat_formalize.svg',
				prompt: _('Rewrite the selected text in a formal tone.'),
				combination: 'FO',
			},
			casualize: {
				id: 'casualize',
				label: _('Casualize'),
				icon: 'lc_aichat_casualize.svg',
				prompt: _('Rewrite the selected text in a casual, friendly tone.'),
				combination: 'CA',
			},
			summarize: {
				id: 'summarize',
				label: _('Summarize'),
				icon: 'lc_aichat_summarize.svg',
				prompt: _('Summarize the key points of the selected text.'),
				combination: 'SU',
			},
			expand: {
				id: 'expand',
				label: _('Expand'),
				icon: 'lc_aichat_expand.svg',
				prompt: _('Expand the selected text and add more detail.'),
				combination: 'EX',
			},
			fixgrammar: {
				id: 'fixgrammar',
				label: _('Fix Grammar'),
				icon: 'lc_aichat_fixgrammar.svg',
				prompt: _(
					'Fix grammar, spelling, and punctuation errors in the selected text.',
				),
				combination: 'FG',
			},
			makebullets: {
				id: 'makebullets',
				label: _('Make Bullets'),
				icon: 'lc_aichat_makebullets.svg',
				prompt: _('Rewrite the selected text as short bullet points.'),
				combination: 'MB',
			},
			createslides: {
				id: 'createslides',
				label: _('Create Slides'),
				icon: 'lc_aichat_createslides.svg',
				command: 'aichatcreateslides',
				combination: 'CS',
			},
			generateimage: {
				id: 'generateimage',
				label: _('Generate Image'),
				icon: 'lc_insertgraphic.svg',
				command: 'aichatgenerateimage',
				combination: 'GI',
			},
			speakernotes: {
				id: 'speakernotes',
				label: _('Generate Speaker Notes'),
				icon: 'lc_notesmode.svg',
				command: 'aichatspeakernotes',
				combination: 'SN',
			},
			createformula: {
				id: 'createformula',
				label: _('Create Formula'),
				icon: 'lc_aichat_createformula.svg',
				command: 'aichatcreateformula',
				combination: 'CF',
			},
			explainformula: {
				id: 'explainformula',
				label: _('Explain Formula'),
				icon: 'lc_aichat_explainformula.svg',
				prompt: _('Explain the formula in the selected cell.'),
				combination: 'EF',
			},
			fixformulaerror: {
				id: 'fixformulaerror',
				label: _('Fix Formula Error'),
				icon: 'lc_aichat_fix_formula.svg',
				command: 'helpfixformulaerror',
				combination: 'FE',
			},
			cleanupdata: {
				id: 'cleanupdata',
				label: _('Clean Up Data'),
				icon: 'lc_aichat_cleanupdata.svg',
				command: 'aichatcleanupdata',
				combination: 'CD',
			},
			suggestchart: {
				id: 'suggestchart',
				label: _('Suggest Chart'),
				icon: 'lc_drawchart.svg',
				prompt: _('Suggest a chart for the selected data.'),
				combination: 'SC',
			},
		};
	}

	public getContent(): NotebookbarTabContent {
		const content: WidgetJSON[] = [
			{
				id: 'aiassistant-open-sidebar',
				type: 'bigcustomtoolitem',
				text: _('AI Assistant'),
				tooltip: _('AI Assistant'),
				command: 'aichat',
				icon: 'lc_ai_sidebar.svg',
				accessibility: { focusBack: true, combination: 'OA', de: null },
			} as ToolItemWidgetJSON,
		];

		// Without a provider the tab is just the way in to the setup dialog;
		// the actions show up once ServerConnectionService rebuilds us.
		const groups = app.map.isAIConfigured ? this.getGroups() : [];
		if (!groups.length) {
			// The tab wrapper is only built for more than one child - same pin
			// as Control.Notebookbar.js getExtensionsTab() needs.
			content.push({ id: 'aiassistant-tail-pin', type: 'spacer' });
			return content as NotebookbarTabContent;
		}

		content.push({
			type: 'separator',
			id: 'aiassistant-open-sidebar-break',
			orientation: 'vertical',
		} as SeparatorWidgetJSON);

		groups.forEach((group, index) => {
			if (index)
				content.push({
					type: 'separator',
					id: group.id + '-break',
					orientation: 'vertical',
				} as SeparatorWidgetJSON);

			const items = group.actions.map(this.toolItem);
			if (!group.name) {
				content.push(...items);
				return;
			}
			content.push({
				id: group.id,
				type: 'overflowgroup',
				name: group.name,
				accessibility: { focusBack: false, combination: group.combination },
				children: items,
			} as OverflowGroupWidgetJSON);
		});

		return content as NotebookbarTabContent;
	}

	private toolItem(action: AIQuickAction): ToolItemWidgetJSON {
		return {
			id: 'aiassistant-' + action.id,
			type: 'bigcustomtoolitem',
			text: action.label,
			tooltip: action.label,
			command: action.command || 'aichatquick-' + action.id,
			icon: action.icon,
			accessibility: {
				focusBack: true,
				combination: action.combination,
				de: null,
			},
		} as ToolItemWidgetJSON;
	}
}

JSDialog.AIAssistantTab = new AIAssistantTab();
