/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Posts Doc_PartChanged when the current part, the part count or the document type changes. The
// posted Part counts from 1. An event that repeats the last values posts nothing. So does a part
// event that lacks one of the three values, or a page event that lacks the page or the page count.
class DocPartChanged {
	private post: (message: { msgId: string; args: any }) => void;
	private lastPart = -1;
	private lastPartCount = -1;
	private lastDocType: string | undefined = '';

	constructor(post: (message: { msgId: string; args: any }) => void) {
		this.post = post;
	}

	public listenTo(map: any): void {
		map.on('pagenumberchanged', (e: any) => this.onPageNumberChanged(e), map);
		map.on('setpart', (e: any) => this.onSetPart(e), map);
		map.on('updateparts', (e: any) => this.onUpdateParts(e), map);
	}

	public onPageNumberChanged(e: any): void {
		if (!e) return;

		this.update(e.currentPage, e.pages, e.docType);
	}

	public onSetPart(e: any): void {
		this.onPartEvent(e);
	}

	public onUpdateParts(e: any): void {
		this.onPartEvent(e);
	}

	private onPartEvent(e: any): void {
		if (
			!e ||
			e.selectedPart === undefined ||
			e.parts === undefined ||
			e.docType === undefined
		)
			return;

		this.update(e.selectedPart, e.parts, e.docType);
	}

	private update(part: any, partCount: any, docType: string | undefined): void {
		const normalizedPart = Number(part);
		const normalizedPartCount = Number(partCount);

		if (
			!Number.isInteger(normalizedPart) ||
			!Number.isInteger(normalizedPartCount)
		)
			return;

		if (
			normalizedPart === this.lastPart &&
			normalizedPartCount === this.lastPartCount &&
			docType === this.lastDocType
		)
			return;

		this.lastPart = normalizedPart;
		this.lastPartCount = normalizedPartCount;
		this.lastDocType = docType;
		this.post({
			msgId: 'Doc_PartChanged',
			args: {
				Part: normalizedPart + 1,
				PartCount: normalizedPartCount,
				DocType: docType,
			},
		});
	}
}
