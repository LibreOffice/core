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

interface BackstageTabConfig {
	id: string;
	label: string;
	type: 'view' | 'action' | 'separator';
	icon?: string;
	visible?: boolean;
	viewType?: 'home' | 'templates' | 'info' | 'export';
	actionType?:
		| 'open'
		| 'save'
		| 'saveas'
		| 'print'
		| 'share'
		| 'repair'
		| 'properties'
		| 'history'
		| 'options'
		| 'about';
}

interface TemplateTypeMap {
	writer: 'writer';
	calc: 'calc';
	impress: 'impress';
}

type TemplateType = TemplateTypeMap[keyof TemplateTypeMap];

interface TemplateData {
	id: string;
	name: string;
	type: TemplateType;
	path?: string;
	basename?: string;
	preview?: string;
	featured?: boolean;
	searchText: string;
}

interface TemplateManifestEntry {
	id?: string;
	name?: string;
	type?: string;
	category?: string;
	path: string;
	basename?: string | null;
	preview?: string | null;
	featured?: boolean;
}

interface TemplateManifest {
	templates?: TemplateManifestEntry[];
}

interface ExportFormatData {
	id: string;
	name: string;
	description: string;
}

interface ExportOptionItem {
	action: string;
	text: string;
	command?: string;
}

// todo: currently export as and downloadAs with pdf as not working, skipping for moment
interface ExportSections {
	exportAs: ExportOptionItem[];
	downloadAs: ExportOptionItem[];
}

/**
 * BackstageSVGIcons is a generated map of the form 'icon_name' -> 'SVG content'.
 *
 * It is created as generated JS directly, and bundled, so that things work
 * together.
 *
 * We need to do it this way, because it is very complicated to combine a
 * compiled TS with generated TS in out-of-tree builds.
 */
declare const BackstageSVGIcons: Record<string, string>;

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
		const container = this.createElement(
			'div',
			'backstage-view hidden',
			'backstage-view',
		);

		if (this.isStarterMode) {
			container.classList.add('is-starter-mode');
		}

		const header = this.createHeader();
		container.appendChild(header);

		const mainWrapper = this.createElement('div', 'backstage-main-wrapper');
		const sidebar = this.createSidebar();
		this.contentArea = this.createElement('div', 'backstage-content');

		mainWrapper.appendChild(sidebar);
		mainWrapper.appendChild(this.contentArea);
		container.appendChild(mainWrapper);

		this.renderHomeView();
		return container;
	}

	private createHeader(): HTMLElement {
		const header = this.createElement('div', 'backstage-header');
		const title = this.createElement('span', 'backstage-header-title');
		title.textContent = 'Collabora Office';

		header.appendChild(title);

		// Only show close button if not in starter screen mode
		if (!this.isStarterMode) {
			const closeButton = this.createElement('div', 'backstage-header-close');
			closeButton.setAttribute('aria-label', _('Close backstage'));
			closeButton.title = _('Close backstage');

			const closeBtn = this.createElement(
				'span',
				'backstage-header-close-icon',
			);
			closeBtn.setAttribute('aria-hidden', 'true');
			closeButton.appendChild(closeBtn);

			window.L.DomEvent.on(closeButton, 'click', () => this.hide(), this);

			header.appendChild(closeButton);
		}

		return header;
	}

	private createSidebar(): HTMLElement {
		const sidebar = this.createElement('div', 'backstage-sidebar');

		// Only show back button if not in starter screen mode
		if (!this.isStarterMode) {
			const backButton = this.createSidebarBackBtn();
			sidebar.appendChild(backButton);
		}

		const tabsContainer = this.createElement('div', 'backstage-sidebar-tabs');
		const tabConfigs = this.getTabsConfig();

		tabConfigs.forEach((config) => {
			if (config.visible === false) return;

			const tabElement = this.createTabElement(config);
			tabsContainer.appendChild(tabElement);
		});

		sidebar.appendChild(tabsContainer);
		return sidebar;
	}

	private createSidebarBackBtn(): HTMLElement {
		const backButton = this.createElement('div', 'backstage-sidebar-back');

		backButton.setAttribute('aria-label', _('Back to document'));
		backButton.title = _('Back to document');

		const icon = this.createElement('span', 'backstage-sidebar-back-icon');
		icon.setAttribute('aria-hidden', 'true');
		backButton.appendChild(icon);

		window.L.DomEvent.on(backButton, 'click', () => this.hide(), this);

		return backButton;
	}

	private createTabElement(config: BackstageTabConfig): HTMLElement {
		const element = this.createElement('div', 'backstage-sidebar-item');
		element.id = `backstage-${config.id}`;

		if (config.id === 'save') this.saveTabElement = element;

		if (config.icon) {
			const iconContainer = this.createElement(
				'span',
				'backstage-sidebar-icon',
			);
			iconContainer.setAttribute('aria-hidden', 'true');

			const svgContent = BackstageSVGIcons[config.icon];
			if (svgContent) {
				iconContainer.innerHTML = svgContent;
			}
			element.insertBefore(iconContainer, element.firstChild);
		}

		const label = this.createElement('span');
		label.textContent = config.label;
		element.appendChild(label);

		const action =
			config.type === 'view'
				? () => this.handleViewTab(config)
				: () => this.handleActionTab(config);

		window.L.DomEvent.on(element, 'click', action, this);
		return element;
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

		this.addSectionHeader(
			_('Home'),
			_('Start a new document from one of the following templates.'),
		);

		if (!this.templates) {
			this.renderTemplatesLoadingState();
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

		if (blankTemplate) {
			templatesForHome.push(blankTemplate);
		}

		templatesForHome.push(...appSpecificTemplates);

		const templatesRow = this.createTemplateRow(
			templatesForHome,
			'backstage-home-templates-row',
		);

		const moreTemplatesContainer = this.createElement(
			'div',
			'backstage-home-more-templates',
		);
		const divider = this.createElement('div', 'backstage-home-divider');
		const moreTemplatesButton = this.createElement(
			'div',
			'backstage-home-more-button',
		);
		moreTemplatesButton.textContent = _('More Templates');
		moreTemplatesButton.setAttribute('role', 'button');
		moreTemplatesButton.setAttribute('tabindex', '0');
		moreTemplatesButton.setAttribute('aria-label', _('More Templates'));

		const handleMoreTemplatesClick = () => {
			const newTabElement = document.getElementById('backstage-new');
			if (newTabElement) {
				newTabElement.click();
			} else {
				this.renderNewView();
			}
		};

		window.L.DomEvent.on(
			moreTemplatesButton,
			'click',
			handleMoreTemplatesClick,
			this,
		);

		moreTemplatesContainer.appendChild(divider);
		moreTemplatesContainer.appendChild(moreTemplatesButton);

		if (templatesRow) {
			this.contentArea.appendChild(templatesRow);
		}
		this.contentArea.appendChild(moreTemplatesContainer);

		this.addSectionHeader(_('Recent'), '');

		this.renderRecentDocuments();
	}

	private async renderRecentDocuments(): Promise<void> {
		try {
			const result = await window.postMobileCall('GETRECENTDOCS');
			if (!result || typeof result !== 'string') {
				this.showEmptyMessage();
				return;
			}

			const docs = JSON.parse(result);
			if (!Array.isArray(docs) || docs.length === 0) {
				this.showEmptyMessage();
				return;
			}

			const table = this.createRecentDocumentsTable(docs);
			this.contentArea.appendChild(table);
		} catch (e) {
			console.error('Failed to get recent documents:', e);
			this.showEmptyMessage();
		}
	}

	private showEmptyMessage(): void {
		const description = this.createElement(
			'p',
			'backstage-content-description',
		);
		description.textContent = _('Recently opened documents will appear here');
		this.contentArea.appendChild(description);
	}

	private createRecentDocumentsTable(docs: any[]): HTMLElement {
		const table = this.createElement(
			'table',
			'backstage-recent-documents-table',
		);
		const thead = this.createRecentDocumentsTableHeader();
		const tbody = this.createElement(
			'tbody',
			'backstage-recent-documents-body',
		);

		let i: number = 0;
		docs.forEach((doc) => {
			const rows = this.createRecentDocumentRows(doc, i++);
			rows.forEach((row) => tbody.appendChild(row));
		});

		table.appendChild(thead);
		table.appendChild(tbody);
		return table;
	}

	private createRecentDocumentsTableHeader(): HTMLElement {
		const thead = this.createElement(
			'thead',
			'backstage-recent-documents-header',
		);
		const row = this.createElement(
			'tr',
			'backstage-recent-documents-header-row',
		);

		const nameHeader = this.createElement(
			'th',
			'backstage-recent-documents-header-cell',
		);
		nameHeader.textContent = _('Name');

		const dateHeader = this.createElement(
			'th',
			'backstage-recent-documents-header-cell',
		);
		dateHeader.textContent = _('Modified Date');

		row.appendChild(nameHeader);
		row.appendChild(dateHeader);
		thead.appendChild(row);
		return thead;
	}

	private createRecentDocumentRows(doc: any, i: number): HTMLElement[] {
		const { fileName, filePath, timestamp, uri } = this.parseDocumentData(doc);
		const docType = doc.doctype || 'writer';
		const formattedTime = this.formatTimestamp(timestamp);

		const row = this.createRecentDocumentRow(
			fileName,
			filePath,
			formattedTime,
			uri,
			docType,
			i,
		);
		return [row];
	}

	private createRecentDocumentRow(
		fileName: string,
		filePath: string,
		formattedTime: string,
		uri: string,
		docType: string,
		i: number,
	): HTMLElement {
		const row = this.createElement('tr', 'backstage-recent-document-row');

		const nameCell = this.createElement(
			'td',
			'backstage-recent-document-name-cell',
			'backstage-recent-document-' + i,
		);
		const nameText = this.createElement(
			'div',
			'backstage-recent-document-name-text',
		);
		nameText.textContent = fileName;

		const pathText = this.createElement(
			'div',
			'backstage-recent-document-path-text',
		);
		pathText.textContent = filePath;

		const textWrapper = this.createElement(
			'div',
			'backstage-recent-document-text-wrapper',
		);
		textWrapper.appendChild(nameText);
		textWrapper.appendChild(pathText);

		const iconClass = this.getDocumentIconClass(docType);
		const icon = this.createElement(
			'span',
			`backstage-recent-document-icon ${iconClass}`,
		);

		const contentWrapper = this.createElement(
			'div',
			'backstage-recent-document-content-wrapper',
		);
		contentWrapper.appendChild(icon);
		contentWrapper.appendChild(textWrapper);

		nameCell.appendChild(contentWrapper);

		const timeCell = this.createElement(
			'td',
			'backstage-recent-document-time-cell',
		);
		timeCell.textContent = formattedTime;

		row.appendChild(nameCell);
		row.appendChild(timeCell);

		window.L.DomEvent.on(
			row,
			'click',
			() => this.openRecentDocument(uri),
			this,
		);

		return row;
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

		this.addSectionHeader(_('New'));

		if (!this.templates) {
			this.renderTemplatesLoadingState();
			this.loadTemplatesData().then(() => {
				if (this.isVisible) this.renderNewView();
			});
			return;
		}

		const templates = this.getTemplatesData();
		const explorer = this.renderTemplateExplorer(templates);
		this.contentArea.appendChild(explorer);
	}

	private renderInfoView(): void {
		this.setActiveTab('backstage-info');
		this.clearContent();

		this.addSectionHeader(
			_('Document Info'),
			_('View document properties and information'),
		);

		const propertiesColumn = this.createInfoPropertiesColumn();
		this.contentArea.appendChild(propertiesColumn);
	}

	private renderExportView(): void {
		this.setActiveTab('backstage-export');
		this.clearContent();

		// extract list from notebookbar - in case need to add condition better to add there?
		const exportOptions = this.getExportOptionsFromNotebookbar();

		if (exportOptions.downloadAs.length > 0) {
			this.addSectionHeader(
				_('Export Document'),
				_('export your documents in different formats'),
			);

			const downloadAsGrid = this.createExportGridFromOptions(
				exportOptions.downloadAs,
			);
			this.contentArea.appendChild(downloadAsGrid);
		}
	}

	private createExportGridFromOptions(
		options: ExportOptionItem[],
	): HTMLElement {
		const grid = this.createElement('div', 'backstage-formats-grid');

		options.forEach((option) => {
			const card = this.createExportCardFromOption(option);
			grid.appendChild(card);
		});

		return grid;
	}

	private createExportCardFromOption(option: ExportOptionItem): HTMLElement {
		const card = this.createElement('div', 'backstage-format-card');

		const format = option.action.startsWith('downloadas-')
			? option.action.substring('downloadas-'.length)
			: option.action.startsWith('export')
				? option.action.substring('export'.length)
				: '';
		const extension = format ? `.${format}` : '';

		const icon = this.createElement('div', 'format-icon');
		icon.textContent = extension.toUpperCase().replace('.', '');
		card.appendChild(icon);

		const description = this.createElement('div', 'format-description');
		description.textContent = option.text;
		card.appendChild(description);

		window.L.DomEvent.on(
			card,
			'click',
			() => this.dispatchExportAction(option.action, option.command),
			this,
		);
		return card;
	}

	private createInfoPropertiesColumn(): HTMLElement {
		const column = this.createElement('div', 'backstage-info-properties');

		const header = this.createElement('h3', 'backstage-section-header');
		header.textContent = _('Properties');
		column.appendChild(header);

		const propertiesList = this.createPropertiesList();
		column.appendChild(propertiesList);

		const button = this.createPrimaryButton(_('More Property Info'), () =>
			this.executeDocumentProperties(),
		);
		column.appendChild(button);

		return column;
	}

	private createElement(
		tag: string,
		className?: string,
		id?: string,
	): HTMLElement {
		const element = window.L.DomUtil.create(tag, className || '');
		if (id) element.id = id;
		return element;
	}

	private createPrimaryButton(
		label: string,
		onClick: () => void,
	): HTMLButtonElement {
		const button = this.createElement(
			'button',
			'backstage-property-button',
		) as HTMLButtonElement;
		button.textContent = label;
		window.L.DomEvent.on(button, 'click', onClick, this);
		return button;
	}

	private createPropertiesList(): HTMLElement {
		const list = this.createElement('div', 'backstage-properties-list');

		const properties = this.getDocumentProperties();

		properties.forEach((prop) => {
			if (prop.value) {
				const item = this.createPropertyItem(prop.label, prop.value);
				list.appendChild(item);
			}
		});

		return list;
	}

	private createPropertyItem(label: string, value: string): HTMLElement {
		const item = this.createElement('div', 'backstage-property-item');

		const labelEl = this.createElement('div', 'property-label');
		labelEl.textContent = label;
		item.appendChild(labelEl);

		const valueEl = this.createElement('div', 'property-value');
		valueEl.textContent = value;
		item.appendChild(valueEl);

		return item;
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

	private getDocumentProperties(): Array<{ label: string; value: string }> {
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

		const properties = [
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

	private addSectionHeader(title: string, description: string = ''): void {
		const titleElement = this.createElement('h2', 'backstage-content-title');
		titleElement.textContent = title;
		this.contentArea.appendChild(titleElement);

		if (description !== '') {
			const descElement = this.createElement(
				'p',
				'backstage-content-description',
			);
			descElement.textContent = description;
			this.contentArea.appendChild(descElement);
		}
	}

	private clearContent(): void {
		while (this.contentArea.firstChild) {
			this.contentArea.removeChild(this.contentArea.firstChild);
		}
	}

	private setActiveTab(tabId: string): void {
		$('.backstage-sidebar-item').removeClass('active');
		$('#' + tabId).addClass('active');
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

	private renderTemplatesLoadingState(): void {
		const wrapper = this.createElement('div', 'backstage-templates-empty');
		const info = this.createElement('p', 'backstage-content-description');
		info.textContent = _('Loading templates…');
		wrapper.appendChild(info);
		this.contentArea.appendChild(wrapper);
	}

	private renderTemplateExplorer(allTemplates: TemplateData[]): HTMLElement {
		const previousType = this.activeTemplateType;
		const detectedType = this.detectTemplateTypeFromDoc();
		if (detectedType !== previousType) this.templateSearchQuery = '';
		this.activeTemplateType = detectedType;

		const container = this.createElement('div', 'backstage-template-explorer');

		const filteredTemplates = this.getFilteredTemplates(allTemplates);

		const featuredRow = this.renderFeaturedRow();
		if (featuredRow) {
			container.appendChild(featuredRow);
			this.templateFeaturedRowContainer = featuredRow;
		} else {
			this.templateFeaturedRowContainer = null;
		}

		// Only show search bar if there are custom templates to search through
		if (allTemplates.length > 0) {
			const search = this.renderTemplateSearch();
			this.templateSearchContainer = search;
			container.appendChild(search);
		} else {
			this.templateSearchContainer = null;
		}

		const grid = this.renderTemplateGrid(filteredTemplates);
		this.templateGridContainer = grid;
		container.appendChild(grid);

		return container;
	}

	private renderTemplateSearch(): HTMLElement {
		const container = this.createElement('div', 'template-search');
		const input = this.createElement(
			'input',
			'template-search-input',
		) as HTMLInputElement;
		input.type = 'search';
		input.placeholder = _('Search templates');
		input.value = this.templateSearchQuery;

		window.L.DomEvent.on(
			input,
			'input',
			() => {
				this.templateSearchQuery = input.value || '';
				this.updateTemplateGrid();
			},
			this,
		);

		container.appendChild(input);
		return container;
	}

	private getFilteredTemplates(allTemplates: TemplateData[]): TemplateData[] {
		const query = this.templateSearchQuery.trim().toLowerCase();
		return allTemplates.filter((template) => {
			if (!query) return true;
			return template.searchText.includes(query);
		});
	}

	private createTemplateRow(
		templates: TemplateData[],
		className: string = 'template-featured-row',
		cardOptions: { variant?: 'featured'; isBlank?: boolean } = {},
	): HTMLElement | null {
		if (templates.length === 0) return null;

		const row = this.createElement('div', className);
		templates.forEach((template) => {
			row.appendChild(this.createTemplateCard(template, cardOptions));
		});
		return row;
	}

	private renderFeaturedRow(): HTMLElement | null {
		const blankTemplates = [
			this.getBlankTemplate('writer'),
			this.getBlankTemplate('calc'),
			this.getBlankTemplate('impress'),
		].filter((t): t is TemplateData => t !== null);

		const query = this.templateSearchQuery.trim().toLowerCase();
		const filteredBlankTemplates = query
			? blankTemplates.filter((t) => t.searchText.includes(query))
			: blankTemplates;

		return this.createTemplateRow(
			filteredBlankTemplates,
			'template-featured-row',
			{
				variant: 'featured',
				isBlank: true,
			},
		);
	}

	private renderTemplateGrid(templates: TemplateData[]): HTMLElement {
		const grid = this.createElement('div', 'backstage-templates-grid');

		if (!templates.length) {
			// Only show "no match" message if user is actively searching
			if (this.templateSearchQuery.trim()) {
				const empty = this.createElement('div', 'template-grid-empty');
				empty.textContent = _('No templates match your search.');
				grid.appendChild(empty);
			}
			return grid;
		}

		templates.forEach((template) => {
			grid.appendChild(this.createTemplateCard(template));
		});

		return grid;
	}

	private updateTemplateGrid(): void {
		if (!this.templates || !this.templateGridContainer) return;

		const allTemplates = this.getTemplatesData();
		const filteredTemplates = this.getFilteredTemplates(allTemplates);
		const newGrid = this.renderTemplateGrid(filteredTemplates);
		this.templateGridContainer.replaceWith(newGrid);
		this.templateGridContainer = newGrid;

		const newFeaturedRow = this.renderFeaturedRow();
		if (this.templateFeaturedRowContainer) {
			if (newFeaturedRow) {
				this.templateFeaturedRowContainer.replaceWith(newFeaturedRow);
				this.templateFeaturedRowContainer = newFeaturedRow;
			} else {
				this.templateFeaturedRowContainer.remove();
				this.templateFeaturedRowContainer = null;
			}
		} else if (newFeaturedRow && this.templateSearchContainer) {
			// Only try to insert if search container exists
			if (this.templateSearchContainer.parentElement) {
				this.templateSearchContainer.parentElement.insertBefore(
					newFeaturedRow,
					this.templateSearchContainer,
				);
				this.templateFeaturedRowContainer = newFeaturedRow;
			}
		} else if (newFeaturedRow && this.templateGridContainer.parentElement) {
			// If no search container, insert before grid
			this.templateGridContainer.parentElement.insertBefore(
				newFeaturedRow,
				this.templateGridContainer,
			);
			this.templateFeaturedRowContainer = newFeaturedRow;
		}
	}

	private createTemplateCard(
		template: TemplateData,
		options: { variant?: 'featured'; isBlank?: boolean } = {},
	): HTMLElement {
		const card = this.createElement(
			'div',
			'backstage-template-card',
			template.id,
		);
		if (options.variant === 'featured') card.classList.add('is-featured');
		if (options.isBlank) card.classList.add('is-blank');

		const previewWrapper = this.createElement('div', 'template-thumbnail');
		const preview = this.createElement('img') as HTMLImageElement;
		preview.alt = template.name;
		if (template.preview) preview.src = template.preview;
		else preview.src = this.getDefaultPreview(template.type);
		previewWrapper.appendChild(preview);
		card.appendChild(previewWrapper);

		const name = this.createElement('div', 'template-name');
		name.textContent = template.name;
		card.appendChild(name);

		window.L.DomEvent.on(
			card,
			'click',
			() => this.triggerNewDocument(template),
			this,
		);
		return card;
	}

	private getDefaultPreview(type: TemplateType): string {
		const previews: Record<TemplateType, string> = {
			writer: 'images/filetype/writer.svg',
			calc: 'images/filetype/calc.svg',
			impress: 'images/filetype/impress.svg',
		};
		return previews[type] || 'images/filetype/document.svg';
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
