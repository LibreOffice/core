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
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.view.Menu;
import android.view.MenuItem;

import org.libreoffice.impressremote.adapter.ComputersPagerAdapter;
import org.libreoffice.impressremote.fragment.ComputersFragment;
import org.libreoffice.impressremote.util.BluetoothOperator;
import org.libreoffice.impressremote.util.Fragments;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.util.Preferences;
import org.libreoffice.impressremote.util.SavedStates;

public class ComputersActivity extends ActionBarActivity implements ActionBar.TabListener, ViewPager.OnPageChangeListener {
    private boolean mBluetoothWasEnabled;

    @Override
    protected void onCreate(Bundle aSavedInstanceState) {
        super.onCreate(aSavedInstanceState);

        saveBluetoothState(aSavedInstanceState);
        BluetoothOperator.enable();

        // Looks hacky but it seems to be the best way to set activity’s title
        // different to application’s label. The other way is setting title
        // to intents filter but it shows wrong label for recent apps screen then.

        ActionBar aActionBar = getSupportActionBar();

        aActionBar.setTitle(R.string.title_computers);
        aActionBar.setDisplayShowTitleEnabled(true);

        if (BluetoothOperator.isAvailable()) {
            setUpComputersLists();
        }
        else {
            Fragment aComputersFragment = ComputersFragment.newInstance(ComputersFragment.Type.WIFI);
            Fragments.Operator.add(this, aComputersFragment);
        }
    }

    private void saveBluetoothState(Bundle aSavedInstanceState) {
        // In more ideal world this work should be done at the service.
        // Unfortunately service cannot save or restore its state.
        // On other hand we should remember Bluetooth state exactly
        // after the app’s start and pass it during recreation cycle.

        if (!BluetoothOperator.isAvailable()) {
            return;
        }

        mBluetoothWasEnabled = wasBluetoothEnabled(aSavedInstanceState);
    }

    private boolean wasBluetoothEnabled(Bundle aSavedInstanceState) {
        if (aSavedInstanceState == null) {
            return BluetoothOperator.getAdapter().isEnabled();
        }

        return aSavedInstanceState.getBoolean(SavedStates.Keys.BLUETOOTH_ENABLED);
    }

    private void setUpComputersLists() {
        setContentView(R.layout.activity_computers);

        ActionBar aActionBar = getSupportActionBar();

        aActionBar.addTab(buildActionBarTab(
            R.string.title_bluetooth), ComputersPagerAdapter.PagesIndices.BLUETOOTH);
        aActionBar.addTab(buildActionBarTab(
            R.string.title_wifi), ComputersPagerAdapter.PagesIndices.WIFI);

        ViewPager aComputersPager = getComputersPager();

        aComputersPager.setAdapter(new ComputersPagerAdapter(getSupportFragmentManager()));
        aComputersPager.setOnPageChangeListener(this);

        getSupportActionBar().setSelectedNavigationItem(loadTabIndex());
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

        supportInvalidateOptionsMenu();
    }

    private ViewPager getComputersPager() {
        return (ViewPager) findViewById(R.id.pager_computers);
    }

    @Override
    public void onTabUnselected(ActionBar.Tab aTab, FragmentTransaction aTransaction) {
    }

    @Override
    public void onTabReselected(ActionBar.Tab aTab, FragmentTransaction aTransaction) {
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

    private int loadTabIndex() {
        Preferences aPreferences = Preferences.getApplicationStatesInstance(this);

        return aPreferences.getInt(Preferences.Keys.SELECTED_COMPUTERS_TAB_INDEX);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu aMenu) {
        getMenuInflater().inflate(R.menu.menu_action_bar_computers, aMenu);

        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu aMenu) {
        if (!BluetoothOperator.isAvailable()) {
            return super.onPrepareOptionsMenu(aMenu);
        }

        MenuItem aComputerAddingMenuItem = aMenu.findItem(R.id.menu_add_computer);

        switch (getSupportActionBar().getSelectedNavigationIndex()) {
            case ComputersPagerAdapter.PagesIndices.BLUETOOTH:
                aComputerAddingMenuItem.setVisible(false);
                break;

            case ComputersPagerAdapter.PagesIndices.WIFI:
                aComputerAddingMenuItem.setVisible(true);

            default:
                break;
        }

        return super.onPrepareOptionsMenu(aMenu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case R.id.menu_settings:
                callSettingsActivity();
                return true;

            case R.id.menu_requirements:
                callRequirementsActivity();
                return true;

            case R.id.menu_licenses:
                callLicensesActivity();
                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void callSettingsActivity() {
        Intent aIntent = Intents.buildSettingsIntent(this);
        startActivity(aIntent);
    }

    private void callRequirementsActivity() {
        Intent aIntent = Intents.buildRequirementsIntent(this);
        startActivity(aIntent);
    }

    private void callLicensesActivity() {
        Intent aIntent = Intents.buildLicensesIntent(this);
        startActivity(aIntent);
    }

    @Override
    protected void onStop() {
        super.onStop();

        Preferences aPreferences = Preferences.getApplicationStatesInstance(this);
        int aTabIndex = getSupportActionBar().getSelectedNavigationIndex();

        aPreferences.setInt(Preferences.Keys.SELECTED_COMPUTERS_TAB_INDEX, aTabIndex);
    }

    @Override
    protected void onSaveInstanceState(Bundle aSavedInstanceState) {
        super.onSaveInstanceState(aSavedInstanceState);

        aSavedInstanceState.putBoolean(SavedStates.Keys.BLUETOOTH_ENABLED, mBluetoothWasEnabled);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (!BluetoothOperator.isAvailable()) {
            return;
        }

        if (mBluetoothWasEnabled) {
            return;
        }

        BluetoothOperator.disable();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
