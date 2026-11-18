/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import WebSocket from 'ws';
import { decompress } from 'fzstd';

/// A message from the server.
export interface ServerMessage {
	/// The whole text of a text frame, or the first line of a binary frame, which names the
	/// message.
	text: string;
	/// The bytes after the first line of a binary frame. Undefined for a text frame.
	body?: Uint8Array;
}

/// The JSON object that a vector primitives answer or delta carries. A binary frame carries it
/// compressed with zstd after its first line. A text frame carries it after the name of the
/// message and the input sequence.
export function vectorJsonOf(message: ServerMessage): any {
	if (message.body)
		return JSON.parse(new TextDecoder().decode(decompress(message.body)));
	return JSON.parse(message.text.substring(message.text.indexOf('{')));
}

/// The JSON object of a text message, which follows the name of the message.
export function jsonOf(message: ServerMessage): any {
	return JSON.parse(message.text.substring(message.text.indexOf('{')));
}

/// The JSON object of a vector primitives message for partId, whether it came compressed or as
/// text, or undefined for any other message. A full answer comes as zstdvectorprimitives, and a
/// pushed delta as zstdvectorprimitivesdelta. Either falls back to commandvalues when it cannot
/// be compressed.
function vectorAnswerOf(message: ServerMessage, partId: string): any {
	if (
		!message.text.startsWith('zstdvectorprimitives:') &&
		!message.text.startsWith('zstdvectorprimitivesdelta:') &&
		!message.text.startsWith('commandvalues:')
	)
		return undefined;
	let json: any;
	try {
		json = vectorJsonOf(message);
	} catch (error) {
		return undefined;
	}
	if (json.partId !== partId) return undefined;
	if (json.type !== 'vectorprimitives' && json.type !== 'vectorprimitivesdelta')
		return undefined;
	return json;
}

/// Throws for a vector primitives message that came as text. The kit sends an answer and a pushed
/// delta that it could not compress as the same commandvalues text, so only the compressed ones
/// say which of the two they are.
function requireCompressed(message: ServerMessage): void {
	if (!message.body)
		throw new Error(
			'the kit sent vector primitives as text, where an answer and a pushed delta look the same',
		);
}

/// The parameters of a .uno:VectorPrimitives request.
export interface VectorPrimitivesOptions {
	/// The page list of the part, 0 for the slides.
	mode?: number;
	/// The version of the part that the client holds. The answer is then the change since that
	/// version.
	since?: number;
	/// The epoch that since counts in. Two versions compare only within one epoch.
	epoch?: number;
}

/// A client for the COOL protocol over WebSocket, running in Node. It connects to a running
/// coolwsd, loads a document, and lets tests send commands and wait for the answers.
export class SocketClient {
	private _socket: WebSocket;
	private _messages: ServerMessage[] = [];
	private _waiters: Array<{
		predicate: (message: ServerMessage) => boolean;
		resolve: (message: ServerMessage) => void;
		timer: NodeJS.Timeout;
	}> = [];

	/// Every message received so far, oldest first.
	get messages(): readonly ServerMessage[] {
		return this._messages;
	}

	constructor(socket: WebSocket) {
		this._socket = socket;
		// ws 7 gives a text frame as a string and a binary frame as a Buffer. Later versions
		// give both as a Buffer, with a flag that tells them apart.
		this._socket.on('message', (data: string | Buffer, isBinary?: boolean) => {
			const binary =
				isBinary !== undefined ? isBinary : typeof data !== 'string';
			const message = binary
				? SocketClient._splitBinary(data as Buffer)
				: { text: data.toString() };
			if (process.env.COOL_TEST_DEBUG) {
				const size = message.body ? ' <' + message.body.length + ' bytes>' : '';
				console.log('[ws<-]', message.text.slice(0, 200) + size);
			}
			this._messages.push(message);
			this._dispatch(message);
		});
	}

	private static _splitBinary(data: Buffer): ServerMessage {
		const newline = data.indexOf(10);
		if (newline < 0) return { text: '', body: new Uint8Array(data) };
		return {
			text: data.subarray(0, newline).toString(),
			body: new Uint8Array(data.subarray(newline + 1)),
		};
	}

	private _dispatch(message: ServerMessage): void {
		for (let i = 0; i < this._waiters.length; ++i) {
			const waiter = this._waiters[i];
			if (waiter.predicate(message)) {
				clearTimeout(waiter.timer);
				this._waiters.splice(i, 1);
				waiter.resolve(message);
				return;
			}
		}
	}

	/// Sends a protocol message.
	send(message: string): void {
		if (process.env.COOL_TEST_DEBUG)
			console.log('[ws->]', message.slice(0, 200));
		this._socket.send(message);
	}

	/// The number of messages received so far. A wait that starts from it matches only the
	/// messages that come after this call.
	mark(): number {
		return this._messages.length;
	}

	/// Waits for a message that matches the predicate. It looks at the messages received from
	/// the index since onward, and at the ones still to come. Without since, it looks only at
	/// the messages still to come, so an answer to an earlier request does not match.
	waitFor(
		predicate: (message: ServerMessage) => boolean,
		timeoutMs: number = 10000,
		since?: number,
	): Promise<ServerMessage> {
		if (since !== undefined) {
			for (let i = since; i < this._messages.length; ++i) {
				if (predicate(this._messages[i]))
					return Promise.resolve(this._messages[i]);
			}
		}
		return new Promise<ServerMessage>((resolve, reject) => {
			const timer = setTimeout(() => {
				const index = this._waiters.findIndex((w) => w.timer === timer);
				if (index >= 0) this._waiters.splice(index, 1);
				reject(new Error('SocketClient.waitFor timed out'));
			}, timeoutMs);
			this._waiters.push({ predicate, resolve, timer });
		});
	}

	/// Waits for a message whose text or first line starts with prefix, such as "status:" or
	/// "commandvalues:".
	waitForPrefix(
		prefix: string,
		timeoutMs?: number,
		since?: number,
	): Promise<ServerMessage> {
		return this.waitFor((m) => m.text.startsWith(prefix), timeoutMs, since);
	}

	/// Asks for the status of the document and returns its JSON object.
	async status(): Promise<any> {
		const since = this.mark();
		this.send('status');
		return jsonOf(await this.waitForPrefix('status:', undefined, since));
	}

	/// Asks for the vector primitives of a part and returns the JSON object of the answer.
	async vectorPrimitives(
		partId: string,
		options: VectorPrimitivesOptions = {},
	): Promise<any> {
		// The mode is 0, the slides, when it is not given. since and epoch go out only when they
		// are given, so a test can also send one without the other.
		let request =
			'commandvalues command=.uno:VectorPrimitives?partid=' +
			partId +
			'&mode=' +
			String(options.mode ?? 0);
		if (options.since !== undefined)
			request += '&since=' + String(options.since);
		if (options.epoch !== undefined)
			request += '&epoch=' + String(options.epoch);
		const mark = this.mark();
		this.send(request);
		// The answer to a request comes under zstdvectorprimitives, even when it is a delta.
		const answer = await this.waitFor(
			(m) =>
				!m.text.startsWith('zstdvectorprimitivesdelta:') &&
				vectorAnswerOf(m, partId) !== undefined,
			undefined,
			mark,
		);
		requireCompressed(answer);
		return vectorAnswerOf(answer, partId);
	}

	/// Waits for a pushed vector delta of the part and returns its JSON object. It looks at the
	/// messages from the index since onward, or at the ones still to come without since.
	async vectorDelta(
		partId: string,
		timeoutMs?: number,
		since?: number,
	): Promise<any> {
		const message = await this.waitFor(
			(m) =>
				!m.text.startsWith('zstdvectorprimitives:') &&
				vectorAnswerOf(m, partId)?.type === 'vectorprimitivesdelta',
			timeoutMs,
			since,
		);
		requireCompressed(message);
		return vectorAnswerOf(message, partId);
	}

	/// Asks for the file of the font face with the given id and returns the JSON object of the
	/// answer. The JSON is the first line of the answer, and the font file that follows it,
	/// compressed with zstd, is the bytes of the object, when the font is known.
	async vectorFont(fontId: string): Promise<any> {
		const mark = this.mark();
		this.send('commandvalues command=.uno:VectorRenderingFont?id=' + fontId);
		const answer = await this.waitForPrefix(
			'zstdvectorrenderingfont:',
			undefined,
			mark,
		);
		const values = jsonOf(answer);
		if (answer.body && answer.body.length > 0)
			values.bytes = decompress(answer.body);
		return values;
	}

	/// Asks for the image with the given checksum and returns the JSON object of the answer. The
	/// JSON is the first line of the answer, and the bytes of the image that follow it are the
	/// bytes of the object, when there is an image.
	async vectorGraphics(checksum: string): Promise<any> {
		const mark = this.mark();
		this.send(
			'commandvalues command=.uno:VectorRenderingGraphics?checksum=' + checksum,
		);
		const answer = await this.waitForPrefix(
			'vectorrenderinggraphics:',
			undefined,
			mark,
		);
		const values = jsonOf(answer);
		if (answer.body && answer.body.length > 0) values.bytes = answer.body;
		return values;
	}

	/// Asks for the primitive sample with the given name, such as "backgroundColor", and returns
	/// the JSON object of the answer. The answer holds "primitives" only when the kit serves the
	/// samples and has one under that name.
	async vectorPrimitiveSample(name: string): Promise<any> {
		const mark = this.mark();
		this.send('commandvalues command=.uno:VectorPrimitiveSample?name=' + name);
		const answer = await this.waitFor(
			(m) =>
				m.text.startsWith('commandvalues:') &&
				m.text.includes('".uno:VectorPrimitiveSample"') &&
				jsonOf(m).name === name,
			undefined,
			mark,
		);
		return jsonOf(answer);
	}

	/// The answer for the primitive sample with the given name, which holds "primitives". It
	/// throws when the kit serves no sample under that name.
	async primitiveSample(name: string): Promise<any> {
		const answer = await this.vectorPrimitiveSample(name);
		if (!Array.isArray(answer.primitives))
			throw new Error(
				'the kit serves no sample "' +
					name +
					'". Start coolwsd with COOL_VECTOR_PRIMITIVE_SAMPLES=1 in its environment.',
			);
		return answer;
	}

	/// Makes the part with the given page id the one that mouse and key input go to.
	setClientPart(partId: string): void {
		this.send('setclientpart part=' + partId);
	}

	/// Clicks the left button at a point of the current part, in twips.
	click(x: number, y: number): void {
		const at = ' x=' + String(Math.round(x)) + ' y=' + String(Math.round(y));
		this.send('mouse type=buttondown' + at + ' count=1 buttons=1 modifier=0');
		this.send('mouse type=buttonup' + at + ' count=1 buttons=1 modifier=0');
	}

	/// Presses and releases a key that types no character, such as an arrow, by its UNO key code.
	pressKey(keyCode: number): void {
		this.send('key type=input char=0 key=' + String(keyCode));
		this.send('key type=up char=0 key=' + String(keyCode));
	}

	/// Runs a UNO command, with its arguments as the JSON the browser sends.
	uno(command: string, args?: object): void {
		this.send(
			'uno ' + command + (args !== undefined ? ' ' + JSON.stringify(args) : ''),
		);
	}

	close(): void {
		this._socket.close();
	}

	/// Opens a WebSocket to a coolwsd instance and loads the document.
	/// baseUrl: such as "ws://localhost:9980".
	/// documentUrl: the file:// URL of the document.
	static async connectAndLoad(
		baseUrl: string,
		documentUrl: string,
	): Promise<SocketClient> {
		const encoded = encodeURIComponent(documentUrl);
		const url = baseUrl.replace(/\/$/, '') + '/cool/' + encoded + '/ws';

		// coolwsd takes a WebSocket upgrade only from an origin that matches its own URL.
		const origin = baseUrl.replace(/^ws/, 'http').replace(/\/$/, '');

		const socket = new WebSocket(url, { origin: origin });
		await new Promise<void>((resolve, reject) => {
			socket.once('open', () => resolve());
			socket.once('error', (error: Error) => reject(error));
		});

		const client = new SocketClient(socket);
		const mark = client.mark();
		client.send('coolclient 0.1');
		client.send('load url=' + encoded);
		// The status comes once the document is loaded and the engine takes commands. A load
		// that fails closes the socket, since the caller gets no client to close.
		try {
			await client.waitForPrefix('status:', 30000, mark);
		} catch (error) {
			client.close();
			throw error;
		}
		return client;
	}
}
