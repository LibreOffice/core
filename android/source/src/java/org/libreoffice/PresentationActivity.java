package org.libreoffice;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.view.GestureDetectorCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.ImageButton;

public class PresentationActivity extends AppCompatActivity {

    private static final String LOGTAG = PresentationActivity.class.getSimpleName();
    WebView mWebView;
    View mGestureView;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // First we hide the status bar
        if (Build.VERSION.SDK_INT < 16) {
            // If the Android version is lower than Jellybean, use this call to hide
            // the status bar.
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                    WindowManager.LayoutParams.FLAG_FULLSCREEN);
        } else {
            // If higher than Jellybean
            View decorView = getWindow().getDecorView();
            // Hide the status bar.
            int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
            decorView.setSystemUiVisibility(uiOptions);
        }

        setContentView(R.layout.presentation_mode);

        // get intent and url
        Intent intent = getIntent();
        String filePath = intent.getDataString();

        // set up WebView
        mWebView = (WebView) findViewById(R.id.presentation_view);
        mWebView.getSettings().setJavaScriptEnabled(true);
        mWebView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                return true;
            }
        });

        // set up buttons within presentation_gesture_view
        ImageButton prevButton = (ImageButton) findViewById(R.id.slide_show_nav_prev);
        ImageButton nextButton = (ImageButton) findViewById(R.id.slide_show_nav_next);
        Button backButton = (Button) findViewById(R.id.slide_show_nav_back);

        prevButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                pageLeft();
            }
        });
        nextButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                pageRight();
            }
        });
        backButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onBackPressed();
            }
        });

        // set up presentation_gesture_view
        mGestureView = findViewById(R.id.presentation_gesture_view);
        final GestureDetectorCompat gestureDetector =
                new GestureDetectorCompat(this, new presentationGestureViewListener());
        mGestureView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                return gestureDetector.onTouchEvent(event);
            }
        });

        // load url
        mWebView.loadUrl(filePath);
    }

    private class presentationGestureViewListener extends GestureDetector.SimpleOnGestureListener {
        private static final int SWIPE_VELOCITY_THRESHOLD = 100;
        private static final int SCROLL_THRESHOLD = 10; // if scrollCounter is larger than this, a page switch is triggered
        private int scrollCounter = 0; // a counter for measuring scrolling distance

        @Override
        public boolean onDown(MotionEvent e) {
            return true;
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            boolean result = false;
            try {
                float diffY = e2.getY() - e1.getY();
                float diffX = e2.getX() - e1.getX();
                if (Math.abs(diffX) > Math.abs(diffY)) {
                    if (Math.abs(velocityX) > SWIPE_VELOCITY_THRESHOLD) {
                        if (diffX > 0) {
                            pageRight();
                        } else {
                            pageLeft();
                        }
                        result = true;
                    }
                }
            } catch (Exception exception) {
                exception.printStackTrace();
            }
            return result;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
            boolean result = false;
            try {
                float diffY = e2.getY() - e1.getY();
                float diffX = e2.getX() - e1.getX();
                if (Math.abs(diffX) < Math.abs(diffY)) {
                    if (distanceY > 0) {
                        scrollCounter++;
                        if (scrollCounter >= SCROLL_THRESHOLD) {
                            pageRight();
                            scrollCounter = 0;
                        }
                    } else {
                        scrollCounter--;
                        if (scrollCounter <= -SCROLL_THRESHOLD) {
                            pageLeft();
                            scrollCounter = 0;
                        }
                    }
                    result = true;
                }
            } catch (Exception exception) {
                exception.printStackTrace();
            }
            return result;
        }

        @Override
        public boolean onSingleTapUp(MotionEvent e) {
            if (e.getX() < mGestureView.getWidth()/3) {
                pageLeft();
            } else if (e.getX() < mGestureView.getWidth()*2/3) {
                hideControlButtons();
            } else {
                pageRight();
            }
            return true;
        }
    }

    private void hideControlButtons() {
        View[] views= {findViewById(R.id.slide_show_nav_prev),findViewById(R.id.slide_show_nav_next),findViewById(R.id.slide_show_nav_back)} ;
        for (View view : views) {
            if (view.getVisibility() == View.GONE) {
                view.setVisibility(View.VISIBLE);
            } else if (view.getVisibility() == View.VISIBLE) {
                view.setVisibility(View.GONE);
            }
        }
    }

    private void pageLeft() {
        mWebView.dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_LEFT));
    }

    private void pageRight() {
        mWebView.dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_RIGHT));
    }
}