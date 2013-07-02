package org.libreoffice.impressremote;

import android.os.Bundle;
import android.support.v4.app.Fragment;

import com.actionbarsherlock.app.ActionBar;
import com.actionbarsherlock.app.SherlockFragmentActivity;

public class ComputersActivity extends SherlockFragmentActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setUpTabs();
    }

    private void setUpTabs() {
        ActionBar aActionBar = getSupportActionBar();

        aActionBar.addTab(buildBluetoothServersTab());
        aActionBar.addTab(buildWiFiServersTab());
    }

    private ActionBar.Tab buildBluetoothServersTab() {
        ComputersFragment aFragment = ComputersFragment.newInstance();

        return buildActionBarTab(aFragment, R.string.title_bluetooth);
    }

    private ActionBar.Tab buildActionBarTab(Fragment aFragment, int aTitleResourceId) {
        ActionBar.Tab aTab = getSupportActionBar().newTab();

        aTab.setTabListener(new ActionBarTabListener(aFragment));
        aTab.setText(aTitleResourceId);

        return aTab;
    }

    private ActionBar.Tab buildWiFiServersTab() {
        ComputersFragment aFragment = ComputersFragment.newInstance();

        return buildActionBarTab(aFragment, R.string.title_wifi);
    }
}
