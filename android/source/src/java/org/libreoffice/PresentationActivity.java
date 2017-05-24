package org.libreoffice;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.ImageButton;

public class PresentationActivity extends AppCompatActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.presentation_mode);

        // get intent and url
        Intent intent = getIntent();
        String filePath = intent.getDataString();

        // set up WebView
        final WebView wv = (WebView) findViewById(R.id.presentation_view);
        wv.getSettings().setJavaScriptEnabled(true);

        // set up overlay view
        ImageButton prevButton = (ImageButton) findViewById(R.id.slide_show_nav_prev);
        ImageButton nextButton = (ImageButton) findViewById(R.id.slide_show_nav_next);
        Button backButton = (Button) findViewById(R.id.slide_show_nav_back);
        prevButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                wv.dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_LEFT));
            }
        });
        nextButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                wv.dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_RIGHT));
            }
        });
        backButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onBackPressed();
            }
        });

        // load url
        wv.loadUrl(filePath);
    }
}
