package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.view.View;
import android.view.View.OnClickListener;

public class StartPresentationActivity extends Activity {
    private CommunicationService mCommunicationService = null;
    private boolean mIsBound = false;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_startpresentation);
        bindService(new Intent(this, CommunicationService.class), mConnection,
                        Context.BIND_IMPORTANT);
        mIsBound = true;

        IntentFilter aFilter = new IntentFilter(
                        CommunicationService.MSG_SLIDESHOW_STARTED);
        LocalBroadcastManager.getInstance(this).registerReceiver(mListener,
                        aFilter);

        findViewById(R.id.startpresentation_button).setOnClickListener(
                        mClickListener);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        LocalBroadcastManager.getInstance(this).unregisterReceiver(mListener);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName aClassName,
                        IBinder aService) {

            mCommunicationService = ((CommunicationService.CBinder) aService)
                            .getService();

            if (mCommunicationService.isSlideShowRunning()) {
                Intent nIntent = new Intent(StartPresentationActivity.this,
                                PresentationActivity.class);
                startActivity(nIntent);
            }

        }

        @Override
        public void onServiceDisconnected(ComponentName aClassName) {
            mCommunicationService = null;
        }
    };

    private OnClickListener mClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            if (mCommunicationService != null) {
                mCommunicationService.getTransmitter().startPresentation();
            }
        }
    };

    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (aIntent.getAction().equals(
                            CommunicationService.MSG_SLIDESHOW_STARTED)) {
                Intent nIntent = new Intent(StartPresentationActivity.this,
                                PresentationActivity.class);
                startActivity(nIntent);
            }

        }
    };
}
