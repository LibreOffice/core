/*************************************************************************
 *
 *  $RCSfile: characterdata.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:31:01 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "characterdata.hxx"

namespace DOM
{

    CCharacterData::CCharacterData()        
    {}

    void CCharacterData::init_characterdata(const xmlNodePtr aNodePtr)
    {
        init_node(aNodePtr);
    }

    /**
    Append the string to the end of the character data of the node.
    */
    void SAL_CALL CCharacterData::appendData(const OUString& arg)
        throw (RuntimeException)
    {
        if (m_aNodePtr != NULL)
        {
            xmlNodeAddContent(m_aNodePtr, (const xmlChar*)(OUStringToOString(arg, RTL_TEXTENCODING_UTF8).getStr()));
        }
    }
    
    /**
    Remove a range of 16-bit units from the node.
    */
    void SAL_CALL CCharacterData::deleteData(sal_Int32 offset, sal_Int32 count) 
        throw (RuntimeException)
    {
        if (m_aNodePtr != NULL)
        {
            // get current data
            OString aData((const sal_Char*)xmlNodeGetContent(m_aNodePtr));
            OUString tmp(aData, aData.getLength(), RTL_TEXTENCODING_UTF8);
            if (offset > tmp.getLength() || offset < 0 || count < 0) 
                return;
            if ((offset+count) > tmp.getLength())
                count = tmp.getLength() - offset;

            OUString tmp2 = tmp.copy(0, offset);
            tmp2 += tmp.copy(offset+count, tmp.getLength() - (offset+count));
            xmlNodeSetContent(m_aNodePtr, (const xmlChar*)(OUStringToOString(tmp2, RTL_TEXTENCODING_UTF8).getStr()));
        }
    }


    /**
    Return the character data of the node that implements this interface.
    */
    OUString SAL_CALL CCharacterData::getData() throw (RuntimeException)
    {
        OUString aData;
        if (m_aNodePtr != NULL)
        {
            aData = OUString((const sal_Char*)m_aNodePtr->content, strlen((const sal_Char*)m_aNodePtr->content),  RTL_TEXTENCODING_UTF8);
        }
        return aData;
    }

    /**
    The number of 16-bit units that are available through data and the
    substringData method below.
    */
    sal_Int32 CCharacterData::getLength() throw (RuntimeException)
    {
        sal_Int32 length = 0;
        if (m_aNodePtr != NULL)
        {
             OUString aData((const sal_Char*)m_aNodePtr->content, strlen((const sal_Char*)m_aNodePtr->content),  RTL_TEXTENCODING_UTF8);
             length = aData.getLength();
        }
        return length;
    }

    /**
    Insert a string at the specified 16-bit unit offset.
    */
    void SAL_CALL CCharacterData::insertData(sal_Int32 offset, const OUString& arg)
        throw (RuntimeException)
    {
        if (m_aNodePtr != NULL)
        {
            // get current data
            OString aData((const sal_Char*)xmlNodeGetContent(m_aNodePtr));
            OUString tmp(aData, aData.getLength(), RTL_TEXTENCODING_UTF8);
            if (offset > tmp.getLength() || offset < 0) 
                return;

            OUString tmp2 = tmp.copy(0, offset);
            tmp2 += arg;
            tmp2 += tmp.copy(offset, tmp.getLength() - offset);
            xmlNodeSetContent(m_aNodePtr, (const xmlChar*)(OUStringToOString(tmp2, RTL_TEXTENCODING_UTF8).getStr()));
        }
    }


    /**
    Replace the characters starting at the specified 16-bit unit offset 
    with the specified string.
    */
    void SAL_CALL CCharacterData::replaceData(sal_Int32 offset, sal_Int32 count, const OUString& arg)
        throw (RuntimeException)
    {
        if (m_aNodePtr != NULL)
        {
            // get current data
            OString aData((const sal_Char*)xmlNodeGetContent(m_aNodePtr));
            OUString tmp(aData, aData.getLength(), RTL_TEXTENCODING_UTF8);
            if (offset > tmp.getLength() || offset < 0 || count < 0) 
                return;
            if ((offset+count) > tmp.getLength())
                count = tmp.getLength() - offset;

            OUString tmp2 = tmp.copy(0, offset);
            tmp2 += arg;
            tmp2 += tmp.copy(offset+count, tmp.getLength() - (offset+count));
            xmlNodeSetContent(m_aNodePtr, (const xmlChar*)(OUStringToOString(tmp2, RTL_TEXTENCODING_UTF8).getStr()));
        }
    }

    /**
    Set the character data of the node that implements this interface.
    */
    void SAL_CALL CCharacterData::setData(const OUString& data)
        throw (RuntimeException)
    {
        if (m_aNodePtr != NULL)
        {
            xmlNodeSetContent(m_aNodePtr, (const xmlChar*)(OUStringToOString(data, RTL_TEXTENCODING_UTF8).getStr()));
        }
    }

    /**
    Extracts a range of data from the node.
    */
    OUString SAL_CALL CCharacterData::subStringData(sal_Int32 offset, sal_Int32 count)
        throw (RuntimeException)
    {
        OUString aStr;
        if (m_aNodePtr != NULL)
        {
            // get current data
            OString aData((const sal_Char*)xmlNodeGetContent(m_aNodePtr));
            OUString tmp(aData, aData.getLength(), RTL_TEXTENCODING_UTF8);
            aStr = tmp.copy(offset, count);
        }
        return aStr;
    }


} // namspace DOM