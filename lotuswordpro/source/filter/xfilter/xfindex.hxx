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
/*************************************************************************
 * Change History
   2005-02-02	create and implemente.
 ************************************************************************/
#ifndef		_XFINDEX_HXX
#define		_XFINDEX_HXX
#include <map>
#include <vector>

#include	"xfcontent.hxx"
#include	"ixfproperty.hxx"
#include	"xfcontentcontainer.hxx"
#include	"xftabstop.hxx"

class XFIndex;
class XFIndexEntry;
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
     * @descr	Set template level.
     */
    void	SetLevel(rtl::OUString level);
    /**
     * @descr	Set style.
     */
    void	SetStyleName(rtl::OUString style);
    /**
     * @descr	Add a entry in the template.
     */
    void	AddEntry(enumXFIndexTemplate entry, rtl::OUString styleName = A2OUSTR(""));

    /**
     * @descr	Add a tab entry in the template.
     */
    void	AddTabEntry(enumXFTab type, double len, sal_Unicode leader = '*', sal_Unicode delimiter='.', rtl::OUString styleName = A2OUSTR(""));

    /**
     * @descr	Add a entry in the template.
     */
    void	AddTextEntry(rtl::OUString sSpan, rtl::OUString styleName = A2OUSTR(""));

    /**
     * @descr	clear all index template parts.
     */
    void	Clear();
    virtual void	ToXml(IXFStream *pStrm);

private:
    /**
     * @descr	Helper function.
     */
    void	SetTagName(rtl::OUString tag);

    friend class XFIndex;
private:
    rtl::OUString	m_nLevel;
    rtl::OUString	m_strStyle;
    enumXFTab	m_eTabType;
    double		m_fTabLength;
    rtl::OUString	m_strTabDelimiter;
    rtl::OUString	m_strTabLeader;
    rtl::OUString	m_strTagName;
    typedef std::pair<enumXFIndexTemplate, rtl::OUString> TOCTEMPLATE_ENTRY_TYPE;
    std::vector<TOCTEMPLATE_ENTRY_TYPE>	m_aEntries; // template entry + text style
    std::map<sal_uInt16, rtl::OUString> m_aTextEntries;

    rtl::OUString m_strChapterTextStyle;
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
     * @descr	Set index type.
     */
    void	SetIndexType(enumXFIndex type);

    /**
     * @descr	Set index section style. Perhaps you should create a section style first.
     */
    void	SetSectionStyle(rtl::OUString strSectStyleName);

    /**
     * @descr	Set index title.
     */
    void	SetTitle(rtl::OUString title, rtl::OUString strParaStyle);

    /**
     * @descr	Set index entry template.
     * @param	level [1-10].
     */
    void	AddTemplate(sal_uInt32 level,
        rtl::OUString style,
        enumXFIndexTemplate type1 = enumXFIndexTemplateChapter,
        enumXFIndexTemplate type2 = enumXFIndexTemplateText,
        enumXFIndexTemplate type3 = enumXFIndexTemplateTab,
        enumXFIndexTemplate type4 = enumXFIndexTemplatePage);

    /**
     * @descr	Add index templaet entry.
     */
    void	AddTemplate(rtl::OUString level, rtl::OUString style, XFIndexTemplate* templ);

    /**
     * @descr	Add index entry.
     */
    void	AddEntry(rtl::OUString entry, rtl::OUString strParaStyle);

    /**
     * @descr	return entry count.
     */
    sal_Int32	GetEntryCount();

    /**
     * @descr	Set if protected index to prevent handly-revise.
     */
    void	SetProtected(sal_Bool protect);

    /**
     * @descr	Set separator.
     */
    void	SetSeparator(sal_Bool sep);

    virtual void	ToXml(IXFStream *pStrm);

    void AddTocSource(sal_uInt16 nLevel, const rtl::OUString sStyleName);

    /**
     * @descr	Set default value for index.
     */
    void SetDefaultAlphaIndex(rtl::OUString strDivision,sal_Bool bRunin, sal_Bool bSeparator);

private:
    enumXFIndex		m_eType;
    rtl::OUString		m_strTitle;
    rtl::OUString		m_strSectStyle;
    bool				m_bProtect;
    bool			m_bSeparator;
    XFParagraph		*m_pTitle;

    std::vector<XFIndexTemplate *>	m_aTemplates; // template entry + style

    #define MAX_TOC_LEVEL 10
    std::vector<rtl::OUString> m_aTOCSource[MAX_TOC_LEVEL+1];

    sal_uInt32		m_nMaxLevel;
};

inline XFIndexTemplate::XFIndexTemplate()
{
    m_nLevel = Int32ToOUString(0);
}

inline void XFIndexTemplate::SetLevel(rtl::OUString level)
{
    m_nLevel = level;
}
inline void XFIndexTemplate::SetStyleName(rtl::OUString style)
{
    m_strStyle = style;
}

inline void XFIndexTemplate::SetTagName(rtl::OUString tag)
{
    m_strTagName = tag;
}

inline void XFIndexTemplate::AddEntry(enumXFIndexTemplate entry, rtl::OUString styleName)
{
    std::pair<enumXFIndexTemplate, rtl::OUString> pair(entry, styleName);
    m_aEntries.push_back(pair);
}

inline void XFIndexTemplate::AddTabEntry(enumXFTab type, double len, sal_Unicode leader, sal_Unicode delimiter, rtl::OUString styleName)
{
    m_eTabType = type;
    m_strTabLeader = rtl::OUString( leader );
    m_strTabDelimiter = rtl::OUString( delimiter );
    m_fTabLength = len;

    AddEntry(enumXFIndexTemplateTab, styleName);
}

inline void XFIndexTemplate::AddTextEntry(rtl::OUString sText, rtl::OUString styleName)
{
    sal_uInt16 nLen = m_aEntries.size();
    AddEntry(enumXFIndexTemplateSpan, styleName);

    m_aTextEntries[nLen] = sText;
}


inline void XFIndexTemplate::Clear()
{
    m_aEntries.clear();
}

inline void XFIndex::SetIndexType(enumXFIndex type)
{
    m_eType = type;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
