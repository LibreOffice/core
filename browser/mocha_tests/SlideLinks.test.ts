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

describe('SlideLinks', function () {
	// The messages of the server the class listens for, by name, so a test
	// can hand it one as the socket would.
	let listeners: Map<string, (e: any) => void>;
	// What the class sent, said and asked the host for.
	let sent: string[];
	let told: string[];
	let posted: any[];
	// What the class told the menubar and the uiManager about the update
	// command. hidden starts true and commandShown starts empty, so a test
	// that reads them shown proves the show was really called.
	let hidden: boolean;
	let commandShown: Map<string, boolean>;
	let editable: boolean;
	let links: SlideLinks;
	let savedSocket: any;
	let savedEvents: any;
	let savedRelated: any;

	// The linked pages, by the identifier each one holds.
	const numbersPart = '{6A1C31B0-0001-4C5B-9E77-2F3D4A5B6C7D}';
	const outlookPart = '{6A1C31B0-0002-4C5B-9E77-2F3D4A5B6C7D}';
	const ticketsPart = '{6A1C31B0-0003-4C5B-9E77-2F3D4A5B6C7D}';
	// A page this document does not hold.
	const unknownPart = '{6A1C31B0-0099-4C5B-9E77-2F3D4A5B6C7D}';

	const list = {
		links: [
			{
				source: 'Sales deck.odp',
				slides: [
					{ part: numbersPart, name: 'Numbers' },
					{ part: outlookPart, name: 'Outlook' },
				],
			},
			{
				source: 'Support deck.odp',
				slides: [{ part: ticketsPart, name: 'Tickets' }],
			},
		],
	};

	function deliver(name: string, message: any): void {
		const listener = listeners.get(name);
		nodeassert.ok(listener, 'nothing listens for ' + name);
		listener(message);
	}

	// The document reports the links above, and what it did about them is
	// dropped, so that what follows is read on its own.
	function documentHoldsLinks(): void {
		deliver('slidelinks', { message: list });
		sent = [];
		told = [];
		posted = [];
	}

	// The related documents the storage named, one per source of the list above, each already
	// connected so that a refresh asks it straight away.
	function relatedDocuments(state: string = 'connected'): any[] {
		return [
			{ wopiSrc: wopiSrcOf('Sales deck.odp'), state: state },
			{ wopiSrc: wopiSrcOf('Support deck.odp'), state: state },
		];
	}

	function wopiSrcOf(source: string): string {
		return 'https://host/wopi/files/' + encodeURIComponent(source);
	}

	// The command that asks one source to write its pages out.
	function exportOf(source: string): string {
		return (
			'remotedoccommand wopisrc=' +
			encodeURIComponent(wopiSrcOf(source)) +
			' exportslides'
		);
	}

	// The command that has the document read the pages of one source from a staged file.
	function updateOf(source: string, staged: string): string {
		return (
			'slidelink update source=' +
			encodeURIComponent(source) +
			' file=' +
			encodeURIComponent(staged)
		);
	}

	// The server stages the pages a source wrote and names the file.
	function pagesStaged(staged: string): void {
		deliver('remotedoccommandresult', {
			textMsg: 'exportslides: {"status":"staged","name":"' + staged + '"}',
		});
	}

	// What the document made of the pages it was given.
	function refreshEnded(source: string, count: number): void {
		deliver('slidelink', {
			message: { status: 'updated', source: source, count: count },
		});
	}

	beforeEach(function () {
		listeners = new Map();
		sent = [];
		told = [];
		posted = [];
		hidden = true;
		commandShown = new Map();
		editable = true;

		const map: any = {
			on: function (name: string, callback: any, context: any) {
				listeners.set(name, callback.bind(context));
			},
			fire: function (name: string, args: any) {
				if (name === 'postMessage') posted.push(args);
			},
			isEditMode: () => editable,
			isPresentationOrDrawing: () => true,
			uiManager: {
				showSnackbar: (message: string) => told.push(message),
				showCommand: (command: string, show: boolean) => {
					commandShown.set(command, show);
				},
			},
			menubar: {
				showItem: () => {
					hidden = false;
				},
				hideItem: () => {
					hidden = true;
				},
			},
		};

		savedSocket = app.socket;
		savedEvents = app.events;
		savedRelated = (app as any).relatedDocuments;
		(app as any).relatedDocuments = relatedDocuments();
		(app as any).socket = {
			sendMessage: (message: string) => sent.push(message),
		};
		(app as any).events = {
			on: () => {},
			fire: () => {},
		};

		links = new SlideLinks(map);
	});

	afterEach(function () {
		(app as any).socket = savedSocket;
		(app as any).events = savedEvents;
		(app as any).relatedDocuments = savedRelated;
	});

	it('reads the links of the document once it is open', function () {
		deliver('docloaded', { status: true });
		// Every status update of the open document fires docloaded again,
		// and the list is read once all the same.
		deliver('docloaded', { status: true });
		nodeassert.deepEqual(sent, ['slidelink list']);
	});

	it('reads the links again once slides have been inserted', function () {
		documentHoldsLinks();
		deliver('slideimport', { message: { status: 'inserted', count: 2 } });
		nodeassert.deepEqual(sent, ['slidelink list']);

		// An import that opened or closed a file changed no page of this
		// document.
		sent = [];
		deliver('slideimport', { message: { status: 'open' } });
		deliver('slideimport', { message: { status: 'closed' } });
		nodeassert.deepEqual(sent, []);
	});

	it('reports which source and slide a page is linked to', function () {
		deliver('slidelinks', { message: list });

		nodeassert.equal(links.hasLinks(), true);
		nodeassert.deepEqual(links.getPageLink(outlookPart), {
			source: 'Sales deck.odp',
			name: 'Outlook',
		});
		nodeassert.deepEqual(links.getPageLink(ticketsPart), {
			source: 'Support deck.odp',
			name: 'Tickets',
		});
		// A page that is linked to nothing.
		nodeassert.equal(links.getPageLink(unknownPart), null);
	});

	it('marks a page as broken when its source is missing from storage', function () {
		(app as any).relatedDocuments = [
			{ wopiSrc: wopiSrcOf('Sales deck.odp'), state: 'missing' },
			{ wopiSrc: wopiSrcOf('Support deck.odp'), state: 'connected' },
		];
		deliver('slidelinks', { message: list });

		// Both pages read from the missing source are broken.
		nodeassert.equal(links.isPageBroken(numbersPart), true);
		nodeassert.equal(links.isPageBroken(outlookPart), true);
		// A page whose source is present is not.
		nodeassert.equal(links.isPageBroken(ticketsPart), false);
		// A page that is linked to nothing is not.
		nodeassert.equal(links.isPageBroken(unknownPart), false);
	});

	it('stops marking pages broken once the source is read again', function () {
		(app as any).relatedDocuments = [
			{ wopiSrc: wopiSrcOf('Sales deck.odp'), state: 'missing' },
			{ wopiSrc: wopiSrcOf('Support deck.odp'), state: 'connected' },
		];
		deliver('slidelinks', { message: list });
		nodeassert.equal(links.isPageBroken(numbersPart), true);

		// The source is subscribed again and connects, so its links are whole.
		(app as any).relatedDocuments = relatedDocuments();
		deliver('relateddocuments', { documents: (app as any).relatedDocuments });
		nodeassert.equal(links.isPageBroken(numbersPart), false);
	});

	it('offers the update command for a document that holds links alone', function () {
		deliver('slidelinks', { message: list });
		nodeassert.equal(commandShown.get('updateslidelinks'), true);
		nodeassert.equal(hidden, false);

		deliver('slidelinks', { message: { links: [] } });
		nodeassert.equal(links.hasLinks(), false);
		nodeassert.equal(commandShown.get('updateslidelinks'), false);
		nodeassert.equal(hidden, true);

		// The command comes back when the document gains links again.
		deliver('slidelinks', { message: list });
		nodeassert.equal(commandShown.get('updateslidelinks'), true);
		nodeassert.equal(hidden, false);
	});

	it('asks nothing of the user to update the slides of a source', function () {
		documentHoldsLinks();
		links.updateAll();

		nodeassert.deepEqual(posted, []);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);
	});

	it('refreshes one source at a time, in the order they are listed', function () {
		documentHoldsLinks();
		links.updateAll();
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);

		// While that source is writing its pages the next one waits.
		pagesStaged('sourceslides-1.odp');
		nodeassert.deepEqual(sent, [
			exportOf('Sales deck.odp'),
			updateOf('Sales deck.odp', 'sourceslides-1.odp'),
		]);

		// The document has read them, and only now is the second source asked.
		refreshEnded('Sales deck.odp', 2);
		nodeassert.deepEqual(sent, [
			exportOf('Sales deck.odp'),
			updateOf('Sales deck.odp', 'sourceslides-1.odp'),
			exportOf('Support deck.odp'),
		]);
		nodeassert.deepEqual(told, ['2 slides updated from Sales deck.odp.']);
	});

	it('subscribes to a source nothing holds a link to yet', function () {
		(app as any).relatedDocuments = relatedDocuments('available');
		documentHoldsLinks();
		links.updateAll();

		// The source is asked for nothing until the link to it is live.
		nodeassert.deepEqual(sent, [
			'remotedocsubscribe wopisrc=' +
				encodeURIComponent(wopiSrcOf('Sales deck.odp')),
		]);

		(app as any).relatedDocuments = relatedDocuments();
		deliver('relateddocuments', { documents: (app as any).relatedDocuments });
		nodeassert.equal(sent[1], exportOf('Sales deck.odp'));
	});

	it('refreshes a source once however often the update is asked for', function () {
		documentHoldsLinks();
		links.updateAll();
		links.updateAll();

		pagesStaged('sourceslides-1.odp');
		refreshEnded('Sales deck.odp', 2);
		pagesStaged('sourceslides-2.odp');
		refreshEnded('Support deck.odp', 1);

		nodeassert.deepEqual(sent, [
			exportOf('Sales deck.odp'),
			updateOf('Sales deck.odp', 'sourceslides-1.odp'),
			exportOf('Support deck.odp'),
			updateOf('Support deck.odp', 'sourceslides-2.odp'),
		]);
	});

	it('says why a source wrote no pages, and carries on', function () {
		documentHoldsLinks();
		links.updateAll();
		deliver('remotedoccommandresult', {
			textMsg: 'exportslides: {"status":"failed"}',
		});

		nodeassert.equal(told.length, 1);
		nodeassert.ok(told[0].indexOf('Sales deck.odp') >= 0);
		nodeassert.deepEqual(sent, [
			exportOf('Sales deck.odp'),
			exportOf('Support deck.odp'),
		]);
	});

	it('says when the document is related to no source of that name', function () {
		(app as any).relatedDocuments = [];
		documentHoldsLinks();
		links.updateAll();

		nodeassert.ok(told[0].indexOf('not one of the documents') >= 0);
		nodeassert.deepEqual(sent, []);
	});

	it('goes on to the next source when the document refuses the pages', function () {
		documentHoldsLinks();
		links.updateAll();
		pagesStaged('sourceslides-1.odp');

		// The document answers the update with an error naming the source.
		deliver('slidelinkerror', { kind: 'failed', source: 'Sales deck.odp' });

		nodeassert.deepEqual(told, [
			'Updating the slides of Sales deck.odp failed.',
		]);
		nodeassert.equal(sent[sent.length - 1], exportOf('Support deck.odp'));
	});

	it('leaves the refresh in hand alone when another command fails', function () {
		documentHoldsLinks();
		links.updateAll();

		// A break of a page that is linked to nothing, and an answer naming a
		// source this one is not waiting for, are answered while the refresh in
		// hand is still out.
		deliver('slidelinkerror', { kind: 'notlinked', part: numbersPart });
		deliver('slidelinkerror', { kind: 'failed', source: 'Support deck.odp' });

		nodeassert.deepEqual(told, []);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);
	});

	it('says so rather than refreshing a document that holds no links', function () {
		deliver('slidelinks', { message: { links: [] } });
		sent = [];
		told = [];
		links.updateAll();

		nodeassert.deepEqual(sent, []);
		nodeassert.equal(told.length, 1);
	});

	it('updates nothing from a view that cannot edit', function () {
		documentHoldsLinks();
		editable = false;
		links.updateAll();

		nodeassert.deepEqual(sent, []);
		nodeassert.deepEqual(told, []);
	});
});
