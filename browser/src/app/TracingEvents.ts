/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class TraceEvents {
	private recordingToggle: boolean;
	private asyncCounter: number;
	private asyncPseudoThread: number;
	private socket: Socket;

	constructor(socket: Socket) {
		this.socket = socket;
		this.recordingToggle = false;
		this.asyncCounter = 0;
		// simulate a threads per live async event to help the chrome
		// renderer.
		this.asyncPseudoThread = 1;
	}

	public getRecordingToggle(): boolean {
		return this.recordingToggle;
	}

	public decrementAsyncPseudoThread(): void {
		this.asyncPseudoThread--;
	}

	public setLogging(enabled: boolean) {
		this.recordingToggle = enabled;
		this.socket.sendMessage(
			'traceeventrecording ' + (this.recordingToggle ? 'start' : 'stop'),
		);

		// Just as a test, uncomment this to toggle SAL_WARN and
		// SAL_INFO selection between two states: 1) the default
		// as directed by the SAL_LOG environment variable, and
		// 2) all warnings on plus SAL_INFO for sc.
		//
		// (Note that coolwsd sets the SAL_LOG environment variable
		// to "-WARN-INFO", i.e. the default is that nothing is
		// logged from core.)

		// app.socket.sendMessage('sallogoverride ' + (app.socket.traceEventRecordingToggle ? '+WARN+INFO.sc' : 'default'));
	}

	public createAsync(name: string, args?: any): CompleteTraceEvent | null {
		if (!this.recordingToggle) return null;

		const result: CompleteTraceEvent = {
			id: -1,
			tid: -1,
			active: false,
			args: {},
			begin: 0,
			// eslint-disable-next-line @typescript-eslint/no-empty-function
			finish: () => {},
			// eslint-disable-next-line @typescript-eslint/no-empty-function
			abort: () => {},
		};

		result.id = this.asyncCounter++;
		result.tid = this.asyncPseudoThread++;
		result.active = true;
		result.args = args;

		this.send(name, 'S', undefined, args, result.id, result.tid);

		result.finish = () => {
			// this refers to the current TraceEvents object.
			this.decrementAsyncPseudoThread();
			if (result.active) {
				this.send(name, 'F', undefined, result.args, result.id, result.tid);
				result.active = false;
			}
		};
		result.abort = () => {
			// this refers to the current TraceEvents object.
			this.decrementAsyncPseudoThread();
			result.active = false;
		};
		return result;
	}

	public send(
		name: any,
		ph: string,
		timeRange: undefined | string,
		args?: any,
		id?: number | string,
		tid?: number | string,
	): void {
		if (timeRange === undefined)
			timeRange = 'ts=' + Math.round(performance.now() * 1000);
		if (!id) id = 1;
		if (!tid) tid = 1;

		this.socket.sendMessage(
			'TRACEEVENT name=' +
				JSON.stringify(name) +
				' ph=' +
				ph +
				' ' +
				timeRange +
				' id=' +
				id +
				' tid=' +
				tid +
				this.socket._stringifyArgs(args),
		);
	}

	public createComplete(name: string, args?: any): CompleteTraceEvent | null {
		if (!this.recordingToggle) return null;

		const result: CompleteTraceEvent = {
			id: -1,
			tid: -1,
			active: true,
			args: args,
			begin: performance.now(),

			finish: () => {
				if (result.active) {
					const now = performance.now();
					this.send(
						name,
						'X',
						'ts=' +
							Math.round(result.begin * 1000) +
							' dur=' +
							Math.round((now - result.begin) * 1000),
						args,
					);
					result.active = false;
				}
			},

			abort: () => {
				result.active = false;
			},
		};

		return result;
	}

	// something we can grok quickly in the trace viewer
	public createCompleteFromEvent(textMsg?: string): CompleteTraceEvent | null {
		if (!this.recordingToggle) return null;

		let pretty: string;
		if (!textMsg) pretty = 'blob';
		else {
			const idx = textMsg.indexOf(':');
			if (idx > 0) pretty = textMsg.substring(0, idx);
			else if (textMsg.length < 25) pretty = textMsg;
			else pretty = textMsg.substring(0, 25);
		}
		return this.createComplete(pretty, { message: textMsg });
	}
}
