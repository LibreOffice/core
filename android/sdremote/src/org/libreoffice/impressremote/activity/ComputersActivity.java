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

public class ComputersActivity extends SherlockFragmentActivity implements ActionBar.TabListener, ViewPager.OnPageChangeListener {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setUpContent();
    }

    private void setUpContent() {
        if (BluetoothOperator.isAvailable()) {
            setUpComputersLists();
        }
        else {
            setUpComputersList();
        }
    }

    private void setUpComputersLists() {
        setContentView(R.layout.activity_computers);

        setUpTabs();
        setUpComputersPager();
    }

    private void setUpTabs() {
        ActionBar aActionBar = getSupportActionBar();

        aActionBar.addTab(buildActionBarTab(R.string.title_bluetooth));
        aActionBar.addTab(buildActionBarTab(R.string.title_wifi));
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
