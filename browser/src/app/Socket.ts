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
/* eslint-disable @typescript-eslint/no-empty-function */

class Socket {
	private ProtocolVersionNumber: string = '0.1';
	private ReconnectCount: number = 0;
	private WasShownLimitDialog: boolean = false;
	public WSDServer: WSDServerInfo = {
		Id: '',
		Version: '',
		Hash: '',
		Protocol: '',
		Options: '',
		TimeZone: '',
	};
	private IndirectSocketReconnectCount: number = 0;
	private _initialConnectRetries: number = 0;
	private _connectCount: number = 0;

	/// Whether Trace Event recording is enabled or not. ("Enabled" here means whether it can be
	/// turned on (and off again), not whether it is on.)
	private enableTraceEventLogging: boolean = false;

	// Will be set from lokitversion message
	private TunnelledDialogImageCacheSize: number = 0;

	public _map: MapInterface;
	private _msgQueue: MessageInterface[];
	private _delayedMessages: DelayedMessageInterface[];
	private _slurpQueue: SlurpMessageEvent[];
	private _handlingDelayedMessages: boolean;
	private _inLayerTransaction: boolean;
	private _slurpDuringTransaction: boolean;
	private _accessTokenExpireTimeout: TimeoutHdl | undefined;
	private _reconnecting: boolean;
	private _slurpTimer: TimeoutHdl | undefined;
	private _renderEventTimer: TimeoutHdl | undefined;
	private _renderEventTimerStart: DOMHighResTimeStamp | undefined;
	private _slurpTimerDelay: number | undefined;
	private _slurpTimerLaunchTime: number | undefined;
	private timer: ReturnType<typeof setInterval> | undefined;
	private workers: Worker[] = [];
	private workerMessageHandlers: Map<string, any> = new Map();
	private workerErrorHandlers: any[] = [];
	public threadLocalLoggingLevelToggle: boolean;

	private socket?: SockInterface;
	public traceEvents: TraceEvents;

	constructor(map: MapInterface) {
		window.app.console.debug('socket.initialize:');
		this._map = map;
		this._slurpQueue = [];
		this._msgQueue = [];
		this._delayedMessages = [];
		this._handlingDelayedMessages = false;
		this._inLayerTransaction = false;
		this._slurpDuringTransaction = false;
		this.threadLocalLoggingLevelToggle = false;
		this._accessTokenExpireTimeout = undefined;
		this._reconnecting = false;
		this._slurpTimer = undefined;
		this._renderEventTimer = undefined;
		this._renderEventTimerStart = undefined;
		this._slurpTimerDelay = undefined;
		this._slurpTimerLaunchTime = undefined;
		this.timer = undefined;
		this.socket = undefined;
		this.traceEvents = new TraceEvents(this);

		if (
			window.Worker &&
			(!(window as any).ThisIsAMobileApp || (window as any).mode.isCODesktop())
		) {
			window.app.console.info('Creating TaskWorkers');
			for (let i = 0; i < 4; ++i) {
				try {
					this.workers.push(
						new Worker(app.LOUtil.getURL('/src/app/TaskWorker.js')),
					);
					this.workers[i].addEventListener('message', (e: any) =>
						this.onWorkerMessage(e),
					);
					this.workers[i].addEventListener('error', (e: any) =>
						this.onWorkerError(e),
					);
				} catch (e) {
					this.onWorkerError(e);
					break;
				}
			}
		}
	}

	public getTaskWorkers(): Worker[] {
		return this.workers.slice();
	}

	public setTaskHandler(message: string, callback: any) {
		if (this.workerMessageHandlers.has(message))
			window.app.console.warn(
				'Duplicate task handler for ' + message,
				callback,
			);
		this.workerMessageHandlers.set(message, callback);
	}

	public addTaskErrorHandler(callback: any) {
		this.workerErrorHandlers.push(callback);
	}

	private onWorkerMessage(e: any) {
		const callback = this.workerMessageHandlers.get(e.data.message);
		if (!callback) {
			window.app.console.warn('Unhandled worker message', e);
			return;
		}

		callback(e);
	}

	private onWorkerError(e: any) {
		if (e) window.app.console.error('Worker-related error encountered', e);
		while (this.workers.length) {
			const worker = this.workers.shift();
			if (!worker) continue;
			try {
				worker.terminate();
			} catch (e) {
				window.app.console.error('Error terminating worker thread', e);
			}
		}
		this.workerMessageHandlers.clear();
		if (e) for (const callback of this.workerErrorHandlers) callback(e);
		this.workerErrorHandlers = [];
	}

	public disableTaskWorkers() {
		this.onWorkerError(null);
	}

	public sendMessage(msg: MessageInterface): void {
		if (!this.socket) {
			console.error('sendMessage() called with non-existent socket!');
			return;
		}

		if (this._map._debug.eventDelayWatchdog) this._map._debug.timeEventDelay();

		if (this._map._fatal) {
			// Avoid communicating when we're in fatal state
			return;
		}

		if (!app.idleHandler._active) {
			// Avoid communicating when we're inactive.
			if (typeof msg !== 'string') return;

			if (!msg.startsWith('useractive') && !msg.startsWith('userinactive')) {
				window.app.console.log(
					'Ignore outgoing message due to inactivity: "' + msg + '"',
				);
				return;
			}
		}

		if (this._map.uiManager && this._map.uiManager.isUIBlocked()) return;

		const socketState = this.socket.readyState;
		if (socketState === 2 || socketState === 3) {
			this._map.loadDocument();
		}

		if (socketState === 1) {
			this._doSend(msg);
		} else {
			// push message while trying to connect socket again.
			this._msgQueue.push(msg);
		}
	}

	public sendTraceEvent(
		name: any,
		ph: string,
		timeRange: undefined | string,
		args?: any,
		id?: number | string,
		tid?: number | string,
	): void {
		this.traceEvents.send(name, ph, timeRange, args, id, tid);
	}

	get traceEventRecordingToggle(): boolean {
		return this.traceEvents.getRecordingToggle();
	}

	public setTraceEventLogging(enabled: boolean) {
		this.traceEvents.setLogging(enabled);
	}

	public createAsyncTraceEvent(
		name: string,
		args?: any,
	): CompleteTraceEvent | null {
		return this.traceEvents.createAsync(name, args);
	}

	public _stringifyArgs(args: any): string {
		return args == null ? '' : ' args=' + JSON.stringify(args);
	}

	public createCompleteTraceEvent(
		name: string,
		args?: any,
	): CompleteTraceEvent | null {
		return this.traceEvents.createComplete(name, args);
	}

	public createCompleteTraceEventFromEvent(
		textMsg?: string,
	): CompleteTraceEvent | null {
		return this.traceEvents.createCompleteFromEvent(textMsg);
	}

	public parseServerCmd(msg: string): ServerCommand {
		return new ServerCommand(msg, this._map);
	}

	private getWebSocketBaseURI(map: MapInterface): string {
		if (window.enableExperimentalFeatures && map.options.wopiSrc) {
			// Use the new Cool WS URL for WOPI documents.
			return window.makeWopiCoolWsUrl(
				window.makeWsUrl('/cool'),
				$.param(map.options.docParams),
			);
		} else {
			return window.makeWsUrlWopiSrc(
				'/cool/',
				map.options.doc + '?' + $.param(map.options.docParams),
			);
		}
	}

	public connect(socket: SockInterface): void {
		const map = this._map;
		map.options.docParams['permission'] = app.getPermission();
		if (this.socket) {
			this.close();
		}
		if (socket && (socket.readyState === 1 || socket.readyState === 0)) {
			this.socket = socket;
		} else if (window.ThisIsAMobileApp) {
			// We have already opened the FakeWebSocket or MobileSocket over in global.js
			// With the FakeWebSocket do we then set this.socket at all?
			// With the MobileSocket we definitely do - this is load-bearing for opening password protected documents
		} else {
			try {
				this.socket = window.createWebSocket(this.getWebSocketBaseURI(map));
				window.socket = this.socket;
			} catch (e) {
				this._map.fire('error', {
					msg:
						_('Oops, there is a problem connecting to {productname}: ').replace(
							'{productname}',
							typeof brandProductName !== 'undefined'
								? brandProductName
								: 'Collabora Online Development Edition (unbranded)',
						) + e,
					cmd: 'socket',
					kind: 'failed',
					id: 3,
				});
				return;
			}
		}

		if (!this.socket) {
			console.error('connect: this.socket is still undefined!');
			return;
		}

		this.socket.onerror = this._onSocketError.bind(this);
		this.socket.onclose = this._onSocketClose.bind(this);
		this.socket.onopen = this._onSocketOpen.bind(this);
		this.socket.onmessage = this._slurpMessage.bind(this);
		this.socket.binaryType = 'arraybuffer';

		this._connectCount++;
		this._faultInjection();
		this.resetTokenExpiryTimer();

		// process messages for early socket connection
		this._emptyQueue();
	}

	public resetTokenExpiryTimer(): void {
		clearTimeout(this._accessTokenExpireTimeout); // Always clear the old timer.
		const ttl = parseInt(
			this._map.options.docParams.access_token_ttl as string,
		);
		if (this._map.options.docParams.access_token && ttl) {
			const tokenExpiryWarning = 900 * 1000; // Warn when 15 minutes remain
			this._accessTokenExpireTimeout = setTimeout(
				this._sessionExpiredWarning.bind(this),
				ttl - Date.now() - tokenExpiryWarning,
			);
		}
	}

	public close(code?: number, reason?: string): void {
		if (!this.socket) {
			console.error('Tried close() on non-existent socket!');
			return;
		}
		this.socket.onerror = function () {};
		this.socket.onclose = function () {};
		this.socket.onmessage = function () {};
		this.socket.close();

		// Reset wopi's app loaded so that reconnecting again informs outerframe about initialization
		this._map['wopi'].resetAppLoaded();
		this._map.fire('docloaded', { status: false });
		clearTimeout(this._accessTokenExpireTimeout);
	}

	private _doSend(msg: MessageInterface): void {
		if (!this.socket) {
			console.error('_doSend() called when socket is non-existent!');
			return;
		}
		// Only attempt to log text frames, not binary ones.
		if (typeof msg === 'string') this._logSocket('OUTGOING', msg);

		this.socket.send(msg);
	}

	private _onSocketOpen(evt: Event): void {
		window.app.console.debug('_onSocketOpen:');
		app.idleHandler._serverRecycling = false;
		app.idleHandler._documentIdle = false;

		// Always send the protocol version number.
		// TODO: Move the version number somewhere sensible.

		// Note there are two socket "onopen" handlers, this one which ends up as part of
		// bundle.js and the other in browser/js/global.js. The global.js one attempts to
		// set up the connection early while bundle.js is still loading. If bundle.js
		// starts before global.js has connected, then this _onSocketOpen will do the
		// connection instead, after taking over the socket in "connect"

		// Typically in a "make run" scenario it is the global.js case that sends the
		// 'coolclient' and 'load' messages while currently in the "WASM app" case it is
		// this code that gets invoked.

		// Also send information about our performance timer epoch
		const now0 = Date.now();
		const now1 = performance.now();
		const now2 = Date.now();
		this._doSend(
			'coolclient ' +
				this.ProtocolVersionNumber +
				' ' +
				(now0 + now2) / 2 +
				' ' +
				now1,
		);

		let msg = 'load url=' + encodeURIComponent(this._map.options.doc);
		if (this._map._docLayer) {
			this._reconnecting = true;
			// we are reconnecting after a lost connection
			msg += ' part=' + this._map.getCurrentPartNumber();
		}
		if (this._map.options.timestamp) {
			msg += ' timestamp=' + this._map.options.timestamp;
		}
		if (this._map._docPassword) {
			msg += ' password=' + this._map._docPassword;
		}
		if (String.locale) {
			msg += ' lang=' + String.locale;
		}
		if (window.deviceFormFactor) {
			msg += ' deviceFormFactor=' + window.deviceFormFactor;
		}

		msg += ' timezone=' + Intl.DateTimeFormat().resolvedOptions().timeZone;

		if (this._map.options.renderingOptions) {
			const options = {
				rendering: this._map.options.renderingOptions,
			};
			msg += ' options=' + JSON.stringify(options);
		}
		const spellOnline = window.prefs.get('spellOnline');
		if (spellOnline) {
			msg += ' spellOnline=' + spellOnline;
		}

		const darkTheme = window.prefs.getBoolean('darkTheme');
		msg += ' darkTheme=' + darkTheme;

		const darkBackground = window.prefs.getBoolean(
			'darkBackgroundForTheme.' + (darkTheme ? 'dark' : 'light'),
			darkTheme,
		);
		msg += ' darkBackground=' + darkBackground;
		this._map.uiManager.initDarkBackgroundUI(darkBackground);

		msg += ' accessibilityState=' + window.getAccessibilityState();

		msg += ' clientvisiblearea=' + window.makeClientVisibleArea();

		this._doSend(msg);
		for (let i = 0; i < this._msgQueue.length; i++) {
			this._doSend(this._msgQueue[i]);
		}
		this._msgQueue = [];

		app.idleHandler._activate();
	}

	private _onSocketClose(event: CloseEvent): void {
		window.app.console.debug(
			'_onSocketClose: code=' +
				event.code +
				' wasClean=' +
				event.wasClean +
				' reason=' +
				event.reason,
		);
		if (!this._map._docLoadedOnce && this.ReconnectCount === 0) {
			let errorType: string = '';
			let errorMsg: string;
			const reason = event.reason;
			if (reason && reason.startsWith('error:')) {
				var command = this.parseServerCmd(reason);
				if (
					command.errorCmd === 'internal' &&
					command.errorKind === 'unauthorized'
				) {
					errorType = 'websocketunauthorized';
					errorMsg = this._buildUnauthorizedMessage(command);
				} else if (
					command.errorCmd === 'storage' &&
					command.errorKind === 'loadfailed'
				) {
					errorType = 'websocketloadfailed';
					errorMsg = window.errorMessages.storage.loadfailed;
				} else {
					errorType = 'websocketgenericfailure';
					errorMsg = window.errorMessages.websocketgenericfailure;
				}
			} else if (this._initialConnectRetries++ < 3) {
				// Transient connection failure with no server error.
				// Retry the initial connection before giving up.
				window.app.console.debug(
					'_onSocketClose: transient failure during initial load, attempt ' +
						this._initialConnectRetries,
				);
				this._map['wopi'].resetAppLoaded();
				setTimeout(() => {
					this._scheduleReconnect();
				}, 1);
				return;
			} else {
				errorType = 'websocketproxyfailure';
				errorMsg = window.errorMessages.websocketproxyfailure;
			}
			this._map.fire('error', {
				msg: errorMsg,
				cmd: 'socket',
				kind: 'closed',
				id: 4,
			});
			const postMessageObj = {
				errorType: errorType,
				success: false,
				errorMsg: errorMsg,
				result: '',
			};
			this._map.fire('postMessage', {
				msgId: 'Action_Load_Resp',
				args: postMessageObj,
			});
			return;
		}
		if (this.ReconnectCount > 0) return;

		const isActive = app.idleHandler._active;
		this._map.hideBusy();
		app.idleHandler._active = false;
		app.idleHandler._serverRecycling = false;

		if (this._map._docLayer) {
			this._map._docLayer.removeAllViews();
			this._map._docLayer._resetClientVisArea();
			if (GraphicSelection.hasActiveSelection())
				GraphicSelection.rectangle = null;
			if (this._map._docLayer._docType === 'presentation')
				app.setCursorVisibility(false);

			this._map._docLayer._resetCanonicalIdStatus();
			this._map._docLayer._resetViewId();
			this._map._docLayer._resetDocumentInfo();
		}

		// We don't want them back on re-connection.
		this._delayedMessages = [];

		if (isActive && this._reconnecting) {
			// Don't show this before first transparently trying to reconnect.
			this._map.fire('error', {
				msg: _(
					'Well, this is embarrassing, we cannot connect to your document. Please try again.',
				),
				cmd: 'socket',
				kind: 'closed',
				id: 4,
			});
		}

		// Reset wopi's app loaded so that reconnecting again informs outerframe about initialization
		this._map['wopi'].resetAppLoaded();
		this._map.fire('docloaded', { status: false });

		// We need to make sure that the message slurping processes the
		// events first, because there could have been a message like
		// "close: idle" from the server.
		// Without the timeout, we'd immediately reconnect (because the
		// "close: idle" was not processed yet).
		setTimeout(() => {
			if (!this._reconnecting) {
				this._reconnecting = true;
				this._scheduleReconnect();
			}
		}, 1 /* ms */);

		if (this._map.isEditMode()) {
			this._map.setPermission('view');
		}

		if (
			!this._map['wopi'].DisableInactiveMessages &&
			app.sectionContainer &&
			!app.sectionContainer.testing
		)
			this._map.uiManager.showSnackbar(_('The server has been disconnected.'));
	}

	// Simulate error conditions for testing via the
	// simulateError URL parameter.
	private _faultInjection(): void {
		if (
			this._connectCount === 1 &&
			this.socket &&
			window.simulateError('socket:initialClose')
		) {
			const sock = this.socket;
			if (sock.readyState === 1) {
				// global.js may have already connected the socket
				// before bundle.js loaded - close after current
				// event processing
				setTimeout(function () {
					sock.close();
				}, 0);
			} else {
				const realOnOpen = sock.onopen;
				sock.onopen = function (evt: Event) {
					realOnOpen(evt);
					setTimeout(function () {
						sock.close();
					}, 0);
				};
			}
		}
	}

	private _scheduleReconnect(): void {
		app.idleHandler._active = false;
		if (!app.idleHandler._documentIdle)
			this._map.showBusy(_('Reconnecting...'), false);
		app.idleHandler._activate();
	}

	private _onSocketError(evt: Event): void {
		window.app.console.warn('_onSocketError:', evt);
		this._map.hideBusy();
		// Let onclose (_onSocketClose) report errors.
	}

	// The problem: if we process one websocket message at a time, the
	// browser -loves- to trigger a re-render as we hit the main-loop,
	// this takes ~200ms on a large screen, and worse we get
	// producer/consumer issues that can fill a multi-second long
	// buffer of web-socket messages in the client that we can't
	// process so - slurp and then emit at idle - its faster to delay!
	private _slurpMessage(evt: MessageEvent): void {
		const e = evt as SlurpMessageEvent;
		this._extractTextImg(e);

		// Some messages - we want to process & filter early.
		const docLayer = this._map ? this._map._docLayer : undefined;
		if (docLayer && docLayer.filterSlurpedMessage(e)) return;

		const predictedTiles = TileManager.predictTilesToSlurp();
		// scale delay, to a max of 50ms, according to the number of
		// tiles predicted to arrive.
		const delayMS = Math.max(Math.min(predictedTiles, 50), 1);

		if (!this._slurpQueue) this._slurpQueue = [];
		this._slurpQueue.push(e);
		this._queueSlurpEventEmission(delayMS);
	}

	private _emptyQueue(): void {
		if (window.queueMsg && window.queueMsg.length > 0) {
			for (let it = 0; it < window.queueMsg.length; it++) {
				const msg: MessageInterface = window.queueMsg[it];
				const evt: any = { data: msg, textMsg: msg };
				this._slurpMessage(evt);
			}
			window.queueMsg = [];
		}
	}

	private _sessionExpiredWarning(): void {
		clearTimeout(this._accessTokenExpireTimeout);
		let expirymsg = window.errorMessages.sessionexpiry;
		if (
			parseInt(this._map.options.docParams.access_token_ttl as string) -
				Date.now() <=
			0
		) {
			expirymsg = window.errorMessages.sessionexpired;
		}
		const dateTime = new Date(
			parseInt(this._map.options.docParams.access_token_ttl as string),
		);
		const dateOptions: Intl.DateTimeFormatOptions = {
			year: 'numeric',
			month: 'short',
			day: 'numeric',
			hour: '2-digit',
			minute: '2-digit',
		};
		const timerepr = dateTime.toLocaleDateString(String.locale, dateOptions);
		this._map.fire('warn', { msg: expirymsg.replace('{time}', timerepr) });

		// If user still doesn't refresh the session, warn again periodically
		this._accessTokenExpireTimeout = setTimeout(
			this._sessionExpiredWarning.bind(this),
			120 * 1000,
		);
	}

	public setUnloading(): void {
		if (this.socket && this.socket.setUnloading) this.socket.setUnloading();
	}

	public connected(): boolean {
		return this.socket !== undefined && this.socket.readyState === 1;
	}

	private _logSocket(type: Direction, msg: string): void {
		const logMessage =
			this._map._debug.debugNeverStarted ||
			this._map._debug.logIncomingMessages;
		if (!logMessage) return;

		if (window.ThisIsTheQtApp) window.postMobileDebug(type + ' ' + msg);

		const debugOn = this._map._debug.debugOn;

		if (this._map._debug.overlayOn) {
			this._map._debug.setOverlayMessage('postMessage', type + ': ' + msg);
		}

		if (!debugOn && !window.L.Browser.cypressTest && msg.length > 256)
			// for reasonable performance.
			msg =
				msg.substring(0, 256) + '<truncated ' + (msg.length - 256) + 'chars>';

		let status = '';
		if (!window.fullyLoadedAndReady) status += '[!fullyLoadedAndReady]';
		if (!window.bundlejsLoaded) status += '[!bundlejsLoaded]';

		app.Log.log(msg, type, status);

		if (!window.protocolDebug && !debugOn) return;

		const color = type === 'OUTGOING' ? 'color:red' : 'color:#2e67cf';
		window.app.console.log(
			+new Date() +
				' %c' +
				type +
				status +
				'%c: ' +
				msg.concat(' ').replace(' ', '%c '),
			'background:#ddf;color:black',
			color,
			'color:',
		);
	}

	private _getParameterByName(url: string, name: string): string {
		// Escape all regex characters.
		const escape = (str: string): string => {
			return str.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
		};
		const regex = new RegExp('[\\?&]' + escape(name) + '=([^&#]*)');
		const results = regex.exec(url);
		return results === null ? '' : results[1].replace(/\+/g, ' ');
	}

	private _utf8ToString(data: Uint8Array): string {
		let strBytes = '';
		for (let it = 0; it < data.length; it++) {
			strBytes += String.fromCharCode(data[it]);
		}
		return strBytes;
	}

	// Returns true if, and only if, we are ready to start loading
	// the tiles and rendering the document.
	private _isReady(): boolean {
		if (window.bundlejsLoaded == false || window.fullyLoadedAndReady == false) {
			return false;
		}

		if (
			typeof this._map == 'undefined' ||
			isNaN(this._map.options.tileWidthTwips) ||
			isNaN(this._map.options.tileHeightTwips)
		) {
			return false;
		}

		const center = this._map.getCenter();
		if (isNaN(center.lat) || isNaN(center.lng) || isNaN(this._map.getZoom())) {
			return false;
		}

		return true;
	}

	private _queueSlurpEventEmission(delayMS: number): void {
		let now = Date.now();
		if (this._slurpTimer && this._slurpTimerDelay != delayMS) {
			// The timer already exists, but now want to change timeout _slurpTimerDelay to delayMS.
			// Cancel it and reschedule by replacement with another timer using the desired delayMS
			// adjusted as if used at the original launch time.
			clearTimeout(this._slurpTimer);
			this._slurpTimer = undefined;
			this._slurpTimerDelay = delayMS;

			now = Date.now();
			if (this._slurpTimerLaunchTime === undefined) {
				console.assert(
					false,
					'_slurpTimerLaunchTime is undefined when _slurpTimer exists!',
				);
				return;
			}
			const sinceLaunchMS = now - this._slurpTimerLaunchTime;
			delayMS -= sinceLaunchMS;
			if (delayMS <= 0) delayMS = 1;
		}

		if (!this._slurpTimer) {
			if (!this._slurpTimerLaunchTime) {
				// The initial launch of the timer, rescheduling replacements retain
				// the launch time
				this._slurpTimerLaunchTime = now;
				this._slurpTimerDelay = delayMS;
			}
			this._slurpTimer = setTimeout(
				function (this: Socket) {
					this._slurpTimer = undefined;
					this._slurpTimerLaunchTime = undefined;
					this._slurpTimerDelay = undefined;
					if (this._inLayerTransaction) {
						this._slurpDuringTransaction = true;
						return;
					}
					this._emitSlurpedEvents();
				}.bind(this),
				delayMS,
			);
		}
	}

	private _emitSlurpedEvents(): void {
		if (this._map._debug.eventDelayWatchdog) this._map._debug.timeEventDelay();

		const queueLength = this._slurpQueue.length;
		const completeEventWholeFunction = this.createCompleteTraceEvent(
			'emitSlurped-' + String(queueLength),
			{ '_slurpQueue.length': String(queueLength) },
		);
		if (this._map && this._map._docLayer) {
			TileManager.beginTransaction();
			this._inLayerTransaction = true;

			// Queue an instant timeout early to try to measure the
			// re-rendering delay before we get back to the main-loop.
			if (this.traceEventRecordingToggle) {
				if (!this._renderEventTimer)
					this._renderEventTimer = setTimeout(function (this: Socket) {
						if (this._renderEventTimerStart === undefined) {
							console.assert(
								false,
								'_renderEventTimerStart is undefined when renderEvent is run!',
							);
							return;
						}
						const now = performance.now();
						const delta = now - this._renderEventTimerStart;
						if (delta >= 2 /* ms */) {
							// significant
							this.sendTraceEvent(
								name,
								'X',
								'ts=' +
									Math.round(this._renderEventTimerStart * 1000) +
									' dur=' +
									Math.round((now - this._renderEventTimerStart) * 1000),
							);
							this._renderEventTimerStart = undefined;
						}
						this._renderEventTimer = undefined;
					}, 0);
			}
		}
		// window.app.console.log('Slurp events ' + that._slurpQueue.length);
		let complete = true;
		try {
			for (let i = 0; i < queueLength; ++i) {
				const evt = this._slurpQueue[i];

				if (evt.isComplete()) {
					let textMsg;
					if (typeof evt.data === 'string') {
						textMsg = evt.data.replace(/\s+/g, '.');
					} else if (typeof evt.data === 'object') {
						textMsg = evt.textMsg.replace(/\s+/g, '.');
					}

					const completeEventOneMessage =
						this.createCompleteTraceEventFromEvent(textMsg);
					try {
						// it is - are you ?
						this._onMessage(evt);
					} catch (e: any) {
						// unpleasant - but stops this one problem event
						// stopping an unknown number of others.
						const msg =
							'Exception ' + e + ' emitting event ' + evt.data + '\n' + e.stack;
						window.app.console.error(msg);

						// When debugging let QA know something is up.
						if (window.enableDebug || window.L.Browser.cypressTest)
							this._map.uiManager.showInfoModal(
								'cool_alert',
								'',
								msg,
								'',
								_('Close'),
								function () {
									/* Do nothing. */
								},
								false,
							);

						// If we're cypress testing, fail the run. Cypress will fail anyway, but this way we may get
						// a nice error in the logs rather than guessing that the run failed from our popup blocking input...
						if (
							window.L.Browser.cypressTest &&
							window.parent !== window &&
							e !== null
						) {
							console.log('Sending event error to Cypress...', e);
							window.parent.postMessage(e);
						}
					} finally {
						if (completeEventOneMessage) completeEventOneMessage.finish();
					}
				} else {
					// Stop emitting, re-start when we async images load.
					this._slurpQueue = this._slurpQueue.slice(i, queueLength);
					complete = false;
					break;
				}
			}
		} finally {
			if (completeEventWholeFunction) completeEventWholeFunction.finish();
		}

		if (complete)
			// Finished all elements in the queue.
			this._slurpQueue = [];

		if (this._map) {
			const completeCallback = () => {
				// Let other layers / overlays catch up.
				this._map.fire('messagesdone');

				this._renderEventTimerStart = performance.now();

				this._inLayerTransaction = false;
				if (this._slurpDuringTransaction) {
					this._slurpDuringTransaction = false;
					this._queueSlurpEventEmission(1);
				}
			};

			if (this._inLayerTransaction && this._map._docLayer) {
				// Resume with redraw if dirty due to previous _onMessage() calls.
				TileManager.endTransaction(completeCallback);
			} else {
				completeCallback();
			}
		}
	}

	private _exportAsCallback(command: ServerCommand): void {
		this._map.hideBusy();
		Util.ensureValue(command.filename);
		this._map.uiManager.showInfoModal(
			'exported-success',
			_('Exported to storage'),
			_('Successfully exported: ') + decodeURIComponent(command.filename),
			'',
			_('OK'),
		);
	}

	private _onStatusMsg(textMsg: string, command: ServerCommand): void {
		if (!this._isReady()) {
			// Retry in a bit.
			setTimeout(() => {
				this._onStatusMsg(textMsg, command);
			}, 10);
			return;
		}

		if (!this._map._docLayer) {
			Util.ensureValue(command.type);
			// initialize and append text input before doc layer
			this._map.initTextInput(command.type);

			// Reinitialize the menubar and top toolbar if browser settings are enabled.
			// During the initial `initializeBasicUI` call, we don't know if compact mode is enabled.
			// Before `doclayerinit`, we recheck the compact mode setting and if conditions are met,
			// add the top toolbar and menubar controls to the map.
			if (window.prefs.useBrowserSetting) {
				if (
					!window.mode.isSmallScreenDevice() &&
					this._map.uiManager.getCurrentMode() === 'notebookbar'
				)
					this._map.uiManager.removeClassicUI();
				else if (!this._map.menubar)
					this._map.uiManager.initializeMenubarAndTopToolbar();
			}

			// first status message, we need to create the document layer
			let tileWidthTwips = this._map.options.tileWidthTwips;
			let tileHeightTwips = this._map.options.tileHeightTwips;
			if (this._map.options.zoom !== this._map.options.defaultZoom) {
				const scale = this._map.options.crs.scale(
					this._map.options.defaultZoom - this._map.options.zoom,
				);
				tileWidthTwips = Math.round(tileWidthTwips * scale);
				tileHeightTwips = Math.round(tileHeightTwips * scale);
			}

			let docLayer: DocLayerInterface | null = null;
			const options = {
				tileWidthTwips: tileWidthTwips / app.dpiScale,
				tileHeightTwips: tileHeightTwips / app.dpiScale,
				docType: command.type,
				viewId: command.viewid,
			};
			if (command.type === 'text')
				docLayer = new window.L.WriterTileLayer(options);
			else if (command.type === 'spreadsheet')
				docLayer = new window.L.CalcTileLayer(options);
			else if (command.type === 'presentation' || command.type === 'drawing')
				docLayer = new window.L.ImpressTileLayer(options);

			Util.ensureValue(docLayer);
			this._map._docLayer = docLayer;
			this._map.addLayer(docLayer);
			this._map.fire('doclayerinit');
		} else if (this._reconnecting) {
			// we are reconnecting ...
			this._map._docLayer._resetClientVisArea();
			TileManager.refreshTilesInBackground();
			this._map.fire('statusindicator', { statusType: 'reconnected' });

			const darkTheme = window.prefs.getBoolean('darkTheme');
			this._map.uiManager.activateDarkModeInCore(darkTheme);
			this._map.uiManager.applyInvert();
			this._map.uiManager.setCanvasColorAfterModeChange();

			if (!window.mode.isSmallScreenDevice())
				this._map.uiManager.initializeNotebookbarInCore();

			// close all the popups otherwise document textArea will not get focus
			this._map.uiManager.closeAll();
			this._map.setPermission(app.file.permission);
			window.migrating = false;
			this._map.uiManager.initializeSidebar();
			this._map.uiManager.refreshTheme();
		}

		this._map.fire('docloaded', { status: true });
		if (this._map._docLayer) {
			this._map._docLayer._onMessage(textMsg);

			// call update view list viewId if it is not defined yet
			if (!this._map._docLayer._getViewId()) this._map.fire('updateviewslist');

			this._reconnecting = false;

			// Applying delayed messages
			// note: delayed messages cannot be done before:
			// a) docLayer.map is set by map.addLayer(docLayer)
			// b) docLayer._onStatusMsg (via _docLayer._onMessage)
			// has set the viewid
			this._handleDelayedMessages(this._map._docLayer);
		}
	}

	private _onJSDialog(
		textMsg: string,
		callback?: JSDialogCallback | DefCallBack,
	): void {
		const msgData = JSON.parse(textMsg.substring('jsdialog:'.length + 1));

		if (msgData.children && !app.util.isArray(msgData.children)) {
			window.app.console.warn(
				"_onJSDialogMsg: The children's data should be created of array type",
			);
			return;
		}

		JSDialog.MessageRouter.processMessage(msgData, callback);
	}

	private _onHyperlinkClickedMsg(textMsg: string): void {
		let link: string | null = null;
		let coords: string | null = null;
		const hyperlinkMsgStart = 'hyperlinkclicked: ';
		const coordinatesMsgStart = ' coordinates: ';

		if (textMsg.indexOf(coordinatesMsgStart) !== -1) {
			const coordpos = textMsg.indexOf(coordinatesMsgStart);
			link = textMsg.substring(hyperlinkMsgStart.length, coordpos);
			coords = textMsg.substring(coordpos + coordinatesMsgStart.length);
		} else link = textMsg.substring(hyperlinkMsgStart.length);

		this._map.fire('hyperlinkclicked', { url: link, coordinates: coords });
	}

	private _delayMessage(textMsg: string): void {
		const message = { msg: textMsg };
		this._delayedMessages.push(message);
	}

	private _handleDelayedMessages(docLayer: DocLayerInterface): void {
		this._handlingDelayedMessages = true;

		while (this._delayedMessages.length) {
			const message = this._delayedMessages.shift();
			Util.ensureValue(message);
			try {
				docLayer._onMessage(message.msg);
			} catch (e) {
				// unpleasant - but stops this one problem
				// event stopping an unknown number of others.
				window.app.console.error(
					'Exception ' + e + ' emitting event ' + message,
					(e as Error).stack,
				);
			}
		}

		this._handlingDelayedMessages = false;
	}

	public _onMessage(e: SlurpMessageEvent | MinimalMessageEvent): void {
		let textMsg = e.textMsg;
		const imgBytes: Uint8Array | undefined = (e as SlurpMessageEvent).imgBytes;

		if (window.L.Browser.cypressTest) {
			window.L.initial._stubMessage(textMsg);
		}

		this._logSocket('INCOMING', textMsg);

		const command = this.parseServerCmd(textMsg);

		if (textMsg.startsWith('coolserver ')) {
			if (this._onCoolServerMsg(textMsg)) {
				return;
			}
		} else if (textMsg.startsWith('lokitversion ')) {
			this._onLokitVersionMsg(textMsg);
		} else if (textMsg.startsWith('enabletraceeventlogging ')) {
			this.enableTraceEventLogging = true;
		} else if (textMsg.startsWith('osinfo ')) {
			this._onOsInfoMsg(textMsg);
		} else if (textMsg.startsWith('clipboardkey: ')) {
			this._onClipboardKeyMsg(textMsg);
		} else if (textMsg.startsWith('perm:')) {
			this._onPermMsg(textMsg);
			return;
		} else if (textMsg.startsWith('filemode:')) {
			this._onFileModeMsg(textMsg);
		} else if (textMsg.startsWith('lockfailed:')) {
			this._map.onLockFailed(textMsg.substring('lockfailed:'.length).trim());
			return;
		} else if (textMsg.startsWith('wopi: ')) {
			this._onWopiMsg(textMsg);
			return;
		} else if (textMsg.startsWith('loadstorage: ')) {
			this._onLoadStorageMsg(textMsg);
		} else if (textMsg.startsWith('lastmodtime: ')) {
			this._onLastModTimeMsg(textMsg);
			return;
		} else if (textMsg.startsWith('commandresult: ')) {
			this._onCommandResultMsg(textMsg);
			return;
		} else if (textMsg.startsWith('migrate:') && window.indirectSocket) {
			this._onMigrateMsg(textMsg);
			return;
		} else if (textMsg.startsWith('close: ')) {
			this._onCloseMsg(textMsg);
			return;
		} else if (textMsg.startsWith('error:')) {
			if (this._onErrorMsg(textMsg, command)) return;
		} else if (textMsg.startsWith('fontsmissing:')) {
			this._onFontsMissing(textMsg, command);
		} else if (textMsg.startsWith('info:') && command.errorCmd === 'socket') {
			this._onInfoMsg(textMsg, command);
		} else if (textMsg.startsWith('pong ') && this._map._debug.pingOn) {
			this._map._debug.reportPong(
				command.rendercount ? command.rendercount : 0,
			);
		} else if (
			textMsg.startsWith('saveas:') ||
			textMsg.startsWith('renamefile:')
		) {
			this._renameOrSaveAsCallback(textMsg, command);
		} else if (textMsg.startsWith('exportas:')) {
			this._exportAsCallback(command);
		} else if (textMsg.startsWith('warn:')) {
			this._onWarnMsg(textMsg, command);
		} else if (window.ThisIsAMobileApp && textMsg.startsWith('mobile:')) {
			// allow passing some events easily from the mobile app
			const mobileEvent = textMsg.substring('mobile: '.length);
			this._map.fire(mobileEvent);
		} else if (textMsg.startsWith('blockui:')) {
			this._onBlockUIMsg(textMsg);
			return;
		} else if (textMsg.startsWith('unblockui:')) {
			this._map.fire('unblockUI');
			return;
		} else if (textMsg.startsWith('featurelock: ')) {
			// Handle feature locking related messages
			const lockInfo = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
			this._map._setLockProps(lockInfo);
			return;
		} else if (textMsg.startsWith('restrictedCommands: ')) {
			// Handle restriction related messages
			const restrictionInfo = JSON.parse(
				textMsg.substring(textMsg.indexOf('{')),
			);
			this._map._setRestrictions(restrictionInfo);
			return;
		} else if (textMsg.startsWith('blockedcommand: ')) {
			this._onBlockedCommandMsg(textMsg);
			return;
		} else if (
			textMsg.startsWith('updateroutetoken') &&
			window.indirectSocket
		) {
			window.routeToken = textMsg.split(' ')[1];
			window.app.console.log('updated routeToken: ' + window.routeToken);
		} else if (textMsg.startsWith('reload')) {
			// Switching modes.
			// boolean argument is deprecated. false is default.
			window.location.reload();
		} else if (textMsg.startsWith('slidelayer:')) {
			this._onSlideLayerMsg(textMsg, e);
			return;
		} else if (textMsg.startsWith('zstdslidelayer:')) {
			this._onZstdSlideLayerMsg(textMsg, e);
			return;
		} else if (textMsg.startsWith('sliderenderingcomplete:')) {
			this._onSlideRenderingCompleteMsg(textMsg, e);
			return;
		} else if (
			!textMsg.startsWith('tile:') &&
			!textMsg.startsWith('delta:') &&
			!textMsg.startsWith('slidelayer:') &&
			!textMsg.startsWith('zstdslidelayer:') &&
			!textMsg.startsWith('windowpaint:')
		) {
			if (imgBytes !== undefined) {
				try {
					// if it's not a tile, parse the whole message
					textMsg = String.fromCharCode.apply(
						null,
						imgBytes as unknown as number[],
					);
				} catch (error) {
					// big data string
					textMsg = this._utf8ToString(imgBytes);
				}
			}

			// Decode UTF-8 in case it is binary frame. Disable this block
			// in the iOS app as the image data is not URL encoded.
			if (typeof (e as SlurpMessageEvent).data === 'object') {
				// FIXME: Not sure what this code is supposed to do. Doesn't
				// decodeURIComponent() exactly reverse what window.escape() (which
				// is a deprecated equivalent of encodeURIComponent()) does? In what
				// case is this code even hit? If somebody figures out what is going
				// on here, please replace this comment with an explanation.
				textMsg = decodeURIComponent(window.escape(textMsg));
			}
		}

		if (textMsg.startsWith('status:')) {
			this._onStatusMsg(
				textMsg,
				JSON.parse(textMsg.replace('status:', '').replace('statusupdate:', '')),
			);
			return;
		}

		// These can arrive very early during the startup, and never again.
		if (textMsg.startsWith('progress:')) {
			this._onProgressMsg(textMsg);
			return;
		} else if (textMsg.startsWith('jsdialog:')) {
			this._onJSDialog(textMsg, e.callback);
		} else if (textMsg.startsWith('hyperlinkclicked:')) {
			this._onHyperlinkClickedMsg(textMsg);
		} else if (textMsg.startsWith('browsersetting:')) {
			window.prefs._initializeBrowserSetting(textMsg);
		} else if (textMsg.startsWith('viewsetting:')) {
			const settingJSON = JSON.parse(
				textMsg.substring('viewsetting:'.length + 1),
			);
			app.serverConnectionService.onViewSetting(settingJSON);
		}

		if (textMsg.startsWith('downloadas:') || textMsg.startsWith('exportas:')) {
			const postMessageObj = {
				success: true,
				result: 'exportas',
				errorMsg: '',
			};

			this._map.fire('postMessage', {
				msgId: 'Action_Save_Resp',
				args: postMessageObj,
			});
			// intentional falltrough
		}

		if (!this._map._docLayer || this._handlingDelayedMessages) {
			this._delayMessage(textMsg);
		} else {
			this._map._docLayer._onMessage(textMsg, (e as SlurpMessageEvent).image);
		}
	}

	private _extractTextImg(e: SlurpMessageEvent): void {
		if (
			(window.ThisIsTheiOSApp ||
				window.ThisIsTheWindowsApp ||
				window.ThisIsTheMacOSApp ||
				window.ThisIsTheEmscriptenApp ||
				window.ThisIsTheQtApp) &&
			typeof e.data === 'string'
		) {
			// Another fix for issue #5843 limit splitting on the first newline
			// to only certain message types on iOS. Also, fix mangled UTF-8
			// text on iOS in jsdialogs when using languages like Greek and
			// Japanese by only setting the image bytes for only the same set
			// of message types.
			if (
				e.data.startsWith('tile:') ||
				e.data.startsWith('tilecombine:') ||
				e.data.startsWith('delta:') ||
				e.data.startsWith('rendersearchlist:') ||
				e.data.startsWith('slidelayer:') ||
				e.data.startsWith('zstdslidelayer:') ||
				e.data.startsWith('windowpaint:')
			) {
				let index: number;
				index = e.data.indexOf('\n');
				if (index < 0) index = e.data.length;
				e.imgBytes = new Uint8Array(e.data.length);
				for (let i = 0; i < e.data.length; i++) {
					e.imgBytes[i] = e.data.charCodeAt(i);
				}
				e.imgIndex = index + 1;
				e.textMsg = e.data.substring(0, index);
			} else {
				e.textMsg = e.data;
			}
		} else if (typeof e.data === 'string') {
			e.textMsg = e.data;
		} else if (typeof e.data === 'object') {
			this._extractCopyObject(e);
		}
		e.isComplete = function () {
			if (this.image) return !!this.imageIsComplete;
			return true;
		};

		const isTile = e.textMsg.startsWith('tile:');
		const isDelta = e.textMsg.startsWith('delta:');
		if (
			!isTile &&
			!isDelta &&
			!e.textMsg.startsWith('slidelayer:') &&
			!e.textMsg.startsWith('windowpaint:')
		)
			return;

		if (e.textMsg.indexOf(' nopng') !== -1) return;

		// pass deltas through quickly.
		if (
			e.imgBytes &&
			e.imgIndex !== undefined &&
			(isTile || isDelta) &&
			e.imgBytes[e.imgIndex] != 80 /* P(ng) */
		) {
			// window.app.console.log('Passed through delta object');
			const imgEl: CoolHTMLImageElement = new Image();
			imgEl.rawData = e.imgBytes.subarray(e.imgIndex);
			imgEl.isKeyframe = isTile;
			e.image = imgEl;
			e.imageIsComplete = true;
			return;
		}

		// window.app.console.log('PNG preview');

		// lazy-loaded PNG slide previews
		const img = this._extractImage(e);
		if (isTile) {
			const imgEl: CoolHTMLImageElement = new Image();
			imgEl.src = img;
			e.image = imgEl;
			e.imageIsComplete = true;
			return;
		}

		// PNG dialog bits
		const imageElement: CoolHTMLImageElement = new Image();
		e.image = imageElement;
		imageElement.onload = function (this: Socket) {
			e.imageIsComplete = true;
			this._queueSlurpEventEmission(1);
			if (imageElement.completeTraceEvent)
				imageElement.completeTraceEvent.finish();
		}.bind(this);
		// imageElement.onerror expects a different type of handler according to tsc.
		// (event: Event | string, source?: string, lineno?: number, colno?: number, error?: Error): any;
		// So use addEventListener() for 'error' event.
		imageElement.addEventListener(
			'error',
			function (this: Socket, err: ErrorEvent) {
				window.app.console.log('Failed to load image ' + img + ' fun ' + err);
				e.imageIsComplete = true;
				this._queueSlurpEventEmission(1);
				if (imageElement.completeTraceEvent)
					imageElement.completeTraceEvent.abort();
			}.bind(this),
		);

		// This can return null.
		const traceEvt = this.createAsyncTraceEvent('loadTile');
		imageElement.completeTraceEvent = traceEvt ? traceEvt : undefined;
		imageElement.src = img;
	}

	// make profiling easier
	private _extractCopyObject(e: SlurpMessageEvent): void {
		let index: number;

		e.imgBytes = new Uint8Array(e.data as ArrayBufferLike);

		// search for the first newline which marks the end of the message
		index = e.imgBytes.indexOf(10);
		if (index < 0) index = e.imgBytes.length;

		e.textMsg = String.fromCharCode.apply(
			null,
			e.imgBytes.subarray(0, index) as unknown as number[],
		);

		e.imgIndex = index + 1;
	}

	// convert to string of bytes without blowing the stack if data is large.
	private _strFromUint8(prefix: string, data: Uint8Array): string {
		let i: number;
		const chunk = 4096;
		let strBytes = prefix;
		for (i = 0; i < data.length; i += chunk)
			strBytes += String.fromCharCode.apply(
				null,
				data.slice(i, i + chunk) as unknown as number[],
			);
		strBytes += String.fromCharCode.apply(
			null,
			data.slice(i) as unknown as number[],
		);
		return strBytes;
	}

	private _extractImage(e: SlurpMessageEvent): string {
		if (!e.imgBytes) {
			console.assert(
				false,
				'Called _extractImage with and event that does not have imgBytes member!',
			);
			return '';
		}

		if (e.imgIndex === undefined) {
			console.assert(false, '_extractImage: event does not have imgIndex!');
			return '';
		}

		const data = e.imgBytes.subarray(e.imgIndex);
		let prefix = '';
		// FIXME: so we prepend the PNG pre-byte here having removed it in TileCache::appendBlob
		if (data[0] != 0x89) prefix = String.fromCharCode(0x89);
		const img =
			'data:image/png;base64,' + window.btoa(this._strFromUint8(prefix, data));
		if (
			window.L.Browser.cypressTest &&
			window.prefs.getBoolean('image_validation_test')
		) {
			if (!window.imgDatas) window.imgDatas = [];
			window.imgDatas.push(img);
		}
		return img;
	}

	private _buildUnauthorizedMessage(command: ServerCommand): string {
		let unauthorizedMsg = window.errorMessages.unauthorized;
		if (command.errorCode) {
			// X509_verify_cert_error_string output
			const authError = window.atob(command.errorCode);
			const verifyError = window.errorMessages.verificationerror.replace(
				'{errormessage}',
				authError,
			);
			unauthorizedMsg += ' ' + verifyError;
		}
		return unauthorizedMsg;
	}

	public manualReconnect(timeout: number): void {
		if (this._map._docLayer) {
			this._map._docLayer.removeAllViews();
		}
		app.idleHandler._active = false;
		this.close();
		clearTimeout(this.timer);
		setTimeout(function () {
			try {
				app.idleHandler._activate();
			} catch (error) {
				window.app.console.warn('Cannot activate map');
			}
		}, timeout);
	}

	private _showDocumentConflictPopUp(): void {
		const buttonList: { id: string; text: string }[] = [];
		const callbackList: { id: string; func_: (() => void) | null }[] = [];

		buttonList.push({ id: 'cancel-conflict-popup', text: _('Cancel') });
		callbackList.push({ id: 'cancel-conflict-popup', func_: null });

		buttonList.push({ id: 'discard-button', text: _('Discard') });
		buttonList.push({ id: 'overwrite-button', text: _('Overwrite') });

		callbackList.push({
			id: 'discard-button',
			func_: function (this: Socket) {
				this.sendMessage('closedocument');
			}.bind(this),
		});

		callbackList.push({
			id: 'overwrite-button',
			func_: function (this: Socket) {
				this.sendMessage('savetostorage force=1');
			}.bind(this),
		});

		if (!this._map['wopi'].UserCanNotWriteRelative) {
			buttonList.push({ id: 'save-to-new-file', text: _('Save to new file') });
			callbackList.push({
				id: 'save-to-new-file',
				func_: function (this: Socket) {
					let filename = this._map['wopi'].BaseFileName;
					if (filename) {
						filename = app.LOUtil.generateNewFileName(filename, '_new');
						this._map.saveAs(filename);
					}
				}.bind(this),
			});
		}

		const title = _('Document has been changed');
		const message = _(
			'Document has been changed in storage. What would you like to do with your unsaved changes?',
		);

		this._map.uiManager.showModalWithCustomButtons(
			'document-conflict-popup',
			title,
			message,
			false,
			buttonList,
			callbackList,
		);
	}

	private _askForDocumentPassword(passwordType: string, msg: string): void {
		this._map.uiManager.showInputModal(
			'password-popup',
			'',
			msg,
			'',
			_('OK'),
			function (this: Socket, data: string): void {
				if (data) {
					this._map._docPassword = data;
					if (window.ThisIsAMobileApp) {
						window.postMobileMessage('loadwithpassword password=' + data);
					}
					this._map.loadDocument();
				} else if (passwordType === 'to-modify') {
					this._map._docPassword = '';
					this._map.loadDocument();
				} else {
					this._map.fire('postMessage', { msgId: 'UI_Cancel_Password' });
					this._map.hideBusy();
				}
			}.bind(this),
			true /* password input */,
		);
	}

	private _renameOrSaveAsCallback(
		textMsg: string,
		command: ServerCommand,
	): void {
		this._map.hideBusy();
		if (
			command !== undefined &&
			command.url !== undefined &&
			command.url !== ''
		) {
			const url = command.url;

			// setup for loading the new document, and trigger the load
			const docUrl = url.split('?')[0];
			this._map.options.doc = docUrl;
			// After save-as op, we may connect to another server, then
			// code will think that server has restarted. In this case,
			// we don't want to reload the page (detect the file name is
			// different).
			this._map.options.previousWopiSrc = this._map.options.wopiSrc;
			this._map.options.wopiSrc = docUrl;
			window.wopiSrc = this._map.options.wopiSrc;

			if (textMsg.startsWith('renamefile:')) {
				this._map.uiManager.documentNameInput.showLoadingAnimation();
				this._map.fire('postMessage', {
					msgId: 'File_Rename',
					args: {
						NewName: command.filename,
					},
				});
			} else if (textMsg.startsWith('saveas:')) {
				const accessToken = this._getParameterByName(url, 'access_token');
				let accessTokenTtl = this._getParameterByName(url, 'access_token_ttl');
				const noAuthHeader = this._getParameterByName(url, 'no_auth_header');

				if (accessToken !== undefined) {
					if (accessTokenTtl === '') {
						accessTokenTtl = '0';
					}
					this._map.options.docParams = {
						access_token: accessToken,
						access_token_ttl: accessTokenTtl,
					};
					if (noAuthHeader == '1' || noAuthHeader == 'true') {
						this._map.options.docParams.no_auth_header = noAuthHeader;
					}
				} else {
					this._map.options.docParams = {};
				}

				// if this is save-as, we need to load the document with
				// edit permission otherwise the user has to close the
				// doc then re-open it again in order to be able to
				// edit.
				app.setPermission('edit');
				this.close();
				if (window.ThisIsTheQtApp || window.ThisIsTheWindowsApp) {
					window.postMobileMessage('loaddocument url=' + this._map.options.doc);
					this.socket?.onopen(new Event('open'));
				}
				this._map.loadDocument();
				this._map.sendInitUNOCommands();
				Util.ensureValue(command.filename);
				this._map.fire('postMessage', {
					msgId: 'Action_Save_Resp',
					args: {
						success: true,
						fileName: decodeURIComponent(command.filename),
					},
				});
			}
		}
		// let name = command.name; - ignored, we get the new name via the wopi's BaseFileName
	}

	/* _onMessage() subtasks */

	// 'coolserver ' message.
	// returns boolean whether or not to return immediately from _onMessage().
	private _onCoolServerMsg(textMsg: string): boolean {
		// This must be the first message, unless we reconnect.
		let oldVersion = '';
		let sameFile = true;
		// Check if we are reconnecting.
		if (this.WSDServer && this.WSDServer.Id) {
			// Yes we are reconnecting.
			// If our connection was lost and is ready again, we will not need to refresh the page.
			oldVersion = this.WSDServer.Version;

			window.app.console.assert(
				this._map.options.wopiSrc === window.wopiSrc,
				'wopiSrc mismatch!: ' +
					this._map.options.wopiSrc +
					' != ' +
					window.wopiSrc,
			);
			// If another file is opened, we will not refresh the page.
			if (this._map.options.previousWopiSrc && this._map.options.wopiSrc) {
				if (this._map.options.previousWopiSrc !== this._map.options.wopiSrc)
					sameFile = false;
			}
		}

		this.WSDServer = JSON.parse(
			textMsg.substring(textMsg.indexOf('{')),
		) as WSDServerInfo;

		if (oldVersion && sameFile) {
			if (this.WSDServer.Version !== oldVersion) {
				let reloadMessage = _(
					'Server is now reachable. We have to refresh the page now.',
				);
				if (window.mode.isSmallScreenDevice())
					reloadMessage = _('Server is now reachable...');

				const reloadFunc = function () {
					window.location.reload();
				};
				if (!this._map['wopi'].DisableInactiveMessages)
					this._map.uiManager.showSnackbar(
						reloadMessage,
						_('RELOAD'),
						reloadFunc,
					);
				else
					this._map.fire('postMessage', {
						msgId: 'Reloading',
						args: { Reason: 'Reconnected' },
					});
				setTimeout(reloadFunc, 5000);
			}
		}
		if (window.indirectSocket) {
			if (
				window.expectedServerId &&
				window.expectedServerId != this.WSDServer.Id
			) {
				if (this.IndirectSocketReconnectCount++ >= 3) {
					let msg = window.errorMessages.clusterconfiguration.replace(
						'{productname}',
						typeof brandProductName !== 'undefined'
							? brandProductName
							: 'Collabora Online Development Edition (unbranded)',
					);
					msg = msg.replace('{0}', window.expectedServerId);
					msg = msg.replace('{1}', window.routeToken);
					msg = msg.replace('{2}', this.WSDServer.Id);
					this._map.uiManager.showInfoModal(
						'wrong-server-modal',
						_('Cluster configuration warning'),
						msg,
						'',
						_('OK'),
						null,
						false,
					);
					this.IndirectSocketReconnectCount = 0;
				} else {
					this._map.showBusy(_('Wrong server, reconnecting...'), false);
					this.manualReconnect(3000);
					// request to indirection server to sanity check the tokens
					this.sendMessage('routetokensanitycheck');
					return true;
				}
			}
		}

		Object.assign(window.app.serverInfo, {
			coolwsdVersion: this.WSDServer.Version || '',
			coolwsdHash: this.WSDServer.Hash || '',
			serverId: this.WSDServer.Id || '',
			wsdOptions: this.WSDServer.Options || '',
		});

		// TODO: For now we expect perfect match in protocol versions
		if (this.WSDServer.Protocol !== this.ProtocolVersionNumber) {
			this._map.fire('error', { msg: _('Unsupported server version.') });
		}

		return false;
	}

	// 'lokitversion ' message.
	private _onLokitVersionMsg(textMsg: string): void {
		const lokitVersionObj = JSON.parse(textMsg.substring(textMsg.indexOf('{')));

		this.TunnelledDialogImageCacheSize =
			lokitVersionObj.tunnelled_dialog_image_cache_size;
	}

	// 'osinfo ' message.
	private _onOsInfoMsg(textMsg: string) {
		const osInfo = textMsg.replace('osinfo ', '');
		window.app.serverInfo.osInfo = osInfo;
	}

	// 'clipboardkey:' message.
	private _onClipboardKeyMsg(textMsg: string): void {
		const key = textMsg.substring('clipboardkey: '.length);
		if (this._map._clip) this._map._clip.setKey(key);
	}

	// 'perm:' message.
	private _onPermMsg(textMsg: string): void {
		const perm = textMsg.substring('perm:'.length).trim();

		// Never make the permission more permissive than it originally was.
		if (!app.isReadOnly()) app.setPermission(perm);

		if (this._map._docLayer) this._map.setPermission(app.file.permission);

		app.file.disableSidebar = app.isReadOnly();
	}

	// 'filemode:' message.
	private _onFileModeMsg(textMsg: string): void {
		const json = JSON.parse(textMsg.substring('filemode:'.length).trim());

		// Never make the permission more permissive than it originally was.
		if (!app.isReadOnly() && json.readOnly) {
			app.setPermission('readonly');
		}

		if (this._map._docLayer) {
			this._map.setPermission(app.file.permission);
		}

		// Store the view mode extensions list from server configuration
		if (json.viewModeExtensions) {
			app.file.viewModeExtensions = json.viewModeExtensions;
		}

		app.setCommentEditingPermission(json.editComment); // May be allowed even in readonly mode.
		app.setRedlineManagementAllowed(json.manageRedlines); // May be allowed even in readonly mode.
	}

	// 'wopi:' message.
	private _onWopiMsg(textMsg: string): void {
		// Handle WOPI related messages
		const wopiInfo = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
		this._map.fire('wopiprops', wopiInfo);
	}

	// 'loadstorage: ' message.
	private _onLoadStorageMsg(textMsg: string): void {
		if (textMsg.substring(textMsg.indexOf(':') + 2) === 'failed') {
			window.app.console.debug('Loading document from a storage failed');
			this._map.fire('postMessage', {
				msgId: 'App_LoadingStatus',
				args: {
					Status: 'Failed',
				},
			});
		}
	}

	// 'lastmodtime: ' message.
	private _onLastModTimeMsg(textMsg: string): void {
		const time = textMsg.substring(textMsg.indexOf(' ') + 1);
		this._map.updateModificationIndicator(time);
	}

	// 'commandresult: ' message.
	private _onCommandResultMsg(textMsg: string): void {
		const commandresult = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
		if (
			commandresult['command'] === 'savetostorage' ||
			commandresult['command'] === 'save'
		) {
			const postMessageObj = {
				success: commandresult['success'],
				result: commandresult['result'],
				errorMsg: commandresult['errorMsg'],
			};

			this._map.fire('postMessage', {
				msgId: 'Action_Save_Resp',
				args: postMessageObj,
			});
		} else if (commandresult['command'] === 'load') {
			const postMessageObj = {
				success: commandresult['success'],
				result: commandresult['result'],
				errorMsg: commandresult['errorMsg'],
			};
			this._map.fire('postMessage', {
				msgId: 'Action_Load_Resp',
				args: postMessageObj,
			});
		}
	}

	// 'migrate: ' message.
	private _onMigrateMsg(textMsg: string): void {
		const migrate = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
		const afterSave = migrate.afterSave as boolean;
		app.idleHandler._serverRecycling = false;
		if (!afterSave) {
			window.migrating = true;
			this._map.uiManager.closeAll();
			if (this._map.isEditMode()) {
				this._map.setPermission('view');
				this._map.uiManager.showSnackbar(
					_('Document is getting migrated'),
					null,
					null,
					3000,
				);
			}
			if (migrate.saved) {
				window.routeToken = migrate.routeToken;
				window.expectedServerId = migrate.serverId;
				this.manualReconnect(2000);
			}
			return;
		}
		// even after save attempt, if document is unsaved reset the file permission
		if (migrate.saved) {
			window.routeToken = migrate.routeToken;
			window.expectedServerId = migrate.serverId;
			this.manualReconnect(2000);
		} else {
			this._map.setPermission(app.file.permission);
			window.migrating = false;
		}
	}

	// 'close: ' message.
	private _onCloseMsg(textMsg: string): void {
		textMsg = textMsg.substring('close: '.length);
		const postMsgData: any = {};
		let showMsgAndReload = false;
		// This is due to document owner terminating the session
		if (textMsg === 'ownertermination') {
			postMsgData['Reason'] = 'OwnerTermination';
		} else if (textMsg === 'idle' || textMsg === 'oom') {
			app.idleHandler._documentIdle = true;
			this._map._docLayer._documentInfo = undefined;
			postMsgData['Reason'] = 'DocumentIdle';
			if (textMsg === 'oom') postMsgData['Reason'] = 'OOM';
		} else if (textMsg === 'shuttingdown') {
			postMsgData['Reason'] = 'ShuttingDown';
			app.idleHandler._active = false;
			app.idleHandler._serverRecycling = true;
		} else if (textMsg === 'docdisconnected') {
			postMsgData['Reason'] = 'DocumentDisconnected';
		} else if (textMsg === 'recycling') {
			app.idleHandler._active = false;
			app.idleHandler._serverRecycling = true;

			// Prevent reconnecting the world at the same time.
			const min = 5000;
			const max = 10000;
			const timeoutMs = Math.floor(Math.random() * (max - min) + min);

			clearTimeout(this.timer);
			this.timer = setInterval(() => {
				if (this.connected()) {
					// We're connected: cancel timer and dialog.
					clearTimeout(this.timer);
					return;
				}

				try {
					this._map.loadDocument(this.socket);
				} catch (error) {
					window.app.console.warn('Cannot load document.');
				}
			}, timeoutMs);
		} else if (textMsg.startsWith('documentconflict')) {
			showMsgAndReload = true;
		} else if (textMsg.startsWith('versionrestore:')) {
			textMsg = textMsg.substring('versionrestore:'.length).trim();
			if (textMsg === 'prerestore_ack') {
				this._map.fire('postMessage', {
					msgId: 'App_VersionRestore',
					args: { Status: 'Pre_Restore_Ack' },
				});
				showMsgAndReload = true;
			}
		} else if (textMsg.startsWith('reloadafterrename')) {
			showMsgAndReload = true;
		}

		if (showMsgAndReload) {
			if (this._map._docLayer) {
				this._map._docLayer.removeAllViews();
			}
			// Detach all the handlers from current socket, otherwise _onSocketClose tries to reconnect again
			// However, we want to reconnect manually here.
			this.close();

			// Reload the document
			app.idleHandler._active = false;
			clearTimeout(this.timer);
			this.timer = setInterval(() => {
				try {
					// Activate and cancel timer and dialogs.
					app.idleHandler._activate();
					this._map.uiManager.documentNameInput.hideLoadingAnimation();
				} catch (error) {
					window.app.console.warn('Cannot activate map');
				}
			}, 3000);
		}

		// Close any open dialogs first.
		this._map.uiManager.closeAll();

		if (textMsg === 'idle' || textMsg === 'oom') {
			app.idleHandler._dim();
			TileManager.discardAllCache();
		}

		if (postMsgData['Reason']) {
			// Tell WOPI host about it which should handle this situation
			this._map.fire('postMessage', {
				msgId: 'Session_Closed',
				args: postMsgData,
			});
		}

		if (textMsg === 'ownertermination') {
			this._map.remove();
		}
	}

	// 'error: ' message.
	// returns true if the caller need to exit immediately.
	private _onErrorMsg(textMsg: string, command: ServerCommand): boolean {
		const errorMessages = window.errorMessages;
		let msg = '';
		let passwordType: string = '';
		if (
			(textMsg.startsWith('error:') &&
				(command.errorCmd === 'storage' || command.errorCmd === 'saveas')) ||
			command.errorCmd === 'downloadas' ||
			command.errorCmd === 'exportas'
		) {
			if (command.errorCmd !== 'storage') {
				this._map.fire('postMessage', {
					msgId: 'Action_Save_Resp',
					args: {
						success: false,
						result: command.errorKind,
					},
				});
			}

			this._map.hideBusy();
			let storageError: string | undefined = undefined;
			if (command.errorKind === 'savediskfull') {
				storageError = errorMessages.storage.savediskfull;
			} else if (command.errorKind === 'savetoolarge') {
				storageError = errorMessages.storage.savetoolarge;
			} else if (command.errorKind === 'savefailed') {
				storageError = errorMessages.storage.savefailed;
			} else if (command.errorKind === 'renamefailed') {
				storageError = errorMessages.storage.renamefailed;
			} else if (command.errorKind === 'saveunauthorized') {
				storageError = errorMessages.storage.saveunauthorized;
			} else if (command.errorKind === 'saveasfailed') {
				storageError = errorMessages.storage.saveasfailed;
			} else if (command.errorKind === 'loadfailed') {
				storageError = errorMessages.storage.loadfailed;
				// Since this is a document load failure, wsd will disconnect the socket anyway,
				// better we do it first so that another error message doesn't override this one
				// upon socket close.
				this.close();
			} else if (command.errorKind === 'documentconflict') {
				if (this._map.isReadOnlyMode())
					return true; // caller should exit immediately.
				else this._showDocumentConflictPopUp();

				return true; // caller should exit immediately.
			}

			// Skip empty errors (and allow for suppressing errors by making them blank).
			if (storageError && storageError != '') {
				// Parse the storage url as link
				const tmpLink = document.createElement('a');
				tmpLink.href = this._map.options.doc;
				// Insert the storage server address to be more friendly
				storageError = storageError.replace('%storageserver', tmpLink.host);

				// show message to the user in Control.AlertDialog
				this._map.fire('warn', { msg: storageError });

				// send to wopi handler so we can respond
				const postMessageObj = {
					success: false,
					cmd: command.errorCmd,
					result: command.errorKind,
					errorMsg: storageError,
				};

				this._map.fire('postMessage', {
					msgId: 'Action_Save_Resp',
					args: postMessageObj,
				});

				return true; // caller should exit immediately.
			}
		} else if (
			textMsg.startsWith('error:') &&
			command.errorCmd === 'internal'
		) {
			this._map.hideBusy();
			this._map._fatal = true;
			if (command.errorKind === 'diskfull') {
				this._map.fire('error', { msg: errorMessages.diskfull });
			} else if (command.errorKind === 'unauthorized') {
				const postMessageObj = {
					errorType: 'websocketunauthorized',
					success: false,
					errorMsg: this._buildUnauthorizedMessage(command),
					result: '',
				};
				this._map.fire('postMessage', {
					msgId: 'Action_Load_Resp',
					args: postMessageObj,
				});
			}

			if (this._map._docLayer) {
				this._map._docLayer.removeAllViews();
				this._map._docLayer._resetClientVisArea();
			}
			this.close();

			return true; // caller should exit immediately.
		} else if (textMsg.startsWith('error:') && command.errorCmd === 'load') {
			this._map.hideBusy();
			this.close();

			const errorKind = command.errorKind ? command.errorKind : '';
			let passwordNeeded = false;
			if (errorKind.startsWith('passwordrequired')) {
				passwordNeeded = true;
				msg = '';
				passwordType = errorKind.split(':')[1];
				if (passwordType === 'to-view') {
					msg += _('Document requires password to view.');
				} else if (passwordType === 'to-modify') {
					msg += _('Document requires password to modify.');
					msg += ' ';
					msg += _('Hit Cancel to open in view-only mode.');
				}
			} else if (errorKind.startsWith('wrongpassword')) {
				passwordNeeded = true;
				msg = _('Wrong password provided. Please try again.');
			} else if (errorKind.startsWith('faileddocloading')) {
				this._map._fatal = true;
				this._map.fire('error', { msg: errorMessages.faileddocloading });
			} else if (errorKind.startsWith('docloadtimeout')) {
				this._map._fatal = true;
				this._map.fire('error', { msg: errorMessages.docloadtimeout });
			} else if (errorKind.startsWith('docunloading')) {
				// The document is unloading. Have to wait a bit.
				app.idleHandler._active = false;

				clearTimeout(this.timer);
				if (this.ReconnectCount++ >= 10) {
					this._map.fire('error', { msg: errorMessages.docunloadinggiveup });
					// Give up.
					return true; // caller should exit immediately.
				}

				this.timer = setInterval(
					function () {
						try {
							// Activate and cancel timer and dialogs.
							app.idleHandler._activate();
						} catch (error) {
							window.app.console.warn('Cannot activate map');
						}
						// .5, 2, 4.5, 8, 12.5, 18, 24.5, 32, 40.5 seconds
					},
					500 * this.ReconnectCount * this.ReconnectCount,
				); // Quadratic back-off.

				if (this.ReconnectCount > 1) {
					this._map.showBusy(errorMessages.docunloadingretry, false);
				}
			}

			if (passwordNeeded) {
				this._askForDocumentPassword(passwordType, msg);
				return true; // caller should exit immediately.
			}
		} else if (
			textMsg.startsWith('error:') &&
			command.errorCmd === 'dialogevent' &&
			command.errorKind === 'cantchangepass'
		) {
			const msg = _('Only the document owner can change the password.');
			this._map.uiManager.showInfoModal('cool_alert', '', msg, '', _('OK'));
			return true; // caller should exit immediately.
		} else if (textMsg.startsWith('error:') && !this._map._docLayer) {
			textMsg = textMsg.substring(6);
			if (command.errorKind === 'hardlimitreached') {
				textMsg = errorMessages.limitreachedprod;
				if (command.params) {
					textMsg = textMsg.replace('{0}', command.params[0]);
					textMsg = textMsg.replace('{1}', command.params[1]);
				}
			} else if (command.errorKind === 'serviceunavailable') {
				textMsg = errorMessages.serviceunavailable;
			}
			this._map._fatal = true;
			app.idleHandler._active = false; // Practically disconnected.
			this._map.fire('error', { msg: textMsg });
		}

		return false;
	}

	// 'fontsmissing: ' message.
	private _onFontsMissing(textMsg: string, command: ServerCommand): void {
		const fontsMissingObj = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
		let msg = ' ';
		for (let i = 0; i < fontsMissingObj.fontsmissing.length; ++i) {
			if (i > 0) msg += ', ';
			msg += fontsMissingObj.fontsmissing[i];
		}

		if (
			this._map.welcome &&
			!this._map.welcome.isGuest() &&
			this._map.welcome.shouldWelcome() &&
			window.autoShowWelcome
		) {
			setTimeout(
				function (this: Socket) {
					this._map.uiManager.showInfoModal(
						'fontsmissing',
						_('Missing Fonts'),
						msg,
						null,
						_('Close'),
					);
				}.bind(this),
				60000,
			);
		} else {
			this._map.uiManager.showInfoModal(
				'fontsmissing',
				_('Missing Fonts'),
				msg,
				null,
				_('Close'),
			);
		}
	}

	// 'info: ' message.
	private _onInfoMsg(textMsg: string, command: ServerCommand): void {
		if (command.errorKind === 'limitreached' && !this.WasShownLimitDialog) {
			this.WasShownLimitDialog = true;
			textMsg = window.errorMessages.limitreached;
			if (command.params === undefined) {
				console.error('_onInfoMsg(): command.params is undefined!');
			} else {
				textMsg = textMsg.replace('{docs}', command.params[0]);
				textMsg = textMsg.replace('{connections}', command.params[1]);
			}
			textMsg = textMsg.replace(
				'{productname}',
				typeof brandProductName !== 'undefined'
					? brandProductName
					: 'Collabora Online Development Edition (unbranded)',
			);
			this._map.fire('infobar', {
				msg: textMsg,
				action: app.util.getProduct(),
				actionLabel: window.errorMessages.infoandsupport,
			});
		}
	}

	// 'warn: ' message.
	private _onWarnMsg(textMsg: string, command: ServerCommand): void {
		const len = 'warn: '.length;
		textMsg = textMsg.substring(len);
		if (textMsg.startsWith('saveas:')) {
			const userName = command.username ? command.username : _('Someone');
			const message = _(
				'{username} saved this document as {filename}. Do you want to join?',
			)
				.replace('{username}', userName)
				.replace('{filename}', command.filename as string);

			this._map.uiManager.showConfirmModal(
				'save-as-warning',
				'',
				message,
				_('OK'),
				function (this: Socket) {
					this._renameOrSaveAsCallback(textMsg, command);
				}.bind(this),
			);
		}
	}

	// 'blockui: ' message.
	private _onBlockUIMsg(textMsg: string): void {
		textMsg = textMsg.substring('blockui:'.length).trim();
		let msg: string | null = null;

		if (textMsg === 'rename') {
			msg = _('The document is being renamed and will reload shortly');
		} else if (textMsg === 'switchingtooffline') {
			msg = _(
				'The document is switching to Offline mode and will reload shortly',
			);
		}

		this._map.fire('blockUI', { message: msg });
	}

	// 'blockedcommand: ' message.
	private _onBlockedCommandMsg(textMsg: string): void {
		const blockedInfo = app.socket.parseServerCmd(textMsg.substring(16));
		if (blockedInfo.errorKind === 'restricted')
			window.app.console.log(
				'Restricted command "' + blockedInfo.errorCmd + '" was blocked',
			);
		else if (blockedInfo.errorKind === 'locked')
			this._map.openUnlockPopup(blockedInfo.errorCmd as string);
	}

	// 'slidelayer: ' message.
	private _onSlideLayerMsg(
		textMsg: string,
		e: SlurpMessageEvent | MinimalMessageEvent,
	): void {
		const content = JSON.parse(textMsg.substring('slidelayer:'.length + 1));
		this._map.fire('slidelayer', {
			message: content,
			image: (e as SlurpMessageEvent).image,
		});
	}

	// 'zstdslidelayer: ' message.
	private _onZstdSlideLayerMsg(
		textMsg: string,
		e: SlurpMessageEvent | MinimalMessageEvent,
	): void {
		const content = JSON.parse(textMsg.substring('zstdslidelayer:'.length + 1));
		const event = e as SlurpMessageEvent;
		if (event.imgBytes && event.imgIndex !== undefined)
			this._map.fire('slidelayer', {
				message: content,
				imgBytes: event.imgBytes.subarray(event.imgIndex),
			});
		else window.app.console.warn('zstdslidelayer with no image');
	}

	// 'sliderenderingcomplete: ' message.
	private _onSlideRenderingCompleteMsg(
		textMsg: string,
		e: SlurpMessageEvent | MinimalMessageEvent,
	): void {
		const json = JSON.parse(
			textMsg.substring('sliderenderingcomplete:'.length + 1),
		);
		this._map.fire('sliderenderingcomplete', {
			message: json,
			success: json.status === 'success',
		});
	}

	// 'progress: ' message.
	private _onProgressMsg(textMsg: string): void {
		const info = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
		if (!info) {
			window.app.console.error('Missing info in progress: message');
			return;
		}
		info.statusType = info.id;
		info.background = info.type == 'bg';

		if (info.id == 'find' || info.id == 'connect' || info.id == 'ready') {
			this._map.showBusy(
				window.ThisIsAMobileApp ? _('Loading...') : _('Connecting...'),
				true,
			);
			if (info.id == 'ready') {
				// We're connected: cancel timer and dialog.
				this.ReconnectCount = 0;
				clearTimeout(this.timer);
			}
		} else if (info.id == 'start' || info.id == 'setvalue') {
			this._map.fire('statusindicator', info);
		} else if (info.id == 'finish') {
			this._map.fire('statusindicator', info);
			this._map._fireInitComplete('statusindicatorfinish');
			// show shutting down popup after saving is finished
			// if we show the popup just after the shuttingdown message, it will be overwitten by save popup
			if (app.idleHandler._serverRecycling) {
				this._map.showBusy(_('Server is shutting down'), false);
			}
		} else {
			window.app.console.error('Unknown progress status ' + info.id);
		}
	}
}

app.definitions.Socket = Socket;
