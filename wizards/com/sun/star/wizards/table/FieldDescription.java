/*************************************************************************
*
*  $RCSfile: FieldDescription.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: pjunck $ $Date: 2004-10-27 13:37:50 $
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


package com.sun.star.wizards.table;

import java.util.Hashtable;
import java.util.Vector;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.ConfigGroup;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Properties;


public class FieldDescription{
    int category;
    private String tablename = "";
//  String fieldname;
    private String keyname;
    private XNameAccess xNameAccessTableNode;
    private XPropertySet xPropertySet;
    private Vector aPropertyValues;
//  PropertyValue[] aPropertyValues;
    private Integer Type;
    private Integer Scale;
    private Integer Precision;
    private Boolean DefaultValue;
    private String Name;

    public FieldDescription(ScenarioSelector _curscenarioselector, String _fieldname, String _keyname){
        category = _curscenarioselector.getCategory();
        tablename = _curscenarioselector.getTableName();
        Name = _fieldname;
        keyname = _keyname;
        aPropertyValues = new Vector();
        xNameAccessTableNode = _curscenarioselector.oCGTable.xNameAccessFieldsNode;
        XNameAccess xNameAccessFieldNode;
        if (_curscenarioselector.bcolumnnameislimited)
            xNameAccessFieldNode = Configuration.getChildNodebyDisplayName(xNameAccessTableNode, keyname, "ShortName");
        else
            xNameAccessFieldNode = Configuration.getChildNodebyDisplayName(xNameAccessTableNode,keyname, "Name");
        setFieldProperties(xNameAccessFieldNode);
    }

    public FieldDescription(String _fieldname){
        Name = _fieldname;
        aPropertyValues = new Vector();
        Type = new Integer(com.sun.star.sdbc.DataType.VARCHAR);
        aPropertyValues.addElement(Properties.createProperty("Name", _fieldname) );
        aPropertyValues.addElement(Properties.createProperty("Type", Type) );
    }


    public void setName(String _newfieldname){
        for (int i = 0; i < aPropertyValues.size(); i++){
            PropertyValue aPropertyValue = (PropertyValue) aPropertyValues.get(i);
            if (aPropertyValue.Name.equals("Name")){
                aPropertyValue.Value = _newfieldname;
                aPropertyValues.set(i, aPropertyValue);
                Name = _newfieldname;
                return;
            }
        }
    }

    public String getName(){
        return Name;
    }

    public String gettablename(){
        return tablename;
    }


    private boolean propertyexists(String _propertyname){
    boolean bexists = false;
    try {
        if (xPropertySet.getPropertySetInfo().hasPropertyByName(_propertyname)){
            Object oValue = xPropertySet.getPropertyValue(_propertyname);
            bexists = (!com.sun.star.uno.AnyConverter.isVoid(oValue));
        }
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }
    return bexists;
    }


    public void setFieldProperties(XNameAccess _xNameAccessFieldNode){
    try {
        xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, _xNameAccessFieldNode);
//      Integer Index = (Integer) xPropertySet.getPropertyValue("Index");
        if (propertyexists("Name"))
            aPropertyValues.addElement(Properties.createProperty("Name", Name));

        if (propertyexists("Type"))
            aPropertyValues.addElement(Properties.createProperty("Type", (Integer) xPropertySet.getPropertyValue("Type")));
        if (propertyexists("Scale"))
            aPropertyValues.addElement(Properties.createProperty("Scale", (Integer) xPropertySet.getPropertyValue("Scale")));
//          Scale =
        if (propertyexists("Precision"))
            aPropertyValues.addElement(Properties.createProperty("Precision", (Integer) xPropertySet.getPropertyValue("Precision")));
//          Precision = (Integer) xPropertySet.getPropertyValue("Precision");
        if (propertyexists("DefaultValue"))
            aPropertyValues.addElement(Properties.createProperty("DefaultValue",(Boolean) xPropertySet.getPropertyValue("DefaultValue")));
//          DefaultValue = (Boolean) xPropertySet.getPropertyValue("DefaultValue");
        //Type =  new Integer(4); // TODO wo ist der Fehler?(Integer) xPropertySet.getPropertyValue("Type");
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    public PropertyValue[] getPropertyValues(){
        if (aPropertyValues != null){
            PropertyValue[] aProperties = new PropertyValue[aPropertyValues.size()];
            aPropertyValues.toArray(aProperties);
            return aProperties;
        }
        return null;
    }

//  public void setFieldProperties(XNameAccess _xNameAccessFieldNode, String _fieldname){
//  try {
//      xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, _xNameAccessFieldNode);
//      Name = _fieldname;
////        Integer Index = (Integer) xPropertySet.getPropertyValue("Index");
//      if (propertyexists("Type"))
//          aPropertyValues.addElement(Properties.createProperty("Type", (Integer) xPropertySet.getPropertyValue("Type")) );
//
//          Type = (Integer) xPropertySet.getPropertyValue("Type");
//      if (propertyexists("Scale"))
//          Scale = (Integer) xPropertySet.getPropertyValue("Scale");
//      if (propertyexists("Precision"))
//          Precision = (Integer) xPropertySet.getPropertyValue("Precision");
//      if (propertyexists("DefaultValue"))
//          DefaultValue = (Boolean) xPropertySet.getPropertyValue("DefaultValue");
//      //Type =  new Integer(4); // TODO wo ist der Fehler?(Integer) xPropertySet.getPropertyValue("Type");
//  } catch (Exception e) {
//      e.printStackTrace(System.out);
//  }}



}




//public class CGField extends DBField{
//
//  XMultiServiceFactory xMSF;
//  XNameAccess xNameAccessFieldNode;
//  XNameAccess xNameAccessPropertiesNode;
//  XNameAccess xNameAccessTypeNode;
//  XPropertySet xPropertySet;
//  boolean bgetbyShortName;
//
//  private Object oconfigView;
//
//  public CGField(XMultiServiceFactory _xMSF){
//      super(_xMSF);
//  }
//
//
//  public void initialize(XNameAccess _xNameAccessParentNode, String _fieldname, String _keyname, boolean _bgetbyShortName){
//      bgetbyShortName = _bgetbyShortName;
//      if (_bgetbyShortName)
//          xNameAccessFieldNode = Configuration.getChildNodebyDisplayName(_xNameAccessParentNode, _keyname, "ShortName");
//      else
//          xNameAccessFieldNode = Configuration.getChildNodebyDisplayName(_xNameAccessParentNode,_keyname, "Name");
//      setFieldProperties(_fieldname);
//  }
//
//
//  public void initialize(XNameAccess _xNameAccessParentNode, String _fieldname, int _index){
//      xNameAccessFieldNode = Configuration.getChildNodebyIndex( _xNameAccessParentNode, _index);
//      setFieldProperties(_fieldname);
//  }
//
//  private boolean propertyexists(String _propertyname){
//      boolean bexists = false;
//      try {
//          if (xPropertySet.getPropertySetInfo().hasPropertyByName(_propertyname)){
//              Object oValue = xPropertySet.getPropertyValue(_propertyname);
//              bexists = (!com.sun.star.uno.AnyConverter.isVoid(oValue));
//          }
//      } catch (Exception e) {
//          e.printStackTrace(System.out);
//      }
//      return bexists;
//  }
//
//  private void setFieldProperties(String _fieldname){
//  try {
//      xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccessFieldNode);
//      Name = _fieldname;
//      Index = (Integer) xPropertySet.getPropertyValue("Index");
//      if (propertyexists("Type"))
//          Type = (Integer) xPropertySet.getPropertyValue("Type");
//      if (propertyexists("Scale"))
//          Scale = (Integer) xPropertySet.getPropertyValue("Scale");
//      if (propertyexists("Precision"))
//          Precision = (Integer) xPropertySet.getPropertyValue("Precision");
//      if (propertyexists("DefaultValue"))
//          DefaultValue = (Boolean) xPropertySet.getPropertyValue("DefaultValue");
//      //Type =  new Integer(4); // TODO wo ist der Fehler?(Integer) xPropertySet.getPropertyValue("Type");
//  } catch (Exception e) {
//      e.printStackTrace(System.out);
//  }}
//
//
//  public void getProperties(){
//  try {
//      String[] spropnames = Configuration.getNodeDisplayNames(xNameAccessPropertiesNode);
//      for (int i = 0; i < spropnames.length; i++){
//          XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccessPropertiesNode);
//          String sname = (String) xPropertySet.getPropertyValue("Name");
//          String svalue = (String) xPropertySet.getPropertyValue("Value");
//      }
//      int i = 0;
//  } catch (Exception e) {
//      e.printStackTrace(System.out);
////        return null;
//  }}
//
//}
