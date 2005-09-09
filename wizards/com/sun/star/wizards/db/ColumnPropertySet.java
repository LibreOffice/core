/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColumnPropertySet.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:22:53 $
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
 ************************************************************************/package com.sun.star.wizards.db;

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
