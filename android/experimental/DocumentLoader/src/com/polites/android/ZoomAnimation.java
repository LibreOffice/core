/*
 * Copyright (c) 2012 Jason Polites
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.polites.android;

import android.graphics.PointF;

public class ZoomAnimation implements Animation {

	private boolean firstFrame = true;

	private float touchX;
	private float touchY;

	private float zoom;

	private float startX;
	private float startY;
	private float startScale;

	private float xDiff;
	private float yDiff;
	private float scaleDiff;

	private long animationLengthMS = 200;
	private long totalTime = 0;

	private ZoomAnimationListener zoomAnimationListener;

	/* (non-Javadoc)
	 * @see com.polites.android.Animation#update(com.polites.android.GestureImageView, long)
	 */
	@Override
	public boolean update(GestureImageView view, long time) {
		if(firstFrame) {
			firstFrame = false;

			startX = view.getImageX();
			startY = view.getImageY();
			startScale = view.getScale();
			scaleDiff = (zoom * startScale) - startScale;

			if(scaleDiff > 0) {
				// Calculate destination for midpoint
				VectorF vector = new VectorF();

				// Set the touch point as start because we want to move the end
				vector.setStart(new PointF(touchX, touchY));
				vector.setEnd(new PointF(startX, startY));

				vector.calculateAngle();

				// Get the current length
				float length = vector.calculateLength();

				// Multiply length by zoom to get the new length
				vector.length = length*zoom;

				// Now deduce the new endpoint
				vector.calculateEndPoint();

				xDiff = vector.end.x - startX;
				yDiff = vector.end.y - startY;
			}
			else {
				// Zoom out to center
				xDiff = view.getCenterX() - startX;
				yDiff = view.getCenterY() - startY;
			}
		}

		totalTime += time;

		float ratio = (float) totalTime / (float) animationLengthMS;

		if(ratio < 1) {

			if(ratio > 0) {
				// we still have time left
				float newScale = (ratio * scaleDiff) + startScale;
				float newX = (ratio * xDiff) + startX;
				float newY = (ratio * yDiff) + startY;

				if(zoomAnimationListener != null) {
					zoomAnimationListener.onZoom(newScale, newX, newY);
				}
			}

			return true;
		}
		else {

			float newScale = scaleDiff + startScale;
			float newX = xDiff + startX;
			float newY = yDiff + startY;

			if(zoomAnimationListener != null) {
				zoomAnimationListener.onZoom(newScale, newX, newY);
				zoomAnimationListener.onComplete();
			}

			return false;
		}
	}

	public void reset() {
		firstFrame = true;
		totalTime = 0;
	}

	public float getZoom() {
		return zoom;
	}

	public void setZoom(float zoom) {
		this.zoom = zoom;
	}

	public float getTouchX() {
		return touchX;
	}

	public void setTouchX(float touchX) {
		this.touchX = touchX;
	}

	public float getTouchY() {
		return touchY;
	}

	public void setTouchY(float touchY) {
		this.touchY = touchY;
	}

	public long getAnimationLengthMS() {
		return animationLengthMS;
	}

	public void setAnimationLengthMS(long animationLengthMS) {
		this.animationLengthMS = animationLengthMS;
	}

	public ZoomAnimationListener getZoomAnimationListener() {
		return zoomAnimationListener;
	}

	public void setZoomAnimationListener(ZoomAnimationListener zoomAnimationListener) {
		this.zoomAnimationListener = zoomAnimationListener;
	}
}
