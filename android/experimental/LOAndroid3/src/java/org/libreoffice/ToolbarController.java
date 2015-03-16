/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.support.v7.app.ActionBar;
import android.support.v7.widget.Toolbar;

public class ToolbarController {
    private final Toolbar mToolbar;
    private final ActionBar mActionBar;

    public ToolbarController(ActionBar actionBar, Toolbar toolbar) {
        mToolbar = toolbar;
        mActionBar = actionBar;
        switchToViewMode();
    }

    void switchToEditMode() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                mActionBar.setDisplayHomeAsUpEnabled(false);
                mToolbar.setNavigationIcon(R.drawable.ic_check_grey600_24dp);
                mToolbar.setTitle(null);

            }
        });
    }

    void switchToViewMode() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                mToolbar.setNavigationIcon(R.drawable.ic_menu_grey600_24dp);
                mToolbar.setTitle("LibreOffice");
                mActionBar.setDisplayHomeAsUpEnabled(true);
            }
        });
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
