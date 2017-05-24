package org.libreoffice;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.webkit.WebView;

public class PresentationActivity extends AppCompatActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.presentation_mode);

        Intent intent = getIntent();
        String filePath = intent.getDataString();

        WebView wv = (WebView) findViewById(R.id.presentation_view);
        wv.getSettings().setJavaScriptEnabled(true);
        wv.loadUrl(filePath);
    }
}
