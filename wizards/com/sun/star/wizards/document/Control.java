/*************************************************************************
 *
 *  $RCSfile: Control.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $  $Date: 2004-10-27 13:31:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Berend Cornelius
 *
 */

package com.sun.star.wizards.document;
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XLayoutConstrains;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.wizards.common.*;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class Control extends Shape{
    XControlModel xControlModel;
    XControl xControl;
    public XPropertySet xPropertySet;
    XPropertySet xControlPropertySet;
    XWindowPeer xWindowPeer;
    XLayoutConstrains xLayoutConstrains;
    Object oDefaultValue;
    GridControl oGridControl;
    String sServiceName;
    XNamed xNamed;
    final int SOMAXTEXTSIZE = 50;
    int icontroltype;
    protected XNameContainer xFormName;


    public Control(){
    }

    public Control(FormHandler _oFormHandler, String _sServiceName, Point _aPoint){
        super(_oFormHandler, _sServiceName, _aPoint, null);
    }

    public Control(FormHandler _oFormHandler, XNameContainer _xFormName, int _icontroltype, Point _aPoint, Size _aSize){
    super(_oFormHandler, _aPoint, _aSize);
        xFormName = _xFormName;
        createControl(_icontroltype, _aPoint, _aSize, null);
    }


    public Control(FormHandler _oFormHandler, XShapes _xGroupShapes, XNameContainer _xFormName, int _icontroltype, Point _aPoint, Size _aSize){
    super(_oFormHandler, _aPoint, _aSize);
        xFormName = _xFormName;
        createControl(_icontroltype, _aPoint, _aSize, _xGroupShapes);
    }




    public Control(FormHandler _oFormHandler, int _icontroltype, Point _aPoint, Size _aSize){
        super(_oFormHandler, _aPoint, _aSize);
        createControl(_icontroltype, _aPoint, _aSize, null);
    }



    public void createControl(int _icontroltype, Point _aPoint, Size _aSize, XShapes _xGroupShapes){
    try {
        this.icontroltype = _icontroltype;
        this.sServiceName = oFormHandler.sModelServices[icontroltype];
        Object oControlModel = oFormHandler.xMSFDoc.createInstance(sServiceName);
        xControlModel = (XControlModel) UnoRuntime.queryInterface(XControlModel.class, oControlModel);
        xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oControlModel);
//      insertControlInContainer();
        xControlShape.setControl(xControlModel);
        if  (_xGroupShapes == null)
            oFormHandler.xDrawPage.add(xShape);
        else
            _xGroupShapes.add(xShape);
        xControl = oFormHandler.xControlAccess.getControl(xControlModel);
        xControlPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControl);
        xWindowPeer = xControl.getPeer();
        xLayoutConstrains = (XLayoutConstrains) UnoRuntime.queryInterface(XLayoutConstrains.class, xControl.getPeer());
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    protected void insertControlInContainer(){
    try {
        if (xFormName != null){
            XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xFormName);
            String sControlName = Desktop.getUniqueName(xNameAccess, getDefaultControlName());
            xFormName.insertByName(sControlName, xControlModel);
        }
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    // TODO the default name should be set according to the Array ControlData defined in FormHandler
    public String getDefaultControlName(){
        return "LabelField";
    }


    private void setDefaultValue(Object DatabaseField){
        oDefaultValue = Helper.getUnoPropertyValue(DatabaseField, "DefaultValue");
    }


    public int getPreferredWidth(String sText){
        Size aPeerSize = getPreferredSize(sText);
        return ((aPeerSize.Width + 10) * oFormHandler.getXPixelFactor());
    }


    public int getPreferredHeight(String sText){
        Size aPeerSize = getPreferredSize(sText);
        return ((aPeerSize.Height + 2) * oFormHandler.getXPixelFactor());
    }


    public int getPreferredWidth(){
        if (icontroltype == FormHandler.SOIMAGECONTROL)
            return 2000; // TODO  replace literal value by constant
        else{
            Size aPeerSize = getPeerSize();
            // We increase the preferred Width a bit so that the control does not become too small
            // when we change the border from "3D" to "Flat"
            return((aPeerSize.Width + 100) * oFormHandler.getXPixelFactor());   // PixelTo100thmm(nWidth)
        }
    }


    public int getPreferredHeight(){
        if (this.icontroltype == FormHandler.SOIMAGECONTROL)
            return 2000;
        else{
            Size aPeerSize = getPeerSize();
            int nHeight = aPeerSize.Height;
            // We increase the preferred Height a bit so that the control does not become too small
            // when we change the border from "3D" to "Flat"
            return((nHeight+1) * oFormHandler.getYPixelFactor());   // PixelTo100thmm(nHeight)
        }
    }


    public Size getPreferredSize(String sText){
    try {
        if (xPropertySet.getPropertySetInfo().hasPropertyByName("Text")){
//          xPropertySet.setPropertyValue("MaxTextLen", new Integer(sText.length()));
            xPropertySet.setPropertyValue("Text", sText);
        }
        else if (xPropertySet.getPropertySetInfo().hasPropertyByName("Label"))
            xPropertySet.setPropertyValue("Label", sText);
        else
            throw new IllegalArgumentException();

        XLayoutConstrains xLayoutConstrains = (XLayoutConstrains) UnoRuntime.queryInterface(XLayoutConstrains.class, xControl.getPeer());
        return xLayoutConstrains.getPreferredSize();
    } catch (Exception e) {
        e.printStackTrace(System.out);
        return null;
    }}


    public void setPropertyValue(String _sPropertyName, Object _aPropertyValue) throws Exception{
        if (xPropertySet.getPropertySetInfo().hasPropertyByName(_sPropertyName))
            xPropertySet.setPropertyValue(_sPropertyName, _aPropertyValue);
    }


    public Size getPeerSize(){
    try {
        Size aPreferredSize = null;
        double dblEffMax = 0;
        if (xPropertySet.getPropertySetInfo().hasPropertyByName("EffectiveMax")){
            Object oValue = xPropertySet.getPropertyValue("EffectiveMax");
            if (xPropertySet.getPropertyValue("EffectiveMax") != com.sun.star.uno.Any.VOID)
                dblEffMax = AnyConverter.toDouble(xPropertySet.getPropertyValue("EffectiveMax"));
            if (dblEffMax == 0){
                // This is relevant for decimal fields
                xPropertySet.setPropertyValue("EffectiveValue", new Double(99999));
            }
            else
                xPropertySet.setPropertyValue("EffectiveValue", xPropertySet.getPropertyValue("EffectiveMax"));
            aPreferredSize = xLayoutConstrains.getPreferredSize();
            xPropertySet.setPropertyValue("EffectiveValue", com.sun.star.uno.Any.VOID);

        }
        else if (this.icontroltype == FormHandler.SOCHECKBOX){
            aPreferredSize = xLayoutConstrains.getPreferredSize();
        }
        else if (this.icontroltype == FormHandler.SODATECONTROL){
            xPropertySet.setPropertyValue("Date", new Integer(4711));       //TODO find a better date
            aPreferredSize = xLayoutConstrains.getPreferredSize();
            xPropertySet.setPropertyValue("Date",  com.sun.star.uno.Any.VOID);
        }
        else if (this.icontroltype == FormHandler.SOTIMECONTROL){
            xPropertySet.setPropertyValue("Time", new Integer(4711));       //TODO find a better time
            aPreferredSize = xLayoutConstrains.getPreferredSize();
            xPropertySet.setPropertyValue("Time",  com.sun.star.uno.Any.VOID);
        }
        else{
            String stext;
            short iTextLength = AnyConverter.toShort(xPropertySet.getPropertyValue("MaxTextLen"));
            if (iTextLength < this.SOMAXTEXTSIZE)
                stext = FormHandler.SOSIZETEXT.substring(0,this.SOMAXTEXTSIZE);
            else
                stext = FormHandler.SOSIZETEXT.substring(0, iTextLength);
            xPropertySet.setPropertyValue("Text", stext);
            aPreferredSize = xLayoutConstrains.getPreferredSize();
            xPropertySet.setPropertyValue("Text", "");
        }
        return aPreferredSize;
    } catch (Exception e) {
        e.printStackTrace(System.out);
        return null;
    }}


    /**
     * @return
     */
    public int getControlType() {
        return icontroltype;
    }
}
