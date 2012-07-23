/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
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

	// ---------------------------------------------------- TIMER --------------
	private Timer mTimer = new Timer();

	public Timer getTimer() {
		return mTimer;
	}

	public class Timer {
		/**
		 * This stores the starting time of the timer if running.
		 *
		 * If paused this stores how long the timer was previously running.
		 */
		private long aTime = 0;

		private boolean mIsRunning = false;

		public boolean isRunning() {
			return mIsRunning;
		}

		/**
		 * Reset the timer, and stop it it was running.
		 */
		public void reset() {
			mIsRunning = false;
			aTime = 0;
		}

		public void startTimer() {
			if (mIsRunning)
				return;

			aTime = System.currentTimeMillis() - aTime;
		}

		public void stopTimer() {
			if (!mIsRunning)
				return;

			aTime = System.currentTimeMillis() - aTime;
		}

		public long getTimeMillis() {
			if (mIsRunning)
				return (System.currentTimeMillis() - aTime);
			else
				return aTime;
		}

	}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */