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
* Styles for ordered list and unordered list.
************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFLISTSTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFLISTSTYLE_HXX

#include <xfilter/xfglobal.hxx>
#include <xfilter/xfstyle.hxx>
#include <xfilter/xfnumfmt.hxx>
#include <unicode/utypes.h>
#include <memory>
#include <vector>

class IXFStream;

enum enumXFListLevel
{
    enumXFListLevelBullet,
    enumXFListLevelNumber,
    enumXFListLevelImage
};

/**************************************************************************
 * @descr
 * Bullet style wrapper,the underline bullet type can be:
 *      number,image,or bullet.
 *************************************************************************/
class  XFListLevel
{
public:
    XFListLevel();

    virtual ~XFListLevel(){}

    XFListLevel(XFListLevel const &) = default;
    XFListLevel(XFListLevel &&) = default;
    XFListLevel & operator =(XFListLevel const &) = default;
    XFListLevel & operator =(XFListLevel &&) = default;

    void    SetListlevelType(enumXFListLevel type);

    void    SetLevel(sal_Int16 level);

    void    SetDisplayLevel(sal_Int16 nDisplayLevel);

    void    SetIndent(double indent);

    void    SetMinLabelWidth(double labelWidth);

    void    SetMinLabelDistance(double distance);

    void    SetAlignType(enumXFAlignType align);

    virtual void    ToXml(IXFStream *pStrm);

    friend class XFListStyle;
protected:
    sal_Int16   m_nLevel;
    sal_Int16   m_nDisplayLevel;
    double      m_fIndent;
    double      m_fMinLabelWidth;
    double      m_fMinLabelDistance;
    enumXFAlignType m_eAlign;
    enumXFListLevel m_eListType;
};

class XFListlevelNumber : public XFListLevel
{
public:
    XFListlevelNumber()
    {
        m_eListType = enumXFListLevelNumber;
    }
public:
    void    SetNumFmt(XFNumFmt const & fmt)
    {
        m_aNumFmt = fmt;
    }

    void    SetStartValue(sal_Int16 start)
    {
        m_aNumFmt.SetStartValue(start);
    }

    virtual void    ToXml(IXFStream *pStrm) override;
private:
    XFNumFmt    m_aNumFmt;
};

class XFListLevelBullet : public XFListLevel
{
public:
    XFListLevelBullet()
    {
        m_eListType = enumXFListLevelBullet;
    }

    void    SetBulletChar(OUString const & ch)
    {
        m_chBullet = ch;
    }
    void    SetPrefix(const OUString& prefix)
    {
        m_aNumFmt.SetPrefix(prefix);
    }
    void    SetSuffix(const OUString& suffix)
    {
        m_aNumFmt.SetSuffix(suffix);
    }
    void    SetFontName(const OUString& name)
    {
        m_strFontName = name;
    }

    virtual void    ToXml(IXFStream *pStrm) override;
private:
    XFNumFmt    m_aNumFmt;
    OUString m_chBullet;
    OUString m_strFontName;
};

//not complete.
class XFListLevelImage : public XFListLevel
{
public:
    virtual void    ToXml(IXFStream *pStrm) override;
private:
};

class XFListStyle : public XFStyle
{
public:
    XFListStyle();

    XFListStyle(const XFListStyle& other);

    XFListStyle& operator=(const XFListStyle& other);

    virtual ~XFListStyle() override;

public:
    void    SetDisplayLevel(sal_Int32 level, sal_Int16 nShowLevel);

    void    SetListPosition(sal_Int32 level,
                    double indent,
                    double minLabelWidth,
                    double minLabelDistance,
                    enumXFAlignType align=enumXFAlignStart
                    );

    void    SetListBullet(sal_Int32 level,
                            OUString const & bullet_char,
                            const OUString& fontname,
                            const OUString& prefix,
                            const OUString& suffix
                            );

    void    SetListNumber( sal_Int32 level, XFNumFmt const & numFmt, sal_Int16 nStartValue );

    virtual enumXFStyle GetStyleFamily() override
    {
        return enumXFStyleList;
    }
    virtual void    ToXml(IXFStream *pStrm) override;

private:
    std::unique_ptr<XFListLevel> m_pListLevels[10];
};

inline void XFListLevel::SetListlevelType(enumXFListLevel type)
{
    m_eListType = type;
}

inline void XFListLevel::SetAlignType(enumXFAlignType align)
{
    m_eAlign = align;
}

inline void XFListLevel::SetLevel(sal_Int16 level)
{
    m_nLevel = level;
}

inline void XFListLevel::SetDisplayLevel(sal_Int16 nDisplayLevel)
{
    m_nDisplayLevel = nDisplayLevel;
}

inline void XFListLevel::SetIndent(double indent)
{
    m_fIndent = indent;
}

inline void XFListLevel::SetMinLabelWidth(double labelWidth)
{
    m_fMinLabelWidth = labelWidth;
}

inline void XFListLevel::SetMinLabelDistance(double distance)
{
    m_fMinLabelDistance = distance;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
