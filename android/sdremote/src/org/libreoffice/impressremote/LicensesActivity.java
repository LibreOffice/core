package org.libreoffice.impressremote;

import android.content.ContentResolver;
import android.os.Bundle;
import android.webkit.WebView;

import com.actionbarsherlock.app.SherlockActivity;
import com.actionbarsherlock.view.MenuItem;

public class LicensesActivity extends SherlockActivity {
    private static final String SCHEME = ContentResolver.SCHEME_FILE;
    private static final String AUTHORITY = "android_asset";
    private static final String PATH = "licenses.html";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_licenses);

        setUpHomeButton();

        setUpLicenses();
    }

    private void setUpHomeButton() {
        getSupportActionBar().setHomeButtonEnabled(true);
    }

    private void setUpLicenses() {
        getLicensesView().loadUrl(buildLicensesUri());
    }

    private WebView getLicensesView() {
        return (WebView) findViewById(R.id.view_licenses);
    }

    private String buildLicensesUri() {
        return String.format("%s:///%s/%s", SCHEME, AUTHORITY, PATH);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case android.R.id.home:
                navigateUp();
                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void navigateUp() {
        finish();
    }
}
