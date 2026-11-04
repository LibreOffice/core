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

describe('SlideImportSession', function () {
	// The messages of the server the session listens for, by name, so a test
	// can hand it one as the socket would.
	let listeners: Map<string, (e: any) => void>;
	// What the session sent to the document and announced to the pane.
	let sent: string[];
	let fired: string[];
	// The document the pages go into, with the slide on show at _docLayer._selectedPart.
	let map: any;
	let session: SlideImportSession;
	let savedSocket: any;
	let savedEvents: any;
	let savedRemoteLinks: any;

	// The source the pane has open, its slides on show, and the time the
	// storage reports for it.
	const source = 'Sales deck.odp';
	const modified = '2026-11-04T10:00:00Z';
	// The file the server staged the pages of the source under.
	const staged = 'staged-1.odp';
	// A section of the source: its last two slides.
	const opening: SlideImportSection = {
		name: 'Opening',
		id: '{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}',
		startIndex: 1,
		slideCount: 2,
	};

	function deliver(name: string, message: any): void {
		const listener = listeners.get(name);
		nodeassert.ok(listener, 'nothing listens for ' + name);
		listener(message);
	}

	// The head of the insert command of the pane's source, up to the link
	// flag, for pages that go in before the given slide.
	function insertOf(at: number): string {
		return (
			'slideimport insert file=' +
			staged +
			' source=' +
			encodeURIComponent(source) +
			' at=' +
			at +
			' keepdesign=0'
		);
	}

	const time = ' time=' + encodeURIComponent(modified);
	const presentation = ' as=presentation name=' + encodeURIComponent(source);

	// The source is open in the pane with three slides on show, so an insert
	// can be asked of it.
	function sourceShown(): void {
		session.setSource(source);
		session.slideCount = 3;
		session.slidesShown();
		fired = [];
	}

	// The pane's part of an insert: the source wrote its pages, the server
	// staged them, and the pane hands the file to the session. The pages of a
	// pick go in as the given section, or as slides.
	function pagesStaged(section: SlideImportSection | null = null): void {
		nodeassert.ok(
			session.getPendingInsert(),
			'nothing is waiting to be inserted',
		);
		session.insertStaged(staged, section);
	}

	beforeEach(function () {
		listeners = new Map();
		sent = [];
		fired = [];

		map = {
			on: function (name: string, callback: any, context: any) {
				listeners.set(name, callback.bind(context));
			},
			fire: function () {},
			getNumberOfParts: () => 5,
			_docLayer: { _selectedPart: 1 },
			// The document is ODF, which keeps the links of inserted slides.
			wopi: { BaseFileName: 'Plan.odp' },
		};

		savedSocket = app.socket;
		savedEvents = app.events;
		savedRemoteLinks = (app as any).remoteLinks;
		(app as any).socket = {
			sendMessage: (message: string) => sent.push(message),
		};
		(app as any).events = {
			on: () => {},
			fire: (name: string) => fired.push(name),
		};
		(app as any).remoteLinks = [
			{
				name: source,
				persistentLink: source,
				state: 'connected',
				lastModifiedTime: modified,
			},
		];

		session = new SlideImportSession(map);
	});

	afterEach(function () {
		(app as any).socket = savedSocket;
		(app as any).events = savedEvents;
		(app as any).remoteLinks = savedRemoteLinks;
	});

	it('links the whole source as one presentation after the slide on show', function () {
		sourceShown();
		session.insertWhole(source);

		nodeassert.equal(session.state, 'inserting');
		nodeassert.deepEqual(fired, [
			'slideimport:statechange',
			'slideimport:exportwanted',
		]);
		nodeassert.deepEqual(session.getPendingInsert(), {
			slides: [],
			at: 2,
			whole: true,
			name: source,
		});

		pagesStaged();
		nodeassert.deepEqual(sent, [insertOf(2) + ' link=1' + time + presentation]);
	});

	it('links the whole source whatever the link choices say', function () {
		sourceShown();
		session.setLinkToSource(false);
		session.setLinkByPosition(true);
		session.insertWhole(source);

		pagesStaged();
		nodeassert.deepEqual(sent, [insertOf(2) + ' link=1' + time + presentation]);
	});

	it('links the whole source as a presentation even when handed a section', function () {
		sourceShown();
		session.insertWhole(source);

		pagesStaged(opening);
		nodeassert.deepEqual(sent, [insertOf(2) + ' link=1' + time + presentation]);
	});

	it('records as much of the name of the source as a link keeps', function () {
		sourceShown();
		// 127 plain characters, a tab, and a character of two code units that the cut
		// would split, followed by more.
		const name = 'a'.repeat(127) + '\t' + '\u{1F600}' + 'bcd';
		session.insertWhole(name);

		const recorded = session.getPendingInsert().name;
		nodeassert.equal(recorded, 'a'.repeat(127));
		pagesStaged();
		nodeassert.deepEqual(sent, [
			insertOf(2) +
				' link=1' +
				time +
				' as=presentation name=' +
				'a'.repeat(127),
		]);
	});

	it('keeps the pick once the whole source is in', function () {
		sourceShown();
		session.setSelection([0, 2]);
		session.insertWhole(source);
		pagesStaged();
		deliver('slideimport', { message: { status: 'inserted', count: 3 } });

		nodeassert.equal(session.state, 'ready');
		nodeassert.deepEqual(Array.from(session.selection), [0, 2]);
	});

	it('drops the retry of a whole insert once another source is picked', function () {
		sourceShown();
		session.insertWhole(source);
		session.exportFailed();
		nodeassert.ok(session.error.retry);

		session.setSource('Other deck.odp');
		nodeassert.equal(session.error, null);
	});

	it('puts the whole source at the end while no slide is on show', function () {
		map._docLayer._selectedPart = undefined;
		sourceShown();
		session.insertWhole(source);

		nodeassert.equal(session.getPendingInsert().at, 5);
		pagesStaged();
		nodeassert.deepEqual(sent, [insertOf(5) + ' link=1' + time + presentation]);
	});

	it('does not link the whole source while another insert runs', function () {
		sourceShown();
		session.insertWhole(source);
		fired = [];

		session.insertWhole(source);
		nodeassert.deepEqual(fired, []);
	});

	it('does not link the whole source without a persistent link', function () {
		session.setSource('');
		session.slideCount = 3;
		session.slidesShown();
		fired = [];

		session.insertWhole('Sales deck');
		nodeassert.equal(session.state, 'ready');
		nodeassert.deepEqual(fired, []);
		nodeassert.equal(session.getPendingInsert(), null);
	});

	it('asks for the same whole insert again when the source could not be read', function () {
		sourceShown();
		session.insertWhole(source);
		session.exportFailed();

		nodeassert.equal(session.state, 'ready');
		nodeassert.equal(
			session.error.message,
			'The slides of the source could not be read.',
		);
		nodeassert.equal(session.getPendingInsert(), null);

		fired = [];
		session.error.retry();
		nodeassert.equal(session.state, 'inserting');
		nodeassert.ok(fired.includes('slideimport:exportwanted'));
		nodeassert.deepEqual(session.getPendingInsert(), {
			slides: [],
			at: 2,
			whole: true,
			name: source,
		});

		pagesStaged();
		nodeassert.deepEqual(sent, [insertOf(2) + ' link=1' + time + presentation]);
	});

	it('asks for the same whole insert again when the document refused it', function () {
		sourceShown();
		session.insertWhole(source);
		pagesStaged();
		deliver('slideimporterror', { kind: 'failure' });

		nodeassert.equal(session.state, 'ready');
		nodeassert.equal(session.error.message, 'Inserting the slides failed.');

		sent = [];
		session.error.retry();
		nodeassert.deepEqual(session.getPendingInsert(), {
			slides: [],
			at: 2,
			whole: true,
			name: source,
		});
		pagesStaged();
		nodeassert.deepEqual(sent, [insertOf(2) + ' link=1' + time + presentation]);
	});

	it('is ready for the next insert once the presentation is in', function () {
		sourceShown();
		session.insertWhole(source);
		pagesStaged();
		fired = [];
		deliver('slideimport', { message: { status: 'inserted', count: 3 } });

		nodeassert.equal(session.state, 'ready');
		nodeassert.equal(session.getPendingInsert(), null);
		nodeassert.ok(fired.includes('slideimport:inserted'));
	});

	it('inserts a pick as slides', function () {
		sourceShown();
		session.setSelection([2, 0]);
		session.insertSelected();

		nodeassert.deepEqual(session.getPendingInsert(), {
			slides: [0, 2],
			at: 2,
			whole: false,
			name: '',
		});
		pagesStaged();
		nodeassert.deepEqual(sent, [insertOf(2) + ' link=0 as=slides']);

		// The pick is in, so it is done with.
		deliver('slideimport', { message: { status: 'inserted', count: 2 } });
		nodeassert.equal(session.selection.size, 0);
	});

	it('inserts a pick linked to the places its slides hold in the source', function () {
		sourceShown();
		session.setLinkToSource(true);
		session.setLinkByPosition(true);
		session.setSelection([0, 2]);
		session.insertSelected();

		pagesStaged();
		nodeassert.deepEqual(sent, [
			insertOf(2) + ' link=1 sourcepositions=1,3' + time + ' as=slides',
		]);
	});

	it('inserts a pick as the section of the source it makes up', function () {
		sourceShown();
		session.setLinkToSource(true);
		session.setSelection([1, 2]);
		session.insertSelected();

		pagesStaged(opening);
		nodeassert.deepEqual(sent, [
			insertOf(2) +
				' link=1' +
				time +
				' as=section id=' +
				encodeURIComponent(opening.id) +
				' name=Opening',
		]);
	});

	it('asks for the same pick again when the source could not be read', function () {
		sourceShown();
		session.setSelection([0, 2]);
		session.insertSelected();
		session.exportFailed();

		fired = [];
		session.error.retry();
		nodeassert.ok(fired.includes('slideimport:exportwanted'));
		nodeassert.deepEqual(session.getPendingInsert(), {
			slides: [0, 2],
			at: 2,
			whole: false,
			name: '',
		});
	});
});
