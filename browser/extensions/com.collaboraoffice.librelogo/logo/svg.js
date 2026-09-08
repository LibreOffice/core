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

// Renders the turtle's primitives (see turtle.js) as SVG. Coordinates are
// points; the SVG uses a viewBox in points so the drawing keeps its size when
// inserted into the document.

const MM_TO_PT = 72 / 25.4;
const INVISIBLE_ALPHA = 0xff;

// Namespace for the LibreLogo metadata embedded in exported pictures.
export const LIBRELOGO_NS = 'https://www.collaboraoffice.com/ns/librelogo';

// How long the finished picture is held before an animated export loops.
const ANIM_HOLD_MS = 2000;

function esc(s) {
	return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

function fmt(n) {
	return Number.isInteger(n) ? String(n) : n.toFixed(3).replace(/\.?0+$/, '');
}

function rgb(c) {
	return '#' + (c & 0xffffff).toString(16).padStart(6, '0');
}

// 0..1 opacity from LibreLogo's transparency byte.
function opacity(c) {
	const t = (c >>> 24) & 0xff;
	return t >= INVISIBLE_ALPHA ? 0 : 1 - t / 255;
}

// Renders the turtle's shapes (and optionally the turtle) to an SVG string.
export class SvgRenderer {
	constructor(options) {
		this.defs = [];
		this.nextId = 0;
		this.locale = options && options.locale;
	}

	id(prefix) {
		return prefix + (this.nextId++);
	}

	strokeAttrs(stroke) {
		if (!stroke) return ' stroke="none"';
		const color = stroke.color;
		const op = Array.isArray(color) ? 1 : opacity(color);
		if (op === 0) return ' stroke="none"';
		let a = ' stroke="' + rgb(Array.isArray(color) ? color[0] : color) + '"';
		if (op < 1) a += ' stroke-opacity="' + fmt(op) + '"';
		const w = Math.max(stroke.width, 0.01);
		a += ' stroke-width="' + fmt(w) + '"';
		const joint = { none: 'miter', bevel: 'bevel', miter: 'miter', rounded: 'round' }[stroke.joint] || 'round';
		a += ' stroke-linejoin="' + joint + '"';
		let cap = { none: 'butt', round: 'round', square: 'square' }[stroke.cap] || 'butt';
		const style = stroke.style;
		if (style === 'dashed') {
			const d = fmt(1 * MM_TO_PT);
			a += ' stroke-dasharray="' + d + ' ' + d + '"';
		} else if (style === 'dotted') {
			// dots as round zero-length dashes, the size of the pen
			const size = Math.max(1, stroke.width);
			a += ' stroke-width="' + fmt(size) + '" stroke-dasharray="0.01 ' + fmt(size * 2) + '"';
			cap = 'round';
		} else if (Array.isArray(style)) {
			// [dots, dotlen, dashes, dashlen, distance, type]; type 2 = relative to the pen size
			const rel = style[5] === 2 ? Math.max(stroke.width, 0.5) : 1;
			const dots = Math.max(0, Math.trunc(style[0])), dashes = Math.max(0, Math.trunc(style[2]));
			const parts = [];
			for (let i = 0; i < dots; i++) parts.push(fmt(Math.max(0.01, style[1] * rel)), fmt(style[4] * rel));
			for (let i = 0; i < dashes; i++) parts.push(fmt(Math.max(0.01, style[3] * rel)), fmt(style[4] * rel));
			if (parts.length) a += ' stroke-dasharray="' + parts.join(' ') + '"';
		}
		a += ' stroke-linecap="' + cap + '"';
		return a;
	}

	fillAttrs(fill, filled, rotate) {
		if (!fill || filled === false) return ' fill="none"';
		let a = '';
		const color = fill.color;
		if (fill.hatch) {
			a += ' fill="url(#' + this.hatchPattern(fill.hatch) + ')"';
			return a;
		}
		if (Array.isArray(color)) {
			a += ' fill="url(#' + this.gradient(color, rotate || 0) + ')"';
			return a;
		}
		const op = opacity(color);
		if (op === 0) return ' fill="none"';
		a += ' fill="' + rgb(color) + '"';
		if (op < 1) a += ' fill-opacity="' + fmt(op) + '"';
		return a;
	}

	// [color1, color2, style, angle, border, x, y, intensity1, intensity2]
	gradient(g, heading) {
		const id = this.id('g');
		const style = Math.trunc(g[2] || 0);
		const angle = ((g[3] || 0) - heading) % 360;
		const border = (g[4] || 0) / 100;
		const c1 = rgb(g[0]), c2 = rgb(g[1]);
		const o1 = opacity(g[0]), o2 = opacity(g[1]);
		const stop = (off, c, o) => '<stop offset="' + fmt(off) + '" stop-color="' + c + '"' + (o < 1 ? ' stop-opacity="' + fmt(o) + '"' : '') + '/>';
		if (style === 2 || style === 3 || style === 4 || style === 5) {
			// radial-like gradients: center from x/y percentages
			const cx = g[5] !== undefined ? g[5] / 100 : 0.5, cy = g[6] !== undefined ? g[6] / 100 : 0.5;
			this.defs.push('<radialGradient id="' + id + '" cx="' + fmt(cx) + '" cy="' + fmt(cy) + '" r="0.7">'
				+ stop(0, c2, o2) + stop(Math.max(0, 1 - border), c1, o1) + '</radialGradient>');
			return id;
		}
		// linear (0) and axial (1): LibreOffice's angle is counterclockwise
		// with 0 meaning color1 at the top; rotate the gradient vector.
		const rad = -angle * Math.PI / 180;
		const dx = -Math.sin(rad) / 2, dy = Math.cos(rad) / 2;
		const x1 = 0.5 - dx, y1 = 0.5 - dy, x2 = 0.5 + dx, y2 = 0.5 + dy;
		let stops;
		if (style === 1) stops = stop(0, c2, o2) + stop(0.5, c1, o1) + stop(1, c2, o2);
		else stops = stop(border, c1, o1) + stop(1, c2, o2);
		this.defs.push('<linearGradient id="' + id + '" x1="' + fmt(x1) + '" y1="' + fmt(y1) + '" x2="' + fmt(x2) + '" y2="' + fmt(y2) + '">' + stops + '</linearGradient>');
		return id;
	}

	// { style: 1|2|3, color, distance (pt), angle (deg) }
	hatchPattern(h) {
		const id = this.id('h');
		const d = Math.max(1, h.distance);
		const color = rgb(h.color);
		let lines = '<line x1="0" y1="0" x2="' + fmt(d) + '" y2="0" stroke="' + color + '" stroke-width="0.5"/>';
		if (h.style >= 2) lines += '<line x1="0" y1="0" x2="0" y2="' + fmt(d) + '" stroke="' + color + '" stroke-width="0.5"/>';
		if (h.style >= 3) lines += '<line x1="0" y1="' + fmt(d) + '" x2="' + fmt(d) + '" y2="0" stroke="' + color + '" stroke-width="0.5"/>';
		this.defs.push('<pattern id="' + id + '" patternUnits="userSpaceOnUse" width="' + fmt(d) + '" height="' + fmt(d)
			+ '" patternTransform="rotate(' + fmt(-h.angle) + ')">' + lines + '</pattern>');
		return id;
	}

	fontAttrs(font) {
		let a = ' font-family="' + esc(font.family) + '" font-size="' + fmt(font.size) + '"';
		if (font.weight > 100) a += ' font-weight="bold"';
		if (font.style === 2) a += ' font-style="italic"';
		const op = opacity(font.color);
		a += ' fill="' + rgb(font.color) + '"';
		if (op < 1) a += ' fill-opacity="' + fmt(op) + '"';
		return a;
	}

	textElement(x, y, rotate, text, font) {
		const lines = String(text).split('\n');
		const lh = font.size * 1.17;
		let out = '<text x="' + fmt(x) + '" y="' + fmt(y) + '" text-anchor="middle" dominant-baseline="central"' + this.fontAttrs(font);
		if (rotate) out += ' transform="rotate(' + fmt(rotate) + ' ' + fmt(x) + ' ' + fmt(y) + ')"';
		out += '>';
		if (lines.length === 1) out += esc(text);
		else {
			const y0 = y - (lines.length - 1) * lh / 2;
			lines.forEach((line, i) => {
				out += '<tspan x="' + fmt(x) + '" y="' + fmt(y0 + i * lh) + '">' + esc(line) + '</tspan>';
			});
		}
		return out + '</text>';
	}

	// Ellipse or its sector/segment/arc, rotated by `rotate` around its center.
	ellipsePath(s) {
		const tr = s.rotate ? ' transform="rotate(' + fmt(s.rotate) + ' ' + fmt(s.cx) + ' ' + fmt(s.cy) + ')"' : '';
		if (s.start === undefined) {
			return '<ellipse cx="' + fmt(s.cx) + '" cy="' + fmt(s.cy) + '" rx="' + fmt(s.rx) + '" ry="' + fmt(s.ry) + '"' + tr;
		}
		// Angles are clock positions in degrees clockwise from the top.
		const pt = (deg) => {
			const r = deg * Math.PI / 180;
			return [s.cx + s.rx * Math.sin(r), s.cy - s.ry * Math.cos(r)];
		};
		let sweep = (s.end - s.start) % 360;
		if (sweep < 0) sweep += 360;
		if (sweep === 0) sweep = 360;
		const p1 = pt(s.start), p2 = pt(s.start + sweep);
		const large = sweep > 180 ? 1 : 0;
		let d = 'M' + fmt(p1[0]) + ' ' + fmt(p1[1]) + ' A' + fmt(s.rx) + ' ' + fmt(s.ry) + ' 0 ' + large + ' 1 ' + fmt(p2[0]) + ' ' + fmt(p2[1]);
		if (s.arcKind === 1) d += ' L' + fmt(s.cx) + ' ' + fmt(s.cy) + ' Z';
		else if (s.arcKind === 2) d += ' Z';
		return '<path d="' + d + '"' + tr;
	}

	rectPath(s) {
		const tr = s.rotate ? ' transform="rotate(' + fmt(s.rotate) + ' ' + fmt(s.cx) + ' ' + fmt(s.cy) + ')"' : '';
		let out = '<rect x="' + fmt(s.cx - s.w / 2) + '" y="' + fmt(s.cy - s.h / 2) + '" width="' + fmt(s.w) + '" height="' + fmt(s.h) + '"';
		if (s.radius) out += ' rx="' + fmt(Math.min(s.radius, s.w / 2, s.h / 2)) + '"';
		return out + tr;
	}

	shape(s) {
		switch (s.kind) {
		case 'polyline': {
			let d = '';
			for (const sp of s.subpaths) {
				if (sp.length === 0) continue;
				d += 'M' + fmt(sp[0][0]) + ' ' + fmt(sp[0][1]);
				for (let i = 1; i < sp.length; i++) d += ' L' + fmt(sp[i][0]) + ' ' + fmt(sp[i][1]);
			}
			let out = '<path d="' + d + '" fill="none"' + this.strokeAttrs(s.stroke) + '/>';
			if (s.text) out += this.shapeText(s);
			return out;
		}
		case 'polygon': {
			if (s.points.length === 0) return '';
			let d = 'M' + fmt(s.points[0][0]) + ' ' + fmt(s.points[0][1]);
			for (let i = 1; i < s.points.length; i++) d += ' L' + fmt(s.points[i][0]) + ' ' + fmt(s.points[i][1]);
			d += ' Z';
			let out = '<path d="' + d + '"' + this.fillAttrs(s.fill, s.filled, 0) + this.strokeAttrs(s.stroke) + '/>';
			if (s.text) out += this.shapeText(s);
			return out;
		}
		case 'ellipse': {
			let out = this.ellipsePath(s) + this.fillAttrs(s.fill, true, s.rotate) + this.strokeAttrs(s.stroke) + '/>';
			if (s.text) out += this.shapeText(s);
			return out;
		}
		case 'rect': {
			let out = this.rectPath(s) + this.fillAttrs(s.fill, true, s.rotate) + this.strokeAttrs(s.stroke) + '/>';
			if (s.text) out += this.shapeText(s);
			return out;
		}
		case 'dots': {
			const op = opacity(s.color);
			if (op === 0) return '';
			let out = '<g fill="' + rgb(s.color) + '"' + (op < 1 ? ' fill-opacity="' + fmt(op) + '"' : '') + '>';
			for (const p of s.points) out += '<circle cx="' + fmt(p[0]) + '" cy="' + fmt(p[1]) + '" r="' + fmt(s.size / 2) + '"/>';
			return out + '</g>';
		}
		case 'label':
			return this.textElement(s.x, s.y, s.rotate, s.text, s.font);
		case 'group': {
			let out = '<g>';
			for (const c of s.children) out += this.shape(c);
			return out + '</g>';
		}
		default:
			return '';
		}
	}

	// Wrap a shape so it appears at its SLEEP time and stays until the loop
	// restarts. One self-contained animation per shape: a discrete opacity step
	// from 0 to 1 at the shape's fraction of the timeline, repeating forever.
	animatedShape(s, total) {
		const inner = this.shape(s);
		const t = s.t || 0;
		if (t <= 0) return inner; // drawn before the first SLEEP: always visible
		const f = Math.min(0.999, t / total);
		return '<g opacity="0"><animate attributeName="opacity" dur="' + fmt(total) + 'ms"'
			+ ' repeatCount="indefinite" calcMode="discrete" values="0;1" keyTimes="0;' + fmt(f) + '"/>'
			+ inner + '</g>';
	}

	// TEXT on a shape: centered on the shape's bounding box.
	shapeText(s) {
		const b = bbox(s);
		if (!b) return '';
		return this.textElement((b[0] + b[2]) / 2, (b[1] + b[3]) / 2, s.rotate || 0, s.text, s.font);
	}

	// The turtle: a small arrow-shaped turtle, as in LibreLogo.
	turtle(t) {
		if (!t.visible) return '';
		const size = 10;
		const pts = [[0, -size], [size * 0.6, size * 0.7], [0, size * 0.3], [-size * 0.6, size * 0.7]];
		const d = pts.map((p, i) => (i ? 'L' : 'M') + fmt(p[0]) + ' ' + fmt(p[1])).join(' ') + ' Z';
		const fill = Array.isArray(t.fillcolor) ? t.fillcolor[0] : t.fillcolor;
		const fop = Math.min(0.95, opacity(fill));
		return '<path d="' + d + '" transform="translate(' + fmt(t.x) + ' ' + fmt(t.y) + ') rotate(' + fmt(t.heading) + ')"'
			+ ' fill="' + rgb(fill) + '" fill-opacity="' + fmt(fop) + '" stroke="' + rgb(t.pencolor) + '" stroke-opacity="0.75"'
			+ ' stroke-width="' + fmt(Math.min(Math.max(t.pensize, 0.5), 3)) + '" stroke-linejoin="miter"/>';
	}

	// Render the whole drawing. `view` is [x, y, w, h] in points or null for the page.
	// view is [x, y, w, h] in points. pageOutline, when given as [w, h], draws a
	// light page rectangle behind the shapes so the preview shows where the
	// page is even when the drawing spills outside it.
	render(shapes, turtleState, view, pageOutline, source, animate) {
		this.defs = [];
		let body = '';
		if (pageOutline) {
			body += '<rect x="0" y="0" width="' + fmt(pageOutline[0]) + '" height="' + fmt(pageOutline[1])
				+ '" fill="#ffffff" stroke="#c0c0c0" stroke-width="0.75"/>';
		}
		// With SLEEP the shapes carry appearance times; an animated export reveals
		// them on that timeline and loops. A picture is a single group, so animate
		// its children (the shapes the program drew) rather than the group itself.
		let units = shapes;
		if (animate && shapes.length === 1 && shapes[0].kind === 'group') units = shapes[0].children;
		let total = 0;
		if (animate) for (const s of units) total = Math.max(total, s.t || 0);
		if (animate && total > 0) {
			total += ANIM_HOLD_MS;
			for (const s of units) body += this.animatedShape(s, total);
		} else {
			for (const s of shapes) body += this.shape(s);
		}
		if (turtleState) body += this.turtle(turtleState);
		const [x, y, w, h] = view;
		let svg = '<svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="' + fmt(w) + 'pt" height="' + fmt(h) + 'pt" viewBox="'
			+ fmt(x) + ' ' + fmt(y) + ' ' + fmt(w) + ' ' + fmt(h) + '">';
		// Machine-readable metadata (not <desc>, which is for accessible
		// descriptions): mark the file as a LibreLogo drawing and embed its
		// source, so the program that drew the picture can be recovered.
		if (source) {
			svg += '<metadata>'
				+ '<librelogo:program xmlns:librelogo="' + LIBRELOGO_NS + '">'
				+ '<librelogo:source>' + esc(source) + '</librelogo:source>'
				+ '</librelogo:program></metadata>';
		}
		if (this.defs.length) svg += '<defs>' + this.defs.join('') + '</defs>';
		return svg + body + '</svg>';
	}
}

// Bounding box [minX, minY, maxX, maxY] of one shape, including its stroke.
export function bbox(s) {
	let box = null;
	const add = (x, y) => {
		if (!box) box = [x, y, x, y];
		else {
			box[0] = Math.min(box[0], x);
			box[1] = Math.min(box[1], y);
			box[2] = Math.max(box[2], x);
			box[3] = Math.max(box[3], y);
		}
	};
	const rot = (cx, cy, x, y, deg) => {
		const r = deg * Math.PI / 180;
		const dx = x - cx, dy = y - cy;
		add(cx + dx * Math.cos(r) - dy * Math.sin(r), cy + dx * Math.sin(r) + dy * Math.cos(r));
	};
	switch (s.kind) {
	case 'polyline': for (const sp of s.subpaths) for (const p of sp) add(p[0], p[1]); break;
	case 'polygon': for (const p of s.points) add(p[0], p[1]); break;
	case 'dots': for (const p of s.points) { add(p[0] - s.size / 2, p[1] - s.size / 2); add(p[0] + s.size / 2, p[1] + s.size / 2); } break;
	case 'ellipse':
		for (const [sx, sy] of [[-1, -1], [1, -1], [1, 1], [-1, 1]]) rot(s.cx, s.cy, s.cx + sx * s.rx, s.cy + sy * s.ry, s.rotate || 0);
		break;
	case 'rect':
		for (const [sx, sy] of [[-1, -1], [1, -1], [1, 1], [-1, 1]]) rot(s.cx, s.cy, s.cx + sx * s.w / 2, s.cy + sy * s.h / 2, s.rotate || 0);
		break;
	case 'label': {
		const w = s.width || 0, h = s.height || s.font.size;
		for (const [sx, sy] of [[-1, -1], [1, -1], [1, 1], [-1, 1]]) rot(s.x, s.y, s.x + sx * w / 2, s.y + sy * h / 2, s.rotate || 0);
		break;
	}
	case 'group':
		for (const c of s.children) {
			const b = bbox(c);
			if (b) { add(b[0], b[1]); add(b[2], b[3]); }
		}
		break;
	default: break;
	}
	if (box && s.stroke) {
		const w = (s.stroke.width || 0) / 2 + 0.5;
		box = [box[0] - w, box[1] - w, box[2] + w, box[3] + w];
	}
	return box;
}

export function drawingBounds(shapes, turtleState) {
	let box = null;
	const merge = (b) => {
		if (!b) return;
		if (!box) box = b.slice();
		else {
			box[0] = Math.min(box[0], b[0]);
			box[1] = Math.min(box[1], b[1]);
			box[2] = Math.max(box[2], b[2]);
			box[3] = Math.max(box[3], b[3]);
		}
	};
	for (const s of shapes) merge(bbox(s));
	if (turtleState && turtleState.visible) merge([turtleState.x - 8, turtleState.y - 11, turtleState.x + 8, turtleState.y + 9]);
	return box;
}
