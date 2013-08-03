/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.activity;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;

import com.actionbarsherlock.app.ActionBar;
import com.actionbarsherlock.app.SherlockFragmentActivity;
import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuItem;
import org.libreoffice.impressremote.adapter.ComputersPagerAdapter;
import org.libreoffice.impressremote.fragment.ComputersFragment;
import org.libreoffice.impressremote.util.BluetoothOperator;
import org.libreoffice.impressremote.util.FragmentOperator;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.util.Preferences;

public class ComputersActivity extends SherlockFragmentActivity implements ActionBar.TabListener, ViewPager.OnPageChangeListener {
    private static final class TabsIndices {
        private TabsIndices() {
        }

        public static final int BLUETOOTH = 0;
        public static final int WIFI = 1;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setUpTitle();
        setUpContent();
    }

    private void setUpTitle() {
        // Looks hacky but it seems to be the best way to set activity title
        // different to application’s label. The other way is setting title
        // to intents filter but it shows wrong label for recent apps screen.

        ActionBar aActionBar = getSupportActionBar();

        aActionBar.setTitle(R.string.title_computers);
        aActionBar.setDisplayShowTitleEnabled(true);
    }

    private void setUpContent() {
        if (areMultipleComputersTypesAvailable()) {
            setUpComputersLists();
        }
        else {
            setUpComputersList();
        }
    }

    private boolean areMultipleComputersTypesAvailable() {
        return BluetoothOperator.isAvailable();
    }

    private void setUpComputersLists() {
        setContentView(R.layout.activity_computers);

        setUpTabs();
        setUpComputersPager();

        setUpSavedTab();
    }

    private void setUpTabs() {
        ActionBar aActionBar = getSupportActionBar();

        aActionBar.addTab(buildActionBarTab(R.string.title_bluetooth), TabsIndices.BLUETOOTH);
        aActionBar.addTab(buildActionBarTab(R.string.title_wifi), TabsIndices.WIFI);
    }

    private ActionBar.Tab buildActionBarTab(int aTitleResourceId) {
        ActionBar.Tab aTab = getSupportActionBar().newTab();

        aTab.setTabListener(this);
        aTab.setText(aTitleResourceId);

        return aTab;
    }

    @Override
    public void onTabSelected(ActionBar.Tab aTab, FragmentTransaction aTransaction) {
        getComputersPager().setCurrentItem(aTab.getPosition());

        refreshActionBarMenu();
    }

    private ViewPager getComputersPager() {
        return (ViewPager) findViewById(R.id.pager_computers);
    }

    private void refreshActionBarMenu() {
        supportInvalidateOptionsMenu();
    }

    @Override
    public void onTabUnselected(ActionBar.Tab aTab, FragmentTransaction aTransaction) {
    }

    @Override
    public void onTabReselected(ActionBar.Tab aTab, FragmentTransaction aTransaction) {
    }

    private void setUpComputersPager() {
        ViewPager aComputersPager = getComputersPager();

        aComputersPager.setAdapter(buildComputersPagerAdapter());
        aComputersPager.setOnPageChangeListener(this);
    }

    private PagerAdapter buildComputersPagerAdapter() {
        return new ComputersPagerAdapter(getSupportFragmentManager());
    }

    @Override
    public void onPageSelected(int aPosition) {
        getSupportActionBar().setSelectedNavigationItem(aPosition);
    }

    @Override
    public void onPageScrolled(int aPosition, float aPositionOffset, int aPositionOffsetPixels) {
    }

    @Override
    public void onPageScrollStateChanged(int aPosition) {
    }

    private void setUpSavedTab() {
        getSupportActionBar().setSelectedNavigationItem(loadTabIndex());
    }

    private int loadTabIndex() {
        return Preferences.getApplicationStatesInstance(this).getInt("saved_tab");
    }

    private void setUpComputersList() {
        Fragment aComputersFragment = ComputersFragment.newInstance(ComputersFragment.Type.WIFI);

        FragmentOperator.addFragment(this, aComputersFragment);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu aMenu) {
        getSupportMenuInflater().inflate(R.menu.menu_action_bar_computers, aMenu);

        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu aMenu) {
        if (!areMultipleComputersTypesAvailable()) {
            return super.onPrepareOptionsMenu(aMenu);
        }

        MenuItem aComputerAddingMenuItem = aMenu.findItem(R.id.menu_add_computer);

        switch (getSupportActionBar().getSelectedNavigationIndex()) {
            case TabsIndices.BLUETOOTH:
                aComputerAddingMenuItem.setVisible(false);
                break;

            case TabsIndices.WIFI:
                aComputerAddingMenuItem.setVisible(true);

            default:
                break;
        }

        return super.onPrepareOptionsMenu(aMenu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case R.id.menu_licenses:
                callLicensesActivity();
                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void callLicensesActivity() {
        Intent aIntent = Intents.buildLicensesIntent(this);
        startActivity(aIntent);
    }

    @Override
    protected void onStop() {
        super.onStop();

        saveTabIndex();
    }

    private void saveTabIndex() {
        int aTabIndex = getSupportActionBar().getSelectedNavigationIndex();

        Preferences.getApplicationStatesInstance(this).setInt("saved_tab", aTabIndex);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
