/* -*- js-indent-level: 8 -*- */

/*
 * CDarkOverlay is used to render a dark overlay around an OLE object when selected
 */

import Bounds = cool.Bounds;

class CDarkOverlay extends CPathGroup {

	private rectangles: CRectangle[] = [];
	private options: any;

	constructor(pointSet: CPointSet, options: any) {
		super([]);
		this.options = options;
		this.rectangles = this.createRectangles(4);
		for (let i = 0; i < this.rectangles.length; i++)
			this.push(this.rectangles[i]);
		this.setPointSet(pointSet);
	}

	// Moves the four rectangles the group already owns to their new places.
	public setPointSet(pointSet: CPointSet) {
		var points = pointSet.getPointArray();
		if (!points) {
			for (var i = 0; i < this.rectangles.length; i++) {
				this.rectangles[i].setBounds(
					new cool.Bounds(new cool.Point(0, 0), new cool.Point(0, 1)));
			}
			return;
		}

		var rectangleBounds = this.invertOleBounds(new cool.Bounds(points[0], points[2]));

		for (var i = 0; i < this.rectangles.length; i++) {
			this.rectangles[i].setBounds(rectangleBounds[i]);
		}
	}

	private invertOleBounds(oleBounds: cool.Bounds): cool.Bounds[] {
		var rectanglesBounds: cool.Bounds[] = [];

		var minWidth = 0;
		var minHeight = 0;
		var fullWidth = 1000000;
		var fullHeight = 1000000;

		rectanglesBounds.push(new cool.Bounds(new cool.Point(minWidth, minHeight), new cool.Point(fullWidth, oleBounds.min.y)));
		rectanglesBounds.push(new cool.Bounds(new cool.Point(minWidth, oleBounds.min.y), oleBounds.getBottomLeft()));
		rectanglesBounds.push(new cool.Bounds(oleBounds.getTopRight(), new cool.Point(fullWidth, oleBounds.max.y)));
		rectanglesBounds.push(new cool.Bounds(new cool.Point(minWidth, oleBounds.max.y), new cool.Point(fullWidth, fullHeight)));

		return rectanglesBounds;
	}

	private createRectangles(quantity: number): CRectangle[] {
		var rectangles: CRectangle[] = [];
		for (var i = 0; i < quantity; i++) {
			rectangles.push(
				new CRectangle(new cool.Bounds(
					new cool.Point(0, 0), new cool.Point(0, 1)
				), this.options));
		}

		return rectangles;
	}
}
