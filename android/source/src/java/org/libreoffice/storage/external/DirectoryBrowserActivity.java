package org.libreoffice.storage.external;


import android.app.Fragment;
import android.app.FragmentManager;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;

import org.libreoffice.R;

/**
 * Container for DirectoryBrowserFragment
 */
public class DirectoryBrowserActivity extends AppCompatActivity {
    public static final String DIRECTORY_PATH_EXTRA = "org.libreoffie.directory_path_extra";

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent data = getIntent();
        String initialPath = data.getStringExtra(DIRECTORY_PATH_EXTRA);

        setContentView(R.layout.activity_directory_browser);
        FragmentManager fm = getFragmentManager();
        Fragment fragment = DirectoryBrowserFragment.newInstance(initialPath);
        fm.beginTransaction()
                .add(R.id.fragment_container, fragment)
                .commit();
    }

    @Override
    public void onBackPressed() {
        FragmentManager fm = getFragmentManager();
        if(fm.getBackStackEntryCount() > 0) {
            fm.popBackStack();
        } else {
            super.onBackPressed();
        }
    }
}
