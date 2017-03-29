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
import android.util.Log;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.Toast;

import org.libreoffice.LOEvent;
import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.R;

public class TextSelectionController implements View.OnClickListener, TextSelectionEventListener {
    private static String LOGTAG = TextSelectionController.class.getSimpleName();

    private LibreOfficeMainActivity context;
    private LinearLayout formattingToolbar;
    private LinearLayout searchToolbar;
    private LinearLayout textSelectionControlsToolbar;

    private ClipboardManager clipboard;
    private final String defaultMimeType = "text/plain;charset=utf-8";
    private final String writerMimeType = "text/html";

    public TextSelectionController(LibreOfficeMainActivity context) {
        this.context = context;
        clipboard = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);

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
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Cut"));
                break;
            case R.id.copy_button:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Copy"));
                LOKitShell.sendTextCopyEvent(writerMimeType);
                break;
            case R.id.paste_button:
                // LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Paste"));
                pasteText();
                break;
        }
    }

    private void pasteText() {
        ClipData clipData = clipboard.getPrimaryClip();
        ClipData.Item item = clipData.getItemAt(0);
        String text = item.coerceToText(context).toString();
        // TODO: This will work with @defaultMimetype for spreadsheet. Not working right now for writer.
        LOKitShell.sendPasteEvent(writerMimeType, text);
    }

    @Override
    public void copySelectedText(String text) {
        ClipData clip = ClipData.newPlainText("label", text);
        clipboard.setPrimaryClip(clip);
        Toast.makeText(context, R.string.message_text_copied, Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onTextPaste(boolean result) {
        Log.d(LOGTAG, "Paste Result " + result);
    }
}

