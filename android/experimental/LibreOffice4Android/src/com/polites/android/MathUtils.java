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
import android.util.FloatMath;
import android.view.MotionEvent;

public class MathUtils {

	public static float distance(MotionEvent event) {
		float x = event.getX(0) - event.getX(1);
		float y = event.getY(0) - event.getY(1);
		return FloatMath.sqrt(x * x + y * y);
	}

	public static float distance(PointF p1, PointF p2) {
		float x = p1.x - p2.x;
		float y = p1.y - p2.y;
		return FloatMath.sqrt(x * x + y * y);
	}

	public static float distance(float x1, float y1, float x2, float y2) {
		float x = x1 - x2;
		float y = y1 - y2;
		return FloatMath.sqrt(x * x + y * y);
	}

	public static void midpoint(MotionEvent event, PointF point) {
		float x1 = event.getX(0);
		float y1 = event.getY(0);
		float x2 = event.getX(1);
		float y2 = event.getY(1);
		midpoint(x1, y1, x2, y2, point);
	}

	public static void midpoint(float x1, float y1, float x2, float y2, PointF point) {
		point.x = (x1 + x2) / 2.0f;
		point.y = (y1 + y2) / 2.0f;
	}
	/**
	 * Rotates p1 around p2 by angle degrees.
	 * @param p1
	 * @param p2
	 * @param angle
	 */
	public void rotate(PointF p1, PointF p2, float angle) {
		float px = p1.x;
		float py = p1.y;
		float ox = p2.x;
		float oy = p2.y;
		p1.x = (FloatMath.cos(angle) * (px-ox) - FloatMath.sin(angle) * (py-oy) + ox);
		p1.y = (FloatMath.sin(angle) * (px-ox) + FloatMath.cos(angle) * (py-oy) + oy);
	}

	public static float angle(PointF p1, PointF p2) {
		return angle(p1.x, p1.y, p2.x, p2.y);
	}

	public static float angle(float x1, float y1, float x2, float y2) {
		return (float) Math.atan2(y2 - y1, x2 - x1);
	}
}
