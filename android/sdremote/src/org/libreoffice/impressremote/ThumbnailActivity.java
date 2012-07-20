package org.libreoffice.impressremote;

import org.libreoffice.impressremote.TestClient.MessageHandler;
import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;

public class ThumbnailActivity extends Activity {

	private CommunicationService mCommunicationService;
	private boolean mIsBound = false;

	private GridView mGrid;
	private ImageView mCurrentImage;
	private TextView mCurrentText;

	private SlideShow mSlideShow;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_thumbnail);

		bindService(new Intent(this, CommunicationService.class), mConnection,
				Context.BIND_ADJUST_WITH_ACTIVITY);
		mIsBound = true;

		mGrid = (GridView) findViewById(R.id.thumbnail_grid);

		mGrid.setOnItemClickListener(new ClickListener());
	}

	@Override
	protected void onPause() {
		super.onPause();
		mCommunicationService.setActivityMessenger(null);
		if (mIsBound) {
			unbindService(mConnection);
			mIsBound = false;
		}
	}

	private void setSelected(int position) {
		if (mCurrentImage != null) {
			mCurrentImage.setPadding(0, 0, 0, 0);
		}
		if (mCurrentText != null) {
			mCurrentText.setTypeface(Typeface.create(
					mCurrentText.getTypeface(), Typeface.NORMAL));
		}

		View aV = mGrid.getChildAt(position);
		if (aV != null) {
			mCurrentImage = (ImageView) aV.findViewById(R.id.sub_thumbnail);
			mCurrentText = (TextView) aV.findViewById(R.id.sub_number);

			mCurrentImage.setBackgroundColor(Color.RED);
			mCurrentImage.setPadding(2, 2, 2, 2);
			mCurrentText.setTypeface(Typeface.create(mCurrentText.getTypeface(),
					Typeface.BOLD));
		}
	}

	// ------------------------------------------------- SERVICE CONNECTION ----
	final Messenger mMessenger = new Messenger(new MessageHandler());

	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName aClassName,
				IBinder aService) {
			mCommunicationService = ((CommunicationService.CBinder) aService)
					.getService();
			mCommunicationService.setActivityMessenger(mMessenger);
			mSlideShow = mCommunicationService.getSlideShow();
			mGrid.setAdapter(new ThumbnailAdapter(ThumbnailActivity.this,
					mSlideShow));
		}

		@Override
		public void onServiceDisconnected(ComponentName aClassName) {
			mCommunicationService = null;
		}
	};

	// ----------------------------------------------------- CLICK LISTENER ----
	protected class ClickListener implements AdapterView.OnItemClickListener {
		public void onItemClick(AdapterView<?> parent, View v, int position,
				long id) {
			mCommunicationService.getTransmitter().gotoSlide(position);
		}
	}

	// ---------------------------------------------------- MESSAGE HANDLER ----
	protected class MessageHandler extends Handler {
		@Override
		public void handleMessage(Message aMessage) {
			Bundle aData = aMessage.getData();
			switch (aMessage.what) {
			case CommunicationService.MSG_SLIDE_CHANGED:
				int aSlide = aData.getInt("slide_number");
				setSelected(aSlide);
				break;
			case CommunicationService.MSG_SLIDE_PREVIEW:
				// int aNSlide = aData.getInt("slide_number");
				mGrid.invalidateViews();
				break;

			}
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

			Bitmap aBitmap = mSlideShow.getImage(position);
			if (aBitmap != null) {
				aImage.setImageBitmap(aBitmap);
			}

			aText.setText(String.valueOf(position + 1));

//			if ((mSlideShow != null)
//					&& (position == mSlideShow.getCurrentSlide())) {
//				setSelected(position);
//			}

			return v;
		}

	}
}
