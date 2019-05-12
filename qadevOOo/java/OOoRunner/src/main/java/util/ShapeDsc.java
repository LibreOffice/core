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