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
 * Container for content.It will destroy all children when destroy.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFCONTENTCONTAINER_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFCONTENTCONTAINER_HXX

#include <sal/config.h>

#include <vector>

#include <rtl/ref.hxx>

#include "xfcontent.hxx"

/**
 * @brief
 * A container for content.
 * The contents will be deleted when delete container.
 */
class XFContentContainer : public XFContent
{
public:
    XFContentContainer();

    /**
     * @descr   Destructure, all contents will be deleted too.
     */
    virtual ~XFContentContainer();

public:
    /**
     * @descr   Add content.
     */
    virtual void    Add(XFContent *pContent);

    rtl::Reference<XFContent> GetLastContent();
    void    RemoveLastContent();
    /**
     * @descr   convience function for add text content.
     */
    void    Add(const OUString& text);

    /**
     * @descr   return the number of contents in the container.
     */
    int     GetCount() const;

    /**
     * @descr   get content by index.
     */
    rtl::Reference<XFContent> GetContent(sal_uInt32 index) const;

    /**
     * @descr   clear all contents in the container.
     */
    void    Reset();

    /**
     * @descr   helper function, find first content by type.
     */
    rtl::Reference<XFContent> FindFirstContent(enumXFContent type);

    /**
     * @descr   return the content type.
     */
    virtual enumXFContent   GetContentType() override;
    /**
     * @descr   Output to OOo model or local file.
     */
    virtual void ToXml(IXFStream *pStrm) override;

private:
    std::vector< rtl::Reference<XFContent> >    m_aContents;
};

inline rtl::Reference<XFContent> XFContentContainer::GetContent(sal_uInt32 index) const
{
    if (index > m_aContents.size()-1)
        return NULL;
    return m_aContents[index];
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
