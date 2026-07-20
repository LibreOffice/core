// @ts-strict-ignore
/* eslint-disable */


/*
 * Cursor implements a blinking cursor.
 * This is used as the text-cursor of the document.
 */

class Cursor {
	opacity: number = 1;
	zIndex: number = 1000;
	blink: boolean = false;
	color: string;
	header: boolean = false;
	headerName: string;
	headerTimeout: number = 3000;

	private rectangle: cool.SimpleRectangle;
	private width: number;
	private container: HTMLDivElement;
	private cursorHeader: HTMLDivElement;
	private cursor: HTMLDivElement;
	private map: any;
	private blinkTimeout: NodeJS.Timeout;
	private blinkSuspendTimeout: any;
	private visible: boolean = false;
	private domAttached: boolean = false;

	// position and size should be in core pixels.
	constructor(rectangle: cool.SimpleRectangle, map: any, options: any) {
		this.opacity = options.opacity !== undefined ? options.opacity : this.opacity;
		this.zIndex = options.zIndex !== undefined ? options.zIndex : this.zIndex;
		this.blink = options.blink !== undefined ? options.blink : this.blink;
		this.color = options.color !== undefined ? options.color : this.color;
		this.header = options.header !== undefined ? options.header : this.header;
		this.headerName = options.headerName !== undefined ? options.headerName : this.headerName;
		this.headerTimeout = options.headerTimeout !== undefined ? options.headerTimeout : this.headerTimeout;

		this.rectangle = rectangle;
		this.map = map;

		this.initLayout();
	}

	add() {
		if (!this.container) {
			this.initLayout();
		}

		this.setMouseCursor();

		this.map.getCursorOverlayContainer().appendChild(this.container);
		this.visible = true;
		this.domAttached = true;

		this.update();

		if (this.map._docLayer.isCalc())
			this.map.on('splitposchanged move', this.update, this);
		else
			this.map.on('move', this.update, this);

		window.addEventListener('blur', this.onFocusBlur);
		window.addEventListener('focus', this.onFocusBlur);
		window.addEventListener('resize', this.onResize);
	}

	setMouseCursor() {
		if (this.domAttached && this.container && this.container.querySelector('.blinking-cursor') !== null) {
			if (this.map._docLayer._docType === 'presentation') {
				$('.leaflet-interactive').css('cursor', 'text');
			} else {
				const canvas = document.getElementById('document-canvas');
				if (canvas.style.cursor !== 'text')
					canvas.style.cursor = 'text';
			}
		}
	}
	setMouseCursorForTextBox() {
		if (this.domAttached && this.container && this.container.querySelector('.blinking-cursor') !== null) {
			$('.leaflet-interactive').css('cursor', 'text');
		}
		this.addCursorClass(app.file.textCursor.visible);
		this.setOpacity(app.file.textCursor.visible ? 1 : 0);
	}

	remove() {
		this.map.off('splitposchanged', this.update, this);
		if (this.map._docLayer._docType === 'presentation') {
			$('.leaflet-interactive').css('cursor', '');
		} else {
			$('#document-canvas').css('cursor', '');
		}
		if (this.container && this.domAttached) {
			this.map.getCursorOverlayContainer().removeChild(this.container);
		}

		this.visible = false;
		this.domAttached = false;

		window.removeEventListener('blur', this.onFocusBlur);
		window.removeEventListener('focus', this.onFocusBlur);
		window.removeEventListener('resize', this.onResize);
	}

	isDomAttached(): boolean {
		return this.domAttached;
	}

	addCursorClass(visible: boolean) {
		if (visible)
			window.L.DomUtil.removeClass(this.cursor, 'blinking-cursor-hidden');
		else
			window.L.DomUtil.addClass(this.cursor, 'blinking-cursor-hidden');
	}

	isVisible(): boolean {
		return this.visible;
	}

	onFocusBlur = (ev: FocusEvent) => {
		this.addCursorClass(ev.type !== 'blur');
	}

	onResize = () => {
		if (window.devicePixelRatio !== 1)
			this.cursor.style.width = this.width / window.devicePixelRatio + 'px';
		else
			this.cursor.style.removeProperty('width');
	}

	// position and size should be in core pixels.
	setRectangle(rectangle: cool.SimpleRectangle) {
		this.rectangle = rectangle;
		this.update();
	}

	public update() {
		if (!this.container || !this.map || !app.activeDocument || app.activeDocument.fileSize.x === 0)
			return;

		if (!app.isRectangleVisibleInTheDisplayedArea(app.file.textCursor.rectangle.toArray())) {
			this.container.style.visibility = 'hidden';
			this.visible = false;
			this.addCursorClass(this.visible);
			this.showCursorHeader();
			return;
		}

		this.container.style.visibility = 'visible';
		this.visible = true;
		this.addCursorClass(this.visible);

		// Compute tile-section offset in css pixels.
		this.updatePositionAndSize();
		this.showCursorHeader();
	}

	setOpacity(opacity: number) {
		if (this.container)
			window.L.DomUtil.setOpacity(this.cursor, opacity);
		if (this.cursorHeader)
			window.L.DomUtil.setOpacity(this.cursorHeader, opacity);
	}

	// Shows cursor header if cursor is in visible area.
	showCursorHeader() {
		if (this.cursorHeader) {
			if (!this.visible || this.map._docLayer._isZooming) {
				this.hideCursorHeader();
				return;
			}

			window.L.DomUtil.setStyle(this.cursorHeader, 'visibility', 'visible');

			clearTimeout(this.blinkTimeout);
			this.blinkTimeout = setTimeout(window.L.bind(function () {
				this.hideCursorHeader();
			}, this), this.headerTimeout);
		}
	}

	hideCursorHeader() {
		if (this.cursorHeader)
			window.L.DomUtil.setStyle(this.cursorHeader, 'visibility', 'hidden');
	}

	private initLayout() {
		this.container = window.L.DomUtil.create('div', 'leaflet-cursor-container');
		if (this.header) {
			this.cursorHeader = window.L.DomUtil.create('div', 'leaflet-cursor-header', this.container);

			this.cursorHeader.textContent = this.headerName;

			clearTimeout(this.blinkTimeout);
			this.blinkTimeout = setTimeout(window.L.bind(function () {
				window.L.DomUtil.setStyle(this._cursorHeader, 'visibility', 'hidden');
			}, this), this.headerTimeout);
		}
		this.cursor = window.L.DomUtil.create('div', 'leaflet-cursor', this.container);
		if (this.blink) {
			window.L.DomUtil.addClass(this.cursor, 'blinking-cursor');
		}

		if (this.color) {
			window.L.DomUtil.setStyle(this.cursorHeader, 'background', this.color);
			window.L.DomUtil.setStyle(this.cursor, 'background', this.color);
		}

		window.L.DomEvent
			.disableClickPropagation(this.cursor)
			.disableScrollPropagation(this.container);

		let cursorCss = getComputedStyle(this.cursor, null);
		this.width = parseFloat(cursorCss.getPropertyValue('width'));
	}

	private transformX(xpos: number): number {
		if (!this.map._docLayer.isCalcRTL()) {
			return xpos;
		}

		return this.map._size.x - xpos;
	}

	private updatePositionAndSize() {
		app.layoutingService.appendLayoutingTask(() => {
			let x : number;
			let y : number;

			if (app.map.getDocType() === 'spreadsheet') {
				let diffX = -app.activeDocument.activeLayout.viewedRectangle.pX1;
				let diffY = -app.activeDocument.activeLayout.viewedRectangle.pY1;

				if (app.isXOrdinateInFrozenPane(this.rectangle.pX1))
					diffX = 0;

				if (app.isYOrdinateInFrozenPane(this.rectangle.pY1))
					diffY = 0;

				x = Math.round((this.rectangle.pX1 + diffX + app.sectionContainer.getDocumentAnchor()[0]) / app.dpiScale);
				y = Math.round((this.rectangle.pY1 + diffY + app.sectionContainer.getDocumentAnchor()[1]) / app.dpiScale);
			}
			else {
				x = Math.round(this.rectangle.v1X / app.dpiScale);
				y = Math.round(this.rectangle.v1Y / app.dpiScale);
			}

			this.container.style.top = y + 'px';
			this.container.style.left = this.transformX(x) + 'px';
			this.container.style.zIndex = this.zIndex + '';
			this.cursor.style.height = this.rectangle.cHeight + 'px';
			// Suspend blinking animation during cursor movement
			if (this.blink) {
				window.L.DomUtil.addClass(this.cursor, 'blinking-suspended');
				if (this.blinkSuspendTimeout) {
					clearTimeout(this.blinkSuspendTimeout);
				}
				this.blinkSuspendTimeout = setTimeout(() => {
					app.layoutingService.appendLayoutingTask(() => {
						window.L.DomUtil.removeClass(this.cursor, 'blinking-suspended');
					});
				}, 500);
			}
		});
	}

	static hotSpot = new Map<string, cool.Point>([['fill', new cool.Point(7, 16)]]);

	static customCursors = [
		'fill'
	];

	static imagePath: string;

	static isCustomCursor(cursorName: string): boolean {
		return (Cursor.customCursors.indexOf(cursorName) !== -1);
	}

	static getCustomCursor(cursorName: string) {
		var customCursor;

		if (Cursor.isCustomCursor(cursorName)) {
			var cursorHotSpot = Cursor.hotSpot.get(cursorName) || new cool.Point(0, 0);
			customCursor =
				'url(' + Cursor.imagePath + '/' + cursorName + '.png) ' + cursorHotSpot.x + ' ' + cursorHotSpot.y + ', default';
		}
		return customCursor;
	};
}
