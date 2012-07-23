package org.libreoffice.impressremote;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.webkit.WebView;

public class PresentationFragment extends Fragment {

	private View mTopView;

	public View onCreateView(LayoutInflater inflater, ViewGroup container,
	                Bundle savedInstanceState) {
		// Inflate the layout for this fragment
		View v = inflater.inflate(R.layout.fragment_presentation, container,
		                false);

		WebView mWebView = (WebView) v.findViewById(R.id.textView1);

		String summary = "<html><body>This is just a test<br/><ul><li>And item</li><li>And again</li></ul>More text<br/>Blabla<br/>Blabla<br/>blabla<br/>Blabla</body></html>";
		mWebView.loadData(summary, "text/html", null);

		mTopView = v.findViewById(R.id.imageView1);

		v.findViewById(R.id.presentation_handle).setOnTouchListener(
		                new SizeListener());

		return v;
	}

	private class SizeListener implements OnTouchListener {

		@Override
		public boolean onTouch(View aView, MotionEvent aEvent) {

			LayoutParams aParams = mTopView.getLayoutParams();

			switch (aEvent.getAction()) {
			case MotionEvent.ACTION_MOVE:
				int aHeight = mTopView.getHeight();
				aParams.height = aHeight + (int) (aEvent.getY());
				mTopView.setLayoutParams(aParams);
				break;
			}
			// TODO Auto-generated method stub
			return true;
		}
	}
}
