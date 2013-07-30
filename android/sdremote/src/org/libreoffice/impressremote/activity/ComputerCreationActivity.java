/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.activity;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Patterns;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;

import com.actionbarsherlock.app.ActionBar;
import com.actionbarsherlock.app.SherlockFragmentActivity;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;

public class ComputerCreationActivity extends SherlockFragmentActivity implements View.OnClickListener {
    @Override
    protected void onCreate(Bundle aSavedInstanceState) {
        super.onCreate(aSavedInstanceState);
        setContentView(R.layout.activity_computer_creation);

        setUpActionBar();
    }

    private void setUpActionBar() {
        View aActionBarView = buildCustomActionBarView();
        ActionBar.LayoutParams aActionBarParams = buildCustomActionBarLayoutParams();

        getSupportActionBar().setCustomView(aActionBarView, aActionBarParams);

        getCancelButton().setOnClickListener(this);
        getSaveButton().setOnClickListener(this);
    }

    private View buildCustomActionBarView() {
        Context aContext = getSupportActionBar().getThemedContext();
        LayoutInflater aInflater = LayoutInflater.from(aContext);

        return aInflater.inflate(R.layout.action_bar_computer_creation, null);
    }

    private ActionBar.LayoutParams buildCustomActionBarLayoutParams() {
        return new ActionBar.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT);
    }

    private View getCancelButton() {
        return getSupportActionBar().getCustomView().findViewById(R.id.button_cancel);
    }

    private View getSaveButton() {
        return getSupportActionBar().getCustomView().findViewById(R.id.button_save);
    }

    @Override
    public void onClick(View aView) {
        if (aView.equals(getCancelButton())) {
            cancelCreation();
            return;
        }

        if (aView.equals(getSaveButton())) {
            saveServer();
        }
    }

    private void cancelCreation() {
        finish();
    }

    private void saveServer() {
        String aIpAddress = getText(getIpAddressEdit());
        String aName = getText(getNameEdit());

        if (!isIpAddressValid(aIpAddress)) {
            getIpAddressEdit().setError(getText(R.string.message_ip_address_validation));
            return;
        }

        if (TextUtils.isEmpty(aName)) {
            aName = aIpAddress;
        }

        finish(aIpAddress, aName);
    }

    private String getText(EditText aEdit) {
        return aEdit.getText().toString().trim();
    }

    private EditText getIpAddressEdit() {
        return (EditText) findViewById(R.id.edit_ip_address);
    }

    private EditText getNameEdit() {
        return (EditText) findViewById(R.id.edit_name);
    }

    private boolean isIpAddressValid(String aIpAddress) {
        return Patterns.IP_ADDRESS.matcher(aIpAddress).matches();
    }

    private void finish(String aIpAddress, String aName) {
        Intent aIntent = Intents.buildComputerCreationResultIntent(aIpAddress, aName);
        setResult(Activity.RESULT_OK, aIntent);

        finish();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
