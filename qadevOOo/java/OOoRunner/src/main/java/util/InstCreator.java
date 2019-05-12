/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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