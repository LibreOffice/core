package org.libreoffice;

import android.view.View;
import android.widget.EditText;
import android.widget.ImageButton;

import org.json.JSONException;
import org.json.JSONObject;

public class SearchController implements View.OnClickListener {
    private LibreOfficeMainActivity mActivity;

    private enum SearchDriection {
        UP, DOWN
    };

    public SearchController(LibreOfficeMainActivity activity) {
        mActivity = activity;

        ((ImageButton) activity.findViewById(R.id.button_search_up)).setOnClickListener(this);
        ((ImageButton) activity.findViewById(R.id.button_search_down)).setOnClickListener(this);
    }

    private void search(String searchString, SearchDriection direction, float x, float y) {
        try {
            JSONObject rootJson = new JSONObject();

            addProperty(rootJson, "SearchItem.SearchString", "string", searchString);
            addProperty(rootJson, "SearchItem.Backward", "boolean", direction == SearchDriection.DOWN ? "true" : "false");
            addProperty(rootJson, "SearchItem.SearchStartPointX", "long", String.valueOf((long) UnitConverter.pixelToTwip(x, LOKitShell.getDpi())));
            addProperty(rootJson, "SearchItem.SearchStartPointY", "long", String.valueOf((long) UnitConverter.pixelToTwip(y, LOKitShell.getDpi())));
            addProperty(rootJson, "SearchItem.Command", "long", String.valueOf(0)); // search all == 1

            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:ExecuteSearch", rootJson.toString()));

        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void addProperty(JSONObject json, String parentValue, String type, String value) throws JSONException {
        JSONObject child = new JSONObject();
        child.put("type", type);
        child.put("value", value);
        json.put(parentValue, child);
    }

    @Override
    public void onClick(View view) {
        ImageButton button = (ImageButton) view;

        SearchDriection direction = SearchDriection.DOWN;
        switch(button.getId()) {
            case R.id.button_search_down:
                direction = SearchDriection.DOWN;
                break;
            case R.id.button_search_up:
                direction = SearchDriection.UP;
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
