/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
import android.graphics.Color;
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

    private long lastUpdateTime = 0;

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
                                .getCurrentSlideIndex(), true);
                mTopView.setOnItemSelectedListener(new ClickListener());
            }

            updateSlideNumberDisplay(mCommunicationService.getSlideShow()
                            .getCurrentSlideIndex());

        }

        @Override
        public void onServiceDisconnected(ComponentName aClassName) {
            mCommunicationService = null;
        }
    };

    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                    Bundle savedInstanceState) {
        setRetainInstance(true);
        getActivity().bindService(
                        new Intent(getActivity().getApplicationContext(),
                                        CommunicationService.class),
                        mConnection, Context.BIND_IMPORTANT);
        mContext = getActivity().getApplicationContext();
        //        container.removeAllViews();
        View v = inflater.inflate(R.layout.fragment_presentation, container,
                        false);

        mNotes = (WebView) v.findViewById(R.id.presentation_notes);

        String summary = "<html><body>This is just a test<br/><ul><li>And item</li><li>And again</li></ul>More text<br/>Blabla<br/>Blabla<br/>blabla<br/>Blabla</body></html>";
        mNotes.loadDataWithBaseURL(null, summary, "text/html", "UTF-8", null);
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
            if ( aAdapter != null ) {
                aAdapter.setHeight(mNewCoverflowHeight);
                mTopView.setImageHeight(mNewCoverflowHeight);
                aAdapter.setWidth(mNewCoverflowWidth);
                mTopView.setImageWidth(mNewCoverflowWidth);

                // We need to update the view now
                aAdapter.notifyDataSetChanged();
            }
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
        mTopView = null;
        mContext = null;
    }

    private void updateSlideNumberDisplay(int aPosition) {
        //        int aSlide = mCommunicationService.getSlideShow().getCurrentSlide();
        mNumberText.setText((aPosition + 1) + "/"
                        + mCommunicationService.getSlideShow().getSlidesCount());
        mNotes.loadDataWithBaseURL(null, mCommunicationService.getSlideShow()
                        .getSlideNotes(aPosition), "text/html", "UTF-8", null);
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
                mCommunicationService.getTransmitter().setCurrentSlide(
                    aPosition);
            lastUpdateTime = System.currentTimeMillis();
            updateSlideNumberDisplay(aPosition);
        }

        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
        }
    }

    // ---------------------------------------------------- MESSAGE HANDLER ----
    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {

            if (mTopView == null || mTopView.getAdapter() == null)
                return;
            if (aIntent.getAction().equals(
                            CommunicationService.MSG_SLIDE_CHANGED)) {
                int aSlide = aIntent.getExtras().getInt("slide_number");

                if (aSlide == mTopView.getSelectedItemPosition())
                    return;
                if ((System.currentTimeMillis() - lastUpdateTime) < 500) {
                    return;
                }
                mTopView.setSelection(aSlide, true);
            } else if (aIntent.getAction().equals(
                            CommunicationService.MSG_SLIDE_PREVIEW)) {
                ThumbnailAdapter aThumbAdaptor = (ThumbnailAdapter) mTopView.getAdapter();
                aThumbAdaptor.notifyDataSetChanged();
            } else if (aIntent.getAction().equals(
                            CommunicationService.MSG_SLIDE_NOTES)) {
                int aPosition = aIntent.getExtras().getInt("slide_number");
                if ( aPosition == mTopView.getSelectedItemPosition() ) {
                    mNotes.loadDataWithBaseURL(null, mCommunicationService.getSlideShow()
                                    .getSlideNotes(aPosition), "text/html", "UTF-8", null);
                }
            }

        }
    };

    // ------------------------------------------------- THUMBNAIL ADAPTER ----
    protected class ThumbnailAdapter extends AbstractCoverFlowImageAdapter {

        private SlideShow mSlideShow;

        public ThumbnailAdapter(Context aContext, SlideShow aSlideShow) {
            mContext = aContext;
            mSlideShow = aSlideShow;
        }

        @Override
        public int getCount() {
            return mSlideShow.getSlidesCount();
        }

        @Override
        protected Bitmap createBitmap(int position) {
            return mSlideShow.getSlidePreview(position);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
