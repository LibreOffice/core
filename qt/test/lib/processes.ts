/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Helpers for the launcher services: the processes they start, and the HTTP endpoints those
// processes answer on.

import { ChildProcess } from 'child_process';
import http from 'http';
import { promisify } from 'util';

export const sleep = promisify(setTimeout);

export function pipeOutput(proc: ChildProcess, label: string): void {
	proc.stdout!.on('data', (d: Buffer) =>
		console.log(`[${label}]: ${d.toString().trim()}`),
	);
	proc.stderr!.on('data', (d: Buffer) => {
		const msg = d.toString().trim();
		if (msg) console.log(`[${label}]: ${msg}`);
	});
	proc.on('exit', (c, s) =>
		console.log(`${label} exited (code=${c}, signal=${s})`),
	);
}

export async function waitForHttp(
	url: string,
	label: string,
	maxAttempts = 30,
): Promise<void> {
	for (let i = 0; i < maxAttempts; i++) {
		try {
			const ok = await new Promise((resolve, reject) => {
				const req = http.get(url, (res) => resolve(res.statusCode === 200));
				req.on('error', reject);
				req.setTimeout(1000, () => req.destroy(new Error('timeout')));
			});
			if (ok) {
				console.log(`${label} is ready`);
				return;
			}
		} catch (e) {
			// Not ready yet
		}
		await sleep(1000);
	}
	throw new Error(
		`${label} not available at ${url} after ${maxAttempts} attempts`,
	);
}

export function killProcess(proc: ChildProcess | null, name: string): void {
	if (proc && proc.pid && !proc.killed) {
		console.log(`Sending SIGTERM to ${name}`);
		proc.kill('SIGTERM');
		const timer = setTimeout(() => {
			if (proc.exitCode === null) {
				console.log(`Forcing SIGKILL on ${name}`);
				proc.kill('SIGKILL');
			}
		}, 2000);
		proc.once('exit', () => clearTimeout(timer));
	}
}
