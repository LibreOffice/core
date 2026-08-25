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

/*
 * SlideLinks - the pages of the open document that are linked to a source
 * document, and the updates that refresh them from it.
 *
 * Holds the link list the document reports - which source document and
 * which slide of it each linked page came from - and runs the updates: it
 * asks the integration for a location, once per source, and sends the
 * refresh the server carries out.
 *
 * A location comes from the integration in one of two ways. The user picks
 * the source file again, which every host offering a file chooser answers,
 * and which is what the Update Linked Slides command does. Or the host
 * resolves a source it was told about earlier, with no gesture, which is
 * what a document asks for when it opens; a host that does not answer
 * leaves the document showing the slides it holds.
 *
 * A view reads one location at a time and a refresh that would end a read
 * in flight is refused as busy, so the refreshes of a view go one after
 * another: the next location is asked for only once the refresh in hand is
 * reported done.
 */

/* global app _ */

interface SlideLinkPage {
	// The linked page, by its part identifier.
	part: string;
	// The slide of the source the page was made from, as the user sees it
	// in that source.
	name: string;
}

interface SlideLinkSource {
	// The source document, as the user knows it.
	source: string;
	slides: SlideLinkPage[];
}

// A refresh with a location in hand, waiting for its turn.
interface SlideLinkRefresh {
	source: string;
	url: string;
	// Whether this refresh is one the user asked for or one a host offered a
	// location for on its own.
	quiet: boolean;
	// Whether the server has taken this refresh on, which it reports before
	// it reads anything.
	accepted: boolean;
}

class SlideLinks {
	private map: any;
	// The source document and source slide of every linked page, by part.
	private pages: Map<string, { source: string; name: string }> = new Map();
	// The sources the document links to, in the order the list reports them.
	private sources: string[] = [];
	// Whether the first link list of this load has been handled.
	private askedHost: boolean = false;
	// Whether the link list of this load has been asked for.
	private loaded: boolean = false;
	private queue: SlideLinkRefresh[] = [];
	// The refresh the server is running, or null.
	private running: SlideLinkRefresh | null = null;
	// The sources of an update the user asked for that are still to be
	// picked, and the one the user is picking a file for.
	private toPick: string[] = [];
	private picking: string = '';

	constructor(map: any) {
		this.map = map;

		map.on('docloaded', this.onDocLoaded, this);
		map.on('slidelinks', this.onList, this);
		map.on('slidelinkstatus', this.onStatus, this);
		map.on('slidelinkerror', this.onError, this);

		app.events.on('slidelink:picked', this.onPicked.bind(this));
		app.events.on('slidelink:resolved', this.onResolved.bind(this));
	}

	public hasLinks(): boolean {
		return this.sources.length > 0;
	}

	// The source document a page was made from and the slide of it, or null
	// for a page that is linked to nothing.
	public getPageLink(part: string): { source: string; name: string } | null {
		const link = this.pages.get(part);
		return link ? link : null;
	}

	// Refreshes the pages of every source of this document, asking for each
	// source to be picked again and refreshing the pages of it from the file
	// the user picks.
	public updateAll(): void {
		if (!this.map.isEditMode()) return;
		if (!this.hasLinks()) {
			this.say(_('No slide of this presentation is linked to another file.'));
			return;
		}
		// A location for a source comes from the host, so an integration
		// without a file chooser cannot update linked slides at all.
		if (!app.LOUtil.hostOffersFileChooser(this.map['wopi'])) {
			this.say(
				_(
					'Updating linked slides needs the file chooser of the integration, which this one does not offer.',
				),
			);
			return;
		}
		// This run asks for the sources the document holds now, so asking
		// again is how a run the user left half way through is started over.
		this.toPick = this.sources.slice();
		this.pickNext();
	}

	private say(message: string): void {
		this.map.uiManager.showSnackbar(message);
	}

	private onDocLoaded(e: any): void {
		// The connection went down, or the load failed. wsd forgets the
		// refreshes of a session that leaves, so nothing in flight will be
		// answered again: drop it all, and let the next load start clean.
		if (e.status === false) {
			this.loaded = false;
			this.abandonUpdate();
			return;
		}
		if (!this.map.isPresentationOrDrawing()) return;
		// Every status update of the open document fires this event again,
		// so the once-per-load state is reset only the first time.
		if (this.loaded) return;
		this.loaded = true;
		this.askedHost = false;
	}

	// Drops the update in hand: the refresh being run, the locations waiting
	// for their turn, and the sources still to be picked.
	private abandonUpdate(): void {
		this.queue = [];
		this.running = null;
		this.toPick = [];
		this.picking = '';
	}

	private onList(e: any): void {
		const links: SlideLinkSource[] =
			e.message && Array.isArray(e.message.links) ? e.message.links : [];
		this.pages.clear();
		this.sources = [];
		for (const entry of links) {
			if (!entry || !entry.source || !Array.isArray(entry.slides)) continue;
			this.sources.push(entry.source);
			for (const slide of entry.slides)
				this.pages.set(slide.part, {
					source: entry.source,
					name: slide.name,
				});
		}
		app.events.fire('slidelink:changed', {});
		this.showUpdateCommand();

		// A source a host can resolve by itself is refreshed as the document
		// opens, so a meta presentation shows what its sources hold now. The
		// first list of a load is the one asked about: the links a later
		// list brings were just made from content that was read to make
		// them, and are current already.
		if (!this.askedHost && this.map.isEditMode()) {
			this.askedHost = true;
			for (const source of this.sources)
				this.map.fire('postMessage', {
					msgId: 'UI_ResolveSlideSource',
					args: { SourceId: source },
				});
		}
	}

	// The command that updates the linked slides is offered by a document
	// that holds some, and by no other. The record on the uiManager is the
	// one the notebookbar bars consult when they are built, and the menubar
	// keeps a record of its own that its rebuilds read.
	private showUpdateCommand(): void {
		const offered = this.hasLinks();
		this.map.uiManager.showCommand('updateslidelinks', offered);
		const menubar = this.map.menubar;
		if (menubar) {
			if (offered) menubar.showItem('updateslidelinks');
			else menubar.hideItem('updateslidelinks');
		}
	}

	// Asks the integration for the file of the next source of an update.
	private pickNext(): void {
		this.picking = '';
		const next = this.toPick.shift();
		if (next === undefined) return;
		this.picking = next;
		this.say(
			_('Choose the file to update the slides of {0} from.').replace(
				'{0}',
				() => next,
			),
		);
		this.map.fire('postMessage', {
			msgId: 'UI_InsertFile',
			args: {
				callback: 'Action_RefreshSlideSource',
				mimeTypeFilter: app.LOUtil.presentationMimeFilter,
			},
		});
	}

	// The file the user picked for the source of an update. The pages of
	// that source are refreshed from it whatever the picked file is called,
	// since the pick is the answer to a question naming the source.
	private onPicked(e: any): void {
		if (this.picking === '') return;
		const source = this.picking;
		this.picking = '';
		const url = e.detail ? e.detail.url : '';
		if (url) {
			this.enqueue(source, url, false);
			return;
		}
		// A reply that carries the file itself rather than a location would
		// come back the same way for every source, so the run ends on the
		// first one.
		if (e.detail && e.detail.content) {
			this.toPick = [];
			this.say(
				_(
					'This integration hands over the picked file itself rather than a link to it, so linked slides cannot be updated here.',
				),
			);
			return;
		}
		this.say(
			_('Updating the slides of {0} failed.').replace('{0}', () => source),
		);
		this.pickNext();
	}

	// A location the host resolved for a source by itself.
	private onResolved(e: any): void {
		const source = e.detail ? e.detail.source : '';
		const url = e.detail ? e.detail.url : '';
		if (!source || !url) return;
		if (!this.map.isEditMode()) return;
		// A location for a source this document holds no page of refreshes
		// nothing.
		if (this.sources.indexOf(source) < 0) return;
		this.enqueue(source, url, true);
	}

	private enqueue(source: string, url: string, quiet: boolean): void {
		this.queue.push({
			source: source,
			url: url,
			quiet: quiet,
			accepted: false,
		});
		this.sendNext();
	}

	private sendNext(): void {
		if (this.running !== null) return;
		const next = this.queue.shift();
		if (next === undefined) return;
		this.running = next;
		app.socket.sendMessage(
			'slidelink refresh source=' +
				encodeURIComponent(next.source) +
				' url=' +
				encodeURIComponent(next.url),
		);
	}

	// Continues an update once the refresh in hand has ended: the next
	// location already in hand is sent, and the next source is asked for
	// only when none is waiting and no pick is already out with the user.
	private continueAfterEnd(): void {
		this.sendNext();
		if (this.running === null && this.queue.length === 0 && this.picking === '')
			this.pickNext();
	}

	private onStatus(e: any): void {
		const sources =
			e.message && Array.isArray(e.message.sources) ? e.message.sources : [];
		for (const entry of sources) {
			if (!entry || !this.running || entry.source !== this.running.source)
				continue;
			if (entry.state === 'refreshing') {
				this.running.accepted = true;
				continue;
			}
			this.report(entry, this.running.quiet);
			this.running = null;
		}
		this.continueAfterEnd();
	}

	// An error answers a refresh the server would not take on at all. Once it
	// has taken one on, the errors that reach the client are the ones the
	// document raised, and the state of the refresh follows in a status
	// message of its own.
	private onError(e: any): void {
		window.app.console.warn('slidelink error of kind ' + (e.kind || ''));
		if (this.running === null || this.running.accepted) return;
		const refresh = this.running;
		this.running = null;
		if (!refresh.quiet)
			this.say(
				_('Updating the slides of {0} failed.').replace(
					'{0}',
					() => refresh.source,
				),
			);
		this.continueAfterEnd();
	}

	// What a source came to, said to the user. A refresh nobody asked for
	// says what it updated and leaves a failure to the log. Substitutions
	// take a function, which puts a name in as it is.
	private report(entry: any, quiet: boolean): void {
		if (entry.state === 'ok') {
			const count = typeof entry.slides === 'number' ? entry.slides : 0;
			if (count === 0)
				this.say(
					_('No slide of {0} was updated.').replace('{0}', () => entry.source),
				);
			else if (count === 1)
				this.say(
					_('1 slide updated from {0}.').replace('{0}', () => entry.source),
				);
			else
				this.say(
					_('{0} slides updated from {1}.')
						.replace('{0}', String(count))
						.replace('{1}', () => entry.source),
				);
			return;
		}

		window.app.console.warn(
			'The slides of ' +
				entry.source +
				' were not updated: ' +
				(entry.reason || ''),
		);
		if (!quiet) this.say(this.failureText(entry));
	}

	private failureText(entry: any): string {
		switch (entry.reason) {
			case 'cantread':
				return _('The file chosen for {0} could not be read.').replace(
					'{0}',
					() => entry.source,
				);
			case 'unreadable':
				return _(
					'The file chosen for {0} holds no slides that can be read.',
				).replace('{0}', () => entry.source);
			case 'notlinked':
				return _('No slide of this presentation is linked to {0}.').replace(
					'{0}',
					() => entry.source,
				);
			case 'busy':
				return _(
					'Another update is still running. Ask for {0} again in a moment.',
				).replace('{0}', () => entry.source);
		}
		return _('Updating the slides of {0} failed.').replace(
			'{0}',
			() => entry.source,
		);
	}
}
