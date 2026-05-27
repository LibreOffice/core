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
 * Sidebar.FromNotebookbarPanel - sidebar, previously based on core
 *                               now  moved to notebookbar but we need to
 *                               keep sidebar for compact mode. It will reuse
 *                               notebookbar widgets in the core.
 */

enum NotebookbarBasedSidebarId {
	Animations = 'animations',
	Transitions = 'transitions',
}

class SidebarFromNotebookbarPanel extends Sidebar {
	models: Map<string, JSDialogModelState>;

	/// converts notebookbar tab into sidebar-compatible JSON
	convertToModel(id: string, raw: NotebookbarTabContent): JSDialogJSON {
		return {
			id: id,
			jsontype: 'sidebar',
			type: 'container',
			dialogid: '0',
			children: raw,
			vertical: true,
		} as any as JSDialogJSON;
	}

	appendModel(
		id: NotebookbarBasedSidebarId,
		name: string,
		notebookbarTab: NotebookbarTab,
	) {
		this.models.set(id, new JSDialogModelState(name));
		this.models
			.get(id)
			?.fullUpdate(this.convertToModel(name, notebookbarTab.getContent()));
	}

	constructor(map: any) {
		super(map);

		// transitions panel is now in the notebookbar in the core
		this.type = this.allowedJsonType = SidebarType.Notebookbar;
		this.builder?.setWindowId(WindowId.Notebookbar);

		// we need to store all the variants and just copy them to the parent class model if needed
		this.models = new Map<string, JSDialogModelState>();
		this.appendModel(
			NotebookbarBasedSidebarId.Animations,
			'AnimationsSidebar',
			JSDialog.ImpressAnimationTab,
		);
		this.appendModel(
			NotebookbarBasedSidebarId.Transitions,
			'TransitionsSidebar',
			JSDialog.ImpressTransitionTab,
		);

		this.map.off('sidebar', this.onSidebar, this); // from Sidebar class
		this.map.on('customsidebar', this.onSidebar, this);
	}

	onRemove() {
		super.onRemove();
		this.map.off('customsidebar');
	}

	/// apply to the cached model too
	protected onJSUpdate(e: any) {
		var data = e.data;

		if (data.jsontype !== this.allowedJsonType) return false;

		for (const [id, model] of this.models) model.widgetUpdate(data);

		return super.onJSUpdate(e);
	}

	/// apply to the cached model too
	protected onJSAction(e: any) {
		var data = e.data;

		if (data.jsontype !== this.allowedJsonType) return false;

		for (const [id, model] of this.models) model.widgetAction(data);

		return super.onJSAction(e);
	}

	public openTransitionsSidebar() {
		// we need to clean the core based sidebars
		this.closeSidebar();
		this.setupTargetDeck(null);
		// toolbar button highlight is updated in Sidebar.onSidebar once the
		// transitions deck is shown, which also clears the core deck buttons

		this.openSidebar(
			NotebookbarBasedSidebarId.Transitions,
			_('Transitions'),
			this.models.get(NotebookbarBasedSidebarId.Transitions)?.getSnapshot(),
		);
	}

	public openAnimationsSidebar() {
		// we need to clean the core based sidebars
		this.closeSidebar();
		this.setupTargetDeck(null);
		// toolbar button highlight is updated in Sidebar.onSidebar once the
		// animations deck is shown, which also clears the core deck buttons

		this.openSidebar(
			NotebookbarBasedSidebarId.Animations,
			_('Animations'),
			this.models.get(NotebookbarBasedSidebarId.Animations)?.getSnapshot(),
		);
	}

	// reuse Sidebar container
	protected setupContainer(parentContainer?: HTMLElement) {
		this.container = document.getElementById(
			`${this.type}-container`,
		) as HTMLElement;
		this.wrapper = document.getElementById(
			`${this.type}-dock-wrapper`,
		) as HTMLElement;
		this.documentContainer = document.querySelector(
			'#document-container',
		) as HTMLDivElement;
	}

	protected openSidebar(id: string, title: string, content: Array<WidgetJSON>) {
		app.map.fire('customsidebar', {
			data: {
				id: WindowId.Notebookbar,
				jsontype: 'sidebar',
				type: 'container',
				visible: true,
				children: [
					{
						id: id + '-deck',
						type: 'deck',
						enabled: true,
						visible: true,
						text: title,
						name: id + '-deck',
						children: [
							{
								id: id + '-panel',
								name: id + '-panel',
								text: title,
								visible: true,
								enabled: true,
								expanded: true,
								hidden: false,
								type: 'panel',
								children: content,
							},
						],
					},
				],
			},
		});
	}
}

JSDialog.SidebarFromNotebookbarPanel = function (map: any) {
	return new SidebarFromNotebookbarPanel(map);
};
