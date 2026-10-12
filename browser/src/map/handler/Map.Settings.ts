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
	isFrameWindow(source: MessageEventSource | null): boolean;
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
	_remoteCheckersAsked: null as Promise<void> | null,
	_resolveRemoteCheckers: null as (() => void) | null,

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
		this._map.on('doclayerinit', this.askForRemoteCheckers, this);
	},

	removeHooks: function (): void {
		window.L.DomEvent.off(window, 'message', this.onMessage, this);
		this._map.off('commandvalues', this.onCommandValues, this);
		this._map.off('documentsettingsscope', this.sendSettingsScope, this);
		this._map.off('doclayerinit', this.askForSentenceChecking, this);
		this._map.off('doclayerinit', this.askForRemoteCheckers, this);
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

	// The remote sentence checkers, as the engine has them set up, with the
	// same timeout as above: the dialog shows their sections from this.
	askForRemoteCheckers: function (): Promise<void> {
		if (this._remoteCheckersAsked) return this._remoteCheckersAsked;
		this._remoteCheckersAsked = new Promise<void>((resolve) => {
			this._resolveRemoteCheckers = resolve;
			app.socket.sendMessage(
				'commandvalues command=.uno:RemoteGrammarCheckers',
			);
			setTimeout(() => {
				if (!this._resolveRemoteCheckers) return; // answered in time
				this._resolveRemoteCheckers = null;
				this._remoteCheckersAsked = null;
				resolve();
			}, 2000);
		});
		return this._remoteCheckersAsked;
	},

	// Which sentence checking rule packages the engine has. A language with no
	// package installed gets no panel in the dialog. And how the remote
	// checkers are set up.
	onCommandValues: function (e: any): void {
		if (e.commandName === '.uno:SentenceCheckingPackages') {
			app.sentenceCheckingPackages = Array.isArray(e.commandValues)
				? e.commandValues
				: [];
			if (this._resolveSentenceChecking) {
				this._resolveSentenceChecking();
				this._resolveSentenceChecking = null;
			}
		} else if (e.commandName === '.uno:RemoteGrammarCheckers') {
			app.remoteGrammarCheckers = Array.isArray(e.commandValues)
				? e.commandValues
				: [];
			if (this._resolveRemoteCheckers) {
				this._resolveRemoteCheckers();
				this._resolveRemoteCheckers = null;
			}
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
		if (!app.remoteGrammarCheckers) await this.askForRemoteCheckers();

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
			// The remote sentence checkers as the engine runs them, so the
			// dialog can show what is in force and which of it may be changed.
			{
				remote_checkers: JSON.stringify(app.remoteGrammarCheckers || []),
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

	// Every setting, shared ones included, becomes this session's browser
	// settings and is handed to the server, which keeps a copy. The view toggles
	// of each document type are also kept as this browser's preferences, and
	// the open document takes on the ones of its own type at once. The layout
	// and the theme are switched through the same calls their own buttons make,
	// so each is recorded the way the button records it. The zoom and scrolling
	// settings are read as a document opens and go on applying at the next
	// open. The layout goes first: switching it builds the toolbars again and
	// restores the sidebar from the preferences, and the view toggles are then
	// applied to the toolbars that are left.
	applyBrowserSettings: function (
		settings: Record<string, string | number>,
	): void {
		window.prefs.recordStoredBrowserSettings(settings);

		const viewToggles: Record<string, string> = {};
		for (const [key, value] of Object.entries(settings)) {
			const group = key.substring(0, key.indexOf('.'));
			if (this._docTypeSettingGroups.includes(group))
				viewToggles[key] = String(value);
		}
		window.prefs.setMultiple(viewToggles);

		const uiManager = this._map.uiManager;
		let layoutSwitched = false;
		if (settings.compactMode !== undefined) {
			const mode = settings.compactMode === 'true' ? 'classic' : 'notebookbar';
			layoutSwitched = mode !== uiManager.getCurrentMode();
			if (layoutSwitched)
				uiManager.onChangeUIMode({ mode: mode, force: false });
		}
		if (settings.darkTheme !== undefined) {
			const dark = settings.darkTheme === 'true';
			if (dark !== window.prefs.getBoolean('darkTheme'))
				uiManager.applyDarkMode(dark, true);
		}
		uiManager.applyViewPreferences(layoutSwitched);
	},

	onMessage: function (e: MessageEvent): void {
		// Only the settings dialog and this window itself send these:
		if (
			e.source !== window &&
			!(this._iframeDialog && this._iframeDialog.isFrameWindow(e.source))
		)
			return;
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
