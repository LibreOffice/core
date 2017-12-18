package org.libreoffice;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.Rect;
import android.support.design.widget.BottomSheetBehavior;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
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
    private BottomSheetBehavior colorPickerBehavior;
    private BottomSheetBehavior toolBarBottomBehavior;
    private ColorPickerAdapter colorPickerAdapter;

    ColorPaletteListener colorPaletteListener = new ColorPaletteListener() {
        @Override
        public void applyColor(int color) {
            sendFontColorChange(color);
        }

        @Override
        public void updateColorPickerPosition(int color) {
            if (null == colorPickerAdapter) return;
            colorPickerAdapter.findSelectedTextColor(color);
            changeFontColorBoxColor(color);
        }
    };

    private void changeFontColorBoxColor(final int color){
        final ImageButton fontColorPickerButton = mActivity.findViewById(R.id.font_color_picker_button);

        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                if(color == -1){ //Libreoffice recognizes -1 as black
                    fontColorPickerButton.setBackgroundColor(Color.BLACK);
                }else{
                    fontColorPickerButton.setBackgroundColor(color);

                }
            }
        });
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

    private void sendFontColorChange(int color){
        try {
            JSONObject json = new JSONObject();
            JSONObject valueJson = new JSONObject();
            valueJson.put("type", "long");
            valueJson.put("value", color);
            json.put("Color", valueJson);

            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Color", json.toString()));
            changeFontColorBoxColor(color);

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
                setupColorPicker();
            }
        });
    }

    private void setupFontNameSpinner() {
        Spinner fontSpinner = mActivity.findViewById(R.id.font_name_spinner);
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(mActivity, android.R.layout.simple_spinner_item, mFontList);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        fontSpinner.setAdapter(dataAdapter);
    }

    private void setupFontSizeSpinner() {
        Spinner fontSizeSpinner = mActivity.findViewById(R.id.font_size_spinner);
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(mActivity, android.R.layout.simple_spinner_item, mFontSizes);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        fontSizeSpinner.setAdapter(dataAdapter);
    }

    private void setupColorPicker(){
        RecyclerView recyclerView = mActivity.findViewById(R.id.fontColorView);
        GridLayoutManager gridLayoutManager = new GridLayoutManager(mActivity, 11, GridLayoutManager.VERTICAL, true);
        recyclerView.setHasFixedSize(true);
        recyclerView.setLayoutManager(gridLayoutManager);



        RecyclerView recyclerView2 = mActivity.findViewById(R.id.fontColorViewSub);
        GridLayoutManager gridLayoutManager2 = new GridLayoutManager(mActivity,4);
        recyclerView2.setHasFixedSize(true);
        recyclerView2.addItemDecoration(new RecyclerView.ItemDecoration() {

            @Override
            public void getItemOffsets(Rect outRect, View view, RecyclerView parent, RecyclerView.State state) {
                outRect.bottom = 3;
                outRect.top = 3;
                outRect.left = 3;
                outRect.right = 3;
            }
        });
        recyclerView2.setLayoutManager(gridLayoutManager2);

        ColorPaletteAdapter colorPaletteAdapter = new ColorPaletteAdapter(mActivity, colorPaletteListener);
        recyclerView2.setAdapter(colorPaletteAdapter);

        this.colorPickerAdapter = new ColorPickerAdapter(mActivity, colorPaletteAdapter, colorPaletteListener);
        recyclerView.setAdapter(colorPickerAdapter);
        RelativeLayout fontColorPicker = mActivity.findViewById(R.id.font_color_picker);
        ImageButton fontColorPickerButton = mActivity.findViewById(R.id.font_color_picker_button);
        View.OnClickListener clickListener = new View.OnClickListener(){
            @Override
            public void onClick(View view) {
                toolBarBottomBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
                colorPickerBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);
                mActivity.findViewById(R.id.search_toolbar).setVisibility(View.GONE);
            }
        };
        LinearLayout toolbarColorPicker = mActivity.findViewById(R.id.toolbar_color_picker);
        LinearLayout toolbarBottomLayout = mActivity.findViewById(R.id.toolbar_bottom);
        colorPickerBehavior = BottomSheetBehavior.from(toolbarColorPicker);
        toolBarBottomBehavior = BottomSheetBehavior.from(toolbarBottomLayout);

        ImageButton pickerGoBackButton = mActivity.findViewById(R.id.button_go_back_color_picker);
        pickerGoBackButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                toolBarBottomBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);
                colorPickerBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
            }
        });


        fontColorPicker.setOnClickListener(clickListener);
        fontColorPickerButton.setOnClickListener(clickListener);

    }

    public void selectFont(final String fontName) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                selectFontCurrentThread(fontName);
            }
        });
    }

    private void selectFontCurrentThread(String fontName) {
        Spinner spinner = mActivity.findViewById(R.id.font_name_spinner);

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
        Spinner spinner = mActivity.findViewById(R.id.font_size_spinner);
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
        Spinner spinner = mActivity.findViewById(R.id.font_size_spinner);
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
