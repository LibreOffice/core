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
 * SlideImportSession - the state model behind the slide import pane.
 *
 * Owns the lifecycle of one import: the questions asked of the remote
 * link the slides come from, the slide selection, the keep-design flag
 * and the link-to-source flag. What each source answered about its slides is
 * the pane's own.
 * Every state change is announced as a slideimport:* event on app.events.
 *
 * A question of a remote link carries an id, and its answer comes back
 * under that id, so the session knows which of its questions each answer
 * belongs to. The kit's replies to an insert carry no such id and are
 * matched by the state the session is in.
 */

/* global app _ */

type SlideImportState = 'idle' | 'opening' | 'ready' | 'inserting';

interface SlideImportSlide {
	index: number;
	name: string;
	guid: string;
}

interface SlideImportSection {
	name: string;
	// The identifier of the section, a braced GUID, or an empty string for a
	// section that has none.
	id: string;
	// 0-based index of the section's first slide.
	startIndex: number;
	// Number of consecutive slides the section spans.
	slideCount: number;
}

interface SlideImportThumbnail {
	url: string;
	width: number;
	height: number;
}

interface SlideImportError {
	message: string;
	retry: (() => void) | null;
}

// One insert of the pane: the slides asked of the source, counted from zero, and the slide
// of this document the pages go in before, counted from zero, or the count of slides for the
// end of the document. A whole insert takes every slide of the source, lists none, and links
// the pages as one presentation recorded under name, the source as the user knows it. name is
// empty for an insert that is not whole.
interface SlideImportInsert {
	slides: number[];
	at: number;
	whole: boolean;
	name: string;
}

class SlideImportSession {
	public state: SlideImportState = 'idle';
	public fileName: string = '';
	// How many slides the source holds, which bounds every index a selection
	// or an insert names.
	public slideCount: number = 0;
	public selection: Set<number> = new Set();
	public keepDesign: boolean = false;
	// Whether the inserted slides stay bound to the file they came from, so
	// that they can be updated from it later. The user's choice, kept across
	// files.
	public linkToSource: boolean = false;
	// Whether the inserted slides are linked to the position their slides hold in the source
	// rather than to the slides themselves. The user's choice, kept across files.
	public linkByPosition: boolean = false;
	// Whether slides that make up a whole section of the source go in as a
	// section of their own rather than as plain slides. The user's choice, kept
	// across files.
	public insertAsSection: boolean = true;
	// Whether the slides on show can be linked to their source.
	public canLink: boolean = false;
	public error: SlideImportError | null = null;

	private map: any;
	// Insert commands sent whose reply has not arrived yet.
	private pendingInserts: number = 0;
	// The insert waiting on the source to write its pages.
	private pendingInsert: SlideImportInsert | null = null;

	constructor(map: any) {
		this.map = map;

		map.on('slideimport', this.onImportMessage, this);
		map.on('slideimporterror', this.onErrorMessage, this);

		app.events.on('slideimport:dropinsert', this.onDropInsert.bind(this));
	}

	// The pages the source wrote are staged in this document's jail, and the insert takes that
	// file. The pane asks for the export and hands the staged name over. section is the section
	// of the source the slides go in as, which they make up all of and nothing else, or null for
	// slides that go in as slides. The section is named to the document by the identifier and
	// the name it has in the source, and a linked section follows that source section. The
	// pages of a whole insert go in as one presentation linked to the source, whatever the
	// link choice and the section say, since linking is what a whole insert is for. The
	// insert itself is the pending one.
	public insertStaged(
		stagedName: string,
		section: SlideImportSection | null,
	): void {
		const insert = this.pendingInsert;
		if (!insert) return;
		const link = this.canLink && (insert.whole || this.linkToSource);
		const source = this.sourceName();
		// A linked page records the time its source was last modified now, so a
		// later comparison tells whether it is up to date.
		const time = link ? SlideImportSession.sourceModifiedTime(source) : '';
		// A page linked by position records the place its slide holds in the source, which is
		// the slide this insert asked the source for, counted from one.
		const positions =
			link && this.linkByPosition
				? insert.slides.map((index: number) => index + 1)
				: [];
		app.socket.sendMessage(
			'slideimport insert file=' +
				encodeURIComponent(stagedName) +
				(source ? ' source=' + encodeURIComponent(source) : '') +
				' at=' +
				insert.at +
				' keepdesign=' +
				(this.keepDesign ? '1' : '0') +
				' link=' +
				(link ? '1' : '0') +
				(positions.length ? ' sourcepositions=' + positions.join(',') : '') +
				(time ? ' time=' + encodeURIComponent(time) : '') +
				SlideImportSession.insertUnit(insert, section),
		);
	}

	// The tail of the insert command that says what the pages go in as: the whole file as a
	// presentation under the name the insert records, the given section, or plain slides.
	private static insertUnit(
		insert: SlideImportInsert,
		section: SlideImportSection | null,
	): string {
		if (insert.whole)
			return ' as=presentation name=' + encodeURIComponent(insert.name);
		if (section)
			return (
				' as=section id=' +
				encodeURIComponent(section.id) +
				' name=' +
				encodeURIComponent(SlideImportSession.recordableName(section.name))
			);
		return ' as=slides';
	}

	// The given name as a link records it: without control characters and at most 128 UTF-16
	// code units long, which is as much of a name as the document keeps, cut so that a
	// character of two code units stays whole.
	public static recordableName(name: string): string {
		let kept = '';
		for (const character of name)
			if (character.charCodeAt(0) >= 0x20) kept += character;
		if (kept.length <= 128) return kept;
		kept = kept.substring(0, 128);
		const last = kept.charCodeAt(127);
		return last >= 0xd800 && last <= 0xdbff ? kept.substring(0, 127) : kept;
	}

	// The persistent link of the document the slides come from. An error and its retry belong
	// to the insert of the source left behind, so they go with it.
	public setSource(fileName: string): void {
		this.fileName = fileName;
		this.canLink = this.sourceName() !== '' && this.formatKeepsLinks();
		this.setError(null);
		this.setState('opening');
	}

	// Does the format this document is saved in keep the link of an inserted page.
	public formatKeepsLinks(): boolean {
		return app.LOUtil.isFileODF(this.map);
	}

	// The slides of the document are on show, so they can be chosen and inserted.
	public slidesShown(): void {
		this.setError(null);
		this.setState('ready');
	}

	private sourceName(): string {
		const name = this.fileName;
		if (name === '') return '';
		for (let i = 0; i < name.length; i++)
			if (name.charCodeAt(i) < 0x20) return '';
		return name;
	}

	public setKeepDesign(value: boolean): void {
		this.keepDesign = value;
	}

	public setLinkToSource(value: boolean): void {
		this.linkToSource = value;
	}

	public setInsertAsSection(value: boolean): void {
		this.insertAsSection = value;
	}

	public setLinkByPosition(value: boolean): void {
		this.linkByPosition = value;
	}

	public toggleSelection(index: number): void {
		if (this.selection.has(index)) this.selection.delete(index);
		else this.selection.add(index);
		this.fireEvent('slideimport:selection', {});
	}

	public selectOnly(index: number): void {
		this.selection.clear();
		this.selection.add(index);
		this.fireEvent('slideimport:selection', {});
	}

	// Replaces the selection with the given slide indices.
	public setSelection(indices: number[]): void {
		this.selection.clear();
		for (const index of indices) this.selection.add(index);
		this.fireEvent('slideimport:selection', {});
	}

	// Takes the given slides into the selection or out of it, and leaves the
	// rest of the selection as it is.
	public setSelected(indices: number[], selected: boolean): void {
		for (const index of indices) {
			if (selected) this.selection.add(index);
			else this.selection.delete(index);
		}
		this.fireEvent('slideimport:selection', {});
	}

	public selectRange(anchor: number, index: number): void {
		const from = Math.max(0, Math.min(anchor, index));
		const to = Math.min(this.slideCount - 1, Math.max(anchor, index));
		const indices = [];
		for (let i = from; i <= to; i++) indices.push(i);
		this.setSelection(indices);
	}

	public clearSelection(): void {
		this.selection.clear();
		this.fireEvent('slideimport:selection', {});
	}

	// The slide the next insert lands after, counted from one, or 0 for the
	// end of the document.
	public insertAfter(): number {
		const docLayer = this.map._docLayer;
		if (!docLayer || docLayer._selectedPart === undefined) return 0;
		return docLayer._selectedPart + 1;
	}

	// Inserts the selected slides after the slide the user is on.
	public insertSelected(): void {
		if (this.state !== 'ready' || this.selection.size === 0) return;

		const slides = Array.from(this.selection).sort((a, b) => a - b);
		this.requestInsert({
			slides: slides,
			at: this.insertAt(),
			whole: false,
			name: '',
		});
	}

	// Inserts every slide of the source after the slide the user is on, linked as one
	// presentation that records the source under the given name, as far as a link keeps it.
	public insertWhole(name: string): void {
		if (this.state !== 'ready' || !this.canLink) return;

		this.requestInsert({
			slides: [],
			at: this.insertAt(),
			whole: true,
			name: SlideImportSession.recordableName(name),
		});
	}

	// The slide the pages of an insert go in before, counted from zero: the one after the
	// slide the user is on, or the count of slides when no slide is on show, which puts them
	// at the end.
	private insertAt(): number {
		const docLayer = this.map._docLayer;
		return docLayer && docLayer._selectedPart !== undefined
			? docLayer._selectedPart + 1
			: this.map.getNumberOfParts();
	}

	// A thumbnail dragged onto the slides list drops here. pos follows the
	// reorder convention: -1 places the slides before the first slide, and
	// a value k places them after slide k. slideimport insert takes an at
	// index that inserts before a slide, so at is one past pos.
	private onDropInsert(e: any): void {
		if (this.state !== 'ready') return;
		const detail = e.detail || {};
		const wanted = (Array.isArray(detail.slides) ? detail.slides : [])
			.filter((i: number) => i >= 0 && i < this.slideCount)
			.sort((a: number, b: number) => a - b);
		if (wanted.length === 0) return;
		const pos = typeof detail.pos === 'number' ? detail.pos : -1;
		this.requestInsert({ slides: wanted, at: pos + 1, whole: false, name: '' });
	}

	public close(): void {
		this.fileName = '';
		this.canLink = false;
		this.resetImportState();
		this.setError(null);
		this.setState('idle');
	}

	private setState(state: SlideImportState): void {
		this.state = state;
		this.fireEvent('slideimport:statechange', { state: state });
	}

	// Says why the slides of a pick are not on show.
	public showError(message: string): void {
		this.setError({ message: message, retry: null });
	}

	public exportFailed(): void {
		if (this.pendingInserts > 0) this.pendingInserts--;
		const insert = this.pendingInsert;
		this.pendingInsert = null;
		if (this.state === 'inserting') this.setState('ready');
		this.setError({
			message: _('The slides of the source could not be read.'),
			retry: insert ? () => this.requestInsert(insert) : null,
		});
	}

	private setError(error: SlideImportError | null): void {
		if (error === null && this.error === null) return;
		this.error = error;
		this.fireEvent('slideimport:error', {});
	}

	private fireEvent(name: string, detail: any): void {
		app.events.fire(name, detail);
	}

	private resetImportState(): void {
		this.slideCount = 0;
		this.selection.clear();
		this.pendingInsert = null;
	}

	private onImportMessage(e: any): void {
		const message = e.message;
		if (!message || !message.status) return;

		if (message.status === 'inserted') {
			this.pendingInserts = Math.max(0, this.pendingInserts - 1);
			// The slides of the insert are in the document, so nothing is waiting to be
			// inserted whatever state the pane has moved on to.
			const insert = this.pendingInsert;
			this.pendingInsert = null;
			// The insert this answers was abandoned by a close or a new
			// file pick; the session has moved on.
			if (this.state !== 'inserting') return;
			// The pick is in, so it is done with. A whole insert takes no pick, so the slides
			// picked meanwhile stay picked.
			if (!insert || !insert.whole) this.selection.clear();
			this.setState('ready');
			this.fireEvent('slideimport:inserted', { count: message.count });
		}
	}

	private onErrorMessage(e: any): void {
		// Every kind an insert can be answered with reaches the insert below: cantload says
		// the file the source wrote is no longer staged, and the others that the document
		// refused the pages. A kind that arrives while nothing is being inserted belongs to
		// an insert the pane has already moved on from.
		if (this.pendingInserts > 0 && this.state !== 'ready') {
			this.pendingInserts--;
			if (this.state === 'inserting') {
				const insert = this.pendingInsert;
				this.pendingInsert = null;
				this.setState('ready');
				this.setError({
					message: _('Inserting the slides failed.'),
					retry: insert ? () => this.requestInsert(insert) : null,
				});
			} else {
				// The pane was closed while the insert was still running.
				// The document was not changed, so say so out loud.
				this.map.fire('error', {
					msg: _('Inserting the slides failed.'),
					critical: false,
				});
			}
			return;
		}

		if (this.state === 'opening') {
			this.setState('idle');
			this.setError({ message: _('Importing slides failed.'), retry: null });
		}
	}

	// The insert a user asked for. The pane asks the source to write the pages out and hands
	// back the name they were staged under.
	public requestInsert(insert: SlideImportInsert): void {
		this.pendingInsert = insert;
		this.pendingInserts++;
		this.setError(null);
		this.setState('inserting');
		this.fireEvent('slideimport:exportwanted', {});
	}

	/// The insert being prepared, with its slides in the order they were chosen.
	public getPendingInsert(): SlideImportInsert | null {
		return this.pendingInsert;
	}

	// The remote link as the user knows it: the name the server gave it, or
	// its persistent link when the server gave none.
	public static documentName(doc: {
		name?: string;
		persistentLink: string;
	}): string {
		return doc.name || doc.persistentLink;
	}

	// Whether a remote link is the one a source names.
	public static matchesDocument(
		doc: { persistentLink?: string },
		source: string,
	): boolean {
		return !!doc.persistentLink && doc.persistentLink === source;
	}

	// The time the remote link of the given name was last modified now, as
	// the storage announced it, or empty when the storage named no such
	// document or gave it no time.
	public static sourceModifiedTime(source: string): string {
		if (!source) return '';
		for (const doc of app.remoteLinks || []) {
			if (
				SlideImportSession.matchesDocument(doc, source) &&
				doc.lastModifiedTime
			)
				return doc.lastModifiedTime;
		}
		return '';
	}

	// Asks the server to open a live link to the remote link bound to a
	// persistent link; the state in the next remotelinks: message follows the
	// subscription.
	public static subscribeRemoteLink(persistentLink: string): void {
		app.socket.sendMessage(
			'remotedocsubscribe source=' + encodeURIComponent(persistentLink),
		);
	}

	// Asks the server to look for the document behind a persistent link.
	public static resolveRemoteLink(persistentLink: string): void {
		app.socket.sendMessage(
			'remotelinkresolve source=' + encodeURIComponent(persistentLink),
		);
	}

	// Asks the server to drop the remote link bound to a persistent link, for
	// every view of the document.
	public static removeRemoteLink(persistentLink: string): void {
		app.socket.sendMessage(
			'remotelinkremove source=' + encodeURIComponent(persistentLink),
		);
	}

	// Sends a read-only client command to the subscribed remote document bound
	// to a persistent link. Its reply arrives as a remotedoccommandresult map
	// event carrying the same source.
	public static sendRemoteCommand(persistentLink: string, inner: string): void {
		app.socket.sendMessage(
			'remotedoccommand source=' +
				encodeURIComponent(persistentLink) +
				' ' +
				inner,
		);
	}

	public static exportFailureKind(json: string): string {
		try {
			const written = JSON.parse(json);
			return written && written.status === 'failed' && written.kind
				? written.kind
				: '';
		} catch {
			return '';
		}
	}

	// The name the server staged an exported presentation under, from the
	// body of an exportslides: reply, or an empty string when nothing was
	// staged.
	public static stagedExportName(json: string): string {
		try {
			const written = JSON.parse(json);
			return written && written.status === 'staged' && written.name
				? written.name
				: '';
		} catch {
			return '';
		}
	}
}
