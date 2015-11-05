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
import com.sun.star.uno.XInterface;
import com.sun.star.text.XTextTablesSupplier;
import com.sun.star.text.XTextFramesSupplier;
import com.sun.star.text.XTextSectionsSupplier;
import com.sun.star.text.XFootnotesSupplier;
import com.sun.star.text.XBookmarksSupplier;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XIndexAccess;


public class InstCreator implements XInstCreator {
    private final XInterface xParent;
    private final XMultiServiceFactory xMSF;
    private final XInterface xInstance;
    private final XIndexAccess xIA;
    private final InstDescr iDsc;

    public InstCreator( XInterface xParent, InstDescr iDsc ) {
        this.xParent = xParent;
        this.iDsc = iDsc;

        xMSF = UnoRuntime.queryInterface(
                                    XMultiServiceFactory.class, xParent );

        xInstance = createInstance();
        xIA = createCollection();
    }
    public XInterface getInstance() {
        return xInstance;
    }

    public XInterface createInstance() {
        XInterface xIfc = null;
        xIfc = iDsc.createInstance( xMSF );

        return xIfc;
    }

    public XIndexAccess getCollection() {
        return xIA;
    }

    private XIndexAccess createCollection() {
        XNameAccess oNA = null;

        if ( iDsc instanceof TableDsc ) {
            XTextTablesSupplier oTTS = UnoRuntime.queryInterface(
                                XTextTablesSupplier.class, xParent );

            oNA = oTTS.getTextTables();
        }
        if ( iDsc instanceof FrameDsc ) {
            XTextFramesSupplier oTTS = UnoRuntime.queryInterface(
                                XTextFramesSupplier.class, xParent );

            oNA = oTTS.getTextFrames();
        }
        if ( iDsc instanceof BookmarkDsc ) {
            XBookmarksSupplier oTTS = UnoRuntime.queryInterface(
                                XBookmarksSupplier.class, xParent );

            oNA = oTTS.getBookmarks();
        }

        if ( iDsc instanceof FootnoteDsc ) {
            XFootnotesSupplier oTTS = UnoRuntime.queryInterface(
                                XFootnotesSupplier.class, xParent );

            return oTTS.getFootnotes();
        }

        if ( iDsc instanceof TextSectionDsc ) {
            XTextSectionsSupplier oTSS = UnoRuntime.queryInterface(
                                XTextSectionsSupplier.class, xParent );

            oNA = oTSS.getTextSections();
        }

        return UnoRuntime.queryInterface(
                                                    XIndexAccess.class, oNA);
    }
}