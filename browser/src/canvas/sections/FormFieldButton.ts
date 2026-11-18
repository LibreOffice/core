/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * FormFieldButton is used to interact with text based form fields.
 */

class FormFieldButton extends HTMLObjectSection {
	constructor(buttonData: any) {
		super(app.CSections.FormFieldButton.name, 1, 1, new cool.SimplePoint(0, 0));

		window.app.console.assert(buttonData.type === 'drop-down');
		this.sectionProperties.buttonData = buttonData;
		this.sectionProperties.container = null;
		this.sectionProperties.textRectangle = null;
		this.sectionProperties.buttonFrame = null;
		this.sectionProperties.frameWidth = 0;
		this.sectionProperties.frameHeight = 0;
		this.sectionProperties.extraPadding = cool.SimplePoint.fromCorePixels([
			2 * app.dpiScale,
			2 * app.dpiScale,
		]);
		this.sectionProperties.framePosition = null;
		this.sectionProperties.dropDownButton = null;
		this.sectionProperties.dropDownButtonImage = null;
		this.sectionProperties.dropDownList = null;
		this.sectionProperties.selectedItem = 0;
		this.sectionProperties.mouseEntered = false;
		this.sectionProperties.mainContainerSize = null;
	}

	onInitialize(): void {
		this.fetchPositionAndSize();
		this.createMainContainer();
		this.createButtonContainer();
		this.createButton();
		this.createDropDownList();
		this.addToDOM();

		this.getHTMLObject().style.pointerEvents = '';
	}

	private fetchPositionAndSize() {
		var strTwips = this.sectionProperties.buttonData.textArea.match(/\d+/g);
		this.sectionProperties.textRectangle = new cool.SimpleRectangle(
			parseInt(strTwips[0]),
			parseInt(strTwips[1]),
			parseInt(strTwips[2]),
			parseInt(strTwips[3]),
		);

		this.setPosition(
			this.sectionProperties.textRectangle.pX1,
			this.sectionProperties.textRectangle.pY1,
		);

		this.sectionProperties.frameWidth =
			this.sectionProperties.textRectangle.cWidth +
			1.5 * this.sectionProperties.extraPadding.cX;
		this.sectionProperties.frameHeight =
			this.sectionProperties.textRectangle.cHeight +
			1.5 * this.sectionProperties.extraPadding.cY;
	}

	private createMainContainer() {
		const height: number = this.sectionProperties.frameHeight;
		const width: number = this.sectionProperties.frameWidth + height;

		this.sectionProperties.container = document.createElement('div');
		this.sectionProperties.container.classList.add('form-field-frame');
		this.sectionProperties.container.style.height =
			this.sectionProperties.frameHeight + 'px';
		this.sectionProperties.container.style.width =
			this.sectionProperties.frameWidth +
			this.sectionProperties.frameHeight +
			'px';

		this.sectionProperties.mainContainerSize = cool.SimplePoint.fromCorePixels([
			width * app.dpiScale,
			height * app.dpiScale,
		]);
	}

	private createButtonContainer() {
		this.sectionProperties.buttonFrame = document.createElement('div');
		this.sectionProperties.buttonFrame.className =
			'form-field-button-container';
		this.sectionProperties.buttonFrame.style.width =
			this.sectionProperties.mainContainerSize.pY + 'px';
	}

	private createButton() {
		this.sectionProperties.dropDownButton = document.createElement('button');
		this.sectionProperties.dropDownButton.className = 'form-field-button';

		this.sectionProperties.dropDownButtonImage = document.createElement('img');
		this.sectionProperties.dropDownButtonImage.className =
			'form-field-button-image';
		this.sectionProperties.dropDownButtonImage.setAttribute('alt', _('Unfold'));
		this.sectionProperties.dropDownButtonImage.src =
			app.LOUtil.getImageURL('unfold.svg');
		this.sectionProperties.dropDownButtonImage.onclick =
			this.onClickDropdownButton.bind(this);
		this.sectionProperties.dropDownButtonImage.className =
			'form-field-button-image';
	}

	private buildListItem(index: number, text: string) {
		const optionElement = document.createElement('div');
		optionElement.className = 'drop-down-field-list-item';
		optionElement.innerText = text;
		optionElement.style.fontSize =
			this.sectionProperties.mainContainerSize.cY * 0.7 + 'px';
		optionElement.dataset.optionIndex = index.toString();

		optionElement.addEventListener('click', this.onListItemSelect.bind(this));

		// Stop propagation to the main document
		optionElement.addEventListener('mouseup', function (event) {
			event.stopPropagation();
		});
		optionElement.addEventListener('mousedown', function (event) {
			event.stopPropagation();
		});

		if (index === this.sectionProperties.selectedItem)
			optionElement.classList.add('selected');

		this.sectionProperties.dropDownList.appendChild(optionElement);
	}

	private createDropDownList() {
		this.sectionProperties.dropDownList = document.createElement('div');
		this.sectionProperties.dropDownList.className = 'drop-down-field-list';
		this.sectionProperties.dropDownList.style.display = 'none';
		this.sectionProperties.selectedItem = parseInt(
			this.sectionProperties.buttonData.params.selected,
		);

		for (
			let i = 0;
			i < this.sectionProperties.buttonData.params.items.length;
			i++
		) {
			this.buildListItem(i, this.sectionProperties.buttonData.params.items[i]);
		}

		if (this.sectionProperties.buttonData.params.items.length === 0)
			this.buildListItem(
				0,
				this.sectionProperties.buttonData.params.placeholderText,
			);
	}

	private addToDOM() {
		// We should have everything constructed by now. Let's add them to the DOM object.
		// Add elements to each other from bottom to top first, and add the top element to the DOM.
		this.sectionProperties.dropDownButton.appendChild(
			this.sectionProperties.dropDownButtonImage,
		);
		this.sectionProperties.buttonFrame.appendChild(
			this.sectionProperties.dropDownButton,
		);
		this.sectionProperties.container.appendChild(
			this.sectionProperties.buttonFrame,
		);
		this.sectionProperties.container.appendChild(
			this.sectionProperties.dropDownList,
		);

		const width = parseInt(
			this.sectionProperties.container.style.width.replace('px', ''),
		);
		const height = parseInt(
			this.sectionProperties.container.style.height.replace('px', ''),
		);

		this.size[0] = width * app.dpiScale;
		this.size[1] = height * app.dpiScale;
		this.getHTMLObject().style.width = width + 'px';
		this.getHTMLObject().style.height = height + 'px';
		this.getHTMLObject().appendChild(this.sectionProperties.container);

		this.adjustHTMLObjectPosition();
	}

	private onListItemSelect(event: MouseEvent) {
		(
			document.querySelector('.drop-down-field-list') as HTMLDivElement
		).style.display = 'none';
		event.stopPropagation();

		if (this.sectionProperties.buttonData.params.items.length === 0) return;

		const selectedItem = document.querySelector(
			'.drop-down-field-list-item.selected',
		);
		if (selectedItem) selectedItem.classList.remove('selected');

		(event.target as HTMLDivElement).classList.add('selected');

		// Find item index
		let index: string | undefined = '0';
		if (event.target)
			index = (event.target as HTMLDivElement).dataset.optionIndex;

		if (!index) index = '0';

		var message =
			'formfieldevent {"type": "drop-down",' +
			'"cmd": "selected",' +
			'"data":"' +
			index.toString() +
			'"}';

		// Apply selection in the document.
		app.socket.sendMessage(message);

		app.map.focus();
	}

	private onClickDropdownButton(event: MouseEvent) {
		(
			document.querySelector('.drop-down-field-list') as HTMLDivElement
		).style.display = '';
		event.stopPropagation();
	}

	onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		this.sectionProperties.mouseEntered = true;
	}

	onMouseLeave(point: cool.SimplePoint, e: MouseEvent): void {
		this.sectionProperties.mouseEntered = false;
	}

	private resizeHTMLObjectsAfterZoomChange() {
		this.setPosition(
			this.sectionProperties.textRectangle.pX1,
			this.sectionProperties.textRectangle.pY1,
		);

		// Container under HTMLObject.
		this.sectionProperties.container.style.width =
			this.sectionProperties.mainContainerSize.cX + 'px';
		this.sectionProperties.container.style.height =
			this.sectionProperties.mainContainerSize.cY + 'px';

		this.sectionProperties.buttonFrame.style.width =
			this.sectionProperties.mainContainerSize.cY + 'px';

		this.sectionProperties.dropDownList.remove(); // Remove and recreate because font size will change.
		this.createDropDownList();

		this.sectionProperties.container.appendChild(
			this.sectionProperties.dropDownList,
		);

		// HTML Object.
		this.getHTMLObject().style.width =
			this.sectionProperties.container.style.width;
		this.getHTMLObject().style.height =
			this.sectionProperties.container.style.height;
	}

	public onNewDocumentTopLeft(): void {
		// Check if zoom changed.
		if (
			this.position[0] !== this.sectionProperties.textRectangle.pX1 ||
			this.position[1] !== this.sectionProperties.textRectangle.pY1
		)
			this.resizeHTMLObjectsAfterZoomChange();

		super.onNewDocumentTopLeft();
	}
}
