package org.libreoffice.overlay;

import android.util.Log;

import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.R;
import org.mozilla.gecko.gfx.LayerView;

public class CalcHeadersController {
    private static final String LOGTAG = CalcHeadersController.class.getSimpleName();

    private final CalcHeadersView mCalcRowHeadersView;
    private final CalcHeadersView mCalcColumnHeadersView;

    public CalcHeadersController(LibreOfficeMainActivity context, LayerView layerView) {
        mCalcRowHeadersView = (CalcHeadersView) context.findViewById(R.id.calc_header_row);
        mCalcColumnHeadersView = (CalcHeadersView) context.findViewById(R.id.calc_header_column);
        if (mCalcColumnHeadersView == null || mCalcRowHeadersView == null) {
            Log.e(LOGTAG, "Failed to initialize Calc headers - View is null");
        } else {
            mCalcRowHeadersView.initialize(layerView, true);
            mCalcColumnHeadersView.initialize(layerView, false);
            mCalcRowHeadersView.invalidate();
            mCalcColumnHeadersView.invalidate();
        }
    }
}
