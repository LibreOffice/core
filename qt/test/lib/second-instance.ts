/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { ChildProcess, spawn } from 'child_process';
import { existsSync } from 'fs';
import http from 'http';

// Document paths sit in the page URL encoded twice.
function decodeTwice(url: string): string {
	try {
		return decodeURIComponent(decodeURIComponent(url));
	} catch {
		return url;
	}
}

/**
 * Another coda-qt on the same HOME, as when the single-instance forward
 * fails: its own D-Bus session bus keeps it from finding the first one.
 */
export class SecondInstance {
	#proc: ChildProcess;
	#port: number;

	constructor() {
		this.#port = Number(process.env.REMOTE_DEBUGGING_PORT || 9222) + 1;
		// NixOS has no session.conf where dbus-run-session looks by default.
		const busConfig = [
			'/etc/dbus-1/session.conf',
			'/usr/share/dbus-1/session.conf',
			'/run/current-system/sw/share/dbus-1/session.conf',
		].find((path) => existsSync(path));
		this.#proc = spawn(
			'dbus-run-session',
			[
				...(busConfig ? [`--config-file=${busConfig}`] : []),
				'--',
				process.env.CODA_TEST_BINARY!,
			],
			{
				env: {
					...process.env,
					HOME: process.env.CODA_TEST_HOME!,
					QTWEBENGINE_REMOTE_DEBUGGING: this.#port.toString(),
					// Off screen, so it takes no focus from the first one.
					QT_QPA_PLATFORM: 'offscreen',
					QT_QPA_PLATFORMTHEME: 'generic',
					QT_ACCESSIBILITY: '0',
					NO_AT_BRIDGE: '1',
				},
				// Its own process group, so stop() also ends the bus daemon.
				detached: true,
				stdio: ['ignore', 'pipe', 'pipe'],
			},
		);
		const log = (d: Buffer) => {
			const msg = d.toString().trim();
			if (msg) console.log(`[coda-qt 2]: ${msg}`);
		};
		this.#proc.on('error', (e) => console.log(`[coda-qt 2]: ${e.message}`));
		this.#proc.stdout!.on('data', log);
		this.#proc.stderr!.on('data', log);
	}

	/** The URLs of the instance's pages, or none while it is starting. */
	async pageUrls(): Promise<string[]> {
		return new Promise((resolve) => {
			const req = http.get(
				`http://localhost:${this.#port}/json/list`,
				(res) => {
					let body = '';
					res.on('data', (chunk) => (body += chunk));
					res.on('end', () => {
						try {
							resolve(
								JSON.parse(body)
									.filter((t: any) => t.type === 'page')
									.map((t: any) => decodeTwice(t.url)),
							);
						} catch {
							resolve([]);
						}
					});
				},
			);
			req.on('error', () => resolve([]));
			req.setTimeout(1000, () => req.destroy());
		});
	}

	async stop(): Promise<void> {
		const proc = this.#proc;
		const signalGroup = (signal: NodeJS.Signals) => {
			try {
				process.kill(-proc.pid!, signal);
			} catch {
				// The group is gone already.
			}
		};
		if (proc.exitCode === null && !proc.signalCode) {
			const exited = new Promise<void>((resolve) => proc.once('exit', resolve));
			signalGroup('SIGTERM');
			await Promise.race([exited, new Promise((r) => setTimeout(r, 5000))]);
		}
		// The bus launcher can exit before the app in its group.
		signalGroup('SIGKILL');
	}
}
