/* -*- js-indent-level: 8 -*- */
/* global app cool */
/*
 * window.L.Draggable allows you to add dragging capabilities to any element. Supports mobile devices too.
 */

window.L.Draggable = window.L.Evented.extend({

	statics: {
		START: ['touchstart', 'mousedown'],
		END: {
			mousedown: 'mouseup',
			touchstart: 'touchend',
			pointerdown: 'touchend',
			MSPointerDown: 'touchend'
		},
		MOVE: {
			mousedown: 'mousemove',
			touchstart: 'touchmove',
			pointerdown: 'touchmove',
			MSPointerDown: 'touchmove'
		}
	},

	_manualDrag: function() {
		return false;
	},

	noManualDrag: window.memo.decorator(function(f) {
		return function(e) {
			if (!this._manualDrag(e)) {
				return f.apply(this, arguments);
			}
		};
	}),

	initialize: function (element, dragStartTarget, preventOutline, no3d) {

		window.L.Evented.prototype.initialize.call(this);

		this._element = element;
		this._dragStartTarget = dragStartTarget || element;
		this._preventOutline = preventOutline;
		this._freezeX = false;
		this._freezeY = false;
		this._no3d = no3d;
	},

	freezeX: function (boolChoice) {
		this._freezeX = boolChoice;
	},

	freezeY: function (boolChoice) {
		this._freezeY = boolChoice;
	},

	enable: function () {
		if (this._enabled) { return; }

		window.L.DomEvent.on(this._dragStartTarget, window.L.Draggable.START.join(' '), this.noManualDrag(this._onDown), this);

		this._enabled = true;
	},

	disable: function () {
		if (!this._enabled) { return; }

		window.L.DomEvent.off(this._dragStartTarget, window.L.Draggable.START.join(' '), this.noManualDrag(this._onDown), this);

		this._enabled = false;
		this._moved = false;
	},

	_onDown: function (e) {
		this._moved = false;

		if (e.shiftKey || ((e.which !== 1) && (e.button !== 0) && !e.touches)) { return; }

		// enable propagation of the mousedown event from map pane to parent elements in view mode
		// see bug bccu1446
		if (!window.L.DomUtil.hasClass(this._element, 'leaflet-map-pane')) {
			window.L.DomEvent.stopPropagation(e);
		}

		if (this._preventOutline) {
			window.L.DomUtil.preventOutline(this._element);
		}

		window.L.DomUtil.disableImageDrag();
		window.L.DomUtil.disableTextSelection();

		if (this._moving) { return; }

		this.fire('down');

		var first = e.touches ? e.touches[0] : e;

		this._startPoint = new cool.Point(first.clientX, first.clientY);
		this._startPos = this._newPos = window.L.DomUtil.getPosition(this._element);
		var startBoundingRect = this._element.getBoundingClientRect();
		// Store offset between mouse selection position, and top left
		// We don't use this internally, but it is needed for external
		// manipulation of the cursor position, e.g. when adjusting
		// for scrolling during cursor dragging.
		this.startOffset = this._startPoint.subtract(new cool.Point(startBoundingRect.left, startBoundingRect.top));

		window.L.DomEvent
		 .on(document, window.L.Draggable.MOVE[e.type], this.noManualDrag(this._onMove), this)
		 .on(document, window.L.Draggable.END[e.type], this.noManualDrag(this._onUp), this);
	},

	_onMove: function (e) {
		if (!this._startPoint) {
			return;
		}

		if (e.touches && e.touches.length > 1) {
			this._moved = true;
			return;
		}

		var first = (e.touches && e.touches.length === 1 ? e.touches[0] : e),
		    newPoint = new cool.Point(first.clientX, first.clientY),
		    offset = newPoint.subtract(this._startPoint);

		if (this._map) {
			if (this._map._docLayer.isCalcRTL()) {
				offset.x = -offset.x;
			}

			// needed in order to avoid a jump when the document is dragged and the mouse pointer move
			// from over the map into the html document element area which is not covered by tiles
			// (resize-detector iframe)
			if (e.currentTarget && e.currentTarget.frameElement
				&& window.L.DomUtil.hasClass(e.currentTarget.frameElement, 'resize-detector')) {
				var rect = this._map._container.getBoundingClientRect(),
				    correction = new cool.Point(rect.left, rect.top);
				offset = offset.add(correction);
			}
			if (this._map.getDocSize().x < app.activeDocument.activeLayout.frameSize.cX) {
				// don't pan horizontally when the document fits in the viewing
				// area horizontally (docWidth < viewAreaWidth)
				offset.x = 0;
			}
			if (this._map.getDocSize().y < app.activeDocument.activeLayout.frameSize.cY) {
				// don't pan vertically when the document fits in the viewing
				// area horizontally (docHeight < viewAreaHeight)
				offset.y = 0;
			}
		}
		if (!offset.x && !offset.y) { return; }
		if (window.touch.isTouchEvent(e) && Math.abs(offset.x) + Math.abs(offset.y) < 3 && !e.autoscroll) { return; }

		window.L.DomEvent.preventDefault(e);

		if (!this._moved) {
			this.fire('dragstart', {originalEvent: e});

			this._moved = true;
			this._startPos = window.L.DomUtil.getPosition(this._element).subtract(offset);

			window.L.DomUtil.addClass(document.body, 'leaflet-dragging');

			this._lastTarget = e.target || e.srcElement;
			window.L.DomUtil.addClass(this._lastTarget, 'leaflet-drag-target');
		}

		this._newPos = this._startPos.add(offset);

		if (this._freezeY)
			this._newPos.y = this._startPos.y;
		if (this._freezeX)
			this._newPos.x = this._startPos.x;

		this._moving = true;

		app.util.cancelAnimFrame(this._animRequest);
		this._lastEvent = e;

		this._animRequest = app.util.requestAnimFrame(this._updatePosition, this);
	},

	_updatePosition: function () {
		var e = {originalEvent: this._lastEvent};
		this.fire('predrag', e);
		window.L.DomUtil.setPosition(this._element, this._newPos, this._no3d);
		this.fire('drag', e);
	},

	_onUp: function (e) {
		window.L.DomUtil.removeClass(document.body, 'leaflet-dragging');

		if (this._lastTarget) {
			window.L.DomUtil.removeClass(this._lastTarget, 'leaflet-drag-target');
			this._lastTarget = null;
		}

		for (var i in window.L.Draggable.MOVE) {
			window.L.DomEvent
			 .off(document, window.L.Draggable.MOVE[i], this.noManualDrag(this._onMove), this)
			 .off(document, window.L.Draggable.END[i], this.noManualDrag(this._onUp), this);
		}

		window.L.DomUtil.enableImageDrag();
		window.L.DomUtil.enableTextSelection();

		if (this._moved && this._moving) {
			// ensure drag is not fired after dragend
			app.util.cancelAnimFrame(this._animRequest);

			this.fire('dragend', {
				originalEvent: e,
				distance: this._newPos.distanceTo(this._startPos)
			});
		} else {
			this.fire('up', {originalEvent: e});
		}

		this._moving = false;
		this._startPoint = undefined;
	}
});
