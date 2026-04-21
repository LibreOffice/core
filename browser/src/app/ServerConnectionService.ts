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
}

class ServerConnectionService {
	public constructor() {
		TileManager.appendAfterFirstTileTask(this.onFirstTileReceived.bind(this));
	}

	// below methods should be sorted in expected order of execution to help understand the init

	public onBasicUI() {
		app.console.debug('ServerConnectionService: onBasicUI');

		app.tableStyles = new TableStylesService();
	}

	public onWopiProps(props: { AIConfigured: boolean; AIModelName: string }) {
		app.console.debug('ServerConnectionService: onWopiProps');

		if (!app.map) {
			app.console.error('ServerConnectionService: missing map reference');
			return;
		}

		app.map.isAIConfigured = !!props.AIConfigured;
		app.map.aiModelName = props.AIModelName || '';
	}

	public onViewSetting(viewSetting: ViewSetting) {
		app.console.debug('ServerConnectionService: onViewSetting');

		if (!app.map) {
			app.console.error('ServerConnectionService: missing map reference');
			return;
		}

		app.map.isAIConfigured = !!viewSetting.aiConfigured;
		app.map.aiRequestTimeout = viewSetting.aiRequestTimeout
			? Math.max(10, Number(viewSetting.aiRequestTimeout))
			: 120;
		app.map.aiModelName = viewSetting.aiModelName || '';

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
