/* -*- js-indent-level: 8 -*- */
/* global NormalPoint NormalBoundsBase */
/*
 * window.L.LatLngBounds represents a rectangular area on the map in geographical coordinates.
 */

class NormalBounds extends NormalBoundsBase {

	constructor(southWest, northEast) { // (LatLng, LatLng) or (LatLng[])
		super(southWest, northEast);
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

NormalBounds.flexConstruct = function (a, b) { // (LatLngBounds) or (LatLng, LatLng)
	if (!a || a instanceof NormalBounds) {
		return a;
	}
	return new NormalBounds(a, b);
};


window.L.LatLngBounds = NormalBounds;
window.L.latLngBounds = NormalBounds.flexConstruct;
