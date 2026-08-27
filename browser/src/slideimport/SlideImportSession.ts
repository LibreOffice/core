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
 * Owns the lifecycle of one imported presentation: staging the picked file
 * on the server, the slideimport command exchange with the kit, the
 * per-slide thumbnail store, the slide selection, the keep-design flag and
 * the link-to-source flag.
 * Every state change is announced as a slideimport:* event on app.events.
 *
 * Replies from the kit carry no request token, so the session correlates
 * them by order: pendingOpens counts the open commands whose reply (the
 * inventory or a cantload error) has not arrived yet, and only the reply
 * to the newest open is adopted. Staging events from the uploader are
 * correlated by the staged name, which this class generates.
 */

/* global app _ */

type SlideImportState = 'idle' | 'staging' | 'opening' | 'ready' | 'inserting';

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
	// The picked file is kept so a failed upload or open can be retried
	// without asking the user to pick it again.
	private pendingFile: File | null = null;
	// A remote location picked instead of a local file, kept for the same
	// retry reason. Only one of pendingFile and pendingUrl is set at a time.
	private pendingUrl: { url: string; fileName: string } | null = null;
	// The staged name of the file this session is currently waiting on.
	// Staging events for any other name belong to an abandoned pick.
	private stagingToken: string | null = null;
	// Open commands sent whose reply has not arrived yet. Only the reply
	// that brings this count to zero describes the current file.
	private pendingOpens: number = 0;
	// Insert commands sent whose reply has not arrived yet.
	private pendingInserts: number = 0;
	private lastInsert: { slides: number[]; at: number } | null = null;
	private thumbnailQueue: number[] = [];
	private thumbnailBatch: number[] = [];
	private thumbnailsInFlight: Set<number> = new Set();

	private static stagingSerial: number = 0;

	// A thumbnail request names at most this many slides.
	private readonly THUMBNAIL_BATCH = 8;
	// CSS width a thumbnail is displayed at, matching the slide navigator's
	// preview width so imported slides read at the same scale as the
	// document's own slides.
	private readonly THUMBNAIL_CSS_WIDTH = 180;

	constructor(map: any) {
		this.map = map;

		map.on('slideimport', this.onImportMessage, this);
		map.on('slideimportthumbnail', this.onThumbnailMessage, this);
		map.on('slideimporterror', this.onErrorMessage, this);
		map.on('slideimportstaged', this.onStaged, this);
		map.on('slideimportstagingfailed', this.onStagingFailed, this);

		app.events.on('slideimport:dropinsert', this.onDropInsert.bind(this));
	}

	// Opens the local file picker.
	public pickFile(): void {
		window.app.console.log('local slide import not implemented yet');
	}

	public stageFile(file: File): void {
		this.pendingFile = file;
		this.pendingUrl = null;
		this.fileName = file.name;
		this.stagingToken =
			'slideimport-' + ++SlideImportSession.stagingSerial + '-' + Date.now();
		const source = this.sourceName();
		this.canLink = source !== '';
		this.resetImportState();
		this.setError(null);
		this.setState('staging');
		this.map.fire('slideimportfile', {
			file: file,
			name: this.stagingToken,
			source: source,
		});
	}

	// Opens an import document from a location the integration picked. There
	// is no upload step: the server reads the location itself, so the open
	// command is sent straight away.
	public openUrl(url: string, fileName: string): void {
		this.pendingFile = null;
		this.pendingUrl = { url: url, fileName: fileName };
		this.fileName = fileName;
		this.stagingToken = null;
		const source = this.sourceName();
		this.canLink = source !== '';
		this.resetImportState();
		this.setError(null);
		this.pendingOpens++;
		this.setState('opening');
		app.socket.sendMessage(
			'slideimport open name=' +
				encodeURIComponent(fileName) +
				(source ? ' source=' + encodeURIComponent(source) : '') +
				' url=' +
				encodeURIComponent(url),
		);
	}

	// The name the inserted slides record as the document they came from,
	// which is the name of the picked file, or an empty string for a name a
	// source is never known by: an empty one, a directory reference, one
	// holding a path separator, and one holding a control character.
	private sourceName(): string {
		const name = this.fileName;
		if (name === '' || name === '.' || name === '..') return '';
		if (name.indexOf('/') >= 0 || name.indexOf('\\') >= 0) return '';
		for (let i = 0; i < name.length; i++)
			if (name.charCodeAt(i) < 0x20) return '';
		return name;
	}

	// How to retry opening the import document the user is waiting on, or
	// null when nothing was picked to retry.
	private retryOpen(): (() => void) | null {
		if (this.pendingFile) {
			const file = this.pendingFile;
			return () => this.stageFile(file);
		}
		if (this.pendingUrl) {
			const pick = this.pendingUrl;
			return () => this.openUrl(pick.url, pick.fileName);
		}
		return null;
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
		this.sendInsert({ slides: slides, at: at });
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
		this.sendInsert({ slides: wanted, at: pos + 1 });
	}

	public close(): void {
		if (this.state === 'idle') return;
		// During staging the open command has not been sent yet; the open
		// reply handler closes an unwanted import document when it arrives.
		if (this.state !== 'staging') {
			app.socket.sendMessage('slideimport close');
		}
		this.pendingFile = null;
		this.pendingUrl = null;
		this.stagingToken = null;
		this.fileName = '';
		this.canLink = false;
		this.resetImportState();
		this.setError(null);
		this.setState('idle');
	}

	// Moves the given slide indices to the front of the thumbnail queue,
	// so slides scrolled into view render before off-screen ones.
	public prioritizeThumbnails(indices: number[]): void {
		const wanted = indices.filter((i) => this.thumbnailQueue.includes(i));
		if (wanted.length === 0) return;
		this.thumbnailQueue = wanted.concat(
			this.thumbnailQueue.filter((i) => !wanted.includes(i)),
		);
	}

	private setState(state: SlideImportState): void {
		this.state = state;
		this.fireEvent('slideimport:statechange', { state: state });
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
		this.thumbnailQueue = [];
		this.thumbnailBatch = [];
		this.thumbnailsInFlight.clear();
		this.lastInsert = null;
	}

	private onStaged(e: any): void {
		// Every staged upload sends one open command, including an upload
		// the user has already abandoned; count them all so the open
		// replies can be matched to the newest open.
		this.pendingOpens++;
		if (e.name !== this.stagingToken) return;
		if (this.state === 'staging') this.setState('opening');
	}

	private onStagingFailed(e: any): void {
		if (e.name !== this.stagingToken) return;
		if (this.state !== 'staging') return;
		const file = this.pendingFile;
		const retryable = e.retryable !== false;
		this.stagingToken = null;
		this.setState('idle');
		this.setError({
			message: e.message ? e.message : _('The file could not be uploaded.'),
			retry: retryable && file ? () => this.stageFile(file) : null,
		});
	}

	private onImportMessage(e: any): void {
		const message = e.message;
		if (!message || !message.status) return;

		if (message.status === 'open') {
			this.pendingOpens = Math.max(0, this.pendingOpens - 1);
			// An older open succeeded, but a newer one is outstanding and
			// has already replaced this import document in the kit.
			if (this.pendingOpens > 0) return;
			// Only a file this session is actively opening describes the
			// current pane. In any other state the reply is for an import
			// document the kit has since moved on from: the pane was closed
			// during the round trip (idle), or a newer pick is still
			// uploading and its own open has not been sent yet (staging), so
			// this reply belongs to a file the newer pick supersedes. Drop it
			// and tell the kit to close the import document it just opened.
			if (this.state !== 'opening') {
				app.socket.sendMessage('slideimport close');
				return;
			}
			this.stagingToken = null;
			this.slideCount = message.slideCount;
			this.size = message.size;
			this.slides = message.slides;
			this.sections = Array.isArray(message.sections) ? message.sections : [];
			this.setError(null);
			this.setState('ready');
			this.fireEvent('slideimport:inventory', {});
			this.thumbnailQueue = this.slides.map((slide) => slide.index);
			this.requestNextThumbnailBatch();
		} else if (message.status === 'inserted') {
			this.pendingInserts = Math.max(0, this.pendingInserts - 1);
			// The insert this answers was abandoned by a close or a new
			// file pick; the session has moved on.
			if (this.state !== 'inserting') return;
			this.lastInsert = null;
			this.selection.clear();
			this.setState('ready');
			this.fireEvent('slideimport:inserted', { count: message.count });
			this.requestNextThumbnailBatch();
		}
		// status 'closed' needs no handling: the session state was already
		// reset when the close was sent.
	}

	private onThumbnailMessage(e: any): void {
		const index = e.message.index;
		// A thumbnail that was never requested for the current file is a
		// late reply for a replaced import document.
		if (!this.thumbnailsInFlight.has(index)) return;
		this.thumbnails.set(index, {
			url: this.pngBytesToDataUrl(e.imgBytes),
			width: e.message.width,
			height: e.message.height,
		});
		this.thumbnailsInFlight.delete(index);
		this.fireEvent('slideimport:thumbnail', { index: index });
		if (this.thumbnailsInFlight.size === 0) {
			this.requestNextThumbnailBatch();
		}
	}

	// The content security policy allows data: URLs but not blob: URLs for
	// images, so encode the PNG bytes of a thumbnail as a data URL. The
	// bytes are converted in chunks so a large thumbnail does not overflow
	// the argument stack of String.fromCharCode.
	private pngBytesToDataUrl(bytes: Uint8Array): string {
		let binary = '';
		const chunk = 0x8000;
		for (let i = 0; i < bytes.length; i += chunk) {
			binary += String.fromCharCode.apply(
				null,
				bytes.subarray(i, i + chunk) as unknown as number[],
			);
		}
		return 'data:image/png;base64,' + window.btoa(binary);
	}

	private onErrorMessage(e: any): void {
		const kind = e.kind || '';

		// A cantload reply answers the oldest outstanding open.
		if (kind === 'cantload') {
			this.pendingOpens = Math.max(0, this.pendingOpens - 1);
			// A newer open is outstanding; its reply supersedes this one.
			if (this.pendingOpens > 0) return;
			// Only surface the failure for a file the pane is opening now; in
			// any other state a newer pick has already superseded this one.
			if (this.state !== 'opening') return;
			const retry = this.retryOpen();
			this.stagingToken = null;
			this.setState('idle');
			this.setError({
				message: _('This file cannot be imported as slides.'),
				retry: retry,
			});
			return;
		}

		// A thumbnail render failure is reported on its own, so it is never
		// mistaken for an insert failure when a batch is still rendering as
		// the user inserts. It re-queues the rest of the batch and never
		// touches the insert.
		if (kind === 'thumbnailfailed') {
			this.retryThumbnailsAfterFailure();
			return;
		}

		if (this.pendingInserts > 0 && this.state !== 'ready') {
			this.pendingInserts--;
			if (this.state === 'inserting') {
				const insert = this.lastInsert;
				this.setState('ready');
				this.setError({
					message: _('Inserting the slides failed.'),
					retry: insert ? () => this.sendInsert(insert) : null,
				});
				this.requestNextThumbnailBatch();
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
			this.pendingOpens = Math.max(0, this.pendingOpens - 1);
			if (this.pendingOpens > 0) return;
			const retry = this.retryOpen();
			this.stagingToken = null;
			this.setState('idle');
			this.setError({
				message: _('Importing slides failed.'),
				retry: retry,
			});
		}
	}

	// A thumbnail batch failed to render. The kit stops the batch at the
	// first slide it cannot render, so the first index still in flight is
	// the failed one; it keeps its skeleton frame and the rest of the batch
	// go back on the queue. A new batch waits until the pane is ready again,
	// so a failure during an insert resumes once the insert finishes.
	private retryThumbnailsAfterFailure(): void {
		if (this.thumbnailsInFlight.size === 0) return;
		const remaining = this.thumbnailBatch.filter((i) =>
			this.thumbnailsInFlight.has(i),
		);
		this.thumbnailQueue = remaining.slice(1).concat(this.thumbnailQueue);
		this.thumbnailsInFlight.clear();
		this.requestNextThumbnailBatch();
	}

	private sendInsert(insert: { slides: number[]; at: number }): void {
		this.lastInsert = insert;
		this.pendingInserts++;
		this.setError(null);
		this.setState('inserting');
		// Slides are linked to the source the open named, so an import
		// document opened without one inserts plain copies.
		const link = this.linkToSource && this.canLink;
		app.socket.sendMessage(
			'slideimport insert slides=' +
				insert.slides.join(',') +
				' at=' +
				insert.at +
				' keepdesign=' +
				(this.keepDesign ? '1' : '0') +
				' link=' +
				(link ? '1' : '0'),
		);
	}

	private requestNextThumbnailBatch(): void {
		if (this.state !== 'ready') return;
		if (this.thumbnailsInFlight.size > 0) return;
		const batch = this.thumbnailQueue.splice(0, this.THUMBNAIL_BATCH);
		if (batch.length === 0) return;
		this.thumbnailBatch = batch;
		batch.forEach((index) => this.thumbnailsInFlight.add(index));
		app.socket.sendMessage(
			'slideimport thumbnails slides=' +
				batch.join(',') +
				' width=' +
				this.thumbnailWidth(),
		);
	}

	private thumbnailWidth(): number {
		const scale = window.devicePixelRatio || 1;
		const width = Math.round(this.THUMBNAIL_CSS_WIDTH * scale);
		return Math.min(512, Math.max(32, width));
	}

	// Empties the shown slides, their sections, sizes, thumbnails and the
	// selection, so the list can be refilled from a source other than a local
	// file import.
	public reset(): void {
		this.resetImportState();
	}
}
