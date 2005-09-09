/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CGTable.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:39:28 $
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
 ************************************************************************/package com.sun.star.wizards.table;


import com.sun.star.awt.XListBox;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.ConfigGroup;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;


public class CGTable{
    XMultiServiceFactory xMSF;
    XNameAccess xNameAccessFieldsNode;
    XNameAccess xNameAccessTableNode;
    public String Index;
    public String Name;
    private Object oconfigView;
    private final String CGROOTPATH = "/org.openoffice.Office.TableWizard/TableWizard/";


    public CGTable(XMultiServiceFactory _xMSF){
        xMSF = _xMSF;
    }

    public void initialize(XNameAccess _xNameAccessParentNode, int _index){
    try {
        xNameAccessTableNode = Configuration.getChildNodebyIndex( _xNameAccessParentNode, _index);
        xNameAccessFieldsNode = Configuration.getChildNodebyName(xNameAccessTableNode, "Fields");
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    public String[] getFieldNames(boolean _bgetbyShortName){
    try {
        String[] fieldnames = null;
        if (_bgetbyShortName)
            fieldnames = Configuration.getNodeChildNames(xNameAccessFieldsNode, "ShortName");
        else
            fieldnames = Configuration.getNodeChildNames(xNameAccessFieldsNode, "Name");

        for (int i = 0;i < fieldnames.length; i++){
            fieldnames[i] = Desktop.removeSpecialCharacters(xMSF, Configuration.getOfficeLocale(xMSF), fieldnames[i]);
        }
        return fieldnames;
    } catch (Exception e) {
        e.printStackTrace(System.out);
        return null;
    }}
}
