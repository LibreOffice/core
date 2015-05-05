/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.support.v7.app.ActionBar;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

import org.libreoffice.canvas.ImageUtils;
import org.libreoffice.kit.Document;

/**
 * Controls the changes to the toolbar.
 */
public class ToolbarController {
    private static final String LOGTAG = ToolbarController.class.getSimpleName();
    private final Toolbar mToolbar;
    private final ActionBar mActionBar;
    private Menu mOptionsMenu;
    private Context mContext;

    public ToolbarController(Context context, ActionBar actionBar, Toolbar toolbar) {
        mToolbar = toolbar;
        mActionBar = actionBar;
        mContext = context;
        switchToViewMode();
    }

    public void disableMenuItem(final int menuItemId, final boolean disabled) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                MenuItem menuItem = mOptionsMenu.findItem(menuItemId);
                if (menuItem != null) {
                    menuItem.setEnabled(!disabled);
                } else {
                    Log.e(LOGTAG, "MenuItem not found.");
                }
            }
        });
    }

    public void onToggleStateChanged(int type, boolean pressed) {
        MenuItem menuItem = null;
        Bitmap icon = null;
        switch (type) {
            case Document.BOLD:
                menuItem = mOptionsMenu.findItem(R.id.action_bold);
                icon = BitmapFactory.decodeResource(mContext.getResources(), R.drawable.action_bold);
                break;
            case Document.ITALIC:
                menuItem = mOptionsMenu.findItem(R.id.action_italic);
                icon = BitmapFactory.decodeResource(mContext.getResources(), R.drawable.action_italic);
                break;
            case Document.UNDERLINE:
                menuItem = mOptionsMenu.findItem(R.id.action_underline);
                icon = BitmapFactory.decodeResource(mContext.getResources(), R.drawable.action_underline);
                break;
            case Document.STRIKEOUT:
                menuItem = mOptionsMenu.findItem(R.id.action_strikeout);
                icon = BitmapFactory.decodeResource(mContext.getResources(), R.drawable.action_strikeout);
                break;
            default:
                Log.e(LOGTAG, "Uncaptured state change type: " + type);
                return;
        }

        if (menuItem == null) {
            Log.e(LOGTAG, "MenuItem not found.");
            return;
        }

        if (pressed) {
            icon = ImageUtils.bitmapToPressed(icon);
        }

        final MenuItem fMenuItem = menuItem;
        final Bitmap fIcon = icon;
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                fMenuItem.setIcon(new BitmapDrawable(mContext.getResources(), fIcon));
            }
        });
    }

    public void setOptionMenu(Menu menu) {
        mOptionsMenu = menu;
    }

    /**
     * Change the toolbar to edit mode.
     */
    void switchToEditMode() {
        if (!LOKitShell.isEditingEnabled())
            return;

        // Ensure the change is done on UI thread
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                mActionBar.setDisplayHomeAsUpEnabled(false);
                mToolbar.setNavigationIcon(R.drawable.ic_check_grey600_24dp);
                mToolbar.setTitle(null);

            }
        });
    }

    /**
     * Change the toolbar to view mode.
     */
    void switchToViewMode() {
        if (!LOKitShell.isEditingEnabled())
            return;

        // Ensure the change is done on UI thread
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
