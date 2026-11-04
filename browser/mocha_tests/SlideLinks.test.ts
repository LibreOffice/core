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
	// The questions the class put to the user before it went on.
	let confirmed: string[];
	// What the class told the menubar and the notebookbar about the update
	// command. hidden starts true and itemShown starts empty, so a test
	// that reads them shown proves the show was really called.
	let hidden: boolean;
	let itemShown: Map<string, boolean>;
	let editable: boolean;
	let links: SlideLinks;
	let map: any;
	let savedSocket: any;
	let savedEvents: any;
	let savedRemoteLinks: any;
	let savedImpress: any;

	// The linked pages, by the identifier each one holds.
	const numbersPart = '{6A1C31B0-0001-4C5B-9E77-2F3D4A5B6C7D}';
	const outlookPart = '{6A1C31B0-0002-4C5B-9E77-2F3D4A5B6C7D}';
	const ticketsPart = '{6A1C31B0-0003-4C5B-9E77-2F3D4A5B6C7D}';
	// A page this document does not hold.
	const unknownPart = '{6A1C31B0-0099-4C5B-9E77-2F3D4A5B6C7D}';
	// The slides of the sources those pages were made from.
	const numbersGuid = '{7B2D42C1-0001-4D6C-8F88-3A4E5B6C7D8E}';
	const outlookGuid = '{7B2D42C1-0002-4D6C-8F88-3A4E5B6C7D8E}';
	const ticketsGuid = '{7B2D42C1-0003-4D6C-8F88-3A4E5B6C7D8E}';

	// A slide of this document linked to one slide of a source, by the
	// identifier of that slide.
	function slideLink(
		part: string,
		source: string,
		sourceId: string,
		name: string,
		parent: string = '',
	): any {
		return {
			unit: 'slide',
			id: part,
			parent: parent,
			source: source,
			sourceId: sourceId,
			name: name,
			lastModifiedTime: '',
			slides: [part],
		};
	}

	const list = {
		links: [
			slideLink(numbersPart, 'Sales deck.odp', numbersGuid, 'Numbers'),
			slideLink(outlookPart, 'Sales deck.odp', outlookGuid, 'Outlook'),
			slideLink(ticketsPart, 'Support deck.odp', ticketsGuid, 'Tickets'),
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

	// The remote links the storage named, one per source of the list above, each already
	// connected so that a refresh asks it straight away.
	function remoteLinks(state: string = 'connected'): any[] {
		return [linkOf('Sales deck.odp', state), linkOf('Support deck.odp', state)];
	}

	function linkOf(source: string, state: string): any {
		return { name: source, persistentLink: source, state: state };
	}

	// The command that asks one source to write its pages out: the slides of
	// the given identifiers, or its whole deck when none are given.
	function exportOf(source: string, guids?: string[]): string {
		return (
			'remotedoccommand source=' +
			encodeURIComponent(source) +
			' exportslides' +
			(guids ? ' guids=' + guids.join(',') : '')
		);
	}

	// The exports of the two sources of the list above, each asked for the
	// slides its pages were made from.
	const salesExport = exportOf('Sales deck.odp', [numbersGuid, outlookGuid]);
	const supportExport = exportOf('Support deck.odp', [ticketsGuid]);

	// The command that asks one source for the slides it holds.
	function infoOf(source: string): string {
		return (
			'remotedoccommand source=' +
			encodeURIComponent(source) +
			' getpresentationinfo'
		);
	}

	// The command that has the document read the links of one source from a
	// staged file, or the one link it names.
	function updateOf(
		source: string,
		staged: string,
		unit?: string,
		id?: string,
	): string {
		return (
			'slidelink update source=' +
			encodeURIComponent(source) +
			' file=' +
			encodeURIComponent(staged) +
			(unit ? ' unit=' + unit + ' id=' + encodeURIComponent(id) : '')
		);
	}

	// A section of this document linked to the section "Opening" of the sales
	// deck, the identifier that section has in the source, and the two slides
	// of the section here: the first still linked to its source slide, the
	// second edited here, so it holds no link of its own.
	const openingSection = '{4B1D2C3E-0001-4F00-8A00-1234567890AB}';
	const sourceOpening = '{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}';
	const openingFirst = '{6A1C31B0-0011-4C5B-9E77-2F3D4A5B6C7D}';
	const openingSecond = '{6A1C31B0-0012-4C5B-9E77-2F3D4A5B6C7D}';

	function sectionLink(section: string, slides: string[]): any {
		return {
			unit: 'section',
			id: section,
			parent: '',
			source: 'Sales deck.odp',
			sourceId: sourceOpening,
			name: 'Opening',
			lastModifiedTime: '',
			slides: slides,
		};
	}

	const sectionList = {
		links: [
			sectionLink(openingSection, [openingFirst, openingSecond]),
			slideLink(openingFirst, 'Sales deck.odp', 'g1', '', openingSection),
		],
	};

	// The whole sales deck linked into this document: one link over the
	// section that holds the deck's slide outside any section, named after the
	// deck and naming no section of it, and the section "Opening" under it.
	const deckPresentation = '{4B1D2C3E-0010-4F00-8A00-1234567890AB}';
	const looseSection = '{4B1D2C3E-0011-4F00-8A00-1234567890AB}';
	const loosePart = '{6A1C31B0-0021-4C5B-9E77-2F3D4A5B6C7D}';

	const presentationList = {
		links: [
			{
				unit: 'presentation',
				id: deckPresentation,
				parent: '',
				source: 'Sales deck.odp',
				sourceId: '',
				name: 'Sales deck.odp',
				lastModifiedTime: '',
				slides: [loosePart, openingFirst, openingSecond],
			},
			{
				unit: 'section',
				id: looseSection,
				parent: deckPresentation,
				source: 'Sales deck.odp',
				sourceId: '',
				name: 'Sales deck.odp',
				lastModifiedTime: '',
				slides: [loosePart],
			},
			{
				...sectionLink(openingSection, [openingFirst, openingSecond]),
				parent: deckPresentation,
			},
			slideLink(openingFirst, 'Sales deck.odp', 'g1', '', openingSection),
		],
	};

	// The source answers what it holds: three slides, the last two in the
	// section "Opening" under the name given, unless the source lost it.
	function sourceInfo(source: string, sectionName: string | null): void {
		const sections = [
			{
				name: 'Intro',
				id: '{AAAAAAAA-0000-4000-8000-000000000000}',
				startIndex: 0,
				slideCount: 1,
			},
		];
		if (sectionName !== null)
			sections.push({
				name: sectionName,
				id: sourceOpening,
				startIndex: 1,
				slideCount: 2,
			});
		deliver('remotedoccommandresult', {
			source: source,
			textMsg:
				'presentationinfo: ' +
				JSON.stringify({
					slides: [
						{ index: 0, part: 'p0', guid: numbersGuid },
						{ index: 1, part: 'p1', guid: 'g1' },
						{ index: 2, part: 'p2', guid: 'g2' },
					],
					sections: sections,
				}),
		});
	}

	// The server stages the pages a source wrote and names the file.
	function pagesStaged(staged: string): void {
		deliver('remotedoccommandresult', {
			textMsg: 'exportslides: {"status":"staged","name":"' + staged + '"}',
		});
	}

	// What the document made of the pages it was given: the answer names the
	// unit the update named, and none for an update of a whole source.
	function refreshEnded(
		source: string,
		count: number,
		unit?: string,
		id?: string,
		notUpdated: string[] = [],
	): void {
		const message: any = {
			status: 'updated',
			source: source,
			count: count,
			notUpdated: notUpdated,
		};
		if (unit) {
			message.unit = unit;
			message.id = id;
		}
		deliver('slidelink', { message: message });
	}

	beforeEach(function () {
		listeners = new Map();
		sent = [];
		told = [];
		posted = [];
		confirmed = [];
		hidden = true;
		itemShown = new Map();
		editable = true;

		map = {
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
				// The user agrees to every question.
				showConfirmModal: (
					id: string,
					title: string,
					message: string,
					button: string,
					callback: () => void,
				) => {
					confirmed.push(message);
					callback();
				},
				notebookbar: {
					showItem: (id: string) => {
						itemShown.set(id, true);
					},
					hideItem: (id: string) => {
						itemShown.set(id, false);
					},
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
		savedRemoteLinks = (app as any).remoteLinks;
		savedImpress = (app as any).impress;
		(app as any).remoteLinks = remoteLinks();
		(app as any).socket = {
			sendMessage: (message: string) => sent.push(message),
		};
		(app as any).events = {
			on: () => {},
			fire: () => {},
		};
		// The slides of this document, in order, so a part can be named by
		// its number.
		(app as any).impress = {
			getIndexFromPart: (part: string) =>
				[numbersPart, outlookPart, ticketsPart].indexOf(part),
		};

		links = new SlideLinks(map);
	});

	afterEach(function () {
		(app as any).socket = savedSocket;
		(app as any).events = savedEvents;
		(app as any).remoteLinks = savedRemoteLinks;
		(app as any).impress = savedImpress;
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
		nodeassert.deepEqual(links.getLink(outlookPart), {
			unit: 'slide',
			id: outlookPart,
			parent: '',
			source: 'Sales deck.odp',
			sourceId: outlookGuid,
			name: 'Outlook',
			lastModifiedTime: '',
			slides: [outlookPart],
		});
		nodeassert.equal(links.getLink(ticketsPart).source, 'Support deck.odp');
		// A page that is linked to nothing.
		nodeassert.equal(links.getLink(unknownPart), null);
		nodeassert.deepEqual(links.linksOf(unknownPart), []);
	});

	it('leaves out an entry it cannot read', function () {
		deliver('slidelinks', {
			message: {
				links: [
					list.links[0],
					// No identifier, a unit of no known kind, and no slides.
					{ unit: 'slide', source: 'Sales deck.odp', slides: [outlookPart] },
					{ ...list.links[1], unit: 'page' },
					{ ...list.links[2], slides: 'all' },
				],
			},
		});
		nodeassert.ok(links.getLink(numbersPart));
		nodeassert.equal(links.getLink(outlookPart), null);
		nodeassert.equal(links.getLink(ticketsPart), null);
	});

	it('lists the links over a slide, innermost first', function () {
		deliver('slidelinks', { message: sectionList });

		// The first slide holds a link of its own under the section's.
		nodeassert.deepEqual(
			links.linksOf(openingFirst).map((link) => link.unit),
			['slide', 'section'],
		);
		nodeassert.equal(links.linksOf(openingFirst)[1].id, openingSection);
		// The second slide was edited here, so only the section holds it.
		nodeassert.deepEqual(
			links.linksOf(openingSecond).map((link) => link.id),
			[openingSection],
		);
	});

	it('counts the slides linked to a source, through their section too', function () {
		deliver('slidelinks', { message: sectionList });
		nodeassert.equal(links.countSlidesFrom('Sales deck.odp'), 2);
		nodeassert.equal(links.countSlidesFrom('Support deck.odp'), 0);
		nodeassert.equal(links.countOutdatedSlidesFrom('Sales deck.odp'), 0);

		// The section was read at one time, and the source has moved on since.
		const outdated = {
			links: [
				{ ...sectionList.links[0], lastModifiedTime: '2026-11-01T10:00:00Z' },
				sectionList.links[1],
			],
		};
		(app as any).remoteLinks = [
			{
				...linkOf('Sales deck.odp', 'connected'),
				lastModifiedTime: '2026-11-03T10:00:00Z',
			},
		];
		deliver('slidelinks', { message: outdated });
		nodeassert.equal(links.isOutdated(openingSection), true);
		nodeassert.equal(links.isOutdated(openingFirst), false);
		nodeassert.equal(links.countOutdatedSlidesFrom('Sales deck.odp'), 2);
	});

	it('marks a page as broken when its source is missing from storage', function () {
		(app as any).remoteLinks = [
			linkOf('Sales deck.odp', 'missing'),
			linkOf('Support deck.odp', 'connected'),
		];
		deliver('slidelinks', { message: list });

		// Both pages read from the missing source are broken.
		nodeassert.equal(links.getLinkState(numbersPart), 'broken');
		nodeassert.equal(links.getLinkState(outlookPart), 'broken');
		// A page whose source is present is not.
		nodeassert.equal(links.getLinkState(ticketsPart), 'connected');
		// A page that is linked to nothing has no state.
		nodeassert.equal(links.getLinkState(unknownPart), '');
	});

	it('stops marking pages broken once the source is read again', function () {
		(app as any).remoteLinks = [
			linkOf('Sales deck.odp', 'missing'),
			linkOf('Support deck.odp', 'connected'),
		];
		deliver('slidelinks', { message: list });
		nodeassert.equal(links.getLinkState(numbersPart), 'broken');

		// The source is subscribed again and connects, so its links are whole.
		(app as any).remoteLinks = remoteLinks();
		deliver('remotelinks', { documents: (app as any).remoteLinks });
		nodeassert.equal(links.getLinkState(numbersPart), 'connected');
	});

	it('marks a page as connected when its source is held open', function () {
		(app as any).remoteLinks = [
			linkOf('Sales deck.odp', 'connected'),
			linkOf('Support deck.odp', 'available'),
		];
		deliver('slidelinks', { message: list });

		// The pages read from the connected source are connected.
		nodeassert.equal(links.getLinkState(numbersPart), 'connected');
		nodeassert.equal(links.getLinkState(outlookPart), 'connected');
		// A page whose source is only available, not held open, is linked
		// and no more.
		nodeassert.equal(links.getLinkState(ticketsPart), 'linked');
	});

	it('marks a page as broken once its source no longer holds its slide', function () {
		deliver('slidelinks', { message: list });
		nodeassert.equal(links.isMissing(outlookPart), false);

		// The source reports the slides it holds, and the one the outlook
		// page came from is not among them.
		sourceInfo('Sales deck.odp', 'Opening');
		nodeassert.equal(links.isMissing(numbersPart), false);
		nodeassert.equal(links.isMissing(outlookPart), true);
		nodeassert.equal(links.getLinkState(outlookPart), 'broken');
		nodeassert.equal(links.isUpdatable(outlookPart), false);
		nodeassert.equal(links.isUpdatable(numbersPart), true);

		// Such a page is not read again, and the user is told why.
		sent = [];
		links.update(outlookPart);
		nodeassert.deepEqual(sent, []);
		nodeassert.equal(told.length, 1);
		nodeassert.ok(told[0].indexOf('no longer holds') >= 0);
	});

	it('offers the update command for a document that holds links alone', function () {
		deliver('slidelinks', { message: list });
		nodeassert.equal(itemShown.get('insert-update-slide-links'), true);
		nodeassert.equal(hidden, false);

		deliver('slidelinks', { message: { links: [] } });
		nodeassert.equal(links.hasLinks(), false);
		nodeassert.equal(itemShown.get('insert-update-slide-links'), false);
		nodeassert.equal(hidden, true);

		// The command comes back when the document gains links again.
		deliver('slidelinks', { message: list });
		nodeassert.equal(itemShown.get('insert-update-slide-links'), true);
		nodeassert.equal(hidden, false);
	});

	it('asks nothing of the user to update the slides of a source', function () {
		documentHoldsLinks();
		links.updateAll();

		nodeassert.deepEqual(posted, []);
		nodeassert.deepEqual(confirmed, []);
		nodeassert.deepEqual(sent, [salesExport]);
	});

	it('refreshes one source at a time, in the order they are listed', function () {
		documentHoldsLinks();
		links.updateAll();
		nodeassert.deepEqual(sent, [salesExport]);

		// While that source is writing its pages the next one waits.
		pagesStaged('sourceslides-1.odp');
		nodeassert.deepEqual(sent, [
			salesExport,
			updateOf('Sales deck.odp', 'sourceslides-1.odp'),
		]);

		// The document has read them, and only now is the second source asked.
		refreshEnded('Sales deck.odp', 2);
		nodeassert.deepEqual(sent, [
			salesExport,
			updateOf('Sales deck.odp', 'sourceslides-1.odp'),
			supportExport,
		]);
		nodeassert.deepEqual(told, ['2 slides updated from Sales deck.odp.']);
	});

	it('subscribes to a source nothing holds a link to yet', function () {
		(app as any).remoteLinks = remoteLinks('available');
		documentHoldsLinks();
		links.updateAll();

		// The source is asked for nothing until the link to it is live.
		nodeassert.deepEqual(sent, [
			'remotedocsubscribe source=' + encodeURIComponent('Sales deck.odp'),
		]);

		(app as any).remoteLinks = remoteLinks();
		deliver('remotelinks', { documents: (app as any).remoteLinks });

		// Every source that came up is asked for the slides it holds, and the one
		// the refresh in hand names for its pages.
		nodeassert.deepEqual(sent.slice(1), [
			infoOf('Sales deck.odp'),
			infoOf('Support deck.odp'),
			salesExport,
		]);
	});

	it('carries on to the next source when one it waited on cannot be opened', function () {
		(app as any).remoteLinks = remoteLinks('available');
		documentHoldsLinks();
		links.updateAll();

		nodeassert.deepEqual(sent, [
			'remotedocsubscribe source=' + encodeURIComponent('Sales deck.odp'),
		]);

		// The source the refresh waits on could not be opened, so it comes up
		// in no state a refresh can read.
		(app as any).remoteLinks = [
			linkOf('Sales deck.odp', 'failed'),
			linkOf('Support deck.odp', 'connected'),
		];
		deliver('remotelinks', { documents: (app as any).remoteLinks });

		// The user is told why that source was left, and the source behind it
		// in the queue is refreshed.
		nodeassert.equal(told.length, 1);
		nodeassert.ok(told[0].indexOf('could not be read') >= 0);
		nodeassert.deepEqual(sent.slice(1), [
			infoOf('Support deck.odp'),
			supportExport,
		]);
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
			salesExport,
			updateOf('Sales deck.odp', 'sourceslides-1.odp'),
			supportExport,
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
		nodeassert.deepEqual(sent, [salesExport, supportExport]);
	});

	it('says so rather than waiting on a source that cannot be read', function () {
		(app as any).remoteLinks = [
			linkOf('Sales deck.odp', 'missing'),
			linkOf('Support deck.odp', 'connected'),
		];
		documentHoldsLinks();
		links.updateAll();

		// A source the storage could not give never comes up, so it is not
		// waited on, and the next source is asked straight away.
		nodeassert.equal(told.length, 1);
		nodeassert.ok(told[0].indexOf('could not be read') >= 0);
		nodeassert.deepEqual(sent, [supportExport]);
	});

	it('says when the document is related to no source of that name', function () {
		(app as any).remoteLinks = [];
		documentHoldsLinks();
		links.updateAll();

		nodeassert.ok(told[0].indexOf('not one of the documents') >= 0);
		nodeassert.deepEqual(sent, []);
	});

	it('marks a page as broken when no remote link stands for its source', function () {
		// The server reports the source by the persistent link the slides of this document
		// record, as missing, because the storage listed no remote link for it.
		(app as any).remoteLinks = [
			{
				name: 'Sales deck.odp',
				persistentLink: 'Sales deck.odp',
				state: 'missing',
			},
			linkOf('Support deck.odp', 'connected'),
		];
		documentHoldsLinks();

		nodeassert.equal(links.getLinkState(numbersPart), 'broken');
		nodeassert.equal(links.getLinkState(ticketsPart), 'connected');

		// Such a source is asked for nothing, and the user is told why.
		links.updateAll();
		nodeassert.ok(told[0].indexOf('could not be read') >= 0);
		nodeassert.deepEqual(sent, [supportExport]);
	});

	it('names a source by its persistent link whatever the storage calls it', function () {
		// The storage may call the file anything, so the persistent link is the only thing
		// that says which source the slides of this document came from.
		(app as any).remoteLinks = [
			{
				name: 'Q3 sales.odp',
				persistentLink: 'Sales deck.odp',
				state: 'connected',
			},
		];
		documentHoldsLinks();
		links.updateSource('Sales deck.odp');

		nodeassert.deepEqual(sent, [salesExport]);
	});

	it('drops a refresh the connection took with it', function () {
		documentHoldsLinks();
		links.updateAll();
		nodeassert.equal(sent.length, 1);

		// The connection goes down before the source answered, and comes back.
		// The reloaded document drops the refresh in flight rather than waiting
		// forever behind one nothing will answer.
		deliver('docloaded', { status: false });
		sent = [];
		deliver('docloaded', { status: true });
		nodeassert.deepEqual(sent, ['slidelink list']);

		// The document reads its links again, and an update starts over from
		// the first source.
		deliver('slidelinks', { message: list });
		sent = [];
		links.updateAll();
		nodeassert.deepEqual(sent, [salesExport]);
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
		nodeassert.equal(sent[sent.length - 1], supportExport);
	});

	it('leaves the refresh in hand alone when another command fails', function () {
		documentHoldsLinks();
		links.updateAll();

		// A break of a page that is linked to nothing, and an answer naming a
		// source this one is not waiting for, are answered while the refresh in
		// hand is still out.
		deliver('slidelinkerror', {
			kind: 'notlinked',
			unit: 'slide',
			id: numbersPart,
		});
		deliver('slidelinkerror', { kind: 'failed', source: 'Support deck.odp' });

		nodeassert.deepEqual(told, []);
		nodeassert.deepEqual(sent, [salesExport]);
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
		links.update(numbersPart);
		links.breakLink(numbersPart);

		nodeassert.deepEqual(sent, []);
		nodeassert.deepEqual(told, []);
	});

	it('reads one page again and leaves the rest of its source alone', function () {
		documentHoldsLinks();
		links.update(outlookPart);

		// The source writes the one slide out, and the update names the one
		// page the document reads from it.
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp', [outlookGuid])]);
		pagesStaged('sourceslides-1.odp');
		nodeassert.equal(
			sent[sent.length - 1],
			updateOf('Sales deck.odp', 'sourceslides-1.odp', 'slide', outlookPart),
		);

		// The answer of an update of the whole source is not the answer of
		// this one.
		refreshEnded('Sales deck.odp', 2);
		nodeassert.deepEqual(told, []);
		refreshEnded('Sales deck.odp', 1, 'slide', outlookPart);
		nodeassert.deepEqual(told, ['1 slide updated from Sales deck.odp.']);
	});

	it('leaves the answers of another document alone', function () {
		documentHoldsLinks();
		links.update(outlookPart);

		// The import pane may be reading a document of its own at the same time.
		deliver('remotedoccommandresult', {
			source: 'Support deck.odp',
			textMsg: 'exportslides: {"status":"staged","name":"other.odp"}',
		});
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp', [outlookGuid])]);
	});

	it('takes a page that left the list before its turn as up to date', function () {
		documentHoldsLinks();
		links.update(outlookPart);
		links.update(ticketsPart);
		pagesStaged('sourceslides-1.odp');

		// Another view refreshed or unlinked the page meanwhile, so the document
		// knows no page of that part any more. Nothing is wrong, and the next
		// refresh follows.
		deliver('slidelinkerror', {
			kind: 'notlinked',
			source: 'Sales deck.odp',
			unit: 'slide',
			id: outlookPart,
		});
		nodeassert.deepEqual(told, []);
		nodeassert.equal(
			sent[sent.length - 1],
			exportOf('Support deck.odp', [ticketsGuid]),
		);
	});

	it('reads the whole source over pages of it waiting on their own', function () {
		documentHoldsLinks();
		links.update(numbersPart);
		links.update(outlookPart);
		links.updateAll();

		pagesStaged('sourceslides-1.odp');
		refreshEnded('Sales deck.odp', 1, 'slide', numbersPart);
		pagesStaged('sourceslides-2.odp');
		refreshEnded('Sales deck.odp', 2);

		// The page in hand is read on its own, the one waiting is read with the
		// rest of its source, and then the other source follows.
		nodeassert.deepEqual(sent, [
			exportOf('Sales deck.odp', [numbersGuid]),
			updateOf('Sales deck.odp', 'sourceslides-1.odp', 'slide', numbersPart),
			salesExport,
			updateOf('Sales deck.odp', 'sourceslides-2.odp'),
			supportExport,
		]);
	});

	it('announces a list naming new parts once the part list holds them', function () {
		const fired: string[] = [];
		(app as any).events = {
			on: () => {},
			fire: (name: string) => fired.push(name),
		};
		// The view knows the parts of the list above; a refreshed page holds a
		// part it does not know until the next status message.
		let known = [numbersPart, outlookPart, ticketsPart];
		map._docLayer = {
			getIndexFromPart: (part: string) => (known.indexOf(part) >= 0 ? 0 : -1),
		};

		deliver('slidelinks', { message: list });
		nodeassert.deepEqual(fired, ['slidelink:changed']);

		deliver('slidelinks', {
			message: {
				links: [
					slideLink(unknownPart, 'Sales deck.odp', numbersGuid, 'Numbers'),
				],
			},
		});
		nodeassert.deepEqual(fired, ['slidelink:changed']);

		known = [unknownPart];
		deliver('updateparts', {});
		nodeassert.deepEqual(fired, ['slidelink:changed', 'slidelink:changed']);
		// The status that follows an ordinary change announces nothing more.
		deliver('updateparts', {});
		nodeassert.equal(fired.length, 2);
	});

	it('reads a page once when its source is already being refreshed', function () {
		documentHoldsLinks();
		links.updateAll();
		links.update(numbersPart);
		links.update(numbersPart);

		nodeassert.deepEqual(sent, [salesExport]);
	});

	it('says so rather than reading a page that is linked to nothing', function () {
		documentHoldsLinks();
		links.update(unknownPart);

		nodeassert.deepEqual(sent, []);
		nodeassert.equal(told.length, 1);
	});

	it('names the slides an update left as they were', function () {
		documentHoldsLinks();
		links.updateAll();
		pagesStaged('sourceslides-1.odp');

		// The file held nothing for the second page of the document.
		refreshEnded('Sales deck.odp', 1, undefined, undefined, [outlookPart]);
		nodeassert.deepEqual(told, [
			'1 slide updated from Sales deck.odp.',
			'Cannot update slide 2: the source lacks matching slide',
		]);
	});

	it('takes the link off a slide', function () {
		documentHoldsLinks();
		links.breakLink(numbersPart);
		// A page that is linked to nothing has nothing to take off.
		links.breakLink(unknownPart);
		nodeassert.deepEqual(sent, [
			'slidelink break unit=slide id=' + encodeURIComponent(numbersPart),
		]);
	});

	it('says so when the source could not be taken off a page', function () {
		documentHoldsLinks();
		deliver('slidelinkerror', {
			kind: 'failed',
			unit: 'slide',
			id: numbersPart,
		});
		nodeassert.deepEqual(told, ['Unlinking the slide failed.']);
	});

	it('tells whether the source of a page can be read', function () {
		(app as any).remoteLinks = [linkOf('Sales deck.odp', 'noaccess')];
		deliver('slidelinks', { message: list });

		nodeassert.equal(links.getSourceState(numbersPart), 'noaccess');
		nodeassert.equal(links.isUpdatable(numbersPart), false);
		// The storage named no document for this source.
		nodeassert.equal(links.getSourceState(ticketsPart), '');
		nodeassert.equal(links.isUpdatable(ticketsPart), false);

		(app as any).remoteLinks = remoteLinks('available');
		nodeassert.equal(links.getSourceState(numbersPart), 'available');
		nodeassert.equal(links.isUpdatable(numbersPart), true);
		// A page that is linked to nothing has no source to speak of.
		nodeassert.equal(links.getSourceState(unknownPart), '');
	});

	it('asks the server to look for the file of one source again', function () {
		sent = [];
		SlideImportSession.resolveRemoteLink('Q3 #1 100%.odp');

		// The link is a whole word of the command, so it carries no space of
		// its own and no character a command line reads.
		nodeassert.deepEqual(sent, [
			'remotelinkresolve source=Q3%20%231%20100%25.odp',
		]);
	});

	it('reads a page that follows a position from the whole deck', function () {
		// The page reads whichever slide stands third in the source, so the
		// source is asked for every slide rather than for one by identifier.
		deliver('slidelinks', {
			message: {
				links: [slideLink(numbersPart, 'Sales deck.odp', 'Slide 3', 'Slide 3')],
			},
		});
		sent = [];
		links.updateAll();
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);
		// Such a page is never missing from its source.
		sourceInfo('Sales deck.odp', 'Opening');
		nodeassert.equal(links.isMissing(numbersPart), false);
		nodeassert.equal(links.getSourceSlideName(numbersPart), '');
	});

	it('names the slide a page came from as the source calls it now', function () {
		deliver('slidelinks', { message: list });
		nodeassert.equal(links.getSourceSlideName(numbersPart), '');

		// The source reports its slides, the first with no name of its own.
		sourceInfo('Sales deck.odp', 'Opening');
		nodeassert.equal(links.getSourceSlideName(numbersPart), 'Slide 1');
	});

	it('updates a linked section from the slides its source section holds now', function () {
		deliver('slidelinks', { message: sectionList });
		sent = [];

		// The document finds the section in the file by its identifier, so
		// the source writes its whole deck out, after the user agreed.
		links.update(openingSection);
		nodeassert.equal(confirmed.length, 1);
		nodeassert.ok(confirmed[0].indexOf('Sales deck.odp') >= 0);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);

		pagesStaged('staged.odp');
		nodeassert.equal(
			sent[1],
			updateOf('Sales deck.odp', 'staged.odp', 'section', openingSection),
		);

		// The answer for another section is not the answer of this update.
		refreshEnded(
			'Sales deck.odp',
			2,
			'section',
			'{4B1D2C3E-0002-4F00-8A00-1234567890AB}',
		);
		nodeassert.deepEqual(told, []);

		refreshEnded('Sales deck.odp', 2, 'section', openingSection);
		nodeassert.deepEqual(told, ['2 slides updated from Sales deck.odp.']);
	});

	it('asks nothing when the slides of a section go in with the source', function () {
		// Every link of the source under no other link is refreshed by one
		// update, the section among them, so the question is asked once for
		// the source. The other source holds slides alone, which need no
		// question and are read by identifier, so it goes first.
		deliver('slidelinks', {
			message: { links: [...sectionList.links, ...list.links] },
		});
		sent = [];

		links.updateAll();
		nodeassert.equal(confirmed.length, 1);
		nodeassert.ok(confirmed[0].indexOf('Sales deck.odp') >= 0);
		nodeassert.deepEqual(sent, [supportExport]);

		pagesStaged('support.odp');
		refreshEnded('Support deck.odp', 1);
		nodeassert.equal(sent[2], exportOf('Sales deck.odp'));
		pagesStaged('staged.odp');
		nodeassert.equal(sent[3], updateOf('Sales deck.odp', 'staged.odp'));
	});

	it('asks no second time about a section already waiting for its update', function () {
		deliver('slidelinks', {
			message: { links: [...sectionList.links, ...list.links] },
		});
		sent = [];

		// The user agreed to the section from its own menu, and asks for every
		// link while that update is still out. The source is read whole, with
		// no further question, since the one section of it is agreed to.
		links.update(openingSection);
		links.updateAll();
		nodeassert.equal(confirmed.length, 1);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);

		pagesStaged('section.odp');
		refreshEnded('Sales deck.odp', 2, 'section', openingSection);
		nodeassert.deepEqual(sent.slice(2), [exportOf('Sales deck.odp')]);
	});

	it('refuses to update a section its source no longer holds', function () {
		deliver('slidelinks', { message: sectionList });
		nodeassert.equal(links.isMissing(openingSection), false);
		sourceInfo('Sales deck.odp', null);
		sent = [];

		nodeassert.equal(links.isMissing(openingSection), true);
		nodeassert.equal(links.getLinkState(openingSection), 'broken');
		nodeassert.equal(links.isUpdatable(openingSection), false);
		links.update(openingSection);
		nodeassert.deepEqual(confirmed, []);
		nodeassert.deepEqual(sent, []);
		nodeassert.equal(told.length, 1);
		nodeassert.ok(told[0].indexOf('no longer holds') >= 0);

		// The section can be updated again once the source holds it again.
		sourceInfo('Sales deck.odp', 'Opening');
		links.update(openingSection);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);
	});

	it('takes the link off a section, and off its slides when asked', function () {
		deliver('slidelinks', { message: sectionList });
		sent = [];

		links.breakLink(openingSection, false);
		links.breakLink(openingSection, true);
		nodeassert.deepEqual(sent, [
			'slidelink break unit=section id=' + encodeURIComponent(openingSection),
			'slidelink break unit=section id=' +
				encodeURIComponent(openingSection) +
				' deep=1',
		]);

		deliver('slidelinkerror', {
			kind: 'failed',
			unit: 'section',
			id: openingSection,
		});
		nodeassert.deepEqual(told, ['Unlinking the section failed.']);
	});

	it('offers Update Links in a document whose only links are sections', function () {
		deliver('slidelinks', { message: sectionList });
		nodeassert.equal(links.hasLinks(), true);
		nodeassert.equal(hidden, false);
		nodeassert.equal(itemShown.get('insert-update-slide-links'), true);
	});

	it('updates every linked section from Update Links after one question', function () {
		const second = sectionLink('{4B1D2C3E-0002-4F00-8A00-1234567890AB}', [
			numbersPart,
		]);
		deliver('slidelinks', {
			message: { links: [sectionList.links[0], second] },
		});
		sent = [];

		links.updateAll();
		nodeassert.equal(confirmed.length, 1);
		nodeassert.ok(confirmed[0].indexOf('2 linked sections') >= 0);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);
	});

	it('says which sources Update Links leaves as they are', function () {
		(app as any).remoteLinks = [linkOf('Sales deck.odp', 'noaccess')];
		deliver('slidelinks', { message: sectionList });
		sent = [];

		links.updateAll();
		nodeassert.deepEqual(confirmed, []);
		nodeassert.deepEqual(sent, []);
		nodeassert.deepEqual(told, [
			'You do not have access to Sales deck.odp, so its slides cannot be updated.',
		]);
	});

	it('drops a section update the user did not agree to', function () {
		// The user answers every question with Cancel.
		map.uiManager.showConfirmModal = (
			id: string,
			title: string,
			message: string,
			button: string,
			callback: () => void,
			hideCancel: boolean,
			cancel: () => void,
		) => {
			confirmed.push(message);
			cancel();
		};
		deliver('slidelinks', { message: sectionList });
		sent = [];

		links.update(openingSection);
		nodeassert.equal(confirmed.length, 1);
		nodeassert.deepEqual(sent, []);
	});

	it('reads the slides linked on their own when the section update is declined', function () {
		map.uiManager.showConfirmModal = (
			id: string,
			title: string,
			message: string,
			button: string,
			callback: () => void,
			hideCancel: boolean,
			cancel: () => void,
		) => {
			confirmed.push(message);
			cancel();
		};
		deliver('slidelinks', {
			message: { links: [...sectionList.links, ...list.links] },
		});
		sent = [];

		// The section keeps its slides, and the two slides of the same source
		// that are linked on their own are read one by one, the slide of the
		// section not among them.
		links.updateSource('Sales deck.odp');
		nodeassert.equal(confirmed.length, 1);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp', [numbersGuid])]);
		pagesStaged('one.odp');
		refreshEnded('Sales deck.odp', 1, 'slide', numbersPart);
		nodeassert.deepEqual(sent.slice(2), [
			exportOf('Sales deck.odp', [outlookGuid]),
		]);
		pagesStaged('two.odp');
		refreshEnded('Sales deck.odp', 1, 'slide', outlookPart);
		nodeassert.equal(sent.length, 4);
	});

	it('lists the presentations of the document and names each by its file', function () {
		deliver('slidelinks', { message: presentationList });
		nodeassert.deepEqual(
			links.getPresentations().map((link) => link.id),
			[deckPresentation],
		);
		nodeassert.deepEqual(links.getLink(looseSection).parent, deckPresentation);

		// The storage names the file as it is called now while the source is
		// connected, and the link keeps the name it was read under otherwise.
		(app as any).remoteLinks = [
			{
				name: 'Sales deck 2027.odp',
				persistentLink: 'Sales deck.odp',
				state: 'connected',
			},
		];
		nodeassert.equal(
			links.getPresentationName(deckPresentation),
			'Sales deck 2027.odp',
		);
		(app as any).remoteLinks = remoteLinks('disconnected');
		nodeassert.equal(
			links.getPresentationName(deckPresentation),
			'Sales deck.odp',
		);
		// A section is named by nothing here, whatever its link records.
		nodeassert.equal(links.getPresentationName(openingSection), '');
		nodeassert.equal(links.getPresentationName(unknownPart), '');
	});

	it('keeps a presentation and the section of its loose slides while the source holds anything', function () {
		deliver('slidelinks', { message: presentationList });
		// The source lost its section "Opening". The presentation and the
		// section of the slides outside any section name nothing in the
		// source, so they hold as long as the source does.
		sourceInfo('Sales deck.odp', null);
		nodeassert.equal(links.isMissing(openingSection), true);
		nodeassert.equal(links.isMissing(looseSection), false);
		nodeassert.equal(links.isMissing(deckPresentation), false);
		nodeassert.equal(links.getLinkState(looseSection), 'connected');
		nodeassert.equal(links.getLinkState(deckPresentation), 'connected');
		nodeassert.equal(links.isUpdatable(deckPresentation), true);

		(app as any).remoteLinks = remoteLinks('missing');
		nodeassert.equal(links.getLinkState(looseSection), 'broken');
		nodeassert.equal(links.getLinkState(deckPresentation), 'broken');
		nodeassert.equal(links.isUpdatable(deckPresentation), false);
	});

	it('asks about the section of the loose slides in its own words', function () {
		deliver('slidelinks', { message: presentationList });
		sent = [];

		links.update(looseSection);
		nodeassert.equal(confirmed.length, 1);
		nodeassert.ok(confirmed[0].indexOf('outside its sections') >= 0);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);
	});

	it('updates a linked presentation after one question that names it', function () {
		deliver('slidelinks', { message: presentationList });
		sent = [];

		// Every slide of the presentation is replaced, so the user is asked
		// first, and the source writes its whole deck out.
		links.update(deckPresentation);
		nodeassert.equal(confirmed.length, 1);
		nodeassert.ok(confirmed[0].indexOf('linked presentation') >= 0);
		nodeassert.ok(confirmed[0].indexOf('Sales deck.odp') >= 0);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);

		pagesStaged('staged.odp');
		nodeassert.equal(
			sent[1],
			updateOf(
				'Sales deck.odp',
				'staged.odp',
				'presentation',
				deckPresentation,
			),
		);
		refreshEnded('Sales deck.odp', 3, 'presentation', deckPresentation);
		nodeassert.deepEqual(told, ['3 slides updated from Sales deck.odp.']);
	});

	it('asks once for a presentation, its sections included', function () {
		// The sections under the presentation are replaced along with it, so
		// the question counts the presentation alone.
		deliver('slidelinks', { message: presentationList });
		sent = [];
		links.updateAll();
		nodeassert.equal(confirmed.length, 1);
		nodeassert.ok(confirmed[0].indexOf('The linked presentation') >= 0);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);
	});

	it('counts the sections and presentations one update replaces', function () {
		// A section of the same source linked on its own goes with the
		// presentation in one refresh of the source, and the question counts
		// both.
		const alone = sectionLink('{4B1D2C3E-0002-4F00-8A00-1234567890AB}', [
			numbersPart,
		]);
		deliver('slidelinks', {
			message: { links: [...presentationList.links, alone] },
		});
		sent = [];
		links.updateAll();
		nodeassert.equal(confirmed.length, 1);
		nodeassert.ok(
			confirmed[0].indexOf('2 linked sections and presentations') >= 0,
		);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp')]);
	});

	it('reads the slides linked on their own when the presentation update is declined', function () {
		map.uiManager.showConfirmModal = (
			id: string,
			title: string,
			message: string,
			button: string,
			callback: () => void,
			hideCancel: boolean,
			cancel: () => void,
		) => {
			confirmed.push(message);
			cancel();
		};
		deliver('slidelinks', {
			message: { links: [...presentationList.links, ...list.links] },
		});
		sent = [];

		// The presentation keeps its slides, the slide of its section among
		// them, and the slides of the same source linked on their own are
		// read one by one.
		links.updateSource('Sales deck.odp');
		nodeassert.equal(confirmed.length, 1);
		nodeassert.deepEqual(sent, [exportOf('Sales deck.odp', [numbersGuid])]);
	});

	it('takes the link off a presentation, and off everything in it when asked', function () {
		deliver('slidelinks', { message: presentationList });
		sent = [];

		links.breakLink(deckPresentation, false);
		links.breakLink(deckPresentation, true);
		nodeassert.deepEqual(sent, [
			'slidelink break unit=presentation id=' +
				encodeURIComponent(deckPresentation),
			'slidelink break unit=presentation id=' +
				encodeURIComponent(deckPresentation) +
				' deep=1',
		]);

		deliver('slidelinkerror', {
			kind: 'failed',
			unit: 'presentation',
			id: deckPresentation,
		});
		nodeassert.deepEqual(told, ['Unlinking the presentation failed.']);
	});
});
