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
 * names a source and the server refreshes the pages of it.
 *
 * A source is a document the storage named as a related document of this
 * one, and the server holds the access token to read it with, so an update
 * needs nothing of the user beyond asking for it. A source the server knows
 * no related document by is reported as one that cannot be updated.
 *
 * A document reads a few sources at once at the most, and a refresh that
 * arrives past that is refused as busy, so the refreshes of a view go one
 * after another: the next source is sent only once the refresh in hand is
 * reported done.
 */

/* global app _ */

interface SlideLinkPage {
	// The linked page, by its part identifier.
	part: string;
	// The slide of the source the page was made from, as the user sees it
	// in that source.
	name: string;
	// The time the source was last modified when the page was read from it,
	// empty when none was recorded.
	lastModifiedTime: string;
}

interface SlideLinkSource {
	// The source document, as the user knows it.
	source: string;
	slides: SlideLinkPage[];
}

// A refresh waiting for its turn.
interface SlideLinkRefresh {
	source: string;
	// Whether the server has taken this refresh on, which it reports before
	// it reads anything.
	accepted: boolean;
}

class SlideLinks {
	private map: any;
	private pages: Map<
		string,
		{ source: string; name: string; lastModifiedTime: string }
	> = new Map();
	// The sources the document links to, in the order the list reports them.
	private sources: string[] = [];
	// Whether the link list of this load has been asked for.
	private loaded: boolean = false;
	private queue: SlideLinkRefresh[] = [];
	// The refresh the server is running, or null.
	private running: SlideLinkRefresh | null = null;

	constructor(map: any) {
		this.map = map;

		map.on('docloaded', this.onDocLoaded, this);
		map.on('slidelinks', this.onList, this);
		map.on('slidelink', this.onUpdated, this);
		map.on('remotedoccommandresult', this.onRemoteResult, this);
		map.on('relateddocuments', this.onRelatedDocuments, this);
		map.on('slidelinkerror', this.onError, this);
		map.on('slideimport', this.onImportMessage, this);
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

	public isPageOutdated(part: string): boolean {
		const link = this.pages.get(part);
		if (!link || !link.lastModifiedTime) return false;
		const current = this.currentSourceTime(link.source);
		return current !== null && current !== link.lastModifiedTime;
	}

	private currentSourceTime(source: string): string | null {
		const related = this.relatedDocument(source);
		return related && related.lastModifiedTime
			? related.lastModifiedTime
			: null;
	}

	// Refreshes the pages of every source of this document. The server reads
	// each source as a related document of this one, so nothing is asked of
	// the user beyond the command itself.
	public updateAll(): void {
		if (!this.map.isEditMode()) return;
		if (!this.hasLinks()) {
			this.say(_('No slide of this presentation is linked to another file.'));
			return;
		}
		// This run covers the sources the document holds now. One that is
		// already waiting for its turn is left where it is, so asking twice
		// refreshes each source once.
		for (const source of this.sources) {
			if (this.running && this.running.source === source) continue;
			if (this.queue.some((refresh) => refresh.source === source)) continue;
			this.enqueue(source);
		}
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
		// so the list is read once per load.
		if (this.loaded) return;
		this.loaded = true;
		app.socket.sendMessage('slidelink list');
	}

	// Drops the update in hand: the refresh being run and the sources waiting
	// for their turn.
	private abandonUpdate(): void {
		this.queue = [];
		this.running = null;
	}

	// Slides inserted from another file can be links to it, so the list is
	// read again once an insert is done.
	private onImportMessage(e: any): void {
		if (e.message && e.message.status === 'inserted')
			app.socket.sendMessage('slidelink list');
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
					lastModifiedTime: slide.lastModifiedTime || '',
				});
		}
		app.events.fire('slidelink:changed', {});
		this.showUpdateCommand();
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

	private enqueue(source: string): void {
		this.queue.push({ source: source, accepted: false });
		this.sendNext();
	}

	// The related document a source names, as the storage announced it, or null when this
	// document is related to no document of that name.
	private relatedDocument(
		source: string,
	): { wopiSrc: string; state: string; lastModifiedTime?: string } | null {
		for (const doc of app.relatedDocuments || []) {
			if (SlideImportSession.relatedDocumentName(doc.wopiSrc) === source)
				return doc;
		}
		return null;
	}

	// A source writes its pages out over the live link this document holds to it, so a source
	// nothing is linked to yet is subscribed to first and asked once it answers.
	private sendNext(): void {
		if (this.running !== null) return;
		const next = this.queue.shift();
		if (next === undefined) return;

		const related = this.relatedDocument(next.source);
		if (!related) {
			this.say(
				_(
					'{0} is not one of the documents this one is related to, so the server cannot read it.',
				).replace('{0}', () => next.source),
			);
			this.sendNext();
			return;
		}

		this.running = next;
		if (related.state === 'connected') this.askForPages(related.wopiSrc);
		else SlideImportSession.subscribeRelatedDocument(related.wopiSrc);
	}

	private askForPages(wopiSrc: string): void {
		if (this.running === null) return;
		this.running.accepted = true;
		SlideImportSession.sendRemoteCommand(wopiSrc, 'exportslides');
	}

	// A source this run is waiting on has come up, so it is asked for its pages.
	private onRelatedDocuments(): void {
		// A source's time may have moved on, so the pages linked to it are
		// looked at again to see whether they are still up to date.
		app.events.fire('slidelink:changed', {});

		if (this.running === null || this.running.accepted) return;
		const related = this.relatedDocument(this.running.source);
		if (related && related.state === 'connected')
			this.askForPages(related.wopiSrc);
	}

	// The pages the source wrote, staged in this document's jail by the server. The document
	// reads the pages of that source from the file.
	private onRemoteResult(e: any): void {
		if (this.running === null) return;
		const textMsg = e.textMsg || '';
		if (!textMsg.startsWith('exportslides:')) return;

		const stagedName = SlideImportSession.stagedExportName(
			textMsg.substring('exportslides:'.length),
		);
		if (!stagedName) {
			const refresh = this.running;
			this.running = null;
			this.say(
				_(
					'The slides of {0} could not be read into this presentation.',
				).replace('{0}', () => refresh.source),
			);
			this.sendNext();
			return;
		}

		// The pages read now match the source as it is, so they record the
		// source time the related documents list reports for it.
		const current = this.currentSourceTime(this.running.source);
		app.socket.sendMessage(
			'slidelink update source=' +
				encodeURIComponent(this.running.source) +
				' file=' +
				encodeURIComponent(stagedName) +
				(current ? ' time=' + encodeURIComponent(current) : ''),
		);
	}

	// What the document made of the pages it was given.
	private onUpdated(e: any): void {
		const message = e.message;
		if (!message || message.status !== 'updated' || this.running === null)
			return;
		if (message.source !== this.running.source) return;

		const refresh = this.running;
		this.running = null;
		this.report({
			source: refresh.source,
			state: 'ok',
			slides: typeof message.count === 'number' ? message.count : 0,
		});
		this.sendNext();
	}

	// An error ends the refresh in hand. An answer that names a page belongs to a break, and
	// one that names another source to a refresh of that source; the refresh in hand is
	// answered by an error that names it and by one that names nothing.
	private onError(e: any): void {
		window.app.console.warn('slidelink error of kind ' + (e.kind || ''));
		if (e.part) return;
		if (this.running === null) return;
		if (e.source && e.source !== this.running.source) return;
		const refresh = this.running;
		this.running = null;
		this.say(
			_('Updating the slides of {0} failed.').replace(
				'{0}',
				() => refresh.source,
			),
		);
		this.sendNext();
	}

	// What a source came to, said to the user. Substitutions take a function,
	// which puts a name in as it is.
	private report(entry: any): void {
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
		this.say(this.failureText(entry));
	}

	private failureText(entry: any): string {
		switch (entry.reason) {
			case 'nosource':
				return _(
					'{0} is not one of the documents this one is related to, so the server cannot read it.',
				).replace('{0}', () => entry.source);
			case 'cantread':
				return _('{0} could not be read.').replace('{0}', () => entry.source);
			case 'unreadable':
				return _(
					'The slides of {0} could not be read into this presentation.',
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
