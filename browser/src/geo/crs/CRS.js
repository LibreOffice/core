/* -*- js-indent-level: 8 -*- */
/*
 * window.L.CRS is the base object for all defined CRS (Coordinate Reference Systems) in Leaflet.
 */

/* global cool */

window.L.CRS = {
	projection: window.L.Projection.LonLat,
	transformation: new cool.Transformation(1, 0, -1, 0),
	SCALE: 1.2,

	// coordinate space is unbounded (infinite in all directions)
	infinite: true,
};
