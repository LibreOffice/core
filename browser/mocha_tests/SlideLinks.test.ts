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
	// can hand it one as the socket would, and the replies of the host.
	let listeners: Map<string, (e: any) => void>;
	let hostReplies: Map<string, (e: any) => void>;
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
	let hostPicksFiles: boolean;
	let links: SlideLinks;
	let savedSocket: any;
	let savedEvents: any;

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

	function answerHost(name: string, detail: any): void {
		const reply = hostReplies.get(name);
		nodeassert.ok(reply, 'nothing listens for ' + name);
		reply({ detail: detail });
	}

	// The document reports the links above, and what it did about them is
	// dropped, so that what follows is read on its own.
	function documentHoldsLinks(): void {
		deliver('slidelinks', { message: list });
		sent = [];
		told = [];
		posted = [];
	}

	// What wsd reports when a refresh of one source ends.
	function refreshEnded(source: string, state: string, extra: any): void {
		const entry = Object.assign({ source: source, state: state }, extra);
		deliver('slidelinkstatus', {
			message: { state: 'done', sources: [entry] },
		});
	}

	function chooseFileFor(source: string): string {
		return 'Choose the file to update the slides of ' + source + ' from.';
	}

	beforeEach(function () {
		listeners = new Map();
		hostReplies = new Map();
		sent = [];
		told = [];
		posted = [];
		hidden = true;
		commandShown = new Map();
		editable = true;
		hostPicksFiles = true;

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
			get wopi() {
				return { EnableInsertRemoteFile: hostPicksFiles };
			},
		};

		savedSocket = app.socket;
		savedEvents = app.events;
		(app as any).socket = {
			sendMessage: (message: string) => sent.push(message),
		};
		(app as any).events = {
			on: (name: string, callback: any) => hostReplies.set(name, callback),
			fire: () => {},
		};

		links = new SlideLinks(map);
	});

	afterEach(function () {
		(app as any).socket = savedSocket;
		(app as any).events = savedEvents;
	});

	it('sends nothing to the server on load', function () {
		deliver('docloaded', { status: true });
		// Every status update of the open document fires docloaded again.
		deliver('docloaded', { status: true });
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

	it('asks the host to resolve every source once the document opens', function () {
		deliver('docloaded', { status: true });
		deliver('slidelinks', { message: list });

		nodeassert.equal(posted[0].msgId, 'UI_ResolveSlideSource');
		nodeassert.deepEqual(
			posted.map((message) => message.args.SourceId),
			['Sales deck.odp', 'Support deck.odp'],
		);

		// The host is asked once for a document, not again for every list it
		// reports.
		deliver('slidelinks', { message: list });
		nodeassert.equal(posted.length, 2);
	});

	it('asks the host about the first list of a load alone', function () {
		deliver('docloaded', { status: true });
		// The first list of this load reports no links, so there is nothing
		// to ask the host about.
		deliver('slidelinks', { message: { links: [] } });
		nodeassert.deepEqual(posted, []);

		// The links a later list brings were just inserted from content in
		// hand, so they are current already and nothing is asked.
		deliver('slidelinks', { message: list });
		nodeassert.deepEqual(posted, []);
	});

	it('refreshes a source the host resolved by itself', function () {
		documentHoldsLinks();
		answerHost('slidelink:resolved', {
			source: 'Sales deck.odp',
			url: 'https://host/asset/1',
		});

		nodeassert.deepEqual(sent, [
			'slidelink refresh source=Sales%20deck.odp url=https%3A%2F%2Fhost%2Fasset%2F1',
		]);
	});

	it('leaves a source the document holds no page of alone', function () {
		documentHoldsLinks();
		answerHost('slidelink:resolved', {
			source: 'Another deck.odp',
			url: 'https://host/asset/1',
		});

		nodeassert.deepEqual(sent, []);
	});

	it('asks for one source at a time, in the order they are listed', function () {
		documentHoldsLinks();
		links.updateAll();

		// The first source is named to the user and its file asked for.
		nodeassert.deepEqual(told, [chooseFileFor('Sales deck.odp')]);
		nodeassert.equal(posted.length, 1);
		nodeassert.equal(posted[0].msgId, 'UI_InsertFile');
		nodeassert.equal(posted[0].args.callback, 'Action_RefreshSlideSource');

		answerHost('slidelink:picked', { url: 'https://host/asset/1' });
		nodeassert.deepEqual(sent, [
			'slidelink refresh source=Sales%20deck.odp url=https%3A%2F%2Fhost%2Fasset%2F1',
		]);

		// While that refresh is under way the next source is not asked for.
		deliver('slidelinkstatus', {
			message: {
				state: 'refreshing',
				sources: [{ source: 'Sales deck.odp', state: 'refreshing' }],
			},
		});
		nodeassert.equal(posted.length, 1);

		// It ends, and only now is the file of the second source asked for.
		refreshEnded('Sales deck.odp', 'ok', { slides: 2 });
		nodeassert.deepEqual(told, [
			chooseFileFor('Sales deck.odp'),
			'2 slides updated from Sales deck.odp.',
			chooseFileFor('Support deck.odp'),
		]);
		nodeassert.equal(posted.length, 2);

		answerHost('slidelink:picked', { url: 'https://host/asset/2' });
		nodeassert.equal(sent.length, 2);
		nodeassert.ok(sent[1].indexOf('source=Support%20deck.odp') >= 0);
	});

	it('holds the next pick while the user still has a chooser open', function () {
		documentHoldsLinks();
		// A refresh the host set off on its own is running while the user
		// starts an update, so a pick is out with the user.
		answerHost('slidelink:resolved', {
			source: 'Support deck.odp',
			url: 'https://host/asset/9',
		});
		links.updateAll();
		nodeassert.deepEqual(told, [chooseFileFor('Sales deck.odp')]);

		// The quiet refresh ends. The chooser the user is in keeps its
		// source, so no second chooser opens over it.
		refreshEnded('Support deck.odp', 'ok', { slides: 1 });
		nodeassert.equal(posted.length, 1);

		// The pick that comes back refreshes the source it was asked for.
		answerHost('slidelink:picked', { url: 'https://host/asset/1' });
		nodeassert.ok(
			sent[sent.length - 1].indexOf('source=Sales%20deck.odp') >= 0,
		);
	});

	it('drops a refresh the connection took with it', function () {
		documentHoldsLinks();
		answerHost('slidelink:resolved', {
			source: 'Sales deck.odp',
			url: 'https://host/asset/1',
		});
		nodeassert.equal(sent.length, 1);

		// The connection goes down before the refresh is answered, and comes
		// back. wsd forgets the refreshes of the session that left, so the
		// reloaded document drops the refresh in flight rather than waiting
		// forever behind one nothing will answer, and sends nothing new.
		deliver('docloaded', { status: false });
		deliver('docloaded', { status: true });
		nodeassert.equal(sent.length, 1);

		deliver('slidelinks', { message: list });
		answerHost('slidelink:resolved', {
			source: 'Support deck.odp',
			url: 'https://host/asset/2',
		});
		nodeassert.ok(
			sent[sent.length - 1].indexOf('source=Support%20deck.odp') >= 0,
		);
	});

	it('says why a source was not refreshed, and carries on', function () {
		documentHoldsLinks();
		links.updateAll();
		answerHost('slidelink:picked', { url: 'https://host/asset/1' });
		refreshEnded('Sales deck.odp', 'failed', { reason: 'cantread' });

		nodeassert.deepEqual(told, [
			chooseFileFor('Sales deck.odp'),
			'The file chosen for Sales deck.odp could not be read.',
			chooseFileFor('Support deck.odp'),
		]);
	});

	it('keeps quiet about a refresh nobody asked for', function () {
		documentHoldsLinks();
		answerHost('slidelink:resolved', {
			source: 'Sales deck.odp',
			url: 'https://host/asset/1',
		});
		refreshEnded('Sales deck.odp', 'failed', { reason: 'cantread' });
		nodeassert.deepEqual(told, []);

		// What such a refresh did update is worth saying all the same.
		answerHost('slidelink:resolved', {
			source: 'Support deck.odp',
			url: 'https://host/asset/2',
		});
		refreshEnded('Support deck.odp', 'ok', { slides: 1 });
		nodeassert.deepEqual(told, ['1 slide updated from Support deck.odp.']);
	});

	it('goes on to the next source when the pick brought no location', function () {
		documentHoldsLinks();
		links.updateAll();
		answerHost('slidelink:picked', { url: '' });

		nodeassert.deepEqual(sent, []);
		nodeassert.deepEqual(told, [
			chooseFileFor('Sales deck.odp'),
			'Updating the slides of Sales deck.odp failed.',
			chooseFileFor('Support deck.odp'),
		]);
	});

	it('ends the update when the integration hands over the file itself', function () {
		documentHoldsLinks();
		links.updateAll();
		// A reply carrying the content of the file rather than a location
		// would come back the same way for every source, so no chooser opens
		// for the sources that are left.
		answerHost('slidelink:picked', { url: '', content: true });

		nodeassert.deepEqual(sent, []);
		nodeassert.equal(
			posted.filter((message) => message.msgId === 'UI_InsertFile').length,
			1,
		);
		nodeassert.ok(told[told.length - 1].indexOf('cannot be updated') >= 0);
	});

	it('goes on to the next source when a refresh is refused outright', function () {
		documentHoldsLinks();
		links.updateAll();
		answerHost('slidelink:picked', { url: 'https://host/asset/1' });

		// Such a refusal reaches the client as an error and nothing else.
		deliver('slidelinkerror', { kind: 'syntax' });

		nodeassert.deepEqual(told, [
			chooseFileFor('Sales deck.odp'),
			'Updating the slides of Sales deck.odp failed.',
			chooseFileFor('Support deck.odp'),
		]);
	});

	it('leaves a refresh under way to the status the server reports for it', function () {
		documentHoldsLinks();
		links.updateAll();
		answerHost('slidelink:picked', { url: 'https://host/asset/1' });
		deliver('slidelinkstatus', {
			message: {
				state: 'refreshing',
				sources: [{ source: 'Sales deck.odp', state: 'refreshing' }],
			},
		});

		// An error the document raised over a refresh the server took on is
		// not the answer to it, so no next picker opens for it.
		deliver('slidelinkerror', { kind: 'notlinked' });
		nodeassert.equal(posted.length, 1);

		refreshEnded('Sales deck.odp', 'failed', { reason: 'notlinked' });
		nodeassert.deepEqual(told, [
			chooseFileFor('Sales deck.odp'),
			'No slide of this presentation is linked to Sales deck.odp.',
			chooseFileFor('Support deck.odp'),
		]);
	});

	it('says so rather than asking for a file the integration cannot pick', function () {
		hostPicksFiles = false;
		documentHoldsLinks();
		links.updateAll();

		nodeassert.deepEqual(posted, []);
		nodeassert.equal(told.length, 1);
	});

	it('updates nothing from a view that cannot edit', function () {
		documentHoldsLinks();
		editable = false;
		links.updateAll();

		nodeassert.deepEqual(posted, []);
		nodeassert.deepEqual(told, []);
	});
});
