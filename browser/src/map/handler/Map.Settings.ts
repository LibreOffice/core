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
	_sentenceCheckingAsked: null as Promise<void> | null,
	_resolveSentenceChecking: null as (() => void) | null,

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
		this._map.on('commandvalues', this.onCommandValues, this);
		this._map.on('documentsettingsscope', this.sendSettingsScope, this);
		// Asked for once the document is up, so the answer is in hand by the
		// time anyone opens the dialog.
		this._map.on('doclayerinit', this.askForSentenceChecking, this);
	},

	removeHooks: function (): void {
		window.L.DomEvent.off(window, 'message', this.onMessage, this);
		this._map.off('commandvalues', this.onCommandValues, this);
		this._map.off('documentsettingsscope', this.sendSettingsScope, this);
		this._map.off('doclayerinit', this.askForSentenceChecking, this);
	},

	// Resolves when the engine answers, so the dialog can wait for a late
	// reply rather than opening with an empty list.
	askForSentenceChecking: function (): Promise<void> {
		if (this._sentenceCheckingAsked) return this._sentenceCheckingAsked;
		this._sentenceCheckingAsked = new Promise<void>((resolve) => {
			this._resolveSentenceChecking = resolve;
			app.socket.sendMessage(
				'commandvalues command=.uno:SentenceCheckingPackages',
			);
			// The answer may never come. Give up rather than hold the dialog
			// shut.
			setTimeout(() => {
				if (!this._resolveSentenceChecking) return; // answered in time
				this._resolveSentenceChecking = null;
				this._sentenceCheckingAsked = null;
				resolve();
			}, 2000);
		});
		return this._sentenceCheckingAsked;
	},

	// Which sentence checking rule packages the engine has. A language with no
	// package installed gets no panel in the dialog.
	onCommandValues: function (e: any): void {
		if (e.commandName !== '.uno:SentenceCheckingPackages') return;
		app.sentenceCheckingPackages = Array.isArray(e.commandValues)
			? e.commandValues
			: [];
		if (this._resolveSentenceChecking) {
			this._resolveSentenceChecking();
			this._resolveSentenceChecking = null;
		}
	},

	removeIframe: function (): void {
		if (this._iframeDialog) this._iframeDialog.remove();
	},

	/**
	 * target: id of the element to scroll into the view when the dialog shows up
	 */
	showSettingsDialog: async function (target: string): Promise<void> {
		// Normally answered long before anyone opens this. An answer that
		// never came is asked for again here, rather than leaving the
		// sentence checker out of the dialog for the rest of the session.
		if (!app.sentenceCheckingPackages) await this.askForSentenceChecking();

		if (this._iframeDialog && this._iframeDialog.hasLoaded())
			this.removeIframe();

		// The dialog fills the Interface Settings in from the stored
		// browsersetting.json. Preference changes are batched before they are sent
		// there, so send what is still waiting and the dialog opens on the same
		// values the toolbar toggles show.
		window.prefs.sendPendingBrowserSettingsUpdate();

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
			{ scroll_target: target },
			// Which sentence checking rule packages the engine has, so the
			// dialog offers a panel for each and none for the rest.
			{
				sentence_checking: JSON.stringify(app.sentenceCheckingPackages || []),
			},
			// Whether the document settings this document is running with are
			// this user's own, and whether a change made now would be felt
			// here at all, which is what the dialog explains.
			{ user_presets_applied: app.userPresetsApplied !== false },
			{ document_settings_live: app.documentSettingsLive !== false },
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

	// People come and go while the dialog is open, and what a change here
	// would reach changes with them. Tell the dialog, so its note is about
	// the document as it is now.
	sendSettingsScope: function (): void {
		if (!this._iframeDialog || !this._iframeDialog.hasLoaded()) return;
		this._iframeDialog.postMessage({
			MessageId: 'settings-scope',
			user_presets_applied: app.userPresetsApplied !== false,
			document_settings_live: app.documentSettingsLive !== false,
		});
	},

	// The document types browsersetting.json groups the per-document view toggles
	// under.
	_docTypeSettingGroups: ['text', 'spreadsheet', 'presentation', 'drawing'],

	/**
	 * Takes on the Interface Settings the dialog has just saved. The view toggles
	 * of a document type are read when a document opens, so recording them here
	 * keeps this session, localStorage and the stored browsersetting.json on the
	 * same values. The settings shared by every document type (theme, layout,
	 * zoom) have live UI of their own and go on applying at the next open only.
	 * The comments are switched over through the same call the Show Comments
	 * button makes, so that choice takes effect without a reload.
	 */
	applyBrowserSettings: function (settings: Record<string, string>): void {
		const viewToggles: Record<string, string> = {};
		for (const [key, value] of Object.entries(settings)) {
			const group = key.substring(0, key.indexOf('.'));
			if (this._docTypeSettingGroups.includes(group)) viewToggles[key] = value;
		}
		window.prefs.setMultiple(viewToggles);

		const saved = viewToggles[this._map.getDocType() + '.ShowAnnotations'];
		if (saved === undefined) return;

		const handler = this._map['stateChangeHandler'];
		const state = handler.getItemValue('showannotations');
		const shown = state === 'true' || state === true;
		const show = saved === 'true';
		if (show !== shown) this._map.showComments(show);
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
			// A document reads the document settings when it opens, so ask the
			// server to read them again. It does that only when this is the
			// one session on the document: with anyone else on it, the
			// settings in force are whoever opened it's and pulling them out
			// from under the others would be worse than waiting. The apps
			// apply the file as the shell writes it and have no such command.
			if (data.documentSettings && !window.ThisIsAMobileApp)
				app.socket.sendMessage('reloadconfig');
			if (data.browserSettings) this.applyBrowserSettings(data.browserSettings);
			// updateviewsettings applies these to the session (e.g. AI credentials
			// so the AI assistant can authenticate). The apps persist settings
			// separately, through the native bridge.
			if (data.viewSettings) {
				app.socket.sendMessage(
					'updateviewsettings ' + JSON.stringify(data.viewSettings),
				);
			}
			// On the desktop app every provider request authenticates with the
			// API key, so a provider saved without one cannot answer and the
			// AI sidebar would only show failing requests. Point at the
			// missing key instead of opening the sidebar. Self-hosted
			// providers that accept keyless requests remain reachable through
			// a server, where the key stays optional.
			if (
				window.mode.isCODesktop() &&
				data.aiJustConfigured &&
				data.aiKeyMissing
			) {
				app.map.uiManager.showSnackbar(
					_('Settings saved. Add an API key to use the AI assistant.'),
				);
				app.map._aiJustConfigured = false;
				return;
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
