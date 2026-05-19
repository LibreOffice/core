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

interface CanvasRecorderCall {
	method: string;
	args: any[];
}

/// Test helper that mimics a CanvasRenderingContext2D. Pass an instance of
/// CanvasRecorder wherever production code expects a context. Every method
/// invocation is recorded into calls, and every assigned drawing-state
/// property is recorded into properties.
class CanvasRecorder {
	public readonly calls: CanvasRecorderCall[] = [];
	public readonly properties: Record<string, any> = {};
	public readonly canvas: { width: number; height: number };

	private static readonly _PROPS = [
		'fillStyle',
		'strokeStyle',
		'globalAlpha',
		'lineWidth',
		'lineCap',
		'lineJoin',
		'miterLimit',
		'font',
		'textAlign',
		'textBaseline',
		'shadowBlur',
		'shadowColor',
		'shadowOffsetX',
		'shadowOffsetY',
		'globalCompositeOperation',
	];

	constructor(width: number = 100, height: number = 100) {
		this.canvas = { width: width, height: height };

		for (const p of CanvasRecorder._PROPS) {
			Object.defineProperty(this, p, {
				get: () => this.properties[p],
				set: (v) => {
					this.properties[p] = v;
				},
			});
		}
	}

	private _record(method: string, args: any[]): void {
		this.calls.push({ method: method, args: args });
	}

	// inspection helpers

	/// First call of the given method, or undefined.
	findCall(method: string): CanvasRecorderCall | undefined {
		return this.calls.find((c) => c.method === method);
	}

	/// All calls of the given method, in order.
	callsOf(method: string): CanvasRecorderCall[] {
		return this.calls.filter((c) => c.method === method);
	}

	/// Total number of times the given method was called.
	countOf(method: string): number {
		return this.callsOf(method).length;
	}

	// canvas context surface

	fillRect(...args: any[]): void {
		this._record('fillRect', args);
	}
	strokeRect(...args: any[]): void {
		this._record('strokeRect', args);
	}
	clearRect(...args: any[]): void {
		this._record('clearRect', args);
	}
	fill(...args: any[]): void {
		this._record('fill', args);
	}
	stroke(...args: any[]): void {
		this._record('stroke', args);
	}
	beginPath(): void {
		this._record('beginPath', []);
	}
	closePath(): void {
		this._record('closePath', []);
	}
	moveTo(...args: any[]): void {
		this._record('moveTo', args);
	}
	lineTo(...args: any[]): void {
		this._record('lineTo', args);
	}
	rect(...args: any[]): void {
		this._record('rect', args);
	}
	arc(...args: any[]): void {
		this._record('arc', args);
	}
	bezierCurveTo(...args: any[]): void {
		this._record('bezierCurveTo', args);
	}
	quadraticCurveTo(...args: any[]): void {
		this._record('quadraticCurveTo', args);
	}
	fillText(...args: any[]): void {
		this._record('fillText', args);
	}
	strokeText(...args: any[]): void {
		this._record('strokeText', args);
	}
	drawImage(...args: any[]): void {
		this._record('drawImage', args);
	}
	clip(...args: any[]): void {
		this._record('clip', args);
	}
	save(): void {
		this._record('save', []);
	}
	restore(): void {
		this._record('restore', []);
	}
	translate(...args: any[]): void {
		this._record('translate', args);
	}
	scale(...args: any[]): void {
		this._record('scale', args);
	}
	rotate(...args: any[]): void {
		this._record('rotate', args);
	}
	transform(...args: any[]): void {
		this._record('transform', args);
	}
	setTransform(...args: any[]): void {
		this._record('setTransform', args);
	}
	resetTransform(): void {
		this._record('resetTransform', []);
	}
}
