/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InstCreator.java,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package util;

import util.XInstCreator;

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
    XInterface xParent;
    XMultiServiceFactory xMSF;
    XInterface xInstance;
    XIndexAccess xIA;
    InstDescr iDsc;

    public InstCreator( XInterface xParent, InstDescr iDsc ) {
        this.xParent = xParent;
        this.iDsc = iDsc;

        xMSF = (XMultiServiceFactory)UnoRuntime.queryInterface(
                                    XMultiServiceFactory.class, xParent );

        xInstance = createInstance();
        xIA = createCollection();
    }
    public XInterface getInstance() {
        return xInstance;
    }

    public XInterface createInstance() {
        XInterface xIfc = null;
        Object xObj = null;

        xIfc = iDsc.createInstance( xMSF );

        return xIfc;
    }

    public XIndexAccess getCollection() {
        return xIA;
    }

    private XIndexAccess createCollection() {
        XNameAccess oNA = null;

        if ( iDsc instanceof TableDsc ) {
            XTextTablesSupplier oTTS = (XTextTablesSupplier)
                    UnoRuntime.queryInterface(
                                        XTextTablesSupplier.class, xParent );

            oNA = oTTS.getTextTables();
        }
        if ( iDsc instanceof FrameDsc ) {
            XTextFramesSupplier oTTS = (XTextFramesSupplier)
                    UnoRuntime.queryInterface(
                                        XTextFramesSupplier.class, xParent );

            oNA = oTTS.getTextFrames();
        }
        if ( iDsc instanceof BookmarkDsc ) {
            XBookmarksSupplier oTTS = (XBookmarksSupplier)
                    UnoRuntime.queryInterface(
                                        XBookmarksSupplier.class, xParent );

            oNA = oTTS.getBookmarks();
        }

        if ( iDsc instanceof FootnoteDsc ) {
            XFootnotesSupplier oTTS = (XFootnotesSupplier)
                    UnoRuntime.queryInterface(
                                        XFootnotesSupplier.class, xParent );

            return( oTTS.getFootnotes() );
        }

        if ( iDsc instanceof TextSectionDsc ) {
            XTextSectionsSupplier oTSS = (XTextSectionsSupplier)
                    UnoRuntime.queryInterface(
                                        XTextSectionsSupplier.class, xParent );

            oNA = oTSS.getTextSections();
        }

        return (XIndexAccess)UnoRuntime.queryInterface(
                                                    XIndexAccess.class, oNA);
    }
}