/* -*- js-indent-level: 8 -*- */
/* global NormalPoint */
/*
 * window.L.LatLngBounds represents a rectangular area on the map in geographical coordinates.
 */

class NormalBounds {

	constructor(southWest, northEast) { // (LatLng, LatLng) or (LatLng[])
		if (!southWest) { return; }

		var latlngs = northEast ? [southWest, northEast] : southWest;

		for (var i = 0, len = latlngs.length; i < len; i++) {
			this.extend(latlngs[i]);
		}
	}

	// extend the bounds to contain the given point or bounds
	extend(obj) { // (LatLng) or (LatLngBounds)
		var sw = this._southWest,
		    ne = this._northEast,
		    sw2, ne2;

		if (obj instanceof NormalPoint) {
			sw2 = obj;
			ne2 = obj;

		} else if (obj instanceof NormalBounds) {
			sw2 = obj._southWest;
			ne2 = obj._northEast;

			if (!sw2 || !ne2) { return this; }

		} else {
			return obj ? this.extend(window.L.latLng(obj) || window.L.latLngBounds(obj)) : this;
		}

		if (!sw && !ne) {
			this._southWest = new NormalPoint(sw2.lat, sw2.lng);
			this._northEast = new NormalPoint(ne2.lat, ne2.lng);
		} else {
			sw.lat = Math.min(sw2.lat, sw.lat);
			sw.lng = Math.min(sw2.lng, sw.lng);
			ne.lat = Math.max(ne2.lat, ne.lat);
			ne.lng = Math.max(ne2.lng, ne.lng);
		}

		return this;
	}

	getCenter() { // -> LatLng
		return new NormalPoint(
		        (this._southWest.lat + this._northEast.lat) / 2,
		        (this._southWest.lng + this._northEast.lng) / 2);
	}

	getSouthWest() {
		return this._southWest;
	}

	getNorthEast() {
		return this._northEast;
	}

	getNorthWest() {
		return new NormalPoint(this.getNorth(), this.getWest());
	}

	getSouthEast() {
		return new NormalPoint(this.getSouth(), this.getEast());
	}

	getWest() {
		return this._southWest.lng;
	}

	getSouth() {
		return this._southWest.lat;
	}

	getEast() {
		return this._northEast.lng;
	}

	getNorth() {
		return this._northEast.lat;
	}

	getWidth() {
		return Math.abs(this.getEast() - this.getWest());
	}

	getHeight() {
		return Math.abs(this.getNorth() - this.getSouth());
	}

	_getAsLatLngOrBounds (obj) {
		if (typeof obj[0] === 'number' || obj instanceof NormalPoint) {
			obj = window.L.latLng(obj);
		} else {
			obj = window.L.latLngBounds(obj);
		}
		return obj;
	}

	contains(obj) { // (LatLngBounds) or (LatLng) -> Boolean
		obj = this._getAsLatLngOrBounds(obj);

		var sw = this._southWest,
		    ne = this._northEast,
		    sw2, ne2;

		if (obj instanceof NormalBounds) {
			sw2 = obj.getSouthWest();
			ne2 = obj.getNorthEast();
		} else {
			sw2 = ne2 = obj;
		}

		return (sw2.lat >= sw.lat) && (ne2.lat <= ne.lat) &&
		       (sw2.lng >= sw.lng) && (ne2.lng <= ne.lng);
	}

	intersects(bounds) { // (LatLngBounds)
		bounds = window.L.latLngBounds(bounds);

		var sw = this._southWest,
		    ne = this._northEast,
		    sw2 = bounds.getSouthWest(),
		    ne2 = bounds.getNorthEast(),

		    latIntersects = (ne2.lat >= sw.lat) && (sw2.lat <= ne.lat),
		    lngIntersects = (ne2.lng >= sw.lng) && (sw2.lng <= ne.lng);

		return latIntersects && lngIntersects;
	}

	equals(bounds) { // (LatLngBounds)
		if (!bounds) { return false; }

		bounds = window.L.latLngBounds(bounds);

		return this._southWest.equals(bounds.getSouthWest()) &&
		       this._northEast.equals(bounds.getNorthEast());
	}

	isValid() {
		return !!(this._southWest && this._northEast);
	}

	isInAny(latLngBoundsArray) {
		window.app.console.assert(Array.isArray(latLngBoundsArray), 'invalid argument type');

		for (var i = 0; i < latLngBoundsArray.length; ++i) {
			if (latLngBoundsArray[i].contains(this)) {
				return true;
			}
		}

		return false;
	}

	// Please do not remove even if unused. It can be useful in
	// temporary console.log() etc.
	toString() {
		return 'LatLngBounds(' + this._southWest.toString() + ',' + this._northEast.toString() + ')';
	}
};

window.L.LatLngBounds = NormalBounds;

window.L.latLngBounds = function (a, b) { // (LatLngBounds) or (LatLng, LatLng)
	if (!a || a instanceof NormalBounds) {
		return a;
	}
	return new NormalBounds(a, b);
};
