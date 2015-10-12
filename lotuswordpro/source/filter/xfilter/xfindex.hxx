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
 * Represente index source,index body and index index entry.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFINDEX_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFINDEX_HXX
#include <map>
#include <vector>

#include "xfcontent.hxx"
#include "xfcontentcontainer.hxx"
#include "xftabstop.hxx"

class XFIndex;
class XFIndexTemplate;
class XFParagraph;

/**
 * @brief
 * Index template.
 * When index update, the template is used to decide the index body.
 */
class XFIndexTemplate : public XFContent
{
public:
    XFIndexTemplate();

public:
    /**
     * @descr   Set template level.
     */
    void    SetLevel(const OUString& level);
    /**
     * @descr   Set style.
     */
    void    SetStyleName(const OUString& style) override;
    /**
     * @descr   Add a entry in the template.
     */
    void    AddEntry(enumXFIndexTemplate entry, const OUString& styleName = "");

    /**
     * @descr   Add a tab entry in the template.
     */
    void    AddTabEntry(enumXFTab type, double len, sal_Unicode leader = '*', sal_Unicode delimiter='.', const OUString& styleName = "");

    /**
     * @descr   Add a entry in the template.
     */
    void    AddTextEntry(const OUString& sSpan, const OUString& styleName = "");

    virtual void    ToXml(IXFStream *pStrm) override;

private:
    /**
     * @descr   Helper function.
     */
    void    SetTagName(const OUString& tag);

    friend class XFIndex;
private:
    OUString   m_nLevel;
    OUString   m_strStyle;
    enumXFTab  m_eTabType;
    double     m_fTabLength;
    OUString   m_strTabDelimiter;
    OUString   m_strTabLeader;
    OUString   m_strTagName;
    typedef std::pair<enumXFIndexTemplate, OUString> TOCTEMPLATE_ENTRY_TYPE;
    std::vector<TOCTEMPLATE_ENTRY_TYPE> m_aEntries; // template entry + text style
    std::map<sal_uInt16, OUString> m_aTextEntries;
};

/**
 * @brief
 * Index, include index template, index body.
 */
class XFIndex : public XFContentContainer
{
public:
    XFIndex();

    virtual ~XFIndex();

public:
    /**
     * @descr   Set index type.
     */
    void    SetIndexType(enumXFIndex type);

    /**
     * @descr   Add index template entry.
     */
    void    AddTemplate(const OUString& level, const OUString& style, XFIndexTemplate* templ);

    /**
     * @descr   Set if protected index to prevent handy-revise.
     */
    void    SetProtected(bool protect);

    /**
     * @descr   Set separator.
     */
    void    SetSeparator(bool sep);

    virtual void ToXml(IXFStream *pStrm) override;

    void AddTocSource(sal_uInt16 nLevel, const OUString& sStyleName);

private:
    enumXFIndex     m_eType;
    OUString        m_strTitle;
    bool            m_bProtect;
    bool            m_bSeparator;
    XFParagraph*    m_pTitle;

    std::vector<XFIndexTemplate *>  m_aTemplates; // template entry + style

    #define MAX_TOC_LEVEL 10
    std::vector<OUString> m_aTOCSource[MAX_TOC_LEVEL+1];

    sal_uInt32      m_nMaxLevel;
};

inline XFIndexTemplate::XFIndexTemplate()
    : m_eTabType(enumXFTabNone)
    , m_fTabLength(0.0)
{
    m_nLevel = OUString::number(0);
}

inline void XFIndexTemplate::SetLevel(const OUString& level)
{
    m_nLevel = level;
}
inline void XFIndexTemplate::SetStyleName(const OUString& style)
{
    m_strStyle = style;
}

inline void XFIndexTemplate::SetTagName(const OUString& tag)
{
    m_strTagName = tag;
}

inline void XFIndexTemplate::AddEntry(enumXFIndexTemplate entry, const OUString& styleName)
{
    std::pair<enumXFIndexTemplate, OUString> pair(entry, styleName);
    m_aEntries.push_back(pair);
}

inline void XFIndexTemplate::AddTabEntry(enumXFTab type, double len, sal_Unicode leader, sal_Unicode delimiter, const OUString& styleName)
{
    m_eTabType = type;
    m_strTabLeader = OUString( leader );
    m_strTabDelimiter = OUString( delimiter );
    m_fTabLength = len;

    AddEntry(enumXFIndexTemplateTab, styleName);
}

inline void XFIndexTemplate::AddTextEntry(const OUString& sText, const OUString& styleName)
{
    sal_uInt16 nLen = m_aEntries.size();
    AddEntry(enumXFIndexTemplateSpan, styleName);

    m_aTextEntries[nLen] = sText;
}

inline void XFIndex::SetIndexType(enumXFIndex type)
{
    m_eType = type;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
