/* eslint-disable */
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

interface StringConstructor {
	defaultLocale: string;
	locale: string;
}
var _: any = (s) => s.toLocaleString();

/**
 * target: id of the element to scroll into the view when the dialog shows up
 */
interface Window {
	accessToken?: string;
	accessTokenTTL?: string;
	enableAccessibility?: boolean;
	enableDebug?: boolean;
	disableAISettings?: boolean;
	wopiSettingBaseUrl?: string;
	sentenceCheckingPackages?: Array<{ id: string; locale: string }>;
	// The remote sentence checkers as the engine runs them.
	remoteCheckers?: Array<RemoteCheckerInfo>;
	iframeType?: string;
	cssVars?: string;
	serviceRoot?: string;
	versionHash?: string;
	showLeftNav?: boolean;
	scrollTarget?: string;
	// False when this document is running with another user's document
	// settings, because they opened it first.
	userPresetsApplied?: boolean;
	// False when a change made now would not be felt in this document.
	documentSettingsLive?: boolean;
	settingIframe?: SettingIframe;

	WordBook?: WordBook;

	Xcu: typeof Xcu;
	SettingIframe: typeof SettingIframe;
}

// What the engine says about one remote sentence checker: how it is set
// up, whether the user may change that, and what its server offers. The API
// key never comes; hasCredentials says there is one.
interface RemoteCheckerInfo {
	id: 'languagetool' | 'duden' | 'grammalecte';
	enabled: boolean;
	baseURL: string;
	username?: string;
	hasCredentials: boolean;
	sslVerify: boolean;
	allowUserSettings: boolean;
	checksSpelling: boolean;
	locales: string[];
	spellcheckingLevel?: number;
	orthographyStandard?: string;
	messageLevel?: number;
	propertySets?: string[];
	dictionaries?: string[];
	availablePropertySets?: string[];
	availableDictionaries?: string[];
	// The switches the user chooses, as the engine runs them: the Duden style
	// checks by their names, the Grammalecte rule families by the server's ids.
	style?: Record<string, boolean>;
	options?: Record<string, boolean>;
}

interface ConfigItem {
	stamp: string;
	uri: string;
}

interface ConfigData {
	kind: 'shared' | 'user';
	autotext: ConfigItem[] | null;
	wordbook: ConfigItem[] | null;
	browsersetting: ConfigItem[] | null;
	viewsetting: ConfigItem[] | null;
	xcu: ConfigItem[] | null;
	themes: ConfigItem[] | null;
	extensions: ConfigItem[] | null;
	spif: ConfigItem[] | null;
	serverprivateinfo: ConfigItem[] | null;
}

interface ServerPrivateSettings {
	ESignatureBaseUrl: string;
	ESignatureClientId: string;
	ESignatureSecret: string;
	// True when the server holds a secret it did not send down. Travels with the
	// file but is not part of it.
	ESignatureSecretStored?: boolean;
}

interface ViewSettings {
	zoteroAPIKey: string;
	signatureCert: string;
	signatureKey: string;
	signatureCa: string;
	aiProviderURL: string;
	aiProviderAPIKey: string;
	aiProviderModel: string;
	aiImageProviderAPIKey: string;
	aiImageProviderURL: string;
	aiImageModel: string;
	aiImageSize: string;
	aiRequestTimeout: string;
	// Companion flags for the secret fields above. True when the server holds a
	// value it did not send to the browser. Sent back as true to ask that the
	// stored value be kept unchanged.
	zoteroAPIKeyStored?: boolean;
	signatureKeyStored?: boolean;
	aiProviderAPIKeyStored?: boolean;
	aiImageProviderAPIKeyStored?: boolean;
}

// Secret fields the server holds but never sends to the browser in cleartext.
// Each pairs with a "<field>Stored" flag: true on the way down when a value is
// saved, and true on the way back when the field was left untouched and the
// saved value stays. Kept in sync with common/ViewSettings.hpp and
// common/ServerPrivateInfo.hpp on the server.
const SECRET_SETTING_FIELDS = [
	'aiProviderAPIKey',
	'aiImageProviderAPIKey',
	'zoteroAPIKey',
	'signatureKey',
	'ESignatureSecret',
];

const SECRET_FIELDS_WITH_DELETE = ['aiProviderAPIKey', 'aiImageProviderAPIKey'];

// Fills the locked field of a saved key. It is never sent: the key itself is
// not in the browser, and the field is disabled while it shows this.
const STORED_SECRET_MASK = '*'.repeat(16);

interface AIProvider {
	id: string;
	name: string;
	baseUrl: string;
	isCustom?: boolean;
}

// Outcome of saveAll, reported back to the parent. aiJustConfigured means the
// user configured a chat AI provider in this save, so the View tab / AI sidebar
// payoff should fire once the settings are applied. aiKeyMissing means the
// saved chat provider has no API key.
interface SaveAllResult {
	aiJustConfigured: boolean;
	aiKeyMissing: boolean;
	// Whether the document settings were written. A document reads those when
	// it opens, so the one in front of the user can only take them by being
	// told to read them again.
	documentSettings: boolean;
	// The view settings as the user entered them, captured before the upload
	// re-fetch redacts the in-memory copy. Applied to the live session so a
	// freshly entered secret is not lost.
	viewSettings: ViewSettings;
	// The Interface Settings as they were written to browsersetting.json, with
	// flat dotted keys ("text.ShowAnnotations": "false"). A toggle is the string
	// "true" or "false", the zoom index stays a number. Null when the dialog
	// shows no Interface Settings section.
	browserSettings: Record<string, string | number> | null;
}

// Visual state of an AI model-fetch status line. 'hidden' (or an empty message)
// clears it; the rest map to the alert styling in adminIntegratorSettings.css.
type AIStatusState = 'info' | 'loading' | 'success' | 'error' | 'hidden';

interface SectionConfig {
	id: string;
	sectionTitle: string;
	sectionDesc: string;
	listId: string;
	inputId: string;
	buttonId: string;
	fileAccept: string;
	buttonText: string;
	uploadPath: string;
	enabledFor?: string;
	debugOnly?: boolean;
	element: HTMLElement | null;
}

const initTranslationStr = () => {
	const element = document.getElementById('initial-variables');
	const rawLang = (element as HTMLInputElement).dataset.lang;
	// Unsubstituted "%UI_LANG%" reaches us when the desktop loads the
	// static template; fall back to the URL ?lang= that Map.Settings
	// appends.
	const urlLang = new URLSearchParams(window.location.search).get('lang');
	const lang =
		rawLang && !/^(?:%.+%|<!--%.+%-->)$/.test(rawLang)
			? rawLang
			: urlLang && urlLang !== 'undefined'
				? urlLang
				: 'en-US';
	document.documentElement.lang = lang;

	String.defaultLocale = 'en-US';
	String.locale = lang;
};

const getIntegratorOrigin = (): string => {
	try {
		if (window.wopiSettingBaseUrl) {
			return new URL(window.wopiSettingBaseUrl).origin;
		}
	} catch (e) {
		console.warn('Invalid wopiSettingBaseUrl, falling back to window.origin');
	}
	return window.origin;
};

// Target origin for messages we post up to the parent (cool.html). On the
// desktop apps both documents load over file://, but the embedded WebViews
// hand this iframe an opaque "null" origin while the parent stays "file://"
// (most visibly on macOS/WKWebView). A targetOrigin of window.origin ("null")
// then never matches the parent, so the message is silently dropped and the
// settings handshake stalls. The parent is our own trusted local content
// there, so target "*". The online/WOPI path keeps the strict origin.
function parentTargetOrigin(): string {
	return isCODesktop ? '*' : window.origin;
}

const onLoaded = () => {
	window.addEventListener('message', onMessage, false);
	window.parent.postMessage(
		'{"MessageId":"settings-ready"}',
		parentTargetOrigin(),
	);
};

const onMessage = (e) => {
	try {
		const data = JSON.parse(e.data);
		// On the desktop apps the parent (cool.html) and this iframe both
		// load over file://, but WebView2 hands the iframe an opaque "null"
		// origin while the parent stays "file://", so the strict equality
		// never holds and the settings-ready handshake stalls. The iframe is
		// our own trusted local content there, so accept the message.
		const sameOrigin = e.origin === window.origin || isCODesktop;
		if (sameOrigin && window.parent !== window.self) {
			if (data.MessageId === 'settings-ready') {
				window.parent.postMessage(
					'{"MessageId":"settings-show"}',
					parentTargetOrigin(),
				);
			} else if (data.MessageId === 'settings-scope') {
				// Someone joined or left the document while this was open.
				window.userPresetsApplied = data.user_presets_applied !== false;
				window.documentSettingsLive = data.document_settings_live !== false;
				window.settingIframe?.refreshSettingsScope();
			} else if (data.MessageId === 'settings-save-all') {
				const settingIframe = window.settingIframe as SettingIframe;
				if (settingIframe) {
					settingIframe.saveAll().then((result) => {
						window.parent.postMessage(
							JSON.stringify({
								MessageId: 'settings-save-complete',
								// Use the settings as entered (from saveAll), not the
								// re-fetched copy, so a freshly typed key still reaches
								// the live session.
								viewSettings: result.viewSettings,
								browserSettings: result.browserSettings,
								aiJustConfigured: result.aiJustConfigured,
								aiKeyMissing: result.aiKeyMissing,
								documentSettings: result.documentSettings,
							}),
							parentTargetOrigin(),
						);
					});
				}
			}
		}
	} catch (err) {
		console.error('Could not process postmessage:', err);
		return;
	}
};

// Zoom keys as per browsersetting.json.
const ZOOM_SETTING_KEYS: Array<string> = ['smartZoom', 'defaultZoom'];

/* 
	`defaultZoom` - index of the default zoom level in ZOOM_LEVELS (when smartZoom is set to false).
				  - used as `index+1` internally as the fist zoom level in the list has the zoom value of 1)
*/
const defaultBrowserSetting: Record<string, any> = {
	compactMode: {
		value: false,
		label: 'Compact layout',
		customType: 'compactToggle',
	},
	darkTheme: false,
	accessibilityState: false,
	lockAccessibilityOn: false,
	smartZoom: true,
	defaultZoom: {
		value: 9,
		label: 'Default Zoom',
		customType: 'zoomDropdown',
	},
	smoothScroll: true,
	spreadsheet: {
		ShowStatusbar: true,
		A11yCheckDeck: false,
		ShowNavigator: false,
		ShowSidebar: true,
		// A spreadsheet of codes and abbreviations produces mostly false
		// positives, so Calc is the one document type this starts off in.
		spellOnline: false,
	},
	text: {
		ShowRuler: false,
		ShowStatusbar: true,
		A11yCheckDeck: false,
		ShowNavigator: false,
		ShowSidebar: true,
		StyleListDeck: false,
		ShowFormattingMarks: false,
		ShowAnnotations: true,
		spellOnline: true,
	},
	presentation: {
		ShowRuler: false,
		ShowStatusbar: true,
		A11yCheckDeck: false,
		ShowNavigator: false,
		ShowSidebar: true,
		SdCustomAnimationDeck: false,
		// SdMasterPagesDeck: false,
		// SdSlideTransitionDeck: false,
		ShowAnnotations: true,
		spellOnline: true,
	},
	drawing: {
		ShowRuler: false,
		ShowStatusbar: true,
		A11yCheckDeck: false,
		ShowNavigator: false,
		ShowSidebar: true,
		ShowAnnotations: true,
		spellOnline: true,
	},
};

// The form is filled in from a copy of the defaults. Its controls write the
// user's choice straight back into the object they were handed, and a custom
// control such as the zoom dropdown is handed the whole {value, label,
// customType} object, so working on the defaults themselves would leave them
// holding the user's choice - or, where the collected value replaces the object,
// no value at all.
function defaultBrowserSettingCopy(): Record<string, any> {
	return JSON.parse(JSON.stringify(defaultBrowserSetting));
}

abstract class SettingsStorage {
	abstract fetchSettingsConfig(): Promise<ConfigData>;
	abstract uploadSettings(
		filePath: string,
		file: File,
		currentFileUrl?: string,
	): Promise<void>;
	abstract fetchSettingFile(fileUrl: string): Promise<string | null>;
	abstract deleteSettingsConfig(fileId: string): Promise<void>;
}

class DesktopSettingsStorage extends SettingsStorage {
	async fetchSettingsConfig(): Promise<ConfigData> {
		const configJson = await (window.parent as any).postMobileCall(
			'FETCHSETTINGSCONFIG',
		);
		return JSON.parse(configJson);
	}

	async uploadSettings(filePath: string, file: File): Promise<void> {
		// The desktop stores settings locally on the user's own machine, so the
		// secret round-trip that the server path uses does not apply here.
		const text = await file.text();
		(window.parent as any).postMobileMessage(
			'UPLOADSETTINGS ' +
				JSON.stringify({
					filePath,
					fileName: file.name,
					mimeType: file.type,
					content: text,
				}),
		);
	}

	async fetchSettingFile(fileUrl: string): Promise<string | null> {
		const result = await (window.parent as any).postMobileCall(
			'FETCHSETTINGSFILE ' + fileUrl,
		);
		return result.content;
	}

	async deleteSettingsConfig(fileId: string): Promise<void> {
		console.warn('Delete settings config not needed on desktop: ' + fileId);
	}
}

class OnlineSettingsStorage extends SettingsStorage {
	private getAPIEndpoints() {
		return {
			uploadSettings: window.serviceRoot + '/browser/dist/upload-settings',

			fetchSharedConfig:
				window.serviceRoot + '/browser/dist/fetch-settings-config',

			deleteSharedConfig:
				window.serviceRoot + '/browser/dist/delete-settings-config',

			fetchSettingFile:
				window.serviceRoot + '/browser/dist/fetch-settings-file',
		};
	}

	private getConfigType(): string {
		return window.iframeType === 'admin' ? 'systemconfig' : 'userconfig';
	}

	async fetchSettingsConfig(): Promise<ConfigData> {
		if (!window.wopiSettingBaseUrl) {
			console.error(_('Shared Config URL is missing in initial variables.'));
			throw new Error('Shared Config URL is missing');
		}
		if (!window.accessToken) {
			console.error(_('Access token is missing in initial variables.'));
			throw new Error('Access token is missing');
		}

		const formData = new FormData();
		formData.append('sharedConfigUrl', window.wopiSettingBaseUrl);
		formData.append('accessToken', window.accessToken);
		formData.append('type', this.getConfigType());

		const response: Response = await fetch(
			this.getAPIEndpoints().fetchSharedConfig,
			{
				method: 'POST',
				headers: {
					Authorization: `Bearer ${window.accessToken}`,
				},
				body: formData,
			},
		);

		if (!response.ok) {
			console.error(
				'something went wrong shared config response',
				await response.text(),
			);
			const error = new Error(
				`Could not fetch shared config: ${response.statusText}`,
			);
			// Carry the status so the caller can tell "not a signed-in user"
			// (a client error from the host) from a genuine server failure.
			(error as any).status = response.status;
			throw error;
		}

		return await response.json();
	}

	async uploadSettings(
		filePath: string,
		file: File,
		currentFileUrl?: string,
	): Promise<void> {
		const formData = new FormData();
		formData.append('file', file);
		formData.append('filePath', filePath);
		if (window.wopiSettingBaseUrl) {
			formData.append('wopiSettingBaseUrl', window.wopiSettingBaseUrl);
		}
		// The URL of the currently stored file, so the server can read back any
		// secret the user chose to keep instead of receiving it from the browser.
		if (currentFileUrl) {
			formData.append('currentFileUrl', currentFileUrl);
		}

		const apiUrl = this.getAPIEndpoints().uploadSettings;

		const response = await fetch(apiUrl, {
			method: 'POST',
			headers: {
				Authorization: `Bearer ${window.accessToken}`,
			},
			body: formData,
		});

		if (!response.ok) {
			throw new Error(`Upload failed: ${response.statusText}`);
		}
	}

	async fetchSettingFile(fileUrl: string): Promise<string | null> {
		try {
			const formData = new FormData();
			formData.append('fileUrl', fileUrl);
			formData.append('accessToken', window.accessToken ?? '');

			const apiUrl = this.getAPIEndpoints().fetchSettingFile;

			const response = await fetch(apiUrl, {
				method: 'POST',
				headers: {
					Authorization: `Bearer ${window.accessToken}`,
				},
				body: formData,
			});

			if (!response.ok) {
				throw new Error(`Upload failed: ${response.statusText}`);
			}

			return await response.text();
		} catch (error) {
			SettingIframe.showErrorModal(
				_(
					'Something went wrong while fetching setting file. Please try to refresh the page.',
				),
			);
			return null;
		}
	}

	async deleteSettingsConfig(fileId: string): Promise<void> {
		if (!window.accessToken) {
			throw new Error('Access token is missing.');
		}
		if (!window.wopiSettingBaseUrl) {
			throw new Error('wopiSettingBaseUrl is missing.');
		}

		const formData = new FormData();
		formData.append('fileId', fileId);
		formData.append('sharedConfigUrl', window.wopiSettingBaseUrl);
		formData.append('accessToken', window.accessToken);

		const response = await fetch(this.getAPIEndpoints().deleteSharedConfig, {
			method: 'POST',
			headers: {
				Authorization: `Bearer ${window.accessToken}`,
			},
			body: formData,
		});

		if (!response.ok) {
			throw new Error(`Delete failed: ${response.statusText}`);
		}
	}
}

let isCODesktop = false;
try {
	isCODesktop = (window as any).parent.mode.isCODesktop();
} catch (e) {
	isCODesktop = false;
}
// Cross-origin file:// access can block window.parent.mode; the iframe
// being loaded over file:// is itself a reliable desktop signal (the
// server path is always HTTP(S)).
if (!isCODesktop && window.location.protocol === 'file:') {
	isCODesktop = true;
}

// The Windows and macOS desktop apps sign with the native certificate store
// (the Windows certificate store and Keychain Access, respectively), so the
// Document Signing certificate/key configuration is not shown there.
let usesNativeCertStore = false;
try {
	usesNativeCertStore = !!(
		(window as any).parent.ThisIsTheWindowsApp ||
		(window as any).parent.ThisIsTheMacOSApp
	);
} catch (e) {
	usesNativeCertStore = false;
}

const DEFAULT_PROVIDER = 'default';

// Sizes the common image models accept. With none saved, the server uses
// ai.image_size from coolwsd.xml, or else the first one here.
const AI_IMAGE_SIZES = [
	'1024x1024',
	'1536x1024',
	'1024x1536',
	'1792x1024',
	'1024x1792',
	'512x512',
	'256x256',
];

let isQtApp = false;
try {
	isQtApp = !!(window as any).parent.ThisIsTheQtApp;
} catch (e) {
	isQtApp = false;
}

// The Accessibility section holds settings that each desktop app applies
// natively. It is offered only where the native side handles the GETUIZOOM
// and SETUIZOOM messages.
const accessibilitySettingsSupported = isQtApp;

// Keep in sync with the pre-canned provider map in wsd/FileServer.cpp
// fetchModels. The server ignores the baseUrl from the client for non-custom
// providers and uses its own copy, so a caller cannot pair a pre-canned id
// with an arbitrary url.
const AI_PROVIDERS: Array<AIProvider> = [
	{
		id: 'openai',
		name: 'OpenAI',
		baseUrl: 'https://api.openai.com',
	},
	{
		id: 'groq',
		name: 'Groq',
		baseUrl: 'https://api.groq.com/openai',
	},
	{
		id: 'together',
		name: 'Together AI',
		baseUrl: 'https://api.together.xyz',
	},
	{
		id: 'mistral',
		name: 'Mistral AI',
		baseUrl: 'https://api.mistral.ai',
	},
	{
		id: 'custom',
		name: 'Custom (OpenAI Compatible)',
		baseUrl: '',
		isCustom: true,
	},
];

const ZOOM_LEVELS: Array<number> = [
	20, 25, 30, 35, 40, 50, 60, 70, 85, 100, 120, 150, 170, 200, 235, 280, 335,
	400,
];

// The user interface zoom percentages offered in the Accessibility section.
// These magnify the whole application window, so the list starts at 100% and
// follows the steps of ZOOM_LEVELS, continued by one more step to 500%.
const UI_ZOOM_LEVELS: Array<number> = [
	100, 120, 150, 170, 200, 235, 280, 335, 400, 500,
];

// Thunks rather than strings: the module loads before the translations do,
// so _() must run when the message is shown, not here.
const AI_ERROR_MESSAGES: Record<number, () => string> = {
	400: () => _('Invalid request'),
	401: () => _('Invalid API key'),
	403: () => _('API key lacks permissions'),
	421: () =>
		_(
			"This AI host is not in the server's allowed AI host list (ai.allowed_hosts). Ask your administrator to permit it.",
		),
	429: () => _('Rate limited - please wait a moment and retry'),
	500: () => _('API server error - try again later'),
	503: () => _('Service temporarily unavailable'),
};

// Some providers add capability metadata to /v1/models entries: Together uses
// `type` ("chat" | "image" | "embedding" | ...), Fireworks uses `kind`
// ("image-generation-model" | "chat-completion-model" | ...), Mistral uses a
// `capabilities` object. Trust those when present; otherwise fall back to
// matching common model-name patterns.
type AIModelEntry = {
	id: string;
	type?: string;
	kind?: string;
	capabilities?: { completion_chat?: boolean };
};

const IMAGE_MODEL_NAME =
	/dall-e|gpt-image|stable-diffusion|sdxl|sd3|flux|imagen|ideogram/i;
const NON_CHAT_MODEL_NAME =
	/embedding|whisper|tts|moderation|rerank|audio|dall-e|gpt-image|stable-diffusion|sdxl|sd3|flux|imagen|ideogram/i;

const isImageModel = (m: AIModelEntry): boolean => {
	if (typeof m.type === 'string') return /^image$/i.test(m.type);
	if (typeof m.kind === 'string') return /image/i.test(m.kind);
	return IMAGE_MODEL_NAME.test(m.id);
};

const isChatModel = (m: AIModelEntry): boolean => {
	if (typeof m.type === 'string') return /^chat$/i.test(m.type);
	if (typeof m.kind === 'string') return /chat/i.test(m.kind);
	if (m.capabilities && typeof m.capabilities === 'object')
		return m.capabilities.completion_chat === true;
	return !NON_CHAT_MODEL_NAME.test(m.id);
};

class SettingIframe {
	private settingsStorage: SettingsStorage;
	private wordbook;
	private xcuEditor;
	private _viewSetting!: ViewSettings;
	// URL of the stored viewsetting.json, remembered when it is fetched so a save
	// can tell the server where to read back any secret the user chose to keep.
	private _viewSettingFileUrl = '';
	private _serverPrivateSetting: ServerPrivateSettings =
		SettingIframe.getDefaultServerPrivateSettings();
	private _serverPrivateSection: HTMLElement | null = null;
	// URL of the stored serverprivateinfo.json, remembered when it is fetched so
	// a save can tell the server where to read back the secret the
	// administrator chose to keep.
	private _serverPrivateSettingFileUrl = '';
	// Set when the user edits a chat AI field in this dialog session. Drives the
	// View-tab / sidebar payoff so it fires on a real change, not on every save
	// that happens to have a key already set. Set only by user input handlers,
	// never by the load-time model auto-fetch.
	private _aiConfigDirty = false;
	private xcuInitializationAttempted = false;
	private serverPrivateInfoInitializationAttempted = false;
	private _aiModelFetchTimeout: number | null = null;
	private _aiModelFetchAbort: AbortController | null = null;
	private _aiModelFetchSeq = 0;
	private _lastCustomAIProviderURL = '';
	private _lastCustomAIImageProviderURL = '';
	private _aiImageModelFetchTimeout: number | null = null;
	private _aiImageModelFetchAbort: AbortController | null = null;
	private _aiImageModelFetchSeq = 0;
	private _imageProviderMemo: {
		url: string;
		apiKey: string;
		apiKeyStored: boolean;
	} | null = null;
	private _viewSettingLabels = {
		zoteroAPIKey: 'Zotero',
		signatureCert: _('Signature Certificate'),
		signatureKey: _('Signature Key'),
		signatureCa: _('Signature CA'),
		aiProvider: _('Provider'),
		aiProviderAPIKey: _('API key'),
		aiProviderModel: _('Model'),
		aiProviderURL: _('Base URL'),
		aiImageProvider: _('Provider'),
		aiImageProviderAPIKey: _('API key'),
		aiImageProviderURL: _('Base URL'),
		aiImageModel: _('Model'),
		aiImageSize: _('Image size'),
		aiRequestTimeout: _('Request timeout'),
	};
	private _serverPrivateSettingLabels: Record<string, string> = {
		ESignatureClientId: _('Client ID'),
		ESignatureSecret: _('Secret'),
	};
	private readonly settingLabels: Record<string, string> = {
		lockAccessibilityOn: _('In-document Screen Reader'),
		darkTheme: _('Dark Mode'),
		spellOnline: _('Automatic Spell Checking'),
		followServerZoom: _("Use this server's zoom settings"),
		smartZoom: _('Smart Zoom'),
		defaultZoom: _('Default Zoom'),
		smoothScroll: _('Smooth scrolling'),
		compactMode: _('Compact layout'),
		ShowStatusbar: _('Show status bar'),
		ShowRuler: _('Show Ruler'),
		ShowFormattingMarks: _('Show Formatting Marks'),
		ShowAnnotations: _('Show Comments'),
		A11yCheckDeck: _('Accessibility Checker'),
		ShowNavigator: _('Navigator'),
		ShowSidebar: _('Show Sidebar'),
		SdCustomAnimationDeck: _('Custom Animation'),
		// SdMasterPagesDeck: _('Master Pages'),
		// SdSlideTransitionDeck: _('Slide Transition'),
		StyleListDeck: _('Style List'),

		//Document Settings labels
		Grid: _('Grid'),
		Print: _('Print'),
		Other: _('Other'),
		ShowGrid: _('Show Grid'),
		SnapToGrid: _('Snap to grid'),
		Synchronize: _('Synchronize axes'),
		EmptyPages: _('Empty Pages'),
		ForceBreaks: _('Force Breaks'),
		AllSheets: _('All Sheets'),
		Content: _('Content'),
		Display: _('Display'),
		ShowBoundaries: _('Show Boundaries'),
		Drawing: _('Drawing'),
		Page: _('Page'),
		PageSize: _('Fit to page'),
		PageTile: _('Tile pages'),
		Booklet: _('Booklet'),
		BookletFront: _('Booklet front'),
		BookletBack: _('Booklet back'),
		PageName: _('Page name'),
		Date: _('Date'),
		Time: _('Time'),
		HiddenPage: _('Hidden pages'),
		FromPrinterSetup: _('From printer setup'),
		Presentation: _('Presentation'),
		Note: _('Notes'),
		Handout: _('Handouts'),
		Outline: _('Outline'),
		HandoutHorizontal: _('Handout horizontal'),
		Graphic: _('Images'),
		Control: _('Controls'),
		Background: _('Background'),
		PrintBlack: _('Print Black'),
		PrintHiddenText: _('Hidden text'),
		PrintPlaceholders: _('Placeholders'),
		LeftPage: _('Left pages'),
		RightPage: _('Right pages'),
		Brochure: _('Brochure'),
		BrochureRightToLeft: _('Brochure Right to Left'),

		// The remote checkers go by the names of their projects.
		languagetool: 'LanguageTool',
		duden: 'Duden Korrekturserver',
		grammalecte: 'Grammalecte',

		// The sentence checker panels are titled by the language they check.
		// The names of the languages are translated; what is inside a panel
		// is not, see settingTitles below.
		en: _('English'),
		hu_HU: _('Hungarian'),
		pt_BR: _('Portuguese (Brazil)'),
		ru_RU: _('Russian'),
		// Add more as needed
	};

	/**!
	 * The sentence checker options, as [label, help].
	 *
	 * Not translated, and deliberately so: each option names a notion of the
	 * grammar of one language, and whoever has reason to set it is reading
	 * that language already. Rendering the Hungarian options in Japanese
	 * would mean inventing terms for notions the target language may not
	 * have. The text is the rule package's own, printed by
	 * lpcompile.py --emit-titles.
	 *
	 * Where a package gives an option no help of its own the label stands in
	 * for both, which still says more than nothing when a narrow panel has
	 * cut the label short.
	 */
	private readonly settingTitles: Record<string, [string, string]> = {
		// Grammalecte, which checks French. The ids are the server's; the
		// wording is this dialog's own.
		'grammalecte-typo': [
			'Signes typographiques',
			'Guillemets, tirets, points de suspension et autres signes.',
		],
		'grammalecte-apos': [
			'Apostrophe typographique',
			'Signale les apostrophes droites.',
		],
		'grammalecte-eepi': [
			'Écriture épicène',
			'Normalise l’écriture épicène avec points médians.',
		],
		'grammalecte-esp': [
			'Espaces en trop',
			'Espaces inutiles entre les mots, en début et en fin de ligne.',
		],
		'grammalecte-tab': [
			'Tabulations en trop',
			'Tabulations inutiles en début et en fin de ligne.',
		],
		'grammalecte-nbsp': [
			'Espaces insécables',
			'Espaces insécables devant les ponctuations ! ? : ;',
		],
		'grammalecte-unit': [
			'Espaces insécables devant les unités',
			'Espace insécable entre un nombre et son unité de mesure.',
		],
		'grammalecte-tu': [
			'Traits d’union et soudures',
			'Traits d’union manquants ou inutiles.',
		],
		'grammalecte-maj': [
			'Majuscules',
			'Emploi des majuscules et des minuscules.',
		],
		'grammalecte-minis': [
			'Majuscules des ministères',
			'Majuscules dans les intitulés des ministères.',
		],
		'grammalecte-num': [
			'Nombres',
			'Espaces insécables dans les grands nombres, O à la place de 0.',
		],
		'grammalecte-nf': ['Normes françaises', 'Écriture conforme aux normes.'],
		'grammalecte-virg': [
			'Virgules',
			'Virgules manquantes devant mais, car et etc.',
		],
		'grammalecte-poncfin': [
			'Ponctuation finale',
			'Ponctuation manquante en fin de paragraphe.',
		],
		'grammalecte-ocr': [
			'Erreurs de numérisation',
			'Erreurs typiques de la reconnaissance de caractères. Beaucoup de faux positifs.',
		],
		'grammalecte-chim': ['Chimie', 'Typographie des formules chimiques.'],
		'grammalecte-liga': [
			'Ligatures typographiques',
			'Signale les ligatures fi, fl, ff, ffi, ffl, ft, st.',
		],
		'grammalecte-mapos': [
			'Apostrophe manquante après une lettre isolée',
			'Après l, d, s, n, c, j, m, t, ç. Déconseillé pour les textes scientifiques.',
		],
		'grammalecte-conf': [
			'Confusions et faux amis',
			'Erreurs dues à l’homonymie, comme faîte et faite.',
		],
		'grammalecte-loc': ['Locutions', 'Écriture des locutions usuelles.'],
		'grammalecte-gn': [
			'Accords en genre et en nombre',
			'Accords des noms et des adjectifs.',
		],
		'grammalecte-infi': [
			'Infinitif',
			'Confusions entre l’infinitif et d’autres formes.',
		],
		'grammalecte-conj': ['Conjugaison', 'Accord des verbes avec leur sujet.'],
		'grammalecte-ppas': [
			'Participes passés et adjectifs',
			'Accords des participes passés et des adjectifs.',
		],
		'grammalecte-imp': [
			'Impératif',
			'Deuxième personne du singulier, par exemple vas, prend, manges.',
		],
		'grammalecte-inte': [
			'Interrogatif',
			'Formes interrogatives et liaison des pronoms aux verbes.',
		],
		'grammalecte-vmode': ['Modes verbaux', 'Emploi des modes verbaux.'],
		'grammalecte-bs': [
			'Langage populaire',
			'Tournures courantes mais fautives, comme malgré que.',
		],
		'grammalecte-pleo': [
			'Pléonasmes',
			'Redondances comme au jour d’aujourd’hui ou monter en haut.',
		],
		'grammalecte-eleu': [
			'Élisions et euphonie',
			'Élisions incorrectes et tournures dysphoniques.',
		],
		'grammalecte-neg': [
			'Adverbe de négation',
			'Ne … pas, ne … jamais, et les autres.',
		],
		'grammalecte-redon1': [
			'Répétitions dans le paragraphe',
			'Hors mots grammaticaux, mots en majuscule, être et avoir.',
		],
		'grammalecte-redon2': [
			'Répétitions dans la phrase',
			'Hors mots grammaticaux, être et avoir.',
		],
		'grammalecte-date': ['Validité des dates', 'Dates impossibles.'],
		'grammalecte-mc': [
			'Mots composés',
			'Mots à trait d’union absents du dictionnaire.',
		],

		// en
		'en-grammar': ['Possible mistakes', 'Check more grammar errors.'],
		'en-cap': ['Capitalization', 'Check missing capitalization of sentences.'],
		'en-dup': ['Word duplication', 'Check repeated words.'],
		'en-pair': [
			'Parentheses',
			'Check missing or extra parentheses and quotation marks.',
		],
		'en-spaces': ['Word spacing', 'Check single spaces between words.'],
		'en-mdash': [
			'Em dash',
			'Force unspaced em dash instead of spaced en dash.',
		],
		'en-quotation': [
			'Quotation marks',
			'Check double quotation marks: "x" → “x”',
		],
		'en-times': [
			'Multiplication sign',
			'Check true multipliction sign: 5x5 → 5×5',
		],
		'en-spaces2': [
			'Sentence spacing',
			'Check single spaces between sentences.',
		],
		'en-ndash': [
			'En dash',
			'Force spaced en dash instead of unspaced em dash.',
		],
		'en-apostrophe': [
			'Apostrophe',
			'Change typewriter apostrophe, single quotation marks and correct double primes.',
		],
		'en-ellipsis': ['Ellipsis', 'Change three dots with ellipsis.'],
		'en-spaces3': [
			'More spaces',
			'Check more than two extra space characters between words and sentences.',
		],
		'en-minus': ['Minus sign', 'Change hyphen characters to real minus signs.'],
		'en-metric': [
			'Convert to metric (°C, km/h, m, kg, l)',
			'Measurement conversion from °F, mph, ft, in, lb, gal and miles.',
		],
		'en-numsep': [
			'Thousand separation of large numbers',
			'Common (1000000 → 1,000,000) or ISO (1000000 → 1 000 000).',
		],
		'en-nonmetric': [
			'Convert to non-metric (°F, mph, ft, lb, gal)',
			'Measurement conversion from °C; km/h; cm, m, km; kg; l.',
		],
		// hu_HU
		'hu_HU-cap': [
			'Nagy kezdőbetű',
			'Mondatok nagy kezdőbetűjének ellenőrzése.',
		],
		'hu_HU-par': ['Zárójelek', 'Zárójelpárok ellenőrzése.'],
		'hu_HU-quot': [
			'Idézőjelek',
			'A magyar „külső” és »belső« idézőjelek ellenőrzése.',
		],
		'hu_HU-wordpart': [
			'Egybe- és különírási javaslatok',
			'Egybe- és különírási javaslatok',
		],
		'hu_HU-dash': [
			'Nagykötőjel és gondolatjel',
			'A kiskötőjel cseréje a nagykötőjelre (- → –).',
		],
		'hu_HU-comma': [
			'Vesszőhasználatra vonatkozó javaslatok',
			'A valószínűleg hiányzó és felesleges vesszők jelzése.',
		],
		'hu_HU-numpart': [
			'Nagy számok tagolása szóközökkel',
			'Ezrestagolás nem-törő szóközökkel (10000 → 10 000).',
		],
		'hu_HU-grammar': [
			'Javaslat kevésbé egyértelmű esetben is',
			'Javaslat kevésbé egyértelmű esetben is',
		],
		'hu_HU-style': ['Stílusellenőrzés', 'Stílusellenőrzés'],
		'hu_HU-dup0': [
			'Szóismétlés',
			'Szóismétlés egymást közvetlenül követő szavak esetében.',
		],
		'hu_HU-compound': [
			'Egyszerű nem szótári összetett szavak',
			'Rövid, vagy rövid tagot tartalmazó, szóösszetételi szabályok alapján gyakran hibásan elfogadott alakok aláhúzása.',
		],
		'hu_HU-dup': [
			'Szóismétlés tagmondaton belül',
			'Szóismétlés tagmondaton belül',
		],
		'hu_HU-allcompound': [
			'Minden nem szótári összetett szó',
			'Minden szóösszetételi szabály alapján elfogadott alak aláhúzása.',
		],
		'hu_HU-dup2': ['Szóismétlés mondaton belül', 'Szóismétlés mondaton belül'],
		'hu_HU-money': [
			'Számok és átírásuk',
			'A számmal és számnévvel is leírt mennyiség megegyezik-e? (Pl. 10, azaz tíz Ft.)',
		],
		'hu_HU-dup3': [
			'Szóismétlés eltérő toldalékok esetén is',
			'Szóismétlés eltérő toldalékok esetén is',
		],
		'hu_HU-SI': [
			'Mértékegységek',
			'Nem SI mértékegységek átalakítása (°F, mérföld, yard, láb, hüvelyk, gallon, pint, font súlyú).',
		],
		'hu_HU-hyphen': [
			'Hiányzó elválasztás megadása',
			'Nem egyértelműen elválasztható szóalakok elválasztásának megadása (pl. megint, fölül).',
		],
		'hu_HU-apost': [
			'Aposztróf',
			"Az írógépes aposztróf cseréje a tipográfiailag megfelelőre (' → ’).",
		],
		'hu_HU-spaces': [
			'Dupla szóköz',
			'Két vagy három ismétlődő szóköz cseréje egyre.',
		],
		'hu_HU-frac': [
			'Törtek',
			'Perjellel elválasztott törtek cseréje Unicode karakterre (1/2 → ½).',
		],
		'hu_HU-ligature': ['f-ligatúra javaslata', 'Csere Unicode f-ligatúrára.'],
		'hu_HU-elli': [
			'Három pont',
			'Három pont cseréje az egalizált három pont karakterre (...→…).',
		],
		'hu_HU-spaces2': [
			'Sok szóköz',
			'Négy vagy több ismétlődő szóköz cseréje egy tabulátorra.',
		],
		'hu_HU-thin': [
			'Keskeny szóköz',
			'Keskeny szóköz (spácium) használata ezrestagoláshoz és más esetekben.',
		],
		'hu_HU-noligature': [
			'f-ligatúra tiltása',
			'Unicode f-ligatúra cseréje különálló betűkre.',
		],
		'hu_HU-idx': [
			'Indexek',
			'Számok cseréje mértékegységekben és kémiai képletekben valódi indexekre (m2 → m²).',
		],
		'hu_HU-minus': [
			'Mínuszjel',
			'Kötőjelek cseréje Unicode mínuszjelre a számok előtt.',
		],
		// ru_RU
		'ru_RU-hyphen': [
			'Дефис',
			'Правописание слов через дефис, слитно или раздельно.',
		],
		'ru_RU-comma': [
			'Пунктуация',
			'Поиск ошибок в расстановке знаков препинания.',
		],
		'ru_RU-multiword': [
			'Словосочетания',
			'Правописание словосочетаний и устойчивых оборотов.',
		],
		'ru_RU-together': [
			'Слитно/раздельно',
			'Правописание слов слитно или раздельно.',
		],
		'ru_RU-common': ['Общие ошибки', 'Поиск прочих ошибок'],
		'ru_RU-space': [
			'Пробел',
			'Поиск ошибок с пробелом: двойной пробел, пробел перед знаками пунктуации.',
		],
		'ru_RU-abbreviation': [
			'Аббревиатуры',
			'Аббревиатуры, сокращения и знаки пунктуации.',
		],
		'ru_RU-dup': ['Повтор слов', 'Поиск повторяющихся слов'],
		'ru_RU-numsep': [
			'Разделители групп разрядов (ISO) для чисел',
			'Разделители групп разрядов (ISO) для больших чисел',
		],
		'ru_RU-typographica': [
			'Типографика',
			'Предлагать замену сочетаний знаков на специальные символы.',
		],
		'ru_RU-quotation': [
			'Кавычки',
			'Предлагать замену кавычек на кавычки-«ёлочки» или парные кавычки.',
		],
		// pt_BR
		'pt_BR-grammar': ['Crase', 'Identificar o uso de crase.'],
		'pt_BR-cap': [
			'Maiúsculas',
			'Procurar pela falta de letra maiúscula no inicio das frases.',
		],
		'pt_BR-dup': ['Palavras duplicadas', 'Procurar palavras repetidas.'],
		'pt_BR-pair': [
			'Parênteses',
			'Procurar parênteses ou aspas a mais ou a menos.',
		],
		'pt_BR-spaces': [
			'Espaço entre palavras',
			'Verificar espaços entre palavras.',
		],
		'pt_BR-mdash': [
			'Travessão',
			'Forçar travessão não espaçado em vez de meio-traço espaçado.',
		],
		'pt_BR-quotation': ['Aspas Duplas', 'Verificar aspas duplas: "x" → “x”'],
		'pt_BR-times': [
			'Sinal de multiplicação',
			'Verificar verdadeiro sinal de multiplicação: 5x5 → 5×5',
		],
		'pt_BR-spaces2': ['Espaço entre frases', 'Verificar espaços entre frases.'],
		'pt_BR-ndash': [
			'Meio-traço',
			'Forçar meio-traço espaçado em vez de travessão não espaçado.',
		],
		'pt_BR-apostrophe': [
			'Aspas simples',
			'Alterar apóstrofo de máquina de escrever, aspas simples e corrigir primos duplos.',
		],
		'pt_BR-ellipsis': [
			'Reticências',
			'Alterar três pontos seguidos por reticências.',
		],
		'pt_BR-spaces3': [
			'Espaços adicionais',
			'Verificar por mais de dois espaços em branco entre palavras e frases.',
		],
		'pt_BR-minus': [
			'Sinal de menos',
			'Trocar hífen pelo verdadeiro sinal de menos.',
		],
		'pt_BR-metric': [
			'Pleonasmos',
			'Identificação de redundância de termos: Subir pra cima, encarar de frente, regra geral.',
		],
		'pt_BR-gerund': [
			'Gerundismos',
			'Emprego inapropriado do gerúndio: estarei trabalhando, vou estar fazendo.',
		],
		'pt_BR-nonmetric': ['Cacofonias', 'Som feio ou desagradável: por cada.'],
		'pt_BR-paronimo': [
			'Parônimos',
			'Palavras parecidas na grafia ou na pronúncia, mas com significados diferentes.',
		],
		'pt_BR-composto': [
			'Termos compostos',
			'Identifica termos compostos escritos separados. Ex.: auto escola, sub contratado.',
		],
		'pt_BR-malmau': ['Mal ou Mau', 'Emprego de mal ou mau.'],
		'pt_BR-aha': ['Há ou a', 'Emprego de há ou a.'],
		'pt_BR-meiameio': ['Meia ou meio', 'Emprego de meia ou meio.'],
		'pt_BR-verbo': ['Concordância verbal', 'Verifica a concordância verbal.'],
		'pt_BR-pronominal': [
			'Colocação Pronominal',
			'Posição que os pronomes pessoais ocupam em relação ao verbo.',
		],
		'pt_BR-pronome': ['Uso de pronome', 'Emprego de pronome.'],
		'pt_BR-porque': [
			'Emprego do porquê',
			'Identifica uso de porque, por que, porquê e por quê.',
		],
	};

	// SVG templates for icons that are small and always present (no async load needed)
	private readonly SVG_ICONS = {
		download: `<svg fill="currentColor" width="20" height="20" viewBox="0 0 24 24"><path d="M5,20H19V18H5M19,9H15V3H9V9H5L12,16L19,9Z"></path></svg>`,
		delete: `<svg fill="currentColor" width="20" height="20" viewBox="0 0 24 24"><path d="M19,4H15.5L14.5,3H9.5L8.5,4H5V6H19M6,19A2,2 0 0,0 8,21H16A2,2 0 0,0 18,19V7H6V19Z"></path></svg>`,
		edit: `<svg fill="currentColor" width="20" height="20" viewBox="0 0 24 24"><path d="M3 17.25V21h3.75l11-11.03-3.75-3.75L3 17.25zM20.71 7.04a1 1 0 0 0 0-1.41l-2.34-2.34a1 1 0 0 0-1.41 0l-1.83 1.83 3.75 3.75 1.83-1.83z"></path></svg>`,
		add: `<svg fill="currentColor" width="20" height="20" viewBox="0 0 24 24"><path d="M19,13H13V19H11V13H5V11H11V5H13V11H19V13Z"></path></svg>`,
		reset: `<svg fill="currentColor" width="24" height="24" viewBox="0 0 24 24"><path d="M12 4V1L8 5l4 4V6c3.31 0 6 2.69 6 6 0 .34-.03.67-.08 1h2.02c.05-.33.06-.66.06-1 0-4.42-3.58-8-8-8zm-6 7c0-.34.03-.67.08-1H4.06c-.05.33-.06.66-.06 1 0 4.42 3.58 8 8 8v3l4-4-4-4v3c-3.31 0-6-2.69-6-6z"></path></svg>`,
		updateAll: '',
		checkboxMarked: `<svg fill="currentColor" width="24" height="24" viewBox="0 0 24 24"><path d="M10,17L5,12L6.41,10.58L10,14.17L17.59,6.58L19,8M19,3H5C3.89,3 3,3.89 3,5V19A2,2 0 0,0 5,21H19A2,2 0 0,0 21,19V5C21,3.89 20.1,3 19,3Z"></path></svg>`,
		checkboxBlankOutline: `<svg fill="currentColor" width="24" height="24" viewBox="0 0 24 24"><path d="M19,3H5C3.89,3 3,3.89 3,5V19A2,2 0 0,0 5,21H19A2,2 0 0,0 21,19V5C21,3.89 20.1,3 19,3M19,5V19H5V5H19Z"></path></svg>`,
	};
	private _allConfigSection: HTMLElement | null;
	private _sectionObserver: IntersectionObserver | null = null;
	private _visibleSections: Set<Element> = new Set();

	private _browserSettingSection: HTMLElement | null = null;
	private _zoomSection: HTMLElement | null = null;
	private _accessibilitySection: HTMLElement | null = null;
	private _xcuSection: HTMLElement | null = null;
	private _sentenceCheckerSection: HTMLElement | null = null;
	private _remoteCheckerSection: HTMLElement | null = null;
	private _aiSection: HTMLElement | null = null;
	private _docSigningSection: HTMLElement | null = null;
	private _zoteroSection: HTMLElement | null = null;
	private _configSections: SectionConfig[] | null = null;

	// The zoom of the whole application window, as a percentage. Zero means
	// no explicit zoom is chosen and the system scaling applies.
	private _uiZoomPercent = 0;
	// The zoom the native side holds, as last fetched or saved, in the same
	// form as _uiZoomPercent. Null while it is unknown, either before the fetch
	// or after it failed.
	private _uiZoomStoredPercent: number | null = null;

	private getAPIEndpoints() {
		return {
			uploadSettings: window.serviceRoot + '/browser/dist/upload-settings',

			fetchSharedConfig:
				window.serviceRoot + '/browser/dist/fetch-settings-config',

			deleteSharedConfig:
				window.serviceRoot + '/browser/dist/delete-settings-config',

			fetchSettingFile:
				window.serviceRoot + '/browser/dist/fetch-settings-file',
			fetchModels: window.serviceRoot + '/browser/dist/fetch-models',
		};
	}

	private PATH = {
		autoTextUpload: () => this.settingConfigBasePath() + '/autotext/',
		wordBookUpload: () => this.settingConfigBasePath() + '/wordbook/',
		browserSettingsUpload: () =>
			this.settingConfigBasePath() + '/browsersetting/',
		viewSettingsUpload: () => this.settingConfigBasePath() + '/viewsetting/',
		XcuUpload: () => this.settingConfigBasePath() + '/xcu/',
		themesUpload: () => this.settingConfigBasePath() + '/themes/',
		extensionsUpload: () => this.settingConfigBasePath() + '/extensions/',
		spifUpload: () => this.settingConfigBasePath() + '/spif/',
		serverPrivateSettingsUpload: () =>
			this.settingConfigBasePath() + '/serverprivateinfo/',
	};
	private browserSettingOptions: Record<string, any> = {};
	// The Interface Settings as browsersetting.json holds them, before the
	// built-in defaults are merged in. A key is absent or empty when the user has
	// saved no choice of their own for it.
	private storedBrowserSetting: Record<string, any> = {};

	getViewSettings(): ViewSettings {
		return this._viewSetting;
	}

	public async saveAll(): Promise<SaveAllResult> {
		// Store the AI base URLs in their canonical form (no trailing "/v1"), in
		// case a field was edited but never blurred before the save.
		this._viewSetting.aiProviderURL = this.normalizeBaseUrl(
			this._viewSetting.aiProviderURL || '',
		);
		this._viewSetting.aiImageProviderURL = this.normalizeBaseUrl(
			this._viewSetting.aiImageProviderURL || '',
		);

		// The View-tab / AI-sidebar payoff should fire only when the user
		// actually changed the chat AI configuration in this dialog session (not
		// on every save that happens to already be configured). The key is
		// optional, so a base URL and a model are what mark a provider
		// configured. We do not validate the key - if it is wrong, that surfaces
		// at AI use time.
		const aiJustConfigured =
			this._aiConfigDirty &&
			!!this._viewSetting.aiProviderModel &&
			!!this._viewSetting.aiProviderURL;
		const aiKeyMissing = !this._viewSetting.aiProviderAPIKey;

		// Snapshot the view settings as entered now. Uploading a file re-fetches
		// the config, which redacts the in-memory copy, so reading it back after
		// the uploads would drop a freshly typed key. The live session is
		// updated from this snapshot.
		const viewSettings: ViewSettings = { ...this._viewSetting };

		const saves: Promise<void>[] = [];

		// Browser settings
		let browserSettings: Record<string, string | number> | null = null;
		if (this._browserSettingSection) {
			const payload = this.browserSettingsPayload();
			browserSettings = this.flattenBrowserSettings(JSON.parse(payload));
			saves.push(
				(async () => {
					const file = new File([payload], 'browsersetting.json', {
						type: 'application/json',
						lastModified: Date.now(),
					});
					await this.uploadFile(this.PATH.browserSettingsUpload(), file);
					if ((window as any).parent?.mode?.isCODesktop()) {
						(window.parent as any).postMobileMessage('SYNCSETTINGS');
					}
				})(),
			);
		}

		// Document settings (XCU)
		let documentSettings = false;
		if (this.xcuEditor) {
			saves.push(this.xcuEditor.generateXcuAndUpload());
			documentSettings = true;
		}

		// View settings
		saves.push(
			this.uploadViewSettingFile(
				'viewsetting.json',
				JSON.stringify(this._viewSetting),
			),
		);

		// Electronic signature
		if (this._serverPrivateSection) {
			saves.push(
				this.uploadServerPrivateSettingFile(this._serverPrivateSetting),
			);
		}

		// The user interface zoom is persisted and applied by the native side.
		// It is sent only when it differs from the value the native side holds.
		if (
			this._uiZoomStoredPercent !== null &&
			this._uiZoomPercent !== this._uiZoomStoredPercent
		) {
			(window.parent as any).postMobileMessage(
				'SETUIZOOM ' + this._uiZoomPercent,
			);
			this._uiZoomStoredPercent = this._uiZoomPercent;
		}

		await Promise.all(saves);

		return {
			aiJustConfigured,
			aiKeyMissing,
			documentSettings,
			viewSettings,
			browserSettings,
		};
	}

	init(): void {
		this._allConfigSection = document.getElementById('allConfigSection');
		this.initWindowVariables();
		if (!window.showLeftNav) {
			document.getElementById('settings-css')?.remove();
		}
		if (isCODesktop) {
			this.settingsStorage = new DesktopSettingsStorage();
		} else {
			this.settingsStorage = new OnlineSettingsStorage();
		}
		this.fetchAndPopulateSharedConfigs();
		this.wordbook = window.WordBook;
	}

	/// Called when the parent says the document's session count changed.
	public refreshSettingsScope(): void {
		this.updateNextDocumentNote();
	}

	public async uploadXcuFile(filename: string, content: string): Promise<void> {
		const file = new File([content], filename, { type: 'application/xml' });
		await this.uploadFile(this.PATH.XcuUpload(), file);
	}

	async uploadWordbookFile(filename: string, content: string): Promise<void> {
		const file = new File([content], filename, { type: 'text/plain' });
		await this.uploadFile(this.PATH.wordBookUpload(), file);
	}

	async uploadViewSettingFile(
		filename: string,
		content: string,
	): Promise<void> {
		const file = new File([content], filename, { type: 'text/plain' });
		await this.uploadFile(
			this.PATH.viewSettingsUpload(),
			file,
			this._viewSettingFileUrl,
		);
	}

	private initWindowVariables(): void {
		const element = document.getElementById('initial-variables');
		if (!element) return;

		// On desktop the template is loaded as-is from the install tree
		// (no server-side render), so unsubstituted "%TOKEN%" literals
		// reach us here. Fall back to URL query params for the values
		// that matter — Map.Settings.ts passes them via form GET on every
		// open, and reading them inline means populate() runs once with
		// correct values (no postMessage round-trip gymnastics).
		const ds = (element as HTMLInputElement).dataset;
		const urlParams = new URLSearchParams(window.location.search);
		const isPlaceholder = (v: string) => /^(?:%.+%|<!--%.+%-->)$/.test(v);
		const read = (dataKey: string, urlKey: string): string => {
			const v = ds[dataKey] ?? '';
			if (!v || isPlaceholder(v)) {
				// Map.Settings stringifies undefined window.* values as
				// the literal "undefined"; filter that.
				const u = urlParams.get(urlKey);
				return u && u !== 'undefined' ? u : '';
			}
			return v;
		};

		window.accessToken = read('accessToken', 'access_token');
		if (!window.accessToken && !isCODesktop) {
			throw new Error('Access token is missing in initial variables.');
		}

		window.accessTokenTTL = read('accessTokenTtl', 'access_token_ttl');
		window.enableDebug = read('enableDebug', 'enable_debug') === 'true';
		window.enableAccessibility =
			read('enableAccessibility', 'enable_accessibility') === 'true';
		window.disableAISettings =
			read('disableAiSettings', 'disable_ai_settings') === 'true';
		window.showLeftNav = read('showLeftNav', 'show_left_nav') === 'true';
		window.scrollTarget = read('scrollTarget', 'scroll_target');
		// Absent means yes: a host that does not send it is one where the
		// question does not arise.
		window.userPresetsApplied =
			read('userPresetsApplied', 'user_presets_applied') !== 'false';
		window.documentSettingsLive =
			read('documentSettingsLive', 'document_settings_live') !== 'false';
		try {
			window.sentenceCheckingPackages = JSON.parse(
				read('sentenceChecking', 'sentence_checking') || '[]',
			);
		} catch {
			window.sentenceCheckingPackages = [];
		}
		try {
			window.remoteCheckers = JSON.parse(
				read('remoteCheckers', 'remote_checkers') || '[]',
			);
		} catch {
			window.remoteCheckers = [];
		}
		window.wopiSettingBaseUrl = read(
			'wopiSettingBaseUrl',
			'wopi_setting_base_url',
		);
		window.iframeType = read('iframeType', 'iframe_type') || 'user';
		window.cssVars = read('cssVars', 'css_vars');
		if (window.cssVars) {
			window.cssVars = atob(window.cssVars);
			const sheet = new CSSStyleSheet();
			if (typeof (sheet as any).replace === 'function') {
				(sheet as any).replace(window.cssVars);
				(document as any).adoptedStyleSheets.push(sheet);
			}
		}
		window.serviceRoot = read('serviceRoot', 'service_root');
		window.versionHash = read('versionHash', 'version_hash');

		// coolwsd.xml can choose the opening zoom for every text document on this
		// server. Where it does, that choice stands in for the built-in default, so
		// the form starts on the server's value for a user who has saved no zoom
		// setting of their own. Both values are empty when coolwsd.xml chooses
		// neither.
		const smartZoom = read('smartZoom', 'smart_zoom');
		if (smartZoom) defaultBrowserSetting.smartZoom = smartZoom === 'true';
		const defaultZoom = parseInt(read('defaultZoom', 'default_zoom'));
		if (!isNaN(defaultZoom))
			defaultBrowserSetting.defaultZoom.value = defaultZoom;

		// CSS theme switching reads <html data-theme>; the template's
		// literal "%UI_THEME%" would never match. Set it from the URL.
		const theme = urlParams.get('ui_theme');
		if (theme && theme !== 'undefined' && !isPlaceholder(theme)) {
			document.documentElement.setAttribute('data-theme', theme);
		}
	}

	private validateJsonFile(file: File): Promise<any> {
		return new Promise((resolve, reject) => {
			const reader = new FileReader();
			reader.onload = (event) => {
				try {
					const content = event.target?.result as string;
					const jsonData = JSON.parse(content);
					resolve(jsonData);
				} catch (error) {
					reject(new Error(_('Invalid JSON file')));
				}
			};
			reader.onerror = () => {
				reject(new Error(_('Error reading file')));
			};
			reader.readAsText(file);
		});
	}

	private getConfigSections(): SectionConfig[] {
		if (this._configSections) return this._configSections;

		this._configSections = [
			{
				id: 'autotext',
				sectionTitle: _('Autotext'),
				sectionDesc: _(
					'Upload reusable text snippets (.bau). To insert the text in your document, type the shortcut for an AutoText entry and press F3.',
				),
				listId: 'autotextList',
				inputId: 'autotextFile',
				buttonId: 'uploadAutotextButton',
				fileAccept: '.bau',
				buttonText: _('Upload Autotext'),
				uploadPath: this.PATH.autoTextUpload(),
				element: null,
			},
			{
				id: 'wordbook',
				sectionTitle: _('Custom dictionaries'),
				sectionDesc: _(
					'Add or edit words in a spell check dictionary. Words in your wordbook (.dic) will be available for spelling checks.',
				),
				listId: 'wordbookList',
				inputId: 'wordbookFile',
				buttonId: 'uploadWordbookButton',
				fileAccept: '.dic',
				buttonText: _('Upload Wordbook'),
				uploadPath: this.PATH.wordBookUpload(),
				element: null,
			},
			{
				id: 'themes',
				sectionTitle: _('Document themes'),
				sectionDesc: _(
					'Upload custom document themes (.theme). These define color palettes available under Format > Theme.',
				),
				listId: 'themesList',
				inputId: 'themesFile',
				buttonId: 'uploadThemesButton',
				fileAccept: '.theme',
				buttonText: _('Upload Theme'),
				uploadPath: this.PATH.themesUpload(),
				element: null,
			},
			{
				id: 'extensions',
				sectionTitle: _('Extensions'),
				sectionDesc: _(
					'Upload a packaged extension as a .zip; its contents (manifest.json, ...) are unpacked on the server.',
				),
				listId: 'extensionsList',
				inputId: 'extensionsFile',
				buttonId: 'uploadExtensionsButton',
				fileAccept: '.zip',
				buttonText: _('Upload Extension'),
				uploadPath: this.PATH.extensionsUpload(),
				enabledFor: 'systemconfig',
				element: null,
			},
			{
				id: 'spif',
				sectionTitle: _('Security policies'),
				sectionDesc: _(
					'Upload SPIF security policies (.xml). These define the classifications and categories available when applying a security label to a document.',
				),
				listId: 'spifList',
				inputId: 'spifFile',
				buttonId: 'uploadSpifButton',
				fileAccept: '.xml',
				buttonText: _('Upload Security Policy'),
				uploadPath: this.PATH.spifUpload(),
				element: null,
			},
		];

		return this._configSections;
	}

	private insertConfigSections(data: ConfigData | null): void {
		if (!this._allConfigSection) return;

		this.getConfigSections().forEach((cfg) => {
			if (cfg.enabledFor && cfg.enabledFor !== this.getConfigType()) {
				return;
			}

			if (cfg.debugOnly && !window.enableDebug) {
				return;
			}

			if (!data) {
				cfg.element = this.createEmptySection(
					cfg.element,
					cfg.id,
					cfg.sectionTitle,
				);
				return;
			}

			const sectionEl = this.createConfigSection(cfg);
			const fileInput = sectionEl.querySelector<HTMLInputElement>(
				`#${cfg.inputId}`,
			);
			const button = sectionEl.querySelector<HTMLButtonElement>(
				`#${cfg.buttonId}`,
			);

			if (fileInput && button) {
				button.addEventListener('click', () => {
					fileInput.click();
				});

				fileInput.addEventListener('change', async () => {
					if (fileInput.files?.length) {
						if (cfg.uploadPath === this.PATH.wordBookUpload()) {
							this.wordbook.wordbookValidation(
								cfg.uploadPath,
								fileInput.files[0],
							);
						} else {
							let file = fileInput.files[0];

							this.uploadFile(cfg.uploadPath, file);
						}
						fileInput.value = '';
					}
				});
			}

			cfg.element = this.mountConfigSection(
				this._allConfigSection!,
				cfg.element,
				sectionEl,
			);
		});
	}

	private async fetchAndPopulateSharedConfigs(): Promise<void> {
		try {
			this.populateSharedConfigUI(null);
			this.setupLeftNavbar();
			const data = await this.settingsStorage.fetchSettingsConfig();
			await this.populateSharedConfigUI(data);
			console.debug('Shared config data: ', data);
		} catch (error: unknown) {
			// A guest / anonymous session has no per-user settings: the host
			// rejects the config request with a client error (for example a 400
			// when there is no user context), or the settings URL and token were
			// never provided. Say plainly that settings need a signed-in user,
			// and keep the generic message for server or network failures.
			const status = (error as any)?.status;
			const noSettingsStore = !window.wopiSettingBaseUrl || !window.accessToken;
			const notAuthorized = status === 400 || status === 401 || status === 403;
			const isGuest = !isCODesktop && (noSettingsStore || notAuthorized);
			if (isGuest) {
				// The dialog has nothing to offer a guest, so close it when they
				// dismiss the message rather than leaving an empty dialog open.
				SettingIframe.showErrorModal(
					_('Settings are only available to signed-in users.'),
					() =>
						window.parent.postMessage(
							JSON.stringify({ MessageId: 'settings-cancel' }),
							parentTargetOrigin(),
						),
				);
			} else {
				SettingIframe.showErrorModal(
					_('Something went wrong. Please try to refresh the page.'),
				);
			}
			console.error('Error fetching shared config:', error);
		}

		this.scrollWindowTargetIntoView();
	}

	private createConfigSection(config: SectionConfig): HTMLDivElement {
		const sectionEl = document.createElement('div');
		sectionEl.classList.add('section');
		sectionEl.id = config.id;

		sectionEl.appendChild(this.createHeading(config.sectionTitle, 'h3'));
		sectionEl.appendChild(this.createParagraph(config.sectionDesc));
		sectionEl.appendChild(this.createUnorderedList(config.listId));
		sectionEl.appendChild(
			this.createFileInput(config.inputId, config.fileAccept),
		);
		sectionEl.appendChild(
			this.createButton(config.buttonId, config.buttonText),
		);

		return sectionEl;
	}

	private createEmptySection(
		current: HTMLElement | null,
		id: string,
		heading: string,
	): HTMLElement {
		if (current) return current;

		const section = document.createElement('div');
		section.classList.add('section');
		section.id = id;
		section.style.display = 'none';
		section.appendChild(this.createHeading(heading, 'h3'));

		this._allConfigSection!.appendChild(section);
		return section;
	}

	private mountConfigSection(
		container: HTMLElement,
		current: HTMLElement | null,
		next: HTMLElement,
	): HTMLElement {
		if (current && current.parentNode === container) {
			current.replaceWith(next);
		} else {
			container.appendChild(next);
		}
		return next;
	}

	private createHeading(text: string, level: 'h1' | 'h2' | 'h3' = 'h3') {
		const headingEl = document.createElement(level);
		headingEl.textContent = text;
		return headingEl;
	}

	private createParagraph(text: string) {
		const pEl = document.createElement('p');
		pEl.textContent = text;
		return pEl;
	}

	private createUnorderedList(id: string) {
		const ulEl = document.createElement('ul');
		ulEl.id = id;
		return ulEl;
	}

	private createFileInput(id: string, accept: string) {
		const inputEl = document.createElement('input');
		inputEl.type = 'file';
		inputEl.classList.add('hidden');
		inputEl.id = id;
		inputEl.accept = accept;
		return inputEl;
	}

	private createTextInput(
		id: string,
		placeholder: string = '',
		text: string = '',
		onChangeHandler = (input) => {},
	) {
		const inputEl = document.createElement('input');
		inputEl.type = 'text';
		inputEl.id = id;
		inputEl.value = text;
		inputEl.placeholder = placeholder;
		inputEl.classList.add('dic-input-container');

		inputEl.addEventListener('change', () => {
			onChangeHandler(inputEl);
		});
		return inputEl;
	}

	private createSelectInput(
		id: string,
		options: Array<{ value: string; label: string }>,
		selectedValue: string,
		onChangeHandler = (select) => {},
	) {
		const selectEl = document.createElement('select');
		selectEl.id = id;
		selectEl.classList.add('dic-input-container');

		options.forEach((option) => {
			const optionEl = document.createElement('option');
			optionEl.value = option.value;
			optionEl.textContent = option.label;
			selectEl.appendChild(optionEl);
		});

		selectEl.value = selectedValue;

		selectEl.addEventListener('change', () => {
			onChangeHandler(selectEl);
		});
		return selectEl;
	}

	private createTextArea(
		id: string,
		placeholder: string = '',
		text: string = '',
		onChangeHandler = (textarea) => {},
	) {
		const textareaEl = document.createElement('textarea');
		textareaEl.id = id;
		textareaEl.value = text.replace(/\\n/g, '\n');
		textareaEl.placeholder = placeholder;
		textareaEl.classList.add('dic-input-container', 'signature-textarea');
		textareaEl.rows = 6;

		textareaEl.addEventListener('change', () => {
			onChangeHandler(textareaEl);
		});
		return textareaEl;
	}

	private createButton(id: string, text: string) {
		const buttonEl = document.createElement('button');
		buttonEl.id = id;
		buttonEl.type = 'button';
		buttonEl.classList.add(
			'inline-button',
			'button',
			'button--text-only',
			'button--vue-secondary',
		);

		const wrapperSpan = document.createElement('span');
		wrapperSpan.classList.add('button__wrapper');

		const textSpan = document.createElement('span');
		textSpan.classList.add('button__text');
		textSpan.textContent = text; // Safely set text content
		wrapperSpan.appendChild(textSpan);

		buttonEl.appendChild(wrapperSpan);

		return buttonEl;
	}

	private async fetchWordbookFile(fileId: string): Promise<void> {
		this.wordbook.startLoader();
		try {
			const textValue = await this.settingsStorage.fetchSettingFile(fileId);

			if (!textValue) {
				throw new Error('Failed to fetch wordbook file');
			}

			const wordbook = await this.wordbook.parseWordbookFileAsync(textValue);
			const fileName = this.getFilename(fileId, false);
			this.wordbook.stopLoader();
			this.wordbook.openWordbookEditor(fileName, wordbook);
		} catch (error: unknown) {
			const message = error instanceof Error ? error.message : 'Unknown error';
			console.error(`Error uploading file: ${message}`);
			SettingIframe.showErrorModal(
				_(
					'Something went wrong while fetching wordbook. Please try to refresh the page.',
				),
			);
			this.wordbook.stopLoader();
		}
	}
	private createBrowserSettingForm(sharedConfigsContainer: HTMLElement): void {
		// On CODA the Interface Settings duplicate what the View menu / sidebar
		// / zoom slider already persist via localStorage (and the dialog can't
		// reach the engine's xcu anyway), so skip the whole section.
		if (isCODesktop) return;
		const editorContainer = document.createElement('div');
		editorContainer.id = 'browser-setting';
		editorContainer.className = 'section';
		editorContainer.appendChild(this.createHeading(_('Interface Settings')));
		editorContainer.appendChild(
			this.createParagraph(_('Set default interface preferences.')),
		);

		const navContainer = this.createBrowserSettingTabsNav(editorContainer);

		const commonTogglesData: Record<string, boolean> = {};

		for (const [key, value] of Object.entries(this.browserSettingOptions)) {
			// On CODA the layout is always Notebookbar and dark mode is toggled
			// from the Notebookbar, so hide both toggles here.
			if ((key === 'compactMode' || key === 'darkTheme') && isCODesktop)
				continue;
			// The zoom settings have a section of their own.
			if (ZOOM_SETTING_KEYS.includes(key)) continue;
			// Include:
			// - plain booleans
			// - objects that have a customType (like compactToggle and defaultZoom)
			if (
				typeof value === 'boolean' ||
				(typeof value === 'object' && value !== null && 'customType' in value)
			) {
				commonTogglesData[key] = value;
			}
		}

		if (Object.keys(commonTogglesData).length > 0) {
			const commonTogglesElement = this.renderSettingsOption(
				commonTogglesData,
				'common',
			);
			editorContainer.appendChild(commonTogglesElement);
			const separator = document.createElement('hr');
			separator.style.border = 'none';
			separator.style.borderTop = '1px solid var(--settings-border)';
			separator.style.marginTop = '1rem';
			editorContainer.appendChild(separator);
		}

		const contentsContainer = this.createBrowserSettingContentsContainer();
		const actionsContainer = this.createBrowserSettingActions(
			sharedConfigsContainer,
		);

		editorContainer.appendChild(navContainer);
		editorContainer.appendChild(contentsContainer);
		editorContainer.appendChild(actionsContainer);

		this._browserSettingSection = this.mountConfigSection(
			sharedConfigsContainer,
			this._browserSettingSection,
			editorContainer,
		);

		setTimeout(() => {
			const defaultTab = navContainer.querySelector(
				'#bs-tab-spreadsheet',
			) as HTMLElement;
			if (defaultTab) {
				defaultTab.click();
			}
		}, 0);

		this.generateZoomSettingsUI(sharedConfigsContainer);

		this.addSubElements();
		this.installSettingsHooks();
	}

	/**!
	 * The control ids keep the common- prefix, since that prefix is the name the
	 * setting has in browsersetting.json.
	 */
	private generateZoomSettingsUI(sharedConfigsContainer: HTMLElement): void {
		const zoomContainer = document.createElement('div');
		zoomContainer.id = 'zoom-behaviour';
		zoomContainer.className = 'section';
		zoomContainer.appendChild(this.createHeading(_('Zoom Behaviour')));

		// The checkbox starts from what has been saved and is read back off the
		// page when saving, so the object it writes into is a throwaway.
		zoomContainer.appendChild(
			this.createCheckboxToggle(
				'followServerZoom',
				this.isFollowingServerZoom(),
				'common-followServerZoom',
				{},
			),
		);
		zoomContainer.appendChild(
			this.createCheckboxToggle(
				'smartZoom',
				this.browserSettingOptions.smartZoom,
				'common-smartZoom',
				this.browserSettingOptions,
			),
		);
		zoomContainer.appendChild(
			this.renderZoomDropdown(
				'defaultZoom',
				this.browserSettingOptions.defaultZoom,
				'common-defaultZoom',
			),
		);

		this._zoomSection = this.mountConfigSection(
			sharedConfigsContainer,
			this._zoomSection,
			zoomContainer,
		);
	}

	// A checkbox that carries a line of explanation stacks the two, with the
	// text lined up under the label rather than under the box.
	private addCheckboxDescription(containerSelector: string, text: string) {
		const container: HTMLSpanElement | null =
			document.querySelector(containerSelector);

		if (!container) {
			console.error(containerSelector + ' does not exist!');
			return;
		}

		container.classList.add('checkbox-with-description');
		container.appendChild(this.createParagraph(text));
	}

	// True when the user has saved no zoom choice of their own. The zoom the
	// server configured applies then, and goes on applying as the server changes
	// it.
	private isFollowingServerZoom(): boolean {
		const isChosen = (value: any) =>
			value !== undefined && value !== null && value !== '';
		return (
			!isChosen(this.storedBrowserSetting.smartZoom) &&
			!isChosen(this.storedBrowserSetting.defaultZoom)
		);
	}

	private _updateZoomControlsState() {
		const followServerZoomCheckbox: HTMLInputElement | null =
			document.querySelector('#common-followServerZoom-input');
		if (!followServerZoomCheckbox) {
			console.error('#common-followServerZoom-input does not exist!');
			return;
		}

		const smartZoomCheckbox: HTMLInputElement | null = document.querySelector(
			'#common-smartZoom-input',
		);
		if (!smartZoomCheckbox) {
			console.error('#common-smartZoom-input does not exist!');
			return;
		}

		const smartZoomWrapper: HTMLElement | null = document.querySelector(
			'#common-smartZoom-container',
		);
		if (!smartZoomWrapper) {
			console.error('#common-smartZoom-container does not exist!');
			return;
		}

		const defaultZoomDropdown: HTMLSelectElement | null =
			document.querySelector('#common-defaultZoom-select');
		if (!defaultZoomDropdown) {
			console.error('#common-defaultZoom-select does not exist!');
			return;
		}

		const defaultZoomLabel: HTMLHeadingElement | null = document.querySelector(
			'#common-defaultZoom > .view-setting-small-label',
		);
		if (!defaultZoomLabel) {
			console.error(
				'#common-defaultZoom > .view-setting-small-label does not exist!',
			);
			return;
		}

		const updateControlsState = () => {
			const followsServer = followServerZoomCheckbox.checked;
			smartZoomCheckbox.disabled = followsServer;
			smartZoomWrapper.classList.toggle(
				'checkbox-radio-switch--disabled',
				followsServer,
			);
			// Smart zoom fits the document to the window, so it decides the zoom
			// on its own and leaves the dropdown with nothing to say.
			defaultZoomDropdown.disabled = followsServer || smartZoomCheckbox.checked;
			defaultZoomLabel.classList.toggle(
				'disabled-label',
				defaultZoomDropdown.disabled,
			);
		};

		followServerZoomCheckbox.addEventListener('change', () => {
			updateControlsState();
		});

		smartZoomCheckbox.addEventListener('change', () => {
			updateControlsState();
		});

		updateControlsState();
	}

	/** The way we construct the list of toggles is really brittle, there's no
	 * way to add tooltips to these checkboxes or some labels around them. A
	 * quick hack for now is to get the element in this function and then
	 * add tooltip to it, or insert something before/after the element using
	 * the Element.before() and Element.after() methods.
	 *
	 * We have custom types to do fancy things, but adding tooltips or
	 * static description labels around the widgets are stateless operations
	 * i.e. they don't have much to do with the widget or it's state. So
	 * customType doesn't work for this usecase.
	 */
	public addSubElements() {
		this.addCheckboxDescription(
			'#common-followServerZoom-container',
			_(
				'When on, text documents open at the zoom this server chooses. Turn it off to choose the zoom yourself.',
			),
		);
		this.addCheckboxDescription(
			'#common-smartZoom-container',
			_(
				'When on, Writer fits the document to the viewport width on open. When off, the document opens at the Default Zoom level set below.',
			),
		);
	}

	/*
	 * this is where we setup state relations between various widgets. like
	 * "disable that control when the checkbox is checked...". by the time this
	 * function is called, all the controls exist, so we can query them here and
	 * setup listeners...
	 */
	public installSettingsHooks() {
		this._updateZoomControlsState();
	}

	private createBrowserSettingTabsNav(
		editorContainer: HTMLElement,
	): HTMLDivElement {
		const navContainer = document.createElement('div');
		navContainer.className = 'browser-setting-tabs-nav';

		const tabs = [
			{ id: 'spreadsheet', label: 'Calc' },
			{ id: 'text', label: 'Writer' },
			{ id: 'presentation', label: 'Impress' },
			{ id: 'drawing', label: 'Draw' },
		];

		tabs.forEach((tab) => {
			const btn = document.createElement('button');
			btn.type = 'button';
			btn.className = `browser-setting-tab`;
			btn.id = `bs-tab-${tab.id}`;
			btn.textContent = tab.label;
			btn.addEventListener('click', () => {
				navContainer
					.querySelectorAll('.browser-setting-tab')
					.forEach((b) => b.classList.remove('active'));
				btn.classList.add('active');

				const contentsContainer = editorContainer.querySelector(
					'#tab-contents-browserSetting',
				) as HTMLElement;
				contentsContainer.replaceChildren();
				if (this.browserSettingOptions && this.browserSettingOptions[tab.id]) {
					const renderedTree = this.renderSettingsOption(
						this.browserSettingOptions[tab.id],
						tab.id,
					);
					renderedTree.classList.add('browser-settings-grid');
					contentsContainer.appendChild(renderedTree);
				} else {
					contentsContainer.textContent = _(
						'No settings available for {0}',
					).replace('{0}', tab.label);
				}
			});
			navContainer.appendChild(btn);
		});
		return navContainer;
	}

	private createBrowserSettingContentsContainer(): HTMLDivElement {
		const contentsContainer = document.createElement('div');
		contentsContainer.id = 'tab-contents-browserSetting';
		contentsContainer.textContent = _('Select a tab to browser settings.');
		return contentsContainer;
	}

	private createBrowserSettingActions(
		sharedConfigsContainer: HTMLElement,
	): HTMLDivElement {
		// In the dialog (left nav present) the modal's own Save/Cancel handles
		// saving, so skip these redundant per-section actions.
		if (window.showLeftNav) return document.createElement('div');

		const actionsContainer = document.createElement('div');
		actionsContainer.classList.add('browser-settings-editor-actions');

		const resetButton = this.createButtonWithIcon(
			'browser-settings-reset-button',
			'reset', // Use icon key
			_('Reset to default Document settings'),
			['button--vue-secondary', 'xcu-reset-icon'],
			async (button) => {
				const confirmed = window.confirm(
					_('Are you sure you want to reset Document settings?'),
				);
				if (!confirmed) {
					return;
				}
				this.browserSettingOptions = defaultBrowserSettingCopy();
				// A reset drops every choice the user had saved.
				this.storedBrowserSetting = {};
				this.createBrowserSettingForm(sharedConfigsContainer);
			},
			true, // icon-only
		);
		actionsContainer.appendChild(resetButton);

		const saveButton = this.createButtonWithText(
			'browser-settings-save-button',
			_('Save'),
			_('Save Document settings'),
			['button-primary'],
			async (button) => {
				button.disabled = true;

				const file = new File(
					[this.browserSettingsPayload()],
					'browsersetting.json',
					{
						type: 'application/json',
						lastModified: Date.now(),
					},
				);

				await this.uploadFile(this.PATH.browserSettingsUpload(), file);
				if (isCODesktop) {
					(window.parent as any).postMobileMessage('SYNCSETTINGS');
				}
				button.disabled = false;
			},
		);
		actionsContainer.appendChild(saveButton);

		return actionsContainer;
	}

	private createMaterialDesignIconContainer(
		iconSvgString: string,
	): HTMLSpanElement {
		const materialIconContainer = document.createElement('span');
		materialIconContainer.setAttribute('aria-hidden', 'true');
		materialIconContainer.setAttribute('role', 'img'); // Add role for accessibility where appropriate
		materialIconContainer.classList.add('material-design-icon');
		materialIconContainer.innerHTML = iconSvgString; // Safe as it's from trusted SVG_ICONS

		return materialIconContainer;
	}

	private createButtonWithIcon(
		id: string,
		iconKey: keyof typeof this.SVG_ICONS, // Use a type-safe key
		title: string,
		classes: string[],
		onClickHandler: (button: HTMLButtonElement) => void,
		isIconOnly: boolean = false,
	): HTMLButtonElement {
		const buttonEl = document.createElement('button');
		if (id) {
			buttonEl.id = id;
		}
		buttonEl.type = 'button';
		buttonEl.classList.add('button', ...classes);
		if (isIconOnly) {
			buttonEl.classList.add('button--icon-only');
		} else {
			buttonEl.classList.add('button--icon-and-text');
		}
		buttonEl.title = title;

		const wrapperSpan = document.createElement('span');
		wrapperSpan.classList.add('button__wrapper');
		buttonEl.appendChild(wrapperSpan);

		const iconSpan = document.createElement('span');
		iconSpan.setAttribute('aria-hidden', 'true');
		iconSpan.classList.add('button__icon');
		wrapperSpan.appendChild(iconSpan);

		// Now correctly creates the inner span and injects the SVG
		iconSpan.appendChild(
			this.createMaterialDesignIconContainer(this.SVG_ICONS[iconKey]),
		);

		if (!isIconOnly) {
			const textSpan = document.createElement('span');
			textSpan.classList.add('button__text');
			textSpan.textContent = title;
			wrapperSpan.appendChild(textSpan);
		}

		buttonEl.addEventListener('click', () => onClickHandler(buttonEl));
		return buttonEl;
	}

	private createButtonWithText(
		id: string,
		text: string,
		title: string,
		classes: string[],
		onClickHandler: (button: HTMLButtonElement) => void,
	): HTMLButtonElement {
		const buttonEl = document.createElement('button');
		if (id) {
			buttonEl.id = id;
		}
		buttonEl.type = 'button';
		buttonEl.classList.add('button', 'button--text-only', ...classes);
		buttonEl.title = title;

		const wrapperSpan = document.createElement('span');
		wrapperSpan.classList.add('button__wrapper');
		buttonEl.appendChild(wrapperSpan);

		const textSpan = document.createElement('span');
		textSpan.classList.add('button__text');
		textSpan.textContent = text;
		wrapperSpan.appendChild(textSpan);

		buttonEl.addEventListener('click', () => onClickHandler(buttonEl));
		return buttonEl;
	}

	public renderSettingsOption(data: any, pathPrefix: string = ''): HTMLElement {
		const container = document.createElement('div');
		if (typeof data !== 'object' || data === null) {
			container.textContent = String(data);
			return container;
		}
		for (const key in data) {
			// skip accessibilityState as it's only used for determining existing state of Help -> screen reader toggle button
			if (key === 'accessibilityState') continue;

			if (Object.prototype.hasOwnProperty.call(data, key)) {
				const value = data[key];
				const uniqueId = pathPrefix ? `${pathPrefix}-${key}` : key;
				if (
					typeof value === 'object' &&
					value?.customType &&
					this.customRenderers[value.customType]
				) {
					const customElement = this.customRenderers[value.customType](
						key,
						value,
						uniqueId,
					);
					container.appendChild(customElement);
					continue;
				}
				if (
					typeof value === 'object' &&
					value !== null &&
					!Array.isArray(value)
				) {
					container.appendChild(this.createFieldset(key, value, uniqueId));
				} else {
					container.appendChild(
						this.createCheckboxToggle(key, value, uniqueId, data),
					);
				}
			}
		}
		return container;
	}

	private createFieldset(
		key: string,
		value: any,
		uniqueId: string,
	): HTMLFieldSetElement {
		const fieldset = document.createElement('fieldset');
		fieldset.classList.add('xcu-settings-fieldset');
		if (uniqueId.startsWith('Grid-')) {
			fieldset.classList.add('grid-options-fieldset');
		}
		const legend = document.createElement('legend');
		legend.textContent =
			this.settingLabels[uniqueId] || this.settingLabels[key] || key;
		fieldset.appendChild(legend);
		const childContent = this.renderSettingsOption(value, uniqueId);
		fieldset.appendChild(childContent);
		return fieldset;
	}

	// Helper to create a checkbox input element.
	private createCheckboxInput(
		id: string,
		isChecked: boolean,
		isDisabled: boolean,
	): HTMLInputElement {
		const inputCheckbox = document.createElement('input');
		inputCheckbox.type = 'checkbox';
		inputCheckbox.className = 'checkbox-radio-switch-input';
		inputCheckbox.id = id + '-input';
		inputCheckbox.checked = isChecked;
		inputCheckbox.disabled = isDisabled;
		return inputCheckbox;
	}

	private createCheckbox(
		id: string,
		isChecked: boolean,
		labelText: string,
		onClickHandler: (
			checkboxInput: HTMLInputElement,
			checkboxWrapper: HTMLSpanElement,
		) => void,
		isDisabled: boolean = false,
		warningText: string | null = null,
		helpText: string | null = null,
	): HTMLSpanElement {
		const checkboxWrapper = document.createElement('span');
		checkboxWrapper.className = `checkbox-radio-switch checkbox-radio-switch-checkbox ${isChecked ? '' : 'checkbox-radio-switch--checked'} checkbox-wrapper`;
		id = id.replace(/\s/g, '');
		checkboxWrapper.id = id + '-container';
		// On the row rather than the label, so the box answers to the pointer
		// as well as the words beside it.
		if (helpText) checkboxWrapper.title = helpText;

		// Use the new helper here
		const inputCheckbox = this.createCheckboxInput(id, isChecked, isDisabled);
		checkboxWrapper.appendChild(inputCheckbox);

		const checkboxContent = document.createElement('span');
		checkboxContent.className =
			'checkbox-content checkbox-content-checkbox checkbox-content--has-text checkbox-radio-switch__content';
		checkboxContent.id = id + '-content';
		checkboxWrapper.appendChild(checkboxContent);

		checkboxContent.appendChild(inputCheckbox);

		const checkboxLabel = document.createElement('label');
		checkboxLabel.className =
			'checkbox-content__text checkbox-radio-switch__text';
		checkboxLabel.textContent = labelText;
		checkboxLabel.htmlFor = inputCheckbox.id;
		checkboxContent.appendChild(checkboxLabel);

		if (warningText) {
			const container = document.createElement('div');
			container.className = 'checkbox-content__inner';
			container.appendChild(checkboxLabel);
			const warningEl = document.createElement('label');
			warningEl.className = 'ui-state-error-text';
			warningEl.textContent = warningText;
			container.appendChild(warningEl);
			checkboxContent.appendChild(container);
			checkboxContent.classList.add('checkbox-content--with-warning');
		}

		if (!isDisabled) {
			let that = this;
			const checkboxClickHandler = function () {
				onClickHandler(inputCheckbox, checkboxWrapper);
				if (checkboxWrapper.id === 'Grid-ShowGrid-container') {
					that.toggleGridOptionsVisibility(checkboxWrapper);
				}
			};

			inputCheckbox.addEventListener('click', checkboxClickHandler);
			inputCheckbox.addEventListener('keydown', (event) => {
				if (event.key === ' ' || event.key === 'Enter') {
					event.preventDefault();
					inputCheckbox.click();
				}
			});
			if (checkboxWrapper.id === 'Grid-ShowGrid-container') {
				// Set the initial state of Grid fieldsets' visibility
				setTimeout(() => this.toggleGridOptionsVisibility(checkboxWrapper), 0);
			}
		} else {
			checkboxWrapper.classList.add('checkbox-radio-switch--disabled');
		}

		return checkboxWrapper;
	}

	private createCheckboxToggle(
		key: string,
		value: boolean,
		uniqueId: string,
		data: any,
	): HTMLSpanElement {
		const title = this.settingTitles[uniqueId];
		const labelText =
			(title && title[0]) ||
			this.settingLabels[uniqueId] ||
			this.settingLabels[key] ||
			key;
		const helpText = title ? title[1] : null;
		let isDisabled = false;
		let warningText: string | null = null;

		if (key === 'lockAccessibilityOn') {
			isDisabled = !window.enableAccessibility;
			if (isDisabled) {
				warningText = _(
					'(Warning: Server accessibility must be enabled to toggle)',
				);
			}
		}

		return this.createCheckbox(
			uniqueId,
			value && !isDisabled,
			labelText,
			(inputCheckbox, checkboxWrapper) => {
				checkboxWrapper.classList.toggle(
					'checkbox-radio-switch--checked',
					!inputCheckbox.checked,
				);
				data[key] = inputCheckbox.checked;
			},
			isDisabled,
			warningText,
			helpText,
		);
	}

	private static isSettingGroup(value: any): boolean {
		return (
			typeof value === 'object' &&
			value !== null &&
			!Array.isArray(value) &&
			!('customType' in value)
		);
	}

	// browsersetting.json also holds preferences this dialog does not know about:
	// the resolved-comments choice, the open sidebar deck, the hidden status bar
	// entries. Carry those over from the stored file so a save here keeps them.
	private withStoredExtras(
		settings: Record<string, any>,
		stored: Record<string, any> = this.storedBrowserSetting,
	): Record<string, any> {
		const merged: Record<string, any> = { ...settings };

		for (const [key, storedValue] of Object.entries(stored ?? {})) {
			const value = merged[key];
			if (value === undefined) merged[key] = storedValue;
			else if (
				SettingIframe.isSettingGroup(value) &&
				SettingIframe.isSettingGroup(storedValue)
			)
				merged[key] = this.withStoredExtras(value, storedValue);
		}

		return merged;
	}

	// The Interface Settings as flat, dotted keys, the shape a browser preference
	// has: {"text.ShowAnnotations": "false"}. A toggle becomes the string "true"
	// or "false", the way the document records it. A number such as the zoom
	// index stays a number, so the file keeps the same value whichever side
	// writes it.
	private flattenBrowserSettings(
		settings: Record<string, any>,
		parentKey: string = '',
		flattened: Record<string, string | number> = {},
	): Record<string, string | number> {
		for (const [key, entry] of Object.entries(settings)) {
			const fullKey = parentKey ? `${parentKey}.${key}` : key;
			if (SettingIframe.isSettingGroup(entry)) {
				this.flattenBrowserSettings(entry, fullKey, flattened);
				continue;
			}

			// A custom-widget setting carries its value beside the widget type.
			const isWidget =
				entry !== null && typeof entry === 'object' && !Array.isArray(entry);
			const value = isWidget ? entry.value : entry;

			if (Array.isArray(value)) flattened[fullKey] = JSON.stringify(value);
			else if (typeof value === 'boolean')
				flattened[fullKey] = value ? 'true' : 'false';
			else if (typeof value === 'number') flattened[fullKey] = value;
			else flattened[fullKey] = String(value);
		}

		return flattened;
	}

	// The Interface Settings as they should be written to browsersetting.json,
	// collected from what the page shows.
	private browserSettingsPayload(): string {
		this.collectBrowserSettingsFromUI();

		const settings = this.withStoredExtras(this.browserSettingOptions);

		const followServerZoomCheckbox =
			this._zoomSection?.querySelector<HTMLInputElement>(
				'#common-followServerZoom-input',
			);
		// An empty zoom is what the document, the settings form and the zoom
		// dropdown all read as "the user chose none of their own", so it is how a
		// user hands the choice back to the server.
		if (followServerZoomCheckbox?.checked) {
			settings.smartZoom = '';
			settings.defaultZoom = '';
		}

		return JSON.stringify(settings);
	}

	/**!
	 * Reads the Interface Settings back off the page. They are shown in two
	 * sections, the Interface Settings and the Zoom Behaviour beside it.
	 */
	private collectBrowserSettingsFromUI(): void {
		this.collectBrowserSettingsFromSection(this._browserSettingSection);
		this.collectBrowserSettingsFromSection(this._zoomSection);
	}

	private collectBrowserSettingsFromSection(
		browserSettingSection: HTMLElement | null,
	): void {
		if (!browserSettingSection) return;

		const inputs = browserSettingSection.querySelectorAll<HTMLInputElement>(
			'input.checkbox-radio-switch-input,select.dic-input-container',
		);

		inputs.forEach((input) => {
			// Expected ID: section-setting-input (e.g., "writer-ShowSidebar-input")
			const parts = input.id.split('-');
			const controls = ['input', 'select'];
			if (parts.length !== 3 || !controls.includes(parts[2])) return;

			const [sectionRaw, settingKey] = parts;
			let value: any = undefined;
			switch (parts[2]) {
				case 'input':
					value = input.checked;
					break;
				case 'select':
					value = input.value;
					break;
			}

			if (sectionRaw === 'common') {
				// Following the server is not a setting of its own. It is written
				// out as an empty zoom, which every reader takes as "not chosen".
				if (settingKey === 'followServerZoom') return;

				if (settingKey === 'defaultZoom') value = parseInt(value);

				this.browserSettingOptions[settingKey] = value;

				if (settingKey === 'lockAccessibilityOn')
					this.browserSettingOptions['accessibilityState'] = value;
			} else {
				(this.browserSettingOptions[sectionRaw] as Record<string, boolean>)[
					settingKey
				] = value;
			}
		});
	}

	private customRenderers: Record<
		string,
		(key: string, value: any, uniqueId: string) => HTMLElement
	> = {
		compactToggle: this.renderCompactModeToggle.bind(this),
		zoomDropdown: this.renderZoomDropdown.bind(this),
	};

	private renderCompactModeToggle(
		key: string,
		setting: any,
		uniqueId: string,
	): HTMLElement {
		const container = document.createElement('div');
		container.className = 'custom-compact-toggle';

		const inputCheckbox = document.createElement('input');
		inputCheckbox.type = 'checkbox';
		inputCheckbox.className = 'checkbox-radio-switch-input';
		inputCheckbox.id = uniqueId + '-input';
		inputCheckbox.checked = setting.value;
		inputCheckbox.style.display = 'none'; // hidden input for logic
		container.appendChild(inputCheckbox);

		const options = document.createElement('div');
		options.className = 'toggle-options';

		const select = (useCompact: boolean) => {
			inputCheckbox.checked = useCompact;
			setting.value = useCompact;
			notebookImage.classList.toggle('selected', !useCompact);
			compactImage.classList.toggle('selected', useCompact);
		};

		const notebookOption = this.createCompactToggleOption(
			'Notebookbar.svg',
			'Notebookbar',
			_('Notebookbar view'),
			!setting.value,
			() => select(false),
		);
		const compactOption = this.createCompactToggleOption(
			'Compact.svg',
			'Compact',
			_('Compact view'),
			setting.value,
			() => select(true),
		);

		const notebookImage = notebookOption.querySelector(
			'.toggle-image',
		) as HTMLImageElement;
		const compactImage = compactOption.querySelector(
			'.toggle-image',
		) as HTMLImageElement;

		options.appendChild(notebookOption);
		options.appendChild(compactOption);
		container.appendChild(options);

		return container;
	}

	/*
		This seems to be the only way to render a widget which is not checkbox,
		atleast for the browser settings. `renderSettingsOption` adds a prefix to
		the widget's id based on which section of browser settings the widget is.
		`collectBrowserSettingsFromUI` then uses the id to get the value from the
		html widget and then updates the values in `browserSettingOptions` which are
		then written to the browsersetting.json file.

		it's tempting to use just number for defaultZoom, but then in `renderSettingsOption`,
		it's uncertain what to do with that number. with booleans it's clear that we need
		a toggle. with numbers, it can be either of spinfield, dropdown, textinput...
		`customType` handles that well by allowing us to handle these custom types separately.
	*/
	private renderZoomDropdown(
		key: string,
		setting: any,
		uniqueId: string,
	): HTMLElement {
		const container: HTMLDivElement = document.createElement('div');
		container.id = uniqueId;
		container.classList.add('view-input-container');

		const heading = this.createHeading(this.settingLabels.defaultZoom);
		heading.classList.add('view-setting-small-label');
		container.appendChild(heading);

		const zoomOptions = ZOOM_LEVELS.map((zoom, index) => ({
			value: index.toString(),
			label: zoom.toString(),
		}));

		const getDefaultZoomValueId = function (zoom: number | undefined): string {
			if (
				zoom === undefined ||
				!Number.isInteger(zoom) ||
				zoom < 0 ||
				zoom >= ZOOM_LEVELS.length
			)
				return defaultBrowserSetting.defaultZoom.value.toString();
			return zoom.toString();
		};

		const zoomDropdown = this.createSelectInput(
			uniqueId + '-select',
			zoomOptions,
			getDefaultZoomValueId(setting.value),
			(selected) => {
				const zoomLevel = selected.value;
				if (zoomLevel) setting.value = parseInt(zoomLevel);
			},
		);

		container.appendChild(zoomDropdown);
		return container;
	}

	private createCompactToggleOption(
		imageSrc: string,
		imageAlt: string,
		labelText: string,
		isSelected: boolean,
		onClick: () => void,
	): HTMLDivElement {
		const optionDiv = document.createElement('div');
		optionDiv.className = 'toggle-option';

		const image = document.createElement('img');
		let src = `${window.serviceRoot}/browser/${window.versionHash}/admin/images/${imageSrc}`;
		if (isCODesktop) src = `admin/images/${imageSrc}`;
		image.src = src;
		image.alt = imageAlt;
		image.className = `toggle-image ${isSelected ? 'selected' : ''}`;
		optionDiv.appendChild(image);

		const label = document.createElement('div');
		label.textContent = labelText;
		label.className = 'toggle-image-label';
		optionDiv.appendChild(label);

		image.addEventListener('click', onClick);

		return optionDiv;
	}

	private async uploadFile(
		filePath: string,
		file: File,
		currentFileUrl?: string,
	): Promise<void> {
		try {
			await this.settingsStorage.uploadSettings(filePath, file, currentFileUrl);
			await this.fetchAndPopulateSharedConfigs();
		} catch (error: unknown) {
			const message = error instanceof Error ? error.message : 'Unknown error';
			console.error(`Error uploading file: ${message}`);
			SettingIframe.showErrorModal(
				_('Something went wrong while uploading the file. Please try again.'),
			);
		}
	}

	private populateList(
		listId: string,
		items: ConfigItem[],
		category: string,
	): void {
		const listEl = document.getElementById(listId);
		if (!listEl) return;

		listEl.replaceChildren();

		items.forEach((item) => {
			const fileName = this.getFilename(item.uri, false);
			const li = document.createElement('li');
			li.classList.add('list-item__wrapper');

			const listItemDiv = document.createElement('div');
			listItemDiv.classList.add('list-item');

			listItemDiv.appendChild(this.createListItemAnchor(fileName));
			listItemDiv.appendChild(
				this.createListItemActions(item, category, fileName),
			);

			li.appendChild(listItemDiv);
			listEl.appendChild(li);
		});
	}

	private createListItemAnchor(fileName: string): HTMLDivElement {
		const anchor = document.createElement('div');
		anchor.classList.add('list-item__anchor');

		const listItemContentDiv = document.createElement('div');
		listItemContentDiv.classList.add('list-item-content');

		const listItemContentMainDiv = document.createElement('div');
		listItemContentMainDiv.classList.add('list-item-content__main');

		const listItemContentNameDiv = document.createElement('div');
		listItemContentNameDiv.classList.add('list-item-content__name');
		listItemContentNameDiv.textContent = fileName;

		listItemContentMainDiv.appendChild(listItemContentNameDiv);
		listItemContentDiv.appendChild(listItemContentMainDiv);
		anchor.appendChild(listItemContentDiv);

		return anchor;
	}

	private createListItemActions(
		item: ConfigItem,
		category: string,
		fileName: string,
	): HTMLDivElement {
		const extraActionsDiv = document.createElement('div');
		extraActionsDiv.classList.add('list-item-content__extra-actions');

		extraActionsDiv.appendChild(
			this.createButtonWithIcon(
				'', // No specific ID needed for list item buttons
				'download', // Use icon key
				item.uri, // Use URI as title for download link
				['button--vue-secondary', 'download-icon'],
				(button) => window.open(item.uri, '_blank', 'noopener'),
				true,
			),
		);
		extraActionsDiv.appendChild(
			this.createButtonWithIcon(
				'',
				'delete', // Use icon key
				_('Delete'),
				['button--vue-secondary', 'delete-icon'],
				async (button) => {
					try {
						const fileId =
							this.settingConfigBasePath() + category + '/' + fileName;
						await this.settingsStorage.deleteSettingsConfig(fileId);
						await this.fetchAndPopulateSharedConfigs();
					} catch (error: unknown) {
						SettingIframe.showErrorModal(
							_(
								'Something went wrong while deleting the file. Please try refreshing the page.',
							),
						);
						console.error('Error deleting file:', error);
					}
				},
				true,
			),
		);

		if (category === '/wordbook') {
			extraActionsDiv.appendChild(
				this.createButtonWithIcon(
					'',
					'edit', // Use icon key
					_('Edit'),
					['button--vue-secondary', 'edit-icon'],
					async () => await this.fetchWordbookFile(item.uri),
					true,
				),
			);
		}
		return extraActionsDiv;
	}

	// Ask the native side for the stored user interface zoom percentage.
	// Values outside the offered list (including zero) mean "as per system".
	// Returns null when the native side gives no answer.
	private async fetchUIZoomPercent(): Promise<number | null> {
		try {
			const reply = await (window.parent as any).postMobileCall('GETUIZOOM');
			const percent = parseInt(String(reply), 10);
			if (isNaN(percent)) {
				console.error('Unexpected user interface zoom reply:', reply);
				return null;
			}
			return UI_ZOOM_LEVELS.includes(percent) ? percent : 0;
		} catch (error) {
			console.error('Failed to fetch the user interface zoom:', error);
			return null;
		}
	}

	private generateAccessibilitySettingsUI(settingsContainer: HTMLElement) {
		const accessibilityContainer = document.createElement('div');
		accessibilityContainer.id = 'accessibility-section';
		accessibilityContainer.classList.add('section');

		accessibilityContainer.appendChild(this.createHeading(_('Accessibility')));

		const zoomDivContainer = document.createElement('div');
		zoomDivContainer.id = 'accessibility-ui-zoom';
		zoomDivContainer.classList.add('view-input-container');
		accessibilityContainer.appendChild(zoomDivContainer);

		const zoomLabel = this.createHeading(_('User interface zoom'));
		zoomLabel.classList.add('view-setting-small-label');
		zoomDivContainer.appendChild(zoomLabel);

		const zoomDescription = this.createParagraph(
			_('Magnify the whole window, including the document and the menus.'),
		);
		zoomDescription.className = 'view-setting-description';
		zoomDivContainer.appendChild(zoomDescription);

		const zoomOptions = [
			{ value: '0', label: _('As per system') },
			...UI_ZOOM_LEVELS.map((percent) => ({
				value: percent.toString(),
				label: percent + '%',
			})),
		];

		const zoomSelect = this.createSelectInput(
			'uiZoomSelect',
			zoomOptions,
			this._uiZoomPercent.toString(),
			(select) => {
				this._uiZoomPercent = parseInt(select.value, 10) || 0;
			},
		);
		// The choice is offered only once the stored zoom is known, so a save
		// always starts from the value the native side holds.
		zoomSelect.disabled = this._uiZoomStoredPercent === null;
		zoomDivContainer.appendChild(zoomSelect);

		this._accessibilitySection = this.mountConfigSection(
			settingsContainer,
			this._accessibilitySection,
			accessibilityContainer,
		);
	}

	private generateZoteroUI(data: ViewSettings, settingsContainer: HTMLElement) {
		const zoteroContainer = document.createElement('div');
		zoteroContainer.id = 'zotero-section';
		zoteroContainer.classList.add('section');

		zoteroContainer.appendChild(this.createHeading('Zotero'));
		const zoteroDescription = this.createParagraph(
			_(
				'To use Zotero specify your API key here. You can create your API key in your ',
			),
		);
		zoteroDescription.className = 'view-setting-description';

		const zoteroAccountHref = 'https://www.zotero.org/settings/keys';
		const zoteroAccountLink = document.createElement('a');
		zoteroAccountLink.href = zoteroAccountHref;
		zoteroAccountLink.target = '_blank';
		zoteroAccountLink.textContent = _('Zotero account API settings');
		// The dialog is in an iframe, and global.js's window.open override
		// (which reroutes external links through the HYPERLINK bridge) lives
		// on the parent window only - call through window.parent so CODA
		// (Qt/Windows/macOS) hands the URL off to the system browser instead
		// of letting the embedded webview navigate target=_blank itself.
		zoteroAccountLink.addEventListener('click', (e: MouseEvent) => {
			e.preventDefault();
			window.parent.open(zoteroAccountHref, '_blank', 'noopener');
		});

		zoteroDescription.appendChild(zoteroAccountLink);
		zoteroContainer.appendChild(zoteroDescription);

		const zoteroDivContainer = document.createElement('div');
		zoteroDivContainer.id = 'zotero-editor';
		zoteroContainer.appendChild(zoteroDivContainer);

		zoteroDivContainer.appendChild(
			this.createViewSettingsTextBox('zoteroAPIKey', data, true),
		);

		zoteroContainer.appendChild(
			this.createSettingsActions(
				'zotero',
				_('Zotero Settings'),
				'viewsetting.json',
				() => {
					const defaultSettings = this.getDefaultViewSettings();
					return {
						...this._viewSetting,
						zoteroAPIKey: defaultSettings.zoteroAPIKey,
					};
				},
				() => this._viewSetting,
				(settings) =>
					this.uploadViewSettingFile(
						'viewsetting.json',
						JSON.stringify(settings),
					),
			),
		);
		this._zoteroSection = this.mountConfigSection(
			settingsContainer,
			this._zoteroSection,
			zoteroContainer,
		);
	}

	private static getDefaultServerPrivateSettings(): ServerPrivateSettings {
		return {
			ESignatureBaseUrl: '',
			ESignatureClientId: '',
			ESignatureSecret: '',
		};
	}

	// The fields an administrator edits here. The service address lives in the
	// same file but is not offered for editing, and the stored-secret flag is
	// not a field of its own.
	private static readonly EDITABLE_SERVER_PRIVATE_FIELDS = [
		'ESignatureClientId',
		'ESignatureSecret',
	] as const satisfies readonly (keyof ServerPrivateSettings)[];

	private static serverPrivateSettingsFile(
		settings: ServerPrivateSettings,
	): File {
		return new File([JSON.stringify(settings)], 'serverprivateinfo.json', {
			type: 'application/json',
			lastModified: Date.now(),
		});
	}

	async uploadServerPrivateSettingFile(
		settings: ServerPrivateSettings,
	): Promise<void> {
		await this.uploadFile(
			this.PATH.serverPrivateSettingsUpload(),
			SettingIframe.serverPrivateSettingsFile(settings),
			this._serverPrivateSettingFileUrl,
		);
	}

	private generateServerPrivateSettingUI(
		data: ServerPrivateSettings,
		settingsContainer: HTMLElement,
	) {
		this._serverPrivateSetting = data;

		const container = document.createElement('div');
		container.id = 'serverprivate-section';
		container.classList.add('section');

		container.appendChild(this.createHeading(_('Electronic Signature')));
		const description = this.createParagraph(
			_(
				'Credentials for the electronic signature service. They are shared by everyone on this server.',
			),
		);
		description.className = 'view-setting-description';
		container.appendChild(description);

		const editor = document.createElement('div');
		editor.id = 'serverprivate-editor';
		container.appendChild(editor);

		for (const key of SettingIframe.EDITABLE_SERVER_PRIVATE_FIELDS) {
			const field = this.createInputField(
				key,
				this._serverPrivateSettingLabels[key] || key,
				data[key],
				data,
				false,
				true,
			);
			editor.appendChild(field);
		}

		container.appendChild(
			this.createSettingsActions(
				'serverprivatesettings',
				_('Electronic Signature'),
				'serverprivateinfo.json',
				// Reset keeps the stored address, which is not the admin's to change.
				() => ({
					...SettingIframe.getDefaultServerPrivateSettings(),
					ESignatureBaseUrl: this._serverPrivateSetting.ESignatureBaseUrl,
				}),
				() => this._serverPrivateSetting,
				(settings) => this.uploadServerPrivateSettingFile(settings),
			),
		);

		this._serverPrivateSection = this.mountConfigSection(
			settingsContainer,
			this._serverPrivateSection,
			container,
		);
	}

	private generateDocSigningUI(
		data: ViewSettings,
		settingsContainer: HTMLElement,
	) {
		const docSigningContainer = document.createElement('div');
		docSigningContainer.id = 'doc-signing-section';
		docSigningContainer.classList.add('section');

		docSigningContainer.appendChild(this.createHeading(_('Document Signing')));
		const signingDesc = document.createElement('p');
		signingDesc.className = 'view-setting-description';
		signingDesc.textContent = _(
			'To use document signing, specify your signing certificate, key and CA chain here.',
		);
		docSigningContainer.appendChild(signingDesc);

		const docSigningDivContainer = document.createElement('div');
		docSigningDivContainer.id = 'doc-signing-editor';
		docSigningContainer.appendChild(docSigningDivContainer);

		docSigningDivContainer.appendChild(
			this.createViewSettingsTextBox('signatureCert', data, false, true),
		);
		docSigningDivContainer.appendChild(
			this.createViewSettingsTextBox('signatureKey', data, false, true),
		);
		docSigningDivContainer.appendChild(
			this.createViewSettingsTextBox('signatureCa', data, false, true),
		);

		docSigningContainer.appendChild(
			this.createSettingsActions(
				'document-signing',
				_('Document Signing Settings'),
				'viewsetting.json',
				() => {
					const defaultSettings = this.getDefaultViewSettings();
					return {
						...this._viewSetting,
						signatureCert: defaultSettings.signatureCert,
						signatureKey: defaultSettings.signatureKey,
						signatureCa: defaultSettings.signatureCa,
					};
				},
				() => this._viewSetting,
				(settings) =>
					this.uploadViewSettingFile(
						'viewsetting.json',
						JSON.stringify(settings),
					),
			),
		);
		this._docSigningSection = this.mountConfigSection(
			settingsContainer,
			this._docSigningSection,
			docSigningContainer,
		);
	}

	private generateAISettingsUI(
		data: ViewSettings,
		settingsContainer: HTMLElement,
	) {
		if (window.disableAISettings) {
			return;
		}

		const aiContainer = document.createElement('div');
		aiContainer.id = 'ai-section';
		aiContainer.classList.add('section');

		const header = document.createElement('div');
		header.classList.add('ai-section-header');
		const intro = document.createElement('div');
		intro.classList.add('ai-section-intro');
		intro.appendChild(this.createHeading(_('AI Assistant')));
		const aiDesc = document.createElement('p');
		aiDesc.id = 'ai-description';
		aiDesc.className = 'view-setting-description';
		intro.appendChild(aiDesc);
		header.appendChild(intro);
		const resetButton = this.createButtonWithIcon(
			'ai-reset-button',
			'updateAll',
			_('Reset'),
			['button-primary'],
			() => this.resetAIProviders(data, aiContainer),
		);
		resetButton.title = _('Use the AI provider set on the server');
		header.appendChild(resetButton);
		aiContainer.appendChild(header);

		aiContainer.appendChild(this.createAIEmptyState(data, aiContainer));

		const aiDivContainer = document.createElement('div');
		aiDivContainer.id = 'ai-editor';
		aiContainer.appendChild(aiDivContainer);

		aiDivContainer.appendChild(this.createAISettingsBlock(data));
		this.showAIEmptyState(
			aiContainer,
			!data.aiProviderURL && this.isImageProviderSameAsText(data, aiContainer),
		);

		aiContainer.appendChild(
			this.createSettingsActions(
				'ai',
				_('AI Assistant'),
				'viewsetting.json',
				// The section header has its own Reset.
				null,
				() => this._viewSetting,
				(settings) =>
					this.uploadViewSettingFile(
						'viewsetting.json',
						JSON.stringify(settings),
					),
			),
		);
		this._aiSection = this.mountConfigSection(
			settingsContainer,
			this._aiSection,
			aiContainer,
		);
	}

	// Shown while the user has no provider of their own, so the one set on
	// the server answers for both text and image generation.
	private createAIEmptyState(
		data: ViewSettings,
		root: HTMLElement,
	): HTMLDivElement {
		const emptyState = document.createElement('div');
		emptyState.id = 'ai-empty-state';

		const header = document.createElement('div');
		header.classList.add('ai-providers-header');
		const title = document.createElement('span');
		title.textContent = _('Providers');
		const tag = document.createElement('span');
		tag.classList.add('ai-badge', 'ai-admin-tag');
		tag.textContent = _('Managed by your admin');
		header.append(title, tag);
		emptyState.appendChild(header);

		const emptyTitle = document.createElement('div');
		emptyTitle.classList.add('ai-empty-state-title');
		emptyTitle.textContent = _('No providers added yet');
		emptyState.appendChild(emptyTitle);

		const emptyDesc = document.createElement('p');
		emptyDesc.className = 'view-setting-description';
		emptyDesc.textContent = _(
			"Add a provider to start using AI for text generation, image generation, or both. Your administrator's provider is filled in for you.",
		);
		emptyState.appendChild(emptyDesc);

		emptyState.appendChild(
			this.createButtonWithIcon(
				'ai-add-provider-button',
				'add',
				_('Add provider...'),
				['button--vue-tertiary'],
				() => this.addAIProvider(data, root),
			),
		);
		return emptyState;
	}

	private showAIEmptyState(root: HTMLElement, empty: boolean): void {
		const show = (selector: string, visible: boolean) => {
			const el = root.querySelector(selector) as HTMLElement | null;
			if (el) el.style.display = visible ? '' : 'none';
		};
		show('#ai-empty-state', empty);
		show('#ai-editor', !empty);
		show('#ai-reset-button', !empty);

		const description = root.querySelector('#ai-description');
		if (description) {
			description.textContent = empty
				? _('Choose the AI provider to use for text and image generation.')
				: _(
						'Use your own AI provider instead of the one set up by your administrator. This applies only to you.',
					);
		}
	}

	private addAIProvider(data: ViewSettings, root: HTMLElement): void {
		const providerSelect = root.querySelector(
			'#aiProvider',
		) as HTMLSelectElement | null;
		if (providerSelect) {
			providerSelect.value = AI_PROVIDERS[0].id;
			providerSelect.dispatchEvent(new Event('change'));
		}
		this.showAIEmptyState(root, false);
		providerSelect?.focus();
	}

	private createViewSettingsTextBox(
		key: keyof ViewSettings,
		data: ViewSettings,
		skipHeading: boolean = false,
		isSmallHeading: boolean = false,
	): HTMLDivElement {
		const text = data[key] as string;
		const label = this._viewSettingLabels[key] || key;

		return this.createInputField(
			key as string,
			label,
			text,
			data,
			skipHeading,
			isSmallHeading,
		);
	}

	private createAISettingsBlock(data: ViewSettings): HTMLDivElement {
		const container = document.createElement('div');
		container.id = 'ai-settings-container';

		container.appendChild(this.createTextAIGroup(data));
		container.appendChild(this.createImageAIGroup(data));
		container.appendChild(this.createRequestTimeoutField(data));

		this.attachAISettingsAutoFetch(data, container);
		this.attachAIImageSettingsAutoFetch(data, container);

		// Schedule unconditionally; fetchAIModels decides whether it has enough
		// to run (a base URL for self-hosted, or a key for a cloud provider).
		this.scheduleAIModelFetch(data);
		this.scheduleAIImageModelFetch(data);

		return container;
	}

	// A group of AI fields under a title, with the status of its model fetch
	// beside the title. The group is named by the title alone, so the status
	// is not read out as part of its name.
	private createAIGroup(
		id: string,
		title: string,
		statusId: string,
	): HTMLFieldSetElement {
		const group = document.createElement('fieldset');
		group.classList.add('ai-settings-group');
		group.setAttribute('aria-labelledby', `${id}-title`);

		const legend = document.createElement('legend');
		const titleEl = document.createElement('span');
		titleEl.id = `${id}-title`;
		titleEl.textContent = title;
		legend.appendChild(titleEl);

		const status = document.createElement('span');
		status.id = statusId;
		status.className = 'ai-badge ai-model-status';
		status.setAttribute('role', 'status');
		legend.appendChild(status);

		group.appendChild(legend);
		return group;
	}

	private createImageSizeField(data: ViewSettings): HTMLDivElement {
		// A size saved by hand before this became a list stays on offer, so
		// opening the dialog does not change it.
		const sizes =
			data.aiImageSize && !AI_IMAGE_SIZES.includes(data.aiImageSize)
				? [...AI_IMAGE_SIZES, data.aiImageSize]
				: AI_IMAGE_SIZES;
		return this.createSelectField(
			'aiImageSizecontainer',
			this._viewSettingLabels.aiImageSize,
			'aiImageSize',
			sizes.map((size) => ({ value: size, label: size.replace('x', ' × ') })),
			data.aiImageSize || AI_IMAGE_SIZES[0],
			(selectEl) => {
				data.aiImageSize = selectEl.value;
			},
		);
	}

	// A labelled dropdown, laid out like the text fields of createInputField.
	private createSelectField(
		containerId: string,
		label: string,
		selectId: string,
		options: Array<{ value: string; label: string }>,
		selectedValue: string,
		onChangeHandler: (select: HTMLSelectElement) => void,
	): HTMLDivElement {
		const field = document.createElement('div');
		field.id = containerId;
		field.classList.add('view-input-container');

		const heading = this.createHeading(label);
		heading.classList.add('view-setting-small-label');
		field.appendChild(heading);

		// The wrapper draws the chevron, in the text colour of the theme.
		const selectWrapper = document.createElement('div');
		selectWrapper.classList.add('select-field');
		selectWrapper.appendChild(
			this.createSelectInput(selectId, options, selectedValue, onChangeHandler),
		);
		field.appendChild(selectWrapper);
		return field;
	}

	private getAIProviderOptions(): Array<{ value: string; label: string }> {
		return AI_PROVIDERS.map((provider) => ({
			value: provider.id,
			label: provider.name,
		}));
	}

	private createRequestTimeoutField(data: ViewSettings): HTMLDivElement {
		const field = this.createViewSettingsTextBox(
			'aiRequestTimeout',
			data,
			false,
			true,
		);
		const input = field.querySelector(
			'#aiRequestTimeout',
		) as HTMLInputElement | null;
		if (input) {
			input.type = 'number';
			input.min = '10';
			input.placeholder = '300';

			const row = document.createElement('div');
			row.classList.add('ai-input-with-unit');
			input.replaceWith(row);
			const unit = document.createElement('span');
			unit.textContent = _('seconds');
			row.append(input, unit);
		}
		return field;
	}

	private createTextAIGroup(data: ViewSettings): HTMLFieldSetElement {
		const group = this.createAIGroup(
			'aiTextGroup',
			_('Text generation'),
			'ai-model-status',
		);

		group.appendChild(
			this.createSelectField(
				'aiProvidercontainer',
				this._viewSettingLabels.aiProvider,
				'aiProvider',
				[
					{ value: DEFAULT_PROVIDER, label: _('Default') },
					...this.getAIProviderOptions(),
				],
				this.getSelectedProviderId(data, group),
				(selectEl) => {
					const provider = this.getProviderById(selectEl.value);
					if (provider && !provider.isCustom) {
						data.aiProviderURL = provider.baseUrl;
					}
				},
			),
		);

		group.appendChild(
			this.createViewSettingsTextBox('aiProviderURL', data, false, true),
		);
		const customUrlContainer = group.querySelector(
			'#aiProviderURLcontainer',
		) as HTMLElement | null;
		if (customUrlContainer) {
			customUrlContainer.style.display = this.isCustomProviderSelected(
				group,
				data,
			)
				? 'block'
				: 'none';
		}
		const customUrlInput = group.querySelector(
			'#aiProviderURL',
		) as HTMLInputElement | null;
		if (customUrlInput) {
			customUrlInput.placeholder = _('e.g.') + ' http://localhost:11434';
		}

		group.appendChild(
			this.createViewSettingsTextBox('aiProviderAPIKey', data, false, true),
		);
		const apiKeyInput = group.querySelector(
			'#aiProviderAPIKey',
		) as HTMLInputElement | null;
		if (apiKeyInput) {
			apiKeyInput.placeholder = _(
				'Leave empty if your server does not require one',
			);
		}

		const modelField = this.createSelectField(
			'aiModelcontainer',
			this._viewSettingLabels.aiProviderModel,
			'aiProviderModel',
			this.initialModelOptions(data.aiProviderModel),
			data.aiProviderModel || '',
			(selectEl) => {
				data.aiProviderModel = selectEl.value;
			},
		);
		const modelSelect = modelField.querySelector('select') as HTMLSelectElement;
		modelSelect.disabled = true;

		const modelHint = document.createElement('p');
		modelHint.id = 'aiModelHint';
		modelHint.className = 'view-setting-description';
		modelHint.textContent = _(
			'To edit documents, pick a model with tool calling, e.g. gpt-4o or llama3.1. Models without it (e.g. base llama3, gemma) can still chat but cannot use document tools.',
		);
		modelSelect.setAttribute('aria-describedby', modelHint.id);
		modelField.appendChild(modelHint);

		group.appendChild(modelField);

		if (this.getProviderIdFromUrl(data.aiProviderURL) === 'custom') {
			this._lastCustomAIProviderURL = data.aiProviderURL;
		}

		this.syncAISettingsVisibility(data, group);

		return group;
	}

	private createImageAIGroup(data: ViewSettings): HTMLFieldSetElement {
		const group = this.createAIGroup(
			'aiImageGroup',
			_('Image generation'),
			'ai-image-model-status',
		);

		group.appendChild(
			this.createCheckbox(
				'aiImageSameAsText',
				this.isImageProviderSameAsText(data, group),
				_('Same as text provider'),
				(inputCheckbox, checkboxWrapper) => {
					checkboxWrapper.classList.toggle(
						'checkbox-radio-switch--checked',
						!inputCheckbox.checked,
					);
					this.applyImageProviderSameAsText(inputCheckbox.checked, data, group);
				},
			),
		);
		const providerFields = document.createElement('div');
		providerFields.id = 'aiImageProviderFields';
		group.appendChild(providerFields);
		this._imageProviderMemo = null;

		providerFields.appendChild(
			this.createSelectField(
				'aiImageProvidercontainer',
				this._viewSettingLabels.aiImageProvider,
				'aiImageProvider',
				this.getAIProviderOptions(),
				this.getProviderIdFromUrl(
					data.aiImageProviderURL || data.aiProviderURL || '',
				),
				(selectEl) => {
					const provider = this.getProviderById(selectEl.value);
					if (provider && !provider.isCustom) {
						data.aiImageProviderURL = provider.baseUrl;
					}
				},
			),
		);

		providerFields.appendChild(
			this.createViewSettingsTextBox('aiImageProviderURL', data, false, true),
		);
		const imageUrlInput = group.querySelector(
			'#aiImageProviderURL',
		) as HTMLInputElement | null;
		if (imageUrlInput) {
			imageUrlInput.placeholder = _('e.g.') + ' http://localhost:11434';
		}

		providerFields.appendChild(
			this.createViewSettingsTextBox(
				'aiImageProviderAPIKey',
				data,
				false,
				true,
			),
		);
		const imageApiKeyInput = group.querySelector(
			'#aiImageProviderAPIKey',
		) as HTMLInputElement | null;
		if (imageApiKeyInput) {
			imageApiKeyInput.type = 'password';
		}

		const modelField = this.createSelectField(
			'aiImageModelcontainer',
			this._viewSettingLabels.aiImageModel,
			'aiImageModel',
			this.initialModelOptions(data.aiImageModel),
			data.aiImageModel || '',
			(selectEl) => {
				data.aiImageModel = selectEl.value;
			},
		);
		(modelField.querySelector('select') as HTMLSelectElement).disabled = true;
		group.appendChild(modelField);

		group.appendChild(this.createImageSizeField(data));

		if (
			data.aiImageProviderURL &&
			this.getProviderIdFromUrl(data.aiImageProviderURL) === 'custom'
		) {
			this._lastCustomAIImageProviderURL = data.aiImageProviderURL;
		}

		this.syncAIImageSettingsVisibility(data, group);

		return group;
	}

	private syncAISettingsVisibility(
		data: ViewSettings,
		root: ParentNode = document,
	): void {
		const isCustomProvider = this.isCustomProviderSelected(root, data);
		const customUrlContainer = root.querySelector(
			'#aiProviderURLcontainer',
		) as HTMLElement | null;
		if (customUrlContainer) {
			customUrlContainer.style.display = isCustomProvider ? 'block' : 'none';
		}
		const isDefaultProvider =
			this.getSelectedProviderId(data, root) === DEFAULT_PROVIDER;
		for (const id of ['#aiProviderAPIKeycontainer', '#aiModelcontainer']) {
			const field = root.querySelector(id) as HTMLElement | null;
			if (field) field.style.display = isDefaultProvider ? 'none' : '';
		}
	}

	private attachAISettingsAutoFetch(
		data: ViewSettings,
		root: ParentNode = document,
	): void {
		const providerInput = root.querySelector(
			'#aiProvider',
		) as HTMLSelectElement | null;
		const apiKeyInput = root.querySelector(
			'#aiProviderAPIKey',
		) as HTMLInputElement | null;
		const customUrlInput = root.querySelector(
			'#aiProviderURL',
		) as HTMLInputElement | null;
		const modelSelect = root.querySelector(
			'#aiProviderModel',
		) as HTMLSelectElement | null;

		const queueFetch = () => {
			this._aiConfigDirty = true;
			this.scheduleAIModelFetch(data);
			// Re-fetch image models too when image inherits chat credentials
			this.scheduleAIImageModelFetch(data);
		};

		providerInput?.addEventListener('change', () => {
			const selectedProvider = this.getProviderById(providerInput.value);
			if (providerInput.value === DEFAULT_PROVIDER) {
				data.aiProviderURL = '';
				data.aiProviderModel = '';
				data.aiProviderAPIKey = '';
				data.aiProviderAPIKeyStored = false;
				if (apiKeyInput) {
					apiKeyInput.value = '';
					apiKeyInput.placeholder = _(
						'Leave empty if your server does not require one',
					);
				}
				this.syncSecretField('aiProviderAPIKey', data, root);
			} else if (selectedProvider && !selectedProvider.isCustom) {
				if (customUrlInput) {
					this._lastCustomAIProviderURL = customUrlInput.value;
				}
				data.aiProviderURL = selectedProvider.baseUrl;
				if (customUrlInput) {
					customUrlInput.value = selectedProvider.baseUrl;
				}
			} else if (customUrlInput) {
				customUrlInput.value = this._lastCustomAIProviderURL;
				data.aiProviderURL = customUrlInput.value;
			} else {
				data.aiProviderURL = '';
			}
			this.syncAISettingsVisibility(data, root);
			queueFetch();
		});

		apiKeyInput?.addEventListener('input', () => {
			data.aiProviderAPIKey = apiKeyInput.value;
			// The typed value is now authoritative, not the stored one.
			data.aiProviderAPIKeyStored = false;
			queueFetch();
		});

		customUrlInput?.addEventListener('input', () => {
			if (this.isCustomProviderSelected(root, data)) {
				data.aiProviderURL = customUrlInput.value;
				this._lastCustomAIProviderURL = customUrlInput.value;
				queueFetch();
			}
		});

		// On blur, rewrite the field to the canonical base URL (no trailing
		// "/v1"), so the admin sees the value that will actually be stored.
		customUrlInput?.addEventListener('change', () => {
			if (this.isCustomProviderSelected(root, data)) {
				const normalized = this.normalizeBaseUrl(customUrlInput.value);
				customUrlInput.value = normalized;
				data.aiProviderURL = normalized;
				this._lastCustomAIProviderURL = normalized;
			}
		});

		modelSelect?.addEventListener('change', () => {
			this._aiConfigDirty = true;
			data.aiProviderModel = modelSelect.value;
		});
	}

	private attachAIImageSettingsAutoFetch(
		data: ViewSettings,
		root: ParentNode = document,
	): void {
		const providerInput = root.querySelector(
			'#aiImageProvider',
		) as HTMLSelectElement | null;
		const apiKeyInput = root.querySelector(
			'#aiImageProviderAPIKey',
		) as HTMLInputElement | null;
		const customUrlInput = root.querySelector(
			'#aiImageProviderURL',
		) as HTMLInputElement | null;
		const modelSelect = root.querySelector(
			'#aiImageModel',
		) as HTMLSelectElement | null;

		const queueFetch = () => {
			this.scheduleAIImageModelFetch(data);
		};

		providerInput?.addEventListener('change', () => {
			const selectedProvider = this.getProviderById(providerInput.value);
			if (selectedProvider && !selectedProvider.isCustom) {
				if (customUrlInput) {
					this._lastCustomAIImageProviderURL = customUrlInput.value;
				}
				data.aiImageProviderURL = selectedProvider.baseUrl;
				if (customUrlInput) {
					customUrlInput.value = selectedProvider.baseUrl;
				}
			} else if (customUrlInput) {
				customUrlInput.value = this._lastCustomAIImageProviderURL;
				data.aiImageProviderURL = customUrlInput.value;
			} else {
				data.aiImageProviderURL = '';
			}
			this.syncAIImageSettingsVisibility(data, root);
			queueFetch();
		});

		apiKeyInput?.addEventListener('input', () => {
			data.aiImageProviderAPIKey = apiKeyInput.value;
			// The typed value is now authoritative, not the stored one.
			data.aiImageProviderAPIKeyStored = false;
			this.syncAIImageKeyRequirement(data, root);
			queueFetch();
		});

		customUrlInput?.addEventListener('input', () => {
			const imageProvider = root.querySelector(
				'#aiImageProvider',
			) as HTMLSelectElement | null;
			if (imageProvider?.value === 'custom') {
				data.aiImageProviderURL = customUrlInput.value;
				this._lastCustomAIImageProviderURL = customUrlInput.value;
				queueFetch();
			}
		});

		// On blur, rewrite the field to the canonical base URL (no trailing
		// "/v1"), so the admin sees the value that will actually be stored.
		customUrlInput?.addEventListener('change', () => {
			const imageProvider = root.querySelector(
				'#aiImageProvider',
			) as HTMLSelectElement | null;
			if (imageProvider?.value === 'custom') {
				const normalized = this.normalizeBaseUrl(customUrlInput.value);
				customUrlInput.value = normalized;
				data.aiImageProviderURL = normalized;
				this._lastCustomAIImageProviderURL = normalized;
			}
		});

		modelSelect?.addEventListener('change', () => {
			data.aiImageModel = modelSelect.value;
		});
	}

	// Image generation borrows the Text Generation provider while it has no
	// endpoint and no key of its own - what an empty aiImageProviderURL has
	// always meant - so the checkbox reads and writes those fields.
	//
	// Once the section is on screen the checkbox answers, because the fields go
	// through states the stored ones cannot tell apart: a custom provider whose
	// URL has not been typed yet is empty too, and must not read as "same as
	// text". The stored fields answer only while the section is being built.
	private isImageProviderSameAsText(
		data: ViewSettings,
		root: ParentNode = document,
	): boolean {
		const checkbox = root.querySelector(
			'#aiImageSameAsText-input',
		) as HTMLInputElement | null;
		return checkbox
			? checkbox.checked
			: !data.aiImageProviderURL &&
					!data.aiImageProviderAPIKey &&
					!data.aiImageProviderAPIKeyStored;
	}

	private syncAIImageSettingsVisibility(
		data: ViewSettings,
		root: ParentNode = document,
	): void {
		const imageProvider = root.querySelector(
			'#aiImageProvider',
		) as HTMLSelectElement | null;
		const imageUrlContainer = root.querySelector(
			'#aiImageProviderURLcontainer',
		) as HTMLElement | null;
		if (imageUrlContainer) {
			imageUrlContainer.style.display =
				imageProvider?.value === 'custom' ? 'block' : 'none';
		}

		const providerFields = root.querySelector(
			'#aiImageProviderFields',
		) as HTMLElement | null;
		if (providerFields) {
			providerFields.style.display = this.isImageProviderSameAsText(data, root)
				? 'none'
				: 'block';
		}

		this.syncAIImageKeyRequirement(data, root);
	}

	private imageInheritsTextProvider(data: ViewSettings): boolean {
		return (
			!data.aiImageProviderURL ||
			this.normalizeBaseUrl(data.aiImageProviderURL) ===
				this.normalizeBaseUrl(data.aiProviderURL || '')
		);
	}

	private syncAIImageKeyRequirement(
		data: ViewSettings,
		root: ParentNode = document,
	): void {
		const apiKeyInput = root.querySelector(
			'#aiImageProviderAPIKey',
		) as HTMLInputElement | null;
		if (!apiKeyInput) {
			return;
		}

		const imageProvider = root.querySelector(
			'#aiImageProvider',
		) as HTMLSelectElement | null;
		const required =
			!this.imageInheritsTextProvider(data) &&
			imageProvider?.value !== 'custom';
		const hasKey =
			!!data.aiImageProviderAPIKey || !!data.aiImageProviderAPIKeyStored;
		const missing = required && !hasKey;

		apiKeyInput.required = required;
		apiKeyInput.setAttribute('aria-invalid', missing ? 'true' : 'false');
		apiKeyInput.classList.toggle('view-input-invalid', missing);
		apiKeyInput.placeholder = data.aiImageProviderAPIKeyStored
			? '********'
			: required
				? _('Required for this provider')
				: _('Leave empty if your server does not require one');
	}

	private applyImageProviderSameAsText(
		sameAsText: boolean,
		data: ViewSettings,
		root: ParentNode,
	): void {
		const memo = this._imageProviderMemo;
		this._imageProviderMemo = sameAsText
			? {
					url: data.aiImageProviderURL,
					apiKey: data.aiImageProviderAPIKey,
					apiKeyStored: !!data.aiImageProviderAPIKeyStored,
				}
			: null;

		const url = sameAsText
			? ''
			: memo?.url || data.aiProviderURL || this.getDefaultAIProviderURL();
		data.aiImageProviderURL = url;
		data.aiImageProviderAPIKey = sameAsText ? '' : memo?.apiKey || '';
		data.aiImageProviderAPIKeyStored = sameAsText
			? false
			: !!memo?.apiKeyStored;
		if (url && this.getProviderIdFromUrl(url) === 'custom') {
			this._lastCustomAIImageProviderURL = url;
		}

		const setValue = (selector: string, value: string) => {
			const field = root.querySelector(selector) as
				| HTMLInputElement
				| HTMLSelectElement
				| null;
			if (field) field.value = value;
		};
		if (url) setValue('#aiImageProvider', this.getProviderIdFromUrl(url));
		setValue('#aiImageProviderURL', url);
		setValue('#aiImageProviderAPIKey', data.aiImageProviderAPIKey);
		this.syncSecretField('aiImageProviderAPIKey', data, root);

		this.syncAIImageSettingsVisibility(data, root);
		this.scheduleAIImageModelFetch(data);
	}

	private scheduleAIModelFetch(data: ViewSettings): void {
		if (this._aiModelFetchTimeout) {
			window.clearTimeout(this._aiModelFetchTimeout);
		}
		this._aiModelFetchTimeout = window.setTimeout(() => {
			this.fetchAIModels(data);
		}, 600);
	}

	private scheduleAIImageModelFetch(data: ViewSettings): void {
		if (this._aiImageModelFetchTimeout) {
			window.clearTimeout(this._aiImageModelFetchTimeout);
		}
		this._aiImageModelFetchTimeout = window.setTimeout(() => {
			this.fetchAIImageModels(data);
		}, 600);
	}

	private async fetchAIImageModels(data: ViewSettings): Promise<void> {
		// Compute effective credentials (image-specific or fallback to chat)
		const effectiveUrl =
			this.normalizeBaseUrl(data.aiImageProviderURL || '') ||
			this.normalizeBaseUrl(data.aiProviderURL || '');
		const inheritsTextProvider = this.imageInheritsTextProvider(data);
		const effectiveKey =
			data.aiImageProviderAPIKey ||
			(inheritsTextProvider ? data.aiProviderAPIKey : '');
		const effectiveProviderId = this.getProviderIdFromUrl(effectiveUrl);
		const provider = this.getProviderById(effectiveProviderId);
		const isCustom = provider?.isCustom ?? effectiveProviderId === 'custom';

		// Which saved key the server should read back: image key, else chat key.
		const storedImageSecretField = data.aiImageProviderAPIKeyStored
			? 'aiImageProviderAPIKey'
			: inheritsTextProvider && data.aiProviderAPIKeyStored
				? 'aiProviderAPIKey'
				: '';
		const canUseStoredKey =
			!isCODesktop && !!storedImageSecretField && !!this._viewSettingFileUrl;

		// As for text: a custom (self-hosted) image provider lists models with
		// just a base URL; the pre-canned cloud providers still need a key.
		if (!effectiveUrl || (!isCustom && !effectiveKey && !canUseStoredKey)) {
			// Say which of the two it is, so a provider picked here without a key
			// of its own does not just sit there with an empty model list.
			if (effectiveUrl && !isCustom && !inheritsTextProvider) {
				this.setAIImageStatus(
					_('Enter an API key for this provider to list its models.'),
					'error',
				);
			} else {
				this.setAIImageStatus('', 'hidden');
			}
			this.resetAIImageModelSelect(data.aiImageModel);
			return;
		}

		this._aiImageModelFetchSeq += 1;
		const seq = this._aiImageModelFetchSeq;

		this._aiImageModelFetchAbort?.abort();
		this._aiImageModelFetchAbort = new AbortController();

		this.setAIImageStatus(_('Fetching models...'), 'loading');

		try {
			const providerId = provider ? provider.id : effectiveProviderId;
			let json: any;
			if (isCODesktop) {
				// No server proxy on the desktop; the native app fetches the models.
				const body = await (window.parent as any).postMobileCall(
					'FETCHAIMODELS ' +
						JSON.stringify({
							provider: providerId,
							apiKey: effectiveKey,
							baseUrl: effectiveUrl,
						}),
				);
				json = JSON.parse(body);
				if (json.error) throw new Error(json.error);
			} else {
				const formData = new FormData();
				formData.append('provider', providerId);
				formData.append('apiKey', effectiveKey);
				formData.append('baseUrl', effectiveUrl);
				if (!effectiveKey && canUseStoredKey) {
					formData.append('currentFileUrl', this._viewSettingFileUrl);
					formData.append('accessToken', window.accessToken ?? '');
					formData.append('secretField', storedImageSecretField);
				}

				const response = await fetch(this.getAPIEndpoints().fetchModels, {
					method: 'POST',
					body: formData,
					signal: this._aiImageModelFetchAbort.signal,
				});

				if (!response.ok) {
					throw new Error(
						this.describeModelFetchError(
							response.status,
							await response.text(),
						),
					);
				}

				json = await response.json();
			}
			const allModels = (json.data || []) as AIModelEntry[];
			if (!Array.isArray(allModels) || allModels.length === 0) {
				this.setAIImageStatus(_('No models found'), 'error');
				return;
			}

			if (seq !== this._aiImageModelFetchSeq) {
				return;
			}

			// Filter to image-capable models; fall back to everything if that
			// would leave the dropdown empty (a custom provider may use a name
			// we don't recognise).
			const imageModels = allModels.filter(isImageModel);
			const filtered = imageModels.length > 0 ? imageModels : allModels;
			const modelIds = filtered.map((m) => m.id).filter(Boolean);
			if (modelIds.length === 0) {
				this.setAIImageStatus(_('No models found'), 'error');
				return;
			}

			const selectedModel = modelIds.includes(data.aiImageModel)
				? data.aiImageModel
				: '';
			data.aiImageModel = selectedModel;
			this.updateAIImageModelSelect(modelIds, selectedModel);

			this.setAIImageStatus(_('Connected'), 'success');
		} catch (error) {
			if ((error as any)?.name === 'AbortError') {
				return;
			}
			const message =
				error instanceof Error ? error.message : _('Failed to fetch models');
			this.setAIImageStatus(message, 'error');
			this.resetAIImageModelSelect(data.aiImageModel);
		}
	}

	private async fetchAIModels(data: ViewSettings): Promise<void> {
		const providerId = this.getSelectedProviderId(data);
		if (providerId === DEFAULT_PROVIDER) {
			this.setAIStatus('', 'hidden');
			return;
		}
		const provider = this.getProviderById(providerId);
		if (!provider) {
			this.setAIStatus(_('Invalid provider configuration'), 'error');
			return;
		}

		const isCustom = provider.isCustom ?? false;
		const baseUrl = isCustom
			? this.normalizeBaseUrl(data.aiProviderURL || '')
			: provider.baseUrl;
		const apiKey = data.aiProviderAPIKey || '';

		// No typed key, but a saved one the server can read back.
		const canUseStoredKey =
			!isCODesktop &&
			!!data.aiProviderAPIKeyStored &&
			!!this._viewSettingFileUrl;

		// A self-hosted (custom) provider can list its models with just a base
		// URL; the pre-canned cloud providers still need a key to reach theirs.
		if (isCustom ? !baseUrl : !apiKey && !canUseStoredKey) {
			this.setAIStatus('', 'hidden');
			this.resetAIModelSelect(data.aiProviderModel);
			return;
		}

		this._aiModelFetchSeq += 1;
		const seq = this._aiModelFetchSeq;

		this._aiModelFetchAbort?.abort();
		this._aiModelFetchAbort = new AbortController();

		this.setAIStatus(_('Fetching models...'), 'loading');

		try {
			let json: any;
			if (isCODesktop) {
				// No server proxy on the desktop; the native app fetches the models.
				const body = await (window.parent as any).postMobileCall(
					'FETCHAIMODELS ' +
						JSON.stringify({ provider: provider.id, apiKey, baseUrl }),
				);
				json = JSON.parse(body);
				if (json.error) throw new Error(json.error);
			} else {
				const formData = new FormData();
				formData.append('provider', provider.id);
				formData.append('apiKey', apiKey);
				formData.append('baseUrl', baseUrl);
				if (!apiKey && canUseStoredKey) {
					formData.append('currentFileUrl', this._viewSettingFileUrl);
					formData.append('accessToken', window.accessToken ?? '');
					formData.append('secretField', 'aiProviderAPIKey');
				}

				const response = await fetch(this.getAPIEndpoints().fetchModels, {
					method: 'POST',
					body: formData,
					signal: this._aiModelFetchAbort.signal,
				});

				if (!response.ok) {
					throw new Error(
						this.describeModelFetchError(
							response.status,
							await response.text(),
						),
					);
				}

				json = await response.json();
			}
			const allModels = (json.data || []) as AIModelEntry[];
			if (!Array.isArray(allModels) || allModels.length === 0) {
				this.setAIStatus(_('No models found'), 'error');
				return;
			}

			if (seq !== this._aiModelFetchSeq) {
				return;
			}

			// Filter to chat-capable models; fall back to everything if that
			// would leave the dropdown empty.
			const chatModels = allModels.filter(isChatModel);
			const filtered = chatModels.length > 0 ? chatModels : allModels;
			const modelIds = filtered.map((m) => m.id).filter(Boolean);
			if (modelIds.length === 0) {
				this.setAIStatus(_('No models found'), 'error');
				return;
			}

			const selectedModel = modelIds.includes(data.aiProviderModel)
				? data.aiProviderModel
				: modelIds[0];
			data.aiProviderModel = selectedModel;
			this.updateAIModelSelect(modelIds, selectedModel);

			this.setAIStatus(_('Connected'), 'success');
		} catch (error) {
			if ((error as any)?.name === 'AbortError') {
				return;
			}
			const message =
				error instanceof Error ? error.message : _('Failed to fetch models');
			this.setAIStatus(message, 'error');
			this.resetAIModelSelect(data.aiProviderModel);
		}
	}

	private updateAIModelSelect(modelIds: string[], selectedModel: string): void {
		const modelSelect = document.getElementById(
			'aiProviderModel',
		) as HTMLSelectElement | null;
		if (!modelSelect) {
			return;
		}
		modelSelect.replaceChildren();
		modelIds.forEach((modelId) => {
			const option = document.createElement('option');
			option.value = modelId;
			option.textContent = modelId;
			modelSelect.appendChild(option);
		});
		modelSelect.value = selectedModel;
		modelSelect.disabled = modelIds.length === 0;
	}

	private updateAIImageModelSelect(
		modelIds: string[],
		selectedModel: string,
	): void {
		const select = document.getElementById(
			'aiImageModel',
		) as HTMLSelectElement | null;
		if (!select) return;
		select.replaceChildren();
		const noneOpt = document.createElement('option');
		noneOpt.value = '';
		noneOpt.textContent = _('None (disable image generation)');
		select.appendChild(noneOpt);
		modelIds.forEach((modelId) => {
			const option = document.createElement('option');
			option.value = modelId;
			option.textContent = modelId;
			select.appendChild(option);
		});
		select.value = selectedModel;
		select.disabled = false;
	}

	// Seed a model dropdown so a saved model still shows before (or without) a
	// fresh fetch, which needs a key the browser no longer holds.
	private initialModelOptions(
		storedModel: string,
	): Array<{ value: string; label: string }> {
		const options = [{ value: '', label: _('Fetch models to select') }];
		if (storedModel) {
			options.push({ value: storedModel, label: storedModel });
		}
		return options;
	}

	private fillModelSelect(
		select: HTMLSelectElement,
		storedModel: string,
	): void {
		select.replaceChildren();
		this.initialModelOptions(storedModel).forEach((opt) => {
			const option = document.createElement('option');
			option.value = opt.value;
			option.textContent = opt.label;
			select.appendChild(option);
		});
		select.value = storedModel || '';
		select.disabled = true;
	}

	private resetAIImageModelSelect(storedModel: string = ''): void {
		const select = document.getElementById(
			'aiImageModel',
		) as HTMLSelectElement | null;
		if (!select) return;
		this.fillModelSelect(select, storedModel);
	}

	private resetAIModelSelect(storedModel: string = ''): void {
		const modelSelect = document.getElementById(
			'aiProviderModel',
		) as HTMLSelectElement | null;
		if (!modelSelect) {
			return;
		}
		this.fillModelSelect(modelSelect, storedModel);
	}

	// Build a concise, user-facing message for a failed model fetch. The raw
	// response body (which can be a verbose error page echoing the key and
	// internal paths) is logged for diagnostics rather than shown to the user.
	private describeModelFetchError(status: number, body: string): string {
		if (body) {
			console.warn(`fetch-models failed (HTTP ${status}): ${body}`);
		}
		// A 429 covers both a throttle and an exhausted quota. They need
		// opposite advice, so tell them apart from the error body: waiting
		// clears a throttle, but not a quota that is used up.
		if (status === 429 && /insufficient_quota/.test(body)) {
			return _('API quota exceeded - check your plan and billing details');
		}
		return (
			AI_ERROR_MESSAGES[status]?.() ||
			_(
				'Could not fetch models (HTTP {0}). Check the provider URL and API key.',
			).replace('{0}', String(status))
		);
	}

	private setAIStatus(message: string, state: AIStatusState = 'info'): void {
		this.applyModelStatus('ai-model-status', message, state);
	}

	private setAIImageStatus(
		message: string,
		state: AIStatusState = 'info',
	): void {
		this.applyModelStatus('ai-image-model-status', message, state);
	}

	// Drive a model-fetch status line. The data-state attribute selects the
	// alert styling and, by its presence, whether the line shows at all; a
	// 'hidden' state or empty message clears it.
	private applyModelStatus(
		id: string,
		message: string,
		state: AIStatusState,
	): void {
		const status = document.getElementById(id);
		if (!status) {
			return;
		}
		if (state === 'hidden' || !message) {
			status.textContent = '';
			status.removeAttribute('data-state');
			return;
		}
		status.textContent = message;
		status.setAttribute('data-state', state);
	}

	private scrollWindowTargetIntoView() {
		if (window.scrollTarget) {
			const element: HTMLElement | null | undefined = document.getElementById(
				window.scrollTarget,
			);
			if (element == null) {
				console.error(
					'window.scrollTarget element not found in settingsDialog, window.target (id): ' +
						window.scrollTarget,
				);
				return;
			}

			/**!
			 * The left navbar marks the section at the top of the list, so the
			 * whole section comes up. The target keeps its place inside it.
			 */
			const section = element.closest('.section') ?? element;
			section.scrollIntoView({
				behavior: 'smooth',
				block: 'start',
				inline: 'start',
			} as ScrollIntoViewOptions);
			if (!element.hasAttribute('tabindex')) element.tabIndex = -1;
			element.focus({ preventScroll: true });

			/* change the object's opacity from 0.2 to 1.0 to grab user's attention. */
			element.classList.add('element-grab-attention');
		}
	}

	// Runs twice: first with `data === null` to drop in empty, hidden
	// sections (heading only) so the left navbar can be built straight away,
	// then with the real data to fill each section in. The headings are there
	// from the start, so the navbar doesn't change as each section loads.
	/**!
	 * One panel per installed rule package. A package the engine does not have
	 * gets no panel, which is why the list comes from the engine rather than
	 * from the option table the labels live in.
	 *
	 * Named for the checker rather than for the job, because it is not the
	 * only sentence checker the dialog will carry: the remote ones get
	 * sections of their own.
	 */
	private createSentenceCheckerSection(): HTMLElement | null {
		if (!this.xcuEditor) return null;

		// The engine answers with the packages it has, in an order that means
		// nothing to a reader. Sort by the name each one is shown under, in
		// the reader's own language, so the list reads the way they expect
		// rather than the way a directory happened to be read back.
		const packages = (window.sentenceCheckingPackages || [])
			.slice()
			.sort((a, b) =>
				(this.settingLabels[a.id] || a.id).localeCompare(
					this.settingLabels[b.id] || b.id,
				),
			);

		const panels: HTMLElement[] = [];
		for (const entry of packages) {
			const group = this.xcuEditor.getSentenceCheckingGroup(entry.id);
			if (!group) continue;
			const rendered = this.renderSettingsOption(group, entry.id);
			rendered.classList.add('xcu-settings-grid');
			// The options are worded in the language they check, so the grid
			// says which one: a screen reader then reads them in that language
			// and the browser hyphenates a long label by its rules. The legend
			// around it is in the language of the UI, so it stays outside.
			rendered.lang = entry.id.replace('_', '-');
			panels.push(
				this.createFieldsetFor(
					this.settingLabels[entry.id] || entry.id,
					rendered,
				),
			);
		}

		// A heading over nothing is worse than no heading: with no rule
		// package installed there is nothing here to choose.
		if (!panels.length) return null;

		const section = document.createElement('div');
		section.id = 'lightproof';
		section.className = 'section';
		section.appendChild(this.createHeading(_('Lightproof Sentence Checker')));
		section.appendChild(
			this.createParagraph(
				_('Choose what the sentence checker looks for as you type.'),
			),
		);
		section.appendChild(this.createNextDocumentNote());
		for (const panel of panels) section.appendChild(panel);
		return section;
	}

	/**!
	 * The remote sentence checkers: LanguageTool, the Duden Korrekturserver
	 * and Grammalecte, one panel each.
	 *
	 * The engine says how each is set up. What a checker looks for, the
	 * Duden style checks and the Grammalecte rule families, is always the
	 * user's to choose. The server, the account and the key are shown
	 * where the server allows it, and always in the desktop app, which has
	 * no other place to set them: in Online one process checks the document
	 * for everyone who has it open, so one user must not be able to send
	 * everyone's text to a server of their choosing.
	 *
	 * A field that was never touched is not written: the engine keeps what
	 * the server, or the schema, gave it. The engine's value is shown as the
	 * field's starting value, and the first edit makes it the user's own.
	 */
	private createRemoteCheckerSection(): HTMLElement | null {
		if (!this.xcuEditor) return null;
		const infos = window.remoteCheckers || [];

		const panels: HTMLElement[] = [];
		for (const info of infos) {
			const groupName = {
				languagetool: 'LanguageTool',
				duden: 'Duden',
				grammalecte: 'Grammalecte',
			}[info.id];
			if (!groupName) continue;
			const group = this.xcuEditor.getRemoteCheckerGroup(groupName);
			if (!group) continue;
			const panel = this.createRemoteCheckerPanel(info, group);
			if (panel) panels.push(panel);
		}
		if (!panels.length) return null;

		const section = document.createElement('div');
		section.id = 'remote-checkers';
		section.className = 'section';
		section.appendChild(this.createHeading(_('Remote Sentence Checkers')));
		section.appendChild(
			this.createParagraph(
				_(
					'Checkers that run on a server. The text of the document is sent there to be checked.',
				),
			),
		);
		section.appendChild(this.createNextDocumentNote());
		for (const panel of panels) section.appendChild(panel);
		return section;
	}

	private createRemoteCheckerPanel(
		info: RemoteCheckerInfo,
		group: any,
	): HTMLElement | null {
		const canEditServer = isCODesktop || info.allowUserSettings;
		// Nothing to choose for a checker that is off and cannot be turned on
		// from here, nor for LanguageTool where the server is not theirs to
		// set: the style checks and the rule families are the other two's.
		const hasOwnChoices = info.id !== 'languagetool';
		if (!canEditServer && (!info.enabled || !hasOwnChoices)) return null;

		const content = document.createElement('div');
		content.classList.add('remote-checker');
		// The settings of a checker take room only while it is on; the
		// toggle alone stands for one that is off.
		const details = document.createElement('div');
		details.classList.add('remote-checker-details');

		if (canEditServer) {
			const isOn = group.IsEnabled === null ? info.enabled : !!group.IsEnabled;
			details.hidden = !isOn;
			content.appendChild(
				this.createRemoteToggle(
					info,
					group,
					'IsEnabled',
					_('Enabled'),
					info.enabled,
					(checked) => {
						details.hidden = !checked;
					},
				),
			);
			details.appendChild(
				this.createRemoteTextField(
					info,
					group,
					'BaseURL',
					_('Server URL'),
					info.baseURL,
				),
			);
			// LanguageTool's premium API takes an account name with the key;
			// the Duden Korrekturserver takes a token alone.
			if (info.id === 'languagetool') {
				details.appendChild(
					this.createRemoteTextField(
						info,
						group,
						'Username',
						_('Username'),
						info.username || '',
					),
				);
			}
			if (info.id !== 'grammalecte') {
				details.appendChild(
					this.createRemoteTextField(
						info,
						group,
						'ApiKey',
						_('API key'),
						'',
						true,
					),
				);
			}
			details.appendChild(
				this.createRemoteToggle(
					info,
					group,
					'SSLCertVerify',
					_('Verify the server certificate'),
					info.sslVerify,
				),
			);
		} else {
			// What the server set up, which is not theirs to change.
			details.appendChild(
				this.createParagraph(
					_('Enabled on this server for: {0}').replace(
						'{0}',
						(info.locales || []).join(', '),
					),
				),
			);
		}

		if (info.id === 'duden') {
			if (canEditServer) {
				details.appendChild(
					this.createRemoteSelect(
						info,
						group,
						'SpellcheckingLevel',
						_('Checking level'),
						[
							{ value: '0', label: _('None') },
							{ value: '1', label: _('Single words') },
							{ value: '2', label: _('Spelling') },
							{ value: '3', label: _('Spelling and grammar') },
						],
						String(info.spellcheckingLevel ?? 3),
					),
				);
				details.appendChild(
					this.createRemoteSelect(
						info,
						group,
						'OrthographyStandard',
						_('German spelling'),
						[
							{ value: 'duden', label: _('Duden recommendation') },
							{ value: 'conservative', label: _('Conservative') },
							{ value: 'progressive', label: _('Progressive') },
							{ value: 'extended', label: _('Tolerant') },
							{ value: 'press', label: _('Press agencies') },
						],
						info.orthographyStandard || 'duden',
					),
				);
				details.appendChild(
					this.createRemoteSelect(
						info,
						group,
						'MessageLevel',
						_('Explanations'),
						[
							{ value: '0', label: _('None') },
							{ value: '1', label: _('Short') },
							{ value: '2', label: _('Long') },
							{ value: '3', label: _('Long, with an example') },
						],
						String(info.messageLevel ?? 1),
					),
				);
				details.appendChild(
					this.createRemoteNameList(
						info,
						group,
						'PropertySets',
						_('Correction profiles'),
						info.propertySets || [],
						info.availablePropertySets || [],
					),
				);
				details.appendChild(
					this.createRemoteNameList(
						info,
						group,
						'Dictionaries',
						_('Dictionaries'),
						info.dictionaries || [],
						info.availableDictionaries || [],
					),
				);
			}
			const styleLabels: Record<string, string> = {
				Foreign: _('Foreign words with a German equivalent'),
				Old: _('Archaic words'),
				Regional: _('Regional words'),
				Colloquial: _('Colloquial and vulgar words'),
				Fillers: _('Filler words'),
				Offensive: _('Offensive wording'),
				GenderInclusive: _('Gender-inclusive wording'),
				Complexity: _('Complex sentences'),
			};
			// A switch the user has not set shows what the engine runs with.
			const styleGrid = document.createElement('div');
			styleGrid.classList.add('xcu-settings-grid');
			for (const key of Object.keys(styleLabels)) {
				styleGrid.appendChild(
					this.createCheckbox(
						`duden-Style-${key}`,
						!!(group.Style[key] ?? info.style?.[key]),
						styleLabels[key],
						(inputCheckbox, checkboxWrapper) => {
							checkboxWrapper.classList.toggle(
								'checkbox-radio-switch--checked',
								!inputCheckbox.checked,
							);
							group.Style[key] = inputCheckbox.checked;
						},
					),
				);
			}
			details.appendChild(
				this.createFieldsetFor(_('Style checks (German only)'), styleGrid),
			);
		}

		if (info.id === 'grammalecte') {
			// The server's own grouping of its rule families.
			const families: Array<[string, string[]]> = [
				[
					'Typographie',
					[
						'typo',
						'apos',
						'eepi',
						'esp',
						'tab',
						'nbsp',
						'unit',
						'tu',
						'maj',
						'minis',
						'num',
						'nf',
						'virg',
						'poncfin',
						'ocr',
						'chim',
						'liga',
						'mapos',
					],
				],
				['Noms et adjectifs', ['conf', 'loc', 'gn']],
				['Verbes', ['infi', 'conj', 'ppas', 'imp', 'inte', 'vmode']],
				['Style', ['bs', 'pleo', 'eleu', 'neg', 'redon1', 'redon2']],
				['Divers', ['date', 'mc']],
			];
			const options = group.Options;
			for (const [title, ids] of families) {
				const grid = document.createElement('div');
				grid.classList.add('xcu-settings-grid');
				for (const id of ids) {
					if (!(id in options)) continue;
					grid.appendChild(
						this.createCheckboxToggle(
							id,
							!!(options[id] ?? info.options?.[id]),
							`grammalecte-${id}`,
							options,
						),
					);
				}
				const family = this.createFieldsetFor(title, grid);
				// Worded in French, like the options of the checker.
				family.lang = 'fr';
				details.appendChild(family);
			}
		}

		content.appendChild(details);
		const panel = this.createFieldsetFor(
			this.settingLabels[info.id] || info.id,
			content,
		);
		panel.id = `remote-checker-${info.id}`;
		return panel;
	}

	// A toggle for a boolean of a remote checker's group. The engine's value
	// is shown until the user sets their own.
	private createRemoteToggle(
		info: RemoteCheckerInfo,
		group: any,
		key: string,
		label: string,
		effective: boolean,
		onChange?: (checked: boolean) => void,
	): HTMLElement {
		const value = group[key] === null ? effective : !!group[key];
		return this.createCheckbox(
			`${info.id}-${key}`,
			value,
			label,
			(inputCheckbox, checkboxWrapper) => {
				checkboxWrapper.classList.toggle(
					'checkbox-radio-switch--checked',
					!inputCheckbox.checked,
				);
				group[key] = inputCheckbox.checked;
				if (onChange) onChange(inputCheckbox.checked);
			},
		);
	}

	// A text field for a string of a remote checker's group. A secret is
	// never shown: a saved one is a masked placeholder, and the field stays
	// empty until the user types a new one.
	private createRemoteTextField(
		info: RemoteCheckerInfo,
		group: any,
		key: string,
		label: string,
		effective: string,
		isSecret: boolean = false,
	): HTMLElement {
		const container = document.createElement('div');
		container.id = `${info.id}-${key}container`;
		container.classList.add('view-input-container');
		const heading = this.createHeading(label);
		heading.classList.add('view-setting-small-label');
		container.appendChild(heading);

		const own = group[key] !== null && group[key] !== undefined;
		const placeholder =
			isSecret && !own && info.hasCredentials
				? '********'
				: _('Enter {0}').replace('{0}', label);
		const input = this.createTextInput(
			`${info.id}-${key}`,
			placeholder,
			own ? String(group[key]) : isSecret ? '' : effective,
			(inputElement) => {
				group[key] = inputElement.value;
			},
		);
		if (isSecret) input.type = 'password';
		container.appendChild(input);
		return container;
	}

	private createRemoteSelect(
		info: RemoteCheckerInfo,
		group: any,
		key: string,
		label: string,
		options: Array<{ value: string; label: string }>,
		effective: string,
	): HTMLElement {
		const own = group[key] !== null && group[key] !== undefined;
		return this.createSelectField(
			`${info.id}-${key}container`,
			label,
			`${info.id}-${key}`,
			options,
			own ? String(group[key]) : effective,
			(selectEl) => {
				group[key] = selectEl.value;
			},
		);
	}

	// A list of names the server knows: a multiple choice when the server
	// has said what it offers, a text field of names separated by spaces
	// when it has not.
	private createRemoteNameList(
		info: RemoteCheckerInfo,
		group: any,
		key: string,
		label: string,
		effective: string[],
		available: string[],
	): HTMLElement {
		const own = group[key] !== null && group[key] !== undefined;
		const current: string[] = own
			? String(group[key])
					.split(/\s+/)
					.filter((name) => name !== '')
			: effective;
		if (!available.length) {
			const field = this.createRemoteTextField(
				info,
				group,
				key,
				label,
				effective.join(' '),
			);
			return field;
		}

		const container = document.createElement('div');
		container.id = `${info.id}-${key}container`;
		container.classList.add('view-input-container');
		const heading = this.createHeading(label);
		heading.classList.add('view-setting-small-label');
		container.appendChild(heading);

		const select = document.createElement('select');
		select.id = `${info.id}-${key}`;
		select.multiple = true;
		select.size = Math.min(available.length, 5);
		// A name in use that the server no longer lists stays selectable,
		// so opening the dialog does not drop it.
		for (const name of [
			...available,
			...current.filter((name) => !available.includes(name)),
		]) {
			const option = document.createElement('option');
			option.value = name;
			option.textContent = name;
			option.selected = current.includes(name);
			select.appendChild(option);
		}
		select.addEventListener('change', () => {
			group[key] = Array.from(select.selectedOptions)
				.map((option) => option.value)
				.join(' ');
		});
		container.appendChild(select);
		return container;
	}

	/**!
	 * Said only when a change would not be felt in the document in front of
	 * them, which is the case that needs explaining. Alone with a document
	 * running on their own settings they get no note, because saving applies
	 * the change here and now.
	 *
	 * There are two ways to be short of that. A document opened by another
	 * session is not running on this user's settings at all. A document that
	 * is running on them, but is open in more than one place, would have them
	 * changed under whoever else is there, so it waits.
	 *
	 * Neither text says who the others are: another window of this user's own
	 * is a session like any other, and the dialog cannot tell the two apart.
	 */
	private createNextDocumentNote(): HTMLElement {
		const note = this.createParagraph('');
		note.className = 'settings-scope-note';
		// The note is not in the page yet, so it is written directly: looked
		// up through the document it would stay an empty, visible box until
		// the parent next reports a change.
		this.writeNextDocumentNote(note);
		return note;
	}

	private writeNextDocumentNote(note: HTMLElement): void {
		note.textContent =
			window.userPresetsApplied === false
				? _(
						'This document is using the document settings it was opened with. What you change here applies to the next document you open.',
					)
				: _(
						'This document is open in more than one window, so what you change here applies to the next document you open rather than this one.',
					);
		note.hidden = window.documentSettingsLive !== false;
	}

	/**!
	 * People join and leave while the dialog is open, so the note is written
	 * again whenever the parent says the answer has changed. Every note in
	 * the page is written: the sections are built more than once while the
	 * dialog fills in, and a note left over from an earlier build must not go
	 * on saying the opposite of the one beside it.
	 */
	private updateNextDocumentNote(): void {
		document
			.querySelectorAll<HTMLElement>('.settings-scope-note')
			.forEach((note) => this.writeNextDocumentNote(note));
	}

	private createFieldsetFor(
		legendText: string,
		content: HTMLElement,
	): HTMLElement {
		const fieldset = document.createElement('fieldset');
		fieldset.classList.add('xcu-settings-fieldset');
		const legend = document.createElement('legend');
		legend.textContent = legendText;
		fieldset.appendChild(legend);
		fieldset.appendChild(content);
		return fieldset;
	}

	private async populateSharedConfigUI(data: ConfigData | null): Promise<void> {
		const settingsContainer = this._allConfigSection;
		if (!settingsContainer) return;

		const isUserConfig = data ? data.kind === 'user' : !this.isAdmin();

		const browserSettingButton = document.getElementById(
			'uploadBrowserSettingsButton',
		) as HTMLButtonElement | null;

		if (browserSettingButton && data) {
			if (data.browsersetting && data.browsersetting.length > 0) {
				browserSettingButton.style.display = 'none';
			} else {
				browserSettingButton.style.removeProperty('display');
			}
		}

		// Accessibility (user interface zoom), applied natively by the desktop
		// apps
		if (isUserConfig && accessibilitySettingsSupported) {
			if (!data) {
				this._accessibilitySection = this.createEmptySection(
					this._accessibilitySection,
					'accessibility-section',
					_('Accessibility'),
				);
			} else {
				this._uiZoomStoredPercent = await this.fetchUIZoomPercent();
				this._uiZoomPercent = this._uiZoomStoredPercent ?? 0;
				this.generateAccessibilitySettingsUI(settingsContainer);
			}
		}

		// Interface settings
		if (isUserConfig && !isCODesktop) {
			if (!data) {
				this._browserSettingSection = this.createEmptySection(
					this._browserSettingSection,
					'browser-setting',
					_('Interface Settings'),
				);
				this._zoomSection = this.createEmptySection(
					this._zoomSection,
					'zoom-behaviour',
					_('Zoom Behaviour'),
				);
			} else {
				if (data.browsersetting && data.browsersetting.length > 0) {
					const browserSettingContent =
						await this.settingsStorage.fetchSettingFile(
							data.browsersetting[0].uri,
						);
					this.storedBrowserSetting = browserSettingContent
						? JSON.parse(browserSettingContent)
						: {};
					this.browserSettingOptions = browserSettingContent
						? this.mergeWithDefault(
								defaultBrowserSettingCopy(),
								this.storedBrowserSetting,
							)
						: defaultBrowserSettingCopy();
				} else {
					this.storedBrowserSetting = {};
					this.browserSettingOptions = defaultBrowserSettingCopy();
				}
				this.createBrowserSettingForm(settingsContainer);
			}
		}

		// Document settings (xcu), and the sentence checker options, which are
		// kept in the same file. The apps show the second but not the first:
		// their document settings are reached from the menus instead.
		{
			if (!data) {
				if (!isCODesktop)
					this._xcuSection = this.createEmptySection(
						this._xcuSection,
						'xcu-section',
						_('Document Settings'),
					);
			} else {
				if (data.xcu && data.xcu.length > 0) {
					const xcuFileContent = await this.settingsStorage.fetchSettingFile(
						data.xcu[0].uri,
					);
					this.xcuEditor = new window.Xcu(
						this.getFilename(data.xcu[0].uri, false),
						xcuFileContent,
					);

					if (!isCODesktop) {
						const xcuContainer = document.createElement('div');
						xcuContainer.id = 'xcu-section';
						xcuContainer.classList.add('section');
						const xcuSection = this.xcuEditor.createXcuEditorUI(xcuContainer);
						xcuSection.appendChild(this.createNextDocumentNote());
						this.appendXcuDebugUploadControls(xcuContainer, data);

						this._xcuSection = this.mountConfigSection(
							settingsContainer,
							this._xcuSection,
							xcuSection,
						);
					}
				} else if (!isCODesktop) {
					// If user doesn't have any xcu file, we generate with default settings...
					try {
						if (!this.xcuInitializationAttempted) {
							this.xcuInitializationAttempted = true;
							this.xcuEditor = new window.Xcu('documentView.xcu', null);
							await this.xcuEditor.generateXcuAndUpload();
							return await this.fetchAndPopulateSharedConfigs();
						} else {
							this._xcuSection?.remove();
							this._xcuSection = null;
							console.warn('XCU file not found and automatic creation failed.');
						}
					} catch (error) {
						console.error(
							'Something went wrong while generating or uploading xcu file:',
							error,
						);
						this._xcuSection?.remove();
						this._xcuSection = null;
					}
				} else {
					// The apps have no xcu until something writes one, and their
					// upload is a one-way message to the shell, so writing one
					// here and reloading would race it. Edit in memory instead;
					// saving writes the file through the same message.
					this.xcuEditor = new window.Xcu('documentView.xcu', null);
				}

				// The sentence checker options do not need the file to exist:
				// an option nobody has set yet falls back to its schema default.
				const sentenceChecker = this.createSentenceCheckerSection();
				if (sentenceChecker) {
					this._sentenceCheckerSection = this.mountConfigSection(
						settingsContainer,
						this._sentenceCheckerSection,
						sentenceChecker,
					);
				} else {
					this._sentenceCheckerSection?.remove();
					this._sentenceCheckerSection = null;
				}

				const remoteCheckers = this.createRemoteCheckerSection();
				if (remoteCheckers) {
					this._remoteCheckerSection = this.mountConfigSection(
						settingsContainer,
						this._remoteCheckerSection,
						remoteCheckers,
					);
				} else {
					this._remoteCheckerSection?.remove();
					this._remoteCheckerSection = null;
				}
			}
		}

		// AI settings
		if (isUserConfig) {
			if (!data && !window.disableAISettings) {
				this._aiSection = this.createEmptySection(
					this._aiSection,
					'ai-section',
					_('AI Assistant'),
				);
			} else if (data) {
				let viewSetting = this.getDefaultViewSettings();
				this._viewSettingFileUrl =
					data.viewsetting && data.viewsetting.length > 0
						? data.viewsetting[0].uri
						: '';
				if (data.viewsetting && data.viewsetting.length > 0) {
					const fetchContent = await this.settingsStorage.fetchSettingFile(
						data.viewsetting[0].uri,
					);
					if (fetchContent) {
						// Merge with default values to ensure all fields are present
						viewSetting = this.mergeWithDefault(
							viewSetting,
							JSON.parse(fetchContent),
						);
					}
				}
				this._viewSetting = viewSetting;
				// A key or model saved without a URL is OpenAI's, as shown before.
				this._viewSetting.aiProviderURL =
					this.normalizeBaseUrl(viewSetting.aiProviderURL || '') ||
					(viewSetting.aiProviderAPIKey ||
					viewSetting.aiProviderAPIKeyStored ||
					viewSetting.aiProviderModel
						? this.getDefaultAIProviderURL()
						: '');
				// An empty image URL means "Same as text provider", so keep it empty
				// rather than falling back to a default.
				this._viewSetting.aiImageProviderURL = this.normalizeBaseUrl(
					viewSetting.aiImageProviderURL || '',
				);
				this._aiConfigDirty = false;
				this.generateAISettingsUI(viewSetting, settingsContainer);
			}
		}

		// Autotext, Custom dictionaries and Document themes
		if (!isCODesktop) {
			this.insertConfigSections(data);
		}

		if (!isUserConfig && !isCODesktop && data && this.isAdmin()) {
			let stored: any = null;
			this._serverPrivateSettingFileUrl =
				data.serverprivateinfo && data.serverprivateinfo.length > 0
					? data.serverprivateinfo[0].uri
					: '';
			if (data.serverprivateinfo && data.serverprivateinfo.length > 0) {
				const content = await this.settingsStorage.fetchSettingFile(
					data.serverprivateinfo[0].uri,
				);
				if (content !== null) {
					stored = {};
					if (content.trim()) {
						try {
							stored = JSON.parse(content);
						} catch (error) {
							console.error('Could not read serverprivateinfo.json', error);
						}
					}
				}
			}
			if (stored === null && !this.serverPrivateInfoInitializationAttempted) {
				this.serverPrivateInfoInitializationAttempted = true;
				try {
					await this.settingsStorage.uploadSettings(
						this.PATH.serverPrivateSettingsUpload(),
						SettingIframe.serverPrivateSettingsFile(
							SettingIframe.getDefaultServerPrivateSettings(),
						),
					);
					return await this.fetchAndPopulateSharedConfigs();
				} catch (error) {
					console.error('Could not create serverprivateinfo.json', error);
				}
			}

			if (stored !== null) {
				this.generateServerPrivateSettingUI(
					{
						ESignatureBaseUrl: stored.ESignatureBaseUrl || '',
						ESignatureClientId: stored.ESignatureClientId || '',
						ESignatureSecret: stored.ESignatureSecret || '',
						ESignatureSecretStored: !!stored.ESignatureSecretStored,
					},
					settingsContainer,
				);
			}
		}

		// Document Signing and Zotero
		if (isUserConfig) {
			if (!data) {
				// On the Windows and macOS apps, signing uses the native
				// certificate store, so the Document Signing section is not
				// offered here.
				if (!usesNativeCertStore)
					this._docSigningSection = this.createEmptySection(
						this._docSigningSection,
						'doc-signing-section',
						_('Document Signing'),
					);
				this._zoteroSection = this.createEmptySection(
					this._zoteroSection,
					'zotero-section',
					'Zotero',
				);
			} else {
				if (!usesNativeCertStore)
					this.generateDocSigningUI(this._viewSetting, settingsContainer);
				this.generateZoteroUI(this._viewSetting, settingsContainer);
			}
		}

		this.setupLeftNavbar();

		if (data) {
			if (data.autotext)
				this.populateList('autotextList', data.autotext, '/autotext');
			if (data.wordbook)
				this.populateList('wordbookList', data.wordbook, '/wordbook');
			if (data.xcu) this.populateList('XcuList', data.xcu, '/xcu');
			if (data.themes) this.populateList('themesList', data.themes, '/themes');
			if (data.extensions)
				this.populateList('extensionsList', data.extensions, '/extensions');
			if (data.spif) this.populateList('spifList', data.spif, '/spif');
		}

		var navItem = document.querySelector<HTMLElement>(
			'#settings-nav .settings-nav-item',
		);
		if (navItem) navItem.focus();
	}

	private appendXcuDebugUploadControls(
		xcuContainer: HTMLElement,
		data: ConfigData,
	): void {
		if (!window.enableDebug) return;

		const list = this.createUnorderedList('XcuList');
		const fileInput = this.createFileInput('XcuFile', '.xcu');
		// TODO: replace btn with rich interface (toggles)
		const button = this.createButton('uploadXcuButton', _('Upload Xcu'));

		if (data.xcu && data.xcu.length > 0) {
			button.style.display = 'none';
		}

		button.addEventListener('click', () => fileInput.click());
		fileInput.addEventListener('change', () => {
			if (fileInput.files?.length) {
				this.uploadFile(this.PATH.XcuUpload(), fileInput.files[0]);
				fileInput.value = '';
			}
		});

		xcuContainer.appendChild(list);
		xcuContainer.appendChild(fileInput);
		xcuContainer.appendChild(button);
	}

	private setupLeftNavbar(): void {
		if (this.isAdmin()) return;
		if (!window.showLeftNav) return;

		// Prevent double scrollbars
		document.body.style.margin = '0';

		const content = this._allConfigSection;
		if (!content) return;

		const newNav = document.createElement('nav');
		newNav.id = 'settings-nav';

		if (this._sectionObserver) {
			this._sectionObserver.disconnect();
		}

		this._visibleSections.clear();

		const observerOptions = {
			root: content,
			rootMargin: '-30px 0px 0px 0px',
		};

		this._sectionObserver = new IntersectionObserver((entries) => {
			entries.forEach((entry) => {
				if (entry.isIntersecting) {
					this._visibleSections.add(entry.target);
				} else {
					this._visibleSections.delete(entry.target);
				}
			});

			let activeSection: Element | null = null;
			let minTop = Infinity;

			for (const section of Array.from(this._visibleSections)) {
				const rect = section.getBoundingClientRect();
				if (rect.top < minTop) {
					minTop = rect.top;
					activeSection = section;
				}
			}

			if (activeSection) {
				const id = activeSection.id;
				newNav.querySelectorAll('.settings-nav-item').forEach((link) => {
					if (link.getAttribute('href') === '#' + id) {
						link.classList.add('active');
					} else {
						link.classList.remove('active');
					}
				});
			}
		}, observerOptions);

		content.querySelectorAll('.section').forEach((section) => {
			this._sectionObserver?.observe(section);
			const header = section.querySelector('h3');
			if (header) {
				const link = document.createElement('a');
				link.textContent = header.textContent;
				link.classList.add('settings-nav-item');
				link.href = '#' + section.id;
				newNav.appendChild(link);
			}
		});

		const oldNav = document.getElementById('settings-nav');
		if (oldNav) {
			oldNav.replaceWith(newNav);
		} else {
			let wrapper = document.getElementById('settingIframe');
			wrapper!.insertBefore(newNav, content);
		}
	}

	private mergeWithDefault(defaults: any, overrides: any): any {
		const result: any = {};

		for (const key in defaults) {
			const value = defaults[key];
			let override = overrides?.[key];
			if (override === 'true') override = true;
			else if (override === 'false') override = false;
			if (
				typeof value === 'boolean' ||
				(typeof value === 'object' && value !== null && 'customType' in value)
			) {
				// Use override directly for booleans or objects with customType (set value)
				result[key] =
					typeof override === 'boolean' || typeof override === 'number'
						? typeof value === 'object'
							? { ...value, value: override }
							: override
						: value;
			} else if (typeof value === 'object' && value !== null) {
				result[key] = this.mergeWithDefault(value, override);
			} else {
				result[key] = override !== undefined ? override : value;
			}
		}

		return result;
	}

	private createInputField(
		key: string,
		label: string,
		value: string = '',
		data: any,
		skipHeading: boolean = false,
		isSmallHeading: boolean = false,
	): HTMLDivElement {
		const container = document.createElement('div');
		container.id = `${key}container`;
		container.classList.add('view-input-container');

		// Add heading unless skipped
		if (!skipHeading) {
			const heading = this.createHeading(label);
			if (isSmallHeading) {
				heading.classList.add('view-setting-small-label');
			}
			container.appendChild(heading);
		}

		const isSignatureField = [
			'signatureCert',
			'signatureKey',
			'signatureCa',
		].includes(key);

		const isSecretField = SECRET_SETTING_FIELDS.includes(key);
		const storedFlag = `${key}Stored`;
		const hasStoredSecret = isSecretField && !!(data as any)[storedFlag];

		// A stored secret is never sent down. Show a masked pattern so the field
		// reads as "a value is saved"; the field stays empty, and leaving it blank
		// keeps that value.
		const placeholder = hasStoredSecret
			? '********'
			: _('Enter {0}').replace('{0}', label);

		// Once the user edits a secret field, its value is authoritative: clear
		// the keep flag so the typed value (empty to clear, text to replace) is
		// what gets saved.
		const markEdited = () => {
			if (isSecretField) {
				(data as any)[storedFlag] = false;
			}
		};

		if (isSignatureField) {
			const textarea = this.createTextArea(
				key as string,
				placeholder,
				value,
				(textareaElement) => {
					(data as any)[key] = textareaElement.value;
					markEdited();
				},
			);
			container.appendChild(textarea);
		} else {
			const input = this.createTextInput(
				key as string,
				placeholder,
				value,
				(inputElement) => {
					(data as any)[key] = inputElement.value;
					markEdited();
				},
			);
			if (isSecretField) {
				input.type = 'password';
			}
			container.appendChild(
				SECRET_FIELDS_WITH_DELETE.includes(key)
					? this.createSecretInputRow(input, key, label, data)
					: input,
			);
		}

		return container;
	}
	// Input with delete button to reset.
	private createSecretInputRow(
		input: HTMLInputElement,
		key: string,
		label: string,
		data: any,
	): HTMLDivElement {
		const row = document.createElement('div');
		row.classList.add('secret-input-row');

		const storedFlag = `${key}Stored`;
		const syncField = () => this.syncSecretField(key, data, row);

		// The trash icon comes from the stylesheet, which picks the light or
		// dark version for the theme.
		const deleteButton = document.createElement('button');
		deleteButton.type = 'button';
		deleteButton.id = `${key}-delete`;
		deleteButton.classList.add(
			'button',
			'button--icon-only',
			'button--vue-tertiary',
			'secret-delete-button',
		);
		deleteButton.title = _('Delete {0}').replace('{0}', label);
		deleteButton.setAttribute('aria-label', deleteButton.title);
		deleteButton.addEventListener('click', () => {
			input.value = '';
			data[key] = '';
			data[storedFlag] = false;
			syncField();
			input.dispatchEvent(new Event('input', { bubbles: true }));
			input.focus();
		});

		input.addEventListener('input', syncField);

		row.appendChild(input);
		row.appendChild(deleteButton);
		syncField();
		return row;
	}

	// The one rule for a secret field and its delete button, for the field's
	// own input event and for the places that change it without the user
	// typing in it. A saved key shows as a filled, locked field: deleting it
	// is how to enter a new one. Nothing to clear, nothing to press.
	private syncSecretField(
		key: string,
		data: any,
		root: ParentNode = document,
	): void {
		const input = root.querySelector(`#${key}`) as HTMLInputElement | null;
		const button = root.querySelector(
			`#${key}-delete`,
		) as HTMLButtonElement | null;
		const stored = !!data[`${key}Stored`];
		if (input) {
			if (stored) input.value = STORED_SECRET_MASK;
			else if (input.disabled) input.value = data[key] || '';
			input.disabled = stored;
		}
		if (button) {
			button.disabled = !stored && !input?.value;
		}
	}

	private createSettingsActions(
		prefix: string,
		settingsName: string,
		filename: string,
		// Null leaves the Reset button out.
		getDefaultSettings: (() => any) | null,
		getCurrentSettings: () => any,
		uploadSettings: (settings: any) => Promise<void>,
	): HTMLDivElement {
		// In the dialog (left nav present) the modal's own Save/Cancel handles
		// saving, so skip these redundant per-section actions.
		if (window.showLeftNav) return document.createElement('div');

		const actionsContainer = document.createElement('div');
		actionsContainer.classList.add('xcu-editor-actions');

		if (getDefaultSettings) {
			const resetButton = this.createButtonWithText(
				`${prefix}-reset-button`,
				_('Reset'),
				_('Reset to default {0}').replace('{0}', settingsName),
				['button--vue-secondary', `${prefix}-reset-icon`],
				async (button) => {
					const confirmed = window.confirm(
						_('Are you sure you want to reset {0}?').replace(
							'{0}',
							settingsName,
						),
					);
					if (!confirmed) {
						return;
					}
					button.disabled = true;
					const defaultSettings = getDefaultSettings();
					await uploadSettings(defaultSettings);
					button.disabled = false;
				},
			);
			actionsContainer.appendChild(resetButton);
		}

		const saveButton = this.createButtonWithText(
			`${prefix}-save-button`,
			_('Save'),
			_('Save {0}').replace('{0}', settingsName),
			['button-primary'],
			async (button) => {
				button.disabled = true;
				const currentSettings = getCurrentSettings();
				console.log(
					`${settingsName} - Current settings being saved:`,
					currentSettings,
				);
				await uploadSettings(currentSettings);
				button.disabled = false;
			},
		);
		actionsContainer.appendChild(saveButton);

		return actionsContainer;
	}

	private getDefaultViewSettings(): ViewSettings {
		return {
			zoteroAPIKey: '',
			signatureCert: '',
			signatureKey: '',
			signatureCa: '',
			aiProviderURL: '',
			aiProviderAPIKey: '',
			aiProviderModel: '',
			aiImageProviderAPIKey: '',
			aiImageProviderURL: '',
			aiImageModel: '',
			aiImageSize: '',
			aiRequestTimeout: '',
			zoteroAPIKeyStored: false,
			signatureKeyStored: false,
			aiProviderAPIKeyStored: false,
			aiImageProviderAPIKeyStored: false,
		};
	}

	private normalizeBaseUrl(value: string): string {
		if (!value) return '';
		// The server appends "/v1/..." to this base URL, so reduce it to the bare
		// origin. A value that already ends in "/v1" (users often paste one) would
		// otherwise produce a doubled "/v1/v1/..." path. Trailing slashes go first,
		// then a single trailing "/v1", then any slash left behind.
		return value.replace(/\/+$/, '').replace(/\/v1$/i, '').replace(/\/+$/, '');
	}

	private getProviderById(id: string): AIProvider | undefined {
		return AI_PROVIDERS.find((provider) => provider.id === id);
	}

	private getProviderByUrl(url: string): AIProvider | undefined {
		const normalizedUrl = this.normalizeBaseUrl(url || '');
		return AI_PROVIDERS.find(
			(provider) =>
				!provider.isCustom &&
				this.normalizeBaseUrl(provider.baseUrl) === normalizedUrl,
		);
	}

	private resetAIProviders(data: ViewSettings, root: HTMLElement): void {
		const providerSelect = root.querySelector(
			'#aiProvider',
		) as HTMLSelectElement | null;
		if (providerSelect) {
			providerSelect.value = DEFAULT_PROVIDER;
			providerSelect.dispatchEvent(new Event('change'));
		}
		const sameAsText = root.querySelector(
			'#aiImageSameAsText-input',
		) as HTMLInputElement | null;
		if (sameAsText && !sameAsText.checked) sameAsText.click();
		data.aiImageModel = '';
		this.resetAIImageModelSelect('');

		this.showAIEmptyState(root, true);
		(
			root.querySelector('#ai-add-provider-button') as HTMLElement | null
		)?.focus();
	}

	private getProviderIdFromUrl(url: string): string {
		const provider = this.getProviderByUrl(url);
		return provider ? provider.id : 'custom';
	}

	private getSelectedProviderId(
		data: ViewSettings,
		root: ParentNode = document,
	): string {
		const providerSelect = root.querySelector(
			'#aiProvider',
		) as HTMLSelectElement | null;
		if (providerSelect?.value) {
			return providerSelect.value;
		}
		return data.aiProviderURL
			? this.getProviderIdFromUrl(data.aiProviderURL)
			: DEFAULT_PROVIDER;
	}

	private isCustomProviderSelected(
		root: ParentNode,
		data: ViewSettings,
	): boolean {
		return this.getSelectedProviderId(data, root) === 'custom';
	}

	private getDefaultAIProviderURL(): string {
		const provider = this.getProviderById('openai');
		return provider ? provider.baseUrl : '';
	}

	private getConfigType(): string {
		return this.isAdmin() ? 'systemconfig' : 'userconfig';
	}

	private isAdmin(): boolean {
		return window.iframeType === 'admin';
	}

	static showErrorModal(message: string, onClose?: () => void): void {
		const modal = document.createElement('div');
		modal.className = 'modal';

		const modalContent = document.createElement('div');
		modalContent.className = 'modal-content';

		const header = document.createElement('h2');
		header.textContent = _('Error');
		header.style.textAlign = 'center';
		modalContent.appendChild(header);

		const messageEl = document.createElement('p');
		messageEl.textContent = message;
		modalContent.appendChild(messageEl);

		const buttonContainer = document.createElement('div');
		buttonContainer.className = 'modal-button-container';

		const okButton = document.createElement('button');
		okButton.textContent = _('OK');
		okButton.classList.add('button', 'button--vue-secondary');
		okButton.addEventListener('click', () => {
			document.body.removeChild(modal);
			onClose?.();
		});

		buttonContainer.appendChild(okButton);
		modalContent.appendChild(buttonContainer);

		modal.appendChild(modalContent);
		document.body.appendChild(modal);
	}

	private settingConfigBasePath(): string {
		return '/settings/' + this.getConfigType();
	}

	private getFilename(uri: string, removeExtension = true): string {
		const url = new URL(uri, window.location.origin);
		let filename = url.searchParams.get('file_name');
		if (!filename) {
			// Remove query parameters from url
			uri = uri.split('?')[0];
			filename = uri.substring(uri.lastIndexOf('/') + 1);
		}

		if (removeExtension) {
			filename = filename.replace(/\.[^.]+$/, '');
		}
		return filename;
	}

	private toggleGridOptionsVisibility(checkbox: HTMLElement): void {
		const gridFieldset = checkbox.closest('.xcu-settings-fieldset');
		const childFieldsets = gridFieldset?.querySelectorAll(
			'.grid-options-fieldset',
		) as NodeListOf<HTMLElement>;
		childFieldsets?.forEach((fieldset) => {
			if (checkbox.classList.contains('checkbox-radio-switch--checked')) {
				fieldset.style.display = 'none';
			} else {
				fieldset.style.display = 'block';
			}
		});
	}
}

document.addEventListener('DOMContentLoaded', () => {
	const adminContainer = document.getElementById('allConfigSection');
	if (adminContainer) {
		initTranslationStr();
		window.settingIframe = new SettingIframe();
		window.settingIframe.init();
		const postHeight = () => {
			window.parent.postMessage(
				JSON.stringify({
					MessageId: 'Iframe_Height',
					SendTime: Date.now(),
					Values: {
						ContentHeight: document.documentElement.offsetHeight + 'px',
					},
				}),
				getIntegratorOrigin(),
			);
		};

		let timeout: any;
		const debouncePostHeight = () => {
			clearTimeout(timeout);
			timeout = setTimeout(postHeight, 100);
		};

		const mutationObserver = new MutationObserver(debouncePostHeight);
		mutationObserver.observe(document.body, {
			childList: true,
			subtree: true,
			attributes: true,
			characterData: true,
		});

		// init() builds the settings DOM before the observer is attached, so a
		// page that never mutates afterwards would otherwise never send its
		// first height and the parent would keep the iframe at its CSS default
		// height (short frame with an inner scrollbar). Post one height now,
		// and again once fonts/images have settled the final layout.
		postHeight();
		window.addEventListener('load', postHeight);
	}
});

(window as any)._ = _;
window.onload = onLoaded;
