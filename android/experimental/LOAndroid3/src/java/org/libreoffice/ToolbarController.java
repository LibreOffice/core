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
