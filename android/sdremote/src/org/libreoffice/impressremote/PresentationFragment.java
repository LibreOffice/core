package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow;

import pl.polidea.coverflow.AbstractCoverFlowImageAdapter;
import pl.polidea.coverflow.CoverFlow;
import android.app.Fragment;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.os.Bundle;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.webkit.WebView;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ImageView;
import android.widget.TextView;

public class PresentationFragment extends Fragment {

	private CoverFlow mTopView;
	private ImageView mHandle;
	private View mLayout;
	private WebView mNotes;
	private Context mContext;
	private TextView mNumberText;

	private CommunicationService mCommunicationService;
	private SlideShow mSlideShow;

	private float mOriginalCoverflowWidth;
	private float mOriginalCoverflowHeight;

	private float mNewCoverflowWidth = 0;
	private float mNewCoverflowHeight = 0;

	public View onCreateView(LayoutInflater inflater, ViewGroup container,
	                Bundle savedInstanceState) {
		mContext = container.getContext();

		View v = inflater.inflate(R.layout.fragment_presentation, container,
		                false);

		mNotes = (WebView) v.findViewById(R.id.presentation_notes);

		String summary = "<html><body>This is just a test<br/><ul><li>And item</li><li>And again</li></ul>More text<br/>Blabla<br/>Blabla<br/>blabla<br/>Blabla</body></html>";
		mNotes.loadData(summary, "text/html", null);
		mNotes.setBackgroundColor(Color.TRANSPARENT);

		//		TextView aText = new TextView();
		//		aText.setText

		mTopView = (CoverFlow) v.findViewById(R.id.presentation_coverflow);

		mLayout = v.findViewById(R.id.presentation_layout);

		mNumberText = (TextView) v.findViewById(R.id.presentation_slidenumber);

		mHandle = (ImageView) v.findViewById(R.id.presentation_handle);
		mHandle.setOnTouchListener(new SizeListener());

		// Call again to set things up if necessary.
		setCommunicationService(mCommunicationService);

		// Save the height/width for future reference
		mOriginalCoverflowHeight = mTopView.getImageHeight();
		mOriginalCoverflowWidth = mTopView.getImageWidth();

		if (mNewCoverflowHeight != 0) {
			ThumbnailAdapter aAdapter = (ThumbnailAdapter) mTopView
			                .getAdapter();
			aAdapter.setHeight(mNewCoverflowHeight);
			mTopView.setImageHeight(mNewCoverflowHeight);
			aAdapter.setWidth(mNewCoverflowWidth);
			mTopView.setImageWidth(mNewCoverflowWidth);

			// We need to update the view now
			aAdapter.notifyDataSetChanged();
		}
		return v;
	}

	private void updateSlideNumberDisplay() {
		mNumberText.setText((mSlideShow.getCurrentSlide() + 1) + "/"
		                + mSlideShow.getSize());
	}

	// -------------------------------------------------- RESIZING LISTENER ----
	private class SizeListener implements OnTouchListener {

		@Override
		public boolean onTouch(View aView, MotionEvent aEvent) {

			switch (aEvent.getAction()) {
			case MotionEvent.ACTION_DOWN:
				mHandle.setImageResource(R.drawable.handle_light);
				break;
			case MotionEvent.ACTION_UP:
				mHandle.setImageResource(R.drawable.handle_default);
				break;
			case MotionEvent.ACTION_MOVE:
				LayoutParams aParams = mTopView.getLayoutParams();
				int aHeight = mTopView.getHeight();
				int aViewSize = mLayout.getHeight();

				final int DRAG_MARGIN = 120;

				// Calculate height change, taking limits into account
				int aDiff = (int) (aEvent.getY());
				if (aDiff + aHeight < DRAG_MARGIN) {
					aDiff = DRAG_MARGIN - aHeight;
				} else if ((aHeight + aDiff) > (aViewSize - DRAG_MARGIN)) {
					aDiff = (aViewSize - DRAG_MARGIN) - aHeight;
				}

				// Now deal with the internal height
				AbstractCoverFlowImageAdapter aAdapter = (AbstractCoverFlowImageAdapter) mTopView
				                .getAdapter();

				double aRatio = mOriginalCoverflowWidth
				                / mOriginalCoverflowHeight;
				float aHeightNew = mTopView.getImageHeight() + aDiff;
				float aWidthNew = (float) (aRatio * aHeightNew);

				//				 Too wide -- so scale down
				if (aWidthNew > mLayout.getWidth() - 50) {
					aWidthNew = mLayout.getWidth() - 50;
					aHeightNew = (float) (aWidthNew / aRatio);
					aDiff = (int) (aHeightNew - mTopView.getImageHeight());
				}

				mNewCoverflowHeight = aHeightNew;
				mNewCoverflowWidth = aWidthNew;

				aAdapter.setHeight(aHeightNew);
				mTopView.setImageHeight(aHeightNew);
				aAdapter.setWidth(aWidthNew);
				mTopView.setImageWidth(aWidthNew);

				// Force an update of the view
				aAdapter.notifyDataSetChanged();

				break;
			}
			// TODO Auto-generated method stub
			return true;
		}
	}

	// ----------------------------------------------------- CLICK LISTENER ----

	protected class ClickListener implements OnItemSelectedListener {

		@Override
		public void onItemSelected(AdapterView<?> arg0, View arg1,
		                int aPosition, long arg3) {
			if (mCommunicationService != null)
				mCommunicationService.getTransmitter().gotoSlide(aPosition);
		}

		@Override
		public void onNothingSelected(AdapterView<?> arg0) {
		}
	}

	// ---------------------------------------------------- MESSAGE HANDLER ----
	public void setCommunicationService(
	                CommunicationService aCommunicationService) {
		mCommunicationService = aCommunicationService;
		if (mCommunicationService == null)
			return;

		mSlideShow = mCommunicationService.getSlideShow();
		if (mTopView != null && mSlideShow != null) {
			mTopView.setAdapter(new ThumbnailAdapter(mContext, mSlideShow));
			mTopView.setSelection(mSlideShow.getCurrentSlide(), true);
			mTopView.setOnItemSelectedListener(new ClickListener());
		}

		updateSlideNumberDisplay();
	}

	public void handleMessage(Message aMessage) {
		Bundle aData = aMessage.getData();
		switch (aMessage.what) {
		case CommunicationService.MSG_SLIDE_CHANGED:
			int aSlide = aData.getInt("slide_number");
			mTopView.setSelection(aSlide, true);
			updateSlideNumberDisplay();
			break;
		case CommunicationService.MSG_SLIDE_PREVIEW:
			int aNSlide = aData.getInt("slide_number");
			if (mTopView.getSelectedItemPosition() == aNSlide) {
				// mTopView. // TODO: update the current item
			}
			break;

		}
	}

	// ------------------------------------------------- THUMBNAIL ADAPTER ----
	protected class ThumbnailAdapter extends AbstractCoverFlowImageAdapter {

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
		protected Bitmap createBitmap(int position) {
			Bitmap aBitmap = mSlideShow.getImage(position);
			final int borderWidth = 8;

			Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
			p.setShadowLayer(borderWidth, 0, 0, Color.BLACK);

			RectF aRect = new RectF(borderWidth, borderWidth, borderWidth
			                + aBitmap.getWidth(), borderWidth
			                + aBitmap.getHeight());
			Bitmap aOut = Bitmap.createBitmap(aBitmap.getWidth() + 2
			                * borderWidth, aBitmap.getHeight() + 2
			                * borderWidth, aBitmap.getConfig());
			Canvas canvas = new Canvas(aOut);
			canvas.drawColor(Color.TRANSPARENT);
			canvas.drawRect(aRect, p);
			canvas.drawBitmap(aBitmap, null, aRect, null);

			return aOut;
		}
	}
}
