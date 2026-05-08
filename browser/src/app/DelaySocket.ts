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

// DelaySocket wraps an existing SockInterface and forwards send() and
// onmessage events with a fixed delay while preserving FIFO order in
// each direction. Open / close / error callbacks and readyState /
// binaryType are passed straight through.

class DelaySocket implements SockInterface {
	private _inner: SockInterface;
	private _delayMs: number;

	private _sendQueue: { ts: number; data: MessageInterface }[] = [];
	private _sendTimer: ReturnType<typeof setTimeout> | null = null;
	private _recvQueue: { ts: number; evt: MessageEvent }[] = [];
	private _recvTimer: ReturnType<typeof setTimeout> | null = null;

	public onclose: (event: CloseEvent) => void = () => {};
	public onerror: (event: Event) => void = () => {};
	public onmessage: (event: MessageEvent) => void = () => {};
	public onopen: (event: Event) => void = () => {};

	constructor(inner: SockInterface, delayMs: number) {
		this._inner = inner;
		this._delayMs = delayMs;

		this._inner.onopen = (e: Event) => this.onopen(e);
		this._inner.onerror = (e: Event) => this.onerror(e);
		this._inner.onclose = (e: CloseEvent) => this.onclose(e);
		this._inner.onmessage = (e: MessageEvent) => this._enqueueRecv(e);
	}

	public get readyState(): 0 | 1 | 2 | 3 {
		return this._inner.readyState;
	}

	public get binaryType(): 'blob' | 'arraybuffer' {
		return this._inner.binaryType;
	}

	public set binaryType(v: 'blob' | 'arraybuffer') {
		this._inner.binaryType = v;
	}

	public send(data: MessageInterface): void {
		this._sendQueue.push({ ts: Date.now(), data });
		if (!this._sendTimer) this._scheduleSend();
	}

	public close(code?: number, reason?: string): void {
		if (this._sendTimer) clearTimeout(this._sendTimer);
		if (this._recvTimer) clearTimeout(this._recvTimer);
		this._sendTimer = null;
		this._recvTimer = null;
		this._sendQueue = [];
		this._recvQueue = [];
		this._inner.close(code, reason);
	}

	public setUnloading(): void {
		if (this._inner.setUnloading) this._inner.setUnloading();
	}

	// Detach from the inner socket: flush queued messages immediately
	// (preserving order), stop timers, and return the inner socket so
	// the caller can rewire callbacks back to it.
	public unwrap(): SockInterface {
		if (this._sendTimer) clearTimeout(this._sendTimer);
		this._sendTimer = null;
		for (const item of this._sendQueue) this._inner.send(item.data);
		this._sendQueue = [];

		if (this._recvTimer) clearTimeout(this._recvTimer);
		this._recvTimer = null;
		for (const item of this._recvQueue) this.onmessage(item.evt);
		this._recvQueue = [];

		return this._inner;
	}

	private _scheduleSend(): void {
		if (this._sendQueue.length === 0) return;
		const head = this._sendQueue[0];
		const wait = Math.max(0, head.ts + this._delayMs - Date.now());
		this._sendTimer = setTimeout(() => {
			this._sendTimer = null;
			const item = this._sendQueue.shift();
			if (item) this._inner.send(item.data);
			this._scheduleSend();
		}, wait);
	}

	private _enqueueRecv(evt: MessageEvent): void {
		this._recvQueue.push({ ts: Date.now(), evt });
		if (!this._recvTimer) this._scheduleRecv();
	}

	private _scheduleRecv(): void {
		if (this._recvQueue.length === 0) return;
		const head = this._recvQueue[0];
		const wait = Math.max(0, head.ts + this._delayMs - Date.now());
		this._recvTimer = setTimeout(() => {
			this._recvTimer = null;
			const item = this._recvQueue.shift();
			if (item) this.onmessage(item.evt);
			this._scheduleRecv();
		}, wait);
	}
}
