/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface Path2DRecorderCall {
	method: string;
	args: any[];
}

/// Path2D mock recorder. Install it as a stand-in for the
/// global Path2D constructor in tests. The constructor argument (an
/// SVG path data string or another path) is captured into the path
/// field, and every method call is recorded into calls.
class Path2DRecorder {
	public readonly path: string;
	public readonly calls: Path2DRecorderCall[] = [];

	constructor(path?: string | Path2DRecorder) {
		if (typeof path === 'string') {
			this.path = path;
		} else if (path instanceof Path2DRecorder) {
			this.path = path.path;
		} else {
			this.path = '';
		}
	}

	private _record(method: string, args: any[]): void {
		this.calls.push({ method: method, args: args });
	}

	addPath(...args: any[]): void {
		this._record('addPath', args);
	}
	moveTo(...args: any[]): void {
		this._record('moveTo', args);
	}
	lineTo(...args: any[]): void {
		this._record('lineTo', args);
	}
	bezierCurveTo(...args: any[]): void {
		this._record('bezierCurveTo', args);
	}
	quadraticCurveTo(...args: any[]): void {
		this._record('quadraticCurveTo', args);
	}
	arc(...args: any[]): void {
		this._record('arc', args);
	}
	arcTo(...args: any[]): void {
		this._record('arcTo', args);
	}
	ellipse(...args: any[]): void {
		this._record('ellipse', args);
	}
	rect(...args: any[]): void {
		this._record('rect', args);
	}
	closePath(): void {
		this._record('closePath', []);
	}
}
