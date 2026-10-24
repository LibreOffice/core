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

// Keeps the messages the client sends to the server, in place of a live connection.
class SocketRecorder {
	// The messages sent since the recorder was made, oldest first.
	readonly sent: string[] = [];

	private readonly savedSend: any;

	// Takes the place of app.socket.sendMessage until restore is called.
	constructor() {
		const socket: any = (app as any).socket || ((app as any).socket = {});
		this.savedSend = socket.sendMessage;
		socket.sendMessage = (message: string) => {
			this.sent.push(message);
		};
	}

	// Puts back the sendMessage that was there before.
	restore(): void {
		(app as any).socket.sendMessage = this.savedSend;
	}

	// Records what the steps send, and puts back the sendMessage from before even when they
	// throw.
	static during(steps: () => void): string[] {
		const recorder = new SocketRecorder();
		try {
			steps();
		} finally {
			recorder.restore();
		}
		return recorder.sent;
	}
}
