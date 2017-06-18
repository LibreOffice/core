package org.libreoffice.overlay;

import android.graphics.RectF;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.libreoffice.LOEvent;
import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.R;
import org.mozilla.gecko.gfx.LayerView;

import java.util.ArrayList;

public class CalcHeadersController {
    private static final String LOGTAG = CalcHeadersController.class.getSimpleName();

    private final CalcHeadersView mCalcRowHeadersView;
    private final CalcHeadersView mCalcColumnHeadersView;

    private LibreOfficeMainActivity mContext;

    public CalcHeadersController(LibreOfficeMainActivity context, LayerView layerView) {
        mContext = context;
        mContext.getDocumentOverlay().setCalcHeadersController(this);
        mCalcRowHeadersView = (CalcHeadersView) context.findViewById(R.id.calc_header_row);
        mCalcColumnHeadersView = (CalcHeadersView) context.findViewById(R.id.calc_header_column);
        if (mCalcColumnHeadersView == null || mCalcRowHeadersView == null) {
            Log.e(LOGTAG, "Failed to initialize Calc headers - View is null");
        } else {
            mCalcRowHeadersView.initialize(layerView, true);
            mCalcColumnHeadersView.initialize(layerView, false);
        }
        LOKitShell.sendEvent(new LOEvent(LOEvent.UPDATE_CALC_HEADERS));
    }

    public void setHeaders(String headers) {
        HeaderInfo parsedHeaders = parseHeaderInfo(headers);
        if (parsedHeaders != null) {
            mCalcRowHeadersView.setHeaders(parsedHeaders.rowLabels, parsedHeaders.rowDimens);
            mCalcColumnHeadersView.setHeaders(parsedHeaders.columnLabels, parsedHeaders.columnDimens);
            showHeaders();
        } else {
            Log.e(LOGTAG, "Parse header info JSON failed.");
        }
    }

    public void showHeaders() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                mCalcColumnHeadersView.invalidate();
                mCalcRowHeadersView.invalidate();
            }
        });
    }

    private HeaderInfo parseHeaderInfo(String headers) {
        HeaderInfo headerInfo = new HeaderInfo();
        try {
            JSONObject collectiveResult = new JSONObject(headers);
            JSONArray rowResult = collectiveResult.getJSONArray("rows");
            for (int i = 0; i < rowResult.length(); i++) {
                headerInfo.rowLabels.add(rowResult.getJSONObject(i).getString("text"));
                headerInfo.rowDimens.add(twipToPixel(rowResult.getJSONObject(i).getLong("size"), LOKitShell.getDpi(mContext)));
            }
            JSONArray columnResult = collectiveResult.getJSONArray("columns");
            for (int i = 0; i < columnResult.length(); i++) {
                headerInfo.columnLabels.add(columnResult.getJSONObject(i).getString("text"));
                headerInfo.columnDimens.add(twipToPixel(columnResult.getJSONObject(i).getLong("size"), LOKitShell.getDpi(mContext)));
            }
            return headerInfo;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    private float twipToPixel(float input, float dpi) {
        return input / 1440.0f * dpi;
    }

    public void showHeaderSelection(RectF cellCursorRect) {
        mCalcRowHeadersView.setHeaderSelection(cellCursorRect);
        mCalcColumnHeadersView.setHeaderSelection(cellCursorRect);
        showHeaders();
    }

    private class HeaderInfo {
        ArrayList<String> rowLabels;
        ArrayList<Float> rowDimens;
        ArrayList<String> columnLabels;
        ArrayList<Float> columnDimens;

        private HeaderInfo() {
            rowLabels = new ArrayList<String>();
            rowDimens = new ArrayList<Float>();
            columnDimens = new ArrayList<Float>();
            columnLabels = new ArrayList<String>();
        }
    }
}
