/**
	 * Copyright 2010 Per-Erik Bergman (per-erik.bergman@jayway.com)
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

package libreoffice.android.ui;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

import javax.microedition.khronos.opengles.GL10;

import android.graphics.Bitmap;
import android.opengl.GLUtils;
import android.util.Log;


public class Page extends Mesh {
	/**
	 * Create a plane with a default with and height of 1 unit.
	 */
	public Page() {
		this(1, 1);
	}

	/**
	 * Create a plane.
	 * 
	 * @param width
	 *            the width of the plane.
	 * @param height
	 *            the height of the plane.
	 */
	public Page(float width, float height) {
		// Mapping coordinates for the vertices
		float textureCoordinates[] = { 
				0.0f, 1.0f, //
				1.0f, 1.0f, //
				0.0f, 0.0f, //
				1.0f, 0.0f, //
		};

		short[] indices = new short[] { 0, 1, 2, 1, 3, 2 };

		float[] vertices = new float[] { -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
				-0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f };

		setIndices(indices);
		setVertices(vertices);
		setTextureCoordinates(textureCoordinates);
	}
	
	public void zoomBy(float z){
		this.z += z;
		if ( this.z > mMaxZoom)
			this.z = mMaxZoom;
		return;
	}
	
	public void translateBy(float x, float y){
		this.x += x;
		this.y += y;
		if( Math.abs( this.x ) > 0.5f )
			this.x -= x;
		if( Math.abs( this.y ) > 0.5f )
			this.y -= y;
		return;
	}
	
	float mMaxZoom = -0.1f; //positive z values are not visible

	
	
	
	
	
}