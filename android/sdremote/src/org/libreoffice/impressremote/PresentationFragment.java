package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow;

import pl.polidea.coverflow.AbstractCoverFlowImageAdapter;
import pl.polidea.coverflow.CoverFlow;
import android.app.Fragment;
import android.content.Context;
import android.graphics.Bitmap;
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
import android.widget.ImageView;

public class PresentationFragment extends Fragment {

	private CoverFlow mTopView;
	private ImageView mHandle;
	private View mLayout;
	private WebView mNotes;
	private Context mContext;

	private CommunicationService mCommunicationService;
	private SlideShow mSlideShow;

	public View onCreateView(LayoutInflater inflater, ViewGroup container,
	                Bundle savedInstanceState) {
		mContext = container.getContext();

		View v = inflater.inflate(R.layout.fragment_presentation, container,
		                false);

		mNotes = (WebView) v.findViewById(R.id.presentation_notes);

		String summary = "<html><body>This is just a test<br/><ul><li>And item</li><li>And again</li></ul>More text<br/>Blabla<br/>Blabla<br/>blabla<br/>Blabla</body></html>";
		mNotes.loadData(summary, "text/html", null);

		mTopView = (CoverFlow) v.findViewById(R.id.presentation_coverflow);
		mLayout = v.findViewById(R.id.presentation_layout);

		mHandle = (ImageView) v.findViewById(R.id.presentation_handle);
		mHandle.setOnTouchListener(new SizeListener());

		if (mCommunicationService != null && mSlideShow != null) {
			mTopView.setAdapter(new ThumbnailAdapter(mContext, mSlideShow));
		}

		return v;
	}

	// -------------------------------------------------- RESIZING LISTENER ----
	private class SizeListener implements OnTouchListener {

		@Override
		public boolean onTouch(View aView, MotionEvent aEvent) {

			LayoutParams aParams = mTopView.getLayoutParams();

			switch (aEvent.getAction()) {
			case MotionEvent.ACTION_DOWN:
				mHandle.setImageResource(R.drawable.handle_light);
				break;
			case MotionEvent.ACTION_UP:
				mHandle.setImageResource(R.drawable.handle_default);
				break;
			case MotionEvent.ACTION_MOVE:
				int aHeight = mTopView.getHeight();
				aParams.height = aHeight + (int) (aEvent.getY());

				final int DRAG_MARGIN = 120;

				int aSize = mLayout.getHeight();
				if (aParams.height < DRAG_MARGIN) {
					aParams.height = DRAG_MARGIN;
				} else if (aParams.height > aSize - DRAG_MARGIN) {
					aParams.height = aSize - DRAG_MARGIN;
				}

				mTopView.setLayoutParams(aParams);
				break;
			}
			// TODO Auto-generated method stub
			return true;
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
		if (mTopView != null) {
			mTopView.setAdapter(new ThumbnailAdapter(mContext, mSlideShow));
		}
	}

	public void handleMessage(Message aMessage) {
		Bundle aData = aMessage.getData();
		switch (aMessage.what) {
		case CommunicationService.MSG_SLIDE_CHANGED:
			int aSlide = aData.getInt("slide_number");
			mTopView.setSelection(aSlide, true);
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

		// @Override
		// public View getView(int position, View convertView, ViewGroup parent)
		// {
		// LayoutInflater aInflater = (LayoutInflater) mContext
		// .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		// View v = aInflater.inflate(R.layout.slide_thumbnail, null);
		//
		// ImageView aImage = (ImageView) v.findViewById(R.id.sub_thumbnail);
		// TextView aText = (TextView) v.findViewById(R.id.sub_number);
		//
		// // Do the image & number styling
		// int aBorderWidth = getResources().getInteger(
		// R.integer.thumbnail_border_width);
		// aImage.setPadding(aBorderWidth, aBorderWidth, aBorderWidth,
		// aBorderWidth);
		//
		// Bitmap aBitmap = mSlideShow.getImage(position);
		// if (aBitmap != null) {
		// aImage.setImageBitmap(aBitmap);
		// }
		//
		// aText.setText(String.valueOf(position + 1));
		//
		// return v;
		// }

		@Override
		protected Bitmap createBitmap(int position) {
			Bitmap aBitmap = mSlideShow.getImage(position);
			return aBitmap;
		}
	}
}
