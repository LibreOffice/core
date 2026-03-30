// @ts-strict-ignore
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

declare var brandProductFAQURL: string | undefined;
declare var unoShortcutsModifierL10N: any;

interface IDAble {
	_leaflet_id: number;
	[name: string]: any;
}

type CallbackFunctionVariadic = (...args: any[]) => void;

class Util {
	private static lastId = 0;
	private static nextId(): number {
		return ++Util.lastId;
	}

	/// Returns the id of the object. Initializes it if necessary.
	public static stamp(obj: IDAble): number {
		if (obj._leaflet_id > 0) {
			return obj._leaflet_id;
		}
		obj._leaflet_id = Util.nextId();
		return obj._leaflet_id;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public static merge(one: any, two: any): any {
		return { ...one, ...two };
	}

	// return a function that won't be called more often than the given interval
	public static throttle(
		fn: CallbackFunctionVariadic,
		time: number,
		// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
		context: any,
	): CallbackFunctionVariadic {
		let lock: boolean = false;
		// eslint-disable-next-line prefer-const
		let wrapperFn: CallbackFunctionVariadic;
		let args: any[] | boolean = false;

		const later = function () {
			// reset lock and call if queued.
			lock = false;
			if (args) {
				wrapperFn.apply(context, args);
				args = false;
			}
		};

		wrapperFn = function (...args_: any[]) {
			if (lock) {
				// called too soon, queue to call later
				args = args_;
			} else {
				// call and lock until later
				fn.apply(context, args_);
				setTimeout(later, time);
				lock = true;
			}
		};

		return wrapperFn;
	}

	// wrap the given number to lie within a certain range (used for wrapping longitude)
	public static wrapNum(
		x: number,
		range: Array<number>,
		includeMax: boolean,
	): number {
		const max = range[1];
		const min = range[0];
		const d = max - min;
		return x === max && includeMax ? x : ((((x - min) % d) + d) % d) + min;
	}

	// do nothing (used as a noop throughout the code)
	public static falseFn(): boolean {
		return false;
	}

	// round a given number to a given precision
	public static formatNum(num: number, digits: number): number {
		var pow = Math.pow(10, digits || 5);
		return Math.round(num * pow) / pow;
	}

	// removes prefix from string if string starts with that prefix
	public static trimStart(str: string, prefix: string): string {
		if (str.indexOf(prefix) === 0) return str.substring(prefix.length);
		return str;
	}

	// removes suffix from string if string ends with that suffix
	public static trimEnd(str: string, suffix: string): string {
		var suffixIndex = str.lastIndexOf(suffix);
		if (suffixIndex !== -1 && str.length - suffix.length === suffixIndex)
			return str.substring(0, suffixIndex);
		return str;
	}

	// removes given prefix and suffix from the string if exists
	// if suffix is not specified, prefix is trimmed from both end of string
	// trim whitespace from both sides of a string if prefix and suffix are not given
	public static trim(str: string, prefix?: string, suffix?: string): string {
		if (!prefix) return str.trim ? str.trim() : str.replace(/^\s+|\s+$/g, '');
		let result = Util.trimStart(str, prefix);
		result = Util.trimEnd(result, suffix);
		return result;
	}

	// split a string into words
	public static splitWords(str: string): string[] {
		return Util.trim(str).split(/\s+/);
	}

	public static round(x: number, e?: number): number {
		if (!e) {
			return Math.round(x);
		}
		var f = 1.0 / e;
		return Math.round(x * f) * e;
	}

	public static templateRe = /\{ *([\w_]+) *\}/g;

	// super-simple templating facility, used for TileLayer URLs
	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public static template(str: string, data: any): string {
		return str.replace(Util.templateRe, function (str, key) {
			let value: any = data[key];

			if (value === undefined) {
				throw new Error('No value provided for variable ' + str);
			} else if (typeof value === 'function') {
				value = value(data);
			}
			return value;
		});
	}

	public static isArray =
		Array.isArray ||
		function (obj) {
			return Object.prototype.toString.call(obj) === '[object Array]';
		};

	// minimal image URI, set to an image when disposing to flush memory
	public static emptyImageUrl =
		'data:image/gif;base64,R0lGODlhAQABAAD/ACwAAAAAAQABAAACADs=';

	public static toggleFullScreen(): void {
		const doc = document as any;
		if (
			!doc.fullscreenElement &&
			!doc.mozFullscreenElement &&
			!doc.msFullscreenElement &&
			!doc.webkitFullscreenElement
		) {
			if (doc.documentElement.requestFullscreen) {
				doc.documentElement.requestFullscreen();
			} else if (doc.documentElement.msRequestFullscreen) {
				doc.documentElement.msRequestFullscreen();
			} else if (doc.documentElement.mozRequestFullScreen) {
				doc.documentElement.mozRequestFullScreen();
			} else if (doc.documentElement.webkitRequestFullscreen) {
				doc.documentElement.webkitRequestFullscreen(
					(Element as any).ALLOW_KEYBOARD_INPUT,
				);
			}
		} else if (doc.exitFullscreen) {
			doc.exitFullscreen();
		} else if (doc.msExitFullscreen) {
			doc.msExitFullscreen();
		} else if (doc.mozCancelFullScreen) {
			doc.mozCancelFullScreen();
		} else if (doc.webkitExitFullscreen) {
			doc.webkitExitFullscreen();
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public static isEmpty(o: any): boolean {
		return !(o && o.length);
	}

	public static mm100thToInch(mm: number): number {
		return mm / 2540;
	}

	private static _canvas: HTMLCanvasElement | null = null;

	private static getTempCanvas(): HTMLCanvasElement {
		if (Util._canvas) {
			return Util._canvas;
		}
		Util._canvas = document.createElement('canvas');
		return Util._canvas;
	}

	public static getTextWidth(text: string, font: string): number {
		const canvas = Util.getTempCanvas();
		const context = canvas.getContext('2d');
		context.font = font;
		const metrics = context.measureText(text);
		return Math.floor(metrics.width);
	}

	public static getProduct(): string {
		let brandFAQURL =
			typeof brandProductFAQURL !== 'undefined'
				? brandProductFAQURL
				: 'https://collaboraonline.github.io/post/faq/';
		const customWindow = window as any;
		if (customWindow.feedbackUrl && customWindow.buyProductUrl) {
			const integratorUrl = encodeURIComponent(customWindow.buyProductUrl);
			brandFAQURL = customWindow.feedbackUrl;
			brandFAQURL =
				brandFAQURL.substring(0, brandFAQURL.lastIndexOf('/')) +
				'/product.html?integrator=' +
				integratorUrl;
		}
		return brandFAQURL;
	}

	public static replaceCtrlAltInMac(msg: string): string {
		if (!window.L.Browser.mac) return msg;

		// Find the localized modifier names for the current language
		// from the generated unoShortcutsModifierL10N table.
		let ctrlName = 'Ctrl';
		let altName = 'Alt';
		if (typeof unoShortcutsModifierL10N !== 'undefined') {
			for (const [lang, replacements] of Object.entries(
				unoShortcutsModifierL10N,
			)) {
				if ((String as any).locale.startsWith(lang)) {
					const r = replacements as Record<string, string>;
					if (r['Ctrl']) ctrlName = r['Ctrl'];
					if (r['Alt']) altName = r['Alt'];
					break;
				}
			}
		}

		return msg
			.replace(new RegExp(ctrlName, 'gi'), '\u2318')
			.replace(new RegExp(altName, 'gi'), '\u2325');
	}

	public static randomString(len: number): string {
		let result = '';
		const ValidCharacters =
			'0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
		for (let i = 0; i < len; i++) {
			result += ValidCharacters.charAt(
				Math.floor(Math.random() * ValidCharacters.length),
			);
		}
		return result;
	}

	public static requestAnimFrame(
		fn: () => void,
		// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
		context: any,
	): number {
		return window.requestAnimationFrame(fn.bind(context));
	}

	public static cancelAnimFrame(id: number): void {
		if (id) {
			window.cancelAnimationFrame(id);
		}
	}

	public static ensureValue<T>(obj: T | undefined | null): asserts obj is T {
		if (obj === undefined) throw new Error('Object is undefined!');
		if (obj === null) throw new Error('Object is null!');
	}

	public static MAX_SAFE_INTEGER = Math.pow(2, 53) - 1;
	public static MIN_SAFE_INTEGER = -Util.MAX_SAFE_INTEGER;
}

app.util = Util;
