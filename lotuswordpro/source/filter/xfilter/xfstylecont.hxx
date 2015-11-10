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
 * Style container,which response for special style,ie. text style.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFSTYLECONT_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFSTYLECONT_HXX

#include "xfglobal.hxx"
#include <vector>

class IXFStyle;

struct IXFStyleRet
{
    IXFStyle* m_pStyle;
    bool m_bOrigDeleted;
    IXFStyleRet()
        : m_pStyle(nullptr)
        , m_bOrigDeleted(false)
    {
    }
};

/**
 * @descr   container object for styles.
 *          All styles can be placed into an style container.
 */
class XFStyleContainer
{
public:
    XFStyleContainer(){}

    explicit XFStyleContainer(const OUString& strStyleNamePrefix);

    XFStyleContainer(const XFStyleContainer& other);

    XFStyleContainer& operator=(const XFStyleContainer& other);

    virtual ~XFStyleContainer();

public:
    /**
     * @descr   Add style to container.
     *          If the same style has exist, then pStyle will be deleted, and the same style will be return.
     */
    IXFStyleRet     AddStyle(IXFStyle *pStyle);

    /**
     * @descr   Find the same style.
     */
    IXFStyle*       FindSameStyle(IXFStyle *pStyle);

    /**
     * @descr   get style by name.
     */
    IXFStyle*       FindStyle(const OUString& name);

    /**
     * @descr   clear container.
     */
    void            Reset();

    /**
     * @descr   get count of styles in the container.
     */
    size_t      GetCount() const;

    /**
     * @descr   get style by index.
     */
    const IXFStyle* Item(size_t index) const;

    /**
     * @descr   Output all style.
     */
    virtual void    ToXml(IXFStream *pStrm);

    friend bool operator==(XFStyleContainer& b1, XFStyleContainer& b2);
    friend bool operator!=(XFStyleContainer& b1, XFStyleContainer& b2);
private:
    static void     ManageStyleFont(IXFStyle *pStyle);
private:
    std::vector<IXFStyle*>  m_aStyles;
    OUString   m_strStyleNamePrefix;
};

inline size_t XFStyleContainer::GetCount() const
{
    return m_aStyles.size();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
