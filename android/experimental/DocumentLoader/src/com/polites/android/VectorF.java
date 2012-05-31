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

public class VectorF {

	public float angle;
	public float length;

	public final PointF start = new PointF();
	public final PointF end = new PointF();

	public void calculateEndPoint() {
		end.x = FloatMath.cos(angle) * length + start.x;
		end.y = FloatMath.sin(angle) * length + start.y;
	}

	public void setStart(PointF p) {
		this.start.x = p.x;
		this.start.y = p.y;
	}

	public void setEnd(PointF p) {
		this.end.x = p.x;
		this.end.y = p.y;
	}

	public void set(MotionEvent event) {
		this.start.x = event.getX(0);
		this.start.y = event.getY(0);
		this.end.x = event.getX(1);
		this.end.y = event.getY(1);
	}

	public float calculateLength() {
		length = MathUtils.distance(start, end);
		return length;
	}

	public float calculateAngle() {
		angle = MathUtils.angle(start, end);
		return angle;
	}


}
