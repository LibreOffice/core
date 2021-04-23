package org.libreoffice;

import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;

import org.json.JSONException;
import org.json.JSONObject;

public class SearchController implements View.OnClickListener {
    private final LibreOfficeMainActivity mActivity;

    private enum SearchDirection {
        UP, DOWN
    }

    SearchController(LibreOfficeMainActivity activity) {
        mActivity = activity;

        activity.findViewById(R.id.button_search_up).setOnClickListener(this);
        activity.findViewById(R.id.button_search_down).setOnClickListener(this);

        ((EditText) mActivity.findViewById(R.id.search_string)).setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_SEARCH) {
                    // search downward when the "search button" on keyboard is pressed,
                    SearchDirection direction = SearchDirection.DOWN;
                    String searchText = ((EditText) mActivity.findViewById(R.id.search_string)).getText().toString();
                    float x = mActivity.getCurrentCursorPosition().centerX();
                    float y = mActivity.getCurrentCursorPosition().centerY();
                    search(searchText, direction, x, y);
                    return true;
                }
                return false;
            }
        });
    }

    private void search(String searchString, SearchDirection direction, float x, float y) {
        try {
            JSONObject rootJson = new JSONObject();

            addProperty(rootJson, "SearchItem.SearchString", "string", searchString);
            addProperty(rootJson, "SearchItem.Backward", "boolean", direction == SearchDirection.UP ? "true" : "false");
            addProperty(rootJson, "SearchItem.SearchStartPointX", "long", String.valueOf((long) UnitConverter.pixelToTwip(x, LOKitShell.getDpi(mActivity))));
            addProperty(rootJson, "SearchItem.SearchStartPointY", "long", String.valueOf((long) UnitConverter.pixelToTwip(y, LOKitShell.getDpi(mActivity))));
            addProperty(rootJson, "SearchItem.Command", "long", String.valueOf(0)); // search all == 1

            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:ExecuteSearch", rootJson.toString()));

        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public static void addProperty(JSONObject json, String parentValue, String type, String value) throws JSONException {
        JSONObject child = new JSONObject();
        child.put("type", type);
        child.put("value", value);
        json.put(parentValue, child);
    }

    @Override
    public void onClick(View view) {
        ImageButton button = (ImageButton) view;

        SearchDirection direction = SearchDirection.DOWN;
        switch(button.getId()) {
            case R.id.button_search_down:
                direction = SearchDirection.DOWN;
                break;
            case R.id.button_search_up:
                direction = SearchDirection.UP;
                break;
            default:
                break;
        }

        String searchText = ((EditText) mActivity.findViewById(R.id.search_string)).getText().toString();

        float x = mActivity.getCurrentCursorPosition().centerX();
        float y = mActivity.getCurrentCursorPosition().centerY();
        search(searchText, direction, x, y);
    }
}
