/*
 * Copyright (C) 2010 Neil Davies
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This code is base on the Android Gallery widget and was Created
 * by Neil Davies neild001 'at' gmail dot com to be a Coverflow widget
 *
 * @author Neil Davies
 */
package pl.polidea.coverflow;

import org.libreoffice.impressremote.R;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Camera;
import android.graphics.Matrix;
import android.util.AttributeSet;
import android.view.View;
import android.view.animation.Transformation;
import android.widget.Gallery;
import android.widget.ImageView;
import android.widget.SpinnerAdapter;

/**
 * Cover Flow implementation.
 *
 */
public class CoverFlow extends Gallery {

	/**
	 * Graphics Camera used for transforming the matrix of ImageViews.
	 */
	private final Camera mCamera = new Camera();

	/**
	 * The maximum angle the Child ImageView will be rotated by.
	 */
	private int mMaxRotationAngle = 60;

	/**
	 * The maximum zoom on the centre Child.
	 */
	private int mMaxZoom = -160;

	/**
	 * The Centre of the Coverflow.
	 */
	private int mCoveflowCenter;

	/** The image height. */
	private float imageHeight;

	/** The image width. */
	private float imageWidth;

	/** The reflection gap. */
	private float reflectionGap;

	/** The with reflection. */
	private boolean withReflection;

	/** The image reflection ratio. */
	private float imageReflectionRatio;

	/**
	 * Gets the image height.
	 *
	 * @return the image height
	 */
	public float getImageHeight() {
		return imageHeight;
	}

	/**
	 * Sets the image height.
	 *
	 * @param imageHeight
	 *            the new image height
	 */
	public void setImageHeight(final float imageHeight) {
		this.imageHeight = imageHeight;
	}

	/**
	 * Gets the image width.
	 *
	 * @return the image width
	 */
	public float getImageWidth() {
		return imageWidth;
	}

	/**
	 * Sets the image width.
	 *
	 * @param imageWidth
	 *            the new image width
	 */
	public void setImageWidth(final float imageWidth) {
		this.imageWidth = imageWidth;
	}

	/**
	 * Gets the reflection gap.
	 *
	 * @return the reflection gap
	 */
	public float getReflectionGap() {
		return reflectionGap;
	}

	/**
	 * Sets the reflection gap.
	 *
	 * @param reflectionGap
	 *            the new reflection gap
	 */
	public void setReflectionGap(final float reflectionGap) {
		this.reflectionGap = reflectionGap;
	}

	/**
	 * Checks if is with reflection.
	 *
	 * @return true, if is with reflection
	 */
	public boolean isWithReflection() {
		return withReflection;
	}

	/**
	 * Sets the with reflection.
	 *
	 * @param withReflection
	 *            the new with reflection
	 */
	public void setWithReflection(final boolean withReflection) {
		this.withReflection = withReflection;
	}

	/**
	 * Sets the image reflection ratio.
	 *
	 * @param imageReflectionRatio
	 *            the new image reflection ratio
	 */
	public void setImageReflectionRatio(final float imageReflectionRatio) {
		this.imageReflectionRatio = imageReflectionRatio;
	}

	/**
	 * Gets the image reflection ratio.
	 *
	 * @return the image reflection ratio
	 */
	public float getImageReflectionRatio() {
		return imageReflectionRatio;
	}

	public CoverFlow(final Context context) {
		super(context);
		this.setStaticTransformationsEnabled(true);
	}

	public CoverFlow(final Context context, final AttributeSet attrs) {
		this(context, attrs, android.R.attr.galleryStyle);
	}

	public CoverFlow(final Context context, final AttributeSet attrs,
	                final int defStyle) {
		super(context, attrs, defStyle);
		parseAttributes(context, attrs);
		this.setStaticTransformationsEnabled(true);
	}

	/**
	 * Get the max rotational angle of the image.
	 *
	 * @return the mMaxRotationAngle
	 */
	public int getMaxRotationAngle() {
		return mMaxRotationAngle;
	}

	/**
	 * Sets the.
	 *
	 * @param adapter
	 *            the new adapter
	 */
	@Override
	public void setAdapter(final SpinnerAdapter adapter) {
		if (!(adapter instanceof AbstractCoverFlowImageAdapter)) {
			throw new IllegalArgumentException(
			                "The adapter should derive from "
			                                + AbstractCoverFlowImageAdapter.class
			                                                .getName());
		}
		final AbstractCoverFlowImageAdapter coverAdapter = (AbstractCoverFlowImageAdapter) adapter;
		coverAdapter.setWidth(imageWidth);
		coverAdapter.setHeight(imageHeight);
		if (withReflection) {
			final ReflectingImageAdapter reflectAdapter = new ReflectingImageAdapter(
			                coverAdapter);
			reflectAdapter.setReflectionGap(reflectionGap);
			reflectAdapter.setWidthRatio(imageReflectionRatio);
			reflectAdapter.setWidth(imageWidth);
			reflectAdapter.setHeight(imageHeight * (1 + imageReflectionRatio));
			super.setAdapter(reflectAdapter);
		} else {
			super.setAdapter(adapter);
		}
	}

	/**
	 * Set the max rotational angle of each image.
	 *
	 * @param maxRotationAngle
	 *            the mMaxRotationAngle to set
	 */
	public void setMaxRotationAngle(final int maxRotationAngle) {
		mMaxRotationAngle = maxRotationAngle;
	}

	/**
	 * Get the Max zoom of the centre image.
	 *
	 * @return the mMaxZoom
	 */
	public int getMaxZoom() {
		return mMaxZoom;
	}

	/**
	 * Set the max zoom of the centre image.
	 *
	 * @param maxZoom
	 *            the mMaxZoom to set
	 */
	public void setMaxZoom(final int maxZoom) {
		mMaxZoom = maxZoom;
	}

	/**
	 * Get the Centre of the Coverflow.
	 *
	 * @return The centre of this Coverflow.
	 */
	private int getCenterOfCoverflow() {
		return (getWidth() - getPaddingLeft() - getPaddingRight()) / 2
		                + getPaddingLeft();
	}

	/**
	 * Get the Centre of the View.
	 *
	 * @return The centre of the given view.
	 */
	private static int getCenterOfView(final View view) {
		return view.getLeft() + view.getWidth() / 2;
	}

	/**
	 * {@inheritDoc}
	 *
	 * @see #setStaticTransformationsEnabled(boolean)
	 */
	@Override
	protected boolean getChildStaticTransformation(final View child,
	                final Transformation t) {

		final int childCenter = getCenterOfView(child);
		final int childWidth = child.getWidth();
		int rotationAngle = 0;

		t.clear();
		t.setTransformationType(Transformation.TYPE_MATRIX);

		if (childCenter == mCoveflowCenter) {
			transformImageBitmap((ImageView) child, t, 0);
		} else {
			rotationAngle = (int) ((float) (mCoveflowCenter - childCenter)
			                / childWidth * mMaxRotationAngle);
			if (Math.abs(rotationAngle) > mMaxRotationAngle) {
				rotationAngle = rotationAngle < 0 ? -mMaxRotationAngle
				                : mMaxRotationAngle;
			}
			transformImageBitmap((ImageView) child, t, rotationAngle);
		}

		return true;
	}

	/**
	 * This is called during layout when the size of this view has changed. If
	 * you were just added to the view hierarchy, you're called with the old
	 * values of 0.
	 *
	 * @param w
	 *            Current width of this view.
	 * @param h
	 *            Current height of this view.
	 * @param oldw
	 *            Old width of this view.
	 * @param oldh
	 *            Old height of this view.
	 */
	@Override
	protected void onSizeChanged(final int w, final int h, final int oldw,
	                final int oldh) {
		mCoveflowCenter = getCenterOfCoverflow();
		super.onSizeChanged(w, h, oldw, oldh);
	}

	/**
	 * Transform the Image Bitmap by the Angle passed.
	 *
	 * @param imageView
	 *            ImageView the ImageView whose bitmap we want to rotate
	 * @param t
	 *            transformation
	 * @param rotationAngle
	 *            the Angle by which to rotate the Bitmap
	 */
	private void transformImageBitmap(final ImageView child,
	                final Transformation t, final int rotationAngle) {
		mCamera.save();
		final Matrix imageMatrix = t.getMatrix();

		final int height = child.getLayoutParams().height;

		final int width = child.getLayoutParams().width;
		final int rotation = Math.abs(rotationAngle);

		mCamera.translate(0.0f, 0.0f, 100.0f);

		// As the angle of the view gets less, zoom in
		if (rotation < mMaxRotationAngle) {
			final float zoomAmount = (float) (mMaxZoom + rotation * 1.5);
			mCamera.translate(0.0f, 0.0f, zoomAmount);
		}

		mCamera.rotateY(rotationAngle);
		mCamera.getMatrix(imageMatrix);
		imageMatrix.preTranslate(-(width / 2.0f), -(height / 2.0f));
		imageMatrix.postTranslate((width / 2.0f), (height / 2.0f));
		mCamera.restore();
	}

	/**
	 * Parses the attributes.
	 *
	 * @param context
	 *            the context
	 * @param attrs
	 *            the attrs
	 */
	private void parseAttributes(final Context context, final AttributeSet attrs) {
		final TypedArray a = context.obtainStyledAttributes(attrs,
		                R.styleable.CoverFlow);
		try {
			imageWidth = a.getDimension(R.styleable.CoverFlow_imageWidth, 480);
			imageHeight = a.getDimension(R.styleable.CoverFlow_imageHeight, 320);
			withReflection = a.getBoolean(R.styleable.CoverFlow_withReflection,
			                false);
			imageReflectionRatio = a.getFloat(
			                R.styleable.CoverFlow_imageReflectionRatio, 0.2f);
			reflectionGap = a.getDimension(R.styleable.CoverFlow_reflectionGap,
			                4);
			setSpacing(-15);
		} finally {
			a.recycle();
		}
	}

}