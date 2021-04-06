/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.ui;

import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.graphics.drawable.Icon;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.view.ViewCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.OvershootInterpolator;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.libreoffice.AboutDialogFragment;
import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.LocaleHelper;
import org.libreoffice.R;
import org.libreoffice.SettingsActivity;
import org.libreoffice.SettingsListenerModel;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class LibreOfficeUIActivity extends AppCompatActivity implements SettingsListenerModel.OnSettingsPreferenceChangedListener, View.OnClickListener{
    public enum DocumentType {
        WRITER,
        CALC,
        IMPRESS,
        DRAW,
        INVALID
    }

    private String LOGTAG = LibreOfficeUIActivity.class.getSimpleName();

    private DocumentType newDocType = DocumentType.INVALID;

    public static final String EXPLORER_PREFS_KEY = "EXPLORER_PREFS";
    private static final String RECENT_DOCUMENTS_KEY = "RECENT_DOCUMENT_URIS";
    // delimiter used for storing multiple URIs in a string
    private static final String RECENT_DOCUMENTS_DELIMITER = " ";
    private static final String DISPLAY_LANGUAGE = "DISPLAY_LANGUAGE";

    public static final String NEW_DOC_TYPE_KEY = "NEW_DOC_TYPE_KEY";
    public static final String NEW_WRITER_STRING_KEY = "private:factory/swriter";
    public static final String NEW_IMPRESS_STRING_KEY = "private:factory/simpress";
    public static final String NEW_CALC_STRING_KEY = "private:factory/scalc";
    public static final String NEW_DRAW_STRING_KEY = "private:factory/sdraw";

    // keep this in sync with 'AndroidManifext.xml'
    private static final String[] SUPPORTED_MIME_TYPES = {
            "application/vnd.oasis.opendocument.text",
            "application/vnd.oasis.opendocument.graphics",
            "application/vnd.oasis.opendocument.presentation",
            "application/vnd.oasis.opendocument.spreadsheet",
            "application/vnd.oasis.opendocument.text-flat-xml",
            "application/vnd.oasis.opendocument.graphics-flat-xml",
            "application/vnd.oasis.opendocument.presentation-flat-xml",
            "application/vnd.oasis.opendocument.spreadsheet-flat-xml",
            "application/vnd.oasis.opendocument.text-template",
            "application/vnd.oasis.opendocument.spreadsheet-template",
            "application/vnd.oasis.opendocument.graphics-template",
            "application/vnd.oasis.opendocument.presentation-template",
            "application/rtf",
            "text/rtf",
            "application/msword",
            "application/vnd.ms-powerpoint",
            "application/vnd.ms-excel",
            "application/vnd.visio",
            "application/vnd.visio.xml",
            "application/x-mspublisher",
            "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
            "application/vnd.openxmlformats-officedocument.presentationml.presentation",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.template",
            "application/vnd.openxmlformats-officedocument.spreadsheetml.template",
            "application/vnd.openxmlformats-officedocument.presentationml.template",
            "text/csv",
            "text/comma-separated-values",
            "application/vnd.ms-works",
            "application/vnd.apple.keynote",
            "application/x-abiword",
            "application/x-pagemaker",
            "image/x-emf",
            "image/x-svm",
            "image/x-wmf",
            "image/svg+xml",
    };

    private static final int REQUEST_CODE_OPEN_FILECHOOSER = 12345;
    private static final int REQUEST_CODE_CREATE_NEW_DOCUMENT = 12346;

    private Animation fabOpenAnimation;
    private Animation fabCloseAnimation;
    private boolean isFabMenuOpen = false;
    private FloatingActionButton editFAB;
    private FloatingActionButton writerFAB;
    private FloatingActionButton drawFAB;
    private FloatingActionButton impressFAB;
    private FloatingActionButton calcFAB;
    private LinearLayout drawLayout;
    private LinearLayout writerLayout;
    private LinearLayout impressLayout;
    private LinearLayout calcLayout;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        readPreferences();
        SettingsListenerModel.getInstance().setListener(this);

        // init UI
        createUI();
        fabOpenAnimation = AnimationUtils.loadAnimation(this, R.anim.fab_open);
        fabCloseAnimation = AnimationUtils.loadAnimation(this, R.anim.fab_close);
    }

    @Override
    protected void attachBaseContext(Context newBase) {
        super.attachBaseContext(LocaleHelper.onAttach(newBase));
    }

    public void createUI() {
        setContentView(R.layout.activity_document_browser);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();

        if (actionBar != null) {
            actionBar.setIcon(R.drawable.lo_icon);
        }

        editFAB = findViewById(R.id.editFAB);
        editFAB.setOnClickListener(this);
        impressFAB = findViewById(R.id.newImpressFAB);
        impressFAB.setOnClickListener(this);
        writerFAB = findViewById(R.id.newWriterFAB);
        writerFAB.setOnClickListener(this);
        calcFAB = findViewById(R.id.newCalcFAB);
        calcFAB.setOnClickListener(this);
        drawFAB = findViewById(R.id.newDrawFAB);
        drawFAB.setOnClickListener(this);
        writerLayout = findViewById(R.id.writerLayout);
        impressLayout = findViewById(R.id.impressLayout);
        calcLayout = findViewById(R.id.calcLayout);
        drawLayout = findViewById(R.id.drawLayout);
        TextView openFileView = findViewById(R.id.open_file_view);
        openFileView.setOnClickListener(this);


        RecyclerView recentRecyclerView = findViewById(R.id.list_recent);

        SharedPreferences prefs = getSharedPreferences(EXPLORER_PREFS_KEY, MODE_PRIVATE);
        String recentPref = prefs.getString(RECENT_DOCUMENTS_KEY, "");
        List<String> recentFileStrings = Arrays.asList(recentPref.split(RECENT_DOCUMENTS_DELIMITER));

        final List<RecentFile> recentFiles = new ArrayList();
        for (String recentFileString : recentFileStrings) {
            Uri uri = Uri.parse(recentFileString);
            String filename = FileUtilities.retrieveDisplayNameForDocumentUri(getContentResolver(), uri);
            if (!filename.isEmpty()) {
                recentFiles.add(new RecentFile(uri, filename));
            }
        }

        recentRecyclerView.setLayoutManager(new GridLayoutManager(this, 2));
        recentRecyclerView.setAdapter(new RecentFilesAdapter(this, recentFiles));
    }

    private void expandFabMenu() {
        ViewCompat.animate(editFAB).rotation(45.0F).withLayer().setDuration(300).setInterpolator(new OvershootInterpolator(10.0F)).start();
        drawLayout.startAnimation(fabOpenAnimation);
        impressLayout.startAnimation(fabOpenAnimation);
        writerLayout.startAnimation(fabOpenAnimation);
        calcLayout.startAnimation(fabOpenAnimation);
        writerFAB.setClickable(true);
        impressFAB.setClickable(true);
        drawFAB.setClickable(true);
        calcFAB.setClickable(true);
        isFabMenuOpen = true;
    }

    private void collapseFabMenu() {
        ViewCompat.animate(editFAB).rotation(0.0F).withLayer().setDuration(300).setInterpolator(new OvershootInterpolator(10.0F)).start();
        writerLayout.startAnimation(fabCloseAnimation);
        impressLayout.startAnimation(fabCloseAnimation);
        drawLayout.startAnimation(fabCloseAnimation);
        calcLayout.startAnimation(fabCloseAnimation);
        writerFAB.setClickable(false);
        impressFAB.setClickable(false);
        drawFAB.setClickable(false);
        calcFAB.setClickable(false);
        isFabMenuOpen = false;
    }

    @Override
    public void onBackPressed() {
        if (isFabMenuOpen) {
            collapseFabMenu();
        } else {
            super.onBackPressed();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_CODE_OPEN_FILECHOOSER && resultCode == RESULT_OK) {
            final Uri fileUri = data.getData();
            openDocument(fileUri);
        } else if (requestCode == REQUEST_CODE_CREATE_NEW_DOCUMENT) {
            // "forward" to LibreOfficeMainActivity to create + open the file
            final Uri fileUri = data.getData();
            loadNewDocument(newDocType, fileUri);
        }
    }

    private void showSystemFilePickerAndOpenFile() {
        Intent intent = new Intent();
        try {
            intent.setAction(Intent.ACTION_OPEN_DOCUMENT);
        } catch (ActivityNotFoundException exception) {
            // Intent.ACTION_OPEN_DOCUMENT added in API level 19, but minSdkVersion is currently 16
            intent.setAction(Intent.ACTION_GET_CONTENT);
        }

        intent.setType("*/*");
        intent.putExtra(Intent.EXTRA_MIME_TYPES, SUPPORTED_MIME_TYPES);;

        if (intent.resolveActivity(getPackageManager()) != null) {
            startActivityForResult(intent, REQUEST_CODE_OPEN_FILECHOOSER);
        }
    }

    public void openDocument(final Uri documentUri) {
        // "forward" to LibreOfficeMainActivity to open the file
        Intent intent = new Intent(Intent.ACTION_VIEW, documentUri);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

        addDocumentToRecents(documentUri);

        String packageName = getApplicationContext().getPackageName();
        ComponentName componentName = new ComponentName(packageName,
            LibreOfficeMainActivity.class.getName());
        intent.setComponent(componentName);
        startActivity(intent);
    }

    private void createNewFileDialog() {
        final String extension;
        if (newDocType == DocumentType.WRITER) {
            extension = FileUtilities.DEFAULT_WRITER_EXTENSION;
        } else if (newDocType == DocumentType.CALC) {
            extension = FileUtilities.DEFAULT_SPREADSHEET_EXTENSION;
        } else if (newDocType == DocumentType.IMPRESS) {
            extension = FileUtilities.DEFAULT_IMPRESS_EXTENSION;
        } else if (newDocType == DocumentType.DRAW) {
            extension = FileUtilities.DEFAULT_DRAWING_EXTENSION;
        } else {
            Log.e(LOGTAG, "Invalid document type passed.");
            return;
        }

        String defaultFileName = getString(R.string.default_document_name) + extension;
        String mimeType = FileUtilities.getMimeType(defaultFileName);

        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType(mimeType);
        intent.putExtra(Intent.EXTRA_TITLE, defaultFileName);

        startActivityForResult(intent, REQUEST_CODE_CREATE_NEW_DOCUMENT);
    }

    private void loadNewDocument(DocumentType docType, Uri newFileUri) {
        final String newDocumentType;
        if (docType == DocumentType.WRITER) {
            newDocumentType = NEW_WRITER_STRING_KEY;
        } else if (docType == DocumentType.CALC) {
            newDocumentType = NEW_CALC_STRING_KEY;
        } else if (docType == DocumentType.IMPRESS) {
            newDocumentType = NEW_IMPRESS_STRING_KEY;
        } else if (docType == DocumentType.DRAW) {
            newDocumentType = NEW_DRAW_STRING_KEY;
        } else {
            Log.w(LOGTAG, "invalid document type passed to loadNewDocument method. Ignoring request");
            return;
        }

        Intent intent = new Intent(LibreOfficeUIActivity.this, LibreOfficeMainActivity.class);
        intent.putExtra(NEW_DOC_TYPE_KEY, newDocumentType);
        intent.setData(newFileUri);
        startActivity(intent);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.view_menu, menu);

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_about: {
                AboutDialogFragment aboutDialogFragment = new AboutDialogFragment();
                aboutDialogFragment.show(getSupportFragmentManager(), "AboutDialogFragment");
            }
                return true;
            case R.id.action_settings:
                startActivity(new Intent(getApplicationContext(), SettingsActivity.class));
                return true;

            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public void readPreferences(){
        SharedPreferences defaultPrefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        final String displayLanguage = defaultPrefs.getString(DISPLAY_LANGUAGE, LocaleHelper.SYSTEM_DEFAULT_LANGUAGE);
        LocaleHelper.setLocale(this, displayLanguage);
    }

    @Override
    public void settingsPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        readPreferences();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(LOGTAG, "onResume");
        createUI();
    }

    private void addDocumentToRecents(Uri fileUri) {
        SharedPreferences prefs = getSharedPreferences(EXPLORER_PREFS_KEY, MODE_PRIVATE);
        if (Build.VERSION.SDK_INT < 19) {
            // ContentResolver#takePersistableUriPermission only available from SDK level 19 on
            Log.i(LOGTAG, "Recently used files not supported, requires SDK version >= 19.");
            // drop potential entries
            prefs.edit().putString(RECENT_DOCUMENTS_KEY, "").apply();
            return;
        }

        // preserve permissions across device reboots,
        // s. https://developer.android.com/training/data-storage/shared/documents-files#persist-permissions
        getContentResolver().takePersistableUriPermission(fileUri, Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

        String newRecent = fileUri.toString();
        List<String> recentsList = new ArrayList(Arrays.asList(prefs.getString(RECENT_DOCUMENTS_KEY, "").split(RECENT_DOCUMENTS_DELIMITER)));

        // remove string if present, so that it doesn't appear multiple times
        recentsList.remove(newRecent);

        // put the new value in the first place
        recentsList.add(0, newRecent);

        /*
         * 4 because the number of recommended items in App Shortcuts is 4, and also
         * because it's a good number of recent items in general
         */
        final int RECENTS_SIZE = 4;

        while (recentsList.size() > RECENTS_SIZE) {
            recentsList.remove(RECENTS_SIZE);
        }

        // serialize to String that can be set for pref
        String value = TextUtils.join(RECENT_DOCUMENTS_DELIMITER, recentsList);
        prefs.edit().putString(RECENT_DOCUMENTS_KEY, value).apply();

        //update app shortcuts (7.0 and above)
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N_MR1) {
            ShortcutManager shortcutManager = getSystemService(ShortcutManager.class);

            //Remove all shortcuts, and apply new ones.
            shortcutManager.removeAllDynamicShortcuts();

            ArrayList<ShortcutInfo> shortcuts = new ArrayList<ShortcutInfo>();
            for (String recentDoc : recentsList) {
                Uri docUri = Uri.parse(recentDoc);
                String filename = FileUtilities.retrieveDisplayNameForDocumentUri(getContentResolver(), docUri);
                if (filename.isEmpty()) {
                    continue;
                }

                //find the appropriate drawable
                int drawable = 0;
                switch (FileUtilities.getType(filename)) {
                    case FileUtilities.DOC:
                        drawable = R.drawable.writer;
                        break;
                    case FileUtilities.CALC:
                        drawable = R.drawable.calc;
                        break;
                    case FileUtilities.DRAWING:
                        drawable = R.drawable.draw;
                        break;
                    case FileUtilities.IMPRESS:
                        drawable = R.drawable.impress;
                        break;
                }

                Intent intent = new Intent(Intent.ACTION_VIEW, docUri);
                String packageName = this.getApplicationContext().getPackageName();
                ComponentName componentName = new ComponentName(packageName, LibreOfficeMainActivity.class.getName());
                intent.setComponent(componentName);

                ShortcutInfo shortcut = new ShortcutInfo.Builder(this, filename)
                        .setShortLabel(filename)
                        .setLongLabel(filename)
                        .setIcon(Icon.createWithResource(this, drawable))
                        .setIntent(intent)
                        .build();

                shortcuts.add(shortcut);
            }
            shortcutManager.setDynamicShortcuts(shortcuts);
        }
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        switch (id){
            case R.id.editFAB:
                // Intent.ACTION_CREATE_DOCUMENT, used in 'createNewFileDialog' requires SDK version 19
                if (Build.VERSION.SDK_INT < 19) {
                    Toast.makeText(this,
                        getString(R.string.creating_new_files_not_supported), Toast.LENGTH_SHORT).show();
                    return;
                }
                if (isFabMenuOpen) {
                    collapseFabMenu();
                } else {
                    expandFabMenu();
                }
                break;
            case R.id.open_file_view:
                showSystemFilePickerAndOpenFile();
                break;
            case R.id.newWriterFAB:
                newDocType = DocumentType.WRITER;
                createNewFileDialog();
                break;
            case R.id.newImpressFAB:
                newDocType = DocumentType.IMPRESS;
                createNewFileDialog();
                break;
            case R.id.newCalcFAB:
                newDocType = DocumentType.CALC;
                createNewFileDialog();
                break;
            case R.id.newDrawFAB:
                newDocType = DocumentType.DRAW;
                createNewFileDialog();
                break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
