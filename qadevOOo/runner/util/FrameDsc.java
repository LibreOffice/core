/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FrameDsc.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:33:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package util;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFrame;
import com.sun.star.drawing.XShape;
import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
/**
 * the class FrameDsc
 */
public class FrameDsc extends InstDescr {

    private Size size = null;
    private int height = 2000;
    private int width = 2000;
    private String name = null;
    private int autoheigth = 0;
    private int anchorType = 0;// bound at paragraph

    final String ifcName = "com.sun.star.text.XTextFrame";
    final String service = "com.sun.star.text.TextFrame";

    public FrameDsc() {
        initFrame();
    }

    public FrameDsc( int nHeight, int nWidth ) {
        height = nHeight;
        width = nWidth;
        initFrame();
    }

    public FrameDsc( String FrameName, int nHeight, int nWidth ) {
        name = FrameName;
        height = nHeight;
        width = nWidth;
        initFrame();
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

    private void initFrame() {
        try {
            ifcClass = Class.forName( ifcName );
        }
        catch( ClassNotFoundException cnfE ) {
        }
    }
    public XInterface createInstance( XMultiServiceFactory docMSF ) {
        Object SrvObj = null;

        size = new Size();
        size.Height = height;
        size.Width = width;

        try {
            SrvObj = docMSF.createInstance( service );
        }
        catch( com.sun.star.uno.Exception cssuE ){
        }
        XShape shape = (XShape)UnoRuntime.queryInterface( XShape.class, SrvObj );
        try {
            shape.setSize(size);
        }
        catch( com.sun.star.beans.PropertyVetoException pvE ){
        }

        XTextFrame TF = (XTextFrame)UnoRuntime.queryInterface( ifcClass, SrvObj );

        XPropertySet oPropSet = (XPropertySet)
                        UnoRuntime.queryInterface( XPropertySet.class, SrvObj );


        try {
            oPropSet.setPropertyValue("AnchorType", new Integer(2));
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