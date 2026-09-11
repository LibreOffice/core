/* -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* global app JSDialog _ _n */

/*
 * Control.CleanupSidebar - the Cleanup deck.
 *
 * The deck is a JSDialog component of its own that borrows the sidebar dock. It builds with a
 * builder of its own, keeps its own model, and answers the widget events of its own widgets, so
 * none of them travels to a window in the kit. The dock says when its content changes hands, and
 * the deck stands down when it does.
 *
 * The panel holds the state of the deck and turns it into widget JSON: the whole deck when the
 * dock opens, and the region that changed when an event arrives.
 *
 * The panel is a view of a run that lives in the kit. Every request travels as one
 * .uno:PresentationCleanup command carrying a JSON object, and every reply and every event of the
 * run comes back as the map event commandresult under that same command name.
 */

declare var JSDialog: any;

/// The five kinds of finding, in the order the kit lists them.
type CleanupCategory =
	| 'image'
	| 'hiddenSlide'
	| 'unusedMaster'
	| 'notes'
	| 'embeddedObject';

/// One finding, as the kit describes it. The kit sends figures rather than sentences, and
/// the panel writes the row from them in the language of this view.
interface CleanupRow {
	id: number;
	category: CleanupCategory;
	/// Zero-based number of the slide, and -1 for a finding about a master slide or about
	/// the document as a whole.
	slide: number;
	/// Which image of its slide this one is, counting from one, and zero for an image that
	/// stands alone for its slide.
	imageNumber: number;
	/// Dots per inch the image is stored at, and zero when there is no such figure.
	dpi: number;
	/// The share of the picture a crop hides, from zero to a hundred.
	hiddenPercent: number;
	/// What the finding is about by name, such as a master slide, and empty when the row
	/// names no such thing.
	name: string;
	currentBytes: number;
	saving: number;
	measured: boolean;
	canGoTo: boolean;
	canFix: boolean;
}

/// The three values a scan is run with. resolution is a target in dots per inch, zero
/// meaning images are left as they are; quality is 1 to 100; forPublication asks for the
/// cleanups that prepare a presentation to be handed out.
interface CleanupOptions {
	resolution: number;
	quality: number;
	forPublication: boolean;
}

/// How far the measuring of the images has come, as two counts of images.
interface CleanupMeasured {
	done: number;
	total: number;
}

/// What the whole list comes to. estimated and saved are byte figures, count is how many
/// rows there are and fixable how many of them offer a cleanup.
interface CleanupTotal {
	estimated?: number;
	saved?: number;
	count?: number;
	fixable?: number;
}

/// One event of the run, as it arrives from the kit. The fields the kit fills with a
/// container or with the values of a scan arrive unchecked and of unknown shape.
interface CleanupResult {
	event: string;
	request?: number;
	run?: number;
	ok?: boolean;
	reason?: string;
	status?: string;
	options?: unknown;
	measured?: unknown;
	/// A whole list on the list event and a patch of the rows on screen on the others.
	rows?: unknown;
	/// The ids of the rows the kit has taken off the list, and of the rows a cleanup
	/// dealt with.
	dropped?: unknown;
	removed?: unknown;
	/// The whole list on every event but progress, where it is the number of steps of
	/// the phase.
	total?: CleanupTotal | number;
	phase?: string;
	done?: number;
	stopped?: boolean;
	cancelled?: boolean;
	saved?: number;
	rescan?: boolean;
}

/// What an event says about a row that is already on screen. Only the id is always there.
interface CleanupRowPatch {
	id: number;
	imageNumber?: number;
	saving?: number;
}

/// Everything the deck draws. The JSON of the deck is a function of this alone, so the
/// panel changes it and then asks for the regions that changed.
interface CleanupPanelState {
	readOnly: boolean;
	busy: boolean;
	/// True once the kit has handed over a list of a scan, whether or not it holds
	/// rows.
	hasList: boolean;
	statusName: string;
	measuredDone: number;
	measuredTotal: number;
	savedBytes: number;
	estimatedBytes: number;
	rowCount: number;
	fixableCount: number;
	/// The values the last scan was run with, which the headings of the list stand
	/// under.
	options: CleanupOptions;
	/// The values the option controls hold, which the next scan is run with.
	chosenOptions: CleanupOptions;
	/// The rows of the list in the order the kit gave them, by id.
	rows: Map<number, CleanupRow>;
	/// The rows a cleanup dealt with that wait, dimmed, for the scan that follows it.
	goneRows: Set<number>;
	/// The groups the reader folded shut.
	foldedGroups: Set<CleanupCategory>;
}

const CLEANUP_CATEGORIES: CleanupCategory[] = [
	'image',
	'hiddenSlide',
	'unusedMaster',
	'notes',
	'embeddedObject',
];

/// The part of a group's widget ids that names its category.
const CLEANUP_GROUP_SLUG: { [key in CleanupCategory]: string } = {
	image: 'image',
	hiddenSlide: 'hidden-slide',
	unusedMaster: 'unused-master',
	notes: 'notes',
	embeddedObject: 'embedded-object',
};

/// The targets a scan can be asked for, in the order the list offers them. Zero leaves the
/// images as they are.
const CLEANUP_RESOLUTIONS = [0, 96, 150, 220, 300];

/// The ids of the widgets the panel refreshes one at a time. A group is named by its
/// category and a row by its id, see groupWidgetId and rowWidgetId.
const CleanupWidgetId = {
	deck: 'cleanup-deck',
	options: 'cleanup-options',
	resolutionLabel: 'cleanup-resolution-label',
	resolution: 'cleanup-resolution',
	qualityLabel: 'cleanup-quality-label',
	quality: 'cleanup-quality',
	sharing: 'cleanup-sharing',
	run: 'cleanup-run',
	scan: 'cleanup-scan',
	status: 'cleanup-status',
	statusLine: 'cleanup-status-line',
	statusText: 'cleanup-status-text',
	progressLine: 'cleanup-progress-line',
	progressFigures: 'cleanup-progress-figures',
	progress: 'cleanup-progress',
	stop: 'cleanup-stop',
	summary: 'cleanup-summary',
	total: 'cleanup-total',
	fixAll: 'cleanup-fix-all',
	separator: 'cleanup-separator',
	message: 'cleanup-message',
};

const CLEANUP_GROUP_PREFIX = 'cleanup-group-';
const CLEANUP_ROW_PREFIX = 'cleanup-row-';

function groupWidgetId(category: CleanupCategory): string {
	return CLEANUP_GROUP_PREFIX + CLEANUP_GROUP_SLUG[category];
}

function rowWidgetId(rowId: number): string {
	return CLEANUP_ROW_PREFIX + String(rowId);
}

/// The category a group widget id names, and null for any other id.
function categoryOfGroupWidgetId(id: string): CleanupCategory | null {
	for (const category of CLEANUP_CATEGORIES)
		if (id === groupWidgetId(category)) return category;

	return null;
}

/// The row a widget id of the form cleanup-row-<id><suffix> names, and null for any other
/// id.
function rowOfWidgetId(id: string, suffix: string): number | null {
	if (!id.startsWith(CLEANUP_ROW_PREFIX) || !id.endsWith(suffix)) return null;

	const digits = id.slice(CLEANUP_ROW_PREFIX.length, id.length - suffix.length);
	if (!/^\d+$/.test(digits)) return null;

	return parseInt(digits, 10);
}

/// A state with no list, the way the panel starts and the way it is left when the deck
/// closes.
function newCleanupPanelState(): CleanupPanelState {
	return {
		readOnly: false,
		busy: false,
		hasList: false,
		statusName: 'idle',
		measuredDone: 0,
		measuredTotal: 0,
		savedBytes: 0,
		estimatedBytes: 0,
		rowCount: 0,
		fixableCount: 0,
		options: { resolution: 150, quality: 80, forPublication: false },
		chosenOptions: { resolution: 150, quality: 80, forPublication: false },
		rows: new Map<number, CleanupRow>(),
		goneRows: new Set<number>(),
		foldedGroups: new Set<CleanupCategory>(),
	};
}

/// Says how many bytes a figure stands for: whole bytes under a kilobyte, whole kilobytes
/// under a megabyte, and megabytes to one decimal place with a period, whatever the
/// language of the reader.
function formatByteSize(bytes: number): string {
	const oneKilobyte = 1024;
	const oneMegabyte = 1024 * 1024;

	if (bytes < oneKilobyte) {
		const wholeBytes = Math.round(bytes);
		return _n('%1 byte', '%1 bytes', wholeBytes).replace(
			'%1',
			String(wholeBytes),
		);
	}

	if (bytes < oneMegabyte) {
		const kilobytes = Math.floor((bytes + oneKilobyte / 2) / oneKilobyte);
		return _('%1 KB').replace('%1', String(kilobytes));
	}

	const tenths = Math.floor((bytes * 10 + oneMegabyte / 2) / oneMegabyte);
	const megabytes = String(Math.floor(tenths / 10)) + '.' + String(tenths % 10);
	return _('%1 MB').replace('%1', megabytes);
}

/// The label a screen reader hears on the Fix button of a row, naming the row it belongs
/// to.
function fixLabelFor(rowText: string): string {
	return _('Fix %1').replace('%1', rowText);
}

function isFiniteNumber(value: unknown): value is number {
	return typeof value === 'number' && isFinite(value);
}

/// The entries a payload field holds, and an empty list when the field holds anything but a
/// list.
function asArray(value: unknown): unknown[] {
	return Array.isArray(value) ? value : [];
}

/// The numbers a payload field holds, with every entry that is not a number left out.
function asNumberArray(value: unknown): number[] {
	return asArray(value).filter(isFiniteNumber);
}

/// True for a whole set of scan values: the two figures are numbers and the choice is a
/// boolean.
function isCleanupOptions(value: unknown): value is CleanupOptions {
	if (!value || typeof value !== 'object') return false;

	const options = value as CleanupOptions;
	return (
		isFiniteNumber(options.resolution) &&
		isFiniteNumber(options.quality) &&
		typeof options.forPublication === 'boolean'
	);
}

/// How far the measuring has come, as the panel reads it from a payload. A count that is
/// not a number counts as zero.
function asMeasuredProgress(value: unknown): CleanupMeasured {
	if (!value || typeof value !== 'object') return { done: 0, total: 0 };

	const measured = value as CleanupMeasured;
	return {
		done: isFiniteNumber(measured.done) ? measured.done : 0,
		total: isFiniteNumber(measured.total) ? measured.total : 0,
	};
}

/// The line that names where a finding sits. An image says which slide it is on, and which
/// image of that slide it is when the slide has more than one to tell apart. A slide number
/// of -1 stands for a master slide.
function rowTextFor(row: CleanupRow): string {
	const slide = String(row.slide + 1);

	switch (row.category) {
		case 'image':
			if (row.slide < 0)
				return row.imageNumber <= 0
					? _('Master slide')
					: _('Master slide, image %1').replace('%1', String(row.imageNumber));

			return row.imageNumber <= 0
				? _('Slide %1').replace('%1', slide)
				: _('Slide %1, image %2')
						.replace('%1', slide)
						.replace('%2', String(row.imageNumber));
		case 'hiddenSlide':
			return _('Slide %1 is hidden and can be removed').replace('%1', slide);
		case 'unusedMaster':
			return _("Master slide '%1' is not used by any slide").replace(
				'%1',
				row.name,
			);
		case 'notes':
			return _('Slide %1 has speaker notes').replace('%1', slide);
		case 'embeddedObject':
			return row.canFix
				? _('Slide %1: embedded object').replace('%1', slide)
				: _('Slide %1: embedded object, no image to replace it with').replace(
						'%1',
						slide,
					);
	}
}

/// The line under that one, which says what the document spends on an image and what makes
/// it worth a row. A finding with nothing more to say has none.
function rowDetailFor(row: CleanupRow): string {
	const size = formatByteSize(row.currentBytes);

	if (row.category === 'image') {
		if (row.currentBytes === 0)
			return _('%1 DPI').replace('%1', String(row.dpi));

		return _('%1 at %2 DPI').replace('%1', size).replace('%2', String(row.dpi));
	}

	return '';
}

/// True for a whole finding the panel can draw a row from.
function isCleanupRow(value: unknown): value is CleanupRow {
	if (!value || typeof value !== 'object') return false;

	const row = value as CleanupRow;
	return (
		isFiniteNumber(row.id) &&
		isFiniteNumber(row.slide) &&
		isFiniteNumber(row.imageNumber) &&
		isFiniteNumber(row.currentBytes) &&
		isFiniteNumber(row.saving) &&
		CLEANUP_CATEGORIES.indexOf(row.category) !== -1
	);
}

/// True for a patch of a row already on screen: an id, and each figure it carries of the
/// right kind. A patch names only what changed, so a field that is missing is not a fault.
function isCleanupRowPatch(value: unknown): value is CleanupRowPatch {
	if (!value || typeof value !== 'object') return false;

	const patch = value as CleanupRowPatch;
	if (!isFiniteNumber(patch.id)) return false;
	if (patch.imageNumber !== undefined && !isFiniteNumber(patch.imageNumber))
		return false;
	if (patch.saving !== undefined && !isFiniteNumber(patch.saving)) return false;

	return true;
}

/// True for the actions that put the panel into its busy state: scan, fix and fixAll. goTo
/// and list are allowed while a run is going and their refusal says nothing about that run.
function refusalEndsBusyState(action: string | undefined): boolean {
	return action === 'scan' || action === 'fix' || action === 'fixAll';
}

/// True while the event belongs to a list the panel is not showing. An event carrying a run
/// while the panel has none is stale as well, unless the panel has asked for a list and is
/// waiting to hear which run it now has.
function isStaleEvent(
	result: CleanupResult,
	run: number | null,
	listRequested: boolean,
): boolean {
	if (typeof result.run !== 'number') return false;
	if (run === null) return !listRequested;

	return result.run !== run;
}

// --- the deck as JSON ------------------------------------------------------------------

function resolutionTexts(): string[] {
	return [
		_('Leave images alone'),
		_('96 DPI'),
		_('150 DPI'),
		_('220 DPI'),
		_('300 DPI'),
	];
}

/// The place of a target in the list of resolutions, and the place of the default when the
/// target is not one the list offers.
function resolutionIndex(resolution: number): number {
	const index = CLEANUP_RESOLUTIONS.indexOf(resolution);
	return index === -1 ? CLEANUP_RESOLUTIONS.indexOf(150) : index;
}

function rowsOf(
	state: CleanupPanelState,
	category: CleanupCategory,
): CleanupRow[] {
	const rows: CleanupRow[] = [];
	for (const row of state.rows.values())
		if (row.category === category) rows.push(row);
	return rows;
}

function statusTextFor(status: string): string {
	switch (status) {
		case 'scanning':
			return _('Scanning...');
		case 'measuring':
			return _('Measuring images...');
		case 'preparing':
			return _('Preparing cleanup...');
		case 'cleaning':
			return _('Cleaning up...');
	}
	return '';
}

function headingFor(
	state: CleanupPanelState,
	category: CleanupCategory,
	count: number,
): string {
	switch (category) {
		case 'image':
			return _('Images over %1 DPI (%2)')
				.replace('%1', String(state.options.resolution))
				.replace('%2', String(count));
		case 'hiddenSlide':
			return _('Hidden slides (%1)').replace('%1', String(count));
		case 'unusedMaster':
			return _('Unused master slides (%1)').replace('%1', String(count));
		case 'notes':
			return _('Speaker notes (%1)').replace('%1', String(count));
		case 'embeddedObject':
			return _('Embedded objects (%1)').replace('%1', String(count));
	}
}

/// What the band under the list says the whole list comes to, and nothing while it has no
/// figure and no count to give.
function totalTextFor(state: CleanupPanelState): string {
	if (state.savedBytes > 0)
		return _('Saved %1').replace('%1', formatByteSize(state.savedBytes));

	if (state.estimatedBytes > 0)
		return _('Estimated saving: %1').replace(
			'%1',
			formatByteSize(state.estimatedBytes),
		);

	if (state.fixableCount > 0)
		return _n(
			'%1 thing to clean up',
			'%1 things to clean up',
			state.fixableCount,
		).replace('%1', String(state.fixableCount));

	return '';
}

/// The three option rows: a label and its control side by side, and the choice under them.
/// Each control names the label beside it, so the label alone is the name a screen reader
/// reads out for the control.
function optionsJSON(state: CleanupPanelState): GridWidgetJSON {
	const enabled = !state.busy;

	return {
		id: CleanupWidgetId.options,
		type: 'grid',
		cols: 2,
		rows: 3,
		visible: !state.readOnly,
		cssClass: 'cleanup-options',
		children: [
			{
				id: CleanupWidgetId.resolutionLabel,
				type: 'fixedtext',
				text: _('Image resolution'),
				labelFor: CleanupWidgetId.resolution,
				labelForType: 'listbox',
				left: '0',
				top: '0',
			} as TextWidget,
			{
				id: CleanupWidgetId.resolution,
				type: 'listbox',
				labelledBy: CleanupWidgetId.resolutionLabel,
				entries: resolutionTexts(),
				selectedEntries: [
					String(resolutionIndex(state.chosenOptions.resolution)),
				],
				enabled: enabled,
				hexpand: true,
				left: '1',
				top: '0',
			} as ListBoxWidget,
			{
				id: CleanupWidgetId.qualityLabel,
				type: 'fixedtext',
				text: _('JPEG quality in %'),
				labelFor: CleanupWidgetId.quality,
				labelForType: 'spinfield',
				left: '0',
				top: '1',
			} as TextWidget,
			{
				id: CleanupWidgetId.quality,
				type: 'spinfield',
				labelledBy: CleanupWidgetId.qualityLabel,
				// The field shows its text and reads its bounds from the
				// figures.
				text: String(state.chosenOptions.quality),
				value: state.chosenOptions.quality,
				min: 1,
				max: 100,
				step: 5,
				enabled: enabled,
				hexpand: true,
				left: '1',
				top: '1',
			} as SpinFieldWidgetJSON,
			{
				id: CleanupWidgetId.sharing,
				type: 'checkbox',
				text: _('Prepare for sharing'),
				checked: state.chosenOptions.forPublication,
				enabled: enabled,
				left: '0',
				top: '2',
				width: '2',
			} as CheckboxWidgetJSON,
		],
	};
}

/// The phase the run is in. It stands in a line that is read out as it changes, so it is
/// sent on its own: a phase is announced once, when it is new.
function statusTextJSON(state: CleanupPanelState): TextWidget {
	return {
		id: CleanupWidgetId.statusText,
		type: 'fixedtext',
		text: statusTextFor(state.statusName),
	} as TextWidget;
}

/// How far the run has come: a share of the whole for the eye and the two counts beside it.
/// The counts move with every step, so they stand outside the line that is read out.
function progressLineJSON(state: CleanupPanelState): ContainerWidgetJSON {
	const total = state.measuredTotal;
	const done = Math.min(state.measuredDone, total);
	const figures =
		total === 0
			? ''
			: _('%1 of %2').replace('%1', String(done)).replace('%2', String(total));

	return {
		id: CleanupWidgetId.progressLine,
		type: 'container',
		cssClass: 'cleanup-progress-line',
		children: [
			{
				id: CleanupWidgetId.progress,
				type: 'progressbar',
				value: done,
				maxValue: total === 0 ? 100 : total,
				infinite: total === 0,
				aria: { label: _('Cleanup progress') },
			} as ProgressBarWidgetJSON,
			{
				id: CleanupWidgetId.progressFigures,
				type: 'fixedtext',
				text: figures,
				visible: figures !== '',
				cssClass: 'cleanup-progress-figures',
			} as TextWidget,
		],
	};
}

/// The status of the run that is going: the phase with Stop beside it, and the bar under
/// them.
function statusJSON(state: CleanupPanelState): ContainerWidgetJSON {
	return {
		id: CleanupWidgetId.status,
		type: 'container',
		vertical: true,
		visible: state.busy && !state.readOnly,
		cssClass: 'cleanup-status',
		children: [
			{
				id: CleanupWidgetId.statusLine,
				type: 'container',
				ariaLive: 'polite',
				cssClass: 'cleanup-status-line',
				children: [
					statusTextJSON(state),
					{
						id: CleanupWidgetId.stop,
						type: 'pushbutton',
						text: _('Stop'),
					} as PushButtonWidget,
				],
			} as ContainerWidgetJSON,
			progressLineJSON(state),
		],
	};
}

/// One place over the list holds either the button that starts a run or the status of the
/// run that is going.
function runJSON(state: CleanupPanelState): ContainerWidgetJSON {
	return {
		id: CleanupWidgetId.run,
		type: 'container',
		vertical: true,
		cssClass: 'cleanup-run',
		children: [
			{
				id: CleanupWidgetId.scan,
				type: 'pushbutton',
				text: _('Scan'),
				enabled: !state.busy,
				visible: !state.busy && !state.readOnly,
			} as PushButtonWidget,
			statusJSON(state),
		],
	};
}

/// The band under the list says what the whole list comes to. It waits for the run to
/// settle rather than reporting a figure that is still moving, and stands there only while
/// it has something in it: a figure, a count, or the button that deals with the whole list.
function summaryJSON(state: CleanupPanelState): ContainerWidgetJSON {
	const total = totalTextFor(state);
	const offersFixAll = !state.readOnly && state.fixableCount > 0;

	return {
		id: CleanupWidgetId.summary,
		type: 'container',
		ariaLive: 'polite',
		visible: !state.busy && !state.readOnly && (total !== '' || offersFixAll),
		cssClass: 'cleanup-summary',
		children: [
			{
				id: CleanupWidgetId.total,
				type: 'fixedtext',
				text: total,
				visible: total !== '',
				cssClass: 'cleanup-total',
			} as TextWidget,
			{
				id: CleanupWidgetId.fixAll,
				type: 'pushbutton',
				text: _('Fix all'),
				enabled: !state.busy,
				visible: offersFixAll,
			} as PushButtonWidget,
		],
	};
}

function separatorJSON(state: CleanupPanelState): SeparatorWidgetJSON {
	return {
		id: CleanupWidgetId.separator,
		type: 'separator',
		orientation: 'horizontal',
		visible: !state.readOnly,
	};
}

/// The panel says what the scan button is for until a list arrives, what the list came to
/// once one has, and why there is nothing to do when the document may not be changed.
function messageJSON(state: CleanupPanelState): TextWidget {
	let text: string;
	let visible = true;

	if (state.readOnly) {
		text = _(
			'This document cannot be changed here, so there is nothing to clean up.',
		);
	} else if (!state.hasList) {
		text = _(
			'Choose a target resolution, then press Scan to find what can be cleaned up.',
		);
	} else {
		text = _('Nothing to clean up.');
		visible = state.rowCount === 0;
	}

	return {
		id: CleanupWidgetId.message,
		type: 'fixedtext',
		text: text,
		visible: visible,
		ariaLive: 'polite',
		cssClass: 'cleanup-message',
	} as TextWidget;
}

/// A row: where the finding is, as a link when the view can go there; the figures it was
/// made on under that; how much it saves, once measured; and its Fix button, when it offers
/// one.
function rowJSON(state: CleanupPanelState, row: CleanupRow): GridWidgetJSON {
	const id = rowWidgetId(row.id);
	const text = rowTextFor(row);
	const detail = rowDetailFor(row);
	const children: WidgetJSON[] = [];

	if (row.canGoTo)
		children.push({
			id: id + '-goto',
			type: 'linkbutton',
			text: text,
			cssClass: 'cleanup-row-text',
			hexpand: true,
			left: '0',
			top: '0',
		} as WidgetJSON);
	else
		children.push({
			id: id + '-text',
			type: 'fixedtext',
			text: text,
			cssClass: 'cleanup-row-text',
			hexpand: true,
			left: '0',
			top: '0',
		} as TextWidget);

	if (detail !== '')
		children.push({
			id: id + '-detail',
			type: 'fixedtext',
			text: detail,
			cssClass: 'cleanup-row-detail',
			left: '0',
			top: '1',
		} as TextWidget);

	if (row.saving > 0)
		children.push({
			id: id + '-saving',
			type: 'fixedtext',
			text: _('saves %1').replace('%1', formatByteSize(row.saving)),
			cssClass: 'cleanup-row-saving',
			left: '1',
			top: '0',
		} as TextWidget);

	if (row.canFix)
		children.push({
			id: id + '-fix',
			type: 'pushbutton',
			text: _('Fix'),
			enabled: !state.busy,
			aria: { label: fixLabelFor(text) },
			left: '2',
			top: '0',
		} as PushButtonWidget);

	return {
		id: id,
		type: 'grid',
		cols: 3,
		rows: detail === '' ? 1 : 2,
		cssClass:
			'cleanup-row' + (state.goneRows.has(row.id) ? ' cleanup-row-gone' : ''),
		children: children,
	};
}

/// A group is a section that folds shut under a heading saying what kind of finding it
/// holds and how many. A group with nothing to report is off the screen.
function groupJSON(
	state: CleanupPanelState,
	category: CleanupCategory,
): ExpanderWidgetJSON {
	const id = groupWidgetId(category);
	const rows = rowsOf(state, category);
	const children: WidgetJSON[] = [
		{
			id: id + '-heading',
			type: 'fixedtext',
			text: headingFor(state, category, rows.length),
		} as TextWidget,
	];
	for (const row of rows) children.push(rowJSON(state, row));

	return {
		id: id,
		type: 'expander',
		expanded: !state.foldedGroups.has(category),
		visible: rows.length > 0,
		cssClass: 'cleanup-group',
		children: children,
	};
}

/// The JSON of one region of the deck, named by the id of its widget, and null for an id
/// that names no region. The regions are the widgets the panel sends on their own once the
/// deck is built: the options, the run, the band, the divider, the message, a group and a
/// row.
function cleanupRegionJSON(
	state: CleanupPanelState,
	id: string,
): WidgetJSON | null {
	switch (id) {
		case CleanupWidgetId.options:
			return optionsJSON(state);
		case CleanupWidgetId.run:
			return runJSON(state);
		case CleanupWidgetId.statusText:
			return statusTextJSON(state);
		case CleanupWidgetId.progressLine:
			return progressLineJSON(state);
		case CleanupWidgetId.summary:
			return summaryJSON(state);
		case CleanupWidgetId.separator:
			return separatorJSON(state);
		case CleanupWidgetId.message:
			return messageJSON(state);
	}

	const category = categoryOfGroupWidgetId(id);
	if (category) return groupJSON(state, category);

	const rowId = rowOfWidgetId(id, '');
	if (rowId !== null) {
		const row = state.rows.get(rowId);
		return row ? rowJSON(state, row) : null;
	}

	return null;
}

/// What a region looks like, for telling whether it has to be rebuilt. A group is its
/// heading and the rows it holds, not what those rows say: a row that changes is rebuilt on
/// its own, so a group is rebuilt only when a row joins or leaves it or its heading
/// changes.
function regionLook(json: WidgetJSON): string {
	if (json.type !== 'expander') return JSON.stringify(json);

	const children = json.children || [];
	return JSON.stringify(
		Object.assign({}, json, {
			children: children.map((child) => child.id),
			heading: children.length ? children[0].text : '',
		}),
	);
}

/// The whole deck: a heading row that says the title, and the regions under it.
function cleanupDeckJSON(state: CleanupPanelState): DeckWidgetJSON {
	const title = _('Clean Up');
	const children: WidgetJSON[] = [
		optionsJSON(state),
		runJSON(state),
		summaryJSON(state),
		separatorJSON(state),
		messageJSON(state),
	];
	for (const category of CLEANUP_CATEGORIES)
		children.push(groupJSON(state, category));

	return {
		id: CleanupWidgetId.deck,
		type: 'deck',
		enabled: true,
		visible: true,
		text: title,
		headerText: title,
		name: CleanupWidgetId.deck,
		children: children,
	};
}

/// The ids of the elements that take the focus: the builder names the button inside a push
/// button widget and the input inside a field with a suffix on the widget id.
function buttonElementId(widgetId: string): string {
	return widgetId + '-button';
}

function inputElementId(widgetId: string): string {
	return widgetId + '-input';
}

/// True for the element ids of the controls a run turns off: the options, the two buttons
/// that act on a list and the button of every row.
function goesDeadWhenBusy(elementId: string): boolean {
	if (
		elementId === buttonElementId(CleanupWidgetId.scan) ||
		elementId === buttonElementId(CleanupWidgetId.fixAll) ||
		elementId === inputElementId(CleanupWidgetId.resolution) ||
		elementId === inputElementId(CleanupWidgetId.quality) ||
		elementId === inputElementId(CleanupWidgetId.sharing)
	)
		return true;

	return rowOfWidgetId(elementId, '-fix-button') !== null;
}

/// Which fields of the panel state the deck's own widgets write, keyed by the widget event
/// that carries the new value. Returns true when the event was one of them.
function applyOptionEvent(
	options: CleanupOptions,
	objectType: string,
	eventType: string,
	id: string,
	data: unknown,
): boolean {
	if (
		objectType === 'combobox' &&
		eventType === 'selected' &&
		id === CleanupWidgetId.resolution
	) {
		// The listbox reports the place of the entry and its text, as 'index;text'.
		const index = parseInt(String(data), 10);
		if (index >= 0 && index < CLEANUP_RESOLUTIONS.length)
			options.resolution = CLEANUP_RESOLUTIONS[index];
		return true;
	}

	if (
		objectType === 'spinfield' &&
		eventType === 'change' &&
		id === CleanupWidgetId.quality
	) {
		const quality = Number(data);
		if (isFinite(quality))
			options.quality = Math.min(100, Math.max(1, Math.round(quality)));
		return true;
	}

	if (
		objectType === 'checkbox' &&
		eventType === 'change' &&
		id === CleanupWidgetId.sharing
	) {
		options.forPublication = data === true;
		return true;
	}

	return false;
}

class CleanupSidebar extends SidebarBase {
	private state = newCleanupPanelState();

	/// Request numbers increase for the life of the view, and the action of a request
	/// is kept until the event that finishes it arrives.
	private nextRequestId = 0;
	private requestActions = new Map<number, string>();

	/// The list the panel is showing, and null while it has none.
	private run: number | null = null;
	private shown = false;

	/// What each region looked like when it was last given to the sidebar, so a region
	/// whose state has not changed is not rebuilt.
	private lastSent = new Map<string, string>();

	/// The row whose Fix button was pressed, and the row under it that offers one, so
	/// that the focus has somewhere to go once the row it was on has left the list.
	private fixedRowId: number | null = null;
	private fixedRowCategory: CleanupCategory | null = null;
	private rowAfterFixedId: number | null = null;

	/// True from the moment the panel puts the focus on Stop for a run until it places
	/// it again at the end of that run, and the id of the element it took the focus
	/// from, which is empty while it holds none.
	private focusHeldForRun = false;
	private focusTakenFrom = '';

	/// True while the panel is listening for what its run reports.
	private listening = false;
	private onCommandResultBound = this.onCommandResult.bind(this);

	constructor(map: MapInterface) {
		super(map, SidebarType.Cleanup);
		this.map.on('dockcontent', this.onDockContent, this);
		app.events.on('updatepermission', () => this.onPermissionChanged());
	}

	onRemove() {
		super.onRemove();
		this.map.off('dockcontent', this.onDockContent, this);
	}

	/// The deck is built and driven in the browser, so the builder is told the window
	/// the other browser-driven decks of this dock use, and every widget event stops
	/// here.
	protected createBuilder() {
		this.builder = new window.L.control.jsDialogBuilder({
			mobileWizard: this,
			map: this.map,
			windowId: WindowId.Notebookbar,
			cssClass: 'jsdialog sidebar',
			useScrollAnimation: false,
			suffix: 'sidebar',
			callback: this.callback.bind(this),
		} as JSBuilderOptions);
	}

	/// The deck borrows the sidebar dock rather than having one of its own.
	protected setupContainer(parentContainer?: HTMLElement) {
		this.container = document.getElementById(
			'sidebar-container',
		) as HTMLElement;
		this.wrapper = document.getElementById(
			'sidebar-dock-wrapper',
		) as HTMLElement;
		this.documentContainer = document.querySelector(
			'#document-container',
		) as HTMLDivElement;
	}

	/// Turns the deck on or off. Once the dock holds the deck, the panel asks the kit
	/// for the state of the run.
	public toggle(): void {
		if (this.shown) {
			this.closeSidebar();
			return;
		}

		// The deck is built from the state it opens in, so nothing has to follow on
		// its heels.
		this.state.readOnly = this.map.isReadOnlyMode();
		this.lastSent.clear();

		this.takeOverDock();
		this.buildDeck();
		this.showSidebar();
		this.updatePresentationDeckHighlight(CleanupWidgetId.deck);

		this.listenToMap();

		if (this.state.readOnly) return;

		this.sendRequest('list');
	}

	/// Empties the dock for the deck. A core deck showing there is closed in the kit as
	/// well, so the deck commands keep toggling in step with what the user sees.
	private takeOverDock(): void {
		// isVisible is true exactly while a core deck fills the dock
		if (this.isVisible()) app.socket.sendMessage('uno .uno:SidebarHide');
		this.closeSidebar();
	}

	/// Puts the deck in the dock, built with this component's own builder, and takes the
	/// content of the dock over so that patches meant for another component pass it by.
	private buildDeck(): void {
		if (!this.builder || !this.container) return;

		this.model.fullUpdate({
			// The container the deck sits in. It carries a name of its own rather than the
			// deck's, so that an id names one thing.
			id: SidebarType.Cleanup,
			jsontype: SidebarType.Cleanup,
			type: 'container',
			dialogid: '0',
			children: [cleanupDeckJSON(this.state)],
		} as any as JSDialogJSON);
		this.markContainerContentOwner();

		// The deck is built away from the page and put in once it is whole.
		const fragment = new DocumentFragment();
		const temporary = window.L.DomUtil.create('div', '', fragment);
		this.builder.build(temporary, [this.model.getSnapshot()], false);

		const container = this.container;
		app.layoutingService.appendLayoutingTask(() =>
			container.replaceChildren(temporary.firstChild),
		);
		this.onResize();
	}

	/// Draws one widget of the deck again from new JSON. The model takes the new state
	/// as well, so a rebuild of the whole deck draws the same thing. True when it was
	/// drawn.
	private updateWidget(control: WidgetJSON): boolean {
		if (!this.shown || !this.builder || !this.container) return false;

		this.model.widgetUpdate(control);
		this.builder.updateWidget(this.container, control);
		return true;
	}

	/// The dock this deck borrows now holds something else, or nothing at all. Every way
	/// the deck can go ends here: the toggle closing it, another deck taking the dock,
	/// and the dock being closed by the read-only switch.
	private onDockContent(e: DockContentEvent): void {
		if (e.wrapper !== this.wrapper) return;

		if (e.jsontype === this.allowedJsonType) {
			this.shown = true;
			return;
		}

		this.onHidden();
	}

	/// The deck no longer holds the panel. The kit throws its session away and the
	/// panel forgets the list, so the next opening starts from an empty panel and a
	/// fresh run.
	public onHidden(): void {
		if (!this.shown) return;
		this.shown = false;

		this.updatePresentationDeckHighlight('');

		if (!this.state.readOnly) this.sendRequest('release');

		this.stopListeningToMap();
		this.run = null;
		this.requestActions.clear();
		this.lastSent.clear();
		this.fixedRowId = null;
		this.fixedRowCategory = null;
		this.rowAfterFixedId = null;
		this.focusHeldForRun = false;

		// The options and the permission outlive the list, the rest starts over.
		const fresh = newCleanupPanelState();
		fresh.options = this.state.options;
		fresh.chosenOptions = this.state.chosenOptions;
		fresh.readOnly = this.state.readOnly;
		this.state = fresh;
	}

	/// What the run reports arrives on the map.
	private listenToMap(): void {
		if (this.listening) return;

		this.map.on('commandresult', this.onCommandResultBound);
		this.listening = true;
	}

	private stopListeningToMap(): void {
		if (!this.listening) return;

		this.map.off('commandresult', this.onCommandResultBound);
		this.listening = false;
	}

	/// The view changed between editing and reading, so the panel takes the state it
	/// now has. A view that can no longer be changed gives its run up and shows why the
	/// list is empty.
	private onPermissionChanged(): void {
		const readOnly = this.map.isReadOnlyMode();
		if (readOnly === this.state.readOnly) return;

		if (readOnly && this.shown && this.run !== null)
			this.sendRequest('release');

		this.state.readOnly = readOnly;
		if (readOnly) {
			this.run = null;
			this.state.hasList = false;
			this.requestActions.clear();
			this.clearRows();
			this.recountRows();
		}

		this.applyReadOnly();

		if (!readOnly && this.shown) this.sendRequest('list');
	}

	// --- the deck on screen
	// --------------------------------------------------------------

	/// Draws the regions named again, each one only when it does not look as it did when
	/// it was last drawn. A region drawn carries its children with it, so what was last
	/// drawn for those is forgotten.
	private refresh(...ids: string[]): void {
		if (!this.shown) return;

		for (const id of ids) {
			const json = cleanupRegionJSON(this.state, id);
			if (!json) continue;

			const look = regionLook(json);
			if (this.lastSent.get(id) === look) continue;

			if (!this.updateWidget(json)) continue;

			this.forgetSentChildren(json);
			this.lastSent.set(id, look);
		}
	}

	private forgetSentChildren(widget: WidgetJSON): void {
		for (const child of widget.children || []) {
			this.lastSent.delete(child.id);
			this.forgetSentChildren(child);
		}
	}

	private refreshGroups(): void {
		this.refresh(...CLEANUP_CATEGORIES.map(groupWidgetId));
	}

	private refreshAll(): void {
		this.refresh(
			CleanupWidgetId.options,
			CleanupWidgetId.run,
			CleanupWidgetId.summary,
			CleanupWidgetId.separator,
			CleanupWidgetId.message,
		);
		this.refreshGroups();
	}

	/// Puts the focus on the element named once the rebuilds queued before it are on
	/// screen.
	private focusAfterRefresh(elementId: string): void {
		app.layoutingService.appendLayoutingTask(() => {
			const element = document.getElementById(elementId);
			if (element) element.focus();
		});
	}

	private focusedElementId(): string {
		const focused = document.activeElement;
		return focused ? focused.id : '';
	}

	private focusIsInside(widgetId: string): boolean {
		const element = document.getElementById(widgetId);
		return !!element && element.contains(document.activeElement);
	}

	// --- what the person presses
	// ---------------------------------------------------------

	/// Every widget event of the deck is answered here, so none of them travels on to
	/// the kit.
	callback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder?: JSBuilder,
	): boolean {
		const id: string = object && typeof object.id === 'string' ? object.id : '';
		if (!id.startsWith('cleanup-')) return false;

		if (objectType === 'pushbutton' && eventType === 'click') {
			if (id === CleanupWidgetId.scan) this.onScanPressed();
			else if (id === CleanupWidgetId.stop) this.onStopPressed();
			else if (id === CleanupWidgetId.fixAll) this.onFixAllPressed();
			else {
				const rowId = rowOfWidgetId(id, '-fix');
				if (rowId !== null) this.onFixPressed(rowId);
			}
		} else if (objectType === 'linkbutton' && eventType === 'click') {
			const rowId = rowOfWidgetId(id, '-goto');
			if (rowId !== null) this.onGoToPressed(rowId);
		} else if (objectType === 'expander' && eventType === 'toggle') {
			const category = categoryOfGroupWidgetId(id);
			if (category) {
				if (this.state.foldedGroups.has(category))
					this.state.foldedGroups.delete(category);
				else this.state.foldedGroups.add(category);
			}
		} else {
			applyOptionEvent(
				this.state.chosenOptions,
				objectType,
				eventType,
				id,
				data,
			);
		}

		return true;
	}

	private onScanPressed(): void {
		if (this.state.busy || this.state.readOnly) return;

		this.state.options = Object.assign({}, this.state.chosenOptions);
		this.state.savedBytes = 0;
		this.setBusy(true);
		this.setStatus('scanning');
		this.setProgress(0, 0);
		this.sendRequest('scan', { options: this.state.options });
	}

	private onStopPressed(): void {
		this.sendRequest('stop');
	}

	private onFixAllPressed(): void {
		if (this.state.busy || this.run === null) return;

		this.setBusy(true);
		this.setStatus('cleaning');
		this.setProgress(0, 0);
		this.sendRequest('fixAll', { run: this.run });
	}

	private onFixPressed(rowId: number): void {
		if (this.state.busy || this.run === null) return;

		this.fixedRowId = rowId;
		const row = this.state.rows.get(rowId);
		this.fixedRowCategory = row ? row.category : null;
		this.rowAfterFixedId = this.rowWithFixAfter(rowId);
		this.setBusy(true);
		this.setStatus('cleaning');
		this.setProgress(0, 0);
		this.sendRequest('fix', { run: this.run, row: rowId });
	}

	private onGoToPressed(rowId: number): void {
		if (this.run === null) return;

		this.sendRequest('goTo', { run: this.run, row: rowId });
	}

	/// The kit says what the list was made with, so the controls take those values as
	/// well.
	private writeOptions(options: CleanupOptions): void {
		this.state.options = options;
		this.state.chosenOptions = Object.assign({}, options);
		this.refresh(CleanupWidgetId.options);
	}

	// --- the wire
	// ------------------------------------------------------------------------

	/// Sends one request and returns the number it was given. The request goes to the
	/// socket as it stands.
	private sendRequest(
		action: string,
		fields?: { [key: string]: unknown },
	): number {
		const requestId = ++this.nextRequestId;
		const request: { [key: string]: unknown } = {
			request: requestId,
			action: action,
		};

		if (fields) for (const key in fields) request[key] = fields[key];

		this.requestActions.set(requestId, action);

		app.socket.sendMessage(
			'uno .uno:PresentationCleanup ' +
				JSON.stringify({
					DataJson: { type: 'string', value: JSON.stringify(request) },
				}),
		);

		return requestId;
	}

	private onCommandResult(event: {
		commandName: string;
		success: boolean;
		result: unknown;
	}): void {
		if (event.commandName !== '.uno:PresentationCleanup') return;
		if (!this.shown) return;

		let result: CleanupResult | null = null;
		if (typeof event.result === 'string') {
			try {
				result = JSON.parse(event.result) as CleanupResult;
			} catch {
				result = null;
			}
		} else if (event.result && typeof event.result === 'object') {
			result = event.result as CleanupResult;
		}

		if (!result || !result.event) return;

		// Every event is applied inside one guard, so a payload the panel cannot
		// make sense of is reported and the panel stays as it was.
		try {
			switch (result.event) {
				case 'reply':
					this.onReply(result);
					break;
				case 'list':
					this.onList(result);
					break;
				case 'progress':
					this.onProgress(result);
					break;
				case 'measured':
					this.onMeasured(result);
					break;
				case 'finished':
					this.onFinished(result);
					break;
				case 'fixed':
					this.onFixed(result);
					break;
			}
		} catch (exception) {
			app.console.warn(
				'cleanup: the panel could not apply an event: ' + String(exception),
			);
		}
	}

	/// True while a request for the whole list is out and its reply has not arrived.
	private hasListRequestOut(): boolean {
		for (const action of this.requestActions.values())
			if (action === 'list') return true;

		return false;
	}

	/// True while the event belongs to a list the panel is not showing.
	private isStaleEvent(result: CleanupResult): boolean {
		return isStaleEvent(result, this.run, this.hasListRequestOut());
	}

	private onReply(result: CleanupResult): void {
		const requestId = result.request;
		if (typeof requestId !== 'number') return;

		// A cleanup request is finished by the fixed event that follows its reply,
		// and that event reads the action back. Every other request is finished by
		// its reply.
		const action = this.requestActions.get(requestId);
		if (result.ok === false || (action !== 'fix' && action !== 'fixAll'))
			this.requestActions.delete(requestId);

		if (result.ok !== false) return;

		// The kit turns a request away with readOnly when the document may not be
		// changed or its content may not leave the machine, and the browser cannot
		// always see either of those for itself. So the panel takes that state from
		// the answer and says there is nothing to do.
		const refusedAsReadOnly = result.reason === 'readOnly';
		if (refusedAsReadOnly) this.state.readOnly = true;

		// The request was refused, so what pressing it disabled comes back. A goTo
		// or a list is allowed while a run is going, so a refusal of one of those
		// leaves the run alone.
		if (refusalEndsBusyState(action)) {
			this.setBusy(false);
			this.setStatus('idle');
			this.updateSummary();
		}

		if (refusedAsReadOnly) this.applyReadOnly();

		if (result.reason === 'stale' || result.reason === 'gone')
			this.sendRequest('list');
	}

	private onList(result: CleanupResult): void {
		this.run = typeof result.run === 'number' ? result.run : null;
		// The kit numbers its lists from one, so a list under run zero is the state
		// of a session that has scanned nothing yet, and the panel keeps inviting
		// the reader to scan.
		this.state.hasList = this.run !== null && this.run > 0;

		if (result.reason !== 'fix') this.state.savedBytes = 0;

		if (result.reason === 'request' && isCleanupOptions(result.options))
			this.writeOptions(result.options);

		const measured = asMeasuredProgress(result.measured);

		// The rows the panel drew are what the counts are worked out over, so a row
		// the panel could not read is left out of them as well as off the screen.
		this.takeTotal(result.total);
		this.rebuildRows(asArray(result.rows));

		this.setStatus(result.status || 'idle');
		this.setBusy(this.state.statusName !== 'idle');
		this.setProgress(measured.done, measured.total);
		this.updateSummary();
		this.updateMessage();
	}

	private onProgress(result: CleanupResult): void {
		if (this.isStaleEvent(result)) return;

		const phase = result.phase;
		this.setStatus(
			phase === 'preparing' || phase === 'cleaning' ? phase : 'measuring',
		);
		this.setProgress(
			isFiniteNumber(result.done) ? result.done : 0,
			isFiniteNumber(result.total) ? result.total : 0,
		);
	}

	private onMeasured(result: CleanupResult): void {
		if (this.isStaleEvent(result)) return;

		for (const entry of asArray(result.rows)) {
			if (!isCleanupRowPatch(entry)) {
				app.console.warn('cleanup: dropping a row patch the panel cannot read');
				continue;
			}

			const row = this.state.rows.get(entry.id);
			if (!row) continue;
			if (typeof entry.saving === 'number') row.saving = entry.saving;
			row.measured = true;
			this.refresh(rowWidgetId(row.id));
		}

		for (const droppedId of asNumberArray(result.dropped))
			this.removeRow(droppedId);

		// The counts are worked out over the rows on screen, and the message
		// carries the estimate alone, so what it does not say keeps the value the
		// list gave it.
		this.recountRows();
		this.takeTotal(result.total);

		this.updateGroupHeadings();
		this.updateSummary();
		this.updateMessage();
	}

	private onFinished(result: CleanupResult): void {
		if (this.isStaleEvent(result)) return;

		this.writeRowNumbers(asArray(result.rows));
		this.takeTotal(result.total);
		this.recountRows();

		this.setStatus('idle');
		this.setBusy(false);
		this.updateGroupHeadings();
		this.updateSummary();
		this.updateMessage();
	}

	private onFixed(result: CleanupResult): void {
		if (this.isStaleEvent(result)) return;

		const action =
			typeof result.request === 'number'
				? this.requestActions.get(result.request)
				: undefined;
		if (typeof result.request === 'number')
			this.requestActions.delete(result.request);

		if (result.cancelled) {
			this.setStatus('idle');
			this.setBusy(false);
			this.updateSummary();
			return;
		}

		const rescan = result.rescan === true;

		// A cleanup that reaches beyond the rows it names is followed by a scan of
		// its own. The rows it dealt with stay on screen greyed out until that list
		// arrives, so the reader sees the list they pressed on rather than an empty
		// panel.
		for (const removedId of asNumberArray(result.removed)) {
			if (rescan) this.greyRow(removedId);
			else this.removeRow(removedId);
		}

		this.writeRowNumbers(asArray(result.rows));

		if (action === 'fixAll' && typeof result.saved === 'number')
			this.state.savedBytes = result.saved;

		if (!rescan) {
			this.takeTotal(result.total);
			this.recountRows();
			this.setStatus('idle');
			this.setBusy(false);
			this.updateGroupHeadings();
			this.updateSummary();
			this.updateMessage();
		}
	}

	/// The rows in the order they stand on screen: group by group, and within a group
	/// as the kit listed them.
	private rowsInScreenOrder(): CleanupRow[] {
		const rows: CleanupRow[] = [];
		for (const category of CLEANUP_CATEGORIES)
			rows.push(...rowsOf(this.state, category));
		return rows;
	}

	/// The id of the first row under the given one that offers a cleanup, and null when
	/// the row is the last one that does. Read before the cleanup runs, while both rows
	/// are still there.
	private rowWithFixAfter(rowId: number): number | null {
		const withFix = this.rowsInScreenOrder().filter((row) => row.canFix);
		const index = withFix.findIndex((row) => row.id === rowId);
		if (index === -1 || index + 1 >= withFix.length) return null;

		return withFix[index + 1].id;
	}

	/// The run is over and its status line is about to leave the screen, so the focus
	/// the line is holding moves to something that stays: the Fix button of the row
	/// that followed the one dealt with, the heading of that row's group, or the button
	/// that starts a run. The focus is placed again when the status line holds it, when
	/// the line has left the screen, or when the document took it. A focus the reader
	/// moved anywhere else is left where they put it. Returns the id of the element to
	/// focus, decided while the status line is still on screen, and null when the focus
	/// is to be left alone.
	private focusTargetAfterRun(): string | null {
		const heldForRun = this.focusHeldForRun;
		const takenFrom = this.focusTakenFrom;
		const fixedRowCategory = this.fixedRowCategory;
		const rowAfterFixedId = this.rowAfterFixedId;
		this.focusHeldForRun = false;
		this.focusTakenFrom = '';
		this.fixedRowId = null;
		this.fixedRowCategory = null;
		this.rowAfterFixedId = null;

		if (!this.shown) return null;

		const focused = document.activeElement;
		const focusInStatusLine = this.focusIsInside(CleanupWidgetId.status);
		if (!heldForRun && !focusInStatusLine) return null;

		// A run short enough to end before the move to Stop is drawn leaves the focus
		// on the widget the run took it from, and that widget goes with the rebuild.
		// So the panel still has the focus to place, the same as when nothing holds
		// it. A widget the reader moved to in the meantime is left alone.
		const heldByNothing =
			!focused ||
			focused === document.body ||
			(takenFrom !== '' && focused.id === takenFrom);
		if (!focusInStatusLine && !heldByNothing && !this.map.hasFocus())
			return null;

		const scanButton = this.state.readOnly
			? null
			: buttonElementId(CleanupWidgetId.scan);

		if (!heldForRun) return scanButton;

		const next =
			rowAfterFixedId === null ? null : this.state.rows.get(rowAfterFixedId);
		if (next && next.canFix)
			return buttonElementId(rowWidgetId(next.id) + '-fix');

		if (fixedRowCategory && this.countOf(fixedRowCategory) > 0)
			return buttonElementId(groupWidgetId(fixedRowCategory) + '-heading');

		return scanButton;
	}

	private takeTotal(total: CleanupTotal | number | undefined): void {
		if (!total || typeof total === 'number') return;

		if (typeof total.estimated === 'number')
			this.state.estimatedBytes = total.estimated;
		// The kit carries the saved figure on every list, so an undo of a cleanup
		// leaves the two sides agreeing on what the document has shed.
		if (typeof total.saved === 'number') this.state.savedBytes = total.saved;
		if (typeof total.count === 'number') this.state.rowCount = total.count;
		if (typeof total.fixable === 'number')
			this.state.fixableCount = total.fixable;
	}

	// --- the rows
	// ------------------------------------------------------------------------

	private clearRows(): void {
		this.state.rows.clear();
		this.state.goneRows.clear();
		this.refreshGroups();
	}

	private rebuildRows(rows: unknown[]): void {
		// The kit numbers the rows of every list from one, so the row the last
		// cleanup was pressed on and the row that followed it are named by ids of a
		// list that is gone. The group they were in stands over the same kind of
		// finding in every list, so it stays.
		this.fixedRowId = null;
		this.rowAfterFixedId = null;

		this.state.rows.clear();
		this.state.goneRows.clear();

		for (const entry of rows) {
			if (!isCleanupRow(entry)) {
				app.console.warn('cleanup: dropping a row the panel cannot read');
				continue;
			}

			this.state.rows.set(entry.id, entry);
		}

		this.recountRows();
		this.refreshGroups();
	}

	private removeRow(rowId: number): void {
		const row = this.state.rows.get(rowId);
		if (!row) return;

		this.state.rows.delete(rowId);
		this.state.goneRows.delete(rowId);
		this.refresh(groupWidgetId(row.category));
	}

	private greyRow(rowId: number): void {
		if (!this.state.rows.has(rowId)) return;

		this.state.goneRows.add(rowId);
		this.refresh(rowWidgetId(rowId));
	}

	private writeRowNumbers(patches: unknown[]): void {
		for (const entry of patches) {
			if (!isCleanupRowPatch(entry)) {
				app.console.warn('cleanup: dropping a row patch the panel cannot read');
				continue;
			}

			if (typeof entry.imageNumber !== 'number') continue;

			const row = this.state.rows.get(entry.id);
			if (!row) continue;

			row.imageNumber = entry.imageNumber;
			this.refresh(rowWidgetId(row.id));
		}
	}

	/// Works the counts out over the rows on screen, for the events that patch the list
	/// without saying what it now comes to.
	private recountRows(): void {
		let count = 0;
		let fixable = 0;

		for (const row of this.state.rows.values()) {
			count++;
			if (row.canFix) fixable++;
		}

		this.state.rowCount = count;
		this.state.fixableCount = fixable;
	}

	private countOf(category: CleanupCategory): number {
		return rowsOf(this.state, category).length;
	}

	/// A heading says how many rows its group holds, so a group folded away still says
	/// how much there is in it. A group with nothing to report is off the screen.
	private updateGroupHeadings(): void {
		this.refreshGroups();
	}

	// --- the top of the panel
	// ------------------------------------------------------------

	/// The line over the bar names the phase the run is in, and is read out as it
	/// changes.
	private setStatus(status: string): void {
		this.state.statusName = status;
		this.refresh(CleanupWidgetId.statusText);
	}

	/// How far the run has come stands on the bar, as a share of the whole for the eye
	/// and as the two counts beside it.
	private setProgress(done: number, total: number): void {
		this.state.measuredDone = done;
		this.state.measuredTotal = total;
		this.refresh(CleanupWidgetId.progressLine);
	}

	/// One place over the list holds either the button that starts a run or the status
	/// line of the run that is going. The options, the two buttons that act on a list
	/// and the button of every row are turned off while the run lasts.
	private setBusy(busy: boolean): void {
		if (busy) {
			// A run turns off the controls, so one of them holding the focus
			// hands it to Stop, which stays live for as long as the run does.
			const focusedId = this.focusedElementId();
			this.state.busy = true;
			this.refreshBusyRegions();

			if (goesDeadWhenBusy(focusedId)) {
				this.focusHeldForRun = true;
				this.focusTakenFrom = focusedId;
				this.focusAfterRefresh(buttonElementId(CleanupWidgetId.stop));
			}
			return;
		}

		// The status line leaves the screen with this rebuild, so where the focus
		// it may hold goes is decided while the line is still there, and the focus
		// is placed once the rebuild is on screen.
		const target = this.focusTargetAfterRun();
		this.state.busy = false;
		this.refreshBusyRegions();
		if (target) this.focusAfterRefresh(target);
	}

	/// The busy state shows in the options, the run, the band and the button of every
	/// row.
	private refreshBusyRegions(): void {
		this.refresh(
			CleanupWidgetId.options,
			CleanupWidgetId.run,
			CleanupWidgetId.summary,
		);
		this.refreshGroups();
		this.refresh(
			...Array.from(this.state.rows.keys()).map((rowId) => rowWidgetId(rowId)),
		);
	}

	private updateSummary(): void {
		this.refresh(CleanupWidgetId.summary);
	}

	private updateMessage(): void {
		this.refresh(CleanupWidgetId.message);
	}

	/// A read-only document offers no cleanup, so the options, the run and the band all
	/// go and the panel says why the list is empty. The line that divides the list off
	/// goes with it.
	private applyReadOnly(): void {
		if (!this.state.readOnly) this.setBusy(false);

		this.refreshAll();
	}
}

JSDialog.CleanupSidebar = function (map: MapInterface) {
	return new CleanupSidebar(map);
};
