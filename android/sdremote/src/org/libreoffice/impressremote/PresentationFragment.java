package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.SlideShow;

import pl.polidea.coverflow.AbstractCoverFlowImageAdapter;
import pl.polidea.coverflow.CoverFlow;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ImageView;
import android.widget.TextView;

import com.actionbarsherlock.app.SherlockFragment;

public class PresentationFragment extends SherlockFragment {
    private CoverFlow mTopView;
    private ImageView mHandle;
    private View mLayout;
    private WebView mNotes;
    private Context mContext;
    private TextView mNumberText;

    private CommunicationService mCommunicationService;

    private float mOriginalCoverflowWidth;
    private float mOriginalCoverflowHeight;

    private float mNewCoverflowWidth = 0;
    private float mNewCoverflowHeight = 0;

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName aClassName,
                        IBinder aService) {
            mCommunicationService = ((CommunicationService.CBinder) aService)
                            .getService();

            if (mTopView != null) {
                mTopView.setAdapter(new ThumbnailAdapter(mContext,
                                mCommunicationService.getSlideShow()));
                mTopView.setSelection(mCommunicationService.getSlideShow()
                                .getCurrentSlide(), true);
                mTopView.setOnItemSelectedListener(new ClickListener());
            }

            updateSlideNumberDisplay();

        }

        @Override
        public void onServiceDisconnected(ComponentName aClassName) {
            mCommunicationService = null;
        }
    };

    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                    Bundle savedInstanceState) {
        getActivity().bindService(
                        new Intent(getActivity().getApplicationContext(),
                                        CommunicationService.class),
                        mConnection, Context.BIND_IMPORTANT);
        mContext = getActivity().getApplicationContext();
        container.removeAllViews();
        View v = inflater.inflate(R.layout.fragment_presentation, container,
                        false);

        mNotes = (WebView) v.findViewById(R.id.presentation_notes);

        String summary = "<html><body>This is just a test<br/><ul><li>And item</li><li>And again</li></ul>More text<br/>Blabla<br/>Blabla<br/>blabla<br/>Blabla</body></html>";
        mNotes.loadData(summary, "text/html", null);
        mNotes.setBackgroundColor(Color.TRANSPARENT);

        mTopView = (CoverFlow) v.findViewById(R.id.presentation_coverflow);

        mLayout = v.findViewById(R.id.presentation_layout);

        mNumberText = (TextView) v.findViewById(R.id.presentation_slidenumber);

        mHandle = (ImageView) v.findViewById(R.id.presentation_handle);
        mHandle.setOnTouchListener(new SizeListener());

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

        IntentFilter aFilter = new IntentFilter(
                        CommunicationService.MSG_SLIDE_CHANGED);
        aFilter.addAction(CommunicationService.MSG_SLIDE_NOTES);
        aFilter.addAction(CommunicationService.MSG_SLIDE_PREVIEW);
        LocalBroadcastManager
                        .getInstance(getActivity().getApplicationContext())
                        .registerReceiver(mListener, aFilter);

        return v;
    }

    @Override
    public void onDestroyView() {
        getActivity().unbindService(mConnection);
        super.onDestroyView();
        LocalBroadcastManager
                        .getInstance(getActivity().getApplicationContext())
                        .unregisterReceiver(mListener);

    }

    private void updateSlideNumberDisplay() {
        int aSlide = mCommunicationService.getSlideShow().getCurrentSlide();
        mNumberText.setText((aSlide + 1) + "/"
                        + mCommunicationService.getSlideShow().getSize());
        mNotes.loadData(mCommunicationService.getSlideShow().getNotes(aSlide),
                        "text/html", null);
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

                final int DRAG_MARGIN = 120;

                boolean aPortrait = (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT);

                int aFlowSize = aPortrait ? mTopView.getHeight() : mTopView
                                .getWidth();
                int aViewSize = aPortrait ? mLayout.getHeight() : mLayout
                                .getWidth();

                // Calculate height change, taking limits into account
                int aDiff = (int) (aPortrait ? aEvent.getY() : aEvent.getX());
                if (aDiff + aFlowSize < DRAG_MARGIN) {
                    aDiff = DRAG_MARGIN - aFlowSize;
                } else if ((aFlowSize + aDiff) > (aViewSize - DRAG_MARGIN)) {
                    aDiff = (aViewSize - DRAG_MARGIN) - aFlowSize;
                }

                // Now deal with the internal height
                AbstractCoverFlowImageAdapter aAdapter = (AbstractCoverFlowImageAdapter) mTopView
                                .getAdapter();

                double aRatio = mOriginalCoverflowWidth
                                / mOriginalCoverflowHeight;
                float aHeightNew;
                float aWidthNew;
                if (aPortrait) {
                    aHeightNew = mTopView.getImageHeight() + aDiff;
                    aWidthNew = (float) (aRatio * aHeightNew);
                    //               Too wide -- so scale down
                    if (aWidthNew > mLayout.getWidth() - 50) {
                        aWidthNew = mLayout.getWidth() - 50;
                        aHeightNew = (float) (aWidthNew / aRatio);
                        aDiff = (int) (aHeightNew - mTopView.getImageHeight());
                    }
                } else {
                    aWidthNew = mTopView.getImageWidth() + aDiff;
                    aHeightNew = (float) (aWidthNew / aRatio);
                    //              Too High -- so scale down
                    if (aHeightNew > mLayout.getHeight() - 50) {
                        aHeightNew = mLayout.getHeight() - 50;
                        aWidthNew = (float) (aHeightNew * aRatio);
                        aDiff = (int) (aWidthNew - mTopView.getImageWidth());
                    }
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
    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (aIntent.getAction().equals(
                            CommunicationService.MSG_SLIDE_CHANGED)) {
                int aSlide = aIntent.getExtras().getInt("slide_number");
                mTopView.setSelection(aSlide, true);
                updateSlideNumberDisplay();
            } else if (aIntent.getAction().equals(
                            CommunicationService.MSG_SLIDE_PREVIEW)) {
                // int aNSlide = aIntent.getExtras().getInt("slide_number");
                ((ThumbnailAdapter) mTopView.getAdapter())
                                .notifyDataSetChanged();
                //                mTopView.requestLayout();
            } else if (aIntent.getAction().equals(
                            CommunicationService.MSG_SLIDE_NOTES)) {
                // TODO: update me
            }

        }
    };

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
