package org.mozilla.gecko;

import android.content.Context;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.util.Log;

import org.libreoffice.LOKitShell;


public class OnSlideSwipeListener implements OnTouchListener {
    private static String LOGTAG = OnSlideSwipeListener.class.getName();

    private final GestureDetector mGestureDetector;

    public OnSlideSwipeListener(Context ctx){
        mGestureDetector = new GestureDetector(ctx, new GestureListener());
    }

    private final class GestureListener extends SimpleOnGestureListener {

        private static final int SWIPE_THRESHOLD = 100;
        private static final int SWIPE_VELOCITY_THRESHOLD = 100;

        @Override
        public boolean onDown(MotionEvent e) {
            return false;
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velX, float velY) {
            try {
                float diffY = e2.getY() - e1.getY();
                float diffX = e2.getX() - e1.getX();
                if (Math.abs(diffX) > Math.abs(diffY)) {
                    if (Math.abs(diffX) > SWIPE_THRESHOLD
                            && Math.abs(velX) > SWIPE_VELOCITY_THRESHOLD) {
                        if (diffX > 0) {
                            onSwipeRight();
                        } else {
                            onSwipeLeft();
                        }
                    }
                }
            } catch (Exception exception) {
                exception.printStackTrace();
            }
            return false;
        }
    }

    public void onSwipeRight() {
        Log.d(LOGTAG, "onSwipeRight");
        LOKitShell.sendSwipeRightEvent();
    }

    public void onSwipeLeft() {
        Log.d(LOGTAG, "onSwipeLeft");
        LOKitShell.sendSwipeLeftEvent();
    }

    @Override
    public boolean onTouch(View v, MotionEvent me) {
        return mGestureDetector.onTouchEvent(me);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
