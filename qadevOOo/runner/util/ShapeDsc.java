/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ShapeDsc.java,v $
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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.drawing.*;
import com.sun.star.awt.*;
/**
 * the class TableDsc
 */
public class ShapeDsc extends InstDescr {

    private int x = 0;
    private int y = 0;
    private int height = 0;
    private int width = 0;
     private String name = null;
    final String ifcName = "com.sun.star.drawing.XShape";
    String service = "com.sun.star.drawing.RectangleShape";

    public ShapeDsc( int nheight, int nwidth, int nx, int ny, String kind ) {
        x=nx;
                y=ny;
                height=nheight;
                width=nwidth;
        service="com.sun.star.drawing."+kind+"Shape";
        initShape();
    }
    public String getName() {
        return name;
    }

    public String getIfcName() {
        return ifcName;
    }
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