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

/*
 * This file contains service which will coordinate operations which
 * should happen on server connection changes.
 */

interface ViewSetting {
	zoteroAPIKey?: string;
	accessibilityState: boolean;
	signatureCertificate?: string;
	aiConfigured?: boolean;
	aiRequestTimeout?: string;
	aiModelName?: string;
	aiEthicalRating?: string;
	presentationViewMode?: string;
}

class ServerConnectionService {
	public constructor() {
		RenderManager.appendAfterFirstTileTask(this.onFirstTileReceived.bind(this));
	}

	// below methods should be sorted in expected order of execution to help understand the init

	public onBasicUI() {
		app.console.debug('ServerConnectionService: onBasicUI');

		app.tableStyles = new TableStylesService();
	}

	public onWopiProps(props: {
		AIConfigured: boolean;
		AIModelName: string;
		AIEthicalRating: string;
	}) {
		app.console.debug('ServerConnectionService: onWopiProps');

		if (!app.map) {
			app.console.error('ServerConnectionService: missing map reference');
			return;
		}

		app.map.isAIConfigured = !!props.AIConfigured;
		app.map.aiModelName = props.AIModelName || '';
		app.map.aiEthicalRating = props.AIEthicalRating || 'U';
	}

	public onViewSetting(viewSetting: ViewSetting) {
		app.console.debug('ServerConnectionService: onViewSetting');

		if (!app.map) {
			app.console.error('ServerConnectionService: missing map reference');
			return;
		}

		// Remember the view mode the user last used for this document. Only
		// ImpressTileLayer acts on this, so it is harmless for other doc types.
		app.impress.savedViewMode = viewSetting.presentationViewMode ?? null;

		app.map.isAIConfigured = !!viewSetting.aiConfigured;
		app.map.aiRequestTimeout = viewSetting.aiRequestTimeout
			? Math.max(10, Number(viewSetting.aiRequestTimeout))
			: 120;
		app.map.aiModelName = viewSetting.aiModelName || '';
		app.map.aiEthicalRating = viewSetting.aiEthicalRating || 'U';

		// The user just changed the AI provider from the settings dialog. Now
		// that isAIConfigured / aiModelName / aiEthicalRating reflect the new
		// state, give them the payoff.
		if (app.map._aiJustConfigured) {
			app.map._aiJustConfigured = false;
			if (app.map.isAIConfigured) {
				const sidebar = JSDialog.getAIChatSidebar();
				if (sidebar.isVisible()) {
					sidebar.refreshModelAndRating();
				} else {
					const viewTab = document.getElementById('View-tab-label');
					if (viewTab) viewTab.click();
					sidebar.show();
				}
			}
		}

		let zoteroPlugin = app.map.zotero;
		const zoteroAPIKey = viewSetting.zoteroAPIKey;
		if (
			window.zoteroEnabled &&
			zoteroAPIKey &&
			!zoteroPlugin &&
			!window.mode.isSmallScreenDevice()
		) {
			app.console.debug('ServerConnectionService: initialize Zotero plugin');

			zoteroPlugin = window.L.control.zotero(app.map);
			zoteroPlugin.apiKey = zoteroAPIKey;

			app.map.zotero = zoteroPlugin;
			app.map.addControl(zoteroPlugin);

			zoteroPlugin.updateUserID();
		}
	}

	public onSpecializedUI(docType: string) {
		app.console.debug('ServerConnectionService: onSpecializedUI - ' + docType);
		app.map.fire('initializedui');
	}

	/// see _appLoadedConditions in Map.Wopi.js
	public onDocumentLoaded() {
		app.console.debug('ServerConnectionService: onDocumentLoaded');

		if (!app.map._extensions) {
			// Mark synchronously so a re-entry of onDocumentLoaded doesn't
			// kick off a second discovery; loadExtensions replaces this with
			// the real map once each manifest has resolved.
			app.map._extensions = {};
			window.L.loadExtensions(app.map, app.map.getDocType()).then(
				function (exts: { [id: string]: any }) {
					app.map._extensions = exts;
					// Nothing to add to the notebookbar tab or the menubar
					// submenu, the initial render's "no extensions" placeholder
					// is still correct, so skip the rebuild work:
					if (Object.keys(exts).length === 0) return;
					// Both the menubar's Extensions submenu and the
					// notebookbar's Extensions tab build from
					// app.map._extensions; refresh whichever is in use so the
					// just-discovered list shows up.
					if (app.map.menubar) app.map.menubar.refresh();
					// uiManager.notebookbar is the NotebookbarBase wrapper;
					// the JS notebookbar that holds the model + loadTab is on
					// its .impl field (see Control.NotebookbarBase.ts).
					if (
						app.map.uiManager &&
						app.map.uiManager.notebookbar &&
						app.map.uiManager.notebookbar.impl
					) {
						app.map.uiManager.notebookbar.impl.refresh();
					}
				},
			);
		}
	}

	public onFirstTileReceived() {
		app.console.debug('ServerConnectionService: onFirstTileReceived');

		if (!window.mode.isSmallScreenDevice()) {
			// show zotero items if needed
			const zoteroItems = [
				'zoteroaddeditbibliography',
				'zoterocontaineradd',
				'zoterocontainerrefresh',
				'zoteroSetDocPrefs',
				'references-zoterosetdocprefs-break',
			];
			const isWriter = app.map?._docLayer?.isWriter();
			if (isWriter && window.zoteroEnabled && app.map.zotero) {
				app.console.debug('ServerConnectionService: show UI for zotero');
				zoteroItems.forEach((id: string) =>
					app.map.uiManager.notebookbar.showItem(id),
				);
			} else {
				app.console.debug('ServerConnectionService: hide UI for zotero');
				zoteroItems.forEach((id: string) =>
					app.map.uiManager.notebookbar.hideItem(id),
				);
			}
		}

		// initialize notebookbar in core
		app.map.uiManager.initializeLateComponents();
		JSDialog.RefreshScrollables();
	}

	/// only called the first time the sidebar is shown
	public onShowSidebar() {
		app.console.debug('ServerConnectionService: onShowSidebar');
		app.map._docLayer.recalculateZoomOnResize();
	}

	public onNotebookbarInCoreInit() {
		app.console.debug('ServerConnectionService: onNotebookbarInCoreInit');
	}
}
