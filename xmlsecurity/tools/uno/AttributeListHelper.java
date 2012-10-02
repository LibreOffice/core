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

package com.sun.star.xml.security.uno;

import java.util.Vector;
import com.sun.star.xml.sax.XAttributeList;

/**
 * Class to construct an attribute list, and provide a XAttributeList
 * interface.
 *
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

