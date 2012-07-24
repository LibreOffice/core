package org.libreoffice.impressremote;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.webkit.WebView;
import android.widget.ImageView;

public class PresentationFragment extends Fragment {

	private View mTopView;
	private ImageView mHandle;
	private View mLayout;

	private Context mContext;

	public View onCreateView(LayoutInflater inflater, ViewGroup container,
	                Bundle savedInstanceState) {
		mContext = container.getContext();

		View v = inflater.inflate(R.layout.fragment_presentation, container,
		                false);

		WebView mWebView = (WebView) v.findViewById(R.id.textView1);

		String summary = "<html><body>This is just a test<br/><ul><li>And item</li><li>And again</li></ul>More text<br/>Blabla<br/>Blabla<br/>blabla<br/>Blabla</body></html>";
		mWebView.loadData(summary, "text/html", null);

		mTopView = v.findViewById(R.id.imageView1);
		mLayout = v.findViewById(R.id.presentation_layout);

		mHandle = (ImageView) v.findViewById(R.id.presentation_handle);
		mHandle.setOnTouchListener(new SizeListener());

		return v;
	}

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
}
