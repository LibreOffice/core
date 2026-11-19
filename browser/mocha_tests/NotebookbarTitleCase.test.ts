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

// Reads the notebookbar sources as text, so that items shown only under some
// condition (a tablet, a WOPI host with revision history, an OOXML document, a
// context tab) are checked as well.
describe('Notebookbar labels', function () {
	const fs = require('fs');
	const path = require('path');
	const vm = require('vm');

	type Found = { where: string; label: string };

	const browserDir = path.join(__dirname, '..', '..');
	const controlDir = path.join(browserDir, 'src', 'control');

	// The engine source tree that LO_PATH, its instdir, belongs to.
	const engineDir = process.env.ENGINE_WORKDIR
		? path.dirname(process.env.ENGINE_WORKDIR)
		: path.join(browserDir, '..', 'engine');
	const notebookbarSource = path.join(
		engineDir,
		'sfx2',
		'source',
		'notebookbar',
		'SfxNotebookBar.cxx',
	);
	const uiDir = path.join(
		engineDir,
		'instdir',
		'share',
		'config',
		'soffice.cfg',
	);

	// _UNO as the browser runs it, over the generated command labels.
	const unoSandbox: any = { _: (text: string) => text, window: {} };
	vm.createContext(unoSandbox);
	vm.runInContext(
		fs.readFileSync(path.join(browserDir, 'src', 'unocommands.js'), 'utf8'),
		unoSandbox,
	);

	const definitionFiles: string[] = fs
		.readdirSync(controlDir)
		.filter((file: string) =>
			/^(Control\.Notebookbar.*\.(js|ts)|Notebookbar\..*\.ts)$/.test(file),
		)
		.sort();

	// Every string these translate is a label of the notebookbar, though most are
	// not given under a key.
	const widgetFiles: string[] = [
		'Widget.OverflowGroup.ts',
		'Widget.NotebookbarIconView.ts',
	];

	const translated =
		/\b_\(\s*'(?:\\.|[^'])*'\s*\)|\b_\(\s*"(?:\\.|[^"])*"\s*\)|\b_UNO\([^)]*\)/g;

	function lineAt(source: string, at: number): number {
		return source.slice(0, at).split('\n').length;
	}

	// The expression after a key, up to the comma or bracket that ends it.
	function valueAt(source: string, from: number): string {
		let depth = 0;
		let quote: string | null = null;

		for (let at = from; at < source.length; at++) {
			const c = source.charAt(at);

			if (quote) {
				if (c === '\\') at++;
				else if (c === quote) quote = null;
			} else if (c === "'" || c === '"' || c === '`') {
				quote = c;
			} else if ('([{'.indexOf(c) >= 0) {
				depth++;
			} else if (')]}'.indexOf(c) >= 0) {
				if (depth === 0) return source.slice(from, at);
				depth--;
			} else if ((c === ',' || c === ';') && depth === 0) {
				return source.slice(from, at);
			}
		}

		return source.slice(from);
	}

	function labelOf(call: string): string {
		if (call.indexOf('_UNO') === 0)
			return vm.runInContext('window.' + call, unoSandbox);

		return call.replace(/^_\(\s*['"]/, '').replace(/['"]\s*\)$/, '');
	}

	// Every translated string a text, name or label key of the definitions is given.
	// The offset just past the bracket that closes the one at this offset.
	function closingBracket(source: string, open: number): number {
		let depth = 0;
		let quote: string | null = null;

		for (let at = open; at < source.length; at++) {
			const c = source.charAt(at);

			if (quote) {
				if (c === '\\') at++;
				else if (c === quote) quote = null;
			} else if (c === "'" || c === '"' || c === '`') {
				quote = c;
			} else if ('([{'.indexOf(c) >= 0) {
				depth++;
			} else if (')]}'.indexOf(c) >= 0) {
				depth--;
				if (depth === 0) return at + 1;
			}
		}

		return source.length;
	}

	// The bodies of the functions that build the entries of the File tab's
	// dropdown menus. Menu entries are not held to Title Case.
	function submenuBodies(source: string): [number, number][] {
		const bodies: [number, number][] = [];
		const builder = /\b_get\w*SubmenuOpts\s*:\s*function\s*\([^)]*\)\s*\{/g;
		let m: RegExpExecArray | null;

		while ((m = builder.exec(source)) !== null) {
			const open = m.index + m[0].length - 1;
			bodies.push([open, closingBracket(source, open)]);
		}

		return bodies;
	}

	function definitionLabels(): Found[] {
		const found: Found[] = [];

		definitionFiles.forEach((file: string) => {
			const source: string = fs.readFileSync(
				path.join(controlDir, file),
				'utf8',
			);
			const menus = submenuBodies(source);
			const key = /(['"]?)\b(text|name|label)\1\s*:\s*/g;
			let m: RegExpExecArray | null;

			while ((m = key.exec(source)) !== null) {
				const at = m.index;

				if (menus.some((body) => at > body[0] && at < body[1])) continue;

				const where = file + ':' + lineAt(source, m.index);
				const calls = valueAt(source, key.lastIndex).match(translated) || [];

				calls.forEach((call: string) => {
					found.push({ where: where, label: labelOf(call) });
				});
			}
		});

		return found;
	}

	function widgetLabels(): Found[] {
		const found: Found[] = [];

		widgetFiles.forEach((file: string) => {
			const source: string = fs.readFileSync(
				path.join(controlDir, 'jsdialog', file),
				'utf8',
			);
			let m: RegExpExecArray | null;

			translated.lastIndex = 0;
			while ((m = translated.exec(source)) !== null)
				found.push({
					where: 'jsdialog/' + file + ':' + lineAt(source, m.index),
					label: labelOf(m[0]),
				});
		});

		return found;
	}

	// The .ui files core welds into the notebookbar, from its list of extra panels.
	function weldedPanels(): string[] {
		const source: string = fs.readFileSync(notebookbarSource, 'utf8');
		const panels: string[] = [];
		const ui = /u"([^"]+\.ui)"_ustr/g;
		let m: RegExpExecArray | null;

		while ((m = ui.exec(source)) !== null)
			if (m[1].indexOf('notebookbar_online.ui') < 0) panels.push(m[1]);

		return panels;
	}

	function unescapeXml(text: string): string {
		return text
			.replace(/&lt;/g, '<')
			.replace(/&gt;/g, '>')
			.replace(/&quot;/g, '"')
			.replace(/&apos;/g, "'")
			.replace(/&amp;/g, '&');
	}

	// The properties of the object a property at this offset belongs to, up to its
	// first child.
	function ownProperties(source: string, at: number): string {
		const from = source.lastIndexOf('<object ', at);
		const ends = [source.indexOf('<child', at), source.indexOf('</object>', at)]
			.filter((end) => end >= 0)
			.sort((a, b) => a - b);

		return source.slice(from, ends.length ? ends[0] : source.length);
	}

	// What core shows of a welded panel: the labels of its .ui, without the
	// mnemonic, and the labels of the commands its buttons run. A widget the .ui
	// hides, as the switch core and online report the Formula tab with, shows none.
	function weldedLabels(): Found[] {
		const found: Found[] = [];

		weldedPanels().forEach((panel: string) => {
			const source: string = fs.readFileSync(path.join(uiDir, panel), 'utf8');
			const label =
				/<property name="label" translatable="yes"[^>]*>([^<]*)<\/property>/g;
			const action = /<property name="action_name">(\.uno:[^<]+)<\/property>/g;
			let m: RegExpExecArray | null;

			while ((m = label.exec(source)) !== null) {
				const hidden = /<property name="visible">False<\/property>/.test(
					ownProperties(source, m.index),
				);

				if (!hidden)
					found.push({
						where: panel + ':' + lineAt(source, m.index),
						label: unescapeXml(m[1]).replace(/_(?!_)/g, ''),
					});
			}

			while ((m = action.exec(source)) !== null) {
				const command = m[1];
				const text: string = unoSandbox.window._UNO(command);

				// A command unocommands.js does not list is labelled by core alone.
				if (text !== command.substring(5))
					found.push({
						where: panel + ':' + lineAt(source, m.index),
						label: text,
					});
			}
		});

		return found;
	}

	function notInTitleCase(found: Found[]): string[] {
		return found
			.filter((item) => TitleCase.lowerCaseWords(item.label).length > 0)
			.map((item) => item.where + ' "' + item.label + '"');
	}

	it('are read from every notebookbar definition', function () {
		const all = definitionLabels();

		nodeassert.ok(definitionFiles.length > 0, 'notebookbar definition files');
		nodeassert.ok(
			all.some((found) => found.label === 'Wrap Text'),
			'a label of a _UNO command is resolved',
		);
		nodeassert.ok(
			all.some((found) => found.label === 'Show Tour'),
			'a label of a translated string is read',
		);
	});

	it('are written in Title Case in the definitions', function () {
		const offenders = notInTitleCase(definitionLabels());

		nodeassert.deepStrictEqual(offenders, [], offenders.join('\n'));
	});

	it('are written in Title Case by the widgets', function () {
		const found = widgetLabels();

		nodeassert.ok(
			found.some((item) => item.label === 'More Options for {1}'),
			'the label of an overflow group is read',
		);

		const offenders = notInTitleCase(found);

		nodeassert.deepStrictEqual(offenders, [], offenders.join('\n'));
	});

	it('are written in Title Case in the panels core welds in', function () {
		if (!fs.existsSync(notebookbarSource) || !fs.existsSync(uiDir)) {
			console.log(
				'\nNo engine source tree with an instdir at ' +
					engineDir +
					', so the welded panels are not checked. Point ENGINE_WORKDIR at ' +
					'the workdir of the engine LO_PATH is built from.\n',
			);
			this.skip();
		}

		const panels = weldedPanels();
		const found = weldedLabels();

		nodeassert.ok(panels.length > 0, 'welded panels listed by core');
		nodeassert.ok(
			found.some((item) => item.label === 'Start:'),
			'a label of a welded panel is read',
		);

		const offenders = notInTitleCase(found);

		nodeassert.deepStrictEqual(offenders, [], offenders.join('\n'));
	});
});
