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

interface Window {
	accessToken?: string;
	accessTokenTTL?: string;
	enableAccessibility?: boolean;
	enableDebug?: boolean;
	disableAISettings?: boolean;
	wopiSettingBaseUrl?: string;
	iframeType?: string;
	cssVars?: string;
	serviceRoot?: string;
	versionHash?: string;
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
}

interface AIProvider {
	id: string;
	name: string;
	baseUrl: string;
	isCustom?: boolean;
}

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
}

const initTranslationStr = () => {
	const element = document.getElementById('initial-variables');
	document.documentElement.lang =
		(element as HTMLInputElement).dataset.lang || 'en-US';

	String.defaultLocale = 'en-US';
	String.locale =
		document.documentElement.getAttribute('lang') || String.defaultLocale;
};

const onLoaded = () => {
	window.addEventListener('message', onMessage, false);
	window.parent.postMessage('{"MessageId":"settings-ready"}', window.origin);
};

const onMessage = (e) => {
	try {
		const data = JSON.parse(e.data);
		if (e.origin === window.origin && window.parent !== window.self) {
			if (data.MessageId === 'settings-ready')
				window.parent.postMessage(
					'{"MessageId":"settings-show"}',
					window.origin,
				);
			else if (data.MessageId === 'settings-save-all') {
				const settingIframe = (window as any).settingIframe as SettingIframe;
				if (settingIframe) {
					settingIframe.saveAll().then(() => {
						window.parent.postMessage(
							JSON.stringify({
								MessageId: 'settings-save-complete',
								viewSettings: settingIframe.getViewSettings(),
							}),
							window.origin,
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

const defaultBrowserSetting: Record<string, any> = {
	compactMode: {
		value: false,
		label: 'Compact layout',
		customType: 'compactToggle',
	},
	darkTheme: false,
	accessibilityState: false,
	lockAccessibilityOn: false,
	spreadsheet: {
		ShowStatusbar: false,
		A11yCheckDeck: false,
		NavigatorDeck: false,
		PropertyDeck: true,
	},
	text: {
		ShowRuler: false,
		ShowStatusbar: false,
		A11yCheckDeck: false,
		NavigatorDeck: false,
		PropertyDeck: true,
		StyleListDeck: false,
	},
	presentation: {
		ShowRuler: false,
		ShowStatusbar: false,
		A11yCheckDeck: false,
		NavigatorDeck: false,
		PropertyDeck: true,
		SdCustomAnimationDeck: false,
		// SdMasterPagesDeck: false,
		// SdSlideTransitionDeck: false,
	},
	drawing: {
		ShowRuler: false,
		ShowStatusbar: false,
		A11yCheckDeck: false,
		NavigatorDeck: false,
		PropertyDeck: true,
	},
};

abstract class SettingsStorage {
	abstract fetchSettingsConfig(): Promise<ConfigData>;
	abstract uploadSettings(filePath: string, file: File): Promise<void>;
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
				response.text(),
			);
			throw new Error(`Could not fetch shared config: ${response.statusText}`);
		}

		return await response.json();
	}

	async uploadSettings(filePath: string, file: File): Promise<void> {
		const formData = new FormData();
		formData.append('file', file);
		formData.append('filePath', filePath);
		if (window.wopiSettingBaseUrl) {
			formData.append('wopiSettingBaseUrl', window.wopiSettingBaseUrl);
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

const AI_ERROR_MESSAGES: Record<number, string> = {
	400: 'Invalid request',
	401: 'Invalid API key',
	403: 'API key lacks permissions',
	429: 'Rate limited - please wait a moment and retry',
	500: 'API server error - try again later',
	503: 'Service temporarily unavailable',
};

class SettingIframe {
	private settingsStorage: SettingsStorage;
	private wordbook;
	private xcuEditor;
	private _viewSetting!: ViewSettings;
	private xcuInitializationAttempted = false;
	private _aiModelFetchTimeout: number | null = null;
	private _aiModelFetchAbort: AbortController | null = null;
	private _aiModelFetchSeq = 0;
	private _lastCustomAIProviderURL = '';
	private _lastCustomAIImageProviderURL = '';
	private _aiImageModelFetchTimeout: number | null = null;
	private _aiImageModelFetchAbort: AbortController | null = null;
	private _aiImageModelFetchSeq = 0;
	private _viewSettingLabels = {
		zoteroAPIKey: 'Zotero',
		signatureCert: _('Signature Certificate'),
		signatureKey: _('Signature Key'),
		signatureCa: _('Signature CA'),
		aiProvider: _('Provider'),
		aiProviderAPIKey: _('API Key'),
		aiProviderModel: _('Model'),
		aiProviderURL: _('Base URL'),
		aiImageProvider: _('Provider'),
		aiImageProviderAPIKey: _('API Key'),
		aiImageProviderURL: _('Base URL'),
		aiImageModel: _('Model'),
		aiImageSize: _('Image Size'),
		aiRequestTimeout: _('Request Timeout (seconds)'),
	};
	private readonly settingLabels: Record<string, string> = {
		lockAccessibilityOn: _('In-document Screen Reader'),
		darkTheme: _('Dark Mode'),
		compactMode: _('Compact layout'),
		ShowStatusbar: _('Show status bar'),
		ShowRuler: _('Show Ruler'),
		A11yCheckDeck: _('Accessibility Checker'),
		NavigatorDeck: _('Navigator'),
		PropertyDeck: _('Show Sidebar'),
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
		SizeToGrid: _('Size to grid'),
		Synchronize: _('Synchronize axes'),
		SnapGrid: _('Snap grid'),
		EmptyPages: _('Empty Pages'),
		ForceBreaks: _('Force Breaks'),
		AllSheets: _('All Sheets'),
		Size: _('Size to grid'),
		Content: _('Content'),
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
		Table: _('Tables'),
		Control: _('Controls'),
		Background: _('Background'),
		PrintBlack: _('Print Black'),
		PrintHiddenText: _('Hidden text'),
		PrintPlaceholders: _('Placeholders'),
		LeftPage: _('Left pages'),
		RightPage: _('Right pages'),
		Brochure: _('Brochure'),
		BrochureRightToLeft: _('Brochure Right to Left'),
		GraphicObject: _('Images and Objects'),
		// Add more as needed
	};

	// SVG templates for icons that are small and always present (no async load needed)
	private readonly SVG_ICONS = {
		download: `<svg fill="currentColor" width="20" height="20" viewBox="0 0 24 24"><path d="M5,20H19V18H5M19,9H15V3H9V9H5L12,16L19,9Z"></path></svg>`,
		delete: `<svg fill="currentColor" width="20" height="20" viewBox="0 0 24 24"><path d="M19,4H15.5L14.5,3H9.5L8.5,4H5V6H19M6,19A2,2 0 0,0 8,21H16A2,2 0 0,0 18,19V7H6V19Z"></path></svg>`,
		edit: `<svg fill="currentColor" width="20" height="20" viewBox="0 0 24 24"><path d="M3 17.25V21h3.75l11-11.03-3.75-3.75L3 17.25zM20.71 7.04a1 1 0 0 0 0-1.41l-2.34-2.34a1 1 0 0 0-1.41 0l-1.83 1.83 3.75 3.75 1.83-1.83z"></path></svg>`,
		reset: `<svg fill="currentColor" width="24" height="24" viewBox="0 0 24 24"><path d="M12 4V1L8 5l4 4V6c3.31 0 6 2.69 6 6 0 .34-.03.67-.08 1h2.02c.05-.33.06-.66.06-1 0-4.42-3.58-8-8-8zm-6 7c0-.34.03-.67.08-1H4.06c-.05.33-.06.66-.06 1 0 4.42 3.58 8 8 8v3l4-4-4-4v3c-3.31 0-6-2.69-6-6z"></path></svg>`,
		checkboxMarked: `<svg fill="currentColor" width="24" height="24" viewBox="0 0 24 24"><path d="M10,17L5,12L6.41,10.58L10,14.17L17.59,6.58L19,8M19,3H5C3.89,3 3,3.89 3,5V19A2,2 0 0,0 5,21H19A2,2 0 0,0 21,19V5C21,3.89 20.1,3 19,3Z"></path></svg>`,
		checkboxBlankOutline: `<svg fill="currentColor" width="24" height="24" viewBox="0 0 24 24"><path d="M19,3H5C3.89,3 3,3.89 3,5V19A2,2 0 0,0 5,21H19A2,2 0 0,0 21,19V5C21,3.89 20.1,3 19,3M19,5V19H5V5H19Z"></path></svg>`,
	};
	private _allConfigSection: HTMLElement | null;
	private _sectionObserver: IntersectionObserver | null = null;
	private _visibleSections: Set<Element> = new Set();

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
	};
	private browserSettingOptions: Record<string, any> = {};

	getViewSettings(): ViewSettings {
		return this._viewSetting;
	}

	public async saveAll(): Promise<void> {
		const saves: Promise<void>[] = [];

		// Browser settings
		const browserSettingEl = document.getElementById('browser-setting');
		if (browserSettingEl) {
			saves.push(
				(async () => {
					this.collectBrowserSettingsFromUI(browserSettingEl);
					const file = new File(
						[JSON.stringify(this.browserSettingOptions)],
						'browsersetting.json',
						{ type: 'application/json', lastModified: Date.now() },
					);
					await this.uploadFile(this.PATH.browserSettingsUpload(), file);
					if ((window as any).parent?.mode?.isCODesktop()) {
						(window.parent as any).postMobileMessage('SYNCSETTINGS');
					}
				})(),
			);
		}

		// Document settings (XCU)
		if (this.xcuEditor) {
			saves.push(this.xcuEditor.generateXcuAndUpload());
		}

		// View settings
		saves.push(
			this.uploadViewSettingFile(
				'viewsetting.json',
				JSON.stringify(this._viewSetting),
			),
		);

		await Promise.all(saves);
	}

	init(): void {
		this._allConfigSection = document.getElementById('allConfigSection');
		this.initWindowVariables();
		if (isCODesktop) {
			this.settingsStorage = new DesktopSettingsStorage();
		} else {
			this.settingsStorage = new OnlineSettingsStorage();
		}
		if (!isCODesktop) {
			this.insertConfigSections();
			this.setupLeftNavbar();
		}
		this.fetchAndPopulateSharedConfigs();
		this.wordbook = (window as any).WordBook;
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
		await this.uploadFile(this.PATH.viewSettingsUpload(), file);
	}

	private initWindowVariables(): void {
		const element = document.getElementById('initial-variables');
		if (!element) return;

		window.accessToken = element.dataset.accessToken;
		if (!window.accessToken) {
			throw new Error('Access token is missing in initial variables.');
		}

		window.accessTokenTTL = element.dataset.accessTokenTtl;
		window.enableDebug = element.dataset.enableDebug === 'true';
		window.enableAccessibility = element.dataset.enableAccessibility === 'true';
		window.disableAISettings = element.dataset.disableAiSettings === 'true';
		window.wopiSettingBaseUrl = element.dataset.wopiSettingBaseUrl ?? '';
		window.iframeType = element.dataset.iframeType || 'user';
		window.cssVars = element.dataset.cssVars || '';
		if (window.cssVars) {
			window.cssVars = atob(window.cssVars);
			const sheet = new CSSStyleSheet();
			if (typeof (sheet as any).replace === 'function') {
				(sheet as any).replace(window.cssVars);
				(document as any).adoptedStyleSheets.push(sheet);
			}
		}
		window.serviceRoot = element.dataset.serviceRoot;
		window.versionHash = element.dataset.versionHash;
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

	private insertConfigSections(): void {
		if (!this._allConfigSection) return;

		const configSections: SectionConfig[] = [
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
			},
			{
				id: 'xcu',
				sectionTitle: _('Document settings'),
				sectionDesc: _('Adjust how office documents behave.'),
				listId: 'XcuList',
				inputId: 'XcuFile',
				buttonId: 'uploadXcuButton',
				fileAccept: '.xcu',
				// TODO: replace btn with rich interface (toggles)
				buttonText: _('Upload Xcu'),
				uploadPath: this.PATH.XcuUpload(),
				debugOnly: true,
			},
		];

		configSections.forEach((cfg) => {
			if (cfg.enabledFor && cfg.enabledFor !== this.getConfigType()) {
				return;
			}

			if (cfg.debugOnly && !window.enableDebug) {
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

			this._allConfigSection!.appendChild(sectionEl);
		});
	}

	private async fetchAndPopulateSharedConfigs(): Promise<void> {
		try {
			const data = await this.settingsStorage.fetchSettingsConfig();
			await this.populateSharedConfigUI(data);
			console.debug('Shared config data: ', data);
		} catch (error: unknown) {
			SettingIframe.showErrorModal(
				_('Something went wrong. Please try to refresh the page.'),
			);
			console.error('Error fetching shared config:', error);
		}
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
			// Include:
			// - plain booleans
			// - objects that have a customType (like compactToggle)
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

		const oldEditor = sharedConfigsContainer.querySelector('#browser-setting');
		if (oldEditor && oldEditor.parentNode === sharedConfigsContainer) {
			sharedConfigsContainer.replaceChild(editorContainer, oldEditor);
		} else {
			sharedConfigsContainer.appendChild(editorContainer);
		}

		setTimeout(() => {
			const defaultTab = navContainer.querySelector(
				'#bs-tab-spreadsheet',
			) as HTMLElement;
			if (defaultTab) {
				defaultTab.click();
			}
		}, 0);
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
				contentsContainer.innerHTML = '';
				if (this.browserSettingOptions && this.browserSettingOptions[tab.id]) {
					const renderedTree = this.renderSettingsOption(
						this.browserSettingOptions[tab.id],
						tab.id,
					);
					renderedTree.classList.add('browser-settings-grid');
					contentsContainer.appendChild(renderedTree);
				} else {
					contentsContainer.textContent = _(
						`No settings available for ${tab.label}`,
					);
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
				this.browserSettingOptions = JSON.parse(
					JSON.stringify(defaultBrowserSetting),
				);
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
				this.collectBrowserSettingsFromUI(
					sharedConfigsContainer.querySelector('#browser-setting')!,
				);

				const file = new File(
					[JSON.stringify(this.browserSettingOptions)],
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
			buttonEl.classList.add('button--text-only');
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
		legend.textContent = this.settingLabels[key] || key;
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
	): HTMLSpanElement {
		const checkboxWrapper = document.createElement('span');
		checkboxWrapper.className = `checkbox-radio-switch checkbox-radio-switch-checkbox ${isChecked ? '' : 'checkbox-radio-switch--checked'} checkbox-wrapper`;
		id = id.replace(/\s/g, '');
		checkboxWrapper.id = id + '-container';

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
		const labelText = this.settingLabels[key] || key;
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
		);
	}

	private collectBrowserSettingsFromUI(
		browserSettingSection: HTMLElement,
	): void {
		const inputs = browserSettingSection.querySelectorAll<HTMLInputElement>(
			'input.checkbox-radio-switch-input',
		);

		inputs.forEach((input) => {
			// Expected ID: section-setting-input (e.g., "writer-ShowSidebar-input")
			const parts = input.id.split('-');
			if (parts.length !== 3 || parts[2] !== 'input') return;

			const [sectionRaw, settingKey] = parts;
			const value = input.checked;

			if (sectionRaw === 'common') {
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

	private async uploadFile(filePath: string, file: File): Promise<void> {
		try {
			await this.settingsStorage.uploadSettings(filePath, file);
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

		listEl.innerHTML = '';

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
				(button) => window.open(item.uri, '_blank'),
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

	private generateViewSettingUI(data: ViewSettings) {
		this._viewSetting = data;
		this._viewSetting.aiProviderURL =
			this.normalizeBaseUrl(data.aiProviderURL || '') ||
			this.getDefaultAIProviderURL();
		const settingsContainer = this._allConfigSection;
		if (!settingsContainer) {
			return;
		}

		this.generateAISettingsUI(data, settingsContainer);
		this.generateZoteroUI(data, settingsContainer);
		this.generateDocSigningUI(data, settingsContainer);
	}

	private generateZoteroUI(data: ViewSettings, settingsContainer: HTMLElement) {
		const oldZoteroContainer = document.getElementById('zotero-section');

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

		const zoteroAccountLink = document.createElement('a');
		zoteroAccountLink.href = 'https://www.zotero.org/settings/keys';
		zoteroAccountLink.target = '_blank';
		zoteroAccountLink.textContent = _('Zotero account API settings');

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
				'Zotero Settings',
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
		if (oldZoteroContainer) {
			oldZoteroContainer.replaceWith(zoteroContainer);
		} else {
			settingsContainer.appendChild(zoteroContainer);
		}
	}

	private generateDocSigningUI(
		data: ViewSettings,
		settingsContainer: HTMLElement,
	) {
		const oldDocSigningContainer = document.getElementById(
			'doc-signing-section',
		);

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
				'Document Signing Settings',
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
		if (oldDocSigningContainer) {
			oldDocSigningContainer.replaceWith(docSigningContainer);
		} else {
			settingsContainer.appendChild(docSigningContainer);
		}
	}

	private generateAISettingsUI(
		data: ViewSettings,
		settingsContainer: HTMLElement,
	) {
		if (window.disableAISettings) {
			return;
		}

		const oldAIContainer = document.getElementById('ai-section');

		const aiContainer = document.createElement('div');
		aiContainer.id = 'ai-section';
		aiContainer.classList.add('section');

		aiContainer.appendChild(this.createHeading(_('AI Assistant')));
		const aiDesc = document.createElement('p');
		aiDesc.className = 'view-setting-description';
		aiDesc.textContent = _(
			'Configure AI provider credentials and model. Models are fetched automatically when credentials change.',
		);
		aiContainer.appendChild(aiDesc);

		const aiDivContainer = document.createElement('div');
		aiDivContainer.id = 'ai-editor';
		aiContainer.appendChild(aiDivContainer);

		aiDivContainer.appendChild(this.createAISettingsBlock(data));

		aiContainer.appendChild(
			this.createSettingsActions(
				'ai',
				'AI Assistant',
				'viewsetting.json',
				() => {
					const defaultSettings = this.getDefaultViewSettings();
					return {
						...this._viewSetting,
						aiProviderURL: defaultSettings.aiProviderURL,
						aiProviderAPIKey: defaultSettings.aiProviderAPIKey,
						aiProviderModel: defaultSettings.aiProviderModel,
						aiImageProviderURL: defaultSettings.aiImageProviderURL,
						aiImageProviderAPIKey: defaultSettings.aiImageProviderAPIKey,
						aiImageModel: defaultSettings.aiImageModel,
						aiImageSize: defaultSettings.aiImageSize,
						aiRequestTimeout: defaultSettings.aiRequestTimeout,
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
		if (oldAIContainer) {
			oldAIContainer.replaceWith(aiContainer);
		} else {
			settingsContainer.appendChild(aiContainer);
		}
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

		const timeoutBox = this.createViewSettingsTextBox(
			'aiRequestTimeout',
			data,
			false,
			true,
		);
		container.appendChild(timeoutBox);
		const timeoutInput = timeoutBox.querySelector(
			'#aiRequestTimeout',
		) as HTMLInputElement | null;
		if (timeoutInput) {
			timeoutInput.placeholder = '120';
			timeoutInput.type = 'number';
			timeoutInput.min = '10';
		}

		this.attachAISettingsAutoFetch(data, container);
		this.attachAIImageSettingsAutoFetch(data, container);

		if (data.aiProviderAPIKey) {
			this.scheduleAIModelFetch(data);
		}
		this.scheduleAIImageModelFetch(data);

		return container;
	}

	private createTextAIGroup(data: ViewSettings): HTMLFieldSetElement {
		const group = document.createElement('fieldset');
		group.classList.add('ai-settings-group');
		const legend = document.createElement('legend');
		legend.textContent = _('Text Generation');
		group.appendChild(legend);

		const providerOptions = AI_PROVIDERS.map((provider) => ({
			value: provider.id,
			label: provider.name,
		}));

		const providerField = document.createElement('div');
		providerField.id = 'aiProvidercontainer';
		providerField.classList.add('view-input-container');

		const providerHeading = this.createHeading(
			this._viewSettingLabels.aiProvider,
		);
		providerHeading.classList.add('view-setting-small-label');
		providerField.appendChild(providerHeading);

		const providerSelect = this.createSelectInput(
			'aiProvider',
			providerOptions,
			this.getProviderIdFromUrl(data.aiProviderURL),
			(selectEl) => {
				const provider = this.getProviderById(selectEl.value);
				if (provider && !provider.isCustom) {
					data.aiProviderURL = provider.baseUrl;
				}
			},
		);
		providerField.appendChild(providerSelect);
		group.appendChild(providerField);

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
			customUrlInput.placeholder = _('e.g.') + ' http://localhost:11434/v1';
		}

		group.appendChild(
			this.createViewSettingsTextBox('aiProviderAPIKey', data, false, true),
		);

		const modelField = document.createElement('div');
		modelField.id = 'aiModelcontainer';
		modelField.classList.add('view-input-container');

		const modelHeading = this.createHeading(
			this._viewSettingLabels.aiProviderModel,
		);
		modelHeading.classList.add('view-setting-small-label');
		modelField.appendChild(modelHeading);

		const modelSelect = this.createSelectInput(
			'aiProviderModel',
			[{ value: '', label: _('Fetch models to select') }],
			data.aiProviderModel || '',
			(selectEl) => {
				data.aiProviderModel = selectEl.value;
			},
		);
		modelSelect.disabled = true;
		modelField.appendChild(modelSelect);
		group.appendChild(modelField);

		const status = document.createElement('div');
		status.id = 'ai-model-status';
		status.className = 'view-setting-description';
		status.style.display = 'none';
		group.appendChild(status);

		if (this.getProviderIdFromUrl(data.aiProviderURL) === 'custom') {
			this._lastCustomAIProviderURL = data.aiProviderURL;
		}

		this.syncAISettingsVisibility(data, group);

		return group;
	}

	private createImageAIGroup(data: ViewSettings): HTMLFieldSetElement {
		const group = document.createElement('fieldset');
		group.classList.add('ai-settings-group');
		const legend = document.createElement('legend');
		legend.textContent = _('Image Generation');
		group.appendChild(legend);

		// Provider dropdown with "Same as Text AI" option
		const imageProviderOptions = [
			{ value: '', label: _('Same as Text AI') },
			...AI_PROVIDERS.map((provider) => ({
				value: provider.id,
				label: provider.name,
			})),
		];

		const providerField = document.createElement('div');
		providerField.id = 'aiImageProvidercontainer';
		providerField.classList.add('view-input-container');

		const providerHeading = this.createHeading(
			this._viewSettingLabels.aiImageProvider,
		);
		providerHeading.classList.add('view-setting-small-label');
		providerField.appendChild(providerHeading);

		const selectedImageProvider = data.aiImageProviderURL
			? this.getProviderIdFromUrl(data.aiImageProviderURL)
			: '';

		const providerSelect = this.createSelectInput(
			'aiImageProvider',
			imageProviderOptions,
			selectedImageProvider,
			(selectEl) => {
				if (selectEl.value === '') {
					data.aiImageProviderURL = '';
				} else {
					const provider = this.getProviderById(selectEl.value);
					if (provider && !provider.isCustom) {
						data.aiImageProviderURL = provider.baseUrl;
					}
				}
			},
		);
		providerField.appendChild(providerSelect);
		group.appendChild(providerField);

		group.appendChild(
			this.createViewSettingsTextBox('aiImageProviderURL', data, false, true),
		);
		const imageUrlContainer = group.querySelector(
			'#aiImageProviderURLcontainer',
		) as HTMLElement | null;
		if (imageUrlContainer) {
			imageUrlContainer.style.display =
				selectedImageProvider === 'custom' ? 'block' : 'none';
		}
		const imageUrlInput = group.querySelector(
			'#aiImageProviderURL',
		) as HTMLInputElement | null;
		if (imageUrlInput) {
			imageUrlInput.placeholder = _('e.g.') + ' http://localhost:11434/v1';
		}

		group.appendChild(
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
			imageApiKeyInput.placeholder = _('Leave empty to use Text AI key');
		}

		const modelField = document.createElement('div');
		modelField.id = 'aiImageModelcontainer';
		modelField.classList.add('view-input-container');

		const modelHeading = this.createHeading(
			this._viewSettingLabels.aiImageModel,
		);
		modelHeading.classList.add('view-setting-small-label');
		modelField.appendChild(modelHeading);

		const modelSelect = this.createSelectInput(
			'aiImageModel',
			[{ value: '', label: _('Fetch models to select') }],
			data.aiImageModel || '',
			(selectEl) => {
				data.aiImageModel = selectEl.value;
			},
		);
		modelSelect.disabled = true;
		modelField.appendChild(modelSelect);
		group.appendChild(modelField);

		const status = document.createElement('div');
		status.id = 'ai-image-model-status';
		status.className = 'view-setting-description';
		status.style.display = 'none';
		group.appendChild(status);

		group.appendChild(
			this.createViewSettingsTextBox('aiImageSize', data, false, true),
		);
		const imageSizeInput = group.querySelector(
			'#aiImageSize',
		) as HTMLInputElement | null;
		if (imageSizeInput) {
			imageSizeInput.placeholder = '1024x1024';
			imageSizeInput.addEventListener('input', () => {
				const val = imageSizeInput.value.trim();
				if (val === '' || /^\d+x\d+$/.test(val)) {
					const parts = val ? val.split('x') : [];
					const valid =
						val === '' || (Number(parts[0]) > 0 && Number(parts[1]) > 0);
					imageSizeInput.style.borderColor = valid ? '' : 'red';
					if (valid) {
						data.aiImageSize = val;
					}
				} else {
					imageSizeInput.style.borderColor = 'red';
				}
			});
		}

		if (
			data.aiImageProviderURL &&
			this.getProviderIdFromUrl(data.aiImageProviderURL) === 'custom'
		) {
			this._lastCustomAIImageProviderURL = data.aiImageProviderURL;
		}

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
			this.scheduleAIModelFetch(data);
			// Re-fetch image models too when image inherits chat credentials
			this.scheduleAIImageModelFetch(data);
		};

		providerInput?.addEventListener('change', () => {
			const selectedProvider = this.getProviderById(providerInput.value);
			if (selectedProvider && !selectedProvider.isCustom) {
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
			queueFetch();
		});

		customUrlInput?.addEventListener('input', () => {
			if (this.isCustomProviderSelected(root, data)) {
				data.aiProviderURL = customUrlInput.value;
				this._lastCustomAIProviderURL = customUrlInput.value;
				queueFetch();
			}
		});

		modelSelect?.addEventListener('change', () => {
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
			if (providerInput.value === '') {
				// "Same as Text AI"
				data.aiImageProviderURL = '';
			} else {
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
			}
			this.syncAIImageSettingsVisibility(data, root);
			queueFetch();
		});

		apiKeyInput?.addEventListener('input', () => {
			data.aiImageProviderAPIKey = apiKeyInput.value;
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

		modelSelect?.addEventListener('change', () => {
			data.aiImageModel = modelSelect.value;
		});
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
		const effectiveKey =
			data.aiImageProviderAPIKey || data.aiProviderAPIKey || '';
		const effectiveProviderId = this.getProviderIdFromUrl(effectiveUrl);
		const provider = this.getProviderById(effectiveProviderId);

		if (!effectiveKey || !effectiveUrl) {
			this.setAIImageStatus('', false, true);
			this.resetAIImageModelSelect();
			return;
		}

		this._aiImageModelFetchSeq += 1;
		const seq = this._aiImageModelFetchSeq;

		this._aiImageModelFetchAbort?.abort();
		this._aiImageModelFetchAbort = new AbortController();

		this.setAIImageStatus(_('Fetching models...'), false);

		try {
			const formData = new FormData();
			formData.append('provider', provider ? provider.id : effectiveProviderId);
			formData.append('apiKey', effectiveKey);
			formData.append('baseUrl', effectiveUrl);

			const response = await fetch(this.getAPIEndpoints().fetchModels, {
				method: 'POST',
				body: formData,
				signal: this._aiImageModelFetchAbort.signal,
			});

			if (!response.ok) {
				const errorCode = response.status;
				const errorText = await response.text();
				const fallbackMsg =
					AI_ERROR_MESSAGES[errorCode] ||
					_(`API error (${errorCode}): ${response.statusText}`);
				const errorMsg = errorText
					? `${fallbackMsg}. ${errorText}`
					: fallbackMsg;
				throw new Error(errorMsg);
			}

			const json = await response.json();
			const models: Array<{ id: string }> = json.data || [];
			if (!Array.isArray(models) || models.length === 0) {
				this.setAIImageStatus(_('No models found'), true);
				return;
			}

			if (seq !== this._aiImageModelFetchSeq) {
				return;
			}

			const modelIds = models.map((m) => m.id).filter(Boolean);
			if (modelIds.length === 0) {
				this.setAIImageStatus(_('No models found'), true);
				return;
			}

			const selectedModel = modelIds.includes(data.aiImageModel)
				? data.aiImageModel
				: '';
			data.aiImageModel = selectedModel;
			this.updateAIImageModelSelect(modelIds, selectedModel);

			this.setAIImageStatus(_('Models fetched successfully'), false);
		} catch (error) {
			if ((error as any)?.name === 'AbortError') {
				return;
			}
			const message =
				error instanceof Error ? error.message : _('Failed to fetch models');
			this.setAIImageStatus(message, true);
			this.resetAIImageModelSelect();
		}
	}

	private async fetchAIModels(data: ViewSettings): Promise<void> {
		const providerId = this.getSelectedProviderId(data);
		const provider = this.getProviderById(providerId);
		if (!provider) {
			this.setAIStatus(_('Invalid provider configuration'), true);
			return;
		}

		const isCustom = provider.isCustom ?? false;
		const baseUrl = isCustom
			? this.normalizeBaseUrl(data.aiProviderURL || '')
			: provider.baseUrl;
		const apiKey = data.aiProviderAPIKey || '';

		if (!apiKey || (isCustom && !baseUrl)) {
			this.setAIStatus('', false, true);
			this.resetAIModelSelect();
			return;
		}

		this._aiModelFetchSeq += 1;
		const seq = this._aiModelFetchSeq;

		this._aiModelFetchAbort?.abort();
		this._aiModelFetchAbort = new AbortController();

		this.setAIStatus(_('Fetching models...'), false);

		try {
			const formData = new FormData();
			formData.append('provider', provider.id);
			formData.append('apiKey', apiKey);
			formData.append('baseUrl', baseUrl);

			const response = await fetch(this.getAPIEndpoints().fetchModels, {
				method: 'POST',
				body: formData,
				signal: this._aiModelFetchAbort.signal,
			});

			if (!response.ok) {
				const errorCode = response.status;
				const errorText = await response.text();
				const fallbackMsg =
					AI_ERROR_MESSAGES[errorCode] ||
					_(`API error (${errorCode}): ${response.statusText}`);
				const errorMsg = errorText
					? `${fallbackMsg}. ${errorText}`
					: fallbackMsg;
				throw new Error(errorMsg);
			}

			const json = await response.json();
			const models: Array<{ id: string }> = json.data || [];
			if (!Array.isArray(models) || models.length === 0) {
				this.setAIStatus(_('No models found'), true);
				return;
			}

			if (seq !== this._aiModelFetchSeq) {
				return;
			}

			const modelIds = models.map((m) => m.id).filter(Boolean);
			if (modelIds.length === 0) {
				this.setAIStatus(_('No models found'), true);
				return;
			}

			const selectedModel = modelIds.includes(data.aiProviderModel)
				? data.aiProviderModel
				: modelIds[0];
			data.aiProviderModel = selectedModel;
			this.updateAIModelSelect(modelIds, selectedModel);

			this.setAIStatus(_('Models fetched successfully'), false);
		} catch (error) {
			if ((error as any)?.name === 'AbortError') {
				return;
			}
			const message =
				error instanceof Error ? error.message : _('Failed to fetch models');
			this.setAIStatus(message, true);
			this.resetAIModelSelect();
		}
	}

	private updateAIModelSelect(modelIds: string[], selectedModel: string): void {
		const modelSelect = document.getElementById(
			'aiProviderModel',
		) as HTMLSelectElement | null;
		if (!modelSelect) {
			return;
		}
		modelSelect.innerHTML = '';
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
		select.innerHTML = '';
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

	private resetAIImageModelSelect(): void {
		const select = document.getElementById(
			'aiImageModel',
		) as HTMLSelectElement | null;
		if (!select) return;
		select.innerHTML = '';
		const option = document.createElement('option');
		option.value = '';
		option.textContent = _('Fetch models to select');
		select.appendChild(option);
		select.value = '';
		select.disabled = true;
	}

	private resetAIModelSelect(): void {
		const modelSelect = document.getElementById(
			'aiProviderModel',
		) as HTMLSelectElement | null;
		if (!modelSelect) {
			return;
		}
		modelSelect.innerHTML = '';
		const option = document.createElement('option');
		option.value = '';
		option.textContent = _('Fetch models to select');
		modelSelect.appendChild(option);
		modelSelect.value = '';
		modelSelect.disabled = true;
	}

	private setAIStatus(
		message: string,
		isError: boolean,
		hide: boolean = false,
	): void {
		const status = document.getElementById('ai-model-status');
		if (!status) {
			return;
		}
		if (hide) {
			status.textContent = '';
			status.style.display = 'none';
			status.classList.remove('ui-state-error-text');
			return;
		}
		status.textContent = message;
		status.style.display = message ? 'block' : 'none';
		status.classList.toggle('ui-state-error-text', isError);
	}

	private setAIImageStatus(
		message: string,
		isError: boolean,
		hide: boolean = false,
	): void {
		const status = document.getElementById('ai-image-model-status');
		if (!status) {
			return;
		}
		if (hide) {
			status.textContent = '';
			status.style.display = 'none';
			status.classList.remove('ui-state-error-text');
			return;
		}
		status.textContent = message;
		status.style.display = message ? 'block' : 'none';
		status.classList.toggle('ui-state-error-text', isError);
	}

	private async populateSharedConfigUI(data: ConfigData): Promise<void> {
		const browserSettingButton = document.getElementById(
			'uploadBrowserSettingsButton',
		) as HTMLButtonElement | null;

		if (browserSettingButton) {
			if (data.browsersetting && data.browsersetting.length > 0) {
				browserSettingButton.style.display = 'none';
			} else {
				browserSettingButton.style.removeProperty('display');
			}
		}

		const xcuSettingButton = document.getElementById(
			'uploadXcuButton',
		) as HTMLButtonElement | null;

		if (xcuSettingButton) {
			if (data.xcu && data.xcu.length > 0) {
				xcuSettingButton.style.display = 'none';
			} else {
				xcuSettingButton.style.removeProperty('display');
			}
		}

		if (data.kind === 'user') {
			if (data.viewsetting && data.viewsetting.length > 0) {
				const fetchContent = await this.settingsStorage.fetchSettingFile(
					data.viewsetting[0].uri,
				);
				if (fetchContent) {
					const loadedSettings = JSON.parse(fetchContent);
					// Merge with default values to ensure all fields are present
					const defaultViewSetting = this.getDefaultViewSettings();
					const mergedSettings = this.mergeWithDefault(
						defaultViewSetting,
						loadedSettings,
					);
					this.generateViewSettingUI(mergedSettings);
				} else {
					const defaultViewSetting = this.getDefaultViewSettings();
					this.generateViewSettingUI(defaultViewSetting);
				}
			} else {
				const defaultViewSetting = this.getDefaultViewSettings();
				this.generateViewSettingUI(defaultViewSetting);
			}

			// browser settings
			if (data.browsersetting && data.browsersetting.length > 0) {
				const browserSettingContent =
					await this.settingsStorage.fetchSettingFile(
						data.browsersetting[0].uri,
					);
				this.browserSettingOptions = browserSettingContent
					? this.mergeWithDefault(
							defaultBrowserSetting,
							JSON.parse(browserSettingContent),
						)
					: defaultBrowserSetting;
			} else {
				this.browserSettingOptions = defaultBrowserSetting;
			}
			this.createBrowserSettingForm(this._allConfigSection!);
		}

		const settingsContainer = this._allConfigSection;
		if (!settingsContainer) return;
		if (!isCODesktop) {
			if (data.xcu && data.xcu.length > 0) {
				const xcuFileContent = await this.settingsStorage.fetchSettingFile(
					data.xcu[0].uri,
				);
				this.xcuEditor = new (window as any).Xcu(
					this.getFilename(data.xcu[0].uri, false),
					xcuFileContent,
				);

				const existingXcuSection = document.getElementById('xcu-section');
				if (existingXcuSection) {
					existingXcuSection.remove();
				}

				const xcuContainer = document.createElement('div');
				xcuContainer.id = 'xcu-section';
				xcuContainer.classList.add('section');
				settingsContainer.appendChild(
					this.xcuEditor.createXcuEditorUI(xcuContainer),
				);
			} else {
				// If user doesn't have any xcu file, we generate with default settings...
				try {
					if (!this.xcuInitializationAttempted) {
						this.xcuInitializationAttempted = true;
						this.xcuEditor = new (window as any).Xcu('documentView.xcu', null);
						await this.xcuEditor.generateXcuAndUpload();
						return await this.fetchAndPopulateSharedConfigs();
					} else {
						document.getElementById('xcu-section')?.remove();
						console.warn('XCU file not found and automatic creation failed.');
					}
				} catch (error) {
					console.error(
						'Something went wrong while generating or uploading xcu file:',
						error,
					);
					document.getElementById('xcu-section')?.remove();
				}
			}
		}

		this.setupLeftNavbar();

		if (data.autotext)
			this.populateList('autotextList', data.autotext, '/autotext');
		if (data.wordbook)
			this.populateList('wordbookList', data.wordbook, '/wordbook');
		if (data.xcu) this.populateList('XcuList', data.xcu, '/xcu');

		var navItem = document.querySelector<HTMLElement>(
			'#settings-nav .settings-nav-item',
		);
		if (navItem) navItem.focus();
	}

	private setupLeftNavbar(): void {
		if (this.isAdmin()) return;

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
					typeof override === 'boolean'
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

		const isSecretField = key === 'aiProviderAPIKey';

		if (isSignatureField) {
			const textarea = this.createTextArea(
				key as string,
				_(`Enter ${label}`),
				value,
				(textareaElement) => {
					(data as any)[key] = textareaElement.value;
				},
			);
			container.appendChild(textarea);
		} else {
			const input = this.createTextInput(
				key as string,
				_(`Enter ${label}`),
				value,
				(inputElement) => {
					(data as any)[key] = inputElement.value;
				},
			);
			if (isSecretField) {
				input.type = 'password';
			}
			container.appendChild(input);
		}

		return container;
	}

	private createSettingsActions(
		prefix: string,
		settingsName: string,
		filename: string,
		getDefaultSettings: () => any,
		getCurrentSettings: () => any,
		uploadSettings: (settings: any) => Promise<void>,
	): HTMLDivElement {
		const actionsContainer = document.createElement('div');
		actionsContainer.classList.add('xcu-editor-actions');

		const resetButton = this.createButtonWithText(
			`${prefix}-reset-button`,
			_('Reset'),
			_(`Reset to default ${settingsName}`),
			['button--vue-secondary', `${prefix}-reset-icon`],
			async (button) => {
				const confirmed = window.confirm(
					_(`Are you sure you want to reset ${settingsName}?`),
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

		const saveButton = this.createButtonWithText(
			`${prefix}-save-button`,
			_('Save'),
			_(`Save ${settingsName}`),
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
			aiProviderURL: this.getDefaultAIProviderURL(),
			aiProviderAPIKey: '',
			aiProviderModel: '',
			aiImageProviderAPIKey: '',
			aiImageProviderURL: '',
			aiImageModel: '',
			aiImageSize: '',
			aiRequestTimeout: '',
		};
	}

	private normalizeBaseUrl(value: string): string {
		return value ? value.replace(/\/+$/, '') : '';
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
		return this.getProviderIdFromUrl(data.aiProviderURL);
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

	static showErrorModal(message: string): void {
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
		(window as any).settingIframe = new SettingIframe();
		(window as any).settingIframe.init();
		const postHeight = () => {
			window.parent.postMessage(
				{
					MessageId: 'Iframe_Height',
					SendTime: Date.now(),
					Values: {
						ContentHeight: document.documentElement.offsetHeight + 'px',
					},
				},
				window.origin,
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
	}
});

(window as any)._ = _;
(window as any).onload = onLoaded;
