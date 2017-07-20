package org.libreoffice.overlay;

import android.content.Context;
import android.graphics.PointF;
import android.graphics.RectF;
import android.graphics.drawable.ColorDrawable;
import android.support.design.widget.Snackbar;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.Button;
import android.widget.PopupWindow;
import android.widget.TextView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.libreoffice.LOEvent;
import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.R;
import org.mozilla.gecko.gfx.LayerView;

import java.util.ArrayList;

import static org.libreoffice.SearchController.addProperty;
import static org.libreoffice.UnitConverter.twipToPixel;

public class CalcHeadersController {
    private static final String LOGTAG = CalcHeadersController.class.getSimpleName();

    private final CalcHeadersView mCalcRowHeadersView;
    private final CalcHeadersView mCalcColumnHeadersView;

    private LibreOfficeMainActivity mContext;

    public CalcHeadersController(LibreOfficeMainActivity context, final LayerView layerView) {
        mContext = context;
        mContext.getDocumentOverlay().setCalcHeadersController(this);
        mCalcRowHeadersView = context.findViewById(R.id.calc_header_row);
        mCalcColumnHeadersView = context.findViewById(R.id.calc_header_column);
        if (mCalcColumnHeadersView == null || mCalcRowHeadersView == null) {
            Log.e(LOGTAG, "Failed to initialize Calc headers - View is null");
        } else {
            mCalcRowHeadersView.initialize(layerView, true);
            mCalcColumnHeadersView.initialize(layerView, false);
        }
        LOKitShell.sendEvent(new LOEvent(LOEvent.UPDATE_CALC_HEADERS));
        context.findViewById(R.id.calc_header_top_left).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:SelectAll"));
                if (mCalcColumnHeadersView == null) return;
                mCalcColumnHeadersView.showHeaderPopup(new PointF());
            }
        });
        ((EditText)context.findViewById(R.id.calc_address)).setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE || actionId == EditorInfo.IME_ACTION_GO) {
                    String text = v.getText().toString();
                    JSONObject rootJson = new JSONObject();
                    try {
                        addProperty(rootJson, "ToPoint", "string", text);
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                    LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:GoToCell", rootJson.toString()));
                    mContext.hideSoftKeyboard();
                    layerView.requestFocus();
                }
                return true;
            }
        });
        ((EditText)context.findViewById(R.id.calc_formula)).setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE || actionId == EditorInfo.IME_ACTION_GO) {
                    String text = v.getText().toString();
                    JSONObject rootJson = new JSONObject();
                    try {
                        addProperty(rootJson, "StringName", "string", text);
                        addProperty(rootJson, "DontCommit", "boolean", String.valueOf(false));
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                    LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:EnterString", rootJson.toString()));
                    mContext.hideSoftKeyboard();
                    layerView.requestFocus();
                    mContext.setDocumentChanged(true);
                }
                return true;
            }
        });
        // manually select A1 for address bar and formula bar to update when calc first opens
        JSONObject rootJson = new JSONObject();
        try {
            addProperty(rootJson, "ToPoint", "string", "A1");
        } catch (JSONException e) {
            e.printStackTrace();
        }
        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:GoToCell", rootJson.toString()));
    }

    public void setupHeaderPopupView() {
        LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        String[] rowOrColumn = {"Row","Column"};
        CalcHeadersView[] headersViews= {mCalcRowHeadersView, mCalcColumnHeadersView};
        for (int i = 0; i < rowOrColumn.length; i++) {
            // create popup window
            final String tempName = rowOrColumn[i];
            final CalcHeadersView tempView = headersViews[i];
            final View headerPopupView = inflater.inflate(R.layout.calc_header_popup, null);
            final PopupWindow popupWindow = new PopupWindow(headerPopupView, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
            popupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
                @Override
                public void onDismiss() {
                    headerPopupView.findViewById(R.id.calc_header_popup_optimal_length_dialog).setVisibility(View.GONE);
                    popupWindow.setFocusable(false);
                }
            });
            popupWindow.setOutsideTouchable(true);
            popupWindow.setBackgroundDrawable(new ColorDrawable());
            popupWindow.setAnimationStyle(android.R.style.Animation_Dialog);
            tempView.setHeaderPopupWindow(popupWindow);
            // set up child views in the popup window
            headerPopupView.findViewById(R.id.calc_header_popup_insert).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Insert"+tempName+"s"));
                    tempView.dismissPopupWindow();
                    mContext.setDocumentChanged(true);
                }
            });
            headerPopupView.findViewById(R.id.calc_header_popup_delete).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Delete"+tempName+"s"));
                    tempView.dismissPopupWindow();
                    mContext.setDocumentChanged(true);
                }
            });
            headerPopupView.findViewById(R.id.calc_header_popup_hide).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Hide"+tempName));
                    tempView.dismissPopupWindow();
                    mContext.setDocumentChanged(true);
                }
            });
            headerPopupView.findViewById(R.id.calc_header_popup_show).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Show"+tempName));
                    tempView.dismissPopupWindow();
                    mContext.setDocumentChanged(true);
                }
            });
            headerPopupView.findViewById(R.id.calc_header_popup_optimal_length).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    View view = headerPopupView.findViewById(R.id.calc_header_popup_optimal_length_dialog);
                    if (view.getVisibility() == View.VISIBLE) {
                        view.setVisibility(View.GONE);
                        popupWindow.setFocusable(false);
                        popupWindow.update();
                    } else {
                        popupWindow.dismiss();
                        view.setVisibility(View.VISIBLE);
                        popupWindow.setFocusable(true);
                        popupWindow.showAtLocation(tempView, Gravity.CENTER, 0, 0);
                        LOKitShell.getMainHandler().post(new Runnable() {
                            @Override
                            public void run() {
                                Snackbar.make(tempView, R.string.calc_alert_double_click_optimal_length, Snackbar.LENGTH_LONG).show();
                            }
                        });
                    }
                }
            });
            headerPopupView.findViewById(R.id.calc_header_popup_optimal_length_button).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    String text = ((EditText)headerPopupView.findViewById(R.id.calc_header_popup_optimal_length_text)).getText().toString();
                    tempView.sendOptimalLengthRequest(text);
                    tempView.dismissPopupWindow();
                    mContext.setDocumentChanged(true);
                }
            });
            headerPopupView.findViewById(R.id.calc_header_popup_adjust_length).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mContext.getDocumentOverlay().showAdjustLengthLine(tempView == mCalcRowHeadersView, tempView);
                    tempView.dismissPopupWindow();
                    mContext.setDocumentChanged(true);
                }
            });
            ((Button)headerPopupView.findViewById(R.id.calc_header_popup_adjust_length))
                    .setText(tempView == mCalcRowHeadersView ? R.string.calc_adjust_height : R.string.calc_adjust_width);
            ((Button)headerPopupView.findViewById(R.id.calc_header_popup_optimal_length))
                    .setText(tempView == mCalcRowHeadersView ? R.string.calc_optimal_height : R.string.calc_optimal_width);

        }
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

    public void showHeaderSelection(RectF cellCursorRect) {
        mCalcRowHeadersView.setHeaderSelection(cellCursorRect);
        mCalcColumnHeadersView.setHeaderSelection(cellCursorRect);
        showHeaders();
    }

    public void setPendingRowOrColumnSelectionToShowUp(boolean b) {
        mCalcRowHeadersView.setPendingRowOrColumnSelectionToShowUp(b);
        mCalcColumnHeadersView.setPendingRowOrColumnSelectionToShowUp(b);
    }

    public boolean pendingRowOrColumnSelectionToShowUp() {
        return mCalcColumnHeadersView.pendingRowOrColumnSelectionToShowUp()
                || mCalcRowHeadersView.pendingRowOrColumnSelectionToShowUp();
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
