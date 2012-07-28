package org.libreoffice.impressremote;

import android.app.Fragment;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

public class BlankScreenFragment extends Fragment {

	Bitmap mBitmap;

	public BlankScreenFragment(Bitmap aBitmap) {
		mBitmap = aBitmap;
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
	                Bundle savedInstanceState) {

		View v = inflater.inflate(R.layout.fragment_blankscreen, container,
		                false);

		// Process the image
		final int borderWidth = 8;

		Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
		p.setShadowLayer(borderWidth, 0, 0, Color.BLACK);

		RectF aRect = new RectF(borderWidth, borderWidth, borderWidth
		                + mBitmap.getWidth(), borderWidth + mBitmap.getHeight());
		Bitmap aOut = Bitmap.createBitmap(mBitmap.getWidth() + 2 * borderWidth,
		                mBitmap.getHeight() + 2 * borderWidth,
		                mBitmap.getConfig());
		Canvas canvas = new Canvas(aOut);
		canvas.drawColor(Color.TRANSPARENT);
		canvas.drawRect(aRect, p);
		canvas.drawBitmap(mBitmap, null, aRect, null);

		ImageView aImage = (ImageView) v
		                .findViewById(R.id.blankscreen_slidepreview);
		aImage.setImageBitmap(aOut);
		// TODO Auto-generated method stub
		return v;
	}
}
