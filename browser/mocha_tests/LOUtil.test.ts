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

describe('LOUtil', function () {
	describe('stringToBounds()', function () {
		it('parse from string with separaters and whitespaces', function () {
			const bounds = LOUtil.stringToBounds('1, 2, \n3,    \t4 ');
			const expected = cool.Bounds.toBounds([
				[1, 2],
				[4, 6],
			]);
			nodeassert.deepEqual(expected, bounds);
		});

		it('parse from string with more than 4 numbers', function () {
			const bounds = LOUtil.stringToBounds('1, 2, 3, 4, 5 ');
			const expected = cool.Bounds.toBounds([
				[1, 2],
				[4, 6],
			]);
			nodeassert.deepEqual(expected, bounds);
		});
	});

	describe('stringToRectangles()', function () {
		it('parse one rectangle from string with separaters and whitespaces', function () {
			const rectangles = LOUtil.stringToRectangles('1, 2, \n3,    \t4 ');
			const bottomLeft = cool.Point.toPoint(1, 6);
			const bottomRight = cool.Point.toPoint(4, 6);
			const topLeft = cool.Point.toPoint(1, 2);
			const topRight = cool.Point.toPoint(4, 2);
			const expected = [[bottomLeft, bottomRight, topLeft, topRight]];
			nodeassert.deepEqual(expected, rectangles);
		});

		it('parse two rectangles from string with separaters and whitespaces', function () {
			const rectangles = LOUtil.stringToRectangles(
				'1, 2, \n3,    \t4 ; 101, 202, \t3, \n4; ',
			);
			const bottomLeft = cool.Point.toPoint(1, 6);
			const bottomRight = cool.Point.toPoint(4, 6);
			const topLeft = cool.Point.toPoint(1, 2);
			const topRight = cool.Point.toPoint(4, 2);
			const offset = cool.Point.toPoint(100, 200);
			const first = [bottomLeft, bottomRight, topLeft, topRight];
			const second: cool.Point[] = [];
			for (let idx = 0; idx < 4; ++idx) {
				second.push(first[idx].add(offset));
			}
			const expected = [first, second];
			nodeassert.deepEqual(expected, rectangles);
		});
	});

	describe('findItemWithAttributeRecursive()', function () {
		it('match at depth 0', function () {
			const expected = LOUtilTestData.tree;
			nodeassert.deepEqual(
				expected,
				LOUtil.findItemWithAttributeRecursive(
					LOUtilTestData.tree,
					'id',
					'level0',
				),
			);
		});

		it('match at depth 1', function () {
			const expected = LOUtilTestData.tree.children[1];
			nodeassert.deepEqual(
				expected,
				LOUtil.findItemWithAttributeRecursive(
					LOUtilTestData.tree,
					'id',
					'level1_2',
				),
			);
		});

		it('match at depth 2', function () {
			const expected = LOUtilTestData.tree.children[0].children[0];
			nodeassert.deepEqual(
				expected,
				LOUtil.findItemWithAttributeRecursive(
					LOUtilTestData.tree,
					'id',
					'level2',
				),
			);
		});
		it('match at depth 3', function () {
			const expected = LOUtilTestData.tree.children[0].children[0].children[1];
			nodeassert.deepEqual(
				expected,
				LOUtil.findItemWithAttributeRecursive(
					LOUtilTestData.tree,
					'id',
					'level3',
				),
			);
		});

		it('no match in empty tree', function () {
			nodeassert.deepEqual(
				null,
				LOUtil.findItemWithAttributeRecursive({}, 'id', 'level0'),
			);
		});

		it('no match for value', function () {
			nodeassert.deepEqual(
				null,
				LOUtil.findItemWithAttributeRecursive(
					LOUtilTestData.tree,
					'id',
					'level100',
				),
			);
		});

		it('no match for key', function () {
			nodeassert.deepEqual(
				null,
				LOUtil.findItemWithAttributeRecursive(
					LOUtilTestData.tree,
					'somekey',
					'level3',
				),
			);
		});
	});

	describe('findIndexInParentByAttribute()', function () {
		it('empty tree no match', function () {
			nodeassert.equal(
				-1,
				LOUtil.findIndexInParentByAttribute({}, 'class', 'xyz'),
			);
		});

		it('non empty tree no match', function () {
			nodeassert.equal(
				-1,
				LOUtil.findIndexInParentByAttribute(
					LOUtilTestData.shortTree,
					'class',
					'xyz',
				),
			);
		});

		it('multiple matches; get first', function () {
			nodeassert.equal(
				2,
				LOUtil.findIndexInParentByAttribute(
					LOUtilTestData.shortTree,
					'class',
					'bac',
				),
			);
		});

		it('unique match', function () {
			nodeassert.equal(
				2,
				LOUtil.findIndexInParentByAttribute(
					LOUtilTestData.shortTree,
					'name',
					'BAC',
				),
			);
		});
	});

	describe('_doRectanglesIntersect()', function () {
		it('rectangle intersects with itself', function () {
			nodeassert.ok(
				LOUtil._doRectanglesIntersect([10, 20, 100, 200], [10, 20, 100, 200]),
			);
		});

		it('A contains B, there is intersection', function () {
			nodeassert.ok(
				LOUtil._doRectanglesIntersect([10, 20, 100, 200], [11, 21, 90, 190]),
			);
		});

		it('B contains A, there is intersection', function () {
			nodeassert.ok(
				LOUtil._doRectanglesIntersect([11, 21, 90, 190], [10, 20, 100, 200]),
			);
		});

		it('A meets B tangentially (vertical)', function () {
			nodeassert.ok(
				LOUtil._doRectanglesIntersect([10, 20, 100, 200], [109, 20, 10, 200]),
			);
		});

		it('A meets B tangentially (horizontal)', function () {
			nodeassert.ok(
				LOUtil._doRectanglesIntersect([10, 20, 100, 200], [10, 219, 100, 20]),
			);
		});

		it('A meets B tangentially (single point)', function () {
			nodeassert.ok(
				LOUtil._doRectanglesIntersect([10, 20, 100, 200], [109, 219, 10, 20]),
			);
		});

		it('disjoint ([x2, y2] of first rectangle is away from the [x1, y1] of the second by a pixel)', function () {
			nodeassert.ok(
				!LOUtil._doRectanglesIntersect(
					[10, 20, 100, 200],
					[110, 220, 100, 200],
				),
			);
		});

		it('disjoint (rectangles very far away)', function () {
			nodeassert.ok(
				!LOUtil._doRectanglesIntersect([10, 20, 10, 20], [400, 500, 10, 20]),
			);
		});
	});

	describe('_getIntersectionRectangle', function () {
		it('rectangle intersects with itself', function () {
			const expected = [10, 20, 100, 200];
			nodeassert.deepEqual(
				expected,
				LOUtil._getIntersectionRectangle(
					[10, 20, 100, 200],
					[10, 20, 100, 200],
				),
			);
		});

		it('A contains B, there is intersection', function () {
			const expected = [11, 21, 90, 190];
			nodeassert.deepEqual(
				expected,
				LOUtil._getIntersectionRectangle([10, 20, 100, 200], [11, 21, 90, 190]),
			);
		});

		it('B contains A, there is intersection', function () {
			const expected = [11, 21, 90, 190];
			nodeassert.deepEqual(
				expected,
				LOUtil._getIntersectionRectangle([11, 21, 90, 190], [10, 20, 100, 200]),
			);
		});

		it('A meets B tangentially (vertical)', function () {
			const expected = [109, 20, 1, 200];
			nodeassert.deepEqual(
				expected,
				LOUtil._getIntersectionRectangle(
					[10, 20, 100, 200],
					[109, 20, 10, 200],
				),
			);
		});

		it('A meets B tangentially (horizontal)', function () {
			const expected = [10, 219, 100, 1];
			nodeassert.deepEqual(
				expected,
				LOUtil._getIntersectionRectangle(
					[10, 20, 100, 200],
					[10, 219, 100, 20],
				),
			);
		});

		it('A meets B tangentially (single point)', function () {
			const expected = [109, 219, 1, 1];
			nodeassert.deepEqual(
				expected,
				LOUtil._getIntersectionRectangle(
					[10, 20, 100, 200],
					[109, 219, 10, 20],
				),
			);
		});

		it('disjoint ([x2, y2] of first rectangle is away from the [x1, y1] of the second by a pixel)', function () {
			const expected: number[] = null;
			nodeassert.deepEqual(
				expected,
				LOUtil._getIntersectionRectangle(
					[10, 20, 100, 200],
					[110, 220, 100, 200],
				),
			);
		});

		it('disjoint (rectangles very far away)', function () {
			const expected: number[] = null;
			nodeassert.deepEqual(
				expected,
				LOUtil._getIntersectionRectangle([10, 20, 10, 20], [400, 500, 10, 20]),
			);
		});
	});

	describe('getIconNameOfCommand()', function () {
		const styleApplyFile =
			'lc_styleapply3fstyle3astring3dbad26familyname3astring3dcellstyles.svg';

		it('a plain command maps to its icon file', function () {
			nodeassert.strictEqual(
				'lc_bold.svg',
				LOUtil.getIconNameOfCommand('.uno:Bold'),
			);
		});

		it('an icon theme link name maps to the same file as its command', function () {
			nodeassert.strictEqual(
				'lc_bold.svg',
				LOUtil.getIconNameOfCommand('lc_bold.svg'),
			);
			nodeassert.strictEqual(
				'lc_bold.svg',
				LOUtil.getIconNameOfCommand('lc_bold.svg', true),
			);
			nodeassert.strictEqual(
				'lc_bold.svg',
				LOUtil.getIconNameOfCommand('sc_bold.svg', true),
			);
		});

		it('a parameterized command maps to its icon file', function () {
			nodeassert.strictEqual(
				styleApplyFile,
				LOUtil.getIconNameOfCommand(
					'.uno:StyleApply?Style:string=Bad&FamilyName:string=CellStyles',
				),
			);
		});

		it('the engine link name of a parameterized command maps to the same file', function () {
			const linkName =
				'lc_styleapply%3fstyle%3astring%3dbad&familyname%3astring%3dcellstyles.svg';
			nodeassert.strictEqual(
				styleApplyFile,
				LOUtil.getIconNameOfCommand(linkName),
			);
			nodeassert.strictEqual(
				styleApplyFile,
				LOUtil.getIconNameOfCommand(linkName, true),
			);
			nodeassert.strictEqual(
				styleApplyFile,
				LOUtil.getIconNameOfIcon(linkName),
			);
		});

		it('a link name with a stray percent sign is kept as sent', function () {
			nodeassert.strictEqual(
				'lc_foo25bar.svg',
				LOUtil.getIconNameOfCommand('lc_foo%bar.svg'),
			);
		});
	});

	describe('openTemplatesFromDocumentLogo()', function () {
		function setUpLogo() {
			const logo = document.createElement('a');
			const tabsShown: string[] = [];
			const map = {
				backstageView: {
					show: function (tab: string) {
						tabsShown.push(tab);
					},
				},
			};
			LOUtil.openTemplatesFromDocumentLogo(logo, map);
			return { logo: logo, tabsShown: tabsShown };
		}

		it('the icon takes keyboard focus and is announced as a button', function () {
			const setup = setUpLogo();
			nodeassert.strictEqual('button', setup.logo.getAttribute('role'));
			nodeassert.strictEqual('0', setup.logo.getAttribute('tabindex'));
		});

		it('a click opens the templates', function () {
			const setup = setUpLogo();
			setup.logo.click();
			nodeassert.deepEqual(['new'], setup.tabsShown);
		});

		it('Enter and Space open the templates', function () {
			const setup = setUpLogo();
			setup.logo.dispatchEvent(
				new window.KeyboardEvent('keydown', { key: 'Enter' }),
			);
			setup.logo.dispatchEvent(
				new window.KeyboardEvent('keydown', { key: ' ' }),
			);
			nodeassert.deepEqual(['new', 'new'], setup.tabsShown);
		});

		it('any other key leaves the document on screen', function () {
			const setup = setUpLogo();
			setup.logo.dispatchEvent(
				new window.KeyboardEvent('keydown', { key: 'a' }),
			);
			nodeassert.deepEqual([], setup.tabsShown);
		});
	});

	describe('setImage()', function () {
		let dom: any;
		let dark: boolean;
		let realGetImageURL: any;
		let realHTMLImageElement: any;
		let realImage: any;

		// A map with the on and fire of Evented, enough for themechanged.
		function makeMap(): any {
			const listeners: Array<() => void> = [];
			return {
				listenerCount: () => listeners.length,
				on: (name: string, callback: () => void) => {
					if (name === 'themechanged') listeners.push(callback);
				},
				fireThemeChanged: () => {
					dark = !dark;
					listeners.slice().forEach((callback) => callback());
				},
			};
		}

		// jsdom does not compute visibility, so each icon says whether it is visible.
		function makeIcon(tag: string, visible: boolean = true): any {
			const element = dom.window.document.createElement(tag);
			element.checkVisibility = () => visible;
			return element;
		}

		// Runs the tasks that are already queued, and the tasks they queue, up to the given depth.
		async function runPendingTasks(depth: number = 1): Promise<void> {
			for (let i = 0; i < depth; i++)
				await new Promise((resolve) => setTimeout(resolve, 0));
		}

		before(function () {
			dom = new jsdom.JSDOM('<html><body></body></html>');
			realHTMLImageElement = (globalThis as any).HTMLImageElement;
			realImage = (globalThis as any).Image;
			(globalThis as any).HTMLImageElement = dom.window.HTMLImageElement;
			(globalThis as any).Image = dom.window.Image;
			realGetImageURL = LOUtil.getImageURL;
			LOUtil.getImageURL = (name: string) =>
				'http://host/images/' + (dark ? 'dark/' : '') + name;
		});

		after(function () {
			LOUtil.getImageURL = realGetImageURL;
			(globalThis as any).HTMLImageElement = realHTMLImageElement;
			(globalThis as any).Image = realImage;
		});

		beforeEach(function () {
			dark = false;
		});

		it('every icon follows a theme change', function () {
			const map = makeMap();
			const first = makeIcon('img');
			const second = makeIcon('img');
			LOUtil.setImage(first, 'lc_bold.svg', map);
			LOUtil.setImage(second, 'lc_italic.svg', map);

			map.fireThemeChanged();

			nodeassert.strictEqual(first.src, 'http://host/images/dark/lc_bold.svg');
			nodeassert.strictEqual(
				second.src,
				'http://host/images/dark/lc_italic.svg',
			);
		});

		it('all icons of a map share one theme listener', function () {
			const map = makeMap();
			for (let i = 0; i < 50; i++)
				LOUtil.setImage(makeIcon('img'), 'lc_bold.svg', map);

			nodeassert.strictEqual(map.listenerCount(), 1);
		});

		it('an icon set up again shows its latest name after a theme change', function () {
			const map = makeMap();
			const image = makeIcon('img');
			LOUtil.setImage(image, 'lc_bold.svg', map);
			LOUtil.setImage(image, 'lc_italic.svg', map);

			map.fireThemeChanged();

			nodeassert.strictEqual(
				image.src,
				'http://host/images/dark/lc_italic.svg',
			);
		});

		it('an icon set up five times is loaded once per theme change', function () {
			const map = makeMap();
			const image = makeIcon('img');
			for (let i = 0; i < 5; i++) LOUtil.setImage(image, 'lc_bold.svg', map);

			let loads = 0;
			const stub = LOUtil.getImageURL;
			LOUtil.getImageURL = (name: string) => {
				loads++;
				return stub(name);
			};
			map.fireThemeChanged();
			LOUtil.getImageURL = stub;

			nodeassert.strictEqual(loads, 1);
		});

		it('an element with a background image follows a theme change', function () {
			const map = makeMap();
			const div = makeIcon('div');
			LOUtil.setImage(div, 'lc_bold.svg', map);

			map.fireThemeChanged();

			nodeassert.ok(
				div.style.backgroundImage.includes('images/dark/lc_bold.svg'),
			);
		});

		it('a visible icon follows the theme at once and a hidden icon one task later', async function () {
			const map = makeMap();
			const hidden = makeIcon('img', false);
			const visible = makeIcon('img');
			LOUtil.setImage(hidden, 'lc_bold.svg', map);
			LOUtil.setImage(visible, 'lc_italic.svg', map);

			map.fireThemeChanged();
			const rightAfter = [visible.src, hidden.src];
			await runPendingTasks();

			nodeassert.deepEqual(rightAfter, [
				'http://host/images/dark/lc_italic.svg',
				'http://host/images/lc_bold.svg',
			]);
			nodeassert.strictEqual(hidden.src, 'http://host/images/dark/lc_bold.svg');
		});

		it('many hidden icons all follow the theme', async function () {
			const map = makeMap();
			const images: any[] = [];
			for (let i = 0; i < 45; i++) {
				images.push(makeIcon('img', false));
				LOUtil.setImage(images[i], 'lc_bold.svg', map);
			}

			map.fireThemeChanged();
			await runPendingTasks(5);

			const stale = images.filter(
				(image) => image.src !== 'http://host/images/dark/lc_bold.svg',
			);
			nodeassert.strictEqual(stale.length, 0);
		});

		it('a hidden icon ends in the theme of the last of two quick changes', async function () {
			const map = makeMap();
			const images: any[] = [];
			for (let i = 0; i < 45; i++) {
				images.push(makeIcon('img', false));
				LOUtil.setImage(images[i], 'lc_bold.svg', map);
			}

			map.fireThemeChanged();
			await runPendingTasks();
			map.fireThemeChanged();
			await runPendingTasks(5);

			const stale = images.filter(
				(image) => image.src !== 'http://host/images/lc_bold.svg',
			);
			nodeassert.strictEqual(stale.length, 0);
		});

		it('a quick second change loads each pending hidden icon once', async function () {
			const map = makeMap();
			for (let i = 0; i < 45; i++)
				LOUtil.setImage(makeIcon('img', false), 'icon' + i + '.svg', map);

			map.fireThemeChanged();
			await runPendingTasks();
			const loads: string[] = [];
			const stub = LOUtil.getImageURL;
			LOUtil.getImageURL = (name: string) => {
				loads.push(name);
				return stub(name);
			};
			map.fireThemeChanged();
			await runPendingTasks(5);
			LOUtil.getImageURL = stub;

			nodeassert.strictEqual(
				loads.filter((name) => name === 'icon44.svg').length,
				1,
			);
		});

		it('a hidden icon renamed before it follows the theme shows its new name', async function () {
			const map = makeMap();
			const image = makeIcon('img', false);
			LOUtil.setImage(image, 'lc_bold.svg', map);

			map.fireThemeChanged();
			LOUtil.setImage(image, 'lc_italic.svg', map);
			await runPendingTasks();

			nodeassert.strictEqual(
				image.src,
				'http://host/images/dark/lc_italic.svg',
			);
		});
	});

	describe('checkIfImageExists()', function () {
		let dom: any;

		before(function () {
			dom = new jsdom.JSDOM('<html><body></body></html>');
		});

		it('an image checked many times gets one error handler', function () {
			const image = dom.window.document.createElement('img');
			let errorHandlers = 0;
			const add = image.addEventListener.bind(image);
			image.addEventListener = (type: string, listener: any, options?: any) => {
				if (type === 'error') errorHandlers++;
				add(type, listener, options);
			};

			for (let i = 0; i < 3; i++) LOUtil.checkIfImageExists(image);

			nodeassert.strictEqual(errorHandlers, 1);
		});

		it('a missing dark icon falls back to the light icon', function () {
			const image = dom.window.document.createElement('img');
			LOUtil.checkIfImageExists(image);
			LOUtil.checkIfImageExists(image);
			image.src = 'http://host/images/dark/lc_bold.svg';

			image.dispatchEvent(new dom.window.Event('error'));

			nodeassert.strictEqual(image.src, 'http://host/images/lc_bold.svg');
		});

		it('the latest layout-critical flag decides the fallback', function () {
			const image = dom.window.document.createElement('img');
			LOUtil.checkIfImageExists(image, false);
			LOUtil.checkIfImageExists(image, true);
			image.src = 'http://host/images/lc_bold.svg';

			image.dispatchEvent(new dom.window.Event('error'));

			nodeassert.ok(image.src.startsWith('data:image/png;base64,'));
			nodeassert.notStrictEqual(image.style.display, 'none');
		});
	});
});
