package org.libreoffice.impressremote;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;

import com.actionbarsherlock.app.ActionBar;

public class ActionBarTabListener implements ActionBar.TabListener {
    private final Fragment mTabFragment;

    public ActionBarTabListener(Fragment aTabFragment) {
        mTabFragment = aTabFragment;
    }

    @Override
    public void onTabSelected(ActionBar.Tab aTab, FragmentTransaction aFragmentTransaction) {
        if (mTabFragment.isDetached()) {
            aFragmentTransaction.attach(mTabFragment);
        }

        aFragmentTransaction.replace(android.R.id.content, mTabFragment);
    }

    @Override
    public void onTabUnselected(ActionBar.Tab aTab, FragmentTransaction aFragmentTransaction) {
        if (mTabFragment.isDetached()) {
            return;
        }

        aFragmentTransaction.detach(mTabFragment);
    }

    @Override
    public void onTabReselected(ActionBar.Tab aTab, FragmentTransaction aFragmentTransaction) {
    }
}
