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
 * JSDialog.EditEngine - a contenteditable view of an engine-side EditEngine.
 *
 * The engine keeps the text model and the caret. EditEngineWidgetController serializes both as
 * paragraphs with formatting runs plus a selection, and this widget renders that into a
 * contenteditable element. The widget is a view: every input event is prevented and forwarded, and
 * the next model the engine sends replaces both the text on screen and the caret position. It
 * never guesses where a keystroke will leave the caret.
 *
 * Keystrokes travel as UNO key codes and become real key events on the engine side, so paragraph
 * splitting, outline promote and demote, word navigation, autocorrect and undo grouping all come
 * from the engine. Composed text and pasted text cannot be expressed as a keystroke and are sent
 * as their own command. A pointer gesture is the one caret move reported back, because it is the
 * only one the engine has no other way to learn about.
 *
 * Expected JSON payload (in data.data): see EditEngineWidgetJSON.
 *
 * Sent back as 'key' { keyCode, charCode, repeat }, 'text' { text } and
 * 'selection' { startPara, startIndex, endPara, endIndex }.
 */

declare var JSDialog: any;
declare var UNOKey: any;

interface EditEngineContainer extends HTMLElement {
	builder: JSBuilder;
	editEngineHandlersAttached?: boolean;
	onEditEngineUpdate?: (data: EditEngineWidgetJSON) => void;
	// The last selection sent to the engine, so a pointer gesture that lands on the same caret is
	// not reported twice. Held on the element so it goes away when the widget does.
	editEngineLastSelection?: string;
}

const EDITENGINE_PARAGRAPH_CLASS = 'ui-editengine-paragraph';

/// One outline level indents by this much.
const EDITENGINE_DEPTH_INDENT_PX = 24;

/// True only for the rest of the turn after a keydown was turned into a key event, so a beforeinput
/// that some engines still fire for that same key is not sent again as text.
let editEngineKeyEventPending = false;

/// The layouting task that drops editEngineKeyEventPending after the current turn, kept so a new
/// keydown can cancel a stale clear before scheduling its own.
let editEngineKeyEventClearTask: TaskId | undefined;

function editEngineSendAction(
	container: EditEngineContainer,
	action: string,
	data: any,
): void {
	if (!container.builder || !container.builder.callback) return;
	container.builder.callback(
		'customwidget',
		action,
		container,
		JSON.stringify(data),
		container.builder,
	);
}

function editEngineParagraphElements(container: HTMLElement): HTMLElement[] {
	return Array.from(
		container.querySelectorAll<HTMLElement>('.' + EDITENGINE_PARAGRAPH_CLASS),
	);
}

/// The paragraph plus character index a DOM position sits at, clamped into the rendered text.
function editEnginePositionFromNode(
	container: HTMLElement,
	node: Node,
	offset: number,
): EditEnginePosition | null {
	const paragraphs = editEngineParagraphElements(container);
	for (let i = 0; i < paragraphs.length; ++i) {
		if (paragraphs[i] === node || paragraphs[i].contains(node)) {
			return {
				para: i,
				index: JSDialog.ContentEditable.offsetWithin(
					paragraphs[i],
					node,
					offset,
				),
			};
		}
	}
	return null;
}

function editEngineGetSelection(
	container: HTMLElement,
): EditEngineSelection | null {
	const selection = window.getSelection();
	if (!selection || selection.rangeCount === 0) return null;

	const range = selection.getRangeAt(0);
	if (
		!container.contains(range.startContainer) ||
		!container.contains(range.endContainer)
	)
		return null;

	const start = editEnginePositionFromNode(
		container,
		range.startContainer,
		range.startOffset,
	);
	const end = editEnginePositionFromNode(
		container,
		range.endContainer,
		range.endOffset,
	);
	if (!start || !end) return null;

	return {
		startPara: start.para,
		startIndex: start.index,
		endPara: end.para,
		endIndex: end.index,
	};
}

function editEngineApplySelection(
	container: HTMLElement,
	selection: EditEngineSelection,
): void {
	// Only take over the document selection while this widget has focus. An update pushed by the
	// engine, or a rebuild, must not drag the caret here and clear what the user had selected in
	// another field.
	if (document.activeElement !== container) return;

	const paragraphs = editEngineParagraphElements(container);
	if (!paragraphs.length) return;

	const clampPara = (para: number): number =>
		Math.max(0, Math.min(para, paragraphs.length - 1));

	const [startNode, startOffset] = JSDialog.ContentEditable.findNodeAtOffset(
		paragraphs[clampPara(selection.startPara)],
		selection.startIndex,
	);
	const [endNode, endOffset] = JSDialog.ContentEditable.findNodeAtOffset(
		paragraphs[clampPara(selection.endPara)],
		selection.endIndex,
	);
	JSDialog.ContentEditable.setCaretRange(
		startNode,
		startOffset,
		endNode,
		endOffset,
	);
}

function editEngineKey(selection: EditEngineSelection): string {
	return (
		selection.startPara +
		':' +
		selection.startIndex +
		':' +
		selection.endPara +
		':' +
		selection.endIndex
	);
}

function editEngineSendSelection(container: EditEngineContainer): void {
	const selection = editEngineGetSelection(container);
	if (!selection) return;

	const key = editEngineKey(selection);
	if (container.editEngineLastSelection === key) return;
	container.editEngineLastSelection = key;

	editEngineSendAction(container, 'selection', selection);
}

/// The UNO key code for a keyboard event, reusing the document keymap so there is one table.
function editEngineToUNOKeyCode(
	event: KeyboardEvent,
	builder: JSBuilder,
): number {
	const keyboard = builder.map ? (builder.map as any).keyboard : null;
	let keyCode = keyboard
		? keyboard._toUNOKeyCode(event.keyCode)
		: event.keyCode;

	if (event.shiftKey) keyCode |= app.UNOModifier.SHIFT;
	if (event.ctrlKey || event.metaKey) keyCode |= app.UNOModifier.CTRL;
	if (event.altKey) keyCode |= app.UNOModifier.ALT;

	return keyCode;
}

function editEngineOnKeyDown(
	container: EditEngineContainer,
	event: KeyboardEvent,
): void {
	editEngineKeyEventPending = false;

	// keyCode 229 is what browsers report while an input method owns the keystroke.
	if (event.isComposing || event.keyCode === 229) return;

	event.preventDefault();

	const keyCode = editEngineToUNOKeyCode(event, container.builder);
	const printable = event.key.length === 1 && !event.ctrlKey && !event.metaKey;
	const charCode = printable ? (event.key.codePointAt(0) ?? 0) : 0;

	editEngineSendAction(container, 'key', {
		keyCode: keyCode,
		charCode: charCode,
		repeat: event.repeat ? 1 : 0,
	});

	// A prevented keydown normally suppresses the beforeinput for the same key, but some engines
	// still fire it. Guard against sending that character a second time, then drop the guard after
	// this turn so a later keystroke-less input (virtual keyboard, autocorrect, drop) is not
	// swallowed. A same-key beforeinput runs in this turn, before the layouting task clears it.
	editEngineKeyEventPending = true;
	if (editEngineKeyEventClearTask !== undefined)
		app.layoutingService.cancelLayoutingTask(editEngineKeyEventClearTask);
	editEngineKeyEventClearTask = app.layoutingService.appendLayoutingTask(() => {
		editEngineKeyEventPending = false;
		editEngineKeyEventClearTask = undefined;
	});
}

function editEngineOnBeforeInput(
	container: EditEngineContainer,
	event: InputEvent,
): void {
	event.preventDefault();

	if (event.isComposing) return;

	if (editEngineKeyEventPending) {
		editEngineKeyEventPending = false;
		return;
	}

	// Input with no keystroke behind it: a virtual keyboard, an autocorrection or a drop.
	if (event.inputType.startsWith('insert') && event.data) {
		editEngineSendAction(container, 'text', { text: event.data });
	} else if (event.inputType.startsWith('delete')) {
		const forward = event.inputType.indexOf('Forward') !== -1;
		editEngineSendAction(container, 'key', {
			keyCode: forward ? UNOKey.DELETE : UNOKey.BACKSPACE,
			charCode: 0,
			repeat: 0,
		});
	}
}

function editEngineAttachHandlers(container: EditEngineContainer): void {
	container.addEventListener('keydown', (e: KeyboardEvent) =>
		editEngineOnKeyDown(container, e),
	);

	container.addEventListener('beforeinput', (e: Event) =>
		editEngineOnBeforeInput(container, e as InputEvent),
	);

	container.addEventListener('compositionend', (e: CompositionEvent) => {
		if (e.data) editEngineSendAction(container, 'text', { text: e.data });
	});

	container.addEventListener('paste', (e: ClipboardEvent) => {
		e.preventDefault();
		const text = e.clipboardData ? e.clipboardData.getData('text/plain') : '';
		if (text) editEngineSendAction(container, 'text', { text: text });
	});

	container.addEventListener('cut', (e: ClipboardEvent) => {
		e.preventDefault();
		const selected = window.getSelection()?.toString() || '';
		if (!selected) return;
		if (e.clipboardData) e.clipboardData.setData('text/plain', selected);
		editEngineSendAction(container, 'key', {
			keyCode: UNOKey.DELETE,
			charCode: 0,
			repeat: 0,
		});
	});

	// A pointer gesture is the one caret move the engine cannot know about, so it is reported.
	// Keyboard caret moves are not: those went to the engine as key events, and the model that
	// comes back says where the caret ended up.
	container.addEventListener('mouseup', () =>
		editEngineSendSelection(container),
	);
}

function editEngineRenderRun(run: EditEngineRun, text: string): HTMLElement {
	const span = document.createElement('span');

	if (run.spellError) {
		// The squiggle sits on a child so that it draws alongside an underline the run itself
		// carries, rather than one replacing the other.
		const marked = document.createElement('span');
		marked.classList.add('ui-editengine-spell-error');
		marked.setAttribute('aria-invalid', 'spelling');
		marked.textContent = text;
		span.appendChild(marked);
	} else {
		span.textContent = text;
	}

	if (run.bold) span.style.fontWeight = 'bold';
	if (run.italic) span.style.fontStyle = 'italic';
	if (run.color) span.style.color = run.color;
	if (run.family) span.style.fontFamily = run.family;
	if (run.size) span.style.fontSize = run.size + 'pt';

	const decorations = [];
	if (run.underline) decorations.push('underline');
	if (run.strikeout) decorations.push('line-through');
	if (decorations.length) {
		span.style.textDecorationLine = decorations.join(' ');
		if (run.underline && run.underline !== 'single')
			span.style.textDecorationStyle =
				run.underline === 'wave' ? 'wavy' : run.underline;
	}

	if (run.escapement === 'super') span.style.verticalAlign = 'super';
	else if (run.escapement === 'sub') span.style.verticalAlign = 'sub';

	return span;
}

function editEngineRenderParagraph(
	paragraph: EditEngineParagraph,
): HTMLElement {
	const element = document.createElement('div');
	element.classList.add(EDITENGINE_PARAGRAPH_CLASS);

	if (paragraph.align) element.style.textAlign = paragraph.align;
	// A paragraph outside the outline reports a depth of -1, which must not indent.
	if (paragraph.depth && paragraph.depth > 0)
		element.style.paddingInlineStart =
			paragraph.depth * EDITENGINE_DEPTH_INDENT_PX + 'px';

	// The paragraph's default font, which its runs inherit unless they carry their own.
	if (paragraph.family) element.style.fontFamily = paragraph.family;
	if (paragraph.size) element.style.fontSize = paragraph.size + 'pt';
	if (paragraph.color) element.style.color = paragraph.color;

	// The bullet is rendered as generated content from this attribute, so its characters stay out
	// of the paragraph text. Selection offsets then line up with the engine, which numbers indices
	// over the text alone and treats the bullet as generated.
	if (paragraph.bulletText)
		element.setAttribute('data-bullet', paragraph.bulletText);

	const text = paragraph.text || '';
	const runs = paragraph.runs || [];

	if (!text.length) {
		// An empty paragraph still has to occupy a line and be a caret target.
		element.appendChild(document.createElement('br'));
		return element;
	}

	if (!runs.length) {
		element.appendChild(document.createTextNode(text));
		return element;
	}

	for (const run of runs) {
		const part = text.substring(run.start, run.end);
		if (!part) continue;
		element.appendChild(editEngineRenderRun(run, part));
	}

	return element;
}

/// Applies a fresh model to an existing widget without recreating the container. The engine sends
/// these after the first build, so the contenteditable element that holds focus and the input
/// method state is kept, and only its paragraphs and caret are replaced.
/// It already happens in JSDialog action handling so inside LayoutingTask.
function editEngineUpdateInPlace(
	container: EditEngineContainer,
	widgetData: EditEngineWidgetJSON,
): void {
	if (widgetData.backgroundColor)
		container.style.background = widgetData.backgroundColor;

	const readOnly = widgetData.readOnly === true;
	container.setAttribute('contenteditable', readOnly ? 'false' : 'true');
	if (!readOnly && !container.editEngineHandlersAttached) {
		editEngineAttachHandlers(container);
		container.editEngineHandlersAttached = true;
	}

	const paragraphs = (widgetData.paragraphs || []).map(
		editEngineRenderParagraph,
	);

	container.replaceChildren(...paragraphs);

	// The engine owns the caret, so its selection is applied as given. The container stays in the
	// document across the update, so the caret can be placed at once with no wait for a frame.
	const selection = widgetData.selection || {
		startPara: 0,
		startIndex: 0,
		endPara: 0,
		endIndex: 0,
	};
	container.editEngineLastSelection = editEngineKey(selection);
	editEngineApplySelection(container, selection);
}

function _editEngineControl(
	parentContainer: Element,
	data: any,
	builder: JSBuilder,
): boolean {
	const widgetData: EditEngineWidgetJSON = data.data || data;

	const container = window.L.DomUtil.create(
		'div',
		'ui-editengine ' + (builder.options.cssClass || ''),
		parentContainer,
	) as EditEngineContainer;
	container.id = data.id;
	container.builder = builder;

	container.setAttribute('role', 'textbox');
	container.setAttribute('aria-multiline', 'true');
	container.setAttribute('spellcheck', 'false');
	container.setAttribute('autocorrect', 'off');
	container.tabIndex = 0;

	if (widgetData.backgroundColor)
		container.style.background = widgetData.backgroundColor;

	for (const paragraph of widgetData.paragraphs || [])
		container.appendChild(editEngineRenderParagraph(paragraph));

	const readOnly = widgetData.readOnly === true;
	container.setAttribute('contenteditable', readOnly ? 'false' : 'true');
	if (!readOnly) {
		editEngineAttachHandlers(container);
		container.editEngineHandlersAttached = true;
	}

	container.onEditEngineUpdate = (updateData: EditEngineWidgetJSON) =>
		editEngineUpdateInPlace(container, updateData);

	// The engine owns the caret. Whatever it says is where the caret goes, with no local guess
	// about where a keystroke will have moved it.
	const selection = widgetData.selection || {
		startPara: 0,
		startIndex: 0,
		endPara: 0,
		endIndex: 0,
	};
	container.editEngineLastSelection = editEngineKey(selection);

	// An update builds into a detached fragment and swaps the node in afterwards, and a caret
	// cannot be placed on a node that is not in the document yet, so this waits a frame.
	app.layoutingService.appendLayoutingTask(() => {
		if (container.isConnected) editEngineApplySelection(container, selection);
	});

	return false;
}

JSDialog.editEngine = function (
	parentContainer: Element,
	data: any,
	builder: JSBuilder,
): boolean {
	return _editEngineControl(parentContainer, data, builder);
};

// TODO: editengine might become regular widget, not a custom one
if (JSDialog.registerCustomWidgetHandler) {
	JSDialog.registerCustomWidgetHandler('editengine', _editEngineControl);
}
