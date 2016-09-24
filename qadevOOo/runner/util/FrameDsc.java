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
import com.sun.star.text.XTextFrame;
import com.sun.star.drawing.XShape;
import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
/**
 * the class FrameDsc
 */
public class FrameDsc extends InstDescr {

    private int height = 2000;
    private int width = 2000;
    private static final String ifcName = "com.sun.star.text.XTextFrame";
    private static final String service = "com.sun.star.text.TextFrame";

    public FrameDsc() {
        initFrame();
    }

    public FrameDsc( int nHeight, int nWidth ) {
        height = nHeight;
        width = nWidth;
        initFrame();
    }

    @Override
    public String getName() {
        return null;
    }
    @Override
    public String getIfcName() {
        return ifcName;
    }
    @Override
    public String getService() {
        return service;
    }

    private void initFrame() {
        try {
            ifcClass = Class.forName( ifcName );
        }
        catch( ClassNotFoundException cnfE ) {
        }
    }
    @Override
    public XInterface createInstance( XMultiServiceFactory docMSF ) {
        Object SrvObj = null;

        Size size = new Size();
        size.Height = height;
        size.Width = width;

        try {
            SrvObj = docMSF.createInstance( service );
        }
        catch( com.sun.star.uno.Exception cssuE ){
        }
        XShape shape = UnoRuntime.queryInterface( XShape.class, SrvObj );
        try {
            shape.setSize(size);
        }
        catch( com.sun.star.beans.PropertyVetoException pvE ){
        }

        XTextFrame TF = (XTextFrame)UnoRuntime.queryInterface( ifcClass, SrvObj );

        XPropertySet oPropSet = UnoRuntime.queryInterface( XPropertySet.class, SrvObj );


        try {
            oPropSet.setPropertyValue("AnchorType", Integer.valueOf(2));
        }
        catch( com.sun.star.beans.UnknownPropertyException upE ){
        }
        catch( com.sun.star.beans.PropertyVetoException pvE ){
        }
        catch( com.sun.star.lang.IllegalArgumentException iaE ){
        }
        catch( com.sun.star.lang.WrappedTargetException wtE ){
        }



        return TF;
    }
}
