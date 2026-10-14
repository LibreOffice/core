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
		this.interactable = false; // the popup's own elements take the mouse
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
		this.link.href = window.sanitizeUrl(url); // a link, and reachable by the keyboard
		const copyLinkText = _('Copy link location');
		const copyBtn = window.L.DomUtil.createWithId('button', this.copyButtonId, linkRow);
		copyBtn.setAttribute('type', 'button');
		copyBtn.classList.add('hyperlink-popup-btn', 'unobutton'); // not a dialog button
		copyBtn.setAttribute('title', copyLinkText);
		copyBtn.setAttribute('aria-label', copyLinkText);

        const imgCopyBtn = window.L.DomUtil.create('img', 'hyperlink-pop-up-copyimg', copyBtn);
		app.LOUtil.setImage(imgCopyBtn, 'lc_copyhyperlinklocation.svg', app.map);
		imgCopyBtn.setAttribute('width', 18);
		imgCopyBtn.setAttribute('height', 18);
		imgCopyBtn.setAttribute('alt', copyLinkText);
		imgCopyBtn.style.padding = '4px';

		const editLinkText = _('Edit link');
		const editBtn = window.L.DomUtil.createWithId('button', this.editButtonId, linkRow);
		editBtn.setAttribute('type', 'button');
		editBtn.classList.add('hyperlink-popup-btn', 'unobutton'); // not a dialog button
		editBtn.setAttribute('title', editLinkText);
		editBtn.setAttribute('aria-label', editLinkText);


		const imgEditBtn = window.L.DomUtil.create('img', 'hyperlink-pop-up-editimg', editBtn);
		app.LOUtil.setImage(imgEditBtn, 'lc_edithyperlink.svg', app.map);
		imgEditBtn.setAttribute('width', 18);
		imgEditBtn.setAttribute('height', 18);
		imgEditBtn.setAttribute('alt', editLinkText);
		imgEditBtn.style.padding = '4px';

		const removeLinkText = _('Remove link');
		const removeBtn = window.L.DomUtil.createWithId('button', this.removeButtonId, linkRow);
		removeBtn.setAttribute('type', 'button');
		removeBtn.classList.add('hyperlink-popup-btn', 'unobutton'); // not a dialog button
		removeBtn.setAttribute('title', removeLinkText);
		removeBtn.setAttribute('aria-label', removeLinkText);

		const imgRemoveBtn = window.L.DomUtil.create('img', 'hyperlink-pop-up-removeimg', removeBtn);
		app.LOUtil.setImage(imgRemoveBtn, 'lc_removehyperlink.svg', app.map);
		imgRemoveBtn.setAttribute('width', 18);
		imgRemoveBtn.setAttribute('height', 18);
		imgRemoveBtn.setAttribute('alt', removeLinkText);
		imgRemoveBtn.style.padding = '4px';

		if (app.map._permission === 'readonly') {
			editBtn.style.display = 'none';
			removeBtn.style.display = 'none';
		}
	}

	openLink() {
		URLPopUpSection.openLink(this.sectionProperties.url);
	}

	// '#' is a bookmark in the document, anything else opens after a confirmation
	public static openLink(url: string) {
		if (!url.startsWith('#'))
			app.map.fire('warn', {url: url, map: app.map, cmd: 'openlink'});
		else
			app.map.sendUnoCommand('.uno:JumpToMark?Bookmark:string=' + encodeURIComponent(url.substring(1)));
	}

	// in a spreadsheet the link commands take the tap position inside the cell
	public static commandParameters(linkPosition?: cool.SimplePoint): any {
		if (!linkPosition)
			return undefined;

		return {
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

	// the native apps copy through the engine to reach the system clipboard
	public static copyLinkLocation(url: string, params?: any) {
		if (window.ThisIsTheiOSApp || window.ThisIsTheMacOSApp
				|| window.ThisIsTheWindowsApp || window.ThisIsTheQtApp) {
			app.map._clip.filterExecCopyPaste('.uno:CopyHyperlinkLocation', params);
		} else if (navigator.clipboard && navigator.clipboard.writeText) {
			navigator.clipboard.writeText(url);
		} else {
			const ta = document.createElement('textarea');
			ta.value = url;
			ta.style.cssText = 'position:fixed;opacity:0';
			document.body.appendChild(ta);
			ta.select();
			document.execCommand('copy');
			document.body.removeChild(ta);
		}
	}

	// opened only through openLink(), which shows the confirmation first
	guardLink(anchor: HTMLAnchorElement) {
		for (const type of ['click', 'auxclick', 'contextmenu', 'dragstart']) {
			anchor.addEventListener(type, (event) => {
				event.preventDefault();
				if (type === 'click')
					this.openLink();
			});
		}
	}

	setUpCallbacks(linkPosition?: cool.SimplePoint) {
		this.guardLink(this.link);

		// Escape gives the keyboard back to the document, which the popup belongs to
		this.getHTMLObject().addEventListener('keydown', (event: KeyboardEvent) => {
			if (event.key !== 'Escape')
				return;
			event.preventDefault();
			event.stopPropagation();
			app.map.focus();
		});

		const params = URLPopUpSection.commandParameters(linkPosition);

		document.getElementById(this.copyButtonId).onclick = () => {
			URLPopUpSection.copyLinkLocation(this.sectionProperties.url, params);
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
			// The URL stays clickable there, since it is what the user
			// recognizes as the hyperlink.
			const urlAnchor = window.L.DomUtil.create('a', '', this.preview) as HTMLAnchorElement;
			urlAnchor.innerText = values.url;
			urlAnchor.title = values.url;
			urlAnchor.href = window.sanitizeUrl(values.url);
			this.guardLink(urlAnchor);
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

		section.size = [section.getPopUpWidth() * app.dpiScale, section.getPopUpHeight() * app.dpiScale];
		section.setPosition(originalLeft, originalTop);
		section.adjustHTMLObjectPosition();
		section.containerObject.requestReDraw();
	}

	public static showURLPopUP(url: string, documentPosition: cool.SimplePoint, linkPosition?: cool.SimplePoint, linkIsClientSide?: boolean) {
		if (window.mode.isSmallScreenDevice()) {
			HyperlinkWizard.show(url, linkPosition, linkIsClientSide);
			return;
		}

		if (URLPopUpSection.isOpen())
			URLPopUpSection.closeURLPopUp();

		const section = new URLPopUpSection(url, documentPosition, linkPosition, linkIsClientSide);
		app.sectionContainer.addSection(section);
		this.resetPosition(section);
    }

    public static closeURLPopUp() {
		if (!URLPopUpSection.isOpen())
			return;
		// the focus would stay on a removed element, out of the keyboard's reach
		const popup = URLPopUpSection.getCurrent().getHTMLObject();
		const hadFocus = popup.contains(document.activeElement);
		app.sectionContainer.removeSection(URLPopUpSection.sectionName);
		if (hadFocus)
			app.map.focus();
	}

    public static isOpen() {
		return app.sectionContainer.doesSectionExist(URLPopUpSection.sectionName);
    }
}
