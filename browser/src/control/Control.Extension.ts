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
	// Kit-side command: the script file holding the `commands` object.
	script?: string;
	source?: string;
	// Panel command: instead of running a script in the kit, open the
	// extension's sidebar panel (if it isn't open) and deliver the command
	// id to it as an Extension_Command postMessage; cool.js hands it to
	// cool.onCommand. For extensions whose logic lives in the panel.
	panel?: boolean;
	gasFunctionName?: string;
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

// A labeled cluster of notebookbar items, corresponding to one built-in group
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
// of brand-new tabs the extension adds to the notebookbar; each names its
// own `tab` label, positions itself relative to an existing tab via
// `insertBefore`/`insertAfter` (mutually exclusive; omitting both appends it
// at the end), and lays out its own content as `groups` of the three item
// kinds above - not raw notebookbar item JSON, so an extension can't place
// arbitrary layout into the notebookbar.
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

// One entry of contributes.extensionsMenu: a command to offer, or a divider between groups of
// them.
type ExtensionMenuEntry = { command: string } | { separator: true };

interface ExtensionContributes {
	commands?: ExtensionCommand[];
	// Commands offered under the extension's own name in the Extensions menu and the
	// Extensions notebookbar tab, in this order. For an extension whose commands are its
	// whole user interface, rather than one placing them in a document menu of its own
	// choosing with `menus` below.
	extensionsMenu?: ExtensionMenuEntry[];
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
	isGasExtension?: boolean;
	gasContext?: {
		sources: string[];
		names: string[];
		runnerExpr: string;
	};
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

interface ExtensionSaveFileMessage {
	msgId: 'Extension_SaveFile';
	saveId: string;
	filename: string;
	mimeType: string;
	bytes: number[];
}

interface ExtensionOpenSidebarMessage {
	msgId: 'Extension_OpenSidebar';
	sidebarFile: string;
}

// The dialog an Apps Script add-on asked for through one of the ui.show*Dialog calls: its HTML
// file under the add-on directory, and for a page made from a template, the properties the add-on
// set on that template:
interface GasDialogSpec {
	file: string;
	title?: string;
	width?: number;
	height?: number;
	templateValues?: object;
}

interface ExtensionShowGasDialogMessage {
	msgId: 'Extension_ShowGasDialog';
	dialog: GasDialogSpec;
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
	| ExtensionShowDialogMessage
	| ExtensionSaveFileMessage
	| ExtensionOpenSidebarMessage
	| ExtensionShowGasDialogMessage;

type ExtensionDialogMessage =
	| ExtensionCallMessage
	| ExtensionProxyReturnMessage
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
	// dialogId is null for an Apps Script dialog, which has no cool.dialog.open() call
	// waiting for its result.
	_dialog: null as null | {
		iframeDialog: any;
		origRemove: () => void;
		dialogId: string | null;
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

	_postToIframe: function (payload: object): boolean {
		if (!this._iframe || !this._iframe.contentWindow) return false;
		this._iframe.contentWindow.postMessage(
			JSON.stringify(payload),
			this._targetOrigin(),
		);
		return true;
	},

	// A call or proxy id that cool.js made in the dialog iframe starts with "dlg-", and every
	// other one comes from the sidebar iframe:
	_postToCaller: function (id: string, payload: object): boolean {
		if (!id.startsWith('dlg-')) {
			return this._postToIframe(payload);
		}
		const frame = this._dialog && this._dialog.iframeDialog._iframe;
		if (!frame || !frame.contentWindow) return false;
		frame.contentWindow.postMessage(
			JSON.stringify(payload),
			this._targetOrigin(),
		);
		return true;
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
		if (gasProxies[e.proxyId]) return;
		const delivered = this._postToCaller(e.proxyId, {
			msgId: 'Extension_ProxyCall',
			proxyId: e.proxyId,
			callId: e.callId,
			method: e.method,
			args: e.args,
		});
		if (!e.callId) return;
		// Every Extension control sees every proxy call, and a call that none of them can pass
		// to a frame, as after the dialog making it has closed, returns null to its caller in
		// the kit once all of them have seen it:
		const callId = e.callId;
		let delivery = proxyCallDeliveries.get(callId);
		if (!delivery) {
			delivery = { delivered: false };
			proxyCallDeliveries.set(callId, delivery);
			const pending = delivery;
			queueMicrotask(() => {
				proxyCallDeliveries.delete(callId);
				if (!pending.delivered) {
					app.socket.sendMessage('proxyreturn ' + callId + ' null');
				}
			});
		}
		if (delivered) delivery.delivered = true;
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
		if (command && command.panel) {
			this.invokePanelCommand(commandId);
			return;
		}
		if (command && command.gasFunctionName) {
			this._invokeGasCommand(command);
			return;
		}
		if (!command || command.source === undefined) {
			console.warn(
				'extension ' + this.options.id + ': unknown command ' + commandId,
			);
			return;
		}
		const callId = 'cmd-' + this.options.id + '-' + this._nextCommandCallId++;
		this._pendingCommandCalls[callId] = {
			onSuccess: (value: unknown) => {
				// A result marked __coolGas carries the messages an Apps Script add-on
				// passed to getUi().alert(); the sidebar iframe shows those as a banner,
				// and a command invoked from a menu has the snackbar instead.
				const envelope = value as {
					__coolGas?: boolean;
					alerts?: { title?: string; message?: string }[];
				} | null;
				if (!envelope || envelope.__coolGas !== true) return;
				for (const alert of envelope.alerts || []) {
					if (!this.map.uiManager) return;
					this.map.uiManager.showSnackbar(
						alert.title ? alert.title + ': ' + alert.message : alert.message,
					);
				}
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
				'].apply(null, [' +
				JSON.stringify(commandContext()) +
				']);',
		);
	},

	_invokeGasCommand: function (command: ExtensionCommand): void {
		const manifest = this.options.manifest as ExtensionManifest;
		if (!manifest.gasContext) {
			console.warn(
				'extension ' +
					this.options.id +
					': command ' +
					command.id +
					' has no cached gasContext',
			);
			return;
		}
		const seq = this._nextCommandCallId++;
		const callId = 'cmd-' + this.options.id + '-' + seq;
		const proxyId = 'gas-cmd-proxy-' + this.options.id + '-' + seq;
		registerGasProxy(this.map, proxyId, this.options.id);
		const releaseProxy = () => {
			delete gasProxies[proxyId];
		};
		this._pendingCommandCalls[callId] = {
			onSuccess: (value: unknown) => {
				releaseProxy();
				// Show each getUi().alert() as a snackbar; open the sidebar if a
				// ui.showSidebar named a file:
				const envelope = value as {
					__coolGas?: boolean;
					alerts?: { title?: string; message?: string }[];
					sidebarFile?: string;
					dialog?: GasDialogSpec | null;
				} | null;
				if (!envelope || envelope.__coolGas !== true) return;
				for (const alert of envelope.alerts || []) {
					if (!this.map.uiManager) break;
					this.map.uiManager.showSnackbar(
						alert.title ? alert.title + ': ' + alert.message : alert.message,
					);
				}
				if (typeof envelope.sidebarFile === 'string' && envelope.sidebarFile) {
					this._openPanel(envelope.sidebarFile);
				}
				if (envelope.dialog) {
					this._openGasDialog(envelope.dialog);
				}
			},
			onError: (err: Error) => {
				releaseProxy();
				console.error(
					'extension ' +
						this.options.id +
						': command ' +
						command.id +
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
		setTimeout(() => {
			const pending = this._pendingCommandCalls[callId];
			if (!pending) return;
			delete this._pendingCommandCalls[callId];
			pending.onError(new Error('timed out waiting for a response'));
		}, 30000);
		const gc = manifest.gasContext;
		const args =
			'[' +
			JSON.stringify(proxyId) +
			', ' +
			JSON.stringify(gc.sources) +
			', ' +
			JSON.stringify(gc.names) +
			', ' +
			JSON.stringify(command.gasFunctionName) +
			', [], ' +
			JSON.stringify(this.options.id) +
			']';
		app.socket.sendMessage(
			'executescript ' +
				callId +
				' 1 gas-kit-runner.js\n(\n' +
				gc.runnerExpr +
				'\n).apply(null, ' +
				args +
				');',
		);
	},

	// A `panel: true` command: make sure the sidebar panel is showing, then
	// hand the command id to the iframe. The iframe may still be loading, in
	// which case the message goes out once it has loaded.
	invokePanelCommand: function (commandId: string): void {
		const sidebar = this.map.sidebar;
		if (!sidebar) return;
		const deliver = () => {
			this._postToIframe({ msgId: 'Extension_Command', commandId: commandId });
		};
		if (sidebar.hasExtensionDeck(this) && this._iframe) {
			deliver();
			return;
		}
		if (this._panel) this._finishRemovePanel();
		this._showPanel();
		if (!this._panel) return;
		sidebar.takeExtensionDeckSlot(this);
		this._setToolitemHighlight(true);
		if (this._iframe) {
			this._iframe.addEventListener('load', deliver, { once: true });
		}
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

	// The URL of gas-wrapper.html showing the add-on page named file, with params carrying any
	// further query parameters:
	_gasWrapperUrl: function (file: string, params: URLSearchParams): string {
		const manifest: ExtensionManifest = this.options.manifest;
		params.set('base', new URL(this.options.baseUrl, document.baseURI).href);
		if (manifest.gasContext && manifest.gasContext.names.length) {
			params.set('scripts', manifest.gasContext.names.join(','));
		}
		params.set('sidebar', file + (/\.html?$/i.test(file) ? '' : '.html'));
		return new URL(
			this.options.baseUrl + '../gas-wrapper.html?' + params.toString(),
			document.baseURI,
		).href;
	},

	_openGasDialog: function (spec: GasDialogSpec) {
		// One modal at a time per extension, and the add-on's second request is dropped:
		if (this._dialog) {
			console.warn(
				'extension ' +
					this.options.id +
					': dialog ' +
					spec.file +
					' requested while another one is open',
			);
			return;
		}
		const params = new URLSearchParams();
		params.set('coolRole', 'dialog');
		if (spec.templateValues !== undefined) {
			params.set('template', JSON.stringify(spec.templateValues));
		}
		this._showDialogFrame(this._gasWrapperUrl(spec.file, params), {
			dialogId: null,
			title: spec.title,
			width: spec.width,
			height: spec.height,
		});
	},

	_showPanel: function (sidebarFile?: string) {
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

		// A GAS add-on has no manifest.entry; the iframe URL comes from gasContext plus the
		// ui.showSidebar argument the calling menu command captured in sidebarFile:
		let entryUrl: string;
		if (manifest.isGasExtension) {
			if (!manifest.gasContext || !sidebarFile) return;
			entryUrl = this._gasWrapperUrl(sidebarFile, new URLSearchParams());
		} else {
			entryUrl = this.options.baseUrl + manifest.entry;
		}
		const iframe = document.createElement('iframe');
		iframe.src = withUiLanguage(entryUrl);
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

	// The sidebar and the dialog iframe both send these on to the kit:
	_forwardToKit: function (
		msg: ExtensionCallMessage | ExtensionProxyReturnMessage,
	) {
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
		}
	},

	_handleSidebarMessage: function (msg: ExtensionSidebarMessage) {
		switch (msg.msgId) {
			case 'Extension_Call':
			case 'Extension_ProxyReturn':
				this._forwardToKit(msg);
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
			case 'Extension_SaveFile':
				this._saveFile(msg);
				break;
			case 'Extension_OpenSidebar':
				this._openPanel(msg.sidebarFile);
				break;
			case 'Extension_ShowGasDialog':
				this._openGasDialog(msg.dialog);
				break;
			default:
				console.warn('unexpected msgId: ' + (msg as any).msgId);
				break;
		}
	},

	// Save a file an extension generated. In the desktop app (CODA) it goes to
	// the native side, which shows a save panel; in the browser it downloads.
	// The result ("download" / "filesystem") is posted back so the extension's
	// cool.saveFile promise settles.
	_saveFile: function (msg: ExtensionSaveFileMessage) {
		const reply = (how: string, err?: string) => {
			this._postToIframe({
				msgId: 'Extension_SaveFileResult',
				saveId: msg.saveId,
				how: how,
				err: err,
			});
		};
		let bytes: Uint8Array;
		try {
			bytes = Uint8Array.from(msg.bytes || []);
		} catch (e) {
			reply(undefined, 'invalid file content');
			return;
		}
		const filename = (msg.filename || 'file').replace(/^.*[\\/]/, '');
		if (window.ThisIsAMobileApp) {
			// Hand the bytes to the native app (see the CODA/iOS extensionsavefile
			// handler), base64 so the message stays text.
			let binary = '';
			for (let i = 0; i < bytes.length; i++)
				binary += String.fromCharCode(bytes[i]);
			window.postMobileMessage(
				'extensionsavefile name=' +
					encodeURIComponent(filename) +
					' mime=' +
					encodeURIComponent(msg.mimeType || 'application/octet-stream') +
					' data=' +
					btoa(binary),
			);
			// The native side owns the save panel; report the platform, not success.
			reply('filesystem');
			return;
		}
		try {
			const blob = new Blob([bytes], {
				type: msg.mimeType || 'application/octet-stream',
			});
			const url = URL.createObjectURL(blob);
			const a = document.createElement('a');
			a.href = url;
			a.download = filename;
			document.body.appendChild(a);
			a.click();
			a.remove();
			setTimeout(() => URL.revokeObjectURL(url), 10000);
			reply('download');
		} catch (e) {
			reply(undefined, (e as Error).message);
		}
	},

	_handleDialogMessage: function (msg: ExtensionDialogMessage) {
		switch (msg.msgId) {
			case 'Extension_Call':
			case 'Extension_ProxyReturn':
				this._forwardToKit(msg);
				break;
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
		resolved.searchParams.set('coolRole', 'dialog');
		this._showDialogFrame(resolved.href, msg);
	},

	_showDialogFrame: function (
		url: string,
		msg: {
			dialogId: string | null;
			title?: string;
			width?: number;
			height?: number;
		},
	) {
		const iframeOptions: any = {
			// Own prefix rather than iframe-dialog so extension-specific CSS
			// can stand on its own without disturbing the Feedback dialog.
			prefix: 'iframe-extension',
			titlebar: true,
			directSrc: true,
		};
		if (msg.title !== undefined) iframeOptions.title = msg.title;
		const iframeDialog = window.L.iframeDialog(
			withUiLanguage(url),
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
		dialogId: string | null,
		result: { cancelled: boolean; value?: unknown },
	) {
		if (dialogId === null) return;
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
			this._postToCaller(e.id, {
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

	_openPanel: function (sidebarFile?: string): void {
		const sidebar = this.map.sidebar;
		if (!sidebar) return;
		if (sidebar.hasExtensionDeck(this)) return;
		if (this._panel) this._finishRemovePanel();
		this._showPanel(sidebarFile);
		if (!this._panel) return;
		sidebar.takeExtensionDeckSlot(this);
		this._setToolitemHighlight(true);
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

// The kit-side text behind every command of an Apps Script add-on: the runner, followed by a
// `commands` entry per menu function that hands the add-on's own sources to it.  The sources
// travel as string literals rather than being pasted in, because the runner evaluates each one
// under its own file name so an exception's frames name the add-on's file.  The runner comes
// first and nothing is prepended to it, so its line 1 stays line 1.
// If the directory carries appsscript.json, synthesize a manifest for its Apps Script
// add-on from what a __coolGasMenu invocation of the runner brings back at extension-load
// time; the _cool-gas.json sidecar lists .gs sources:
async function tryLoadAppsScriptExtension(
	id: string,
	baseRel: string,
	map: any,
): Promise<ExtensionManifest | null> {
	const gasResp = await fetch(app.LOUtil.getURL(baseRel + 'appsscript.json'));
	if (!gasResp.ok) return null;
	let listing: {
		scripts?: string[];
		supports?: string[];
		name?: string;
		icon?: string;
	} = {};
	try {
		const listResp = await fetch(app.LOUtil.getURL(baseRel + '_cool-gas.json'));
		if (listResp.ok) listing = await listResp.json();
	} catch {
		// Missing sidecar is not fatal; discovery continues with no scripts.
	}
	const manifest: ExtensionManifest = {
		manifestVersion: '0.1',
		name: listing.name && listing.name.length ? listing.name : id,
		isGasExtension: true,
	};
	if (listing.icon) manifest.icon = listing.icon;
	if (listing.supports && listing.supports.length) {
		manifest.supports = listing.supports;
	}
	const scriptNames = listing.scripts || [];

	let items: {
		caption?: string;
		functionName?: string;
		separator?: boolean;
	}[] = [];
	if (scriptNames.length) {
		try {
			items = await collectGasAddonMenu(id, baseRel, scriptNames, map);
		} catch (err) {
			console.warn(
				'extension ' + id + ': __coolGasMenu invocation failed:',
				err,
			);
		}
	}

	// The add-on menu the runner brought back becomes one command per item, offered under
	// the add-on's name where an editor add-on's menu belongs.  (Two items pointing at the same
	// GAS function each keep their own caption by getting their own command id.)
	const commands: ExtensionCommand[] = [];
	const placement: ExtensionMenuEntry[] = [];
	const usedIds = new Set<string>();
	for (const item of items) {
		if (!item.functionName) {
			// Two dividers in a row, or one before any item, would render as a stray line.
			if (
				placement.length &&
				!('separator' in placement[placement.length - 1])
			) {
				placement.push({ separator: true });
			}
			continue;
		}
		let commandId = item.functionName;
		let n = 2;
		while (usedIds.has(commandId)) {
			commandId = item.functionName + ':' + n;
			n++;
		}
		usedIds.add(commandId);
		commands.push({
			id: commandId,
			title: item.caption || item.functionName,
			gasFunctionName: item.functionName,
		});
		placement.push({ command: commandId });
	}
	while (placement.length && 'separator' in placement[placement.length - 1]) {
		placement.pop();
	}

	// Cache the runner source and the .gs sources so a menu click doesn't refetch them:
	if (commands.length && scriptNames.length) {
		try {
			const [runnerExpr, sources] = await Promise.all([
				loadGasRunnerExpr(baseRel),
				Promise.all(
					scriptNames.map(async (name) => {
						const resp = await fetch(app.LOUtil.getURL(baseRel + name));
						if (!resp.ok) {
							throw new Error(baseRel + name + ' HTTP ' + resp.status);
						}
						return await resp.text();
					}),
				),
			]);
			manifest.gasContext = {
				sources: sources,
				names: scriptNames,
				runnerExpr: runnerExpr,
			};
			manifest.contributes = { commands: commands, extensionsMenu: placement };
		} catch (err) {
			console.warn(
				'extension ' + id + ': Apps Script sources unreadable:',
				err,
			);
		}
	}
	return manifest;
}

// Cached runner function expression, shared across all Apps Script extensions:
let gasRunnerExpr: Promise<string> | null = null;
function loadGasRunnerExpr(baseRel: string): Promise<string> {
	if (gasRunnerExpr !== null) return gasRunnerExpr;
	// The runner file lives one directory above the extension dir, alongside gas-wrapper.html:
	const url = app.LOUtil.getURL(baseRel + '../gas-kit-runner.js');
	gasRunnerExpr = (async () => {
		const resp = await fetch(url);
		if (!resp.ok) throw new Error('gas-kit-runner.js HTTP ' + resp.status);
		const src = await resp.text();
		// Strip the `globalThis.__gasKitRunner =` prefix and the trailing semicolon so what
		// remains is a bare `function(...) { ... }` expression the kit can wrap in an
		// IIFE call, matching how cool.callRemote ships its runner:
		const m = src.match(
			/globalThis\.__gasKitRunner\s*=\s*(function[\s\S]*?);\s*$/,
		);
		if (!m) {
			throw new Error('gas-kit-runner.js: __gasKitRunner assignment not found');
		}
		return m[1];
	})();
	return gasRunnerExpr;
}
// Ask the runner for the menu items the add-on's onOpen() puts together, at extension load:
let nextGasCollectId = 0;
// The proxy calls that the Extension controls are passing on, by call id, each with whether one of
// the controls found a frame for it:
const proxyCallDeliveries = new Map<string, { delivered: boolean }>();

type GasProxyHandlers = { [method: string]: (...args: unknown[]) => unknown };

// The XClientRuntime proxies that the main window serves itself, by proxy id, each with the id of
// the extension whose runner calls it:
const gasProxies: {
	[proxyId: string]: { extensionId: string; handlers: GasProxyHandlers };
} = {};
let gasProxyMap: any = null;

function registerGasProxy(
	map: any,
	proxyId: string,
	extensionId: string,
): void {
	if (gasProxyMap !== map) {
		if (gasProxyMap) gasProxyMap.off('proxycall', answerGasProxyCall);
		map.on('proxycall', answerGasProxyCall);
		gasProxyMap = map;
	}
	gasProxies[proxyId] = {
		extensionId: extensionId,
		handlers: makeGasProxyHandlers(extensionId),
	};
}

function answerGasProxyCall(e: {
	proxyId: string;
	callId?: string;
	method: string;
	args: unknown[];
}): void {
	const proxy = gasProxies[e.proxyId];
	if (!proxy) return;
	const handler = proxy.handlers;
	const send = (value: unknown) => {
		if (!e.callId) return;
		app.socket.sendMessage(
			'proxyreturn ' +
				e.callId +
				' ' +
				JSON.stringify(value === undefined ? null : value),
		);
	};
	const onThrow = (err: unknown) => {
		console.warn(
			'extension ' +
				proxy.extensionId +
				': proxy method ' +
				e.method +
				' threw:',
			err,
		);
		send(null);
	};
	const fn = handler[e.method];
	try {
		// A handler may return a Promise (urlFetch does): wait for it before
		// sending proxyreturn, so a non-void async method blocks the kit-side
		// caller for the real answer instead of getting null immediately.
		const result = fn ? fn(...e.args) : null;
		if (result && typeof (result as { then?: unknown }).then === 'function') {
			(result as Promise<unknown>).then(send, onThrow);
		} else {
			send(result);
		}
	} catch (err) {
		onThrow(err);
	}
}

function makeGasProxyHandlers(extensionId: string): GasProxyHandlers {
	const prefix = 'gas-user-props:' + extensionId + ':';
	const propKeys = () => {
		const out: string[] = [];
		for (let i = 0; i < localStorage.length; ++i) {
			const k = localStorage.key(i);
			if (k !== null && k.indexOf(prefix) === 0) {
				out.push(k.substring(prefix.length));
			}
		}
		return out;
	};
	return {
		translate: () => {
			throw new Error(
				'LanguageApp.translate is not supported in the COOL Apps Script wrapper',
			);
		},
		userPropGetProperty: (...args: unknown[]) => {
			const key = String(args[0]);
			const raw = localStorage.getItem(prefix + key);
			return { IsPresent: raw !== null, Value: raw === null ? '' : raw };
		},
		userPropSetProperty: (...args: unknown[]) => {
			localStorage.setItem(prefix + String(args[0]), String(args[1]));
		},
		userPropDeleteProperty: (...args: unknown[]) => {
			localStorage.removeItem(prefix + String(args[0]));
		},
		userPropGetKeys: () => propKeys(),
		userPropDeleteAll: () => {
			for (const k of propKeys()) localStorage.removeItem(prefix + k);
		},
		urlFetch: async (...args: unknown[]) => {
			const url = String(args[0]);
			const method = String(args[1]);
			const contentType = String(args[2]);
			const payload = String(args[3]);
			const payloadIsBase64 = Boolean(args[4]);
			const headerNames = args[5] as string[];
			const headerValues = args[6] as string[];
			const followRedirects = Boolean(args[7]);
			const headers: { [k: string]: string } = {};
			for (let i = 0; i < headerNames.length; ++i) {
				headers[headerNames[i]] = headerValues[i];
			}
			if (contentType) headers['Content-Type'] = contentType;
			try {
				const body =
					method === 'GET' || method === 'HEAD'
						? undefined
						: payloadIsBase64
							? Uint8Array.from(atob(payload), (c) => c.charCodeAt(0))
							: payload;
				const resp = await fetch(url, {
					method: method,
					headers: headers,
					body: body,
					redirect: followRedirects ? 'follow' : 'manual',
				});
				const buf = await resp.arrayBuffer();
				const bytes = new Uint8Array(buf);
				let binary = '';
				for (let i = 0; i < bytes.length; i += 0x8000) {
					binary += String.fromCharCode.apply(
						null,
						Array.from(bytes.subarray(i, i + 0x8000)),
					);
				}
				const outNames: string[] = [];
				const outValues: string[] = [];
				resp.headers.forEach((v, k) => {
					outNames.push(k);
					outValues.push(v);
				});
				return {
					code: resp.status,
					headerNames: outNames,
					headerValues: outValues,
					body: btoa(binary),
					error: '',
				};
			} catch (err) {
				return {
					code: 0,
					headerNames: [],
					headerValues: [],
					body: '',
					error:
						err && (err as Error).message
							? (err as Error).message
							: String(err),
				};
			}
		},
	};
}

async function collectGasAddonMenu(
	id: string,
	baseRel: string,
	scriptNames: string[],
	map: any,
): Promise<{ caption?: string; functionName?: string; separator?: boolean }[]> {
	const [runnerExpr, sources] = await Promise.all([
		loadGasRunnerExpr(baseRel),
		Promise.all(
			scriptNames.map(async (s) => {
				const resp = await fetch(app.LOUtil.getURL(baseRel + s));
				if (!resp.ok) throw new Error(baseRel + s + ' HTTP ' + resp.status);
				return await resp.text();
			}),
		),
	]);
	const seq = nextGasCollectId++;
	const callId = 'gas-collect-' + id + '-' + seq;
	const proxyId = 'gas-collect-proxy-' + id + '-' + seq;
	registerGasProxy(map, proxyId, id);
	return new Promise((resolve, reject) => {
		const handler = (result: any) => {
			if (result.id !== callId) return;
			map.off('executescriptresult', handler);
			delete gasProxies[proxyId];
			if (result.err) {
				const msg =
					(result.err && result.err.message) ||
					String(result.err) ||
					'GAS collect failed';
				reject(new Error(msg));
				return;
			}
			// The runner wraps every return in a __coolGas envelope whose value is the menu:
			const envelope = result.ok as {
				__coolGas?: boolean;
				value?: unknown;
			} | null;
			const value =
				envelope && envelope.__coolGas === true ? envelope.value : result.ok;
			resolve(Array.isArray(value) ? value : []);
		};
		map.on('executescriptresult', handler);
		const args =
			'[' +
			JSON.stringify(proxyId) +
			', ' +
			JSON.stringify(sources) +
			', ' +
			JSON.stringify(scriptNames) +
			', "__coolGasMenu", [], ' +
			JSON.stringify(id) +
			']';
		app.socket.sendMessage(
			'executescript ' +
				callId +
				' 1 gas-kit-runner.js\n(\n' +
				runnerExpr +
				'\n).apply(null, ' +
				args +
				');',
		);
	});
}

// --- Localization ------------------------------------------------------------------------------
//
// Extensions are translated with gettext catalogs, like the rest of the UI: an
// extension ships `l10n/<lang>.json` files (built by util/po2json.py from its
// po/<lang>.po, see browser/extensions/README.md). The English strings in
// manifest.json and the contributes file are the msgids. Discovery loads the
// catalog for the UI language once per extension and translates the manifest
// in place, so every consumer (notebookbar, menus, context menu, panel header)
// sees translated text without knowing about catalogs. The panel iframe and
// kit-side commands get the language too, and cool.js loads the same catalog
// for the panel's own strings.

// The UI language as COOL knows it, e.g. "en-US", "pt-BR", "de".
function uiLanguage(): string {
	return window.langParam || 'en-US';
}

function uiDirection(): string {
	return document.documentElement.dir === 'rtl' ? 'rtl' : 'ltr';
}

// Catalog file names use the po file convention: pt_BR, zh_CN, ca_valencia.
// For "pt-BR" try pt_BR then pt; for "de" just de. English has no catalog.
function catalogCandidates(lang: string): string[] {
	const norm = lang.replace(/-/g, '_');
	const parts = norm.split('_');
	if (parts[0] === 'en') return [];
	const out = [norm];
	if (parts.length > 1) out.push(parts[0]);
	return out;
}

// Append the UI language and direction to an extension page URL.
function withUiLanguage(url: string): string {
	const u = new URL(url, document.baseURI);
	u.searchParams.set('lang', uiLanguage());
	u.searchParams.set('dir', uiDirection());
	return u.href;
}

// The object passed to kit-side command functions as their argument.
function commandContext(): { lang: string; dir: string } {
	return { lang: uiLanguage(), dir: uiDirection() };
}

const catalogCache: {
	[baseRel: string]: Promise<{ [msgid: string]: string } | null>;
} = {};

// Fetch the extension's catalog for the UI language, or null when there is
// none. 404s are the normal case for untranslated extensions and are silent.
function loadExtensionCatalog(
	baseRel: string,
): Promise<{ [msgid: string]: string } | null> {
	if (catalogCache[baseRel]) return catalogCache[baseRel];
	catalogCache[baseRel] = (async () => {
		// Fetch the candidates (pt_BR, pt) together; the most specific one
		// that exists wins.
		const fetched = await Promise.all(
			catalogCandidates(uiLanguage()).map(async (cand) => {
				try {
					const resp = await fetch(
						app.LOUtil.getURL(baseRel + 'l10n/' + cand + '.json'),
					);
					if (!resp.ok) return null;
					const catalog = await resp.json();
					return catalog && typeof catalog === 'object' ? catalog : null;
				} catch (err) {
					return null;
				}
			}),
		);
		return fetched.find((catalog) => catalog !== null) || null;
	})();
	return catalogCache[baseRel];
}

// Translate the user-visible strings of a manifest (and its resolved
// contributes object) in place; msgids without a translation stay English.
async function localizeManifest(
	manifest: ExtensionManifest,
	baseRel: string,
): Promise<void> {
	const catalog = await loadExtensionCatalog(baseRel);
	if (!catalog) return;
	const tr = (s: string | undefined): string | undefined =>
		s !== undefined &&
		Object.prototype.hasOwnProperty.call(catalog, s) &&
		catalog[s] !== ''
			? catalog[s]
			: s;
	manifest.name = tr(manifest.name);
	const c = manifest.contributes;
	if (!c) return;
	if (c.commands) for (const cmd of c.commands) cmd.title = tr(cmd.title);
	if (c.notebookbar) {
		for (const tab of c.notebookbar) {
			tab.tab = tr(tab.tab);
			for (const group of tab.groups || []) {
				group.label = tr(group.label);
				for (const item of group.items || []) {
					if (item.type === 'menu') item.title = tr(item.title);
				}
			}
		}
	}
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
							// Panel commands have no kit-side script to fetch.
							if (command.panel) return;
							if (!command.script) {
								console.warn(
									'extension ' +
										id +
										': command ' +
										command.id +
										' has neither script nor panel',
								);
								return;
							}
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
				await localizeManifest(manifest, baseRel);
				return { id, baseRel, manifest };
			} catch (err) {
				try {
					const gasManifest = await tryLoadAppsScriptExtension(
						id,
						baseRel,
						map,
					);
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
