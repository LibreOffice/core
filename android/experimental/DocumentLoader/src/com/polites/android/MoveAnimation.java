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

public class MoveAnimation implements Animation {

	private boolean firstFrame = true;

	private float startX;
	private float startY;

	private float targetX;
	private float targetY;
	private long animationTimeMS = 100;
	private long totalTime = 0;

	private MoveAnimationListener moveAnimationListener;

	/* (non-Javadoc)
	 * @see com.polites.android.Animation#update(com.polites.android.GestureImageView, long)
	 */
	@Override
	public boolean update(GestureImageView view, long time) {
		totalTime += time;

		if(firstFrame) {
			firstFrame = false;
			startX = view.getImageX();
			startY = view.getImageY();
		}

		if(totalTime < animationTimeMS) {

			float ratio = (float) totalTime / animationTimeMS;

			float newX = ((targetX - startX) * ratio) + startX;
			float newY = ((targetY - startY) * ratio) + startY;

			if(moveAnimationListener != null) {
				moveAnimationListener.onMove(newX, newY);
			}

			return true;
		}
		else {
			if(moveAnimationListener != null) {
				moveAnimationListener.onMove(targetX, targetY);
			}
		}

		return false;
	}

	public void reset() {
		firstFrame = true;
		totalTime = 0;
	}


	public float getTargetX() {
		return targetX;
	}


	public void setTargetX(float targetX) {
		this.targetX = targetX;
	}


	public float getTargetY() {
		return targetY;
	}

	public void setTargetY(float targetY) {
		this.targetY = targetY;
	}

	public long getAnimationTimeMS() {
		return animationTimeMS;
	}

	public void setAnimationTimeMS(long animationTimeMS) {
		this.animationTimeMS = animationTimeMS;
	}

	public void setMoveAnimationListener(MoveAnimationListener moveAnimationListener) {
		this.moveAnimationListener = moveAnimationListener;
	}
}
