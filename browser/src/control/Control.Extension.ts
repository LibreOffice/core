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
 *     Closes the extension sidebar.
 *
 * Before the iframe is torn down (either via Extension_Close or
 * the close X on the panel) COOL posts:
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
 */

/* global app */

interface ExtensionManifest {
	manifestVersion: string;
	name: string;
	entry: string;
	icon?: string;
	supports?: string[];
}

interface ExtensionCallMessage {
	msgId: 'Extension_Call';
	callId: string;
	fn: string;
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

type ExtensionMessage =
	| ExtensionCallMessage
	| ExtensionCloseMessage
	| ExtensionProxyReturnMessage
	| ExtensionTeardownDoneMessage
	| ExtensionResizeMessage;

interface ExtensionScriptResult {
	id: string;
	ok?: unknown;
	err?: string;
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

	onAdd: function (map: any) {
		this.map = map;
		window.addEventListener('message', this._onPostMessage.bind(this));
		map.on('executescriptresult', this._onScriptResult, this);
		map.on('proxycall', this._onProxyCall, this);
		map.on('comment', this._onComment, this);
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
		this._iframe.contentWindow.postMessage(
			JSON.stringify({
				msgId: 'Extension_DocumentEvent',
				name: name,
				payload: e.comment,
			}),
			'*',
		);
	},

	_onProxyCall: function (e: {
		proxyId: string;
		callId?: string;
		method: string;
		args: unknown[];
	}) {
		if (!this._iframe || !this._iframe.contentWindow) return;
		this._iframe.contentWindow.postMessage(
			JSON.stringify({
				msgId: 'Extension_ProxyCall',
				proxyId: e.proxyId,
				callId: e.callId,
				method: e.method,
				args: e.args,
			}),
			'*',
		);
	},

	// Dispatcher entry point.  The Extensions tab fires
	// extension-toggle-<id>, but the X button on the panel is the only
	// way to remove the extension; here a click on the toolitem just
	// ensures the panel is present and unfolded inside the sidebar.
	toggle: function () {
		if (this._panel) {
			this._panel.classList.remove('folded');
		} else {
			this._showPanel();
		}
		this._ensureSidebarVisible();
	},

	// Make sure the sidebar dock is shown so the user can see the panel
	// we just inserted.  If no core deck is currently loaded (e.g. the
	// user previously closed the sidebar - persisted as <docType>.
	// ShowSidebar=false in localStorage / browser settings - so the
	// startup auto-show in UIManager.initializeSidebar gets suppressed)
	// ask core for the default deck via .uno:SidebarShow so the user
	// still sees the regular sidebar context (Style / Character /
	// Paragraph / ...) below the extension panel.  Pair it with
	// setupTargetDeck so the toolbar's Sidebar button picks up the
	// active-deck state once core's response arrives.
	_ensureSidebarVisible: function () {
		const wrapper = document.getElementById('sidebar-dock-wrapper');
		if (wrapper) wrapper.classList.add('visible');
		if (this.map.sidebar && !this.map.sidebar.isVisible()) {
			app.socket.sendMessage('uno .uno:SidebarShow');
			this.map.sidebar.setupTargetDeck('.uno:SidebarDeck.PropertyDeck');
		}
	},

	_showPanel: function () {
		const manifest: ExtensionManifest = this.options.manifest;

		// The extension is rendered as a foldable panel prepended to
		// #sidebar-panel - sitting above whichever core deck is currently
		// rendered in #sidebar-container, both coexisting.  Folding only
		// hides the iframe via CSS so the iframe stays in the DOM and its
		// listeners keep working; the X button (handled by _removePanel)
		// runs the Extension_Teardown handshake to detach listeners and
		// then drops the panel.
		const sidebarPanel = document.getElementById('sidebar-panel');
		if (!sidebarPanel) return;

		const panel = document.createElement('div');
		panel.classList.add('extension-panel');
		panel.dataset.extensionId = this.options.id;

		const header = document.createElement('div');
		header.classList.add('extension-panel-header');
		header.addEventListener('click', (e) => {
			// Don't toggle fold when the click was on the close button.
			if ((e.target as HTMLElement).closest('.extension-panel-close')) return;
			panel.classList.toggle('folded');
		});

		const disclosure = document.createElement('span');
		disclosure.classList.add('extension-panel-disclosure');
		disclosure.setAttribute('aria-hidden', 'true');
		header.appendChild(disclosure);

		const title = document.createElement('span');
		title.classList.add('extension-panel-title');
		title.textContent = manifest.name;
		header.appendChild(title);

		const closeBtn = document.createElement('button');
		closeBtn.classList.add('extension-panel-close');
		closeBtn.textContent = 'X';
		closeBtn.title = _('Close extension');
		closeBtn.onclick = (e) => {
			e.stopPropagation();
			this._removePanel();
		};
		header.appendChild(closeBtn);
		panel.appendChild(header);

		const body = document.createElement('div');
		body.classList.add('extension-panel-body');
		panel.appendChild(body);

		const iframe = document.createElement('iframe');
		iframe.src = this.options.baseUrl + manifest.entry;
		iframe.setAttribute(
			'sandbox',
			'allow-scripts allow-same-origin allow-forms allow-popups',
		);
		body.appendChild(iframe);
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

		// Prepend so the extension sits above the core-rendered deck in
		// #sidebar-container.  We're a sibling of #sidebar-container, not
		// inside it, so a core sidebar rebuild won't clobber us.
		sidebarPanel.insertBefore(panel, sidebarPanel.firstChild);
		this._panel = panel;
	},

	_removePanel: function () {
		if (!this._panel) return;
		// If a teardown is already in flight, leave it to finish:
		if (this._teardownTimer !== null) return;
		// If the iframe isn't ready, skip the handshake (nothing to detach):
		if (this._iframe && this._iframe.contentWindow) {
			this._iframe.contentWindow.postMessage(
				JSON.stringify({ msgId: 'Extension_Teardown' }),
				'*',
			);
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
		if (this._panel) {
			this._panel.remove();
			this._panel = null;
			this._iframe = null;
		}
	},

	_onPostMessage: function (e: MessageEvent) {
		// Silently ignore postMessages from senders other than our iframe
		// (browser extensions, dev tools, third-party libs).  Anything
		// from the iframe, however, is expected to be our protocol; warn
		// about deviations.
		if (!this._iframe || e.source !== this._iframe.contentWindow) return;
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
		switch (msg.msgId) {
			case 'Extension_Call':
				app.socket.sendMessage(
					'executescript ' +
						msg.callId +
						' (' +
						msg.fn +
						').apply(null, ' +
						JSON.stringify(msg.args || []) +
						');',
				);
				break;
			case 'Extension_ProxyReturn':
				app.socket.sendMessage(
					'proxyreturn ' +
						msg.callId +
						' ' +
						JSON.stringify(msg.value === undefined ? null : msg.value),
				);
				break;
			case 'Extension_Close':
				this._removePanel();
				break;
			case 'Extension_TeardownDone':
				this._finishRemovePanel();
				break;
			case 'Extension_Resize':
				if (this._iframe) {
					this._iframe.style.height = msg.height + 'px';
				}
				break;
			default:
				console.warn('unexpected msgId: ' + (msg as any).msgId);
				break;
		}
	},

	_onScriptResult: function (e: ExtensionScriptResult) {
		if (this._iframe && this._iframe.contentWindow) {
			this._iframe.contentWindow.postMessage(
				JSON.stringify({
					msgId: 'Extension_CallResult',
					callId: e.id,
					ok: e.ok,
					err: e.err,
				}),
				'*',
			);
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
	if (app.userPresetConfigId) await addPresetSource(app.userPresetConfigId);

	// Flatten the sources into one list of (id, baseRel) entries; Map.set's last-write-wins
	// behavior causes per-user extensions to override admin extensions to overrride built-in
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
				return { id, baseRel, manifest };
			} catch (err) {
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
