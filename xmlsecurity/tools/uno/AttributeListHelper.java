/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AttributeListHelper.java,v $
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

package com.sun.star.xml.security.uno;

import java.util.Vector;
import com.sun.star.xml.sax.XAttributeList;

/**
 * Class to construct an attribute list, and provide a XAttributeList
 * interface.
 *
 * @author      Michael Mi
 * @version     %I%, %G%
 */
public class AttributeListHelper implements com.sun.star.xml.sax.XAttributeList
{
    private Vector m_AttributeList;

    public AttributeListHelper()
    {
        m_AttributeList = new Vector();
    }

    public void clear()
    {
        m_AttributeList.removeAllElements();
    }

    public void setAttribute(String name, String type, String value)
    {
        int nLength = m_AttributeList.size();
        boolean bFound = false;

        for (int i=0; i<nLength; ++i)
        {
            if (getNameByIndex((short)i).equals(name))
            {
                Vector attribute = (Vector)m_AttributeList.get(i);
                attribute.setElementAt(type,1);
                attribute.setElementAt(value,2);
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            Vector attribute = new Vector();
            attribute.addElement(name);
            attribute.addElement(type);
            attribute.addElement(value);
            m_AttributeList.addElement(attribute);
        }
    }

    public String getAttributeItem(short index, int itemIndex)
    {
        String item = null;

        if (index>=0 && index<getLength())
        {
            Vector attribute = (Vector)m_AttributeList.get(index);
            item = (String)(attribute.get(itemIndex));
        }

        return item;
    }

    /* XAttributeList */
    public short getLength()
    {
        return (short)m_AttributeList.size();
    }

    public String getNameByIndex(short i)
    {
        return getAttributeItem(i, 0);
    }

    public String getTypeByIndex(short i)
    {
        return getAttributeItem(i, 1);
    }

    public String getValueByIndex(short i)
    {
        return getAttributeItem(i, 2);
    }

    public String getTypeByName(String aName)
    {
        int nLength = m_AttributeList.size();
        String type = null;

        for (int i=0; i<nLength; ++i)
        {
            if (getNameByIndex((short)i).equals(aName))
            {
                type = getTypeByIndex((short)i);
                break;
            }
        }

        return type;
    }

    public String getValueByName(String aName)
    {
        int nLength = m_AttributeList.size();
        String value = null;

        for (int i=0; i<nLength; ++i)
        {
            if (getNameByIndex((short)i).equals(aName))
            {
                value = getValueByIndex((short)i);
                break;
            }
        }
        return value;
    }
}

