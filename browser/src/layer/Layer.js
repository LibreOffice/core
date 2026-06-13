/* -*- js-indent-level: 8 -*- */
/* global app cool */

window.L.Layer = window.L.Evented.extend({

	options: {
		pane: 'overlayPane'
	},

	initialize: function () {
		window.L.Evented.prototype.initialize.call(this);
	},

	addTo: function (map) {
		map.addLayer(this);
		return this;
	},

	remove: function () {
		return this.removeFrom(this._map || this._mapToAdd);
	},

	removeFrom: function (obj) {
		if (obj) {
			obj.removeLayer(this);
		}
		return this;
	},

	getPane: function (name) {
		return this._map.getPane(name ? (this.options[name] || name) : this.options.pane);
	},

	addInteractiveTarget: function (targetEl) {
		this._map._targets[app.util.stamp(targetEl)] = this;
		return this;
	},

	removeInteractiveTarget: function (targetEl) {
		delete this._map._targets[app.util.stamp(targetEl)];
		return this;
	},

	_layerAdd: function (e) {
		var map = e.target;

		// check in case layer gets added and then removed before the map is ready
		if (!map.hasLayer(this)) { return; }

		this._map = map;
		this._zoomAnimated = map._zoomAnimated;

		this.onAdd(map);

		if (this.getEvents) {
			map.on(this.getEvents(), this);
		}

		this.fire('add');
		map.fire('layeradd', {layer: this});
	}
});


window.L.Map.include({
	addLayer: function (layer) {
		var id = app.util.stamp(layer);
		if (this._layers[id]) { return layer; }
		this._layers[id] = layer;

		layer._mapToAdd = this;

		if (layer.beforeAdd) {
			layer.beforeAdd(this);
		}

		this.whenReady(layer._layerAdd, layer);

		return this;
	},

	removeLayer: function (layer) {
		var id = app.util.stamp(layer);

		if (!this._layers[id]) { return this; }

		if (this._loaded) {
			layer.onRemove(this);
		}

		if (layer.getEvents) {
			this.off(layer.getEvents(), layer);
		}

		delete this._layers[id];

		if (this._loaded) {
			this.fire('layerremove', {layer: layer});
			layer.fire('remove');
		}

		layer._map = layer._mapToAdd = null;

		return this;
	},

	hasLayer: function (layer) {
		return !!layer && (app.util.stamp(layer) in this._layers);
	},

	eachLayer: function (method, context) {
		for (var i in this._layers) {
			method.call(context, this._layers[i]);
		}
		return this;
	},

	_addLayers: function (layers) {
		layers = layers ? (app.util.isArray(layers) ? layers : [layers]) : [];

		for (var i = 0, len = layers.length; i < len; i++) {
			this.addLayer(layers[i]);
		}
	},

	_addZoomLimit: function (layer) {
		if (isNaN(layer.options.maxZoom) || !isNaN(layer.options.minZoom)) {
			this._zoomBoundLayers[app.util.stamp(layer)] = layer;
			this._updateZoomLevels();
		}
	},

	_removeZoomLimit: function (layer) {
		var id = app.util.stamp(layer);

		if (this._zoomBoundLayers[id]) {
			delete this._zoomBoundLayers[id];
			this._updateZoomLevels();
		}
	},

	_updateZoomLevels: function () {
		var minZoom = Infinity,
		    maxZoom = -Infinity,
		    oldZoomSpan = this._getZoomSpan();

		for (var i in this._zoomBoundLayers) {
			var options = this._zoomBoundLayers[i].options;

			minZoom = options.minZoom === undefined ? minZoom : Math.min(minZoom, options.minZoom);
			maxZoom = options.maxZoom === undefined ? maxZoom : Math.max(maxZoom, options.maxZoom);
		}

		this._layersMaxZoom = maxZoom === -Infinity ? undefined : maxZoom;
		this._layersMinZoom = minZoom === Infinity ? undefined : minZoom;

		if (oldZoomSpan !== this._getZoomSpan()) {
			this.fire('zoomlevelschange');
		}
	}
});

// Used in window.L.Marker and L.Popup for computing layer position from Intern optionally with offsets
// with or without freeze-panes. This also indicates in the returned object
// whether the object should be visible or not when freeze panes are active.
window.L.Layer.getLayerPositionVisibility = function (intern, boundingClientRect, map, offset) {
	var splitPanesContext = map.getSplitPanesContext();

	if (!splitPanesContext) {
		return {
			position: map.internToLayerPoint(intern).round(),
			visibility: 'visible'
		};
	}

	var splitPos = splitPanesContext.getSplitPos();
	var docPos = map.project(intern);
	var docPosWithOffset = docPos.clone();
	if (offset) {
		docPosWithOffset._add(offset);
	}
	var pixelOrigin = map.getPixelOrigin();
	var mapPanePos = map._getMapPanePos();
	var mirrorX = map._docLayer.isCalcRTL();
	var layerSplitPos = splitPos.subtract(mapPanePos);
	if (mirrorX)
		layerSplitPos.x = map._size.x - splitPos.x - mapPanePos.x;

	var makeHidden = false;

	if (splitPos.x) {
		layerSplitPos.x += (mirrorX ? -1 : 1);
	}

	if (splitPos.y) {
		layerSplitPos.y += 1;
	}

	var layerPos = new cool.Point(0, 0);
	var layerPosWithOffset = new cool.Point(0, 0);
	var eps = new cool.Point(boundingClientRect.width, boundingClientRect.height);

	if (docPosWithOffset.x <= splitPos.x) {
		// fixed region.
		// mirroring should always be done in container coordinates.
		// In the fixed region, document coordinates and container coordinates are the same.
		layerPos.x = (mirrorX ? map._size.x - docPosWithOffset.x : docPos.x) - mapPanePos.x;
		layerPosWithOffset.x = (mirrorX ? map._size.x - docPos.x : docPosWithOffset.x) - mapPanePos.x;
		if (splitPos.x - docPosWithOffset.x <= eps.x) {
			// Hide the object if it is close to the split *and* the non-fixed region has moved away from the fixed.
			makeHidden = (mapPanePos.x !== pixelOrigin.x);
		}
	}
	else {
		// Movable region.
		// Mirroring should always be done in container coordinates.
		// The mirrorX expression does the following:
		// The document coordinate is first converted to container coordinate,
		// then it is mirrored w.r.t the container and finally converted to layer coordinate.
		layerPos.x = mirrorX ? map._size.x - (docPosWithOffset.x - pixelOrigin.x + mapPanePos.x) - mapPanePos.x : docPos.x - pixelOrigin.x;
		layerPosWithOffset.x = mirrorX ? map._size.x - (docPos.x - pixelOrigin.x + mapPanePos.x) - mapPanePos.x : docPosWithOffset.x - pixelOrigin.x;
		if ((!mirrorX && layerPosWithOffset.x < layerSplitPos.x) || (mirrorX && layerPosWithOffset.x >= layerSplitPos.x)) {
			// do not encroach the fixed region.
			makeHidden = true;
		}
	}

	if (docPosWithOffset.y <= splitPos.y) {
		// fixed region.
		layerPos.y = docPos.y - mapPanePos.y;
		layerPosWithOffset.y = docPosWithOffset.y - mapPanePos.y;
		if (splitPos.y - docPosWithOffset.y <= eps.y) {
			// Hide the marker if it is close to the split *and* the non-fixed region has moved away from the fixed.
			makeHidden = (mapPanePos.y !== pixelOrigin.y);
		}
	}
	else {
		layerPos.y = docPos.y - pixelOrigin.y;
		layerPosWithOffset.y = docPosWithOffset.y - pixelOrigin.y;
		if (layerPosWithOffset.y < layerSplitPos.y) {
			// do not encroach the fixed region.
			makeHidden = true;
		}
	}

	return {
		position: layerPos,
		visibility: makeHidden ? 'hidden' : 'visible'
	};
};
