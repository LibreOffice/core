/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('DomUtil', function () {
	const docstr = `
	<html>
	<head>
	<style>
	#parent {
		font-family: "Roboto", sans-serif;
	}
	#one {
		background-color: red;
		font-family: "Libre Baskerville", serif;
		color: auto;
	}
	#two {
		font-family: auto;
	}
	</style>
	</head>
	<body>
		<div id="parent">
			<div id="one"></div>
			<div id="two"></div>
		</div>
	</body>
	</html>
	`;

	class DOMStore {
		public dom: any;

		constructor(docString: string) {
			this.dom = new jsdom.JSDOM(docString);
		}

		get window() {
			return this.dom.window;
		}

		get document() {
			return this.dom.window.document;
		}
	}

	describe('get()', function () {
		const store = new DOMStore(docstr);

		it('element present', function () {
			nodeassert.ok(
				DomUtil.get('one', store.document) instanceof
					store.window.HTMLDivElement,
			);
		});

		it('element absent', function () {
			nodeassert.strictEqual(null, DomUtil.get('ten', store.document));
		});

		it('null id', function () {
			nodeassert.strictEqual(null, DomUtil.get(null, store.document));
		});
	});

	describe('getStyle()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);
		const two = DomUtil.get('two', store.document);

		it('style auto', function () {
			nodeassert.strictEqual(
				'',
				DomUtil.getStyle(one, 'color', store.document),
			);
		});

		it('style non-auto', function () {
			nodeassert.strictEqual(
				'red',
				DomUtil.getStyle(one, 'background-color', store.document),
			);
		});

		it('style non-auto multi word value', function () {
			nodeassert.strictEqual(
				'"Libre Baskerville", serif',
				DomUtil.getStyle(one, 'font-family', store.document),
			);
		});

		// NOTE: jsdom does not implement cascading of styles?
		it('style inherited', function () {
			nodeassert.strictEqual(
				null,
				DomUtil.getStyle(two, 'font-family', store.document),
			);
		});
	});

	describe('setStyle()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);
		const two = DomUtil.get('two', store.document);

		it('auto to green', function () {
			DomUtil.setStyle(one, 'color', 'green');
			nodeassert.strictEqual(
				'green',
				DomUtil.getStyle(one, 'color', store.document),
			);
		});

		it('non-auto to purple', function () {
			DomUtil.setStyle(one, 'background-color', 'purple');
			nodeassert.strictEqual(
				'purple',
				DomUtil.getStyle(one, 'background-color', store.document),
			);
		});

		it('unset to green', function () {
			DomUtil.setStyle(two, 'color', 'green');
			nodeassert.strictEqual(
				'green',
				DomUtil.getStyle(two, 'color', store.document),
			);
		});
	});

	describe('create()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('parentless', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption',
				undefined,
				undefined,
				store.document,
			);
			nodeassert.strictEqual('cool-caption', el.className);
			nodeassert.strictEqual(null, el.parentElement);
		});

		it('with parent', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			nodeassert.strictEqual('cool-caption', el.className);
			nodeassert.strictEqual(one, el.parentElement);
			nodeassert.strictEqual(1, one.childElementCount);
		});
	});

	describe('createWithId()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('parentless', function () {
			const el = DomUtil.createWithId(
				'p',
				'cool-caption',
				undefined,
				undefined,
				store.document,
			);
			nodeassert.strictEqual('cool-caption', el.id);
			nodeassert.strictEqual(null, el.parentElement);
			const result = DomUtil.get('cool-caption', store.document);
			nodeassert.strictEqual(null, result);
		});

		it('with parent', function () {
			const el = DomUtil.createWithId(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			nodeassert.strictEqual('cool-caption', el.id);
			nodeassert.strictEqual(one, el.parentElement);
			nodeassert.strictEqual(1, one.childElementCount);
			const result = DomUtil.get('cool-caption', store.document);
			nodeassert.strictEqual(el, result);
		});
	});

	describe('remove()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('null element', function () {
			nodeassert.doesNotThrow(() => {
				DomUtil.remove();
			}, 'remove() should never throw');
		});

		it('parentless', function () {
			const el = DomUtil.createWithId(
				'p',
				'cool-caption',
				undefined,
				undefined,
				store.document,
			);
			nodeassert.doesNotThrow(() => {
				DomUtil.remove(el);
			}, 'remove() should never throw');
			nodeassert.strictEqual('cool-caption', el.id);
			nodeassert.strictEqual(null, el.parentElement);
			const result = DomUtil.get('cool-caption', store.document);
			nodeassert.strictEqual(null, result);
		});

		it('with parent', function () {
			const el = DomUtil.createWithId(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			nodeassert.doesNotThrow(() => {
				DomUtil.remove(el);
			}, 'remove() should never throw');
			nodeassert.strictEqual('cool-caption', el.id);
			nodeassert.strictEqual(null, el.parentElement);
			nodeassert.strictEqual(0, one.childElementCount);
			const result = DomUtil.get('cool-caption', store.document);
			nodeassert.strictEqual(null, result);
		});
	});

	describe('empty()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('already empty', function () {
			nodeassert.strictEqual(0, one.childElementCount);
			nodeassert.doesNotThrow(() => {
				DomUtil.empty(one);
			}, 'empty() should never throw');
			nodeassert.strictEqual(0, one.childElementCount);
		});

		it('two children', function () {
			nodeassert.strictEqual(0, one.childElementCount);
			DomUtil.createWithId(
				'p',
				'cool-caption1',
				one,
				undefined,
				store.document,
			);
			DomUtil.createWithId(
				'p',
				'cool-caption2',
				one,
				undefined,
				store.document,
			);
			nodeassert.strictEqual(2, one.childElementCount);
			nodeassert.doesNotThrow(() => {
				DomUtil.empty(one);
			}, 'empty() should never throw');
			nodeassert.strictEqual(0, one.childElementCount);

			const c1 = DomUtil.get('cool-caption1', store.document);
			const c2 = DomUtil.get('cool-caption2', store.document);
			nodeassert.strictEqual(null, c1);
			nodeassert.strictEqual(null, c2);
		});
	});

	describe('getClass()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('no class names', function () {
			const el = DomUtil.createWithId(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			nodeassert.strictEqual('', DomUtil.getClass(el));
		});

		it('one class name', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			nodeassert.strictEqual('cool-caption', DomUtil.getClass(el));
		});

		it('multiple class names', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption embossed blurred',
				one,
				undefined,
				store.document,
			);
			nodeassert.strictEqual(
				'cool-caption embossed blurred',
				DomUtil.getClass(el),
			);
		});
	});

	describe('hasClass()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('no class names', function () {
			const el = DomUtil.createWithId(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			nodeassert.ok(!DomUtil.hasClass(el, 'cool-caption'));
		});

		it('one class name', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			nodeassert.ok(!DomUtil.hasClass(el, 'cool'));
			nodeassert.ok(!DomUtil.hasClass(el, 'caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'cool-caption'));
		});

		it('multiple class names', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption embossed blurred',
				one,
				undefined,
				store.document,
			);
			nodeassert.ok(!DomUtil.hasClass(el, 'cool'));
			nodeassert.ok(!DomUtil.hasClass(el, 'caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'blurred'));
			nodeassert.ok(DomUtil.hasClass(el, 'embossed'));
			nodeassert.ok(!DomUtil.hasClass(el, 'embossed blurred'));
			nodeassert.ok(!DomUtil.hasClass(el, 'cool-caption embossed blurred'));
		});
	});

	describe('setClass()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('no class names', function () {
			const el = DomUtil.createWithId(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			DomUtil.setClass(el, 'blink dark');
			nodeassert.ok(!DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(DomUtil.hasClass(el, 'dark'));
		});

		it('one class name', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			DomUtil.setClass(el, 'blink dark');
			nodeassert.ok(!DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(DomUtil.hasClass(el, 'dark'));
		});

		it('multiple class names', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption embossed blurred',
				one,
				undefined,
				store.document,
			);
			DomUtil.setClass(el, 'blink dark');
			nodeassert.ok(!DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blurred'));
			nodeassert.ok(!DomUtil.hasClass(el, 'embossed'));
			nodeassert.ok(DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(DomUtil.hasClass(el, 'dark'));
		});
	});

	describe('addClass()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('no class names', function () {
			const el = DomUtil.createWithId(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			DomUtil.addClass(el, 'blink dark');
			nodeassert.ok(!DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(DomUtil.hasClass(el, 'dark'));
		});

		it('one class name', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			DomUtil.addClass(el, 'blink dark');
			nodeassert.ok(DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(DomUtil.hasClass(el, 'dark'));
		});

		it('multiple class names', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption embossed blurred',
				one,
				undefined,
				store.document,
			);
			DomUtil.addClass(el, 'blink dark');
			nodeassert.ok(DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'blurred'));
			nodeassert.ok(DomUtil.hasClass(el, 'embossed'));
			nodeassert.ok(DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(DomUtil.hasClass(el, 'dark'));
		});
	});

	describe('removeClass()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('no class names', function () {
			const el = DomUtil.createWithId(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			DomUtil.removeClass(el, 'blink');
			nodeassert.ok(!DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blink'));
		});

		it('one class name', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			DomUtil.removeClass(el, 'blink');
			nodeassert.ok(DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blink'));
			DomUtil.removeClass(el, 'cool-caption');
			nodeassert.ok(!DomUtil.hasClass(el, 'cool-caption'));
		});

		it('multiple class names', function () {
			const el = DomUtil.create(
				'p',
				'cool-caption embossed blurred blink dark',
				one,
				undefined,
				store.document,
			);

			DomUtil.removeClass(el, 'blink');
			nodeassert.ok(DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'embossed'));
			nodeassert.ok(DomUtil.hasClass(el, 'blurred'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(DomUtil.hasClass(el, 'dark'));

			DomUtil.removeClass(el, 'blurred');
			nodeassert.ok(DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'embossed'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blurred'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(DomUtil.hasClass(el, 'dark'));

			DomUtil.removeClass(el, 'dark');
			nodeassert.ok(DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'embossed'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blurred'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(!DomUtil.hasClass(el, 'dark'));

			DomUtil.removeClass(el, 'cool-caption');
			nodeassert.ok(!DomUtil.hasClass(el, 'cool-caption'));
			nodeassert.ok(DomUtil.hasClass(el, 'embossed'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blurred'));
			nodeassert.ok(!DomUtil.hasClass(el, 'blink'));
			nodeassert.ok(!DomUtil.hasClass(el, 'dark'));
		});
	});

	describe('removeChildNodes()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('already empty div', function () {
			const el = DomUtil.create(
				'div',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			nodeassert.strictEqual('', el.textContent);
			nodeassert.strictEqual(0, el.childNodes.length);
			DomUtil.removeChildNodes(el);
			nodeassert.strictEqual('', el.textContent);
			nodeassert.strictEqual(0, el.childNodes.length);
		});

		it('div with text', function () {
			const el = DomUtil.create(
				'div',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			el.textContent = 'Hello world!';
			nodeassert.strictEqual('Hello world!', el.textContent);
			nodeassert.strictEqual(1, el.childNodes.length);
			DomUtil.removeChildNodes(el);
			nodeassert.strictEqual('', el.textContent);
			nodeassert.strictEqual(0, el.childNodes.length);
		});

		it('div with text and a paragraph tag', function () {
			const el = DomUtil.create(
				'div',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			el.textContent = 'Hello world!';
			DomUtil.create('p', 'cool-caption', el, undefined, store.document);
			nodeassert.strictEqual('Hello world!', el.textContent);
			nodeassert.strictEqual(2, el.childNodes.length);
			DomUtil.removeChildNodes(el);
			nodeassert.strictEqual('', el.textContent);
			nodeassert.strictEqual(0, el.childNodes.length);
		});
	});

	describe('static data member checks', function () {
		it('TRANSFORM', function () {
			nodeassert.strictEqual('transform', DomUtil.TRANSFORM);
		});

		it('TRANSFORM_ORIGIN', function () {
			nodeassert.strictEqual('transformOrigin', DomUtil.TRANSFORM_ORIGIN);
		});

		it('TRANSITION_END', function () {
			nodeassert.strictEqual('webkitTransitionEnd', DomUtil.TRANSITION_END);
		});
	});

	describe('setTransform()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('no offset or scale', function () {
			const el = DomUtil.create(
				'div',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			DomUtil.setTransform(el);
			const tx = el.style.getPropertyValue(DomUtil.TRANSFORM);
			nodeassert.strictEqual(tx, 'translate3d(0px,0px,0)');
			DomUtil.remove(el);
		});

		it('only offset', function () {
			const el = DomUtil.create(
				'div',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			DomUtil.setTransform(el, new cool.Point(100, 200));
			const tx = el.style.getPropertyValue(DomUtil.TRANSFORM);
			nodeassert.strictEqual(tx, 'translate3d(100px,200px,0)');
			DomUtil.remove(el);
		});

		it('offset and scale', function () {
			const el = DomUtil.create(
				'div',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			DomUtil.setTransform(el, new cool.Point(100, 200), 5);
			const tx = el.style.getPropertyValue(DomUtil.TRANSFORM);
			nodeassert.strictEqual(tx, 'translate3d(100px,200px,0) scale(5)');
			DomUtil.remove(el);
		});
	});

	describe('setPosition()', function () {
		const store = new DOMStore(docstr);
		const one = DomUtil.get('one', store.document);

		it('set/get', function () {
			const el = DomUtil.create(
				'div',
				'cool-caption',
				one,
				undefined,
				store.document,
			);
			const pos = new cool.Point(128, 512);
			DomUtil.setPosition(el, pos);
			const posGot = DomUtil.getPosition(el);
			DomUtil.remove(el);
			nodeassert.deepStrictEqual(posGot, pos);
		});
	});
});
