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
/* global _ */

/*
 * The stops of the onboarding tour, in the order the user walks through them.
 * One more stop is one more entry in the list below.
 *
 * The list is built on every call so that the strings are translated into the
 * language this view loaded with.
 */

function getOnboardingSteps(): OnboardingStep[] {
	return [
		{
			id: 'intro',
			title: _('Welcome to Collabora Online'),
			text: _(
				'This short tour points out the parts of the screen you will use most.',
			),
		},
		{
			id: 'document-name',
			title: _('The document name'),
			text: _(
				'Select the name to rename the document. Every change is saved as you work.',
			),
			target: '#document-titlebar .document-title',
			isAvailable: () => !window.ThisIsAMobileApp,
		},
		{
			id: 'tabs',
			title: _('The tabs'),
			text: _(
				'Each tab gathers the tools for one kind of work, such as inserting or reviewing.',
			),
			target: '.notebookbar-tabs-container',
			uiModes: ['notebookbar'],
		},
		{
			id: 'menubar',
			title: _('The menus'),
			text: _('Every command of the editor is in one of these menus.'),
			target: '#main-menu',
			uiModes: ['classic'],
		},
		{
			id: 'toolbar',
			title: _('The tools'),
			text: _('The buttons of the tab you selected appear along this row.'),
			target: '.notebookbar-scroll-wrapper',
			uiModes: ['notebookbar'],
		},
		{
			id: 'classic-toolbar',
			title: _('The toolbar'),
			text: _('The buttons you need most often sit here, under the menus.'),
			target: '#toolbar-up',
			uiModes: ['classic'],
		},
		{
			id: 'formulabar',
			title: _('The formula bar'),
			text: _(
				'The contents of the selected cell appear here. Type an equals sign to start a formula.',
			),
			target: '#formulabar-row',
			docTypes: ['spreadsheet'],
		},
		{
			id: 'slides',
			title: _('The slide panel'),
			text: _(
				'Every slide or page of the document is listed here. Select one to work on it.',
			),
			target: '#navigation-sidebar',
			anchor: 'top end',
			docTypes: ['presentation', 'drawing'],
		},
		{
			id: 'sidebar',
			title: _('The sidebar'),
			text: _(
				'The sidebar holds the settings for whatever you have selected, such as text, a shape or a cell.',
			),
			target: '#sidebar-dock-wrapper',
			anchor: 'top start',
		},
		{
			id: 'collaborators',
			title: _('The people in the document'),
			text: _('See who else is editing and share the document.'),
			target: () =>
				OnboardingTourController.firstOnScreen([
					'#userListSummaryButton',
					'#ShareAs',
					'#userListHeader',
				]),
		},
		{
			id: 'statusbar',
			title: _('The status bar'),
			text: _(
				'The state of the document, the language of the text and the zoom level are along the bottom.',
			),
			target: '#toolbar-down',
			anchor: 'above',
		},
		{
			id: 'outro',
			title: _('You are ready to work'),
			text: _('You can walk through this tour again from the Help menu.'),
		},
	];
}
