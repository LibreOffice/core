package org.libreoffice.impressremote;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;

public class PresentationActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_presentation);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_presentation, menu);
        return true;
    }
}
