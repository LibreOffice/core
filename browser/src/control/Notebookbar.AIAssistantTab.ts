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

// One button that asks the assistant for a canned rewrite of the selection.
interface AIQuickAction {
	id: string;
	label: string;
	icon: string;
	prompt: string; // sent to the assistant as the user's message
	combination: string; // accessibility shortcut, unique within the tab
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
			accessibility: {
				focusBack: true,
				combination: 'Q',
			},
		} as NotebookbarTabEntry;
	}

	// Rewrites of the selected text, so text documents only for now; another
	// document type wanting its own would return its own list here.
	// docdispatcher registers 'aichatquick-' + id for each of them.
	public getQuickActions(): AIQuickAction[] {
		if (app.map.getDocType() !== 'text') return [];
		return [
			{
				id: 'shorten',
				label: _('Shorten'),
				icon: 'lc_aichat_shorten.svg',
				prompt: _('Rewrite the selected text so it is shorter.'),
				combination: 'SH',
			},
			{
				id: 'formalize',
				label: _('Make Formal'),
				icon: 'lc_aichat_formalize.svg',
				prompt: _('Rewrite the selected text in a formal tone.'),
				combination: 'FO',
			},
			{
				id: 'casualize',
				label: _('Make Casual'),
				icon: 'lc_aichat_casualize.svg',
				prompt: _('Rewrite the selected text in a casual, friendly tone.'),
				combination: 'CA',
			},
			{
				id: 'summarize',
				label: _('Summarize'),
				icon: 'lc_aichat_summarize.svg',
				prompt: _('Summarize the key points of the selected text.'),
				combination: 'SU',
			},
			{
				id: 'expand',
				label: _('Expand'),
				icon: 'lc_aichat_expand.svg',
				prompt: _('Expand the selected text and add more detail.'),
				combination: 'EX',
			},
			{
				id: 'fixgrammar',
				label: _('Fix Grammar'),
				icon: 'lc_aichat_fixgrammar.svg',
				prompt: _(
					'Fix grammar, spelling, and punctuation errors in the selected text.',
				),
				combination: 'FG',
			},
		];
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
		// the rewrites show up once ServerConnectionService rebuilds us.
		const actions = app.map.isAIConfigured ? this.getQuickActions() : [];
		if (!actions.length) {
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

		for (const action of actions) {
			content.push({
				id: 'aiassistant-' + action.id,
				type: 'bigcustomtoolitem',
				text: action.label,
				tooltip: action.label,
				command: 'aichatquick-' + action.id,
				icon: action.icon,
				accessibility: {
					focusBack: true,
					combination: action.combination,
					de: null,
				},
			} as ToolItemWidgetJSON);
		}

		return content as NotebookbarTabContent;
	}
}

JSDialog.AIAssistantTab = new AIAssistantTab();
