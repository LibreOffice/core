/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.ui;

import android.Manifest;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.graphics.drawable.Icon;
import android.net.Uri;
import android.os.Bundle;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.view.ViewCompat;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import androidx.preference.PreferenceManager;
import androidx.appcompat.widget.Toolbar;
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

import org.libreoffice.AboutDialogFragment;
import org.libreoffice.BuildConfig;
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

    private static final String LOGTAG = LibreOfficeUIActivity.class.getSimpleName();

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
            "application/vnd.openxmlformats-officedocument.presentationml.slideshow",
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

    private static final int PERMISSION_WRITE_EXTERNAL_STORAGE = 0;

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
    protected void onStart() {
        super.onStart();
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Log.i(LOGTAG, "no permission to read external storage - asking for permission");
            ActivityCompat.requestPermissions(this,
                new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                PERMISSION_WRITE_EXTERNAL_STORAGE);
        }
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
            actionBar.setIcon(R.mipmap.ic_launcher);
        }

        editFAB = findViewById(R.id.editFAB);
        editFAB.setOnClickListener(this);
        // allow creating new docs only when experimental editing is enabled
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        final boolean bEditingEnabled = BuildConfig.ALLOW_EDITING && preferences.getBoolean(LibreOfficeMainActivity.ENABLE_EXPERIMENTAL_PREFS_KEY, false);
        editFAB.setVisibility(bEditingEnabled ? View.VISIBLE : View.INVISIBLE);

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
        TextView openFileView = findViewById(R.id.open_file_button);
        openFileView.setOnClickListener(this);


        RecyclerView recentRecyclerView = findViewById(R.id.list_recent);

        SharedPreferences prefs = getSharedPreferences(EXPLORER_PREFS_KEY, MODE_PRIVATE);
        String recentPref = prefs.getString(RECENT_DOCUMENTS_KEY, "");
        String[] recentFileStrings = recentPref.split(RECENT_DOCUMENTS_DELIMITER);

        final List<RecentFile> recentFiles = new ArrayList<>();
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
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_OPEN_FILECHOOSER && resultCode == RESULT_OK) {
            final Uri fileUri = data.getData();
            openDocument(fileUri);
        }
    }

    private void showSystemFilePickerAndOpenFile() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.setType("*/*");
        intent.putExtra(Intent.EXTRA_MIME_TYPES, SUPPORTED_MIME_TYPES);

        try {
            startActivityForResult(intent, REQUEST_CODE_OPEN_FILECHOOSER);
        } catch (ActivityNotFoundException e) {
            Log.w(LOGTAG, "No activity available that can handle the intent to open a document.");
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

    private void loadNewDocument(DocumentType docType) {
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
        final int itemId = item.getItemId();
        if (itemId == R.id.action_about) {
            AboutDialogFragment aboutDialogFragment = new AboutDialogFragment();
            aboutDialogFragment.show(getSupportFragmentManager(), "AboutDialogFragment");
            return true;
        }
        if (itemId == R.id.action_settings) {
            startActivity(new Intent(getApplicationContext(), SettingsActivity.class));
            return true;
        }

        return super.onOptionsItemSelected(item);
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

        // preserve permissions across device reboots,
        // s. https://developer.android.com/training/data-storage/shared/documents-files#persist-permissions
        getContentResolver().takePersistableUriPermission(fileUri, Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

        String newRecent = fileUri.toString();
        List<String> recentsList = new ArrayList<>(Arrays.asList(prefs.getString(RECENT_DOCUMENTS_KEY, "").split(RECENT_DOCUMENTS_DELIMITER)));

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

            ArrayList<ShortcutInfo> shortcuts = new ArrayList<>();
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
        if (id == R.id.editFAB) {
            if (isFabMenuOpen) {
                collapseFabMenu();
            } else {
                expandFabMenu();
            }
        } else if (id == R.id.open_file_button) {
            showSystemFilePickerAndOpenFile();
        } else if (id == R.id.newWriterFAB) {
            loadNewDocument(DocumentType.WRITER);
        } else if (id == R.id.newImpressFAB) {
            loadNewDocument(DocumentType.IMPRESS);
        } else if (id == R.id.newCalcFAB) {
            loadNewDocument(DocumentType.CALC);
        } else if (id == R.id.newDrawFAB) {
            loadNewDocument(DocumentType.DRAW);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
