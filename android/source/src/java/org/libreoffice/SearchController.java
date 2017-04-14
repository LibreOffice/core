package org.libreoffice;

import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

class SearchController implements View.OnClickListener, View.OnLongClickListener{
    private static String LOGTAG = SearchController.class.getSimpleName();
    private LibreOfficeMainActivity mActivity;

    private int hits = 0;

    /**
     * Called when a view has been clicked and held.
     *
     * @param view The view that was clicked and held.
     * @return true if the callback consumed the long click, false otherwise.
     */
    @Override
    public boolean onLongClick(View view) {

        final ImageButton button = (ImageButton) view;

        switch(button.getId()) {
            case R.id.button_search_down:
                if (hits == 0) {
                    hits = 1;
                    Log.d(LOGTAG, "01");
                } else if (hits == 1) {
                    hits = 0;
                    Log.d(LOGTAG, "10");
                    LOKitShell.getMainHandler().post(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(mActivity, "Back to normal...", Toast.LENGTH_SHORT).show();
                        }
                    });
                }
                break;
            case R.id.button_search_up:
                if (hits == 1) {
                    hits = 2;
                    Log.d(LOGTAG, "12");
                    LOKitShell.getMainHandler().post(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(mActivity,
                                    "Now you have super power!\n" +
                                    "Type 'SelectAll' and search.", Toast.LENGTH_LONG).show();
                        }
                    });
                } else if (hits == 2) {
                    hits = 1;
                    Log.d(LOGTAG, "21");
                }
                break;
            default:
                break;
        }

        return true;
    }

    private void searchUNO(String searchText) {
        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:" + searchText));
    }

    private enum SearchDirection {
        UP, DOWN
    }

    SearchController(LibreOfficeMainActivity activity) {
        mActivity = activity;

        activity.findViewById(R.id.button_search_up).setOnClickListener(this);
        activity.findViewById(R.id.button_search_down).setOnClickListener(this);
        activity.findViewById(R.id.button_search_up).setOnLongClickListener(this);
        activity.findViewById(R.id.button_search_down).setOnLongClickListener(this);
    }

    private void search(String searchString, SearchDirection direction, float x, float y) {
        try {
            JSONObject rootJson = new JSONObject();

            addProperty(rootJson, "SearchItem.SearchString", "string", searchString);
            addProperty(rootJson, "SearchItem.Backward", "boolean", direction == SearchDirection.DOWN ? "true" : "false");
            addProperty(rootJson, "SearchItem.SearchStartPointX", "long", String.valueOf((long) UnitConverter.pixelToTwip(x, LOKitShell.getDpi(mActivity))));
            addProperty(rootJson, "SearchItem.SearchStartPointY", "long", String.valueOf((long) UnitConverter.pixelToTwip(y, LOKitShell.getDpi(mActivity))));
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
        if (hits == 2) {
            searchUNO(searchText);
        } else {
            search(searchText, direction, x, y);
        }
    }
}
