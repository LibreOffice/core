/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * index entry object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-04-26 create this file.
 ************************************************************************/
#ifndef		_XFENTRY_HXX
#define		_XFENTRY_HXX

#include	"xfcontent.hxx"

/**
 * @brief
 * Index entry obejct.
 */
class XFEntry : public XFContent
{
public:
    XFEntry();

public:
    /**
     * @descr	Set entry type.
     */
    void	SetEntryType(enumXFEntry type);

    /**
     * @descr	Set entry string value.
     */
    void	SetStringValue(const rtl::OUString& value);

    /**
     * @descr	Set display string.
     */
    void	SetStringDisplay(const rtl::OUString& display);

    /**
     * @descr	Set entry name.
     */
    void	SetEntryName(const rtl::OUString& name);

    /**
     * @descr	Set entry key. The keys is available only for enumXFEntryAlphabetical.
     */
    void	SetKey(const rtl::OUString& key1, const rtl::OUString& key2=A2OUSTR(""));

    /**
     * @descr	Set whether it's a main entry. This is available only for enumXFEntryAlphabetical.
     */
    void	SetMainEntry(sal_Bool main);

    /**
     * @descr	Set outline level. This is available for enumXFEntryTOC and enumXFEntryUserIndex.
     */
    void	SetOutlineLevel(sal_Int32 level);

    virtual void	ToXml(IXFStream *pStrm);

private:
    enumXFEntry		m_eType;
    rtl::OUString	m_strValue;
    rtl::OUString	m_strDisplay;
    rtl::OUString	m_strKey1;
    rtl::OUString	m_strKey2;
    rtl::OUString	m_strName;
    sal_Bool		m_bMainEntry;
    sal_Int32		m_nOutlineLevel;
};

inline void	XFEntry::SetEntryType(enumXFEntry type)
{
    m_eType = type;
}

inline void XFEntry::SetStringValue(const rtl::OUString& value)
{
    m_strValue = value;
}

inline void XFEntry::SetStringDisplay(const rtl::OUString& display)
{
    m_strDisplay = display;
}

inline void XFEntry::SetEntryName(const rtl::OUString& name)
{
    m_strName = name;
}

inline void	XFEntry::SetKey(const rtl::OUString& key1, const rtl::OUString& key2/* =A2OUSTR */)
{
    m_strKey1 = key1;
    m_strKey2 = key2;
}

inline void	XFEntry::SetMainEntry(sal_Bool main)
{
    m_bMainEntry = main;
}

inline void XFEntry::SetOutlineLevel(sal_Int32 level)
{
    m_nOutlineLevel = level;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
