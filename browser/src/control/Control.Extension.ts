// @ts-strict-ignore
/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
 * Control.Extension - one instance per loaded extension manifest.
 *
 * The constructor takes the parsed manifest plus the URL prefix that
 * extension-relative paths (`entry`, `icon`) are resolved against; see
 * browser/extensions/README.md for the manifest format.  Discovery
 * (fetching the index and the manifests) and instantiation live in
 * loadExtensions() below.
 *
 * The extension HTML page runs in an iframe and communicates with
 * COOL via postMessage.  The interface between extensions and COOL
 * consists of these messages, each tagged with its own msgId:
 *
 *   { msgId: 'Extension_Call', callId, fn, args }
 *     Calls a JS function in core via QuickJS UNO.  fn is the
 *     function source as a string; args is an array of
 *     JSON-serializable arguments.  callId is an opaque token used
 *     to correlate the response.
 *
 *   { msgId: 'Extension_Close' }
 *     Closes the extension's deck and the sidebar with it.
 *
 * Before the iframe is torn down (either via Extension_Close or
 * the close button on the panel header) COOL posts:
 *
 *   { msgId: 'Extension_Teardown' }
 *
 * to give the iframe a chance to detach any UNO listeners it
 * registered.  The iframe is expected to answer with:
 *
 *   { msgId: 'Extension_TeardownDone' }
 *
 * once it has done so; only then does COOL actually remove the
 * iframe.  A timeout limits how long we wait for the response.
 *
 * The result of an Extension_Call comes back from COOL to the
 * iframe as a postMessage:
 *
 *   { msgId: 'Extension_CallResult', callId, ok? , err? }
 *
 * Exactly one of ok/err is set: ok is the JS result value (already
 * decoded from the JSON the server produced), err is an exception
 * message string.
 *
 * COOL also forwards `proxycall' events (originating from
 * server-side JS-UNO proxy listener stubs) into the iframe as
 * postMessages with msgId 'Extension_ProxyCall':
 *
 *   { msgId: 'Extension_ProxyCall', proxyId, callId?, method, args }
 *
 * The iframe is expected to dispatch the call to whatever JS
 * callback it registered for that proxyId.  When `callId` is present
 * the proxy's invoke is blocked waiting for a synchronous return
 * value and the iframe must answer with:
 *
 *   { msgId: 'Extension_ProxyReturn', callId, value }
 *
 * which is forwarded to the server as `proxyreturn <callId>
 * <json-value>` and unblocks the proxy.  If `callId` is absent the
 * call was fire-and-forget (void-return method or the proxy has a
 * fixed return value); no response is expected.
 *
 * The extension can also open a modal dialog whose body is another
 * page under the extension's own base URL:
 *
 *   { msgId: 'Extension_ShowDialog', dialogId, url, title?, width?,
 *     height? }
 *
 * The dialog page is expected to load cool.js too and use
 * cool.dialog.close(value) / cool.dialog.cancel(), which the dialog
 * iframe posts back as:
 *
 *   { msgId: 'Extension_DialogClose', value }
 *   { msgId: 'Extension_DialogCancel' }
 *
 * COOL then dismisses the dialog and delivers the outcome to the
 * originating sidebar iframe:
 *
 *   { msgId: 'Extension_DialogResult', dialogId, cancelled, value? }
 *
 * The titlebar close X and Esc pressed inside the dialog iframe map
 * to Extension_DialogCancel via the iframe removal path.  Only one
 * dialog per extension can be open at a time; a second open() while
 * the first is still up resolves immediately as cancelled.
 */

/* global app */

// A command an extension registers for use in menu/notebookbar contributions
// (see ExtensionContributes below).  `script` is a path to a JS file (resolved
// the same way `entry`/`icon` are, relative to the manifest) whose top-level
// binding named `commands` is an object mapping command ids to functions;
// discovery fetches it once and fills in `source` with the raw text.
// invokeCommand ships that text to the kit's JS-UNO context verbatim, with a
// call to the right entry of `commands` tacked on after it, the same way
// cool.callRemote ships a function's source from inside the sidebar iframe.
// More than one command may name the same `script` file, sharing its
// `commands` object rather than each getting a one-function file of its own.
// `icon` is shown on a notebookbar button or dropdown-menu entry that
// references this command; the classic menu never renders it.
interface ExtensionCommand {
	id: string;
	title: string;
	icon?: string;
	script: string;
	source?: string;
}

// One notebookbar button, referencing a command declared in
// contributes.commands.  Its label/icon come from that command, not from
// this item, so the same command reads the same wherever it's placed.
// `size` chooses bigcustomtoolitem (icon above label) or customtoolitem
// (icon inline with label); defaults to 'small'.
interface ExtensionNotebookbarButton {
	type: 'button';
	command: string;
	size?: 'large' | 'small';
}

// A vertical divider between items within a notebookbar group.
interface ExtensionNotebookbarSeparator {
	type: 'separator';
}

// A notebookbar dropdown button.  `items` is deliberately flat - one level
// of commands, no nested menu - there is no submenu-of-a-submenu case to
// design or validate.
interface ExtensionNotebookbarMenu {
	type: 'menu';
	title: string;
	icon?: string;
	items: { command: string }[];
}

type ExtensionNotebookbarItem =
	| ExtensionNotebookbarButton
	| ExtensionNotebookbarSeparator
	| ExtensionNotebookbarMenu;

// A labeled cluster of notebookbar items, corresponding to one ribbon group
// (e.g. Writer's "Clipboard" or "Font" group).  `label` is the caption shown
// under the group; `id` only identifies the group in a console warning about
// one of its own items (an unknown command or item type) - it does not need
// to be unique, not even within this one group array.
interface ExtensionNotebookbarGroup {
	id: string;
	label: string;
	items: ExtensionNotebookbarItem[];
}

// Places in the classic menu and the notebookbar an extension can put its
// commands into, without needing its sidebar `entry` (if any) to be open.
// `menus` maps an existing top-level menu id (the `id` field already used in
// each doc type's static menu array in Control.Menubar.ts, e.g. 'insert') to
// the command ids appended to the end of that menu.  `notebookbar` is a list
// of brand-new tabs the extension adds to the ribbon; each names its own
// `tab` label, positions itself relative to an existing tab via
// `insertBefore`/`insertAfter` (mutually exclusive; omitting both appends it
// at the end), and lays out its own content as `groups` of the three item
// kinds above - not raw notebookbar item JSON, so an extension can't place
// arbitrary layout into the ribbon.
interface ExtensionNotebookbarTab {
	tab: string;
	insertBefore?: string;
	insertAfter?: string;
	groups: ExtensionNotebookbarGroup[];
}

// One entry an extension adds to the document's right-click context menu.
// Every extension's entries render together as their own trailing group, after
// a separator, at the end of the menu - never interleaved with the document's
// own items, the same "own space only" rule the notebookbar tab and menu
// contributions above follow.  `contexts` narrows which right-click menu the
// entry shows up in; omitting it (or passing an empty array) shows the entry
// in every right-click menu the manifest's top-level `supports` already
// allows this extension into.
interface ExtensionContextMenuEntry {
	command: string;
	contexts?: ('text-selection' | 'image')[];
}

// One entry an extension adds to the floating toolbar that appears over a mouse
// text selection. That toolbar only ever shows on desktop, outside read-only mode
// and outside Calc, and only while a selection exists, so unlike the context menu
// there is no separate `contexts` field to narrow it further. Every extension's
// buttons share one separator after the toolbar's own built-in content.
interface ExtensionContextToolbarButton {
	command: string;
}

// A single key bound to a command. `modifier` is drawn from "ctrl"/"alt"/"shift" -
// "ctrl" already means Cmd on macOS, the same as it does for every built-in
// keyboard shortcut. A key combination that collides with an existing shortcut
// (built-in, or from another extension) is dropped with a console warning rather
// than registered, since two shortcuts sharing the same combination is something
// the underlying dispatch mechanism cannot recover from at the point the key is
// actually pressed. A single printable key needs at least "ctrl" or "alt" in
// modifier - "shift" alone (or no modifier at all) would fight with ordinary typing.
interface ExtensionKeybinding {
	command: string;
	key: string;
	modifier?: ('ctrl' | 'alt' | 'shift')[];
}

interface ExtensionContributes {
	commands?: ExtensionCommand[];
	menus?: { [menuId: string]: string[] };
	notebookbar?: ExtensionNotebookbarTab[];
	contextMenu?: ExtensionContextMenuEntry[];
	contextToolbar?: ExtensionContextToolbarButton[];
	keybindings?: ExtensionKeybinding[];
}

interface ExtensionManifest {
	manifestVersion: string;
	name: string;
	// Absent for a commands-only extension that contributes no sidebar panel.
	entry?: string;
	icon?: string;
	supports?: string[];
	// On disk this is a string naming a separate JSON file (resolved the same way
	// entry/icon are) holding the ExtensionContributes object - keeping UI wiring
	// out of manifest.json's own metadata is mandatory, not a choice an extension
	// author makes. loadExtensions resolves that indirection once, at discovery
	// time, so this field is always the object form by the time anything else
	// reads it.
	contributes?: ExtensionContributes;
}

interface ExtensionCallMessage {
	msgId: 'Extension_Call';
	callId: string;
	fn: string;
	source: string;
	line: number;
	args?: unknown[];
}

interface ExtensionCloseMessage {
	msgId: 'Extension_Close';
}

interface ExtensionProxyReturnMessage {
	msgId: 'Extension_ProxyReturn';
	callId: string;
	value: unknown;
}

interface ExtensionTeardownDoneMessage {
	msgId: 'Extension_TeardownDone';
}

interface ExtensionResizeMessage {
	msgId: 'Extension_Resize';
	height: number;
}

interface ExtensionShowDialogMessage {
	msgId: 'Extension_ShowDialog';
	dialogId: string;
	url: string;
	title?: string;
	width?: number;
	height?: number;
}

interface ExtensionDialogCloseMessage {
	msgId: 'Extension_DialogClose';
	value: unknown;
}

interface ExtensionDialogCancelMessage {
	msgId: 'Extension_DialogCancel';
}

type ExtensionSidebarMessage =
	| ExtensionCallMessage
	| ExtensionCloseMessage
	| ExtensionProxyReturnMessage
	| ExtensionTeardownDoneMessage
	| ExtensionResizeMessage
	| ExtensionShowDialogMessage;

type ExtensionDialogMessage =
	| ExtensionDialogCloseMessage
	| ExtensionDialogCancelMessage
	| ExtensionResizeMessage;

type ExtensionMessage = ExtensionSidebarMessage | ExtensionDialogMessage;

interface ExtensionScriptStackFrame {
	source: string;
	line: string;
	column: string;
	functionName: string;
}
interface ExtensionScriptError {
	message: string;
	name: string;
	stack: ExtensionScriptStackFrame[];
}
interface ExtensionScriptResult {
	id: string;
	ok?: unknown;
	err?: string | ExtensionScriptError;
	// Set by the engine when the script touched the legacy UNO API:
	legacyUnoApi?: boolean;
}

window.L.Control.Extension = window.L.Control.extend({
	options: {
		// Identifier of the extension (its directory name under
		// browser/extensions/, used as the key in app.map._extensions and
		// as the dispatcher prefix extension-toggle-<id>).
		id: '',
		manifest: null as ExtensionManifest | null,
		// URL prefix (with trailing slash) under which the extension's `entry`,
		// `icon`, etc. are resolved.  Already routed through LOUtil.getURL.
		baseUrl: '',
	},

	_panel: null as HTMLDivElement | null,
	_iframe: null as HTMLIFrameElement | null,
	_teardownTimer: null as ReturnType<typeof setTimeout> | null,
	// Correlates executescript callIds issued directly by invokeCommand (menu clicks) with
	// their completion callbacks, so _onScriptResult can tell them apart from callIds that
	// originated inside the sidebar iframe and must be relayed back there instead.
	// invokeCommand is void and returns no Promise, so these are plain callbacks, not a
	// stashed resolve/reject pair - nothing awaits them.
	_pendingCommandCalls: null as {
		[callId: string]: {
			onSuccess: (value: unknown) => void;
			onError: (err: Error) => void;
		};
	} | null,
	_nextCommandCallId: 0,
	// One modal dialog per extension at a time.  origRemove holds the un-hooked
	// L.IFrameDialog.remove so _closeDialog can dismiss without re-entering the
	// user-close override installed in _openDialog.
	_dialog: null as null | {
		iframeDialog: any;
		origRemove: () => void;
		dialogId: string;
	},

	onAdd: function (map: any) {
		this.map = map;
		this._setToolitemHighlight(false);
		this._pendingCommandCalls = {};
		window.addEventListener('message', this._onPostMessage.bind(this));
		map.on('executescriptresult', this._onScriptResult, this);
		map.on('proxycall', this._onProxyCall, this);
		map.on('consolemsg', this._onConsoleMsg, this);
		map.on('comment', this._onComment, this);
	},

	_onConsoleMsg: function (e: { level: string; message: string }) {
		const fn = (console as any)[e.level];
		if (e.level === 'assert') {
			console.assert(false, e.message);
		} else if (typeof fn === 'function') {
			fn.call(console, e.message);
		} else {
			console.log('unkown level: ' + e.level + ', message: ' + e.message);
		}
	},

	// The extension iframe is our own content from the COOL origin.  The mobile and desktop
	// apps load over file://, where the iframe's origin is opaque and only '*' reaches it.
	_targetOrigin: function (): string {
		return window.origin.startsWith('http') ? window.origin : '*';
	},

	_postToIframe: function (payload: object) {
		if (!this._iframe || !this._iframe.contentWindow) return;
		this._iframe.contentWindow.postMessage(
			JSON.stringify(payload),
			this._targetOrigin(),
		);
	},

	// Forward LOK comment events (Add/Modify/Remove) to the iframe as Extension_DocumentEvent
	// postMessages with the corresponding event name; cool.js maps each to the matching
	// cool.document.onCommentXxx handler.
	_onComment: function (e: { comment?: { action?: string } }) {
		if (!this._iframe || !this._iframe.contentWindow || !e || !e.comment)
			return;
		let name: string;
		switch (e.comment.action) {
			case 'Add':
				name = 'commentAdded';
				break;
			case 'Modify':
				name = 'commentChanged';
				break;
			case 'Remove':
				name = 'commentRemoved';
				break;
			default:
				return;
		}
		this._postToIframe({
			msgId: 'Extension_DocumentEvent',
			name: name,
			payload: e.comment,
		});
	},

	_onProxyCall: function (e: {
		proxyId: string;
		callId?: string;
		method: string;
		args: unknown[];
	}) {
		this._postToIframe({
			msgId: 'Extension_ProxyCall',
			proxyId: e.proxyId,
			callId: e.callId,
			method: e.method,
			args: e.args,
		});
	},

	// Dispatcher entry point for a contributed menu command (docdispatcher's
	// `ext:<id>:<commandId>` branch).  Ships the command's script straight to the
	// kit's JS-UNO context via the same executescript wire message
	// _handleSidebarMessage's Extension_Call case uses, but without needing the
	// sidebar iframe to exist: the command runs whether or not this extension's
	// panel has ever been opened.
	invokeCommand: function (commandId: string): void {
		const commands = this.options.manifest.contributes
			? this.options.manifest.contributes.commands
			: undefined;
		const command =
			commands && commands.find((c: ExtensionCommand) => c.id === commandId);
		if (!command || command.source === undefined) {
			console.warn(
				'extension ' + this.options.id + ': unknown command ' + commandId,
			);
			return;
		}
		const callId = 'cmd-' + this.options.id + '-' + this._nextCommandCallId++;
		this._pendingCommandCalls[callId] = {
			onSuccess: function () {
				// Menu commands run for effect; nothing consumes their return
				// value today.
			},
			onError: (err: Error) => {
				console.error(
					'extension ' +
						this.options.id +
						': command ' +
						commandId +
						' failed:',
					err,
				);
				if (this.map.uiManager) {
					this.map.uiManager.showSnackbar(
						_('Extension command failed: %1').replace('%1', err.message),
					);
				}
			},
		};
		// Nothing clears this entry on its own if the kit never answers - document
		// teardown mid-command, a dropped socket, or the command itself hanging. Bound
		// how long it can wait, the same way _removePanel bounds the teardown handshake.
		setTimeout(() => {
			const pending = this._pendingCommandCalls[callId];
			if (!pending) return;
			delete this._pendingCommandCalls[callId];
			pending.onError(new Error('timed out waiting for a response'));
		}, 30000);
		// Wire format `executescript <id> <line> <source>\n<script>` (see
		// ChildSession::executeScript), matching what _handleSidebarMessage's
		// Extension_Call case sends for a cool.callRemote call: source/line let
		// a thrown exception's stack frames point back at the command's own file.
		// The script is command.source verbatim, with the call to invoke tacked on
		// after it rather than wrapped around it - nothing is prepended, so the
		// file's own line 1 column 1 stays line 1 column 1 in any reported frame.
		const source = (this.options.baseUrl + command.script).replace(/\n/g, '');
		app.socket.sendMessage(
			'executescript ' +
				callId +
				' 1 ' +
				source +
				'\n' +
				command.source +
				'\ncommands[' +
				JSON.stringify(commandId) +
				'].apply(null, []);',
		);
	},

	// Dispatcher entry point.  The notebookbar Extensions tab fires
	// extension-toggle-<id>.  An extension shows as its own sidebar deck, so the
	// toolitem toggles it the way the core deck buttons beside it do: clicking the
	// extension that is already showing takes it down again.
	toggle: function () {
		const sidebar = this.map.sidebar;
		if (!sidebar) return;

		if (sidebar.hasExtensionDeck(this)) {
			this._closeExtension();
			return;
		}

		if (this._panel) this._finishRemovePanel();
		this._showPanel();
		if (!this._panel) return;
		sidebar.takeExtensionDeckSlot(this);
		this._setToolitemHighlight(true);
	},

	_setToolitemHighlight: function (on: boolean) {
		const command = 'extension-toggle-' + this.options.id;
		const state = on ? 'true' : 'false';
		this.map['stateChangeHandler'].setItemValue(command, state);
		this.map.fire('commandstatechanged', {
			commandName: command,
			state: state,
		});
	},

	closeDeck: function () {
		if (this._panel) this._panel.classList.add('closing');
		this._setToolitemHighlight(false);
		this.map.sidebar.releaseExtensionDeckSlot(this);
		this._removePanel();
	},

	_closeExtension: function () {
		this.closeDeck();
		this.map.sidebar.closeSidebar();
		app.socket.sendMessage('uno .uno:SidebarHide');
	},

	_showPanel: function () {
		const manifest: ExtensionManifest = this.options.manifest;

		const sidebarPanel = document.getElementById('sidebar-panel');
		if (!sidebarPanel) return;

		const shell = JSDialog.buildOverlaySidebarPanel({
			id: 'extension-' + this.options.id,
			title: manifest.name,
			cssClass: 'jsdialog sidebar',
			map: this.map,
			onClose: this._closeExtension.bind(this),
		});
		const panel = shell.container;
		panel.classList.add('extension-panel');
		panel.dataset.extensionId = this.options.id;
		shell.content.classList.add('extension-panel-body');

		const iframe = document.createElement('iframe');
		iframe.src = this.options.baseUrl + manifest.entry;
		iframe.setAttribute(
			'sandbox',
			'allow-scripts allow-same-origin allow-forms allow-popups',
		);
		shell.content.appendChild(iframe);
		this._iframe = iframe;

		// Stop document-level shortcut handlers from intercepting keys/clicks
		// destined for the iframe contents:
		const stopProp = function (e: Event) {
			e.stopPropagation();
		};
		(
			[
				'keydown',
				'keyup',
				'keypress',
				'paste',
				'copy',
				'cut',
				'mousedown',
				'mouseup',
				'click',
			] as const
		).forEach(function (evt) {
			panel.addEventListener(evt, stopProp);
		});

		sidebarPanel.appendChild(panel);
		this._panel = panel;
	},

	_removePanel: function () {
		if (!this._panel) return;
		// If a teardown is already in flight, leave it to finish:
		if (this._teardownTimer !== null) return;
		// If the iframe isn't ready, skip the handshake (nothing to detach):
		if (this._iframe && this._iframe.contentWindow) {
			this._postToIframe({ msgId: 'Extension_Teardown' });
			this._teardownTimer = setTimeout(
				this._finishRemovePanel.bind(this),
				1000,
			);
		} else {
			this._finishRemovePanel();
		}
	},

	_finishRemovePanel: function () {
		if (this._teardownTimer !== null) {
			clearTimeout(this._teardownTimer);
			this._teardownTimer = null;
		}
		// A dialog owned by this extension outlives the sidebar iframe if we
		// don't dismiss it; no one would be left to receive the result either.
		if (this._dialog) {
			this._dialog.origRemove();
			this._dialog = null;
		}
		if (this._panel) {
			this._panel.remove();
			this._panel = null;
			this._iframe = null;
		}
	},

	_onPostMessage: function (e: MessageEvent) {
		// Route by the sender window: our sidebar iframe speaks the sidebar
		// half of the protocol, our dialog iframe (when one is open) speaks
		// the dialog half.  Silently ignore everything else (browser
		// extensions, dev tools, third-party libs).
		const fromSidebar = this._iframe && e.source === this._iframe.contentWindow;
		const fromDialog =
			this._dialog &&
			e.source === this._dialog.iframeDialog._iframe.contentWindow;
		if (!fromSidebar && !fromDialog) return;
		let msg: ExtensionMessage | null;
		try {
			msg = typeof e.data === 'string' ? JSON.parse(e.data) : e.data;
		} catch (ex) {
			console.warn('postMessage is not JSON: ' + e.data);
			return;
		}
		if (!msg || typeof msg !== 'object') {
			console.warn('postMessage is not an object: ' + JSON.stringify(msg));
			return;
		}
		if (fromSidebar) {
			this._handleSidebarMessage(msg as ExtensionSidebarMessage);
		} else {
			this._handleDialogMessage(msg as ExtensionDialogMessage);
		}
	},

	_handleSidebarMessage: function (msg: ExtensionSidebarMessage) {
		switch (msg.msgId) {
			case 'Extension_Call': {
				// Wire format `executescript <id> <line> <source>\n<script>`:
				app.socket.sendMessage(
					'executescript ' +
						msg.callId +
						' ' +
						(msg.line - 1) +
						' ' +
						msg.source.replace(/\n/g, '') +
						'\n(\n' +
						msg.fn +
						'\n).apply(null, ' +
						JSON.stringify(msg.args || []) +
						');',
				);
				break;
			}
			case 'Extension_ProxyReturn':
				app.socket.sendMessage(
					'proxyreturn ' +
						msg.callId +
						' ' +
						JSON.stringify(msg.value === undefined ? null : msg.value),
				);
				break;
			case 'Extension_Close':
				this._closeExtension();
				break;
			case 'Extension_TeardownDone':
				this._finishRemovePanel();
				break;
			case 'Extension_Resize':
				if (this._iframe) {
					this._iframe.style.height = msg.height + 'px';
				}
				break;
			case 'Extension_ShowDialog':
				this._openDialog(msg);
				break;
			default:
				console.warn('unexpected msgId: ' + (msg as any).msgId);
				break;
		}
	},

	_handleDialogMessage: function (msg: ExtensionDialogMessage) {
		switch (msg.msgId) {
			case 'Extension_DialogClose':
				this._closeDialog({ cancelled: false, value: msg.value });
				break;
			case 'Extension_DialogCancel':
				this._closeDialog({ cancelled: true });
				break;
			case 'Extension_Resize':
				// Dialog iframe reports its content's actual scrollHeight
				// (cool.js sends this on load and via ResizeObserver).  Fit
				// the iframe height so no empty space is left below.
				if (this._dialog) {
					this._dialog.iframeDialog._iframe.style.height = msg.height + 'px';
				}
				break;
			default:
				console.warn('unexpected msgId: ' + (msg as any).msgId);
				break;
		}
	},

	_openDialog: function (msg: ExtensionShowDialogMessage) {
		if (this._dialog) {
			// One modal at a time per extension; the second open() resolves as
			// cancelled straight away rather than queueing behind the first.
			this._postDialogResult(msg.dialogId, { cancelled: true });
			return;
		}
		const base = new URL(this.options.baseUrl, document.baseURI);
		const resolved = new URL(msg.url, base);
		// Check, just in case:
		if (!resolved.href.startsWith(base.href)) {
			console.warn(
				'Extension_ShowDialog: url [' +
					msg.url +
					'] resolves outside baseUrl [' +
					base.href +
					']; refusing',
			);
			this._postDialogResult(msg.dialogId, { cancelled: true });
			return;
		}
		const iframeOptions: any = {
			// Own prefix rather than iframe-dialog so extension-specific CSS
			// can stand on its own without disturbing the Feedback dialog.
			prefix: 'iframe-extension',
			titlebar: true,
			directSrc: true,
		};
		if (msg.title !== undefined) iframeOptions.title = msg.title;
		const iframeDialog = window.L.iframeDialog(
			resolved.href,
			{},
			null,
			iframeOptions,
		);
		// Size on the wrap, not on the iframe: the wrap is the visible dialog
		// box, and the iframe fills it via width: 100% and the height that
		// Extension_Resize keeps in sync with the picker's actual content.
		if (msg.width !== undefined) {
			iframeDialog._container.style.width = msg.width + 'px';
		}
		if (msg.height !== undefined) {
			iframeDialog._iframe.style.height = msg.height + 'px';
		}
		const origRemove = iframeDialog.remove.bind(iframeDialog);
		// Titlebar X and in-iframe Esc both call remove() directly.  Turn
		// either of those user gestures into a cancel result on the sidebar
		// side; the code path from _closeDialog uses origRemove and does not
		// re-enter this override.
		iframeDialog.remove = () => {
			origRemove();
			if (this._dialog && this._dialog.iframeDialog === iframeDialog) {
				const dialogId = this._dialog.dialogId;
				this._dialog = null;
				this._postDialogResult(dialogId, { cancelled: true });
			}
		};
		this._dialog = {
			iframeDialog: iframeDialog,
			origRemove: origRemove,
			dialogId: msg.dialogId,
		};
		iframeDialog.show();
	},

	_closeDialog: function (result: { cancelled: boolean; value?: unknown }) {
		if (!this._dialog) return;
		const dialogId = this._dialog.dialogId;
		const origRemove = this._dialog.origRemove;
		this._dialog = null;
		origRemove();
		this._postDialogResult(dialogId, result);
	},

	_postDialogResult: function (
		dialogId: string,
		result: { cancelled: boolean; value?: unknown },
	) {
		this._postToIframe({
			msgId: 'Extension_DialogResult',
			dialogId: dialogId,
			cancelled: result.cancelled,
			value: result.cancelled ? null : result.value,
		});
	},

	// Reconstructs a proper Error from the engine's jsuno::Exception payload, the
	// same way cool.js's makeStructuredError does for a cool.callRemote call: the
	// stack text lets the browser console show the command's own source location
	// for each frame rather than just a bare message.
	_toScriptError: function (err: string | ExtensionScriptError): Error {
		if (typeof err === 'string') return new Error(err);
		const e = new Error(err.message || '');
		e.name = err.name || 'Error';
		let stackText = e.name + ': ' + e.message;
		for (const f of err.stack || []) {
			stackText +=
				'\n    at ' +
				(f.functionName || '<anonymous>') +
				' (' +
				(f.source || '') +
				':' +
				f.line +
				':' +
				f.column +
				')';
		}
		e.stack = stackText;
		return e;
	},

	_onScriptResult: function (e: ExtensionScriptResult) {
		const pending = this._pendingCommandCalls[e.id];
		if (pending) {
			delete this._pendingCommandCalls[e.id];
			if (e.err !== undefined) pending.onError(this._toScriptError(e.err));
			else pending.onSuccess(e.ok);
		} else {
			this._postToIframe({
				msgId: 'Extension_CallResult',
				callId: e.id,
				ok: e.ok,
				err: e.err,
			});
		}
		if (e.legacyUnoApi) {
			this.map.uiManager.showLegacyUnoApiSnackbarOnce();
		}
	},
});

window.L.control.extension = function (
	id: string,
	manifest: ExtensionManifest,
	baseUrl: string,
) {
	return new window.L.Control.Extension({
		position: 'topleft',
		id: id,
		manifest: manifest,
		baseUrl: baseUrl,
	});
};

// If the directory carries appsscript.json, synthesize a manifest that hands the sidebar off
// to the shared gas-wrapper.html; the _cool-gas.json sidecar lists .gs sources and sidebar file:
async function tryLoadAppsScriptExtension(
	id: string,
	baseRel: string,
): Promise<ExtensionManifest | null> {
	const gasResp = await fetch(app.LOUtil.getURL(baseRel + 'appsscript.json'));
	if (!gasResp.ok) return null;
	let listing: {
		scripts?: string[];
		sidebar?: string;
		supports?: string[];
		name?: string;
		icon?: string;
	} = {};
	try {
		const listResp = await fetch(app.LOUtil.getURL(baseRel + '_cool-gas.json'));
		if (listResp.ok) listing = await listResp.json();
	} catch {
		// Missing sidecar is not fatal; the wrapper still loads the sidebar with no scripts.
	}
	const params = new URLSearchParams();
	params.set('base', app.LOUtil.getURL(baseRel));
	if (listing.sidebar) params.set('sidebar', listing.sidebar);
	if (listing.scripts && listing.scripts.length) {
		params.set('scripts', listing.scripts.join(','));
	}
	// The shared wrapper sits one directory above <id>/ so a leading "../" reaches it:
	const manifest: ExtensionManifest = {
		manifestVersion: '0.1',
		name: listing.name && listing.name.length ? listing.name : id,
		entry: '../gas-wrapper.html?' + params.toString(),
	};
	if (listing.icon) manifest.icon = listing.icon;
	if (listing.supports && listing.supports.length) {
		manifest.supports = listing.supports;
	}
	return manifest;
}

// Discover and register the JS extensions for this document by fetching three discovery indexes
// (built-in, admin preset, per-user preset), in that order, merging with per-user > admin >
// built-in precedence on ID collision, then loading each surviving manifest.json and registering
// one Control.Extension per entry on `map._extensions` (and manifests with an unsupported
// manifestVersion or that don't apply to the current docType are skipped/ with a console warning
// rather than aborting discovery):
window.L.loadExtensions = async function (map: any, docType: string) {
	// Gated on the experimental-features flag so deployments not opting in to
	// experimental functionality never fetch the discovery index:
	if (!window.enableExperimentalFeatures) return {};

	if (
		map.wopi &&
		(!map.wopi.UserCanWrite ||
			map.wopi.DisableCopy ||
			map.wopi.DisableExport ||
			map.wopi.DisablePrint)
	) {
		return {};
	}

	const sources: { baseRel: string; ids: string[] }[] = [];
	const fetchIndex = async (indexBase: string): Promise<string[]> => {
		try {
			const resp = await fetch(app.LOUtil.getURL(indexBase + 'index.json'));
			if (!resp.ok) throw new Error('HTTP ' + resp.status);
			return await resp.json();
		} catch (err) {
			console.warn(
				'extension discovery: ' + indexBase + 'index.json unreadable:',
				err,
			);
			return [];
		}
	};
	const addPresetSource = async (configId: string) => {
		const presetBase =
			'preset/' + encodeURIComponent(configId) + '/extensions/';
		const presetIds = await fetchIndex(presetBase);
		sources.push({ baseRel: presetBase, ids: presetIds });
	};
	const builtinIds = await fetchIndex('extensions/');
	sources.push({ baseRel: 'extensions/', ids: builtinIds });
	if (app.presetConfigId) await addPresetSource(app.presetConfigId);

	// Flatten the sources into one list of (id, baseRel) entries; Map.set's last-write-wins
	// behavior causes admin extensions to override built-in
	// ones:
	const byId = new Map<string, string>();
	for (const src of sources) {
		for (const id of src.ids) byId.set(id, src.baseRel);
	}

	// Fetch all the manifests in parallel; the loop afterwards just registers what survived:
	const loaded = await Promise.all(
		Array.from(byId.entries()).map(async ([id, baseSourceRel]) => {
			const baseRel = baseSourceRel + id + '/';
			try {
				const resp = await fetch(app.LOUtil.getURL(baseRel + 'manifest.json'));
				if (!resp.ok) throw new Error('HTTP ' + resp.status);
				const manifest: ExtensionManifest = await resp.json();
				// contributes is a string naming a separate JSON file (resolved the same way
				// entry/icon are) holding the actual object, keeping manifest.json itself
				// short and scannable regardless of how much UI an extension wires up.
				// Replace it with the fetched object here, once, so every later reader of
				// manifest.contributes (including the rest of this function) sees only the
				// object form and never has to know it started out as a path.
				if (manifest.contributes) {
					const uiPath = manifest.contributes as unknown as string;
					try {
						const uiResp = await fetch(app.LOUtil.getURL(baseRel + uiPath));
						if (!uiResp.ok) throw new Error('HTTP ' + uiResp.status);
						manifest.contributes = await uiResp.json();
					} catch (err) {
						console.warn(
							'extension ' +
								id +
								': contributes file "' +
								uiPath +
								'" unreadable:',
							err,
						);
						manifest.contributes = undefined;
					}
				}
				if (manifest.contributes && manifest.contributes.commands) {
					await Promise.all(
						manifest.contributes.commands.map(async (command) => {
							try {
								const scriptResp = await fetch(
									app.LOUtil.getURL(baseRel + command.script),
								);
								if (!scriptResp.ok)
									throw new Error('HTTP ' + scriptResp.status);
								command.source = await scriptResp.text();
							} catch (err) {
								console.warn(
									'extension ' +
										id +
										': command ' +
										command.id +
										' script unreadable:',
									err,
								);
							}
						}),
					);
				}
				return { id, baseRel, manifest };
			} catch (err) {
				try {
					const gasManifest = await tryLoadAppsScriptExtension(id, baseRel);
					if (gasManifest) return { id, baseRel, manifest: gasManifest };
				} catch (gasErr) {
					console.warn('extension ' + id + ': failed to load:', gasErr);
					return null;
				}
				console.warn('extension ' + id + ': failed to load:', err);
				return null;
			}
		}),
	);
	const exts: { [id: string]: any } = {};
	for (const entry of loaded) {
		if (entry === null) continue;
		const { id, baseRel, manifest } = entry;
		if (manifest.manifestVersion !== '0.1') {
			console.warn(
				'extension ' +
					id +
					': unsupported manifestVersion ' +
					manifest.manifestVersion,
			);
			continue;
		}
		if (
			manifest.supports &&
			manifest.supports.length > 0 &&
			!manifest.supports.includes(docType)
		) {
			continue;
		}
		const ext = window.L.control.extension(
			id,
			manifest,
			app.LOUtil.getURL(baseRel),
		);
		map.addControl(ext);
		exts[id] = ext;
	}

	return exts;
};
