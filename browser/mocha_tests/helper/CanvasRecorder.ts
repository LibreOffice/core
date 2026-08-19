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
	/// Snapshot of canvas state at the moment the call was made. A
	/// shallow copy of recorder.properties is stored here, so a test
	/// can ask "what was strokeStyle at this stroke() call?" even
	/// after later primitives overwrite it.
	properties: Record<string, any>;
	/// Save/restore nesting depth at the time of the call. Top-level
	/// calls are depth 0. The save() call itself is recorded at the
	/// outer depth, and restore() at the outer depth too. Everything
	/// in between is at outer+1 (or deeper if nested saves happen).
	depth: number;
}

/// A gradient handed out by CanvasRecorder. The stops it was given are
/// kept in the order they were added, so a test can check where along
/// the ramp each color was placed.
class GradientRecorder {
	public readonly stops: { offset: number; color: string }[] = [];

	constructor(
		public readonly kind: string,
		public readonly args: any[],
	) {}

	addColorStop(offset: number, color: string): void {
		this.stops.push({ offset: offset, color: color });
	}

	/// The stops sorted the way the canvas reads them, which is by
	/// offset regardless of the order they were added in.
	sortedStops(): { offset: number; color: string }[] {
		return this.stops.slice().sort((a, b) => a.offset - b.offset);
	}
}

/// Test helper that mimics a CanvasRenderingContext2D. Pass an instance of
/// CanvasRecorder wherever production code expects a context. Every method
/// invocation is recorded into calls, and every assigned drawing-state
/// property is recorded into properties.
///
/// The properties field holds the live "last set" value of each tracked
/// canvas property. Save/restore do not pop these back. For an accurate
/// "what was the state at the time of this call" view, read the
/// properties field on the call entry returned by findCall/callsOf.
class CanvasRecorder {
	public readonly calls: CanvasRecorderCall[] = [];
	public readonly properties: Record<string, any> = {};
	public readonly canvas: { width: number; height: number };
	/// Every gradient handed out, in the order it was created.
	public readonly gradients: GradientRecorder[] = [];
	private _depth: number = 0;
	/// The live transform as [a, b, c, d, e, f], kept up to date through
	/// the transform calls and the save stack so getTransform answers
	/// what a real context would.
	private _transform: number[] = [1, 0, 0, 1, 0, 0];
	private _transformStack: number[][] = [];

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
		'filter',
	];

	constructor(width: number = 100, height: number = 100) {
		this.canvas = { width: width, height: height };

		for (const property of CanvasRecorder._PROPS) {
			Object.defineProperty(this, property, {
				get: () => this.properties[property],
				set: (value) => {
					this.properties[property] = value;
				},
			});
		}
	}

	private _record(method: string, args: any[]): void {
		this.calls.push({
			method: method,
			args: args,
			properties: { ...this.properties },
			depth: this._depth,
		});
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
	measureText(text: string): TextMetrics {
		// Deterministic stub. Real canvases measure against the
		// current font and OS font tables; for tests we just give
		// each character a fixed advance so assertions can predict
		// the result.
		return { width: text.length * 10 } as TextMetrics;
	}
	drawImage(...args: any[]): void {
		this._record('drawImage', args);
	}
	clip(...args: any[]): void {
		this._record('clip', args);
	}
	save(): void {
		// Record at the outer depth, then nest subsequent calls
		// one level deeper.
		this._record('save', []);
		this._depth++;
		this._transformStack.push(this._transform.slice());
	}
	restore(): void {
		// Pop the level first so the restore() entry itself sits at
		// the outer depth, matching the save() entry that opened it.
		if (this._depth > 0) this._depth--;
		this._record('restore', []);
		const saved = this._transformStack.pop();
		if (saved) this._transform = saved;
	}
	translate(...args: any[]): void {
		this._record('translate', args);
		this._apply([1, 0, 0, 1, args[0], args[1]]);
	}
	scale(...args: any[]): void {
		this._record('scale', args);
		this._apply([args[0], 0, 0, args[1], 0, 0]);
	}
	rotate(...args: any[]): void {
		this._record('rotate', args);
		const cos = Math.cos(args[0]);
		const sin = Math.sin(args[0]);
		this._apply([cos, sin, -sin, cos, 0, 0]);
	}
	transform(...args: any[]): void {
		this._record('transform', args);
		this._apply(args.slice(0, 6));
	}
	setTransform(...args: any[]): void {
		this._record('setTransform', args);
		const first = args[0];
		this._transform =
			args.length >= 6
				? args.slice(0, 6)
				: [first.a, first.b, first.c, first.d, first.e, first.f];
	}
	resetTransform(): void {
		this._record('resetTransform', []);
		this._transform = [1, 0, 0, 1, 0, 0];
	}
	getTransform(): {
		a: number;
		b: number;
		c: number;
		d: number;
		e: number;
		f: number;
	} {
		const t = this._transform;
		return { a: t[0], b: t[1], c: t[2], d: t[3], e: t[4], f: t[5] };
	}

	/// Multiply the live transform by another, the way a canvas
	/// composes a new transform onto the one already in place.
	private _apply(m: number[]): void {
		const t = this._transform;
		this._transform = [
			t[0] * m[0] + t[2] * m[1],
			t[1] * m[0] + t[3] * m[1],
			t[0] * m[2] + t[2] * m[3],
			t[1] * m[2] + t[3] * m[3],
			t[0] * m[4] + t[2] * m[5] + t[4],
			t[1] * m[4] + t[3] * m[5] + t[5],
		];
	}
	setLineDash(...args: any[]): void {
		this._record('setLineDash', args);
	}
	createLinearGradient(...args: any[]): GradientRecorder {
		return this._makeGradient('linear', args);
	}
	createRadialGradient(...args: any[]): GradientRecorder {
		return this._makeGradient('radial', args);
	}
	createPattern(...args: any[]): null {
		this._record('createPattern', args);
		return null;
	}

	private _makeGradient(kind: string, args: any[]): GradientRecorder {
		this._record(
			'create' + kind[0].toUpperCase() + kind.slice(1) + 'Gradient',
			args,
		);
		const gradient = new GradientRecorder(kind, args);
		this.gradients.push(gradient);
		return gradient;
	}
}
