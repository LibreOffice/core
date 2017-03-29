/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.textselection;

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.Toast;

import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.R;

public class TextSelectionController implements View.OnClickListener, TextSelectionEventListener {

    private LibreOfficeMainActivity context;
    private LinearLayout formattingToolbar;
    private LinearLayout searchToolbar;
    private LinearLayout textSelectionControlsToolbar;

    public TextSelectionController(LibreOfficeMainActivity context) {
        this.context = context;

        formattingToolbar = (LinearLayout) context.findViewById(R.id.formatting_toolbar);
        searchToolbar = (LinearLayout) context.findViewById(R.id.search_toolbar);
        textSelectionControlsToolbar = (LinearLayout) context.findViewById(R.id.text_selection_toolbar);

        textSelectionControlsToolbar.findViewById(R.id.cut_button).setOnClickListener(this);
        textSelectionControlsToolbar.findViewById(R.id.copy_button).setOnClickListener(this);
        textSelectionControlsToolbar.findViewById(R.id.paste_button).setOnClickListener(this);
    }

    /**
     * Show the text selection controls.
     */
    public void showTextSelectionControls() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                context.showBottomToolbar();
                formattingToolbar.setVisibility(View.GONE);
                searchToolbar.setVisibility(View.GONE);
                textSelectionControlsToolbar.setVisibility(View.VISIBLE);
                context.hideSoftKeyboardDirect();
                context.isFormattingToolbarOpen=false;
                context.isSearchToolbarOpen=false;
            }
        });
    }

    /**
     * Hide text selection controls.
     */
    public void hideTextSelectionControls() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                context.hideBottomToolbar();
            }
        });
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.cut_button:
                // uno:cut
                break;
            case R.id.copy_button:
                LOKitShell.sendTextCopyEvent("text/plain;charset=utf-8");
                break;
            case R.id.paste_button:
                // uno:paste
                break;
        }
    }

    @Override
    public void copySelectedText(String text) {
        ClipboardManager clipboard = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
        ClipData clip = ClipData.newPlainText("label", text);
        clipboard.setPrimaryClip(clip);
        Toast.makeText(context, R.string.message_text_copied, Toast.LENGTH_SHORT).show();
    }
}

