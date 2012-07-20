package org.libreoffice.impressremote.communication;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.SparseArray;

public class SlideShow {

	private SparseArray<byte[]> mPreviewImages = new SparseArray<byte[]>();
	private int mSize = 0;
	private int mCurrentSlide = 0;

	protected SlideShow(int aSize) {
		mSize = aSize;
	}

	public int getSize() {
		return mSize;
	}

	public int getCurrentSlide() {
		return mCurrentSlide;
	}

	protected void setCurrentSlide(int aSlide) {
		mCurrentSlide = aSlide;
	}

	protected void putImage(int aSlide, byte[] aImage) {
		mPreviewImages.put(aSlide, aImage);
	}

	public Bitmap getImage(int aSlide) {
		byte[] aImage = mPreviewImages.get(aSlide);
		return BitmapFactory.decodeByteArray(aImage, 0, aImage.length);
	}
}
