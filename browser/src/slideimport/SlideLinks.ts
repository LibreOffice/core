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
 * SlideLinks - the links the open document holds to other documents, and the
 * updates that refresh them from those documents.
 *
 * A link is a run of slides of this document that holds a copy of one unit of
 * a source document: a slide, a section or the whole presentation. The
 * document reports every link in one list, and this class keeps the list by
 * the identifier of each link: the part of the page for a slide, the
 * identifier of the section for a section. Links nest, and each link names
 * the link that spans it as its parent.
 *
 * A source is a document the storage named as a remote link of this one, and
 * the server holds the access token to read it with, so an update needs
 * nothing of the user beyond asking for it. A source the server knows no
 * remote link by is reported as one that cannot be updated.
 *
 * A document reads a few sources at once at the most, and a refresh that
 * arrives past that is refused as busy, so the refreshes of a view go one
 * after another: the next source is sent only once the refresh in hand is
 * reported done.
 */

/* global app _ */

// What a link holds a copy of: one slide of the source, one section of it,
// or the whole presentation.
type SlideLinkUnit = 'slide' | 'section' | 'presentation';

// One link, as the document lists it.
interface SlideLink {
	unit: SlideLinkUnit;
	// The identifier of the link in this document: the part of the page for a
	// slide, the identifier of the section for a section.
	id: string;
	// The identifier of the link that spans this one, or an empty string for
	// a link that sits under none.
	parent: string;
	// The source document, as the user knows it.
	source: string;
	// The unit in the source: the identifier of the source slide, the name a
	// slide holds by its position when the link follows a position, or the
	// identifier of the source section.
	sourceId: string;
	// The name the unit had in the source when it was last read, empty for a
	// slide with no name of its own.
	name: string;
	// The time the source was last modified when the unit was last read,
	// empty when none was recorded.
	lastModifiedTime: string;
	// The parts the link spans, in slide order.
	slides: string[];
}

// How far a refresh has come. "waiting": the source is not connected yet, or
// went down, and nothing has been asked of it. "export": the source was asked
// to write out the slides. "updating": the slides the source wrote were handed
// to the document, so the answer awaited is the document's own.
type SlideLinkRefreshStep = 'waiting' | 'export' | 'updating';

// What a refresh is asked for: one link of a source, or every link of the
// source that sits under no other link.
interface SlideLinkRefreshRequest {
	source: string;
	// The one link to refresh, by its unit and its identifier, or undefined
	// for every link of the source that sits under no other link.
	unit?: SlideLinkUnit;
	id?: string;
}

// A refresh waiting for its turn, or running.
interface SlideLinkRefresh extends SlideLinkRefreshRequest {
	step: SlideLinkRefreshStep;
	// Whether the source was asked for the slides the links record the
	// identifiers of, rather than for its whole deck.
	byIdentifier: boolean;
	// Whether a subscription this run opened has been recorded by the server.
	subscribed?: boolean;
}

// The state of a link as its mark shows it: '' for no link, 'broken' when the
// source or the unit it came from is gone, 'outdated' when the source changed
// since the last read, 'connected' or 'disconnected' as the live link to the
// source stands, and 'linked' otherwise.
type SlideLinkState =
	'' | 'broken' | 'outdated' | 'connected' | 'disconnected' | 'linked';

class SlideLinks {
	private map: any;
	// Every link the document reports, by its identifier, in the order the
	// list reports them.
	private links: Map<string, SlideLink> = new Map();
	// The links that span each part, innermost first: the slide's own link,
	// then its section, then the presentation.
	private byPart: Map<string, SlideLink[]> = new Map();
	// The sources the document links to, in the order the list reports them.
	private sources: string[] = [];
	// The slides a source reports, by the identifier of each one, the
	// identifiers of its sections, and the time that source was last modified
	// when they were read. A null set stands for a source that has been asked
	// and has not answered yet.
	private sourceSlides: Map<
		string,
		{
			time: string;
			guids: Set<string> | null;
			names: Map<string, string>;
			sections: Set<string> | null;
		}
	> = new Map();
	// Whether the link list of this load has been asked for.
	private loaded: boolean = false;
	private queue: SlideLinkRefresh[] = [];
	// The refresh the server is running, or null.
	private running: SlideLinkRefresh | null = null;
	// Whether a list arrived that names parts this view does not know yet, so
	// the change is announced once the part list has caught up.
	private announceOnParts: boolean = false;
	// The parts of the view's part list when it last changed, or null before
	// the first part list of a load.
	private knownParts: Set<string> | null = null;

	constructor(map: any) {
		this.map = map;

		map.on('docloaded', this.onDocLoaded, this);
		map.on('slidelinks', this.onList, this);
		map.on('updateparts', this.onUpdateParts, this);
		map.on('slidelink', this.onUpdated, this);
		map.on('remotedoccommandresult', this.onRemoteResult, this);
		map.on('remotelinks', this.onRemoteLinks, this);
		map.on('slidelinkerror', this.onError, this);
		map.on('slideimport', this.onImportMessage, this);
	}

	// Whether anything of this document is linked to another file.
	public hasLinks(): boolean {
		return this.links.size > 0;
	}

	// The source of this document's links that names the given remote link,
	// or an empty string when nothing is linked to it.
	public linkedSourceOf(doc: { persistentLink?: string }): string {
		return (
			this.sources.find((source) =>
				SlideImportSession.matchesDocument(doc, source),
			) || ''
		);
	}

	// The link of the given identifier, as the document lists it, or null for
	// a unit that is linked to nothing.
	public getLink(id: string): SlideLink | null {
		return this.links.get(id) || null;
	}

	// The links that span a part, innermost first: the slide's own link when
	// it holds one, then the section that holds the slide, then the
	// presentation. Empty for a slide that sits under no link.
	public linksOf(part: string): SlideLink[] {
		return this.byPart.get(part) || [];
	}

	// The name the source reports now for the slide a link was made from, for
	// a slide link that follows the identifier of its source slide, or an
	// empty string.
	public getSourceSlideName(id: string): string {
		const link = this.links.get(id);
		if (!link || link.unit !== 'slide' || !SlideLinks.isGuid(link.sourceId))
			return '';
		const slides = this.sourceSlides.get(link.source);
		return (slides && slides.names.get(link.sourceId)) || '';
	}

	// Whether the source was modified since the link was last read from it.
	public isOutdated(id: string): boolean {
		const link = this.links.get(id);
		if (!link || !link.lastModifiedTime) return false;
		const current = this.currentSourceTime(link.source);
		return current !== null && current !== link.lastModifiedTime;
	}

	// Whether the source was read and no longer holds the unit the link was
	// made from: the slide of that identifier, or the section. A slide that
	// follows a position reads whichever slide stands there, so it is never
	// missing, and a presentation is missing only when its source is.
	public isMissing(id: string): boolean {
		const link = this.links.get(id);
		if (!link) return false;
		const known = this.sourceSlides.get(link.source);
		if (!known) return false;
		if (link.unit === 'slide')
			return (
				SlideLinks.isGuid(link.sourceId) &&
				known.guids !== null &&
				!known.guids.has(link.sourceId)
			);
		if (link.unit === 'section')
			return known.sections !== null && !known.sections.has(link.sourceId);
		return false;
	}

	// The state of the remote link a link's source names, as the storage
	// announced it, or an empty string when this document links to no
	// document of that name or the unit is linked to nothing.
	public getSourceState(id: string): string {
		const link = this.links.get(id);
		return link ? this.sourceState(link.source) : '';
	}

	// The state the mark of a link shows. A link is broken when the source
	// file is missing, or when the source was read and holds the unit no
	// longer. No access and could not open are not broken, so the mark keeps
	// the link icon for those.
	public getLinkState(id: string): SlideLinkState {
		const link = this.links.get(id);
		if (!link) return '';
		const state = this.sourceState(link.source);
		if (state === 'missing' || this.isMissing(id)) return 'broken';
		if (this.isOutdated(id)) return 'outdated';
		if (state === 'connected' || state === 'disconnected') return state;
		return 'linked';
	}

	// Whether the source of a link can be read and still holds its unit, so
	// that the link can be brought up to date from it.
	public isUpdatable(id: string): boolean {
		return (
			this.links.has(id) &&
			!this.isMissing(id) &&
			SlideLinks.isReadable(this.getSourceState(id))
		);
	}

	// How many slides of this document are linked to one source, through a
	// link of their own or through the section or the presentation that
	// holds them. A plain copy records nothing to count.
	public countSlidesFrom(source: string): number {
		return this.slidesFrom(source, () => true).size;
	}

	// How many of those slides the source has changed since they were read.
	public countOutdatedSlidesFrom(source: string): number {
		return this.slidesFrom(source, (link) => this.isOutdated(link.id)).size;
	}

	// The parts spanned by the links to a source that the given test takes.
	private slidesFrom(
		source: string,
		taken: (link: SlideLink) => boolean,
	): Set<string> {
		const parts = new Set<string>();
		this.links.forEach((link) => {
			if (link.source !== source || !taken(link)) return;
			for (const part of link.slides) parts.add(part);
		});
		return parts;
	}

	// Whether a source in the given state can be read: the storage named it,
	// this view holds a token for it, and reading it has not failed.
	private static isReadable(state: string): boolean {
		return (
			state !== '' &&
			state !== 'noaccess' &&
			state !== 'missing' &&
			state !== 'failed'
		);
	}

	// Whether a source identifier names a slide by its identifier, a braced
	// GUID, rather than by the position it holds in the source.
	private static isGuid(sourceId: string): boolean {
		return /^\{[0-9A-Fa-f]{8}(-[0-9A-Fa-f]{4}){3}-[0-9A-Fa-f]{12}\}$/.test(
			sourceId,
		);
	}

	private static isUnit(unit: any): unit is SlideLinkUnit {
		return unit === 'slide' || unit === 'section' || unit === 'presentation';
	}

	// How deep a unit sits in the chain of links over one slide.
	private static depth(unit: SlideLinkUnit): number {
		return unit === 'slide' ? 0 : unit === 'section' ? 1 : 2;
	}

	// The identifier of the slide on show, or an empty string when the
	// document shows no slide.
	public currentPart(): string {
		return this.map._docLayer ? this.map._docLayer.getSelectedPart() : '';
	}

	// The state of the remote link a source names, as the storage announced
	// it, or an empty string when this document links to no document of that
	// name.
	private sourceState(source: string): string {
		const remoteLink = this.remoteLink(source);
		return remoteLink ? remoteLink.state : '';
	}

	private currentSourceTime(source: string): string | null {
		const remoteLink = this.remoteLink(source);
		return remoteLink && remoteLink.lastModifiedTime
			? remoteLink.lastModifiedTime
			: null;
	}

	// Refreshes every link of the document from its source. The server reads
	// each source as a remote link of this one, so nothing is asked of the
	// user beyond the command itself, and one question when a section is
	// among the links.
	public updateAll(): void {
		if (!this.map.isEditMode()) return;
		if (!this.hasLinks()) {
			this.say(_('No slide of this presentation is linked to another file.'));
			return;
		}
		// This run covers the sources the document holds now.
		this.confirmRefresh(this.sources.map((source) => ({ source: source })));
	}

	// Refreshes every link to one source. A source that is already being
	// read, or waiting for its turn, is left where it is, so asking twice
	// refreshes it once.
	public updateSource(source: string): void {
		if (!this.map.isEditMode()) return;
		if (this.sources.indexOf(source) < 0) return;
		this.confirmRefresh([{ source: source }]);
	}

	// Refreshes one link from its source, and leaves every other link of that
	// source as it is.
	public update(id: string): void {
		if (!this.map.isEditMode()) return;
		const link = this.links.get(id);
		if (!link) {
			this.say(_('This slide is not linked to another file.'));
			return;
		}
		if (this.isMissing(id)) {
			this.say(
				(link.unit === 'section'
					? _(
							'Cannot update the section: {0} no longer holds its source section.',
						)
					: _('Cannot update the slide: {0} no longer holds its source slide.')
				).replace('{0}', () => link.source),
			);
			return;
		}
		this.confirmRefresh([{ source: link.source, unit: link.unit, id: id }]);
	}

	// The unit keeps the slides it holds and becomes a unit of this document
	// alone. With deep, every link beneath it comes off as well, so each slide
	// of a section becomes a slide of this document alone. Without it, each
	// stays linked to its source slide.
	public breakLink(id: string, deep: boolean = false): void {
		if (!this.map.isEditMode()) return;
		const link = this.links.get(id);
		if (!link) return;
		app.socket.sendMessage(
			'slidelink break unit=' +
				link.unit +
				' id=' +
				encodeURIComponent(id) +
				(deep ? ' deep=1' : ''),
		);
	}

	// Queues the given refreshes. A refresh whose source cannot be read is
	// left out and the user told why, and one that is already running or
	// waiting for its turn is left where it is. A refresh that replaces the
	// slides of a section the user has not agreed to yet waits for one
	// question first, because a slide edited here is replaced along with the
	// others. The rest go straight in.
	private confirmRefresh(requests: SlideLinkRefreshRequest[]): void {
		const wanted: SlideLinkRefreshRequest[] = [];
		for (const request of requests) {
			const problem = this.sourceProblem(request.source);
			if (problem) {
				this.say(problem);
				continue;
			}
			if (!this.isQueued(request)) wanted.push(request);
		}

		const asked = wanted.filter(
			(request) => this.sectionsReplacedBy(request).length > 0,
		);
		this.enqueueAll(wanted.filter((request) => asked.indexOf(request) < 0));
		if (asked.length === 0) return;

		const sections = asked
			.map((request) => this.sectionsReplacedBy(request))
			.flat();
		const message =
			sections.length === 1
				? _(
						'The section will hold the slides that its section in {0} holds now. Changes made to these slides in this presentation will be lost.',
					).replace('{0}', () => sections[0].source)
				: _(
						'{0} linked sections will hold the slides that their sections in the source files hold now. Changes made to these slides in this presentation will be lost.',
					).replace('{0}', String(sections.length));

		this.map.uiManager.showConfirmModal(
			'slidelink-update',
			sections.length === 1 ? _('Update section') : _('Update sections'),
			message,
			_('Update'),
			() => {
				// The list may have changed while the user was asked.
				this.enqueueAll(
					asked.filter(
						(request) => this.stillHolds(request) && !this.isQueued(request),
					),
				);
			},
			false,
			() => {
				// The sections stay as they are, and the slides of their sources
				// that are linked on their own are read one by one.
				this.enqueueAll(
					this.slidesAlone(asked).filter((request) => !this.isQueued(request)),
				);
			},
		);
	}

	// The linked sections a refresh replaces the slides of, which the user has
	// not agreed to yet: the section the refresh names, or every section of a
	// source that is refreshed whole and is not waiting for a refresh of its
	// own.
	private sectionsReplacedBy(request: SlideLinkRefreshRequest): SlideLink[] {
		if (request.id !== undefined) {
			const link = this.links.get(request.id);
			return link && link.unit === 'section' ? [link] : [];
		}
		const sections: SlideLink[] = [];
		this.links.forEach((link) => {
			if (link.source !== request.source || link.unit !== 'section') return;
			if (!this.isQueued({ source: link.source, unit: link.unit, id: link.id }))
				sections.push(link);
		});
		return sections;
	}

	// The slides linked on their own to the sources the given refreshes read
	// whole, each as a refresh of its own.
	private slidesAlone(
		requests: SlideLinkRefreshRequest[],
	): SlideLinkRefreshRequest[] {
		const slides: SlideLinkRefreshRequest[] = [];
		for (const request of requests) {
			if (request.id !== undefined) continue;
			this.links.forEach((link) => {
				if (
					link.source === request.source &&
					link.unit === 'slide' &&
					!link.parent
				)
					slides.push({ source: link.source, unit: link.unit, id: link.id });
			});
		}
		return slides;
	}

	// Whether the document still holds what a refresh is asked for.
	private stillHolds(request: SlideLinkRefreshRequest): boolean {
		return request.id === undefined
			? this.sources.indexOf(request.source) >= 0
			: this.links.has(request.id);
	}

	// Whether a refresh that covers the request is running or waiting for
	// its turn.
	private isQueued(request: SlideLinkRefreshRequest): boolean {
		return (
			SlideLinks.covers(this.running, request) ||
			this.queue.some((refresh) => SlideLinks.covers(refresh, request))
		);
	}

	// Whether a refresh reads what the request asks for: the one link, on its
	// own or with the rest of its source, or every link of the source.
	private static covers(
		refresh: SlideLinkRefresh | null,
		request: SlideLinkRefreshRequest,
	): boolean {
		return (
			refresh !== null &&
			refresh.source === request.source &&
			(refresh.id === undefined || refresh.id === request.id)
		);
	}

	private enqueueAll(requests: SlideLinkRefreshRequest[]): void {
		for (const request of requests) {
			// The links of this source waiting to be read on their own are
			// read with the rest of it.
			if (request.id === undefined)
				this.queue = this.queue.filter(
					(refresh) => refresh.source !== request.source,
				);
			this.queue.push({
				source: request.source,
				unit: request.unit,
				id: request.id,
				step: 'waiting',
				byIdentifier: false,
			});
		}
		this.sendNext();
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
			this.knownParts = null;
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
		const entries: any[] =
			e.message && Array.isArray(e.message.links) ? e.message.links : [];
		this.links.clear();
		this.byPart.clear();
		this.sources = [];
		for (const entry of entries) {
			if (
				!entry ||
				!entry.id ||
				!entry.source ||
				!SlideLinks.isUnit(entry.unit) ||
				!Array.isArray(entry.slides)
			)
				continue;
			const link: SlideLink = {
				unit: entry.unit,
				id: entry.id,
				parent: entry.parent || '',
				source: entry.source,
				sourceId: entry.sourceId || '',
				name: entry.name || '',
				lastModifiedTime: entry.lastModifiedTime || '',
				slides: entry.slides.filter((part: any) => typeof part === 'string'),
			};
			this.links.set(link.id, link);
			if (this.sources.indexOf(link.source) < 0) this.sources.push(link.source);
			for (const part of link.slides) {
				const chain = this.byPart.get(part);
				if (chain) chain.push(link);
				else this.byPart.set(part, [link]);
			}
		}
		// The list names a section before the slides it spans, and a part is
		// read from its own link first.
		this.byPart.forEach((chain) =>
			chain.sort(
				(one, other) =>
					SlideLinks.depth(one.unit) - SlideLinks.depth(other.unit),
			),
		);
		this.readSourceSlides();
		// A refreshed page is a new page, and the list reaches a view before
		// the status that renews its part list. Announced now, a list naming
		// a part the view does not know yet would read the slide on show as
		// linked to nothing for a moment, so it is announced with that status.
		if (this.namesUnknownPart()) this.announceOnParts = true;
		else app.events.fire('slidelink:changed', {});
		this.showUpdateCommand();
	}

	// Whether the list names a part the view's part list does not hold.
	private namesUnknownPart(): boolean {
		const layer = this.map._docLayer;
		if (!layer) return false;
		for (const part of this.byPart.keys())
			if (layer.getIndexFromPart(part) < 0) return true;
		return false;
	}

	// A slide added or removed by any means, such as a delete, a paste or an
	// undo, can add or drop a link, so the list is read again. A list waiting
	// for this part list already names its parts, so it is announced instead.
	private onUpdateParts(): void {
		const changed = this.partsChanged();
		if (this.announceOnParts) {
			this.announceOnParts = false;
			app.events.fire('slidelink:changed', {});
		} else if (changed && this.loaded) app.socket.sendMessage('slidelink list');
	}

	// Whether the part list holds other parts than at its last change. The
	// first part list of a load is read with the link list of that load.
	private partsChanged(): boolean {
		const list = (app.impress && app.impress.partList) || [];
		const parts = new Set<string>(list.map((info: any) => info.part));
		const known = this.knownParts;
		this.knownParts = parts;
		if (!known) return false;
		if (known.size !== parts.size) return true;
		for (const part of parts) if (!known.has(part)) return true;
		return false;
	}

	// The command that updates the linked slides is offered by a document
	// that holds some, and by no other. Each bar is told on its own, and the
	// menubar keeps a record of its own that its rebuilds read.
	private showUpdateCommand(): void {
		if (window.coolPreview) {
			return;
		}
		const offered = this.hasLinks();
		const notebookbar = this.map.uiManager.notebookbar;
		if (notebookbar) {
			if (offered) notebookbar.showItem('insert-update-slide-links');
			else notebookbar.hideItem('insert-update-slide-links');
		}
		const menubar = this.map.menubar;
		if (menubar) {
			if (offered) menubar.showItem('updateslidelinks');
			else menubar.hideItem('updateslidelinks');
		}
	}

	// The remote link a source names, as the storage announced it, or null
	// when this document links to no document of that name.
	private remoteLink(source: string): {
		persistentLink: string;
		name?: string;
		state: string;
		lastModifiedTime?: string;
	} | null {
		for (const doc of app.remoteLinks || []) {
			if (SlideImportSession.matchesDocument(doc, source)) return doc;
		}
		return null;
	}

	// Why a source cannot be read now, said to the user, or an empty string
	// for a source that can be read. A source this view holds no token for,
	// one the storage could not give, or one no remote link stands for is
	// never asked: a subscription to it would wait for ever.
	private sourceProblem(source: string): string {
		const remoteLink = this.remoteLink(source);
		if (!remoteLink)
			return _(
				'{0} is not one of the documents this one is related to, so the server cannot read it.',
			).replace('{0}', () => source);
		return SlideLinks.isReadable(remoteLink.state)
			? ''
			: SlideLinks.unreadableMessage(source, remoteLink.state);
	}

	// Why a source in the given state cannot be read, said to the user.
	private static unreadableMessage(source: string, state: string): string {
		const message =
			state === 'noaccess'
				? _('You do not have access to {0}, so its slides cannot be updated.')
				: _('{0} could not be read.');
		return message.replace('{0}', () => source);
	}

	// A source writes its pages out over the live link this document holds to
	// it, so a source nothing is linked to yet is subscribed to first and
	// asked once it answers.
	private sendNext(): void {
		if (this.running !== null) return;
		const next = this.queue.shift();
		if (next === undefined) return;

		// A link that ended while it waited, to an edit or to another view,
		// has nothing to be updated from.
		if (next.id !== undefined && !this.links.has(next.id)) {
			this.sendNext();
			return;
		}

		// The source may have gone since the refresh was queued.
		const problem = this.sourceProblem(next.source);
		if (problem) {
			this.say(problem);
			this.sendNext();
			return;
		}

		this.running = next;
		if (this.sourceState(next.source) === 'connected') this.askForPages();
		else SlideImportSession.subscribeRemoteLink(next.source);
	}

	// A source that cannot be read leaves the run it was asked for. The user
	// is told why the source could not be read, and the next source in the
	// queue goes.
	private abandonUnreadableSource(
		refresh: SlideLinkRefresh,
		state: string,
	): void {
		this.say(SlideLinks.unreadableMessage(refresh.source, state));
		this.sendNext();
	}

	// The source in hand is asked to write out the slides the refresh reads:
	// the slides the links were made from when every one of them is known by
	// identifier, and the whole deck otherwise, since the document finds a
	// section in the file by its identifier and reads the slide a position
	// link follows from wherever it stands.
	private askForPages(): void {
		const refresh = this.running;
		if (refresh === null) return;
		refresh.step = 'export';
		const guids = this.sourceGuids(refresh);
		refresh.byIdentifier = guids !== null;
		SlideImportSession.sendRemoteCommand(
			refresh.source,
			guids === null ? 'exportslides' : 'exportslides guids=' + guids.join(','),
		);
	}

	// The identifiers of the source slides the links a refresh reads were
	// made from, or null when one of those links is no slide link that
	// follows an identifier.
	private sourceGuids(refresh: SlideLinkRefresh): string[] | null {
		const guids: string[] = [];
		for (const link of this.refreshedLinks(refresh)) {
			if (link.unit !== 'slide' || !SlideLinks.isGuid(link.sourceId))
				return null;
			guids.push(link.sourceId);
		}
		return guids.length > 0 ? guids : null;
	}

	// The links a refresh reads: the one it names, or every link of its
	// source that sits under no other link.
	private refreshedLinks(refresh: SlideLinkRefresh): SlideLink[] {
		if (refresh.id !== undefined) {
			const link = this.links.get(refresh.id);
			return link ? [link] : [];
		}
		const links: SlideLink[] = [];
		this.links.forEach((link) => {
			if (link.source === refresh.source && !link.parent) links.push(link);
		});
		return links;
	}

	// A source this run is waiting on has come up, so it is asked for its
	// pages. A source that became unreadable, or whose subscription was
	// refused, ends the run instead, so that the run leaves the queue and the
	// sources behind it are still refreshed. A source that went down after
	// it was asked is asked again once it comes back up.
	private onRemoteLinks(): void {
		this.readSourceSlides();
		app.events.fire('slidelink:changed', {});

		if (this.running === null || this.running.step === 'updating') return;
		const remoteLink = this.remoteLink(this.running.source);

		if (remoteLink && remoteLink.state === 'connected') {
			if (this.running.step === 'waiting') this.askForPages();
			return;
		}

		if (
			this.running.step !== 'waiting' &&
			remoteLink &&
			remoteLink.state === 'disconnected'
		) {
			this.running.step = 'waiting';
			return;
		}

		if (remoteLink && remoteLink.state === 'subscribed')
			this.running.subscribed = true;

		// A subscription this run opened was refused so went back to the
		// "available" state.
		const dead =
			this.running.step !== 'waiting' ||
			(remoteLink !== null &&
				remoteLink.state === 'available' &&
				this.running.subscribed === true);

		if (remoteLink && !dead && SlideLinks.isReadable(remoteLink.state)) return;

		const refresh = this.running;
		this.running = null;
		this.abandonUnreadableSource(refresh, remoteLink ? remoteLink.state : '');
	}

	// The pages the source wrote, staged in this document's jail by the
	// server. The document reads the links of that source from the file. The
	// import pane may be reading another document at the same time, so only
	// the answers of the source in hand are taken.
	private onRemoteResult(e: any): void {
		const textMsg = e.textMsg || '';
		if (textMsg.startsWith('presentationinfo:')) {
			this.onSourceSlides(
				e.source || '',
				textMsg.substring('presentationinfo:'.length),
			);
			return;
		}

		const refresh = this.running;
		if (refresh === null || refresh.step !== 'export') return;
		if (e.source && e.source !== refresh.source) return;
		if (!textMsg.startsWith('exportslides:')) return;

		const body = textMsg.substring('exportslides:'.length);
		const stagedName = SlideImportSession.stagedExportName(body);
		if (!stagedName) {
			this.running = null;
			const missing =
				refresh.byIdentifier &&
				SlideImportSession.exportFailureKind(body) === 'failed';
			this.say(
				missing
					? _(
							'Cannot update the slides of {0}: source lacks the matching slides',
						).replace('{0}', () => refresh.source)
					: _(
							'The slides of {0} could not be read into this presentation.',
						).replace('{0}', () => refresh.source),
			);
			this.sendNext();
			return;
		}

		// The slides read now match the source as it is, so they record the
		// source time the remote links list reports for it. A refresh of one
		// link names it, and the document leaves the other links of the
		// source as they are.
		const current = this.currentSourceTime(refresh.source);
		refresh.step = 'updating';
		app.socket.sendMessage(
			'slidelink update source=' +
				encodeURIComponent(refresh.source) +
				' file=' +
				encodeURIComponent(stagedName) +
				(current ? ' time=' + encodeURIComponent(current) : '') +
				(refresh.id !== undefined
					? ' unit=' + refresh.unit + ' id=' + encodeURIComponent(refresh.id)
					: ''),
		);
	}

	// Whether an answer names what the refresh in hand was sent for: its
	// source, and its unit when it named one.
	private static answers(
		named: { source?: string; unit?: string; id?: string },
		refresh: SlideLinkRefresh,
	): boolean {
		return (
			named.source === refresh.source &&
			(named.unit || undefined) === refresh.unit &&
			(named.id || undefined) === refresh.id
		);
	}

	// What the document made of the pages it was given.
	private onUpdated(e: any): void {
		const message = e.message;
		if (!message || message.status !== 'updated' || this.running === null)
			return;
		if (!SlideLinks.answers(message, this.running)) return;

		const refresh = this.running;
		this.running = null;
		this.report(
			refresh.source,
			typeof message.count === 'number' ? message.count : 0,
		);
		this.sayNotUpdated(message.notUpdated);
		this.sendNext();
	}

	// Asks every source this document is connected to for the slides and the
	// sections it holds.
	private readSourceSlides(): void {
		for (const source of this.sources) {
			const remoteLink = this.remoteLink(source);
			if (!remoteLink || remoteLink.state !== 'connected') {
				// A source that went down says nothing about its slides, so
				// one that was asked and did not answer is asked again once it
				// comes up.
				const pending = this.sourceSlides.get(source);
				if (pending && pending.guids === null) this.sourceSlides.delete(source);
				continue;
			}

			const time = remoteLink.lastModifiedTime || '';
			const known = this.sourceSlides.get(source);
			if (known && known.time === time) continue;

			this.sourceSlides.set(source, {
				time: time,
				guids: null,
				names: new Map(),
				sections: null,
			});
			SlideImportSession.sendRemoteCommand(source, 'getpresentationinfo');
		}
	}

	// The slides and the sections a source holds, as it reports them. Only a
	// source that was asked takes them.
	private onSourceSlides(source: string, json: string): void {
		const asked = this.sourceSlides.get(source);
		if (!asked) return;

		const guids = new Set<string>();
		const names = new Map<string, string>();
		const sections = new Set<string>();
		try {
			const info = JSON.parse(json);
			const slides = Array.isArray(info.slides) ? info.slides : [];
			for (const slide of slides) {
				if (slide && slide.guid) {
					guids.add(slide.guid);
					names.set(
						slide.guid,
						slide.name || _('Slide %1').replace('%1', String(slide.index + 1)),
					);
				}
			}
			// A source with no sections reports none.
			for (const section of Array.isArray(info.sections) ? info.sections : [])
				if (section && section.id) sections.add(section.id);
		} catch {
			return;
		}

		this.sourceSlides.set(source, {
			time: asked.time,
			guids: guids,
			names: names,
			sections: sections,
		});
		app.events.fire('slidelink:changed', {});
	}

	// The slides a refresh left as they were not available.
	private sayNotUpdated(notUpdated: any): void {
		if (!Array.isArray(notUpdated) || notUpdated.length === 0) return;

		const numbers = notUpdated
			.filter((part: string) => typeof part === 'string')
			.map((part: string) => String(app.impress.getIndexFromPart(part) + 1));
		if (numbers.length === 0) return;

		this.say(
			_('Cannot update slide {0}: the source lacks matching slide').replace(
				'{0}',
				() => numbers.join(', '),
			),
		);
	}

	// An error ends the refresh in hand. An answer that names a unit and no
	// source belongs to a break, and one that names another source or
	// another unit to a refresh of that; the refresh in hand is answered by
	// an error that names it and by one that names nothing.
	private onError(e: any): void {
		window.app.console.warn('slidelink error of kind ' + (e.kind || ''));
		if (e.id && !e.source) {
			// A unit that is linked to nothing has nothing to take off, and
			// the toolbar of a slide that lost its link has gone already.
			if (e.kind !== 'notlinked')
				this.say(
					e.unit === 'slide'
						? _('Unlinking the slide failed.')
						: _('Unlinking the section failed.'),
				);
			return;
		}
		if (this.running === null) return;
		if (e.source && e.source !== this.running.source) return;
		if (e.id && e.id !== this.running.id) return;
		const refresh = this.running;
		this.running = null;
		// A link asked for on its own that another view refreshed or unlinked
		// meanwhile is linked to nothing the document knows, and is up to
		// date.
		if (e.kind !== 'notlinked' || refresh.id === undefined)
			this.say(
				_('Updating the slides of {0} failed.').replace(
					'{0}',
					() => refresh.source,
				),
			);
		this.sendNext();
	}

	// What a refresh came to, said to the user. Substitutions take a
	// function, which puts a name in as it is.
	private report(source: string, count: number): void {
		if (count === 0)
			this.say(_('No slide of {0} was updated.').replace('{0}', () => source));
		else if (count === 1)
			this.say(_('1 slide updated from {0}.').replace('{0}', () => source));
		else
			this.say(
				_('{0} slides updated from {1}.')
					.replace('{0}', String(count))
					.replace('{1}', () => source),
			);
	}
}
