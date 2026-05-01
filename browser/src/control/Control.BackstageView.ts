// @ts-strict-ignore -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*-

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* global $ _ */

/*
 * Backstage view: state, event wiring, view transitions, and data
 * preparation. The DOM structure for each view lives in the .tsx files
 * under control/backstage/, which export pure render functions that this
 * class composes.
 */

class BackstageView extends window.L.Class {
	private readonly container: HTMLElement;
	private contentArea!: HTMLElement;
	private isVisible: boolean = false;
	private readonly map: any;
	private templates: TemplateManifestEntry[] | null = null;
	private templatesPromise: Promise<void> | null = null;
	private templatesLoadError: boolean = false;
	private activeTemplateType: TemplateType = 'writer';
	private templateSearchQuery: string = '';
	private templateGridContainer: HTMLElement | null = null;
	private templateFeaturedRowContainer: HTMLElement | null = null;
	private templateSearchContainer: HTMLElement | null = null;
	private saveTabElement: HTMLElement | null = null;
	private readonly isStarterMode: boolean;

	private actionHandlers: Record<string, () => void> = {
		open: () => this.executeOpen(),
		save: () => this.executeSave(),
		saveas: () => this.executeSaveAs(),
		print: () => this.executePrint(),
		share: () => this.executeShare(),
		repair: () => this.executeRepair(),
		properties: () => this.executeDocumentProperties(),
		history: () => this.executeRevisionHistory(),
		options: () => this.executeOptions(),
		about: () => this.executeAbout(),
	};

	constructor(map: any) {
		super();
		this.map = map;
		this.isStarterMode = (window as any).starterScreen;
		this.container = this.createContainer();
		document.body.appendChild(this.container);
		this.map?.on(
			'commandstatechanged',
			(e: any) => {
				if (e.commandName === '.uno:ModifiedStatus') {
					this.updateSaveButtonState();
				}
			},
			this,
		);
	}

	private createContainer(): HTMLElement {
		const container = BackstageTemplates.shell({
			isStarterMode: this.isStarterMode,
			tabs: this.getTabsConfig(),
			onTabClick: (cfg) => this.onTabClick(cfg),
			onClose: () => this.hide(),
			onBackClick: () => this.hide(),
			saveTabRef: (el) => {
				this.saveTabElement = el;
			},
			contentAreaRef: (el) => {
				this.contentArea = el;
			},
		});

		this.renderHomeView();
		return container;
	}

	private onTabClick(config: BackstageTabConfig): void {
		if (config.type === 'view') this.handleViewTab(config);
		else this.handleActionTab(config);
	}

	private getTabsConfig(): BackstageTabConfig[] {
		return [
			{
				id: 'home',
				label: _('Home'),
				type: 'view',
				viewType: 'home',
				icon: 'lc_home.svg',
				visible: true,
			},
			{
				id: 'new',
				label: _('New'),
				type: 'view',
				viewType: 'templates',
				icon: 'lc_newdoc.svg',
				visible: true,
			},
			{
				id: 'open',
				label: _('Open'),
				type: 'action',
				actionType: 'open',
				icon: 'lc_open.svg',
				visible: true,
			},
			{
				id: 'share',
				label: _('Share'),
				type: 'action',
				actionType: 'share',
				icon: 'lc_shareas.svg',
				visible: !this.isStarterMode && this.isFeatureEnabled('share'),
			},
			{
				id: 'info',
				label: _('Info'),
				type: 'view',
				viewType: 'info',
				icon: 'lc_printpreview.svg',
				visible: !this.isStarterMode,
			},
			{
				id: 'save',
				label: _('Save'),
				type: 'action',
				actionType: 'save',
				icon: 'lc_save.svg',
				visible: !this.isStarterMode && this.isFeatureEnabled('save'),
			},
			{
				id: 'saveas',
				label: _('Save As'),
				type: 'action',
				actionType: 'saveas',
				icon: 'lc_saveacopy.svg',
				visible: !this.isStarterMode && this.isFeatureEnabled('saveAs'),
			},
			{
				id: 'print',
				label: _('Print'),
				type: 'action',
				actionType: 'print',
				icon: 'lc_printlayout.svg',
				visible: !this.isStarterMode && this.isFeatureEnabled('print'),
			},
			{
				id: 'export',
				label: _('Export'),
				type: 'view',
				viewType: 'export',
				icon: 'lc_exportto.svg',
				visible: !this.isStarterMode,
			},
			{
				type: 'separator',
				id: 'sidebar-horizontal-break',
				label: '',
				visible: !this.isStarterMode,
			},
			{
				id: 'options',
				label: _('Options'),
				type: 'action',
				actionType: 'options',
				visible: false /* it was !this.isStarterMode and we can revert, when the dialog is good enough */,
			},
			{
				id: 'about',
				label: _('About'),
				type: 'action',
				actionType: 'about',
				visible: !this.isStarterMode,
			},
		];
	}

	private handleViewTab(config: BackstageTabConfig): void {
		const viewRenderers: Record<string, () => void> = {
			home: () => this.renderHomeView(),
			templates: () => this.renderNewView(),
			info: () => this.renderInfoView(),
			export: () => this.renderExportView(),
		};

		const viewType = config.viewType;
		if (!viewType) return;

		const renderer = viewRenderers[viewType];
		if (renderer) renderer();
	}

	private handleActionTab(config: BackstageTabConfig): void {
		if (config.actionType === 'save' && !this.isDocumentModified()) return;
		const handler = this.actionHandlers[config.actionType || ''];
		if (handler) handler();
	}

	private isFeatureEnabled(feature: string): boolean {
		const wopi = this.map['wopi'] || {};
		const featureFlags: Record<string, boolean> = {
			share: !!wopi.EnableShare,
			save: !wopi.HideSaveOption,
			saveAs:
				!wopi.UserCanNotWriteRelative ||
				window.ThisIsTheQtApp ||
				window.ThisIsTheWindowsApp,
			print: !wopi.HidePrintOption,
			repair: !wopi.HideRepairOption,
		};
		return featureFlags[feature] !== false;
	}

	private renderHomeView(): void {
		this.setActiveTab('backstage-home');
		this.clearContent();

		this.contentArea.appendChild(
			BackstageTemplates.sectionHeader(
				_('Home'),
				_('Start a new document from one of the following templates.'),
			),
		);

		if (!this.templates) {
			this.contentArea.appendChild(BackstageTemplates.templatesLoading());
			this.loadTemplatesData().then(() => {
				if (this.isVisible) this.renderHomeView();
			});
			return;
		}

		const templateType = this.isStarterMode
			? 'writer'
			: this.detectTemplateTypeFromDoc();

		const allTemplates = this.getTemplatesData();
		const appSpecificTemplates = allTemplates.filter(
			(template) => template.type === templateType,
		);

		const blankTemplate = this.getBlankTemplate(templateType);
		const templatesForHome: TemplateData[] = [];
		if (blankTemplate) templatesForHome.push(blankTemplate);
		templatesForHome.push(...appSpecificTemplates);

		this.contentArea.appendChild(
			BackstageTemplates.homeTemplatesSection({
				templates: templatesForHome,
				onTemplateClick: (t) => this.triggerNewDocument(t),
				onMoreTemplatesClick: () => {
					const newTabElement = document.getElementById('backstage-new');
					if (newTabElement) newTabElement.click();
					else this.renderNewView();
				},
			}),
		);

		this.contentArea.appendChild(BackstageTemplates.sectionHeader(_('Recent')));
		this.renderRecentDocuments();
	}

	private async renderRecentDocuments(): Promise<void> {
		try {
			const result = await window.postMobileCall('GETRECENTDOCS');
			if (!result || typeof result !== 'string') {
				this.contentArea.appendChild(BackstageTemplates.emptyRecentDocs());
				return;
			}

			const docs = JSON.parse(result);
			if (!Array.isArray(docs) || docs.length === 0) {
				this.contentArea.appendChild(BackstageTemplates.emptyRecentDocs());
				return;
			}

			const rows: BackstageTemplates.RecentDocRowData[] = docs.map((doc, i) => {
				const { fileName, filePath, timestamp, uri } =
					this.parseDocumentData(doc);
				return {
					index: i,
					fileName,
					filePath,
					formattedTime: this.formatTimestamp(timestamp),
					uri,
					iconClass: this.getDocumentIconClass(doc.doctype || 'writer'),
				};
			});

			this.contentArea.appendChild(
				BackstageTemplates.recentDocsTable(rows, (uri) =>
					this.openRecentDocument(uri),
				),
			);
		} catch (e) {
			console.error('Failed to get recent documents:', e);
			this.contentArea.appendChild(BackstageTemplates.emptyRecentDocs());
		}
	}

	private parseDocumentData(doc: any): {
		fileName: string;
		filePath: string;
		timestamp: string | null;
		uri: string;
	} {
		const uri = doc.uri || '';
		const url = new URL(uri);
		let fullPath = url.pathname;

		// On Windows a file: URI for a local file looks like this:
		// file:///C:/Users/tml/foo.odt .  URL::pathname has a leading slash and is thus not
		// valid as a pathname, we need to strip that slash away.
		if (
			window.ThisIsTheWindowsApp &&
			fullPath[0] === '/' &&
			fullPath[2] === ':'
		)
			fullPath = fullPath.slice(1);

		// On Windows, a file: URI for a UNC path looks like this:
		// file://server/share/sibdur/foo.odt . We obviously want to show the complete UNC
		// path for the directory where the document is, and the "server" path is in
		// url.host.
		if (window.ThisIsTheWindowsApp && url.host !== '')
			fullPath = '//' + url.host + fullPath;

		// We want to show a more native pathname with backslashes instead of the slashes as
		// used in file URIs.
		if (window.ThisIsTheWindowsApp)
			fullPath = (fullPath as any).replaceAll('/', '\\');

		// We want to show non-ASCII characters in the pathname as such, not
		// percent-encoded.
		fullPath = decodeURIComponent(fullPath);

		const lastSlashIndex = Math.max(
			fullPath.lastIndexOf('/'),
			fullPath.lastIndexOf('\\'),
		);

		const fileName =
			doc.name ||
			(lastSlashIndex >= 0 ? fullPath.slice(lastSlashIndex + 1) : fullPath);
		const filePath =
			lastSlashIndex >= 0 ? fullPath.slice(0, lastSlashIndex + 1) : '/';

		if (!doc.doctype) {
			const ext = fileName.split('.').pop()?.toLowerCase() || '';
			if (['ods', 'ots', 'xls', 'xlsx', 'csv', 'fods'].includes(ext)) {
				doc.doctype = 'calc';
			} else if (['odp', 'otp', 'ppt', 'pptx', 'fodp'].includes(ext)) {
				doc.doctype = 'impress';
			} else {
				doc.doctype = 'writer'; // Default
			}
		}

		return {
			fileName,
			filePath,
			timestamp: doc.timestamp || '',
			uri,
		};
	}

	private formatTimestamp(timestamp: string): string {
		if (!timestamp) return '';

		try {
			const date = new Date(timestamp);
			if (isNaN(date.getTime())) return timestamp;

			const year = date.getFullYear();
			const month = String(date.getMonth() + 1).padStart(2, '0');
			const day = String(date.getDate()).padStart(2, '0');
			const hours = String(date.getHours()).padStart(2, '0');
			const minutes = String(date.getMinutes()).padStart(2, '0');
			const seconds = String(date.getSeconds()).padStart(2, '0');

			// TODO: Need Local formatting here?
			return `${year}/${month}/${day} at ${hours}:${minutes}:${seconds}`;
		} catch {
			return timestamp;
		}
	}

	private openRecentDocument(uri: string): void {
		if (!uri) {
			console.warn('openRecentDocument: URI is missing');
			return;
		}
		window.postMobileMessage(`opendoc file=${encodeURIComponent(uri)}`);
	}

	private getDocumentIconClass(docType: string): string {
		switch (docType) {
			case 'calc':
				return 'backstage-doc-icon-calc';
			case 'impress':
				return 'backstage-doc-icon-impress';
			case 'writer':
			default:
				return 'backstage-doc-icon-writer';
		}
	}

	private renderNewView(): void {
		this.setActiveTab('backstage-new');
		this.clearContent();
		this.templateGridContainer = null;
		this.templateFeaturedRowContainer = null;
		this.templateSearchContainer = null;

		this.contentArea.appendChild(BackstageTemplates.sectionHeader(_('New')));

		if (!this.templates) {
			this.contentArea.appendChild(BackstageTemplates.templatesLoading());
			this.loadTemplatesData().then(() => {
				if (this.isVisible) this.renderNewView();
			});
			return;
		}

		const allTemplates = this.getTemplatesData();
		this.activeTemplateType = this.detectTemplateTypeFromDoc();

		this.contentArea.appendChild(this.buildTemplateExplorer(allTemplates));
	}

	private buildTemplateExplorer(allTemplates: TemplateData[]): HTMLElement {
		const filteredTemplates = this.getFilteredTemplates(allTemplates);
		return BackstageTemplates.templateExplorer({
			featuredTemplates: this.getFeaturedTemplates(),
			gridTemplates: filteredTemplates,
			searchQuery: this.templateSearchQuery,
			showSearch: allTemplates.length > 0,
			onTemplateClick: (t) => this.triggerNewDocument(t),
			onSearchInput: (value) => {
				this.templateSearchQuery = value;
				this.updateTemplateGrid();
			},
			featuredRowRef: (el) => {
				this.templateFeaturedRowContainer = el;
			},
			searchContainerRef: (el) => {
				this.templateSearchContainer = el;
			},
			gridContainerRef: (el) => {
				this.templateGridContainer = el;
			},
		});
	}

	private getFeaturedTemplates(): TemplateData[] {
		const blankTemplates = [
			this.getBlankTemplate('writer'),
			this.getBlankTemplate('calc'),
			this.getBlankTemplate('impress'),
		].filter((t): t is TemplateData => t !== null);

		const query = this.templateSearchQuery.trim().toLowerCase();
		return query
			? blankTemplates.filter((t) => t.searchText.includes(query))
			: blankTemplates;
	}

	private renderInfoView(): void {
		this.setActiveTab('backstage-info');
		this.clearContent();

		this.contentArea.appendChild(
			BackstageTemplates.sectionHeader(
				_('Document Info'),
				_('View document properties and information'),
			),
		);

		this.contentArea.appendChild(
			BackstageTemplates.infoPropertiesColumn(
				this.getDocumentProperties(),
				() => this.executeDocumentProperties(),
			),
		);
	}

	private renderExportView(): void {
		this.setActiveTab('backstage-export');
		this.clearContent();

		// extract list from notebookbar - in case need to add condition better to add there?
		const exportOptions = this.getExportOptionsFromNotebookbar();

		if (exportOptions.downloadAs.length > 0) {
			this.contentArea.appendChild(
				BackstageTemplates.sectionHeader(
					_('Export Document'),
					_('export your documents in different formats'),
				),
			);

			this.contentArea.appendChild(
				BackstageTemplates.exportGrid(
					exportOptions.downloadAs,
					(action, command) => this.dispatchExportAction(action, command),
				),
			);
		}
	}

	private getFileName = (wopi: any): string => {
		if (wopi?.BreadcrumbDocName) return wopi.BreadcrumbDocName;
		if (wopi?.BaseFileName) return wopi.BaseFileName;

		const doc = this.map?.options?.doc;
		if (doc) {
			const filename = doc.split('/').pop()?.split('?')[0];
			if (filename) return decodeURIComponent(filename);
		}

		return '';
	};

	private getDocumentProperties(): BackstageTemplates.DocumentProperty[] {
		const docType = this.getDocTypeString();
		const docLayer = this.map?._docLayer;
		const wopi = this.map?.['wopi'];

		const typeLabels: Record<string, { type: string; parts: string }> = {
			text: { type: _('Text Document'), parts: _('Pages') },
			spreadsheet: { type: _('Spreadsheet'), parts: _('Sheets') },
			presentation: { type: _('Presentation'), parts: _('Slides') },
			drawing: { type: _('Drawing'), parts: _('Pages') },
		};

		const config = typeLabels[docType] || {
			type: _('Document'),
			parts: _('Parts'),
		};

		const properties: BackstageTemplates.DocumentProperty[] = [
			{ label: _('Type'), value: config.type },
			{
				label: config.parts,
				value: docLayer?._parts ? String(docLayer._parts) : '-',
			},
			{
				label: _('Mode'),
				value: this.map?.isEditMode() ? _('Edit') : _('View Only'),
			},
		];

		const filename = this.getFileName(wopi);
		if (filename) {
			properties.splice(1, 0, {
				label: _('File Name'),
				value: filename,
			});
		}

		return properties;
	}

	private clearContent(): void {
		this.contentArea.replaceChildren();
	}

	private setActiveTab(tabId: string): void {
		document
			.querySelectorAll('.backstage-sidebar-item')
			.forEach((el) => el.classList.remove('active'));
		document.getElementById(tabId)?.classList.add('active');
	}

	private getTemplatesData(): TemplateData[] {
		const entries = this.templates || [];
		const templates: TemplateData[] = [];

		entries.forEach((entry) => {
			const absolutePath = entry.path;
			if (!absolutePath) return;

			const type = this.normalizeTemplateType(entry.type, absolutePath);
			if (!type) return;

			const name = entry.name;
			if (!name) return;

			const id = entry.id || this.slugify(name + absolutePath);
			const searchComponents = [name, type, absolutePath];

			templates.push({
				id,
				name,
				type,
				path: absolutePath,
				basename: entry.basename || undefined,
				preview: entry.preview || undefined,
				featured: !!entry.featured,
				searchText: searchComponents.join(' ').toLowerCase(),
			});
		});

		// Sort by type first (writer, calc, impress), then alphabetically by name
		const typeOrder: Record<TemplateType, number> = {
			writer: 1,
			calc: 2,
			impress: 3,
		};

		return templates.sort((a, b) => {
			const typeComparison = typeOrder[a.type] - typeOrder[b.type];
			if (typeComparison !== 0) return typeComparison;
			return a.name.localeCompare(b.name, undefined, { sensitivity: 'base' });
		});
	}

	private async loadTemplatesData(): Promise<void> {
		if (this.templatesPromise) return this.templatesPromise;

		const loader = (async () => {
			try {
				// fetch templates from templates/templates.js build path
				const entries = (window as any).CODA_TEMPLATES || [];
				this.templates = entries;
				this.templatesLoadError = false;

				if (!entries || entries.length === 0) {
					console.warn('Templates manifest loaded but contains no templates!');
				}
			} catch (error) {
				console.error('Unable to load templates manifest', error);
				this.templates = [];
				this.templatesLoadError = true;
			}
		})();

		this.templatesPromise = loader.finally(() => {
			this.templatesPromise = null;
		});

		return this.templatesPromise;
	}

	private getFilteredTemplates(allTemplates: TemplateData[]): TemplateData[] {
		const query = this.templateSearchQuery.trim().toLowerCase();
		return allTemplates.filter((template) => {
			if (!query) return true;
			return template.searchText.includes(query);
		});
	}

	private updateTemplateGrid(): void {
		if (!this.templates || !this.templateGridContainer) return;

		const allTemplates = this.getTemplatesData();
		const filteredTemplates = this.getFilteredTemplates(allTemplates);
		const newGrid = BackstageTemplates.templateGrid(
			filteredTemplates,
			this.templateSearchQuery,
			(t) => this.triggerNewDocument(t),
		);
		this.templateGridContainer.replaceWith(newGrid);
		this.templateGridContainer = newGrid;

		const featured = this.getFeaturedTemplates();
		const newFeaturedRow = featured.length
			? BackstageTemplates.featuredTemplatesRow(featured, (t) =>
					this.triggerNewDocument(t),
				)
			: null;

		if (this.templateFeaturedRowContainer) {
			if (newFeaturedRow) {
				this.templateFeaturedRowContainer.replaceWith(newFeaturedRow);
				this.templateFeaturedRowContainer = newFeaturedRow;
			} else {
				this.templateFeaturedRowContainer.remove();
				this.templateFeaturedRowContainer = null;
			}
		} else if (newFeaturedRow && this.templateSearchContainer?.parentElement) {
			this.templateSearchContainer.parentElement.insertBefore(
				newFeaturedRow,
				this.templateSearchContainer,
			);
			this.templateFeaturedRowContainer = newFeaturedRow;
		} else if (newFeaturedRow && this.templateGridContainer.parentElement) {
			this.templateGridContainer.parentElement.insertBefore(
				newFeaturedRow,
				this.templateGridContainer,
			);
			this.templateFeaturedRowContainer = newFeaturedRow;
		}
	}

	private getBlankTemplate(type: TemplateType): TemplateData | null {
		let name: string;
		let previewPath: string | undefined;
		switch (type) {
			case 'calc':
				name = _('Blank Spreadsheet');
				previewPath = 'images/templates/preview/blank_spreadsheet.png';
				break;
			case 'impress':
				name = _('Blank Presentation');
				previewPath = 'images/templates/preview/blank_presentation.png';
				break;
			case 'writer':
			default:
				name = _('Blank Document');
				previewPath = 'images/templates/preview/blank_writer.png';
				break;
		}

		return {
			id: `blank-${type}`,
			name,
			type,
			preview: previewPath,
			searchText: `${name.toLowerCase()} ${type} blank`,
		};
	}

	private detectTypeFromPath(templatePath: string): TemplateType | null {
		const extension = templatePath.split('.').pop()?.toLowerCase() || '';
		const map: Record<string, TemplateType> = {
			ott: 'writer',
			oth: 'writer',
			otm: 'writer',
			ots: 'calc',
			otp: 'impress',
		};
		return map[extension] || null;
	}

	private normalizeTemplateType(
		entryType: string | undefined,
		templatePath: string,
	): TemplateType | null {
		const normalized = (entryType || '').toLowerCase();
		if (
			normalized === 'writer' ||
			normalized === 'calc' ||
			normalized === 'impress'
		)
			return normalized as TemplateType;
		return this.detectTypeFromPath(templatePath);
	}

	private slugify(value: string): string {
		return value
			.toLowerCase()
			.replace(/[^a-z0-9]+/g, '-')
			.replace(/^-+|-+$/g, '')
			.replace(/-{2,}/g, '-');
	}

	private getDocTypeString(): string {
		const docLayer = this.map && this.map._docLayer;
		const docType = docLayer && docLayer._docType;
		return docType || 'text';
	}

	private getExportOptionsFromNotebookbar(): ExportSections {
		const docType = this.getDocTypeString();
		const builder = new (window.L.Control.NotebookbarBuilder as any)();

		const downloadAsOpts: ExportOptionItem[] = builder._getDownloadAsSubmenuOpts
			? builder._getDownloadAsSubmenuOpts(docType) || []
			: [];

		return {
			exportAs: [],
			downloadAs: downloadAsOpts,
		};
	}

	private executeOpen(): void {
		if (this.isStarterMode) {
			window.postMobileMessage('uno .uno:Open');
		} else {
			this.sendUnoCommand('.uno:Open');
		}
	}

	private executeOptions(): void {
		this.map.settings.showSettingsDialog();
	}

	private executeAbout(): void {
		this.map.showLOAboutDialog();
	}

	private executeSave(): void {
		if (this.map && this.map.save) {
			this.map.save(false, false);
		}
		this.hide();
	}

	private executeSaveAs(): void {
		this.sendUnoCommand('.uno:SaveAs');
		this.hide();
	}

	private executePrint(): void {
		if (window.ThisIsAMobileApp) {
			window.postMobileMessage('PRINT');
		} else if (this.map && this.map.print) {
			this.map.print();
		}
		this.hide();
	}

	private executeShare(): void {
		this.fireMapEvent('postMessage', { msgId: 'UI_Share' });
		this.hide();
	}

	private executeRevisionHistory(): void {
		this.fireMapEvent('postMessage', { msgId: 'rev-history' });
		this.hide();
	}

	private executeRepair(): void {
		this.sendUnoCommand('.uno:Repair');
		this.hide();
	}

	private executeDocumentProperties(): void {
		this.sendUnoCommand('.uno:SetDocumentProperties');
		this.hide();
	}

	private triggerNewDocument(template: TemplateData): void {
		const docType = template.type || 'writer';
		const params: string[] = ['type=' + docType];
		if (template.path) {
			params.push('template=' + encodeURIComponent(template.path));
		}

		// URI-encode the basename parameter because the syntax for these keyword=value
		// style of messages doesn't allow spaces inside a value, and a translation of
		// the template file basename might contains a space.

		// Such basenames should be in the templates.js file. If a template doesn't
		// have one, the localisation of "Untitled" is the default.
		const basename = template.basename || _('Untitled');
		params.push('basename=' + encodeURIComponent(basename));

		window.postMobileMessage('newdoc ' + params.join(' '));

		if (!this.isStarterMode) {
			this.hide();
		}
	}

	private dispatchExportAction(action: string, command?: string): void {
		const actionToDispatch = command || action;

		if (window.app && window.app.dispatcher) {
			window.app.dispatcher.dispatch(actionToDispatch);
		} else {
			console.warn('app.dispatcher not available, using fallback');
			this.handleExportFallback(actionToDispatch);
		}
		this.hide();
	}

	private getBaseFileName(): string {
		const fileName = this.map?.['wopi']?.BaseFileName || 'document';
		const lastDot = fileName.lastIndexOf('.');
		return lastDot > 0 ? fileName.substring(0, lastDot) : fileName;
	}

	private handleExportFallback(action: string): void {
		if (action === 'exportdirectpdf') {
			if (this.map && this.map.downloadAs) {
				this.map.downloadAs(this.getBaseFileName() + '.pdf', 'pdf');
			}
			return;
		}

		if (action.startsWith('downloadas-')) {
			const format = action.substring('downloadas-'.length);
			if (this.map && this.map.downloadAs) {
				this.map.downloadAs(this.getBaseFileName() + '.' + format, format);
			}
			return;
		}

		console.error('something want wrong with this action: ', action);
	}

	private sendUnoCommand(command: string): void {
		if (this.map && this.map.sendUnoCommand) {
			this.map.sendUnoCommand(command);
		}
	}

	private fireMapEvent(eventName: string, data?: any): void {
		if (this.map && this.map.fire) {
			this.map.fire(eventName, data);
		}
	}

	public show(tab: 'home' | 'new' = 'home'): void {
		if (this.isVisible && tab === 'home') {
			return;
		}

		if (!this.isVisible) {
			this.isVisible = true;
			$(this.container).removeClass('hidden');
			this.hideDocumentContainer();
			this.updateSaveButtonState();
			this.container.focus();
			this.fireMapEvent('backstageshow');
		}

		if (tab === 'new') {
			this.renderNewView();
		} else {
			this.renderHomeView();
		}
	}

	public hide(): void {
		if (!this.isVisible) {
			return;
		}

		this.isVisible = false;
		$(this.container).addClass('hidden');
		this.showDocumentContainer();
		this.focusMap();
		this.fireMapEvent('backstagehide');
	}

	public toggle(): void {
		if (this.isVisible) {
			this.hide();
		} else {
			this.show();
		}
	}

	private hideDocumentContainer(): void {
		$('#document-container').addClass('hidden');
		$('.notebookbar-scroll-wrapper').addClass('hidden');
	}

	private showDocumentContainer(): void {
		$('#document-container').removeClass('hidden');
		$('.notebookbar-scroll-wrapper').removeClass('hidden');
	}

	private focusMap(): void {
		if (this.map && this.map.focus) {
			this.map.focus();
		}
	}

	private detectTemplateTypeFromDoc(): TemplateType {
		const docLayer = this.map && this.map._docLayer;
		const docType = docLayer && docLayer._docType;
		switch (docType) {
			case 'spreadsheet':
				return 'calc';
			case 'presentation':
			case 'drawing':
				return 'impress';
			default:
				return 'writer';
		}
	}

	private isDocumentModified(): boolean {
		return (
			this.map?.stateChangeHandler?.getItemValue('.uno:ModifiedStatus') ===
			'true'
		);
	}

	private updateSaveButtonState(): void {
		if (!this.saveTabElement) return;
		this.saveTabElement.classList.toggle(
			'disabled',
			!this.isDocumentModified(),
		);
	}
}

window.L.Control.BackstageView = BackstageView;
