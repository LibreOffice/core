/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface ParsedJSONResult {
	[name: string]: any;
}

interface CRSInterface {
	scale(zoom: number): number;
}

interface LatLngLike {
	lat: number;
	lng: number;
}

interface MapInterface extends Evented {
	_docLayer: DocLayerInterface;
	uiManager: UIManager;
	_textInput: {
		debug(value: boolean): void;
		_isDebugOn: boolean;
		update(): void;
	};
	addressInputField: AddressInputField;

	removeLayer(layer: any): void;
	addLayer(layer: any): void;
	setZoom(
		targetZoom: number,
		options: { [key: string]: any } | null,
		animate?: boolean,
	): void;

	stateChangeHandler: {
		getItemValue(unoCmd: string): any;
		setItemValue(unoCmd: string, value: any): void;
	};

	sendUnoCommand(unoCmd: string, json?: any, force?: boolean): void;

	getDocType(): 'text' | 'presentation' | 'spreadsheet' | 'drawing';
	isText(): boolean;
	isPresentationOrDrawing(): boolean;

	getDocSize(): cool.Point;
	getSize(): cool.Point;
	getCenter(): LatLngLike;
	getContainer(): Element;
	_getCurrentFontName(): string;

	_docLoadedOnce: boolean;
	_debug: DebugManager;
	_fatal: boolean;
	_docPassword: string;

	options: {
		timestamp: number;
		doc: string;
		docParams: {
			access_token?: string;
			access_token_ttl?: string;
			no_auth_header?: string;
			permission?: 'edit' | 'readonly' | 'view';
		};
		renderingOptions: string;
		tileWidthTwips: number;
		tileHeightTwips: number;
		wopiSrc: string;
		previousWopiSrc: string;
		zoom: number;
		defaultZoom: number;
		crs: CRSInterface;
	};

	wopi: {
		resetAppLoaded(): void;
		DisableInactiveMessages: boolean;
		UserCanNotWriteRelative: boolean;
		IsOwner: boolean;
		BaseFileName: string;
		HideExportOption: boolean;
		DisableAISettings: boolean;
		AIConfigured: boolean;
		AIModelName: string;
		UserCanWrite: boolean;
		HideChangeTrackingControls: boolean;
		EnableRemoteLinkPicker: boolean;
		HideSaveOption: boolean;
	};

	loadDocument(socket?: SockInterface): void;
	getCurrentPartNumber(): number;
	getZoom(): number;
	showBusy(label: string, bar: boolean): void;
	hideBusy(): void;

	_clip: ClipboardInterface;

	setPermission(permission: string): void;
	onLockFailed(reason: string): void;
	updateModificationIndicator(newModificationTime: string): void;
	isEditMode(): boolean;
	isReadOnlyMode(): boolean;
	remove(): MapInterface;

	welcome: WelcomeInterface;
	_setLockProps(lockInfo: ParsedJSONResult): void;
	_setRestrictions(restrictionInfo: ParsedJSONResult): void;
	hideRestrictedItems(it: any, item: any, button: any): void;
	disableLockedItem(it: any, item: any, button: any): void;
	isLockedItem(data: any): boolean;
	openUnlockPopup(cmd: ControlCommand): void;
	isLockedUser(): boolean;
	isRestrictedUser(): boolean;

	focus(acceptInput?: boolean): void;
	editorHasFocus(): boolean;

	_fireInitComplete(condition: string): void;
	sendInitUNOCommands(): void;
	initTextInput(docType: string): void;
	saveAs(filenme: string, format?: string, options?: string): void;

	addControl(control: any): void;
	removeControl(control: any): void;

	_shouldStartReadOnly(): boolean;
	_switchToEditMode(): void;

	_permission: 'edit' | 'readonly' | 'view';

	toolbarUpTemplate: any;
	menubar: Menubar;
	userList: UserList;
	sidebar: Sidebar;
	getViewColor(viewId: number): number;

	// TODO fix types:
	jsdialog: any;
	zotero: any;

	_cacheSVG: string[];
	calcInputBarHasFocus(): boolean;
	lockAccessibilityOn(): void;
	getPixelBounds(center?: LatLngLike, zoom?: number): cool.Bounds;
	getPixelBoundsCore(center?: LatLngLike, zoom?: number): cool.Bounds;
	_partsDirection: number;
	getZoomScale(toZoom: number, fomZoom?: number): number;
	_docLoaded: boolean;
	contextToolbar?: ContextToolbar;
	getSplitPanesContext(): cool.SplitPanesContext | undefined;
	getScaleZoom(scale: number, fromZoom?: number): number;
	scrollingIsHandled: boolean;
	showComments(on?: boolean): void;
	showResolvedComments(on?: boolean): void;
	navigator: NavigatorPanel;
	setPart(
		part: number | string,
		external?: boolean,
		calledFromSetPartHandler?: boolean,
	): void;
	mouseEventToLatLng(e: any): LatLngLike;
	_limitZoom(zoom: number): number;
	setView(
		center: [number, number],
		zoom?: number,
		reset?: boolean,
	): MapInterface;
	isViewReadOnly(viewid: number): boolean;
	scrollHandler: typeof window.L.Map.Scroll;
	context?: { appId: string; context: string };
	eSignature?: cool.ESignature;

	// TODO: Fix type.
	formulabar: any;
	backstageView: any;
	topToolbar: any;
	statusBar: any;

	mobileSearchBar: MobileSearchBar;
	_disableDefaultAction: Record<string, boolean>;
	save(
		dontTerminateEdit: boolean,
		dontSaveIfUnmodified: boolean,
		extendedData?: string,
	): void;
	_everModified: boolean;
	print(options?: string): void;
	openRevisionHistory(): void;
	openShare(): void;
	showHelp(id: string): void;
	openSaveAs(format?: string): void;
	downloadAs(
		name: string,
		format?: string,
		options?: string | null,
		id?: string,
	): void;
	insertComment(): void;
	insertThreadedComment(): void;
	zoomIn(delta: number, options?: any, animate?: boolean): MapInterface;
	zoomOut(delta: number, options?: any, animate?: boolean): MapInterface;
	cancelSearch(): void;
	goToPage(page: string): void;
	serverAuditDialog?: ServerAuditDialog;
	_lockAccessibilityOn: boolean;
	setAccessibilityState(enable: boolean): void;

	// TODO: L.Map.Keyboard
	keyboard: any;

	onFormulaBarBlur(): void;
	onFormulaBarFocus(): void;
	formulabarBlur(): void;
	formulabarFocus(): void;
	formulabarSetDirty(): void;
	_functionWizardData: WidgetJSON;

	insertPage: ((nPos?: number) => void) & { scrollToEnd: boolean };
	deletePage(nPos?: number): void;
	duplicatePage(pos?: number): void;
	slideShowPresenter?: SlideShowPresenter;
	hideSlide(): void;
	showSlide(): void;
	sidebarFromNotebookbar: SidebarFromNotebookbarPanel;
	mobileTopBar?: MobileTopBar;

	// TODO: window.L.control.lokDialog
	dialog: any;
	isAIConfigured?: boolean;
	aiRequestTimeout?: number;
	aiModelName?: string;
	aiEthicalRating?: string;

	_controlCorners: Record<string, Node>;
	_contextMenu: ContextMenuControl;
	_saveImageToWopi: boolean;
}
