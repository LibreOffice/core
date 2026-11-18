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

describe('SlideImportPane', function () {
	let listeners: Map<string, (e: any) => void>;
	let sent: string[];
	let saved: any;

	function deliver(name: string, message: any): void {
		const listener = listeners.get(name);
		nodeassert.ok(listener, 'nothing listens for ' + name);
		listener(message);
	}

	// The source answers with the slides it holds, named by their part identifiers.
	function info(parts: string[], source: string = 'Deck.odp'): void {
		deliver('remotedoccommandresult', {
			source: source,
			textMsg:
				'presentationinfo: ' +
				JSON.stringify({
					docWidth: 28000,
					docHeight: 15750,
					slides: parts.map((p) => ({ part: p, name: p, guid: 'g' + p })),
				}),
		});
	}

	function link(time: string, name: string = 'Deck.odp'): any {
		return {
			name: name,
			persistentLink: name,
			state: 'connected',
			access: 'granted',
			lastModifiedTime: time,
		};
	}

	function command(source: string, inner: string): string {
		return (
			'remotedoccommand source=' + encodeURIComponent(source) + ' ' + inner
		);
	}

	const askInfo = command('Deck.odp', 'getpresentationinfo');

	// The server announces that Deck.odp was saved.
	function saveDeck(others: any[] = []): void {
		(app as any).remoteLinks = [link('2026-11-11T10:05:00Z')].concat(others);
		deliver('remotelinks', {});
	}

	// What the pane asks the source to write out when the reader inserts the picked slides.
	function insertPicked(pane: any): string[] {
		sent = [];
		pane.session.insertSelected();
		pane.onExportWanted();
		return sent.filter((m) => m.indexOf('exportslides') >= 0);
	}

	// An open pane that shows the slides pA, pB and pC of Deck.odp.
	function openPane(): any {
		const map: any = {
			on: (name: string, cb: any, ctx: any) =>
				listeners.set(name, cb.bind(ctx)),
			fire: () => {},
			getDocType: () => 'presentation',
			isEditMode: () => true,
			isPresentationOrDrawing: () => true,
			getNumberOfParts: () => 1,
			wopi: { BaseFileName: 'Host.odp' },
		};
		const pane: any = new SlideImportPane(map);
		pane.render = () => {};

		pane.open();
		nodeassert.ok(sent.indexOf(askInfo) >= 0, 'the source was asked');
		info(['pA', 'pB', 'pC']);
		return pane;
	}

	beforeEach(function () {
		listeners = new Map();
		sent = [];
		saved = {
			socket: app.socket,
			events: app.events,
			remoteLinks: (app as any).remoteLinks,
			document: (globalThis as any).document,
			mode: (globalThis as any).mode,
		};
		const element: any = {
			classList: { add: () => {}, remove: () => {}, toggle: () => {} },
			replaceChildren: () => {},
			querySelector: () => null,
		};
		(globalThis as any).document = { getElementById: () => element };
		(globalThis as any).mode = { isSmallScreenDevice: () => false };
		(app as any).socket = {
			sendMessage: (m: string) => sent.push(m),
			pngDataUrl: () => 'data:',
		};
		(app as any).events = { on: () => {}, fire: () => {} };
		(app as any).remoteLinks = [link('2026-11-11T10:00:00Z')];
	});

	afterEach(function () {
		(app as any).socket = saved.socket;
		(app as any).events = saved.events;
		(app as any).remoteLinks = saved.remoteLinks;
		(globalThis as any).document = saved.document;
		(globalThis as any).mode = saved.mode;
	});

	it('inserts the slide that was picked after the source is saved', function () {
		const pane = openPane();

		// The reader picks the second slide, pB.
		pane.session.setSelection([1]);

		// The source is saved with a new slide in front of the others.
		sent = [];
		saveDeck();
		// The source answers only when it is asked.
		if (sent.indexOf(askInfo) >= 0) info(['pNew', 'pA', 'pB', 'pC']);

		nodeassert.deepEqual(insertPicked(pane), [
			command('Deck.odp', 'exportslides slides=pB'),
		]);
	});

	it('inserts the slide that was picked after the source answers with a new list', function () {
		const pane = openPane();
		pane.session.setSelection([1]);

		// The source answers a question the pane did not ask, with a new slide in front.
		info(['pNew', 'pA', 'pB', 'pC']);

		nodeassert.deepEqual(insertPicked(pane), [
			command('Deck.odp', 'exportslides slides=pB'),
		]);
	});

	it('leaves out a picked slide that the saved source no longer holds', function () {
		const pane = openPane();
		pane.session.setSelection([0, 1]);

		saveDeck();
		info(['pNew', 'pB']);

		nodeassert.deepEqual(insertPicked(pane), [
			command('Deck.odp', 'exportslides slides=pB'),
		]);
	});

	it('keeps the picks of a source the reader moved away from on their slides', function () {
		const other = link('2026-11-11T10:00:00Z', 'Other.odp');
		(app as any).remoteLinks = [link('2026-11-11T10:00:00Z'), other];
		const pane = openPane();
		pane.session.setSelection([1]);

		// The reader opens the other source, which holds the selection from then on.
		pane.toggleSource(pane.findSource('Other.odp'));
		info(['pX', 'pY'], 'Other.odp');
		pane.session.setSelection([0]);

		saveDeck([other]);
		info(['pNew', 'pA', 'pB', 'pC']);

		// The other source still inserts its own pick.
		nodeassert.deepEqual(insertPicked(pane), [
			command('Other.odp', 'exportslides slides=pX'),
		]);
		pane.session.exportFailed();

		// Back in the saved source, the pick is on the slide it was made on.
		pane.activate(pane.findSource('Deck.odp'));
		nodeassert.deepEqual(insertPicked(pane), [
			command('Deck.odp', 'exportslides slides=pB'),
		]);
	});

	it('asks for each thumbnail once when the saved source answers twice', function () {
		openPane();

		// A document with linked slides asks the saved source for its slides too, so two
		// answers arrive one after the other, before any thumbnail does.
		sent = [];
		saveDeck();
		info(['pNew', 'pA', 'pB', 'pC']);
		info(['pNew', 'pA', 'pB', 'pC']);

		const tiles = sent.filter((m) => m.indexOf(' tile ') >= 0);
		nodeassert.equal(tiles.length, 4);
	});

	it('asks for new thumbnails when the saved source holds the same slides', function () {
		openPane();

		// The source is saved with the same slides, one of them edited.
		sent = [];
		saveDeck();
		info(['pA', 'pB', 'pC']);
		info(['pA', 'pB', 'pC']);

		const tiles = sent.filter((m) => m.indexOf(' tile ') >= 0);
		nodeassert.equal(tiles.length, 3);
	});

	it('inserts the picked slide while the saved source has not answered yet', function () {
		const pane = openPane();
		pane.session.setSelection([1]);

		saveDeck();

		nodeassert.deepEqual(insertPicked(pane), [
			command('Deck.odp', 'exportslides slides=pB'),
		]);
	});
});
