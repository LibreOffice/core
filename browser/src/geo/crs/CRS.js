/* -*- js-indent-level: 8 -*- */
/*
 * window.L.CRS is the base object for all defined CRS (Coordinate Reference Systems) in Leaflet.
 */

/* global cool */

window.L.CRS = {
	projection: window.L.Projection.LonLat,
	transformation: new cool.Transformation(1, 0, -1, 0),
	SCALE: 1.2,

	// defines how the world scales with zoom
	scale: function (zoom) {
		return Math.pow(this.SCALE, zoom);
	},

	// equivalent to doing an unproject with oldZoom then a project with newZoom
	// except that unproject is technically invalid (so possibly confusing) for any non-css-pixel
	// but this function will work with any scaling (including twips or core pixels)
	rescale: function (point, oldZoom, newZoom) {
		return cool.Point.toPoint(
			point.x * this.scale(newZoom - oldZoom),
			point.y * this.scale(newZoom - oldZoom),
		);
	},

	distance: function (latlng1, latlng2) {
		var dx = latlng2.lng - latlng1.lng,
		    dy = latlng2.lat - latlng1.lat;

		return Math.sqrt(dx * dx + dy * dy);
	},

	// coordinate space is unbounded (infinite in all directions)
	infinite: true,
};
