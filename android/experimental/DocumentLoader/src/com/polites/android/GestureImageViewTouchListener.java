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

import android.content.res.Configuration;
import android.graphics.PointF;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;

public class GestureImageViewTouchListener implements OnTouchListener {

	private GestureImageView image;
	private OnClickListener onClickListener;

	private final PointF current = new PointF();
	private final PointF last = new PointF();
	private final PointF next = new PointF();
	private final PointF midpoint = new PointF();

	private final VectorF scaleVector = new VectorF();
	private final VectorF pinchVector = new VectorF();

	private boolean touched = false;
	private boolean inZoom = false;

	private float initialDistance;
	private float lastScale = 1.0f;
	private float currentScale = 1.0f;

	private float boundaryLeft = 0;
	private float boundaryTop = 0;
	private float boundaryRight = 0;
	private float boundaryBottom = 0;

	private float maxScale = 5.0f;
	private float minScale = 0.25f;
	private float fitScaleHorizontal = 1.0f;
	private float fitScaleVertical = 1.0f;

	private int canvasWidth = 0;
	private int canvasHeight = 0;

	private float centerX = 0;
	private float centerY = 0;

	private float startingScale = 0;

	private boolean canDragX = false;
	private boolean canDragY = false;

	private boolean multiTouch = false;

	private int displayWidth;
	private int displayHeight;

	private int imageWidth;
	private int imageHeight;

	private FlingListener flingListener;
	private FlingAnimation flingAnimation;
	private ZoomAnimation zoomAnimation;
	private MoveAnimation moveAnimation;
	private GestureDetector tapDetector;
	private GestureDetector flingDetector;
	private GestureImageViewListener imageListener;

	class FlingListener extends SimpleOnGestureListener {

		private float velocityX;
		private float velocityY;

		@Override
		public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
			this.velocityX = velocityX;
			this.velocityY = velocityY;
			if (image.overflowGestureListener != null && !canDragX && !canDragY) {
				return image.overflowGestureListener.onFling(e1, e2, velocityX, velocityY);
			}

			return true;
		}

		public float getVelocityX() {
			return velocityX;
		}

		public float getVelocityY() {
			return velocityY;
		}
	}

	public GestureImageViewTouchListener(final GestureImageView image, int displayWidth, int displayHeight) {
		super();

		this.image = image;

		this.displayWidth = displayWidth;
		this.displayHeight = displayHeight;

		this.centerX = (float) displayWidth / 2.0f;
		this.centerY = (float) displayHeight / 2.0f;

		this.imageWidth = image.getImageWidth();
		this.imageHeight = image.getImageHeight();

		startingScale = image.getScale();

		currentScale = startingScale;
		lastScale = startingScale;

		boundaryRight = displayWidth;
		boundaryBottom = displayHeight;
		boundaryLeft = 0;
		boundaryTop = 0;

		next.x = image.getImageX();
		next.y = image.getImageY();

		flingListener = new FlingListener();
		flingAnimation = new FlingAnimation();
		zoomAnimation = new ZoomAnimation();
		moveAnimation = new MoveAnimation();

		flingAnimation.setListener(new FlingAnimationListener() {
			@Override
			public void onMove(float x, float y) {
				handleDrag(current.x + x, current.y + y);
			}

			@Override
			public void onComplete() {}
		});

		zoomAnimation.setZoom(2.0f);
		zoomAnimation.setZoomAnimationListener(new ZoomAnimationListener() {
			@Override
			public void onZoom(float scale, float x, float y) {
				if(scale <= maxScale && scale >= minScale) {
					handleScale(scale, x, y);
				}
			}

			@Override
			public void onComplete() {
				inZoom = false;
				handleUp();
			}
		});

		moveAnimation.setMoveAnimationListener(new MoveAnimationListener() {

			@Override
			public void onMove(float x, float y) {
				image.setPosition(x, y);
				image.redraw();
			}
		});

		tapDetector = new GestureDetector(image.getContext(), new SimpleOnGestureListener() {
			@Override
			public boolean onDoubleTap(MotionEvent e) {
				startZoom(e);
				return true;
			}

			@Override
			public boolean onSingleTapConfirmed(MotionEvent e) {
				if(!inZoom) {
					if(onClickListener != null) {
						onClickListener.onClick(image);
						return true;
					}
				}

				return false;
			}
		});

		flingDetector = new GestureDetector(image.getContext(), flingListener);
		imageListener = image.getGestureImageViewListener();

		calculateBoundaries();
	}

	private void startFling() {
		flingAnimation.setVelocityX(flingListener.getVelocityX());
		flingAnimation.setVelocityY(flingListener.getVelocityY());
		image.animationStart(flingAnimation);
	}

	private void startZoom(MotionEvent e) {
		inZoom = true;
		zoomAnimation.reset();

		float zoomTo = 1.0f;

		if(image.isLandscape()) {
			if(image.getDeviceOrientation() == Configuration.ORIENTATION_PORTRAIT) {
				int scaledHeight = image.getScaledHeight();

				if(scaledHeight < canvasHeight) {
					zoomTo = fitScaleVertical / currentScale;
					zoomAnimation.setTouchX(e.getX());
					zoomAnimation.setTouchY(image.getCenterY());
				}
				else {
					zoomTo = fitScaleHorizontal / currentScale;
					zoomAnimation.setTouchX(image.getCenterX());
					zoomAnimation.setTouchY(image.getCenterY());
				}
			}
			else {
				int scaledWidth = image.getScaledWidth();

				if(scaledWidth == canvasWidth) {
					zoomTo = currentScale*4.0f;
					zoomAnimation.setTouchX(e.getX());
					zoomAnimation.setTouchY(e.getY());
				}
				else if(scaledWidth < canvasWidth) {
					zoomTo = fitScaleHorizontal / currentScale;
					zoomAnimation.setTouchX(image.getCenterX());
					zoomAnimation.setTouchY(e.getY());
				}
				else {
					zoomTo = fitScaleHorizontal / currentScale;
					zoomAnimation.setTouchX(image.getCenterX());
					zoomAnimation.setTouchY(image.getCenterY());
				}
			}
		}
		else {
			if(image.getDeviceOrientation() == Configuration.ORIENTATION_PORTRAIT) {

				int scaledHeight = image.getScaledHeight();

				if(scaledHeight == canvasHeight) {
					zoomTo = currentScale*4.0f;
					zoomAnimation.setTouchX(e.getX());
					zoomAnimation.setTouchY(e.getY());
				}
				else if(scaledHeight < canvasHeight) {
					zoomTo = fitScaleVertical / currentScale;
					zoomAnimation.setTouchX(e.getX());
					zoomAnimation.setTouchY(image.getCenterY());
				}
				else {
					zoomTo = fitScaleVertical / currentScale;
					zoomAnimation.setTouchX(image.getCenterX());
					zoomAnimation.setTouchY(image.getCenterY());
				}
			}
			else {
				int scaledWidth = image.getScaledWidth();

				if(scaledWidth < canvasWidth) {
					zoomTo = fitScaleHorizontal / currentScale;
					zoomAnimation.setTouchX(image.getCenterX());
					zoomAnimation.setTouchY(e.getY());
				}
				else {
					zoomTo = fitScaleVertical / currentScale;
					zoomAnimation.setTouchX(image.getCenterX());
					zoomAnimation.setTouchY(image.getCenterY());
				}
			}
		}

		zoomAnimation.setZoom(zoomTo);
		image.animationStart(zoomAnimation);
	}


	private void stopAnimations() {
		image.animationStop();
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {

		if(!inZoom) {

			if(!tapDetector.onTouchEvent(event)) {
				if(event.getPointerCount() == 1 && flingDetector.onTouchEvent(event)) {
					startFling();
				}

				if(event.getAction() == MotionEvent.ACTION_UP) {
					handleUp();
				}
				else if(event.getAction() == MotionEvent.ACTION_DOWN) {
					stopAnimations();

					last.x = event.getX();
					last.y = event.getY();

					if(imageListener != null) {
						imageListener.onTouch(last.x, last.y);
					}

					touched = true;
				}
				else if(event.getAction() == MotionEvent.ACTION_MOVE) {
					if(event.getPointerCount() > 1) {
						multiTouch = true;
						if(initialDistance > 0) {

							pinchVector.set(event);
							pinchVector.calculateLength();

							float distance = pinchVector.length;

							if(initialDistance != distance) {

								float newScale = (distance / initialDistance) * lastScale;

								if(newScale <= maxScale) {
									scaleVector.length *= newScale;

									scaleVector.calculateEndPoint();

									scaleVector.length /= newScale;

									float newX = scaleVector.end.x;
									float newY = scaleVector.end.y;

									handleScale(newScale, newX, newY);
								}
							}
						}
						else {
							initialDistance = MathUtils.distance(event);

							MathUtils.midpoint(event, midpoint);

							scaleVector.setStart(midpoint);
							scaleVector.setEnd(next);

							scaleVector.calculateLength();
							scaleVector.calculateAngle();

							scaleVector.length /= lastScale;
						}
					}
					else {
						if(!touched) {
							touched = true;
							last.x = event.getX();
							last.y = event.getY();
							next.x = image.getImageX();
							next.y = image.getImageY();
						}
						else if(!multiTouch) {
							if(handleDrag(event.getX(), event.getY())) {
								image.redraw();
							}
						}
					}
				}
			}
		}

		return true;
	}

	protected void handleUp() {

		multiTouch = false;

		initialDistance = 0;
		lastScale = currentScale;

		if(!canDragX) {
			next.x = centerX;
		}

		if(!canDragY) {
			next.y = centerY;
		}

		boundCoordinates();

		if(!canDragX && !canDragY) {

			if(image.isLandscape()) {
				currentScale = fitScaleHorizontal;
				lastScale = fitScaleHorizontal;
			}
			else {
				currentScale = fitScaleVertical;
				lastScale = fitScaleVertical;
			}
		}

		image.setScale(currentScale);
		image.setPosition(next.x, next.y);

		if(imageListener != null) {
			imageListener.onScale(currentScale);
			imageListener.onPosition(next.x, next.y);
		}

		image.redraw();
	}

	protected void handleScale(float scale, float x, float y) {

		currentScale = scale;

		if(currentScale > maxScale) {
			currentScale = maxScale;
		}
		else if (currentScale < minScale) {
			currentScale = minScale;
		}
		else {
			next.x = x;
			next.y = y;
		}

		calculateBoundaries();

		image.setScale(currentScale);
		image.setPosition(next.x, next.y);

		if(imageListener != null) {
			imageListener.onScale(currentScale);
			imageListener.onPosition(next.x, next.y);
		}

		image.redraw();
	}

	protected boolean handleDrag(float x, float y) {
		current.x = x;
		current.y = y;

		float diffX = (current.x - last.x);
		float diffY = (current.y - last.y);

		if(diffX != 0 || diffY != 0) {

			if(canDragX) next.x += diffX;
			if(canDragY) next.y += diffY;

			boundCoordinates();

			last.x = current.x;
			last.y = current.y;

			if(canDragX || canDragY) {
				image.setPosition(next.x, next.y);

				if(imageListener != null) {
					imageListener.onPosition(next.x, next.y);
				}

				return true;
			}
		}

		return false;
	}

	public void reset() {
		currentScale = startingScale;
		next.x = centerX;
		next.y = centerY;
		calculateBoundaries();
		image.setScale(currentScale);
		image.setPosition(next.x, next.y);
		image.redraw();
	}


	public float getMaxScale() {
		return maxScale;
	}

	public void setMaxScale(float maxScale) {
		this.maxScale = maxScale;
	}

	public float getMinScale() {
		return minScale;
	}

	public void setMinScale(float minScale) {
		this.minScale = minScale;
	}

	public void setOnClickListener(OnClickListener onClickListener) {
		this.onClickListener = onClickListener;
	}

	protected void setCanvasWidth(int canvasWidth) {
		this.canvasWidth = canvasWidth;
	}

	protected void setCanvasHeight(int canvasHeight) {
		this.canvasHeight = canvasHeight;
	}

	protected void setFitScaleHorizontal(float fitScale) {
		this.fitScaleHorizontal = fitScale;
	}

	protected void setFitScaleVertical(float fitScaleVertical) {
		this.fitScaleVertical = fitScaleVertical;
	}

	protected void boundCoordinates() {
		if(next.x < boundaryLeft) {
			next.x = boundaryLeft;
		}
		else if(next.x > boundaryRight) {
			next.x = boundaryRight;
		}

		if(next.y < boundaryTop) {
			next.y = boundaryTop;
		}
		else if(next.y > boundaryBottom) {
			next.y = boundaryBottom;
		}
	}

	protected void calculateBoundaries() {

		int effectiveWidth = Math.round( (float) imageWidth * currentScale );
		int effectiveHeight = Math.round( (float) imageHeight * currentScale );

		canDragX = effectiveWidth > displayWidth;
		canDragY = effectiveHeight > displayHeight;

		if(canDragX) {
			float diff = (float)(effectiveWidth - displayWidth) / 2.0f;
			boundaryLeft = centerX - diff;
			boundaryRight = centerX + diff;
		}

		if(canDragY) {
			float diff = (float)(effectiveHeight - displayHeight) / 2.0f;
			boundaryTop = centerY - diff;
			boundaryBottom = centerY + diff;
		}
	}
}
