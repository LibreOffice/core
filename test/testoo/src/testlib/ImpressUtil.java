/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testlib;

import org.openoffice.test.vcl.widgets.VclWindow;

import static testlib.UIMap.*;
public class ImpressUtil {

    public static VclWindow getCurView() {
        VclWindow[] views = new VclWindow[]{impress, ImpressOutline, ImpressSlideSorter, ImpressHandout};
        for(VclWindow w : views) {
            if (w.exists()) {
                return w;
            }
        }

        return null;
    }
}
