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
 * Definitions.Types - types and interfaces for JSDialog
 */

// common for all widgets
interface WidgetJSON {
	id: string; // unique id of a widget
	type: string; // type of widget
	enabled?: boolean; // enabled state
	visible?: boolean; // visibility state
	userHidden?: boolean; // hidden by user preference (separate from doc-type preferences)
	children?: Array<WidgetJSON>; // child nodes
	title?: string;
	text?: string; // TODO: remove, its for not yet defined widget types
	editText?: string; // text content from WeldEditView (EditEngine)
	tooltip?: string; // tooltip text (QuickHelpText from VCL)
	top?: string; // placement in the grid - row
	left?: string; // placement in the grid - column
	width?: string; // inside grid - width in number of columns
	labelledBy?: string | string[];
	allyRole?: string;
	accessibility?: NotebookbarAccessibilityDescriptor;
	aria?: AriaLabelAttributes; // ARIA Label attributes
	ariaLive?: 'polite' | 'assertive' | 'off';
	gridKeyboardNavigation?: boolean; // receives keyboard navigation for elements in col/rows
}

interface JSBuilderOptions {
	cssClass: string; // class added to every widget root
	windowId: number; // window id to be sent with dialogevent
	map: MapInterface; // reference to map
	mobileWizard: JSDialogComponent; // reference to the parent component FIXME: rename
	useSetTabs?: boolean; // custom tabs placement handled by the parent container
	useScrollAnimation?: boolean; // do we use animation for scrollIntoView

	// modifiers
	noLabelsForUnoButtons?: boolean; // create only icon without label
	useInLineLabelsForUnoButtons?: boolean; // create labels next to the icon
	suffix: string; // add a suffix to the element ID to make it unique among different builder instances.
}

interface JSBuilder {
	_currentDepth: number; // mobile-wizard only FIXME: encapsulate
	_responses: any;

	_unoToolButton: UnoToolButtonHandler; // special handler which returns toolitem object
	_controlHandlers: { [key: string]: JSWidgetHandler }; // handlers for widget types
	_menus: Map<string, Array<MenuDefinition>>;

	options: JSBuilderOptions; // current state
	map: MapInterface; // reference to map
	rendersCache: any; // on demand content cache
	wizard: any;
	windowId?: WindowId | number;

	build: (
		parentContainer: Element,
		data: WidgetJSON[],
		hasVerticalParent: boolean,
	) => boolean;
	updateWidget: (parentContainer: Element, updateJSON: any) => void;
	executeAction: (parentContainer: Element, actionJSON: any) => void;
	callback: JSDialogCallback;
	_defaultCallbackHandlerSendMessage: JSDialogCallback;
	_defaultCallbackHandler: JSDialogCallback;
	postProcess: (parentContainer: Element, data: WidgetJSON) => void;
	setWindowId: (id: WindowId | number) => void;
	onCommandStateChanged: (event: any) => void;

	// helpers FIXME: put as local in Control.Containers.ts
	_getGridColumns: (data: WidgetJSON[]) => number;
	_getGridRows: (data: WidgetJSON[]) => number;
	_preventDocumentLosingFocusOnClick: (container: Element) => void;
	_cleanText: (text: string) => string;
	_setAccessKey: (element: HTMLElement, key: string) => void;
	_getAccessKeyFromText: (text: string) => string;
	_stressAccessKey: (element: HTMLElement, accessKey: string) => void;
	_expanderHandler: any; // FIXME: use handlers getter instead
	_unitToVisibleString: (unit: string) => string;
}

// widget handler, returns true if child nodes should be still processed by the builder
type JSWidgetHandler = (
	parentContainer: Element,
	data: WidgetJSON,
	builder: JSBuilder,
	customCallback?: () => void,
) => boolean;

type UnoToolButtonHandler = (
	parentContainer: Element,
	data: WidgetJSON,
	builder: JSBuilder,
) => {
	container: HTMLElement;
	button: HTMLElement;
	label: HTMLElement;
	arrow?: HTMLElement;
};

// callback triggered by user actions
type JSDialogCallback = (
	objectType: string,
	eventType: string,
	object: any,
	data: any,
	builder: JSBuilder,
) => void;

type JSDialogCallbackConsumer = (
	objectType: string,
	eventType: string,
	object: any,
	data: any,
	builder: JSBuilder,
) => boolean;

type JSDialogMenuCallback = (
	objectType: string,
	eventType: string,
	object: any,
	data: any,
	entry: JSBuilder | MenuDefinition,
) => boolean;

interface DialogResponse {
	id: string;
	response: number;
}

interface ActionData {
	control_id: string;
	action_type: string;
	data: any;
	new_id?: string;
}

// JSDialog message (full, update or action)
interface JSDialogJSON extends WidgetJSON {
	id: string; // unique windowId
	jsontype: string; // specifies target component, on root level only
	action?: string; // optional name of an action
	control?: WidgetJSON;
	data?: ActionData;
	init_focus_id?: string; // id of initially focused widget
}

// JSDialog message for popup
interface PopupData extends JSDialogJSON {
	isAutoCompletePopup?: boolean;
	isAutoFillPreviewTooltip?: boolean;
	cancellable?: boolean;
	hasClose: boolean;
	noOverlay: boolean;
	popupParent?: string;
	clickToClose?: string;
	persistKeyboard?: boolean;
	posx: number;
	posy: number;
}

interface DialogJSON extends JSDialogJSON {
	dialogid: string; // unique id for a dialog type, not instance
	collapsed?: boolean; // if dialog is in collapsed mode
	responses?: Array<DialogResponse>;
}

// Notebookbar

type NotebookbarAccessibilityDescriptor = {
	focusBack: boolean;
	combination: string;
	[language: string]: string | boolean | null | undefined; // language-specific combinations (e.g. 'de' for German)
};

type NotebookbarTabEntry = {
	id: string;
	text: string; // visible in the UI
	name: string; // identifier for tab widget
	accessibility: NotebookbarAccessibilityDescriptor;
};

type NotebookbarTabContent = Array<WidgetJSON>;

interface NotebookbarTab {
	getName: () => string;
	getEntry: () => NotebookbarTabEntry;
	getContent: () => NotebookbarTabContent;
	onCallback?: JSDialogCallbackConsumer;
}

// callback triggered for custom rendered entries
type CustomEntryRenderCallback = (pos: number | string) => void;

// used to define menus
interface MenuDefinition extends WidgetJSON {
	type:
		| 'action'
		| 'colorpicker'
		| 'menu'
		| 'separator'
		| 'html'
		| 'json'
		| 'comboboxentry'; // type of entry
	text?: string; // displayed text
	hint?: string; // hint text
	uno?: string; // uno command
	action?: string; // dispatch command
	htmlId?: string; // DEPRECATED: id of HTMLContent - used in html type
	content?: WidgetJSON; // custom content - used in json type
	img?: string; // icon name
	icon?: string; // icon name FIXME: duplicated property, used in exportMenuButton
	checked?: boolean; // state of check mark
	items?: Array<any>;
	selected?: boolean; // selected state for entry
	statusCommand?: string; // UNO command used to retrieve the status/value of the entry
	pos?: number | string; // identifier of an entry
}

interface HtmlContentJson extends WidgetJSON {
	htmlId: string;
	closeCallback?: EventListenerOrEventListenerObject;
	isReadOnlyMode?: boolean;
	canUserWrite?: boolean;
	text?: string;
}

type FunctionNameAlias = {
	en: string;
	de?: string;
	fr?: string;
	es?: string;
};

interface ContainerWidgetJSON extends WidgetJSON {
	layoutstyle?: string | 'start' | 'end'; // describes alignment of the elements
	vertical?: boolean; // is horizontal or vertical container
}

interface OverflowGroupWidgetJSON extends ContainerWidgetJSON {
	name: string; // visible name of a group
	icon?: string; // Optional icon name. Otherwise it will be guessed.
	more?: MoreOptions;
	nofold?: boolean; // for widgets which use overflowgroup just for the label
}
interface MoreOptions {
	command: string;
}
interface OverflowGroupContainer extends Element {
	foldGroup?: () => void;
	isCollapsed?: () => boolean;
	unfoldGroup?: () => void;
}

interface GridWidgetJSON extends ContainerWidgetJSON {
	cols: number; // number of grid columns
	rows: number; // numer of grid rows
	tabIndex?: number;
	initialSelectedId?: string; // id of the first selected element
}

interface ToolboxWidgetJSON extends WidgetJSON {
	hasVerticalParent: boolean;
}

interface ToolItemWidgetJSON extends WidgetJSON {
	class?: string; // css class
	noLabel?: boolean;
	inlineLabel?: boolean;
	command?: string; // command to trigger options for a panel
	text?: string; // title to show or for tooltip
	icon?: string; // url to an svg
	postmessage?: boolean; // postmessage to WOPI in case the toolitem is added via postmessage
	beforeId?: string; // for added via postmessage, before which to put
	context?: string; // in which context we show the item
	desktop?: boolean; // do we show on desktop
	tablet?: boolean; // do we show on tablet
	mobile?: boolean; // do we show on mobile
	mobilebrowser?: boolean; // do we show on mobile in the browser
	iosapptablet?: boolean; // do we show in app on ios
	hidden?: boolean; // is hidden
	visible?: boolean; // is visible
	pressAndHold?: boolean; // for mobile
	w2icon?: string; // DEPRECATED: w2 icon name
	placeholder?: string; // DEPRECATED: w2 placeholder text
	items?: Array<ToolItemWidgetJSON>; // DEPRECATED: w2 menus
}

interface PanelWidgetJSON extends WidgetJSON {
	hidden: boolean; // is hidden
	command: string; // command to trigger options for a panel
	text: string; // panel title
	name?: string; // legacy panel id
}

type ExpanderWidgetJSON = any;

// type: 'fixedtext'
interface TextWidget extends WidgetJSON {
	text: string;
	html?: string;
	labelFor?: string;
	labelForType?: string;
	style?: string;
	hidden?: boolean;
	renderAsStatic?: boolean;
}

// type: 'pushbutton'
interface PushButtonWidget extends WidgetJSON {
	symbol?: string;
	text?: string;
	image?: string;
	isToggle?: boolean;
	checked?: boolean;
	command?: string;
	hidden?: boolean; // todo: deprecate it in favor of WidgetJson.visible
}

// type: 'menubutton'
interface MenuButtonWidgetJSON extends WidgetJSON {
	menu?: Array<MenuDefinition>; // custom menu
	applyCallback?: () => void; // split button callback for left part
	class?: string;
	image?: string | boolean;
	accessKey?: string;
}

// type: 'buttonbox'
interface ButtonBoxWidget extends WidgetJSON {
	layoutstyle: string;
}

// type: 'listbox'
interface ListBoxWidget extends WidgetJSON {
	entries: Array<string>;
	selectedEntries?: Array<string> | Array<number>;
}

// type: 'radiobutton'
interface RadioButtonWidget extends WidgetJSON {
	text: string;
	image?: string; // replacement image
	group?: string; // identifier of radio group
	checked?: boolean;
	hidden?: boolean;
}

interface ComboBoxEntry extends MenuDefinition {
	customRenderer?: boolean; // can render custom preview
	selected?: boolean; // is selected
	comboboxId?: string; // used to reference parent
	pos: string | number; // identifier of an entry
	initialSelectedId?: string;
}

interface ComboBoxWidget extends WidgetJSON {
	text?: string;
	entries?: Array<string | number | ComboBoxEntry>;
	selectedCount?: number;
	selectedEntries?: Array<number>;
	command?: string;
	customEntryRenderer?: boolean;
}

interface TreeColumnJSON {
	text?: any;
	link?: string;
	collapsed?: string | boolean;
	expanded?: string | boolean;
	customEntryRenderer?: boolean; // has custom rendering enabled
	collapsedimage?: string;
	expandedimage?: string;
	editable?: boolean;
}

interface TreeEntryJSON {
	row: number | string; // unique id of the entry
	text: string; // deprecated: simple text for an entry
	state: boolean; // checked radio/checkbox or not
	enabled: boolean; // enabled entry or not
	selected: boolean; // is entry selected
	collapsed: boolean; // is entry collapsed
	ondemand: boolean; // has content to request
	columns: Array<TreeColumnJSON>; // entry data
	children: Array<TreeEntryJSON>;
}

interface TreeHeaderJSON {
	text: string;
	sortable: boolean; // can be sorted by column
	arrow?: 'up' | 'down'; // sorting arrow to show
	color?: string; // series color as hex string (RRGGBB) for color bar indicator
	headerName?: string; // series name for chart data table headers
}

interface TreeWidgetJSON extends WidgetJSON {
	text: string;
	singleclickactivate: boolean; // activates element on single click instead of just selection
	fireKeyEvents?: boolean; // do we sent key events to core
	hideIfEmpty?: boolean; // hide the widget if no entries available
	checkboxtype: string; // radio or checkbox
	draggable?: boolean; // indicates if we can drag entries to another treeview
	entries: Array<TreeEntryJSON>;
	headers: Array<TreeHeaderJSON>; // header columns
	highlightTerm?: string; // what, if any, entries are we highlighting?
	customEntryRenderer?: boolean;
	noSearchField?: boolean; // When true, the widget shouldn't have a search field added
	sortLocally?: boolean; // When true, the widget will run sort algorithm in JS instead of callback (lists only)
	role?: string; // ARIA role from core: 'tree', 'treegrid', 'listbox', or 'grid'
}

interface IconViewEntry {
	row: number | string; // unique id of the entry
	separator?: boolean; // is separator
	selected?: boolean; // is currently selected
	image: string; // base64 encoded image
	width?: number; // width in pixels; used for on demand rendering
	height?: number; // height in pixels; used for on demand rendering
	text: string; // label of an entry
	tooltip?: string; // tooltip of an entry
	ondemand?: boolean; // if true then we ignore image property and request it on demand (when shown)
}

interface IconViewJSON extends WidgetJSON {
	entries: Array<IconViewEntry>;
	singleclickactivate: boolean; // activates element on single click instead of just selection
	textWithIconEnabled: boolean; // To identify if we should add text below the icon or not.
	selectionmode: string; // single or multiple
}

interface IconViewListJSON extends WidgetJSON {
	children: Array<IconViewJSON>;
}

interface IconViewElement extends HTMLElement {
	requestRenders: (
		entry: IconViewEntry,
		placeholder: Element,
		entryContainer: Element,
	) => void;

	updateRenders: (pos: number) => void;

	updateRendersImpl: (pos: number, id: string, where: HTMLElement) => void;

	builderCallback: (
		objectType: string,
		eventType: string,
		entryData: any,
		builder: JSBuilder,
	) => void;
}

interface EditWidgetJSON extends WidgetJSON {
	placeholder: string; // show when empty
	text: string; // text value
	password: boolean; // is password field
	hidden: boolean; // is hidden, TODO: duplicate?
	changedCallback: any; // callback  for 'change' event
	widthInChars: number; // width hint in characters
}

// type: 'checkbox'
interface CheckboxWidgetJSON extends WidgetJSON {
	text: string;
	command?: string; // used to just execute uno command or dispatch command instead of sending message
	checked?: boolean; // checkbox state
	hidden?: boolean;
}

interface AriaLabelAttributes {
	label?: string;
	description?: string;
	role?: string;
}

interface SeparatorWidgetJSON extends WidgetJSON {
	orientation: 'horizontal' | 'vertical';
}
