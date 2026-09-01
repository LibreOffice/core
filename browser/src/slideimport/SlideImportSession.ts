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
 * Owns the lifecycle of one import: the questions asked of the related
 * document the slides come from, the per-slide thumbnail store, the slide
 * selection, the keep-design flag and the link-to-source flag.
 * Every state change is announced as a slideimport:* event on app.events.
 *
 * A question of a related document carries an id, and its answer comes back
 * under that id, so the session knows which of its questions each answer
 * belongs to. The kit's replies to an insert carry no such id and are
 * matched by the state the session is in.
 */

/* global app _ */

type SlideImportState = 'idle' | 'opening' | 'ready' | 'inserting';

interface SlideImportSlide {
	index: number;
	name: string;
}

interface SlideImportSection {
	name: string;
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

class SlideImportSession {
	public state: SlideImportState = 'idle';
	public fileName: string = '';
	public slideCount: number = 0;
	public slides: SlideImportSlide[] = [];
	// The sections of the opened file, in slide order; empty when the file
	// groups no slides into sections.
	public sections: SlideImportSection[] = [];
	// The slide size of the opened file, in twips. Zero until a file is open.
	public size: { width: number; height: number } = { width: 0, height: 0 };
	public thumbnails: Map<number, SlideImportThumbnail> = new Map();
	public selection: Set<number> = new Set();
	public keepDesign: boolean = false;
	// Whether the inserted slides stay bound to the file they came from, so
	// that they can be updated from it later. The user's choice, kept across
	// files.
	public linkToSource: boolean = false;
	// Whether the open of the current file named it as a source. Slides can
	// only be linked to a source the open named.
	public canLink: boolean = false;
	public error: SlideImportError | null = null;

	private map: any;
	// Insert commands sent whose reply has not arrived yet.
	private pendingInserts: number = 0;
	// The slides an insert is waiting on the source to write, and where they go.
	private pendingInsert: { slides: number[]; at: number } | null = null;

	constructor(map: any) {
		this.map = map;

		map.on('slideimport', this.onImportMessage, this);
		map.on('slideimporterror', this.onErrorMessage, this);

		app.events.on('slideimport:dropinsert', this.onDropInsert.bind(this));
	}

	// The pages the source wrote are staged in this document's jail, and the insert takes that
	// file. The pane asks for the export and hands the staged name over.
	public insertStaged(stagedName: string, at: number): void {
		const link = this.linkToSource && this.canLink;
		const source = this.sourceName();
		// A linked page records the time its source was last modified now, so a
		// later comparison tells whether it is up to date.
		const time = link ? SlideImportSession.sourceModifiedTime(source) : '';
		app.socket.sendMessage(
			'slideimport insert file=' +
				encodeURIComponent(stagedName) +
				(source ? ' source=' + encodeURIComponent(source) : '') +
				' at=' +
				at +
				' keepdesign=' +
				(this.keepDesign ? '1' : '0') +
				' link=' +
				(link ? '1' : '0') +
				(time ? ' time=' + encodeURIComponent(time) : ''),
		);
	}

	// The document the slides come from, as the user knows it. Pages inserted as links record
	// it, and a later refresh finds the related document of that name.
	public setSource(fileName: string): void {
		this.fileName = fileName;
		this.canLink = this.sourceName() !== '';
		this.setState('opening');
	}

	// The slides of the document are on show, so they can be chosen and inserted.
	public slidesShown(): void {
		this.setError(null);
		this.setState('ready');
	}

	private sourceName(): string {
		const name = this.fileName;
		if (name === '' || name === '.' || name === '..') return '';
		if (name.indexOf('/') >= 0 || name.indexOf('\\') >= 0) return '';
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

	// Inserts the selected slides after the slide the user is on.
	public insertSelected(): void {
		if (this.state !== 'ready' || this.selection.size === 0) return;

		const slides = Array.from(this.selection).sort((a, b) => a - b);
		const docLayer = this.map._docLayer;
		const at =
			docLayer && docLayer._selectedPart !== undefined
				? docLayer._selectedPart + 1
				: this.map.getNumberOfParts();
		this.requestInsert(slides, at);
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
		this.requestInsert(wanted, pos + 1);
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

	private setError(error: SlideImportError | null): void {
		if (error === null && this.error === null) return;
		this.error = error;
		this.fireEvent('slideimport:error', {});
	}

	private fireEvent(name: string, detail: any): void {
		app.events.fire(name, detail);
	}

	private resetImportState(): void {
		this.thumbnails.clear();
		this.slides = [];
		this.sections = [];
		this.slideCount = 0;
		this.size = { width: 0, height: 0 };
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
			this.pendingInsert = null;
			// The insert this answers was abandoned by a close or a new
			// file pick; the session has moved on.
			if (this.state !== 'inserting') return;
			this.selection.clear();
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
					retry: insert
						? () => this.requestInsert(insert.slides, insert.at)
						: null,
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

	// The slides a user chose, and where they go. The pane asks the source to write them out
	// and hands back the name they were staged under.
	public requestInsert(slides: number[], at: number): void {
		this.pendingInsert = { slides: slides, at: at };
		this.pendingInserts++;
		this.setError(null);
		this.setState('inserting');
		this.fireEvent('slideimport:exportwanted', {});
	}

	/// The slides the insert being prepared asks for, in the order they were chosen.
	public getPendingInsert(): { slides: number[]; at: number } | null {
		return this.pendingInsert;
	}

	// The name of a related document as the user knows it: the file name at
	// the end of its address.
	public static relatedDocumentName(wopiSrc: string): string {
		const path = wopiSrc.split('?')[0];
		const name = path.substring(path.lastIndexOf('/') + 1);
		try {
			return decodeURIComponent(name) || wopiSrc;
		} catch {
			return name || wopiSrc;
		}
	}

	// The time the related document of the given name was last modified now, as
	// the storage announced it, or empty when the storage named no such
	// document or gave it no time.
	public static sourceModifiedTime(source: string): string {
		if (!source) return '';
		for (const doc of app.relatedDocuments || []) {
			if (
				SlideImportSession.relatedDocumentName(doc.wopiSrc) === source &&
				doc.lastModifiedTime
			)
				return doc.lastModifiedTime;
		}
		return '';
	}

	// The related document with the given address, or null when the storage
	// named none of that address.
	public static findRelatedDocument(
		wopiSrc: string,
	): { wopiSrc: string; state: string } | null {
		return (
			(app.relatedDocuments || []).find(
				(doc: { wopiSrc: string }) => doc.wopiSrc === wopiSrc,
			) || null
		);
	}

	// Asks the server to open a live link to the related document; the state
	// in the next relateddocuments: message follows the subscription.
	public static subscribeRelatedDocument(wopiSrc: string): void {
		app.socket.sendMessage(
			'remotedocsubscribe wopisrc=' + encodeURIComponent(wopiSrc),
		);
	}

	// Sends a read-only client command to a subscribed remote document. Its
	// reply arrives as a remotedoccommandresult map event carrying the same
	// wopiSrc.
	public static sendRemoteCommand(wopiSrc: string, inner: string): void {
		app.socket.sendMessage(
			'remotedoccommand wopisrc=' + encodeURIComponent(wopiSrc) + ' ' + inner,
		);
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
