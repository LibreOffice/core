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
import android.graphics.Color;
import android.support.design.widget.Snackbar;
import android.util.Log;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;

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
    private ImageButton cutButton;
    private ImageButton copyButton;
    private ImageButton pasteButton;

    private ClipboardManager clipboard;
    private final String defaultMimeType = "text/plain;charset=utf-8";
    private final String htmlMimeType = "text/html";
    private final String richTextMimeType = "text/richtext";

    // A boolean which handles whether the content to be pasted is coming from LO Clipboard or Android Clipboard
    public boolean INTERNAL_PASTE_ENABLED = false;

    public TextSelectionController(LibreOfficeMainActivity context) {
        this.context = context;
        clipboard = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);

        formattingToolbar = (LinearLayout) context.findViewById(R.id.formatting_toolbar);
        searchToolbar = (LinearLayout) context.findViewById(R.id.search_toolbar);
        textSelectionControlsToolbar = (LinearLayout) context.findViewById(R.id.text_selection_toolbar);

        cutButton = (ImageButton) textSelectionControlsToolbar.findViewById(R.id.cut_button);
        cutButton.setOnClickListener(this);
        copyButton = (ImageButton) textSelectionControlsToolbar.findViewById(R.id.copy_button);
        copyButton.setOnClickListener(this);
        pasteButton = (ImageButton) textSelectionControlsToolbar.findViewById(R.id.paste_button);
        pasteButton.setOnClickListener(this);

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

                // Disable the cut and paste buttons and grey them out if editing is not enabled
                if (!LOKitShell.isEditingEnabled()) {
                    cutButton.setEnabled(false);
                    pasteButton.setEnabled(false);
                    cutButton.setBackgroundColor(Color.GRAY);
                    pasteButton.setBackgroundColor(Color.GRAY);
                }
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
                // Copy should be done to both LO Clipboard and Android Clipboard
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Copy"));
                LOKitShell.sendTextCopyEvent(defaultMimeType);
                break;
            case R.id.paste_button:
                // If internal paste is not enabled, get the text from Android Clipboard, otherwise LO Clipboard
                if (!INTERNAL_PASTE_ENABLED) {
                    pasteText();
                } else {
                    LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Paste"));
                }
                break;
        }
    }

    /**
     * Paste text from Android Clipboard
     */
    private void pasteText() {
        ClipData clipData = clipboard.getPrimaryClip();
        ClipData.Item item = clipData.getItemAt(0);
        String text = item.coerceToText(context).toString();

        if (context.getMimeType().contains("text")) {
            // The document is a writer doc
            LOKitShell.sendPasteEvent(htmlMimeType, text);
        } else if (context.getMimeType().contains("spreadsheet")) {
            // The document is a spreadsheet
            LOKitShell.sendPasteEvent(defaultMimeType, text);
        } else {
            // For any other text editing based things
            LOKitShell.sendPasteEvent(richTextMimeType, text);
        }
    }

    @Override
    public void copySelectedText(String text) {
        ClipData clip = ClipData.newPlainText("label", text);
        clipboard.setPrimaryClip(clip);
        Snackbar.make(formattingToolbar, R.string.message_text_copied, Snackbar.LENGTH_SHORT).show();
    }

    @Override
    public void onTextPaste(boolean result) {
        Log.d(LOGTAG, "Paste Result " + result);
    }
}

