/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package util;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.datatransfer.clipboard.*;
import com.sun.star.datatransfer.*;

public class SysUtils {

  /**
   * Tries to obtain text data from clipboard if such one exists.
   * The method iterates through all 'text/plain' supported data
   * flavors and returns the first non-null String value.
   *
   * @param msf MultiserviceFactory
   * @return First found string clipboard contents or null if no
   *    text contents were found.
   * @throws com.sun.star.uno.Exception if system clipboard is not accessible.
   */
  public static String getSysClipboardText(XMultiServiceFactory msf)
        throws com.sun.star.uno.Exception {

    XClipboard xCB = UnoRuntime.queryInterface
        (XClipboard.class, msf.createInstance
        ("com.sun.star.datatransfer.clipboard.SystemClipboard"));

    XTransferable xTrans = xCB.getContents();

    DataFlavor[] dfs = xTrans.getTransferDataFlavors();

    for (int i = 0; i < dfs.length; i++) {
        if (dfs[i].MimeType.startsWith("text/plain")) {
            Object data = xTrans.getTransferData(dfs[i]);
            if (data instanceof String) {
                return (String) data;
            }
        }
    }

    return null;
  }
}
