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
	What dragging one of the eight handles does to the shape they frame. Everything here is
	worked out from the shape as it stands, the handle that is dragged and where the mouse is,
	so that whoever draws the handles can ask for it: the section that gives each handle a piece
	of canvas of its own, and the one that draws them all itself.

	The shape is described the way the handles carry it: a center in core pixels, a width and a
	height in core pixels and the angle it is turned by, counted in radians against the clock.
	A handle is named by its kind, "1" for the upper left one to "8" for the lower right, and
	the place it sits in is counted from zero, "0" to "7", which is how the engine numbers them.
*/
class HandleScaling {
	/*
		Whether the drag keeps the ratio between width and height. Control and Shift together
		ask for it, and for an image or a video they ask for the opposite, because keeping the
		ratio is what those do on their own.
	*/
	public static keepsRatio(
		event: MouseEvent | undefined,
		cropMode: boolean,
	): boolean {
		if (cropMode) return false;

		/*
			Where the client draws the drag itself, Shift is the key, as it is in the office: a
			shape keeps its ratio while Shift is held. Where the engine draws the drag the older
			rule stays, Ctrl and Shift together, since that is what the engine is sent and what
			it shows.
		*/
		let keep =
			event !== undefined &&
			event.shiftKey &&
			(RenderManager.drawsFromObjects() || event.ctrlKey);

		const context = app.map.context?.context;
		if (context === 'Graphic' || context === 'Media') keep = !keep;

		return keep;
	}

	/*
		The handle that is committed at the end of the drag. A handle in the middle of a side
		moves one edge only, so a drag that keeps the ratio grows a corner and that corner is
		what the engine is told about.
	*/
	public static committedHandle(handleId: string, keepRatio: boolean): string {
		if (!keepRatio) return handleId;

		const sideToCorner: Record<string, string> = app.map._docLayer.isCalcRTL()
			? { '1': '0', '3': '5', '4': '7', '6': '5' }
			: { '1': '2', '3': '5', '4': '7', '6': '7' };

		return sideToCorner[handleId] ?? handleId;
	}

	/// Where the handle counted as handleId sits on that rectangle, in twips.
	public static positionOfHandle(
		handleId: string,
		rectangle: cool.SimpleRectangle,
	): number[] {
		const isRTL = app.map._docLayer.isCalcRTL();
		const leftHandleX = isRTL ? rectangle.x2 : rectangle.x1;
		const rightHandleX = isRTL ? rectangle.x1 : rectangle.x2;

		if (handleId === '0') return [leftHandleX, rectangle.y1];
		else if (handleId === '1') return [rectangle.center[0], rectangle.y1];
		else if (handleId === '2') return [rightHandleX, rectangle.y1];
		else if (handleId === '3') return [leftHandleX, rectangle.center[1]];
		else if (handleId === '4') return [rightHandleX, rectangle.center[1]];
		else if (handleId === '5') return [leftHandleX, rectangle.y2];
		else if (handleId === '6') return [rectangle.center[0], rectangle.y2];
		// handleId === '7'
		else return [rightHandleX, rectangle.y2];
	}

	/// Takes the point onto the line that keeps the ratio, and modifies it.
	private static ratioPoint(
		point: cool.SimplePoint,
		shape: any,
		kind: string,
	): void {
		const isVerticalHandler = ['2', '7'].includes(kind);

		const primaryDelta = isVerticalHandler
			? point.pY - shape.center.pY
			: point.pX - shape.center.pX;

		const aspectRatio = isVerticalHandler
			? shape.width / shape.height
			: shape.height / shape.width;

		// The kind-based direction table assumes LTR doc-X. In RTL, the abs in
		// convertToTileTwipsIfNeeded mirrors the X axis (kinds 1/4/6 sit at
		// doc-max-X, 3/5/8 at doc-min-X), so the secondary axis runs opposite
		// to user intent.
		let secondaryDelta = primaryDelta * aspectRatio;
		if (app.map._docLayer.isCalcRTL()) secondaryDelta = -secondaryDelta;

		const direction = ['3', '4', '6', '2'].includes(kind) ? -1 : 1;

		if (isVerticalHandler)
			point.pX = shape.center.pX + secondaryDelta * direction;
		else point.pY = shape.center.pY + secondaryDelta * direction;
	}

	/*
		The shape as the drag leaves it: the center it moved to and the width and height it took
		on. The point is where the mouse is, in the document, and it is modified on the way.
	*/
	public static shapeAfterDrag(
		point: cool.SimplePoint,
		shapeRectangleProperties: any,
		kind: string,
		keepRatio: boolean,
	): any {
		const shapeRecProps: any = structuredClone(shapeRectangleProperties);
		shapeRecProps.center = shapeRectangleProperties.center.clone();

		if (keepRatio) HandleScaling.ratioPoint(point, shapeRecProps, kind);

		const diff = [
			point.pX - shapeRecProps.center.pX,
			-(point.pY - shapeRecProps.center.pY),
		];
		const length = Math.pow(Math.pow(diff[0], 2) + Math.pow(diff[1], 2), 0.5);
		const pointAngle = Math.atan2(diff[1], diff[0]);
		point.pX =
			shapeRecProps.center.pX +
			length * Math.cos(pointAngle - shapeRecProps.angleRadian);
		point.pY =
			shapeRecProps.center.pY -
			length * Math.sin(pointAngle - shapeRecProps.angleRadian);

		const rectangle = new cool.SimpleRectangle(
			(shapeRecProps.center.pX - shapeRecProps.width * 0.5) * app.pixelsToTwips,
			(shapeRecProps.center.pY - shapeRecProps.height * 0.5) *
				app.pixelsToTwips,
			shapeRecProps.width * app.pixelsToTwips,
			shapeRecProps.height * app.pixelsToTwips,
		);

		const oldpCenter = rectangle.pCenter;

		// In RTL, convertToTileTwipsIfNeeded's abs flips handle doc-X: kinds
		// 1/4/6 land at doc-max-X (= pX2 side) and 3/5/8 at doc-min-X (pX1).
		// So the edge each handle modifies is swapped relative to LTR.
		const isRTL = app.map._docLayer.isCalcRTL();
		const isMinXHandle = isRTL
			? ['3', '5', '8'].includes(kind)
			: ['1', '4', '6'].includes(kind);
		const isMaxXHandle = isRTL
			? ['1', '4', '6'].includes(kind)
			: ['3', '5', '8'].includes(kind);

		if (isMinXHandle) {
			const pX2 = rectangle.pX2;
			rectangle.pX1 = point.pX;
			rectangle.pX2 = pX2;
		} else if (isMaxXHandle) rectangle.pX2 = point.pX;

		if (['1', '2', '3'].includes(kind)) {
			const pY2 = rectangle.pY2;
			rectangle.pY1 = point.pY;
			rectangle.pY2 = pY2;
		} else if (['6', '7', '8'].includes(kind)) rectangle.pY2 = point.pY;

		if (keepRatio) {
			if (['4', '5'].includes(kind)) {
				rectangle.pY2 = point.pY;
			} else if (['2', '7'].includes(kind)) {
				if (isRTL) {
					const pX2 = rectangle.pX2;
					rectangle.pX1 = point.pX;
					rectangle.pX2 = pX2;
				} else {
					rectangle.pX2 = point.pX;
				}
			}
		}

		const centerAngle = Math.atan2(
			oldpCenter[1] - rectangle.pCenter[1],
			rectangle.pCenter[0] - oldpCenter[0],
		);
		const centerLength = Math.pow(
			Math.pow(rectangle.pCenter[1] - oldpCenter[1], 2) +
				Math.pow(rectangle.pCenter[0] - oldpCenter[0], 2),
			0.5,
		);

		const x = centerLength * Math.cos(shapeRecProps.angleRadian + centerAngle);
		const y = centerLength * Math.sin(shapeRecProps.angleRadian + centerAngle);

		shapeRecProps.center.pX += x;
		shapeRecProps.center.pY -= y;
		shapeRecProps.width = rectangle.pWidth;
		shapeRecProps.height = rectangle.pHeight;

		return shapeRecProps;
	}
}
