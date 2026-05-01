// @ts-strict-ignore -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*-

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * JSX runtime: a tiny zero-dependency h() helper plus JSX type declarations.
 *
 * tsconfig.json sets jsxFactory: "h" and jsxFragmentFactory: "Fragment".
 * Any .tsx file under browser/src/ can use JSX, and TypeScript compiles it
 * to plain h(...) calls that produce real DOM nodes. Because this project
 * uses module: "none" (each file compiled as a script and concatenated),
 * h and Fragment defined here are visible globally in the bundle.
 *
 * Conventions:
 *   - HTML-native attribute names: class, for, tabindex (not className/htmlFor).
 *   - Children may be Nodes, strings/numbers (text), arrays (flattened), or
 *     false/null/undefined (skipped, handy for conditionals).
 *   - Event handlers use the on<Event> prefix and bind via addEventListener.
 *   - "ref" accepts a callback that receives the freshly created element.
 */

type JSXChild =
	| Node
	| string
	| number
	| boolean
	| null
	| undefined
	| JSXChild[];

type JSXProps = Record<string, any> | null;

const JSX_SVG_NS = 'http://www.w3.org/2000/svg';
const JSX_SVG_TAGS = new Set([
	'svg',
	'path',
	'g',
	'circle',
	'rect',
	'line',
	'polyline',
	'polygon',
	'ellipse',
	'text',
	'tspan',
	'defs',
	'use',
	'symbol',
	'mask',
	'clipPath',
	'linearGradient',
	'radialGradient',
	'stop',
	'pattern',
	'filter',
	'foreignObject',
]);

function jsxAppendChildren(parent: Node, children: JSXChild[]): void {
	for (const c of children) {
		if (c == null || c === false || c === true) continue;
		if (Array.isArray(c)) jsxAppendChildren(parent, c);
		else if (c instanceof Node) parent.appendChild(c);
		else parent.appendChild(document.createTextNode(String(c)));
	}
}

function jsxApplyProps(el: HTMLElement, props: JSXProps): void {
	if (!props) return;
	for (const key in props) {
		const value = (props as any)[key];
		if (value == null || value === false) continue;
		if (key === 'children') continue;
		if (key === 'class' || key === 'className') {
			el.setAttribute('class', String(value));
		} else if (key === 'style' && typeof value === 'object') {
			Object.assign(el.style, value);
		} else if (key === 'ref' && typeof value === 'function') {
			value(el);
		} else if (key === 'dangerouslySetInnerHTML') {
			el.innerHTML = value.__html;
		} else if (
			key.length > 2 &&
			key[0] === 'o' &&
			key[1] === 'n' &&
			typeof value === 'function'
		) {
			el.addEventListener(key.slice(2).toLowerCase(), value);
		} else if (value === true) {
			el.setAttribute(key, '');
		} else {
			el.setAttribute(key, String(value));
		}
	}
}

function h(
	tag: string | ((props: any) => Node),
	props: JSXProps,
	...children: JSXChild[]
): HTMLElement {
	if (typeof tag === 'function') {
		return tag({ ...(props || {}), children }) as HTMLElement;
	}

	const el = JSX_SVG_TAGS.has(tag)
		? (document.createElementNS(JSX_SVG_NS, tag) as unknown as HTMLElement)
		: document.createElement(tag);

	jsxApplyProps(el, props);
	jsxAppendChildren(el, children);
	return el;
}

function Fragment(props: { children: JSXChild[] }): HTMLElement {
	const frag = document.createDocumentFragment();
	jsxAppendChildren(frag, props.children || []);
	return frag as unknown as HTMLElement;
}

/*
 * Ambient JSX namespace. Permissive intrinsic element types (any string
 * attribute, any on* handler) avoid per-tag maintenance while still giving
 * us JSX syntax and child typing. We can narrow this later if desired.
 */
namespace JSX {
	export type Element = HTMLElement;

	export interface ElementChildrenAttribute {
		// The TypeScript JSX spec requires this exact shape ({} as the property
		// type) to mark which prop holds children. Narrower types break JSX
		// child resolution, so the ban-types rule does not apply here.
		// eslint-disable-next-line @typescript-eslint/ban-types
		children: {};
	}

	export interface IntrinsicAttributes {
		ref?: (el: HTMLElement) => void;
		key?: string | number;
	}

	export type GenericAttributes = {
		[attr: string]: any;
		class?: string;
		id?: string;
		style?: Partial<CSSStyleDeclaration> | string;
		children?: JSXChild | JSXChild[];
		ref?: (el: HTMLElement) => void;
		dangerouslySetInnerHTML?: { __html: string };
	};

	export interface IntrinsicElements {
		[elemName: string]: GenericAttributes;
	}
}
