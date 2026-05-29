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
 * L.Map.Settings.
 */

interface IFrameDialog {
	remove(): void;
	hasLoaded(): boolean;
	postMessage(message: any): void;
	show(): void;
}

window.L.Map.mergeOptions({
	settings: true,
});

window.L.Map.Settings = window.L.Handler.extend({
	_iframeDialog: null as IFrameDialog | null,
	_url: '',

	_getLocalSettingsUrl: function (): string {
		const settingsLocation: string = app.LOUtil.getURL(
			'adminIntegratorSettings.html',
		);
		return settingsLocation;
	},

	initialize: function (map: any): void {
		window.L.Handler.prototype.initialize.call(this, map);

		this._url = this._getLocalSettingsUrl();
	},

	addHooks: function (): void {
		window.L.DomEvent.on(window, 'message', this.onMessage, this);
	},

	removeHooks: function (): void {
		window.L.DomEvent.off(window, 'message', this.onMessage, this);
	},

	removeIframe: function (): void {
		if (this._iframeDialog) this._iframeDialog.remove();
	},

	showSettingsDialog: function (): void {
		if (this._iframeDialog && this._iframeDialog.hasLoaded())
			this.removeIframe();

		const theme = window.prefs.getBoolean('darkTheme') ? 'dark' : 'light';

		const params: Array<Record<string, any>> = [
			{ ui_theme: theme },
			{ lang: window.langParam },
			{ mobile: window.mode.isSmallScreenDevice() },
			{ access_token: window.accessToken },
			{ access_token_ttl: window.accessTokenTTL },
			{ wopi_setting_base_url: window.wopiSettingBaseUrl },
			{ disable_ai_settings: this._map.wopi.DisableAISettings },
			{ show_left_nav: true },
		];

		const options = {
			prefix: 'iframe-settings',
			titlebar: _('Options'),
			modalButtons: [
				{
					id: 'iframe-settings-cancel',
					text: _('Cancel'),
					align: 'right',
				},
				{
					id: 'iframe-settings-save',
					text: _('Save'),
					align: 'right',
				},
			],
			dialogCssClass:
				'jsdialog-container ui-dialog lokdialog_container ui-widget-content',
			method: window.socketProxy ? 'post' : 'get',
		};

		this._iframeDialog = window.L.iframeDialog(
			this._url,
			params,
			null,
			options,
		);

		const cancelButton = document.getElementById('iframe-settings-cancel');
		const saveButton = document.getElementById('iframe-settings-save');

		window.L.DomEvent.on(
			cancelButton,
			'click',
			() => {
				this.removeIframe();
			},
			this,
		);

		window.L.DomEvent.on(
			saveButton,
			'click',
			() => {
				this._iframeDialog.postMessage({
					MessageId: 'settings-save-all',
				});
			},
			this,
		);
	},

	onMessage: function (e: MessageEvent): void {
		if (typeof e.data !== 'string') return; // Some extensions may inject scripts resulting in load events that are not strings
		const data = JSON.parse(e.data);

		if (data.MessageId === 'settings-show') {
			this._iframeDialog.show();
		} else if (data.MessageId === 'settings-cancel') {
			this.removeIframe();
		} else if (data.MessageId === 'settings-ready') {
			this._iframeDialog.postMessage(data);
		} else if (data.MessageId === 'settings-save-complete') {
			this.removeIframe();
			// updateviewsettings applies these to the session (e.g. AI credentials
			// so the AI assistant can authenticate). The apps persist settings
			// separately, through the native bridge.
			if (data.viewSettings) {
				app.socket.sendMessage(
					'updateviewsettings ' + JSON.stringify(data.viewSettings),
				);
			}
			app.map.uiManager.showSnackbar(_('Settings saved'));
			// Defer the View-tab / AI-sidebar payoff until isAIConfigured is
			// updated from the viewsetting: reply (see ServerConnectionService).
			app.map._aiJustConfigured = !!data.aiJustConfigured;
		}
	},
});

if (window.prefs.canPersist) {
	window.L.Map.addInitHook('addHandler', 'settings', window.L.Map.Settings);
}
