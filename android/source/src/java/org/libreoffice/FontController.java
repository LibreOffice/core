package org.libreoffice;

import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

public class FontController implements AdapterView.OnItemSelectedListener {

    private boolean mFontNameSpinnerSet = false;
    private boolean mFontSizeSpinnerSet = false;
    private Activity mActivity;
    private List<String> mFontList = null;
    private List<String> mFontSizes = new ArrayList<String>();
    private Map<String, List<String>> mAllFontSizes = null;

    private String mCurrentFontSelected = null;
    private String mCurrentFontSizeSelected = null;

    public FontController(Activity activity) {
        mActivity = activity;
    }

    private void sendFontChange(String fontName) {
        try {
            JSONObject json = new JSONObject();
            JSONObject valueJson = new JSONObject();
            valueJson.put("type", "string");
            valueJson.put("value", fontName);
            json.put("CharFontName.FamilyName", valueJson);

            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:CharFontName", json.toString()));

        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void sendFontSizeChange(String fontSize) {
        try {
            JSONObject json = new JSONObject();
            JSONObject valueJson = new JSONObject();
            valueJson.put("type", "float");
            valueJson.put("value", fontSize);
            json.put("FontHeight.Height", valueJson);

            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:FontHeight", json.toString()));

        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
        if (mFontList == null || !mFontNameSpinnerSet)
            return;
        if (parent == mActivity.findViewById(R.id.font_name_spinner)) {
            String currentFontSelected = parent.getItemAtPosition(pos).toString();
            if (!currentFontSelected.equals(mCurrentFontSelected)) {
                mCurrentFontSelected = currentFontSelected;
                sendFontChange(mCurrentFontSelected);
            }
        } else if (parent == mActivity.findViewById(R.id.font_size_spinner)) {
            String currentFontSizeSelected = parent.getItemAtPosition(pos).toString();
            if (!currentFontSizeSelected.equals(mCurrentFontSizeSelected)) {
                mCurrentFontSizeSelected = currentFontSizeSelected;
                sendFontSizeChange(mCurrentFontSizeSelected);
            }
        }
    }

    @Override
    public void onNothingSelected(AdapterView parent) {
        // Do nothing.
    }

    public void parseJson(String json) {
        mFontList = new ArrayList<String>();
        mAllFontSizes = new HashMap<String, List<String>>();
        try {
            JSONObject jObject = new JSONObject(json);
            JSONObject jObject2 = jObject.getJSONObject("commandValues");
            Iterator<String> keys = jObject2.keys();
            List<String> fontSizes;
            while (keys.hasNext()) {
                String key = keys.next();
                mFontList.add(key);
                JSONArray array = jObject2.getJSONArray(key);
                fontSizes = new ArrayList<String>();
                for (int i = 0; i < array.length(); i++) {
                    fontSizes.add(array.getString(i));
                }
                mAllFontSizes.put(key, fontSizes);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void setupFontViews() {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                setupFontNameSpinner();
                setupFontSizeSpinner();
            }
        });
    }

    private void setupFontNameSpinner() {
        Spinner fontSpinner = (Spinner) mActivity.findViewById(R.id.font_name_spinner);
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(mActivity, android.R.layout.simple_spinner_item, mFontList);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        fontSpinner.setAdapter(dataAdapter);
    }

    private void setupFontSizeSpinner() {
        Spinner fontSizeSpinner = (Spinner) mActivity.findViewById(R.id.font_size_spinner);
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(mActivity, android.R.layout.simple_spinner_item, mFontSizes);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        fontSizeSpinner.setAdapter(dataAdapter);
    }

    public void selectFont(final String fontName) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                selectFontCurrentThread(fontName);
            }
        });
    }

    private void selectFontCurrentThread(String fontName) {
        Spinner spinner = (Spinner) mActivity.findViewById(R.id.font_name_spinner);

        if (!mFontNameSpinnerSet) {
            spinner.setOnItemSelectedListener(this);
            mFontNameSpinnerSet = true;
        }

        if (fontName.equals(mCurrentFontSelected))
            return;

        ArrayAdapter<String> arrayAdapter = (ArrayAdapter<String>) spinner.getAdapter();
        int position = arrayAdapter.getPosition(fontName);
        if (position != -1) {
            mCurrentFontSelected = fontName;
            spinner.setSelection(position);
        }

        resetFontSizes(fontName);
    }

    private void resetFontSizes(String fontName) {
        Spinner spinner = (Spinner) mActivity.findViewById(R.id.font_size_spinner);
        ArrayAdapter<String> arrayAdapter = (ArrayAdapter<String>) spinner.getAdapter();

        List<String> fontSizes = mAllFontSizes.get(fontName);
        if (fontSizes != null) {
            arrayAdapter.clear();
            arrayAdapter.addAll(mAllFontSizes.get(fontName));
        }
    }

    public void selectFontSize(final String fontSize) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                selectFontSizeCurrentThread(fontSize);
            }
        });
    }

    private void selectFontSizeCurrentThread(String fontSize) {
        Spinner spinner = (Spinner) mActivity.findViewById(R.id.font_size_spinner);
        if (!mFontSizeSpinnerSet) {
            spinner.setOnItemSelectedListener(this);
            mFontSizeSpinnerSet = true;
        }

        if (fontSize.equals(mCurrentFontSizeSelected))
            return;

        ArrayAdapter<String> arrayAdapter = (ArrayAdapter<String>) spinner.getAdapter();

        int position = arrayAdapter.getPosition(fontSize);
        if (position != -1) {
            mCurrentFontSizeSelected = fontSize;
            spinner.setSelection(position, false);
        }

    }
}
