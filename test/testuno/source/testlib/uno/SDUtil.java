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
package testlib.uno;

import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;

/**
 *
 *
 */
public class SDUtil {

    private SDUtil() {

    }

    public static Object getPageByIndex(XComponent doc, int index) throws Exception {
        XDrawPagesSupplier xDrawPagesSupplier = (XDrawPagesSupplier) UnoRuntime.queryInterface(XDrawPagesSupplier.class, doc);
        Object drawPages = xDrawPagesSupplier.getDrawPages();
        XIndexAccess xIndexedDrawPages = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, drawPages);
        return xIndexedDrawPages.getByIndex(index);
    }

    public static Object getShapeOfPageByIndex(Object page, int index) throws Exception {
        XDrawPage xDrawPage = (XDrawPage) UnoRuntime.queryInterface(XDrawPage.class, page);
        XShapes m_xdrawShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, xDrawPage);
        return m_xdrawShapes.getByIndex(index);
    }

}
