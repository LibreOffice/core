/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow;

import android.app.Fragment;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.TextView;

public class ThumbnailFragment extends Fragment {

	private CommunicationService mCommunicationService;

	private GridView mGrid;
	private ImageView mCurrentImage;
	private TextView mCurrentText;

	private SlideShow mSlideShow;
	private Context mContext;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
	                Bundle savedInstanceState) {
		// Inflate the layout for this fragment
		View v = inflater
		                .inflate(R.layout.fragment_thumbnail, container, false);

		mGrid = (GridView) v.findViewById(R.id.thumbnail_grid);

		mGrid.setOnItemClickListener(new ClickListener());
		mContext = container.getContext();

		if (mCommunicationService != null && mSlideShow != null) {
			mGrid.setAdapter(new ThumbnailAdapter(mContext, mSlideShow));
		}

		return v;
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

	}

	@Override
	public void onPause() {
		super.onPause();
	}

	private void setSelected(int position) {
		formatUnselected(mCurrentImage, mCurrentText);

		if (mGrid == null) {
			return;
		}

		View aV = mGrid.getChildAt(position);
		if (aV != null) {
			mCurrentImage = (ImageView) aV.findViewById(R.id.sub_thumbnail);
			mCurrentText = (TextView) aV.findViewById(R.id.sub_number);

			formatSelected(mCurrentImage, mCurrentText);
		}
	}

	private void formatUnselected(ImageView aImage, TextView aText) {
		if (aImage != null) {
			aImage.setBackgroundColor(getResources().getColor(
			                R.color.thumbnail_border));
		}
		if (aText != null) {
			aText.setTypeface(Typeface.create(aText.getTypeface(),
			                Typeface.NORMAL));
		}
	}

	private void formatSelected(ImageView aImage, TextView aText) {
		if (aImage != null) {
			aImage.setBackgroundColor(getResources().getColor(
			                R.color.thumbnail_border_selected));
		}
		if (aText != null) {
			aText.setTypeface(Typeface.create(aText.getTypeface(),
			                Typeface.BOLD));
		}
	}

	// ----------------------------------------------------- CLICK LISTENER ----
	protected class ClickListener implements AdapterView.OnItemClickListener {
		public void onItemClick(AdapterView<?> parent, View v, int position,
		                long id) {
			if (mCommunicationService != null)
				mCommunicationService.getTransmitter().gotoSlide(position);
		}
	}

	// ---------------------------------------------------- MESSAGE HANDLER ----

	public void setCommunicationService(
	                CommunicationService aCommunicationService) {
		mCommunicationService = aCommunicationService;
		mSlideShow = mCommunicationService.getSlideShow();
		if (mGrid != null) {
			mGrid.setAdapter(new ThumbnailAdapter(mContext, mSlideShow));
		}
	}

	public void handleMessage(Message aMessage) {
		Bundle aData = aMessage.getData();
		switch (aMessage.what) {
		case CommunicationService.MSG_SLIDE_CHANGED:
			int aSlide = aData.getInt("slide_number");
			setSelected(aSlide);
			break;
		case CommunicationService.MSG_SLIDE_PREVIEW:
			mGrid.invalidateViews();
			break;

		}
	}

	// ------------------------------------------------- THUMBNAIL ADAPTER ----
	protected class ThumbnailAdapter extends BaseAdapter {

		private Context mContext;

		private SlideShow mSlideShow;

		public ThumbnailAdapter(Context aContext, SlideShow aSlideShow) {
			mContext = aContext;
			mSlideShow = aSlideShow;
		}

		@Override
		public int getCount() {
			return mSlideShow.getSize();
		}

		@Override
		public Object getItem(int arg0) {
			return null;
		}

		@Override
		public long getItemId(int position) {
			return 0;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			LayoutInflater aInflater = (LayoutInflater) mContext
			                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			View v = aInflater.inflate(R.layout.slide_thumbnail, null);

			ImageView aImage = (ImageView) v.findViewById(R.id.sub_thumbnail);
			TextView aText = (TextView) v.findViewById(R.id.sub_number);

			// Do the image & number styling
			int aBorderWidth = getResources().getInteger(
			                R.integer.thumbnail_border_width);
			aImage.setPadding(aBorderWidth, aBorderWidth, aBorderWidth,
			                aBorderWidth);

			if ((mSlideShow != null)
			                && (position == mSlideShow.getCurrentSlide())) {
				formatSelected(aImage, aText);
				mCurrentImage = aImage;
				mCurrentText = aText;
			} else {
				formatUnselected(aImage, aText);
			}

			Bitmap aBitmap = mSlideShow.getImage(position);
			if (aBitmap != null) {
				aImage.setImageBitmap(aBitmap);
			}

			// Width
			int aWidth = (mGrid.getWidth()) / 3 - 20;
			aImage.setMaxWidth(aWidth);
			aImage.setScaleType(ScaleType.MATRIX);

			aText.setText(String.valueOf(position + 1));

			return v;
		}
	}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */