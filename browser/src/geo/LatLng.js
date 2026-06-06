/* -*- js-indent-level: 8 -*- */
/* global cool NormalPoint */
/*
 * window.L.LatLng represents a geographical point with latitude and longitude coordinates.
 */

window.L.LatLng = NormalPoint;

// Needed in Map.prototype.project()
// constructs css pixel point from LatLng at a given zoom.
// (LatLng, number) -> cool.Point
window.L.LatLng.latLngToPoint = function(latlng, zoom) {
	const scale = window.L.LatLng.scale(zoom);
	return new cool.Point(latlng.lng * scale, -latlng.lat * scale);
};

// used in Map.prototype.rescale(), only makes sense for pixel points.
// (cool.Point, number, number) -> cool.Point
window.L.LatLng.rescale = function(point, oldZoom, newZoom) {
	const scale = window.L.LatLng.scale(newZoom - oldZoom);
	return cool.Point.toPoint(
		point.x * scale,
		point.y * scale,
	);
};

// used in Map.prototype.distance()
// (LatLng, LatLng) -> number
window.L.LatLng.distance = function(latlng1, latlng2) {
	const dx = latlng2.lng - latlng1.lng;
	const dy = latlng2.lat - latlng1.lat;
	return Math.sqrt(dx * dx + dy * dy);
}

// constructs LatLng with different signatures
// (LatLng) or ([Number, Number]) or (Number, Number) or (Object)
window.L.latLng = NormalPoint.flexConstruct;
