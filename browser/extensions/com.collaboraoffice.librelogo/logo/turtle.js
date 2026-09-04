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

// The turtle: position, heading, pen and fill attributes, and the list of
// drawing primitives the program produced. Coordinates are in points with
// the origin at the top-left corner of the page and y growing downwards, as
// in LibreLogo; heading 0 points up and LEFT turns counterclockwise.
//
// Primitives are plain objects rendered by svg.js (and, later, by a kit-side
// renderer that creates real Writer shapes):
//   { kind: 'polyline', subpaths: [[[x, y], ...], ...], stroke }
//   { kind: 'polygon', points: [[x, y], ...], stroke, fill, filled }
//   { kind: 'ellipse', cx, cy, rx, ry, rotate, start, end, arcKind, stroke, fill }
//   { kind: 'rect', cx, cy, w, h, rotate, radius, stroke, fill }
//   { kind: 'dots', points, size, color }
//   { kind: 'label', x, y, rotate, text, font }
//   { kind: 'group', children }
// stroke = { color, width, style, joint, cap }, fill = { color, hatch } where a
// color is LibreLogo's integer (0xTTRRGGBB, TT = transparency 0..255) or a
// gradient array [color1, color2, style, angle, border, x, y, i1, i2].

import { LogoError, Tuple, seqItems, isSequence } from './values.js';
import { COLORS } from './l10n.js';

export const DEFAULT_PAGE = [595.3, 841.89]; // A4 in points
const DEFAULT_FILLCOLOR = 0x8000cc00; // 50% transparent green
const DEFAULT_PENSIZE = 0.5;
const BASE_FONT = 'Linux Biolinum G';
const INVISIBLE = 0xffffffff;

// Pleasant random colors: ported from LibreLogo.py's __NORMCOLORS__ table.
const NORMCOLORS = [[[255, 255, 0], 0, -11, 1, -11],
	[[255, 128, 0], 1, 116, 1, -33], [[255, 0, 0], 1, 95, 2, 42],
	[[255, 0, 255], 2, -213, 0, -106], [[0, 0, 255], 0, 148, 1, 127],
	[[0, 255, 255], 1, -128, 2, -63], [[0, 255, 0], 2, 192, 0, 244]];

const HATCH_PRESETS = [[1, 0, 5, 0], [1, 0, 5, 45], [1, 0, 5, -45], [1, 0, 5, 90],
	[2, [127, 0, 0], 5, 45], [2, [127, 0, 0], 5, 0], [2, [0, 0, 127], 5, 45],
	[2, [0, 0, 127], 5, 0], [3, [0, 0, 127], 5, 0], [1, 0, 25, 45]];

function deg2rad(d) {
	return d * Math.PI / 180;
}

export function colorComponents(c) {
	const a = [(c & 0xff0000) >> 16, (c & 0xff00) >> 8, c & 0xff];
	if (c > 0xffffff) a.push((c >>> 24) & 0xff);
	return a;
}

export class Turtle {
	constructor(locale, options) {
		this.locale = locale;
		this.random = (options && options.random) || Math.random;
		this.page = (options && options.pageSize) || DEFAULT_PAGE.slice();
		this.measure = (options && options.measureText) || null;
		this.shapes = [];
		this.groupStack = [];
		this.time = 0;
		this.visible = true;
		this.reset();
		this.x = this.page[0] / 2;
		this.y = this.page[1] / 2;
		this.heading = 0;
	}

	// Pen, fill and font defaults, as LibreLogo.py's __Doc__.initialize().
	reset() {
		this.pen = true;
		this.pencolor = 0;
		this.pensize = DEFAULT_PENSIZE;
		this.linestyle = 'solid'; // 'solid' | 'dashed' | 'dotted' | [dots, dotlen, dashes, dashlen, distance, type]
		this.linejoint = 'rounded';
		this.linecap = 'none';
		this.areacolor = DEFAULT_FILLCOLOR;
		this.filltransparency = (DEFAULT_FILLCOLOR >>> 24) / 2.55;
		this.hatch = null;
		this.textcolor = 0;
		this.fontfamily = BASE_FONT;
		this.fontheight = 12;
		this.fontweight = 100;
		this.fontstyle = 0;
		this.points = [];
		this.current = null; // the polyline being extended
		this.actual = null; // the last shape, for TEXT and FILL
		this.continuous = true;
	}

	// --- snapshot for persistence between runs --------------------------

	snapshot() {
		return JSON.parse(JSON.stringify({
			x: this.x, y: this.y, heading: this.heading, pen: this.pen, pencolor: this.pencolor,
			pensize: this.pensize, linestyle: this.linestyle, linejoint: this.linejoint,
			linecap: this.linecap, areacolor: this.areacolor, filltransparency: this.filltransparency,
			hatch: this.hatch, textcolor: this.textcolor, fontfamily: this.fontfamily,
			fontheight: this.fontheight, fontweight: this.fontweight, fontstyle: this.fontstyle,
			visible: this.visible, shapes: this.shapes, page: this.page,
		}));
	}

	restore(state) {
		if (!state) return;
		Object.assign(this, JSON.parse(JSON.stringify(state)));
		this.current = null;
		this.actual = null;
		this.points = [];
		this.groupStack = [];
	}

	// --- geometry helpers ---------------------------------------------------

	target() {
		return this.groupStack.length ? this.groupStack[this.groupStack.length - 1].children : this.shapes;
	}

	addShape(shape) {
		shape.t = this.time;
		this.target().push(shape);
		this.actual = shape;
		return shape;
	}

	strokeStyle() {
		return {
			color: this.pen ? this.pencolor : INVISIBLE,
			width: this.pensize,
			style: this.linestyle,
			joint: this.linejoint,
			cap: this.linecap,
		};
	}

	fillStyle() {
		return { color: this.areacolor, hatch: this.hatch };
	}

	styleKey() {
		return JSON.stringify([this.pencolor, this.pensize, this.linestyle, this.linejoint, this.linecap]);
	}

	num(v) {
		if (typeof v === 'number') return v;
		if (typeof v === 'boolean') return Number(v);
		throw new LogoError('ERROR');
	}

	pagesize() {
		return [this.page[0], this.page[1]];
	}

	// --- movement -------------------------------------------------------------

	// Move to (nx, ny), drawing a line if the pen is down.
	moveTo(nx, ny, dot) {
		const from = [this.x, this.y];
		this.x = nx;
		this.y = ny;
		if (!this.pen && !dot) {
			this.continuous = false;
			return;
		}
		const key = this.styleKey();
		let shape = this.current;
		if (shape && shape.styleKey !== key) shape = null;
		if (shape) {
			if (this.continuous) shape.subpaths[shape.subpaths.length - 1].push([nx, ny]);
			else shape.subpaths.push([from, [nx, ny]]);
			this.continuous = true;
			this.actual = shape;
			return;
		}
		this.points = [];
		shape = this.addShape({ kind: 'polyline', subpaths: [[from, [nx, ny]]], stroke: this.strokeStyle(), styleKey: key });
		this.current = shape;
		this.continuous = true;
	}

	// Start a new line shape (PICTURE without a block).
	newShape() {
		this.current = null;
		this.actual = null;
	}

	forward(n) {
		if (typeof n === 'string') {
			// FORWARD "text": print the label and move past it
			const size = this.label([1, 1, n]);
			const h = this.heading;
			const w = size[0];
			this.setPosition([this.x + w * Math.sin(deg2rad(h + 90)), this.y - w * Math.cos(deg2rad(h + 90))]);
			this.heading = h;
			return;
		}
		if (isSequence(n) && typeof n !== 'string') {
			const [dx, dy] = seqItems(n);
			const h = this.heading;
			const ox = this.num(dy) * Math.sin(deg2rad(h)) + this.num(dx) * Math.sin(deg2rad(h + 90));
			const oy = this.num(dy) * Math.cos(deg2rad(h)) + this.num(dx) * Math.cos(deg2rad(h + 90));
			this.setPosition([this.x + ox, this.y - oy]);
			return;
		}
		n = this.num(n);
		const h = deg2rad(this.heading);
		this.moveTo(this.x + n * Math.sin(h), this.y - n * Math.cos(h));
	}

	backward(n) {
		if (isSequence(n) && typeof n !== 'string') {
			const [dx, dy] = seqItems(n);
			this.forward([-this.num(dx), -this.num(dy)]);
			this.right(180);
			return;
		}
		this.forward(-this.num(n));
	}

	angle(v) {
		if (v === 'any') return this.random() * 360;
		return this.num(v);
	}

	// Heading grows clockwise (0 north, 90 east), so LEFT decreases it.
	left(v) {
		this.heading = (this.heading - this.angle(v)) % 360;
		if (this.heading < 0) this.heading += 360;
	}

	right(v) {
		this.heading = (this.heading + this.angle(v)) % 360;
		if (this.heading < 0) this.heading += 360;
	}

	getHeading() {
		return this.heading;
	}

	headingTowards(x, y) {
		const dx = x - this.x, dy = y - this.y;
		if (dx === 0 && dy === 0) return this.heading;
		let h = Math.atan2(dx, -dy) * 180 / Math.PI;
		if (h < 0) h += 360;
		return h;
	}

	setHeading(v) {
		if (v === 'any') this.heading = this.random() * 360;
		else if (isSequence(v) && typeof v !== 'string') {
			const [x, y] = seqItems(v);
			this.heading = this.headingTowards(this.num(x), this.num(y));
		} else {
			this.heading = this.num(v) % 360;
			if (this.heading < 0) this.heading += 360;
		}
	}

	getPosition() {
		return [this.x, this.y];
	}

	// POSITION turns the turtle towards the target and moves there.
	setPosition(v) {
		let x, y;
		if (v === 'any') {
			x = this.random() * this.page[0];
			y = this.random() * this.page[1];
		} else if (isSequence(v) && typeof v !== 'string') {
			const p = seqItems(v);
			x = this.num(p[0]);
			y = this.num(p[1]);
		} else throw new LogoError('ERROR');
		this.heading = this.headingTowards(x, y);
		this.moveTo(x, y);
	}

	penup() {
		this.pen = false;
	}

	pendown() {
		this.pen = true;
	}

	// HOME during a program recenters the turtle and faces it north but keeps
	// the pen and fill state, as LibreLogo.py's home() does while running (it
	// only resets pen/fill colours when the program has been stopped).
	home() {
		this.x = this.page[0] / 2;
		this.y = this.page[1] / 2;
		this.heading = 0;
		this.current = null;
		this.points = [];
	}

	clearscreen() {
		this.shapes.length = 0;
		this.groupStack = [];
		this.current = null;
		this.actual = null;
		this.points = [];
	}

	hideturtle() {
		this.visible = false;
	}

	showturtle() {
		this.visible = true;
	}

	sleep(ms) {
		if (ms > 0) this.time += ms;
		this.current = null;
	}

	// --- colors ----------------------------------------------------------------

	// LibreLogo.py's __color__: a name, "~name", "any", a number, a hex or
	// decimal string, [index], [r, g, b], [r, g, b, transparency], or a
	// gradient [c1, c2, ...].
	color(c) {
		if (typeof c === 'number') return c;
		if (typeof c === 'boolean') return Number(c);
		if (typeof c === 'string') {
			if (c === 'any') return this.randomColor();
			if (c[0] === '~') {
				const base = this.namedColor(c.slice(1));
				const comps = colorComponents(base);
				for (let i = 0; i < 3; i++) {
					comps[i] = Math.max(0, Math.min(255, comps[i] + Math.floor(this.random() * 64) - 32));
				}
				return this.color(comps.slice(0, 3));
			}
			if (/^[0-9]/.test(c)) {
				const n = /^0[xX]/.test(c) ? parseInt(c, 16) : parseInt(c, 10);
				if (Number.isNaN(n)) throw new LogoError('ERR_KEY', [c]);
				return n;
			}
			return this.namedColor(c);
		}
		if (isSequence(c)) {
			const items = seqItems(c);
			if (items.length === 1) {
				const idx = Math.trunc(this.num(items[0]));
				if (idx < 0 || idx >= COLORS.length) throw new LogoError('ERR_INDEX');
				return COLORS[idx][1];
			}
			if (items.length === 3) {
				return ((Math.trunc(this.num(items[0])) % 256) << 16) + ((Math.trunc(this.num(items[1])) % 256) << 8) + (Math.trunc(this.num(items[2])) % 256);
			}
			if (items.length === 4) {
				return ((Math.trunc(this.num(items[3])) % 256) * 0x1000000) + ((Math.trunc(this.num(items[0])) % 256) << 16)
					+ ((Math.trunc(this.num(items[1])) % 256) << 8) + (Math.trunc(this.num(items[2])) % 256);
			}
			if (items.length === 2 || items.length > 4) {
				return [this.color(items[0]), this.color(items[1])].concat(items.slice(2).map((v) => this.num(v)));
			}
		}
		throw new LogoError('ERROR');
	}

	namedColor(name) {
		const rgb = this.locale.colors.get(String(name).toLowerCase());
		if (rgb === undefined) throw new LogoError('ERR_KEY', ["'" + name + "'"]);
		return rgb;
	}

	randomColor() {
		const rc = NORMCOLORS[Math.floor(this.random() * 7)];
		const rv = this.random();
		const rgray = Math.sqrt(this.random());
		const ratio = Math.abs(rc[2]) / (Math.abs(rc[2]) + Math.abs(rc[4]));
		const col = rc[0].slice();
		if (rv < ratio) col[rc[1]] += rc[2] * rv / ratio;
		else col[rc[3]] += rc[4] * (rv - ratio) / (1 - ratio);
		const rdark = 1 - Math.pow(2, 4) * Math.pow(this.random() - 0.5, 4);
		for (let i = 0; i < 3; i++) col[i] = 255 * (rgray + (col[i] / 255 - rgray) * rdark);
		return this.color(col.map((v) => Math.max(0, Math.min(255, Math.round(v)))));
	}

	transparencyOf(c) {
		return Array.isArray(c) ? 0 : ((c >>> 24) & 0xff) / 2.55;
	}

	withTransparency(c, percent) {
		if (Array.isArray(c)) return c;
		return (Math.trunc(percent * 2.55) * 0x1000000) + (c & 0xffffff);
	}

	// --- attribute setters and getters ------------------------------------------

	setPencolor(v) {
		this.pencolor = this.color(v);
		if (Array.isArray(this.pencolor)) this.pencolor = this.pencolor[0];
	}

	getPencolor() {
		return colorComponents(this.pencolor);
	}

	setPensize(v) {
		this.pensize = v === 'any' ? this.random() * 10 : this.num(v);
	}

	getPensize() {
		return this.pensize;
	}

	setPenstyle(v) {
		if (isSequence(v) && typeof v !== 'string' && seqItems(v).length >= 5) {
			this.linestyle = seqItems(v).map((x) => this.num(x));
		} else if (this.locale.matchesConstant(v, 'SOLID')) this.linestyle = 'solid';
		else if (this.locale.matchesConstant(v, 'DASH')) this.linestyle = 'dashed';
		else if (this.locale.matchesConstant(v, 'DOTTED')) this.linestyle = 'dotted';
		else throw new LogoError('ERROR');
	}

	getPenstyle() {
		if (Array.isArray(this.linestyle)) return this.linestyle.slice();
		return this.locale.localName({ solid: 'SOLID', dashed: 'DASH', dotted: 'DOTTED' }[this.linestyle]);
	}

	setPenjoint(v) {
		for (const [key, name] of [['NONE', 'none'], ['BEVEL', 'bevel'], ['MITER', 'miter'], ['ROUNDED', 'rounded']]) {
			if (this.locale.matchesConstant(v, key)) {
				this.linejoint = name;
				return;
			}
		}
		throw new LogoError('ERROR');
	}

	getPenjoint() {
		return this.locale.localName({ none: 'NONE', bevel: 'BEVEL', miter: 'MITER', rounded: 'ROUNDED' }[this.linejoint]);
	}

	setPencap(v) {
		for (const [key, name] of [['NONE', 'none'], ['ROUNDED', 'round'], ['SQUARE', 'square']]) {
			if (this.locale.matchesConstant(v, key)) {
				this.linecap = name;
				return;
			}
		}
		throw new LogoError('ERROR');
	}

	getPencap() {
		return this.locale.localName({ none: 'NONE', round: 'ROUNDED', square: 'SQUARE' }[this.linecap]);
	}

	setFillcolor(v) {
		this.areacolor = this.color(v);
		this.filltransparency = this.transparencyOf(this.areacolor);
	}

	getFillcolor() {
		return Array.isArray(this.areacolor) ? this.areacolor.slice() : colorComponents(this.areacolor);
	}

	setFilltransparency(v) {
		if (v === 'any') v = 100 * this.random();
		if (isSequence(v) && typeof v !== 'string') {
			// transparency gradient: use its first value
			const items = seqItems(v);
			v = items.length ? this.num(items[0]) : 0;
		}
		v = this.num(v);
		this.filltransparency = v;
		this.areacolor = this.withTransparency(this.areacolor, v);
	}

	getFilltransparency() {
		return this.filltransparency;
	}

	setPentransparency(v) {
		if (v === 'any') v = 100 * this.random();
		this.pencolor = this.withTransparency(this.pencolor, this.num(v));
	}

	getPentransparency() {
		return this.transparencyOf(this.pencolor);
	}

	setFillstyle(v) {
		if (isSequence(v) && typeof v !== 'string') {
			const s = seqItems(v);
			this.hatch = { style: Math.trunc(this.num(s[0])), color: this.color(s[1]) & 0xffffff, distance: this.num(s[2]), angle: this.num(s[3]) };
			return;
		}
		v = Math.trunc(this.num(v));
		if (v === 0) this.hatch = null;
		else if (v <= 10) this.setFillstyle(HATCH_PRESETS[v - 1]);
		else throw new LogoError('ERROR');
	}

	setFontcolor(v) {
		this.textcolor = this.color(v);
		if (Array.isArray(this.textcolor)) this.textcolor = this.textcolor[0];
	}

	getFontcolor() {
		return colorComponents(this.textcolor);
	}

	setFonttransparency(v) {
		if (v === 'any') v = 100 * this.random();
		this.textcolor = this.withTransparency(this.textcolor, this.num(v));
	}

	getFonttransparency() {
		return this.transparencyOf(this.textcolor);
	}

	setFontheight(v) {
		this.fontheight = this.num(v);
	}

	getFontheight() {
		return this.fontheight;
	}

	setFontweight(v) {
		if (typeof v === 'number') this.fontweight = v;
		else if (this.locale.matchesConstant(v, 'BOLD')) this.fontweight = 150;
		else if (this.locale.matchesConstant(v, 'NORMAL')) this.fontweight = 100;
		else this.fontweight = 100;
	}

	getFontweight() {
		return this.fontweight;
	}

	setFontstyle(v) {
		if (typeof v === 'number') this.fontstyle = v;
		else if (this.locale.matchesConstant(v, 'ITALIC')) this.fontstyle = 2;
		else this.fontstyle = 0;
	}

	getFontstyle() {
		return this.fontstyle;
	}

	setFontfamily(v) {
		if (typeof v !== 'string') throw new LogoError('ERROR');
		this.fontfamily = v;
	}

	getFontfamily() {
		return this.fontfamily;
	}

	font() {
		return {
			family: this.fontfamily,
			size: this.fontheight,
			weight: this.fontweight,
			style: this.fontstyle,
			color: this.textcolor,
		};
	}

	// --- drawing objects ---------------------------------------------------------

	point() {
		const size = Math.max(1, this.pensize);
		const last = this.actual;
		if (last && last.kind === 'dots' && last.color === this.pencolor && last.size === size) {
			last.points.push([this.x, this.y]);
		} else {
			this.addShape({ kind: 'dots', points: [[this.x, this.y]], size, color: this.pencolor });
		}
		this.current = null;
		this.points.push([this.x, this.y]);
	}

	boxSize(l) {
		if (isSequence(l) && typeof l !== 'string') return seqItems(l).map((v) => this.num(v));
		const n = this.num(l);
		return [n, n];
	}

	ellipse(l) {
		const s = this.boxSize(l);
		const shape = {
			kind: 'ellipse', cx: this.x, cy: this.y, rx: s[0] / 2, ry: s[1] / 2, rotate: this.heading,
			stroke: this.strokeStyle(), fill: this.fillStyle(),
		};
		if (s.length > 2) {
			shape.start = s[2];
			shape.end = s.length > 3 ? s[3] : s[2];
			shape.arcKind = s.length > 4 ? Math.trunc(s[4]) : 1; // 1 sector, 2 segment, 3 arc
		}
		this.addShape(shape);
		this.current = null;
	}

	rectangle(l) {
		const s = this.boxSize(l);
		this.addShape({
			kind: 'rect', cx: this.x, cy: this.y, w: s[0], h: s[1], rotate: this.heading,
			radius: s.length > 2 ? s[2] : 0, stroke: this.strokeStyle(), fill: this.fillStyle(),
		});
		this.current = null;
	}

	// Width and height of a text in the current font, in points.
	textSize(text) {
		const lines = String(text).split('\n');
		let width = 0;
		for (const line of lines) {
			let w;
			if (this.measure) w = this.measure(line, this.font());
			else w = line.length * this.fontheight * 0.52 * (this.fontweight > 100 ? 1.07 : 1);
			width = Math.max(width, w);
		}
		return [width, lines.length * this.fontheight * 1.17];
	}

	// LABEL text or LABEL [dx, dy, text]: dx, dy in -1..1 shift the text by
	// half its width and height relative to the turtle.
	label(st) {
		let dx = 0, dy = 0, text = st;
		if (Array.isArray(st) && st.length === 3) {
			dx = this.num(st[0]);
			dy = this.num(st[1]);
			text = String(st[2]);
		}
		text = String(text);
		const size = this.textSize(text);
		let x = this.x, y = this.y;
		if (dx !== 0 || dy !== 0) {
			const h = deg2rad(this.heading);
			const n = [dx * size[0] / 2, dy * size[1] / 2];
			const ox = n[1] * Math.sin(h) + n[0] * Math.sin(h + Math.PI / 2);
			const oy = n[1] * Math.cos(h) + n[0] * Math.cos(h + Math.PI / 2);
			x += ox;
			y -= oy;
		}
		const prev = this.actual;
		this.addShape({ kind: 'label', x, y, rotate: this.heading, text, font: this.font(), width: size[0], height: size[1] });
		// LABEL does not become the "actual" shape for TEXT/FILL
		this.actual = prev;
		this.current = null;
		return size;
	}

	// TEXT: put text on the last drawing object.
	text(st) {
		const shape = this.actual;
		if (!shape) return;
		shape.text = Array.isArray(st) ? String(st[2]) : String(st);
		shape.font = this.font();
	}

	// FILL closes and fills the current line shape or the collected POINTs;
	// CLOSE only closes it.
	fill(filled) {
		if (this.points.length > 1) {
			const pts = this.points.slice();
			this.points = [];
			const shape = this.addShape({ kind: 'polygon', points: pts, stroke: this.strokeStyle(), fill: this.fillStyle(), filled });
			shape.stroke.color = this.pencolor;
			this.current = null;
			return;
		}
		const shape = this.actual;
		if (!shape) return;
		if (shape.kind === 'polyline') {
			const pts = [];
			for (const sp of shape.subpaths) for (const p of sp) {
				if (!pts.length || pts[pts.length - 1][0] !== p[0] || pts[pts.length - 1][1] !== p[1]) pts.push(p);
			}
			const target = this.target();
			const idx = target.indexOf(shape);
			const poly = { kind: 'polygon', points: pts, stroke: this.strokeStyle(), fill: this.fillStyle(), filled, t: shape.t };
			if (shape.text) {
				poly.text = shape.text;
				poly.font = shape.font;
			}
			if (idx >= 0) target[idx] = poly;
			else target.push(poly);
			this.actual = poly;
			this.current = null;
		} else if (shape.kind === 'polygon') {
			shape.filled = filled;
			shape.fill = this.fillStyle();
			shape.stroke = this.strokeStyle();
		}
	}

	// --- PICTURE groups ---------------------------------------------------------

	pictureStart(name) {
		this.current = null;
		this.actual = null;
		const group = { kind: 'group', children: [], name: name || '' };
		this.groupStack.push(group);
	}

	// Returns the shape the picture produced (the group, or its single child),
	// so a named ".svg" picture can be exported, or null if it drew nothing.
	pictureEnd() {
		const group = this.groupStack.pop();
		this.current = null;
		if (!group) return null;
		if (group.children.length > 1) {
			this.addShape(group);
			return group;
		}
		if (group.children.length === 1) {
			group.children[0].t = this.time;
			this.target().push(group.children[0]);
			this.actual = group.children[0];
			return group.children[0];
		}
		return null;
	}
}

export { Tuple };
