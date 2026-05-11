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
 * ContextToolbar - definition of context toolbar
 * This toolbar should appear over selection when related selection is done using mouse
 */

class ContextToolbar extends JSDialogComponent {
	declare container: HTMLElement;
	initialized: boolean = false;
	lastIinputEvent?: any = {};
	pendingTask: TaskId | null = null;
	pendingShow: boolean = false;
	// roughly twice the height(76px) of default context toolbar in each direction from boundary
	disappearingBoundary: number = 150; // px
	additionalContextButtons: Array<ToolItemWidgetJSON> = [];

	constructor(map: MapInterface) {
		super(map, 'ContextToolbar', 'notebookbar');
		this.createBuilder();
		this.setupContainer(undefined);
		this.registerMessageHandlers();
	}

	protected createBuilder() {
		this.builder = new window.L.control.notebookbarBuilder({
			windowId: WindowId.Notebookbar,
			mobileWizard: this,
			map: this.map,
			cssClass: 'notebookbar',
			suffix: 'context-toolbar',
		} as JSBuilderOptions);
	}

	protected setupContainer(parentContainer?: HTMLElement /* ignored */): void {
		this.container = window.L.DomUtil.createWithId(
			'div',
			'context-toolbar',
			document.body,
		);
		window.L.DomUtil.addClass(this.container, 'notebookbar horizontal');
	}

	showContextToolbar(): void {
		const map = this.map;
		if (
			map.isReadOnlyMode() ||
			!window.mode.isDesktop() ||
			map.getDocType() === 'spreadsheet'
		)
			return;
		if (this.lastIinputEvent.input === 'mouse') this.pendingShow = true;
		if (this.lastIinputEvent.type !== 'buttonup') return;

		this.showContextToolbarImpl();
	}

	showContextToolbarImpl(): void {
		this.pendingShow = false;

		if (this.pendingTask)
			app.layoutingService.cancelLayoutingTask(this.pendingTask);

		this.pendingTask = app.layoutingService.appendLayoutingTask(() => {
			this.pendingTask = null;

			if (!this.initialized) {
				const contextToolbarItems = this.getWriterTextContext();

				for (const i in this.additionalContextButtons) {
					const item = this.additionalContextButtons[i];
					contextToolbarItems.push(item);
				}

				this.builder?.build(this.container, contextToolbarItems, false);

				this.initialized = true;
			}

			document.addEventListener('pointermove', this.pointerMove);
			this.changeOpacity(1);
			this.showHideToolbar(true);
		});
	}

	hideContextToolbar(): void {
		document.removeEventListener('pointermove', this.pointerMove);

		if (this.pendingTask)
			app.layoutingService.cancelLayoutingTask(this.pendingTask);

		this.pendingTask = app.layoutingService.appendLayoutingTask(() => {
			this.pendingTask = null;
			this.showHideToolbar(false);
		});
	}

	private showHideToolbar(show: boolean): void {
		if (!show) {
			window.L.DomUtil.addClass(this.container, 'hidden');
			return;
		}

		URLPopUpSection.closeURLPopUp();
		const statRect = app.file.textCursor.rectangle;
		if (!TextSelections || !TextSelections.getStartRectangle() || !statRect)
			return;

		Util.ensureValue(app.activeDocument);

		// Go via SimplePoint(), which is aware of the active layout.
		const point = new cool.SimplePoint(statRect.x1, statRect.y1);
		const canvasRect = app.sectionContainer.getCanvasBoundingClientRect();
		const pos = {
			x: Math.round(point.vX / app.dpiScale) + canvasRect.x,
			y: Math.round(point.vY / app.dpiScale) + canvasRect.y,
		};

		window.L.DomUtil.removeClass(this.container, 'hidden');
		app.layoutingService.appendLayoutingTask(() => {
			const contextualMenu = this.container.getBoundingClientRect();
			if (contextualMenu.width + pos.x > window.innerWidth) {
				pos.x -= contextualMenu.width + pos.x - window.innerWidth + 5;
			}
			this.container.style.top = pos.y + 'px';
			this.container.style.left = pos.x + 'px';
		});
	}

	getWriterTextContext(): ToolItemWidgetJSON[] {
		const fontEntries = Object.keys(
			this.map._docLayer._toolbarCommandValues['.uno:CharFontName'],
		);
		const sizeEntries = [
			'6',
			'7',
			'8',
			'9',
			'10',
			'10.5',
			'11',
			'12',
			'13',
			'14',
			'15',
			'16',
			'18',
			'20',
			'21',
			'22',
			'24',
			'26',
			'28',
			'32',
			'36',
			'40',
			'44',
			'48',
			'54',
			'60',
			'66',
			'72',
			'80',
			'88',
			'96',
		];
		const currentFontName = this.map._getCurrentFontName();
		const currentFontSize =
			this.map['stateChangeHandler'].getItemValue('.uno:FontHeight');
		const contextItems: ToolItemWidgetJSON[] = [
			{
				type: 'container',
				children: [
					{
						type: 'container',
						children: [
							{
								id: 'fontnamecombobox',
								type: 'combobox',
								text: currentFontName,
								entries: fontEntries,
								selectedCount: 1,
								selectedEntries: [
									fontEntries.findIndex(
										(element) => element === currentFontName,
									),
								],
								command: '.uno:CharFontName',
								customEntryRenderer: true,
							} as ComboBoxWidget,
							{
								id: 'fontsizecombobox',
								type: 'combobox',
								text: currentFontSize,
								entries: sizeEntries,
								selectedCount: 1,
								selectedEntries: [
									sizeEntries.findIndex(
										(element) => element === currentFontSize,
									),
								],
								command: '.uno:FontHeight',
							} as ComboBoxWidget,
							{
								id: 'home-grow',
								type: 'toolitem',
								text: _UNO('.uno:Grow'),
								command: '.uno:Grow',
							} as ToolItemWidgetJSON,
							{
								id: 'home-shrink',
								type: 'toolitem',
								text: _UNO('.uno:Shrink'),
								command: '.uno:Shrink',
							} as ToolItemWidgetJSON,
						],
						vertical: false,
					} as ContainerWidgetJSON,
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'home-bold',
										type: 'toolitem',
										text: _UNO('.uno:Bold'),
										command: '.uno:Bold',
									} as ToolItemWidgetJSON,
									{
										id: 'home-italic',
										type: 'toolitem',
										text: _UNO('.uno:Italic'),
										command: '.uno:Italic',
									} as ToolItemWidgetJSON,
									{
										id: 'home-underline',
										type: 'toolitem',
										text: _UNO('.uno:Underline'),
										command: '.uno:Underline',
									} as ToolItemWidgetJSON,
									{
										id: 'home-strikeout',
										type: 'toolitem',
										text: _UNO('.uno:Strikeout'),
										command: '.uno:Strikeout',
									} as ToolItemWidgetJSON,
									{
										id: 'home-subscript',
										type: 'toolitem',
										text: _UNO('.uno:SubScript'),
										command: '.uno:SubScript',
									} as ToolItemWidgetJSON,
									{
										id: 'home-superscript',
										type: 'toolitem',
										text: _UNO('.uno:SuperScript'),
										command: '.uno:SuperScript',
									} as ToolItemWidgetJSON,
									{
										id: 'home-spacing:CharSpacingMenu',
										type: 'menubutton',
										noLabel: true,
										text: _UNO('.uno:Spacing'),
										command: '.uno:CharSpacing',
									} as ToolItemWidgetJSON,
									{
										id: 'home-back-color:ColorPickerMenu',
										class: 'unospan-CharBackColor',
										type: 'toolitem',
										noLabel: true,
										text: _UNO('.uno:CharBackColor', 'text'),
										command: '.uno:CharBackColor',
									} as ToolItemWidgetJSON,
									{
										id: 'home-color:ColorPickerMenu',
										class: 'unospan-FontColor',
										type: 'toolitem',
										noLabel: true,
										text: _UNO('.uno:Color'),
										command: '.uno:Color',
									} as ToolItemWidgetJSON,
								],
							},
						],
						vertical: false,
					},
				],
				vertical: true,
			} as ContainerWidgetJSON,
			{
				type: 'separator',
				id: 'home-fontcombobox-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				id: 'home-insert-annotation',
				type: 'bigtoolitem',
				text: _UNO('.uno:InsertAnnotation'),
				command: '.uno:InsertAnnotation',
			} as ToolItemWidgetJSON,
		];
		return contextItems;
	}

	insertAdditionalContextButton(button: any) {
		for (const i in this.additionalContextButtons) {
			const item = this.additionalContextButtons[i];
			if (item.id === button.id) return;
		}

		if (this.additionalContextButtons.length === 0) {
			this.additionalContextButtons.push({
				type: 'separator',
				id: 'home-insertannotation-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON);
		}

		const isUnoCommand =
			button.unoCommand && button.unoCommand.indexOf('.uno:') >= 0;
		if (button.unoCommand && !isUnoCommand)
			button.unoCommand = '.uno:' + button.unoCommand;

		const contextButton = {
			id: button.id,
			type: 'bigtoolitem',
			text: button.label ? button.label : button.hint ? _(button.hint) : ' ',
			command: button.unoCommand,
			icon: button.imgurl,
			postmessage: button.unoCommand ? undefined : true,
		} as ToolItemWidgetJSON;

		this.additionalContextButtons.push(contextButton);

		// update context toolbar
		document.getElementById('context-toolbar')?.remove();
		this.setupContainer(undefined);
		this.initialized = false;
	}

	setLastInputEventType(e: any) {
		this.lastIinputEvent = e;
		if (e.type === 'buttonup' && e.input === 'mouse' && this.pendingShow) {
			this.showContextToolbarImpl();
			this.pendingShow = false;
		}
	}

	calculateOpacity(e: PointerEvent): number {
		const clientRect: DOMRect = this.container.getBoundingClientRect();

		// hover over toolbar
		if (
			clientRect.left < e.clientX &&
			e.clientX < clientRect.right &&
			clientRect.top < e.clientY &&
			e.clientY < clientRect.bottom
		) {
			return 1;
		}

		const minX = clientRect.left - this.disappearingBoundary;
		const maxX = clientRect.right + this.disappearingBoundary;
		const minY = clientRect.top - this.disappearingBoundary;
		const maxY = clientRect.bottom + this.disappearingBoundary;

		let xDistance: number = 0;
		// left of toolbar
		if (minX < e.clientX && e.clientX < clientRect.left)
			xDistance = e.clientX - minX;
		// right of toolbar
		else if (clientRect.right < e.clientX && e.clientX < maxX)
			xDistance = maxX - e.clientX;

		let yDistance: number = 0;
		// top of toolbar
		if (minY < e.clientY && e.clientY < clientRect.top)
			yDistance = e.clientY - minY;
		// bottom of toolbar
		else if (clientRect.bottom < e.clientY && e.clientY < maxY)
			yDistance = maxY - e.clientY;

		return (xDistance + yDistance) / (2 * this.disappearingBoundary);
	}

	pointerMove = (e: PointerEvent): void => {
		app.layoutingService.appendLayoutingTask(() => {
			const opacity: number = this.calculateOpacity(e);

			if (opacity === 1) {
				this.makeContextToolbarConstant();
				return;
			} else if (opacity === 0) {
				this.hideContextToolbar();
				return;
			}

			this.changeOpacity(opacity);
		});
	};

	makeContextToolbarConstant(): void {
		document.removeEventListener('pointermove', this.pointerMove);
		this.changeOpacity(1);
	}

	changeOpacity(opacity: number) {
		this.container.style.opacity = opacity.toString();
	}
}
