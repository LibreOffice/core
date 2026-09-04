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

// Web Worker that runs LibreLogo programs off the panel's UI thread. The
// panel terminates the worker to stop a runaway program, so the turtle state
// travels with every frame message and is restored into a fresh worker.
//
// Messages in:  { type: 'init', lang, table, pageSize, state }
//               { type: 'run', source }
//               { type: 'input', value }        answer to an input request
// Messages out: { type: 'ready' }
//               { type: 'print', text }
//               { type: 'input', prompt }
//               { type: 'frame', svg, bounds, state, final }
//               { type: 'error', message, line, kind }
//               { type: 'done', halted }

import { LogoLocale } from './l10n.js';
import { Interpreter, HaltSignal } from './interp.js';
import { Turtle } from './turtle.js';
import { SvgRenderer, drawingBounds, bbox } from './svg.js';
import { LogoError } from './values.js';
import { LogoSyntaxError } from './lexer.js';

let locale = null;
let turtle = null;
let interp = null;
let renderer = null;
let pendingInput = null;
let measureCtx = null;

const FRAME_INTERVAL_MS = 200;

function measureText(text, font) {
	if (measureCtx === null) {
		try {
			const canvas = new OffscreenCanvas(10, 10);
			measureCtx = canvas.getContext('2d') || false;
		} catch (e) {
			measureCtx = false;
		}
	}
	if (!measureCtx) return text.length * font.size * 0.52;
	measureCtx.font = (font.style === 2 ? 'italic ' : '') + (font.weight > 100 ? 'bold ' : '') + font.size + 'px "' + font.family + '", "Linux Biolinum G", serif';
	return measureCtx.measureText(text).width;
}

function turtleState() {
	return {
		x: turtle.x, y: turtle.y, heading: turtle.heading, visible: turtle.visible,
		pencolor: turtle.pencolor, fillcolor: turtle.areacolor, pensize: turtle.pensize,
	};
}

function frame(final) {
	const bounds = drawingBounds(turtle.shapes, turtleState());
	const pw = turtle.page[0], ph = turtle.page[1];
	// The preview never clips: it frames the union of the page and the drawing,
	// with the page drawn as a light outline for reference.
	let view = [0, 0, pw, ph];
	if (bounds) {
		const m = 4;
		const minX = Math.min(0, bounds[0] - m), minY = Math.min(0, bounds[1] - m);
		const maxX = Math.max(pw, bounds[2] + m), maxY = Math.max(ph, bounds[3] + m);
		view = [minX, minY, maxX - minX, maxY - minY];
	}
	const svg = renderer.render(turtle.shapes, turtleState(), view, [pw, ph]);
	postMessage({ type: 'frame', svg, view, bounds, page: turtle.page, state: turtle.snapshot(), final: !!final, position: turtle.getPosition(), heading: turtle.heading });
}

function errorMessage(e) {
	if (e instanceof LogoSyntaxError) {
		// Block and argument errors carry a localized message already; the
		// lexer's other diagnostics are developer-facing English, so show
		// LibreLogo's generic "Error (in line N)" and keep the detail in the console.
		const localized = e.kind === 'block' || e.kind === 'arguments' || e.kind === 'name';
		if (!localized && e.line) console.warn('LibreLogo syntax error, line ' + e.line + ': ' + e.message);
		const text = localized ? e.message : locale.message('ERROR', e.line || '?');
		return { message: text, line: e.line, kind: e.kind };
	}
	if (e instanceof LogoError) {
		let text;
		switch (e.kind) {
		case 'ERR_MAXRECURSION': text = locale.message('ERR_STOP') + ' ' + locale.message('ERR_MAXRECURSION', ...e.args); break;
		case 'ERR_MEMORY': text = locale.message('ERR_STOP') + ' ' + locale.message('ERR_MEMORY'); break;
		case 'ERROR': text = locale.message('ERROR', e.line || '?'); break;
		default: text = locale.message(e.kind, ...e.args); break;
		}
		return { message: text, line: e.line, kind: e.kind };
	}
	if (e instanceof RangeError) {
		return { message: locale.message('ERR_STOP') + ' ' + locale.message('ERR_MAXRECURSION', 400), line: null, kind: 'ERR_MAXRECURSION' };
	}
	return { message: String(e && e.message ? e.message : e), line: null, kind: 'internal' };
}

async function run(source) {
	let program;
	try {
		program = interp.compile(source);
	} catch (e) {
		postMessage({ type: 'error', ...errorMessage(e) });
		frame(true);
		postMessage({ type: 'done', halted: true });
		return;
	}
	const gen = interp.run(program);
	let nextValue;
	let lastFrame = Date.now();
	let halted = false;
	try {
		for (;;) {
			const { value, done } = gen.next(nextValue);
			nextValue = undefined;
			if (done) break;
			switch (value.type) {
			case 'print':
				postMessage({ type: 'print', text: value.text });
				break;
			case 'input':
				frame(false);
				postMessage({ type: 'input', prompt: value.prompt });
				nextValue = await new Promise((resolve) => { pendingInput = resolve; });
				pendingInput = null;
				break;
			case 'sleep':
				frame(false);
				lastFrame = Date.now();
				// Real waits keep the animation feel of SLEEP for short pauses.
				await new Promise((resolve) => setTimeout(resolve, Math.min(value.ms, 2000)));
				break;
			case 'export': {
				// Render just this picture's shape to its own SVG, sized to its
				// bounding box, for the panel to save as a file.
				const box = bbox(value.shape);
				if (box) {
					const m = 2;
					const view = [box[0] - m, box[1] - m, box[2] - box[0] + 2 * m, box[3] - box[1] + 2 * m];
					// Animate the export when the program used SLEEP.
					const svg = renderer.render([value.shape], null, view, null, source, true);
					postMessage({ type: 'export', name: value.name, svg });
				}
				break;
			}
			case 'step':
				if (Date.now() - lastFrame > FRAME_INTERVAL_MS) {
					frame(false);
					lastFrame = Date.now();
					// Let queued messages (none today, but harmless) through.
					await new Promise((resolve) => setTimeout(resolve, 0));
				}
				break;
			default:
				break;
			}
		}
	} catch (e) {
		if (e instanceof HaltSignal) halted = true;
		else postMessage({ type: 'error', ...errorMessage(e) });
	}
	frame(true);
	postMessage({ type: 'done', halted });
}

onmessage = (e) => {
	const msg = e.data;
	switch (msg.type) {
	case 'init':
		locale = new LogoLocale(msg.table, msg.lang);
		turtle = new Turtle(locale, { pageSize: msg.pageSize, measureText });
		if (msg.state) turtle.restore(msg.state);
		if (msg.pageSize) turtle.page = msg.pageSize.slice();
		interp = new Interpreter(locale, turtle, {});
		renderer = new SvgRenderer({ locale });
		postMessage({ type: 'ready', lang: locale.lang });
		break;
	case 'run':
		run(msg.source);
		break;
	case 'input':
		if (pendingInput) pendingInput(msg.value);
		break;
	case 'frame':
		frame(true);
		break;
	case 'clear':
		turtle.clearscreen();
		frame(true);
		break;
	case 'home':
		turtle.home();
		frame(true);
		break;
	default:
		break;
	}
};
