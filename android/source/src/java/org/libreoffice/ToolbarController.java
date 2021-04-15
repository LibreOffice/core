/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.KeyEvent;
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
    private String clipboardText = null;
    ClipboardManager clipboardManager;
    ClipData clipData;

    public ToolbarController(LibreOfficeMainActivity context, Toolbar toolbarTop) {
        mToolbarTop = toolbarTop;
        mContext = context;

        mToolbarTop.inflateMenu(R.menu.main);
        mToolbarTop.setOnMenuItemClickListener(this);
        switchToViewMode();

        mMainMenu = mToolbarTop.getMenu();
        clipboardManager = (ClipboardManager)mContext.getSystemService(Context.CLIPBOARD_SERVICE);
    }

    private void enableMenuItem(final int menuItemId, final boolean enabled) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                MenuItem menuItem = mMainMenu.findItem(menuItemId);
                if (menuItem != null) {
                    menuItem.setEnabled(enabled);
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
                if (!LibreOfficeMainActivity.isDeveloperMode() && mMainMenu.findItem(R.id.action_UNO_commands) != null) {
                    mMainMenu.findItem(R.id.action_UNO_commands).setVisible(false);
                } else {
                    mMainMenu.findItem(R.id.action_UNO_commands).setVisible(true);
                }
                if(mContext.getTileProvider() != null && mContext.getTileProvider().isSpreadsheet()){
                    mMainMenu.setGroupVisible(R.id.group_spreadsheet_options, true);
                } else if(mContext.getTileProvider() != null && mContext.getTileProvider().isPresentation()){
                    mMainMenu.setGroupVisible(R.id.group_presentation_options, true);
                }
                mToolbarTop.setNavigationIcon(R.drawable.ic_check);
                mToolbarTop.setLogo(null);
                setEditModeOn(true);
            }
        });
    }

    /**
     * Show clipboard Actions on the toolbar
     * */
    void showClipboardActions(final String value){
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                if(value  != null){
                    mMainMenu.setGroupVisible(R.id.group_edit_actions, false);
                    mMainMenu.setGroupVisible(R.id.group_edit_clipboard, true);
                    if(getEditModeStatus()){
                        showHideClipboardCutAndCopy(true);
                    } else {
                        mMainMenu.findItem(R.id.action_cut).setVisible(false);
                        mMainMenu.findItem(R.id.action_paste).setVisible(false);
                    }
                    clipboardText = value;
                }
            }
        });
    }

    void hideClipboardActions(){
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                mMainMenu.setGroupVisible(R.id.group_edit_actions, getEditModeStatus());
                mMainMenu.setGroupVisible(R.id.group_edit_clipboard, false);
            }
        });
    }

    void showHideClipboardCutAndCopy(final boolean option){
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                mMainMenu.findItem(R.id.action_copy).setVisible(option);
                mMainMenu.findItem(R.id.action_cut).setVisible(option);
            }
        });
    }

    /**
     * Change the toolbar to view mode.
     */
    void switchToViewMode() {
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
                if(mContext.getTileProvider() != null && mContext.getTileProvider().isSpreadsheet()){
                    mMainMenu.setGroupVisible(R.id.group_spreadsheet_options, false);
                } else if(mContext.getTileProvider() != null && mContext.getTileProvider().isPresentation()){
                    mMainMenu.setGroupVisible(R.id.group_presentation_options, false);
                }
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
                mContext.getTileProvider().saveDocument();
                return true;
            case R.id.action_save_as:
                mContext.saveDocumentAs();
                return true;
            case R.id.action_parts:
                mContext.openDrawer();
                return true;
            case R.id.action_exportToPDF:
                mContext.exportToPDF();
                return true;
            case R.id.action_print:
                mContext.getTileProvider().printDocument();
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
            case R.id.action_add_slide:
                mContext.addPart();
                return true;
            case R.id.action_add_worksheet:
                mContext.addPart();
                return true;
            case R.id.action_rename_worksheet:
            case R.id.action_rename_slide:
                mContext.renamePart();
                return true;
            case R.id.action_delete_worksheet:
                mContext.deletePart();
                return true;
            case R.id.action_delete_slide:
                mContext.deletePart();
                return true;
            case R.id.action_back:
                hideClipboardActions();
                return true;
            case R.id.action_copy:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Copy"));
                clipData = ClipData.newPlainText("clipboard data", clipboardText);
                clipboardManager.setPrimaryClip(clipData);
                Toast.makeText(mContext, mContext.getResources().getString(R.string.action_text_copied), Toast.LENGTH_SHORT).show();
                return true;
            case R.id.action_paste:
                clipData = clipboardManager.getPrimaryClip();
                ClipData.Item clipItem = clipData.getItemAt(0);
                mContext.setDocumentChanged(true);
                return mContext.getTileProvider().paste("text/plain;charset=utf-16", clipItem.getText().toString());
            case R.id.action_cut:
                clipData = ClipData.newPlainText("clipboard data", clipboardText);
                clipboardManager.setPrimaryClip(clipData);
                LOKitShell.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL));
                mContext.setDocumentChanged(true);
                return true;
            case R.id.action_UNO_commands:
                mContext.showUNOCommandsToolbar();
                return true;
        }
        return false;
    }

    void setupToolbars() {
        if (LibreOfficeMainActivity.isExperimentalMode()) {
            boolean enableSaveEntry = !LibreOfficeMainActivity.isReadOnlyMode() && mContext.hasLocationForSave();
            enableMenuItem(R.id.action_save, enableSaveEntry);
            if (LibreOfficeMainActivity.isReadOnlyMode()) {
                // show message in case experimental mode is enabled (i.e. editing is supported in general),
                // but current document is readonly
                Toast.makeText(mContext, mContext.getString(R.string.temp_file_saving_disabled), Toast.LENGTH_LONG).show();
            }
        }
        mMainMenu.findItem(R.id.action_parts).setVisible(mContext.isDrawerEnabled());
    }

    public void showItem(final int item){
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                mMainMenu.findItem(item).setVisible(true);

            }
        });
    }

    public void hideItem(final int item){
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                mMainMenu.findItem(item).setVisible(false);

            }
        });
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
