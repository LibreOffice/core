// @ts-strict-ignore
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

class URLPopUpSection extends HTMLObjectSection {
    static sectionName = 'URL PopUp';
	containerId = 'hyperlink-pop-up-preview';
	linkId = 'hyperlink-pop-up';
	static cssClass = 'hyperlink-pop-up-container';
	copyButtonId = 'hyperlink-pop-up-copy';
	editButtonId = 'hyperlink-pop-up-edit';
	removeButtonId = 'hyperlink-pop-up-remove';
	static horizontalPadding = 5;
	static popupVerticalMargin = 12;

	preview: HTMLDivElement;
	link: HTMLAnchorElement;

	constructor(url: string, documentPosition: cool.SimplePoint, linkPosition?: cool.SimplePoint, linkIsClientSide = false) {
        super(URLPopUpSection.sectionName, null, null, documentPosition, URLPopUpSection.cssClass);

		const objectDiv = this.getHTMLObject();
		objectDiv.remove();
		document.getElementById('document-container').appendChild(objectDiv);

		this.sectionProperties.url = url;
		this.sectionProperties.linkIsClientSide = linkIsClientSide;

		this.createUIElements(url);
		this.setUpCallbacks(linkPosition);

		this.link.title = url;

		if (app.map['wopi'].EnableRemoteLinkPicker)
			app.map.fire('postMessage', { msgId: 'Action_GetLinkPreview', args: { url: url } });

		this.sectionProperties.documentPosition = documentPosition.clone();
		this.getHTMLObject().style.pointerEvents = '';
    }

	adjustHTMLObjectPosition() {
		if (app.map._docLayer.isCalc()) {
			super.adjustHTMLObjectPosition();
			return;
		}

		const divBoundingRectangle = this.sectionProperties.objectDiv.getBoundingClientRect();

		const left = Math.round((this.sectionProperties.documentPosition.vX / app.dpiScale)) + 'px';
		const top = Math.round((this.sectionProperties.documentPosition.vY / app.dpiScale) - divBoundingRectangle.height) + 'px';

		if (this.sectionProperties.objectDiv.style.left !== left)
			this.sectionProperties.objectDiv.style.left = left;

		if (this.sectionProperties.objectDiv.style.top !== top)
			this.sectionProperties.objectDiv.style.top = top;
	}

	getPopUpWidth(): number {
		return this.getHTMLObject().getBoundingClientRect().width;
	}

	getPopUpHeight(): number {
		return this.getHTMLObject().getBoundingClientRect().height;
	}

	getPopUpBoundingRectangle() {
		return this.getHTMLObject().getBoundingClientRect();
	}

	createUIElements(url: string) {
		const parent = this.getHTMLObject();
		this.preview = window.L.DomUtil.createWithId('div', this.containerId, parent) as HTMLDivElement;

		const linkRow = window.L.DomUtil.create('div', 'hyperlink-pop-up-link-row', parent);

        this.link = window.L.DomUtil.createWithId('a', this.linkId, linkRow) as HTMLAnchorElement;
		this.link.innerText = url;
		const copyLinkText = _('Copy link location');
		const copyBtn = window.L.DomUtil.createWithId('div', this.copyButtonId, linkRow);
		window.L.DomUtil.addClass(copyBtn, 'hyperlink-popup-btn');
		copyBtn.setAttribute('title', copyLinkText);
		copyBtn.setAttribute('role', 'button');
		copyBtn.setAttribute('aria-label', copyLinkText);

        const imgCopyBtn = window.L.DomUtil.create('img', 'hyperlink-pop-up-copyimg', copyBtn);
		app.LOUtil.setImage(imgCopyBtn, 'lc_copyhyperlinklocation.svg', app.map);
		imgCopyBtn.setAttribute('width', 18);
		imgCopyBtn.setAttribute('height', 18);
		imgCopyBtn.setAttribute('alt', copyLinkText);
		imgCopyBtn.style.padding = '4px';

		const editLinkText = _('Edit link');
		const editBtn = window.L.DomUtil.createWithId('div', this.editButtonId, linkRow);
		window.L.DomUtil.addClass(editBtn, 'hyperlink-popup-btn');
		editBtn.setAttribute('title', editLinkText);
		editBtn.setAttribute('role', 'button');
		editBtn.setAttribute('aria-label', copyLinkText);


		const imgEditBtn = window.L.DomUtil.create('img', 'hyperlink-pop-up-editimg', editBtn);
		app.LOUtil.setImage(imgEditBtn, 'lc_edithyperlink.svg', app.map);
		imgEditBtn.setAttribute('width', 18);
		imgEditBtn.setAttribute('height', 18);
		imgEditBtn.setAttribute('alt', editLinkText);
		imgEditBtn.style.padding = '4px';

		const removeLinkText = _('Remove link');
		const removeBtn = window.L.DomUtil.createWithId('div', this.removeButtonId, linkRow);
		window.L.DomUtil.addClass(removeBtn, 'hyperlink-popup-btn');
		removeBtn.setAttribute('title', removeLinkText);
		removeBtn.setAttribute('role', 'button');
		removeBtn.setAttribute('aria-label', removeLinkText);

		const imgRemoveBtn = window.L.DomUtil.create('img', 'hyperlink-pop-up-removeimg', removeBtn);
		app.LOUtil.setImage(imgRemoveBtn, 'lc_removehyperlink.svg', app.map);
		imgRemoveBtn.setAttribute('width', 18);
		imgRemoveBtn.setAttribute('height', 18);
		imgRemoveBtn.setAttribute('alt', removeLinkText);
		imgRemoveBtn.style.padding = '4px';

	}

	setUpCallbacks(linkPosition?: cool.SimplePoint) {
		this.link.onclick = () => {
			if (!this.sectionProperties.url.startsWith('#'))
				app.map.fire('warn', {url: this.sectionProperties.url, map: app.map, cmd: 'openlink'});
			else
				app.map.sendUnoCommand('.uno:JumpToMark?Bookmark:string=' + encodeURIComponent(this.sectionProperties.url.substring(1)));
		};

		var params: any;
		if (linkPosition) {
			params = {
				PositionX: {
					type: 'long',
					value: linkPosition.x
				},
				PositionY: {
					type: 'long',
					value: linkPosition.y
				}
			};
		}

		document.getElementById(this.copyButtonId).onclick = () => {
			// Hand the known link target straight to the native clipboard, as a UNO command is dropped here in view mode.
			if (window.mode.isCODesktop()) {
				(window as any).postMobileMessage('TEXTCLIPBOARD ' + this.sectionProperties.url);
			}
			else if (this.sectionProperties.linkIsClientSide) {
				app.map._clip.setTextSelectionText(this.sectionProperties.url);
				app.map._clip._execCopyCutPaste('copy');
			}
			// If _navigatorClipboardWrite is available, use it.
			else if (window.L.Browser.clipboardApiAvailable || window.ThisIsTheiOSApp)
				app.map._clip.filterExecCopyPaste('.uno:CopyHyperlinkLocation', params);
			else // Or use previous method.
				app.map.sendUnoCommand('.uno:CopyHyperlinkLocation', params);
		};

		document.getElementById(this.editButtonId).onclick = () => {
			if (!this.sectionProperties.linkIsClientSide) // For now link in client side works only on readonly mode
				app.map.sendUnoCommand('.uno:EditHyperlink', params);
		};

		document.getElementById(this.removeButtonId).onclick = () => {
			if (!this.sectionProperties.linkIsClientSide) // For now link in client side works only on readonly mode
				app.map.sendUnoCommand('.uno:RemoveHyperlink', params);
			URLPopUpSection.closeURLPopUp();
		};
	}

	public static getCurrent(): URLPopUpSection {
		return app.sectionContainer.getSectionWithName(URLPopUpSection.sectionName) as URLPopUpSection;
	}

	public updatePreview(values: { url: string; image?: string; title?: string }) {
		// Guard against a stale response for a link that is no longer displayed.
		// Compare against sectionProperties.url since DOM text gets overwritten below.
		if (this.sectionProperties.url !== values.url)
			return;

		this.preview.innerText = '';
		if (values.image && values.image.indexOf('data:') === 0) {
			const image = window.L.DomUtil.create('img', '', this.preview) as HTMLImageElement;
			image.src = values.image;
			image.alt = values.title;
			image.onload = function () {
				URLPopUpSection.resetPosition();
			};
		} else {
			window.L.DomUtil.addClass(this.preview, 'no-preview');
		}
		if (values.title) {
			// The fetched page title is promoted to the link row (next to the
			// action buttons) and the raw URL is shown in the preview area.
			const urlEl = window.L.DomUtil.create('p', '', this.preview);
			urlEl.innerText = values.url;
			this.link.innerText = values.title;
			URLPopUpSection.resetPosition();
		}
	}

	public static resetPosition(section?: URLPopUpSection) {
		if (!section) section = URLPopUpSection.getCurrent();
		if (!section) return;

		let originalLeft = section.sectionProperties.documentPosition.pX - section.getPopUpWidth() * 0.5 * app.dpiScale;
		let originalTop = section.sectionProperties.documentPosition.pY - (section.getPopUpHeight() + URLPopUpSection.popupVerticalMargin) * app.dpiScale;

		const checkLeft = originalLeft - app.activeDocument.activeLayout.viewedRectangle.pX1;
		const checkTop = originalTop - app.activeDocument.activeLayout.viewedRectangle.pY1;

		if (checkTop < 0) {
			originalTop = section.sectionProperties.documentPosition.pY + (URLPopUpSection.popupVerticalMargin * 2 * app.dpiScale);
		}

		if (checkLeft < 0) originalLeft = app.activeDocument.activeLayout.viewedRectangle.pX1;

		section.setPosition(originalLeft, originalTop);
		section.adjustHTMLObjectPosition();
		section.containerObject.requestReDraw();
	}

	public static showURLPopUP(url: string, documentPosition: cool.SimplePoint, linkPosition?: cool.SimplePoint, linkIsClientSide?: boolean) {
		if (URLPopUpSection.isOpen())
			URLPopUpSection.closeURLPopUp();

		const section = new URLPopUpSection(url, documentPosition, linkPosition, linkIsClientSide);
		app.sectionContainer.addSection(section);
		this.resetPosition(section);
    }

    public static closeURLPopUp() {
		if (URLPopUpSection.isOpen())
			app.sectionContainer.removeSection(URLPopUpSection.sectionName);
	}

    public static isOpen() {
		return app.sectionContainer.doesSectionExist(URLPopUpSection.sectionName);
    }
}
