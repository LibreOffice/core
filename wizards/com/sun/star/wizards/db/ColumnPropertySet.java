/*************************************************************************
*
*  $RCSfile: ColumnPropertySet.java,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: kz $ $Date: 2005-03-21 18:04:17 $
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
*  Contributor(s): _______________________________________
*
*/
package com.sun.star.wizards.db;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.AnyConverter;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.db.TypeInspector;


public class ColumnPropertySet {
    TypeInspector oTypeInspector;
    public XPropertySet xPropertySet;
    private int nType;
    private String sTypeName = "";


    public ColumnPropertySet(TypeInspector _oTypeInspector, XPropertySet _xPropertySet){
        xPropertySet = _xPropertySet;
        oTypeInspector = _oTypeInspector;
    }


    public void assignPropertyValues(PropertyValue[] _aNewColPropertyValues, boolean _bsetDefaultProperties){
        try {
            nType = ((Integer) Properties.getPropertyValue(_aNewColPropertyValues, "Type")).intValue();
            if (Properties.hasPropertyValue(_aNewColPropertyValues, "TypeName"))
                sTypeName = (String) Properties.getPropertyValue(_aNewColPropertyValues, "TypeName");
            setType(nType, sTypeName);
            for (int i = 0; i < _aNewColPropertyValues.length; i++){
                String sPropName = _aNewColPropertyValues[i].Name;
                if ((!sPropName.equals("Type")) && (!sPropName.equals("TypeName"))){
                    Object oColValue = _aNewColPropertyValues[i].Value;
                    assignPropertyValue(sPropName, oColValue);
                }
            }
            if (_bsetDefaultProperties)
                assignPropertyValue("IsNullable", new Integer(oTypeInspector.isNullable(xPropertySet)));
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
    }


    public void assignNewPropertySet(String _sNewName, XPropertySet _xNewPropertySet){
    try {
        Property[] aNewColProperties = _xNewPropertySet.getPropertySetInfo().getProperties();
        nType = ((Integer)_xNewPropertySet.getPropertyValue("Type")).intValue();
        sTypeName = (String) _xNewPropertySet.getPropertyValue("TypeName");
        setType(nType, sTypeName);
        for (int i = 0; i < aNewColProperties.length; i++){
            String sPropName = aNewColProperties[i].Name;
            if (sPropName.equals("Name"))
                xPropertySet.setPropertyValue("Name", _sNewName);
            else if ((!sPropName.equals("Type")) && (!sPropName.equals("TypeName"))){
                Object oColValue = _xNewPropertySet.getPropertyValue(sPropName);
                assignPropertyValue(sPropName, oColValue);
            }
        }
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    private void setType(int _nType, String _sTypeName){
        if (_sTypeName.equals(""))
            sTypeName = oTypeInspector.getDefaultTypeName(nType);
        else
            sTypeName = _sTypeName;
        nType = oTypeInspector.getDataType(sTypeName);
        assignPropertyValue("Type", new Integer(nType));
        assignPropertyValue("TypeName", sTypeName);
    }


    private void assignPropertyValue(String _spropname, Object _oValue){
        try {
            if (_spropname.equals("Type")){
                nType = ((Integer) _oValue).intValue();
                xPropertySet.setPropertyValue("Type",new Integer(nType));
            }
            else if (_spropname.equals("Name")){
                String sName = (String) _oValue;
                if  (!sName.equals(""))
                    xPropertySet.setPropertyValue("Name", sName);
            }
            else if (_spropname.equals("Scale")){
                int nScale = ((Integer) _oValue).intValue();
                nScale = oTypeInspector.getScale(xPropertySet);
                xPropertySet.setPropertyValue("Scale",new Integer(nScale));
            }
            else if (_spropname.equals("IsNullable")){
                int nNullability = ((Integer) _oValue).intValue();
                nNullability = oTypeInspector.getNullability(xPropertySet, nNullability);
                xPropertySet.setPropertyValue("IsNullable", new Integer(nNullability));
            }
            else if (_spropname.equals("TypeName")){
                String sTypeName = (String) _oValue;
                xPropertySet.setPropertyValue("TypeName", sTypeName);
            }
            else
                xPropertySet.setPropertyValue(_spropname, _oValue);
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
    }


    private int getType(){
        return nType;
    }
}
