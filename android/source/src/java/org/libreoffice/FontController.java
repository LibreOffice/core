package org.libreoffice;

import android.graphics.Color;
import android.graphics.Rect;
import com.google.android.material.bottomsheet.BottomSheetBehavior;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Spinner;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

public class FontController implements AdapterView.OnItemSelectedListener {

    /** -1 as value in ".uno:Color" et al. means "automatic color"/no color set. */
    private static final int COLOR_AUTO = -1;

    private boolean mFontNameSpinnerSet = false;
    private boolean mFontSizeSpinnerSet = false;
    private final LibreOfficeMainActivity mActivity;
    private final ArrayList<String> mFontList = new ArrayList<>();
    private final ArrayList<String> mFontSizes = new ArrayList<>();

    private String mCurrentFontSelected = null;
    private String mCurrentFontSizeSelected = null;

    public FontController(LibreOfficeMainActivity activity) {
        mActivity = activity;
    }
    private BottomSheetBehavior colorPickerBehavior;
    private BottomSheetBehavior backColorPickerBehavior;
    private BottomSheetBehavior toolBarBottomBehavior;
    private ColorPickerAdapter colorPickerAdapter;
    private ColorPickerAdapter backColorPickerAdapter;

    final ColorPaletteListener colorPaletteListener = new ColorPaletteListener() {
        @Override
        public void applyColor(int color) {
            sendFontColorChange(color, false);
        }

        @Override
        public void updateColorPickerPosition(int color) {
            if (colorPickerAdapter == null) {
                return;
            }
            if (color == COLOR_AUTO) {
                colorPickerAdapter.unselectColors();
                changeFontColorBoxColor(Color.TRANSPARENT);
                return;
            }
            final int colorWithAlpha = color | 0xFF000000;
            colorPickerAdapter.findSelectedTextColor(colorWithAlpha);
            changeFontColorBoxColor(colorWithAlpha);
        }
    };

    final ColorPaletteListener backColorPaletteListener = new ColorPaletteListener() {
        @Override
        public void applyColor(int color) {
            sendFontBackColorChange(color, false);
        }

        @Override
        public void updateColorPickerPosition(int color) {
            if (backColorPickerAdapter == null) {
                return;
            }
            if (color == COLOR_AUTO) {
                backColorPickerAdapter.unselectColors();
                changeFontBackColorBoxColor(Color.TRANSPARENT);
                return;
            }
            final int colorWithAlpha = color | 0xFF000000;
            backColorPickerAdapter.findSelectedTextColor(colorWithAlpha);
            changeFontBackColorBoxColor(colorWithAlpha);
        }
    };

    private void changeFontColorBoxColor(final int color){
        final ImageButton fontColorPickerButton = mActivity.findViewById(R.id.font_color_picker_button);

        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                fontColorPickerButton.setBackgroundColor(color);
            }
        });
    }

    private void changeFontBackColorBoxColor(final int color){
        final ImageButton fontBackColorPickerButton = mActivity.findViewById(R.id.font_back_color_picker_button);

        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                fontBackColorPickerButton.setBackgroundColor(color);
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

    private void sendFontColorChange(int color, boolean keepAlpha){
        try {
            JSONObject json = new JSONObject();
            JSONObject valueJson = new JSONObject();
            valueJson.put("type", "long");
            valueJson.put("value", keepAlpha ? color : 0x00FFFFFF & color);
            json.put("Color", valueJson);

            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Color", json.toString()));
            changeFontColorBoxColor(color);

        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    /*
     * 0x00FFFFFF & color operation removes the alpha which is FF,
     * if we don't remove it, the color value becomes negative which is not recognized by LOK
     */
    private void sendFontBackColorChange(int color, boolean keepAlpha) {
        try {
            JSONObject json = new JSONObject();
            JSONObject valueJson = new JSONObject();
            valueJson.put("type", "long");
            valueJson.put("value", keepAlpha ? color : 0x00FFFFFF & color);
            if(mActivity.getTileProvider().isSpreadsheet()){
                json.put("BackgroundColor", valueJson);
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:BackgroundColor", json.toString()));
            }else {
                json.put("CharBackColor", valueJson);
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:CharBackColor", json.toString()));
            }

            changeFontBackColorBoxColor(color);

        } catch (JSONException e) {
            e.printStackTrace();
        }
    }


    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
        if (mFontList.isEmpty() || !mFontNameSpinnerSet)
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
        mFontList.clear();
        mFontSizes.clear();
        try {
            JSONObject jObject = new JSONObject(json);
            final JSONArray fontNameArray = jObject.getJSONArray("FontNames");
            for (int i = 0; i < fontNameArray.length(); i++) {
                mFontList.add(fontNameArray.getString(i));
            }

            final JSONArray fontSizeArray = jObject.getJSONArray("FontSizes");
            for (int i = 0; i < fontSizeArray.length(); i++) {
                mFontSizes.add(fontSizeArray.getString(i));
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
                setupBackColorPicker();
            }
        });
    }

    private void setupFontNameSpinner() {
        Spinner fontSpinner = mActivity.findViewById(R.id.font_name_spinner);
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<>(mActivity, android.R.layout.simple_spinner_item, mFontList);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        fontSpinner.setAdapter(dataAdapter);
    }

    private void setupFontSizeSpinner() {
        Spinner fontSizeSpinner = mActivity.findViewById(R.id.font_size_spinner);
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<>(mActivity, android.R.layout.simple_spinner_item, mFontSizes);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        fontSizeSpinner.setAdapter(dataAdapter);
    }

    private void setupColorPicker(){
        LinearLayout colorPickerLayout = mActivity.findViewById(R.id.toolbar_color_picker);

        RecyclerView recyclerView = colorPickerLayout.findViewById(R.id.fontColorView);
        GridLayoutManager gridLayoutManager = new GridLayoutManager(mActivity, 11, GridLayoutManager.VERTICAL, true);
        recyclerView.setHasFixedSize(true);
        recyclerView.setLayoutManager(gridLayoutManager);



        RecyclerView recyclerView2 = colorPickerLayout.findViewById(R.id.fontColorViewSub);
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
        LinearLayout toolbarBottomLayout = mActivity.findViewById(R.id.toolbar_bottom);
        colorPickerBehavior = BottomSheetBehavior.from(colorPickerLayout);
        toolBarBottomBehavior = BottomSheetBehavior.from(toolbarBottomLayout);

        ImageButton pickerGoBackButton = colorPickerLayout.findViewById(R.id.button_go_back_color_picker);
        pickerGoBackButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                toolBarBottomBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);
                colorPickerBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
            }
        });


        fontColorPicker.setOnClickListener(clickListener);
        fontColorPickerButton.setOnClickListener(clickListener);

        final Button autoColorButton = colorPickerLayout.findViewById(R.id.button_auto_color);
        autoColorButton.setOnClickListener(view -> {
            sendFontColorChange(COLOR_AUTO, true);
        });
    }

    private void setupBackColorPicker(){
        LinearLayout backColorPickerLayout = mActivity.findViewById(R.id.toolbar_back_color_picker);

        RecyclerView recyclerView = backColorPickerLayout.findViewById(R.id.fontColorView);
        GridLayoutManager gridLayoutManager = new GridLayoutManager(mActivity, 11, GridLayoutManager.VERTICAL, true);
        recyclerView.setHasFixedSize(true);
        recyclerView.setLayoutManager(gridLayoutManager);



        RecyclerView recyclerView2 = backColorPickerLayout.findViewById(R.id.fontColorViewSub);
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

        ColorPaletteAdapter colorPaletteAdapter = new ColorPaletteAdapter(mActivity, backColorPaletteListener);
        recyclerView2.setAdapter(colorPaletteAdapter);

        this.backColorPickerAdapter = new ColorPickerAdapter(mActivity, colorPaletteAdapter, backColorPaletteListener);
        recyclerView.setAdapter(backColorPickerAdapter);
        RelativeLayout fontColorPicker = mActivity.findViewById(R.id.font_back_color_picker);
        ImageButton fontColorPickerButton = mActivity.findViewById(R.id.font_back_color_picker_button);
        View.OnClickListener clickListener = new View.OnClickListener(){
            @Override
            public void onClick(View view) {
                toolBarBottomBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
                backColorPickerBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);
                mActivity.findViewById(R.id.search_toolbar).setVisibility(View.GONE);
            }
        };
        LinearLayout toolbarBottomLayout = mActivity.findViewById(R.id.toolbar_bottom);
        backColorPickerBehavior = BottomSheetBehavior.from(backColorPickerLayout);
        toolBarBottomBehavior = BottomSheetBehavior.from(toolbarBottomLayout);

        ImageButton pickerGoBackButton = backColorPickerLayout.findViewById(R.id.button_go_back_color_picker);
        pickerGoBackButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                toolBarBottomBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);
                backColorPickerBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
            }
        });


        fontColorPicker.setOnClickListener(clickListener);
        fontColorPickerButton.setOnClickListener(clickListener);

        final Button autoColorButton = backColorPickerLayout.findViewById(R.id.button_auto_color);
        autoColorButton.setOnClickListener(view -> {
            sendFontBackColorChange(COLOR_AUTO, true);
        });
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

        int position = mFontList.indexOf(fontName);
        if (position != -1) {
            mCurrentFontSelected = fontName;
            spinner.setSelection(position,false);
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

        int position = mFontSizes.indexOf(fontSize);
        if (position != -1) {
            mCurrentFontSizeSelected = fontSize;
            spinner.setSelection(position, false);
        }
    }
}
