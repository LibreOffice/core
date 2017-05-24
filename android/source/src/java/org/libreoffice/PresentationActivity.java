package org.libreoffice;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.ImageButton;

public class PresentationActivity extends AppCompatActivity {

    private static final String LOGTAG = PresentationActivity.class.getSimpleName();
    WebView mWebView;

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

        // set up overlay view
        ImageButton prevButton = (ImageButton) findViewById(R.id.slide_show_nav_prev);
        ImageButton nextButton = (ImageButton) findViewById(R.id.slide_show_nav_next);
        Button backButton = (Button) findViewById(R.id.slide_show_nav_back);
        prevButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mWebView.dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_LEFT));
            }
        });
        nextButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mWebView.dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_RIGHT));
            }
        });
        backButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onBackPressed();
            }
        });

        // load url
        mWebView.loadUrl(filePath);
    }
}