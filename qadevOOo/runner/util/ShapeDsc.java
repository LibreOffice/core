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
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.drawing.*;
import com.sun.star.awt.*;
/**
 * the class TableDsc
 */
public class ShapeDsc extends InstDescr {

    private final int x;
    private final int y;
    private final int height;
    private final int width;
    private static final String name = null;
    private static final String ifcName = "com.sun.star.drawing.XShape";
    private final String service;

    public ShapeDsc(int nheight, int nwidth, int nx, int ny, String kind) {
        x = nx;
        y = ny;
        height = nheight;
        width = nwidth;
        service = "com.sun.star.drawing." + kind + "Shape";
        initShape();
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public String getIfcName() {
        return ifcName;
    }
    @Override
    public String getService() {
        return service;
    }

    private void initShape() {
        try {
               ifcClass = Class.forName( ifcName );
        }
        catch( ClassNotFoundException cnfE ) {
        }
    }
    @Override
    public XInterface createInstance( XMultiServiceFactory docMSF ) {


        Object SrvObj = null;
        try {
            SrvObj = docMSF.createInstance( service );
        }
        catch( com.sun.star.uno.Exception cssuE ){
        }

        XShape Sh = (XShape)UnoRuntime.queryInterface(ifcClass, SrvObj );
                   Size size = new Size();
                Point position = new Point();
                size.Height = height;
        size.Width = width;
        position.X = x;
        position.Y = y;
                try {
             Sh.setSize(size);
             Sh.setPosition(position);
                }
                catch ( com.sun.star.beans.PropertyVetoException e) {
                }

        return Sh;

    }
}