// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Logger contains methods for logging the activity
 */

type Direction = 'INCOMING' | 'OUTGOING';
interface LogMsg {
	msg: string;
	direction: Direction;
	status: string;
	time: number;
}

class Logger {
	private _logs: LogMsg[];
	private startTime: number;
	private _cypressTest: boolean;

	constructor() {
		this._logs = [];
		this.startTime = null;
		this._cypressTest =
			typeof navigator !== 'undefined' &&
			navigator.userAgent.toLowerCase().indexOf('cypress') !== -1;
	}

	public log(msg: string, direction: Direction, status: string = ''): void {
		const time = Date.now();
		if (!this.startTime) this.startTime = time;

		// Limit memory usage of log by only keeping the latest entries
		let maxEntries = 100;
		if (window.enableDebug || this._cypressTest) maxEntries = 1000;

		if (time - this.startTime < 60 * 1000 /* ms */) maxEntries = 500; // enough to capture early start.
		while (this._logs.length > maxEntries) this._logs.shift();

		// Limit memory usage of log by limiting length of message
		const maxMsgLen = this._cypressTest ? 1024 : 128;
		if (msg.length > maxMsgLen) msg = msg.substring(0, maxMsgLen);
		msg = msg.replace(/(\r\n|\n|\r)/gm, ' ');
		this._logs.push({
			msg: msg,
			direction: direction,
			status: status,
			time: time,
		});
	}

	private _getEntries(): string {
		this._logs.sort(function (a, b) {
			if (a.time < b.time) {
				return -1;
			}
			if (a.time > b.time) {
				return 1;
			}
			return 0;
		});
		let data: string = '';
		for (let i = 0; i < this._logs.length; i++) {
			data +=
				this._logs[i].time +
				'.' +
				this._logs[i].direction +
				this._logs[i].status +
				'.' +
				this._logs[i].msg;
			data += '\n';
		}
		return data;
	}

	public print(): void {
		window.app.console.log('Queued log messages:');
		window.app.console.log(this._getEntries());
		window.app.console.log('End of queued log messages:');
	}

	public save(): void {
		const blob = new Blob([this._getEntries()], { type: 'text/csv' });
		const a = document.createElement('a');

		a.download = Date.now() + '.csv';
		a.href = window.URL.createObjectURL(blob);
		a.dataset.downloadurl = ['text/csv', a.download, a.href].join(':');

		const e = new MouseEvent('click', {
			bubbles: true,
			cancelable: false,
			view: window,
			detail: 0,
			screenX: 0,
			screenY: 0,
			clientX: 0,
			clientY: 0,
			ctrlKey: false,
			altKey: false,
			shiftKey: false,
			metaKey: false,
			button: 0,
			relatedTarget: null,
		});
		a.dispatchEvent(e);
	}

	public clear(): void {
		this._logs = [];
	}
}

app.Log = new Logger();
