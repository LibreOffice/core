/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

/**
 * Controls the changes to the toolbar.
 */
public class ToolbarController implements Toolbar.OnMenuItemClickListener {
    private static final String LOGTAG = ToolbarController.class.getSimpleName();
    private final Toolbar mToolbarTop;

    private final LibreOfficeMainActivity mContext;
    private final Menu mMainMenu;

    private boolean isEditModeOn = false;

    public ToolbarController(LibreOfficeMainActivity context, Toolbar toolbarTop) {
        mToolbarTop = toolbarTop;
        mContext = context;

        mToolbarTop.inflateMenu(R.menu.main);
        mToolbarTop.setOnMenuItemClickListener(this);
        switchToViewMode();

        mMainMenu = mToolbarTop.getMenu();
    }

    public void disableMenuItem(final int menuItemId, final boolean disabled) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                MenuItem menuItem = mMainMenu.findItem(menuItemId);
                if (menuItem != null) {
                    menuItem.setEnabled(!disabled);
                } else {
                    Log.e(LOGTAG, "MenuItem not found.");
                }
            }
        });
    }

    public void setEditModeOn(boolean enabled) {
        isEditModeOn = enabled;
    }

    public boolean getEditModeStatus() {
        return isEditModeOn;
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
                mMainMenu.setGroupVisible(R.id.group_edit_actions, true);
                mToolbarTop.setNavigationIcon(R.drawable.ic_check);
                mToolbarTop.setLogo(null);
                setEditModeOn(true);
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
                mMainMenu.setGroupVisible(R.id.group_edit_actions, false);
                mToolbarTop.setNavigationIcon(R.drawable.lo_icon);
                mToolbarTop.setLogo(null);
                setEditModeOn(false);
                mContext.hideBottomToolbar();
                mContext.hideSoftKeyboard();
            }
        });
    }

    @Override
    public boolean onMenuItemClick(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_keyboard:
                mContext.showSoftKeyboard();
                break;
            case R.id.action_format:
                mContext.showFormattingToolbar();
                break;
            case R.id.action_about:
                mContext.showAbout();
                return true;
            case R.id.action_save:
                if (mContext.isNewDocument) {
                    mContext.saveAs();
                } else {
                    mContext.saveDocument();
                }
                return true;
            case R.id.action_parts:
                mContext.openDrawer();
                return true;
            case R.id.action_settings:
                mContext.showSettings();
                return true;
            case R.id.action_search:
                mContext.showSearchToolbar();
                return true;
            case R.id.action_undo:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Undo"));
                return true;
            case R.id.action_redo:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Redo"));
                return true;
            case R.id.action_presentation:
                mContext.preparePresentation();
                return true;
        }
        return false;
    }

    void setupToolbars() {
        if (mContext.usesTemporaryFile()) {
            disableMenuItem(R.id.action_save, true);
            Toast.makeText(mContext, mContext.getString(R.string.temp_file_saving_disabled), Toast.LENGTH_LONG).show();
        }
        mMainMenu.findItem(R.id.action_parts).setVisible(mContext.isDrawerEnabled());
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
