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

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class TestClient extends Activity {

    private boolean mCurrentPreviewImageMissing = false;

    private boolean mIsBound = false;

    private CommunicationService mCommunicationService;

    final Messenger mMessenger = new Messenger(new MessageHandler());

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.testlayout);
        setupUI();

    }

    @Override
    protected void onResume() {
        super.onResume();
        doBindService();

    }

    // FIXME: move all necessary code to CommunicationService.onUnbind

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    public void onBackPressed() {
        // TODO Auto-generated method stub
        mCommunicationService.disconnect();
        stopService(new Intent(this, CommunicationService.class));
        doUnbindService();
        finish();
        super.onBackPressed();
    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();
        // mCommunicationService.disconnect();
        // stopService(new Intent(this, CommunicationService.class));
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName aClassName,
                        IBinder aService) {
            mCommunicationService = ((CommunicationService.CBinder) aService)
                            .getService();
            //            mCommunicationService.connectTo(Protocol.NETWORK, "192.168.0.18");
            mCommunicationService.setActivityMessenger(mMessenger);
            enableButtons(true);
        }

        @Override
        public void onServiceDisconnected(ComponentName aClassName) {
            mCommunicationService = null;
            enableButtons(false);
        }
    };

    void doBindService() {
        Intent aIntent = new Intent(this, CommunicationService.class);
        startService(aIntent);
        bindService(aIntent, mConnection, Context.BIND_IMPORTANT);
        mIsBound = true;
    }

    void doUnbindService() {
        mCommunicationService.setActivityMessenger(null);
        if (mIsBound) {
            unbindService(mConnection);
            mIsBound = false;
        }
    }

    private Button mButtonNext;

    private Button mButtonPrevious;

    private ImageView mImageView;

    private TextView mSlideLabel;

    private void setupUI() {
        mButtonNext = (Button) findViewById(R.id.button_next);
        mButtonPrevious = (Button) findViewById(R.id.button_previous);
        mImageView = (ImageView) findViewById(R.id.image_preview);
        mSlideLabel = (TextView) findViewById(R.id.label_curSlide);

        enableButtons(false);

        mButtonNext.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mCommunicationService.getTransmitter().nextTransition();

            }

        });

        mButtonPrevious.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mCommunicationService.getTransmitter().previousTransition();

            }

        });

        Button mThumbnailButton = (Button) findViewById(R.id.button_thumbnail);

        mThumbnailButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent aIntent = new Intent(TestClient.this,
                                PresentationActivity.class);
                startActivity(aIntent);
            }
        });

    }

    private void enableButtons(boolean aEnabled) {
        mButtonNext.setEnabled(aEnabled);
        mButtonPrevious.setEnabled(aEnabled);
    }

    class MessageHandler extends Handler {
        @Override
        public void handleMessage(Message aMessage) {
            Bundle aData = aMessage.getData();
            switch (aMessage.what) {
            case CommunicationService.MSG_SLIDE_CHANGED:
                int newSlide = aData.getInt("slide_number");
                mSlideLabel.setText("Slide " + newSlide);
                mCurrentPreviewImageMissing = true;
                // We continue on to try and update the image.
            case CommunicationService.MSG_SLIDE_PREVIEW:
                int aSlideNumber = aData.getInt("slide_number");
                if (mCurrentPreviewImageMissing) {
                    Bitmap aImage = mCommunicationService.getSlideShow()
                                    .getImage(aSlideNumber);
                    if (aImage != null) {
                        mImageView.setImageBitmap(aImage);
                        mCurrentPreviewImageMissing = false;
                    }
                }
                break;

            }
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */