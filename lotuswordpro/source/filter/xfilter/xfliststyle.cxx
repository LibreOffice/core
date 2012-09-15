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
#include    "xfliststyle.hxx"

XFListLevel::XFListLevel()
{
    m_nLevel = 0;
    m_nDisplayLevel = 0;
    m_fIndent = 0;
    m_fMinLabelWidth = 0.499;
    m_fMinLabelDistance = 0;
    m_eAlign = enumXFAlignStart;
    m_eListType = enumXFListLevelNumber;
}

void XFListLevel::ToXml(IXFStream * /*pStrm*/)
{
}

//----------------------------------------------------

void XFListlevelNumber::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("text:level"), Int16ToOUString(m_nLevel) );
    //text:style-name,ignore now.
    m_aNumFmt.ToXml(pStrm);
    if( m_nDisplayLevel )
        pAttrList->AddAttribute( A2OUSTR("text:display-levels"), Int16ToOUString(m_nDisplayLevel) );

    pStrm->StartElement( A2OUSTR("text:list-level-style-number") );

    //<style:properties>...</style:properties>
    pAttrList->Clear();
    if( m_fIndent > FLOAT_MIN )
    {
        pAttrList->AddAttribute( A2OUSTR("text:space-before"), DoubleToOUString(m_fIndent) + A2OUSTR("cm") );
    }
    if( m_fMinLabelWidth > FLOAT_MIN )
    {
        pAttrList->AddAttribute( A2OUSTR("text:min-label-width"), DoubleToOUString(m_fMinLabelWidth) + A2OUSTR("cm") );
    }
    if( m_fMinLabelDistance > FLOAT_MIN )
    {
        pAttrList->AddAttribute( A2OUSTR("text:min-label-distance"), DoubleToOUString(m_fMinLabelDistance) + A2OUSTR("cm") );
    }
    pAttrList->AddAttribute( A2OUSTR("fo:text-align"), GetAlignName(m_eAlign) );


    pStrm->StartElement( A2OUSTR("style:properties") );
    pStrm->EndElement( A2OUSTR("style:properties") );

    pStrm->EndElement( A2OUSTR("text:list-level-style-number") );
}

void    XFListLevelBullet::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("text:level"), Int16ToOUString(m_nLevel) );
    //text:style-name,ignore now.
    m_aNumFmt.ToXml(pStrm);
    //bullet-char
    rtl::OUString   bullet(m_chBullet);
    pAttrList->AddAttribute( A2OUSTR("text:bullet-char"), bullet );

    pStrm->StartElement( A2OUSTR("text:list-level-style-bullet") );

    //<style:properties>...</style:properties>
    pAttrList->Clear();
    if( m_fIndent != 0 )
    {
        pAttrList->AddAttribute( A2OUSTR("text:space-before"), DoubleToOUString(m_fIndent) + A2OUSTR("cm") );
    }
    if( m_fMinLabelWidth != 0 )
    {
        pAttrList->AddAttribute( A2OUSTR("text:min-label-width"), DoubleToOUString(m_fMinLabelWidth) + A2OUSTR("cm") );
    }
    if( m_fMinLabelDistance != 0 )
    {
        pAttrList->AddAttribute( A2OUSTR("text:min-label-distance"), DoubleToOUString(m_fMinLabelDistance) + A2OUSTR("cm") );
    }

    pAttrList->AddAttribute( A2OUSTR("fo:text-align"), GetAlignName(m_eAlign) );

    if( !m_strFontName.isEmpty() )
    {
        pAttrList->AddAttribute( A2OUSTR("style:font-name"), m_strFontName );
    }
    pStrm->StartElement( A2OUSTR("style:properties") );
    pStrm->EndElement( A2OUSTR("style:properties") );

    pStrm->EndElement( A2OUSTR("text:list-level-style-bullet") );
}

//----------------------------------------------------
XFListStyle::XFListStyle()
{
    XFNumFmt    nf;
    nf.SetSuffix( A2OUSTR(".") );
    nf.SetFormat( A2OUSTR("1") );

    for( int i=0; i<10; i++ )
    {
        m_pListLevels[i] = new XFListlevelNumber();
        m_pListLevels[i]->SetListlevelType(enumXFListLevelNumber);
        m_pListLevels[i]->SetMinLabelWidth(0.499);
        m_pListLevels[i]->SetIndent(0.501*(i+1));
        m_pListLevels[i]->SetLevel(i+1);
        ( (XFListlevelNumber*)m_pListLevels[i])->SetNumFmt(nf);
    }
}

XFListStyle::XFListStyle(const XFListStyle& other):XFStyle(other)
{
    enumXFListLevel type;
    for( int i=0; i<10; i++ )
    {
        type = other.m_pListLevels[i]->m_eListType;
        if( type == enumXFListLevelNumber )
        {
            XFListlevelNumber *pNum = (XFListlevelNumber*)other.m_pListLevels[i];
            m_pListLevels[i] = new XFListlevelNumber(*pNum);
        }
        else if( type == enumXFListLevelBullet )
        {
            XFListLevelBullet *pBullet = (XFListLevelBullet*)other.m_pListLevels[i];
            m_pListLevels[i] = new XFListLevelBullet(*pBullet);
        }
        else
            m_pListLevels[i] = NULL;
    }
}

XFListStyle& XFListStyle::operator=(const XFListStyle& other)
{
    enumXFListLevel type;
    for( int i=0; i<10; i++ )
    {
        type = other.m_pListLevels[i]->m_eListType;
        if( type == enumXFListLevelNumber )
        {
            XFListlevelNumber *pNum = (XFListlevelNumber*)m_pListLevels[i];
            m_pListLevels[i] = new XFListlevelNumber(*pNum);
        }
        else if( type == enumXFListLevelBullet )
        {
            XFListLevelBullet *pBullet = (XFListLevelBullet*)m_pListLevels[i];
            m_pListLevels[i] = new XFListLevelBullet(*pBullet);
        }
        else
            m_pListLevels[i] = NULL;
    }
    return *this;
}

XFListStyle::~XFListStyle()
{
    for( int i=0; i<10; i++ )
    {
        if( m_pListLevels[i] )
            delete m_pListLevels[i];
    }
}

void    XFListStyle::SetDisplayLevel(sal_Int32 level, sal_Int16 nDisplayLevel)
{
    assert(level>=1&&level<=10);

    XFListLevel *pLevel = m_pListLevels[level-1];
    if( !pLevel )
    {
        pLevel = new XFListlevelNumber();
        pLevel->SetListlevelType(enumXFListLevelNumber);
        pLevel->SetLevel(level+1);
        pLevel->SetMinLabelWidth(0.499);
        pLevel->SetIndent(0.501*(level+1));
        pLevel->SetDisplayLevel(nDisplayLevel);
        m_pListLevels[level-1] = pLevel;
    }
    else
        pLevel->SetDisplayLevel(nDisplayLevel);
}

void    XFListStyle::SetListPosition(sal_Int32 level,
                double indent,
                double minLabelWidth,
                double minLabelDistance,
                enumXFAlignType align
                )
{
    assert(level>=1&&level<=10);

    XFListLevel *pLevel = m_pListLevels[level-1];
    if( !pLevel )
    {
        pLevel = new XFListLevelBullet();
        pLevel->SetListlevelType(enumXFListLevelNumber);
        pLevel->SetLevel(level+1);
        pLevel->SetIndent(indent);
        pLevel->SetMinLabelWidth(minLabelWidth);
        pLevel->SetMinLabelDistance(minLabelDistance);
        pLevel->SetAlignType(align);
        m_pListLevels[level-1] = pLevel;
    }
    else
    {
        pLevel->SetIndent(indent);
        pLevel->SetMinLabelWidth(minLabelWidth);
        pLevel->SetMinLabelDistance(minLabelDistance);
        pLevel->SetAlignType(align);
    }
}

void    XFListStyle::SetListBullet(sal_Int32 level,
                                   UChar32 bullet,
                                   rtl::OUString fontname,
                                   rtl::OUString prefix,
                                   rtl::OUString suffix
                                   )
{
    assert(level>=1&&level<=10);

    if( m_pListLevels[level-1] )
        delete m_pListLevels[level-1];

    XFListLevelBullet *pLevel = new XFListLevelBullet();
    pLevel->SetPrefix(prefix);
    pLevel->SetSuffix(suffix);
    pLevel->SetBulletChar(bullet);
    pLevel->SetFontName(fontname);
    //
    pLevel->SetListlevelType(enumXFListLevelBullet);
    pLevel->SetMinLabelWidth(0.499);
    pLevel->SetIndent(0.501*level);
    pLevel->SetLevel(level);
    m_pListLevels[level-1] = pLevel;
}

void    XFListStyle::SetListNumber(sal_Int32 level, XFNumFmt& fmt, sal_Int16 start )
{
    assert(level>=1&&level<=10);

    if( m_pListLevels[level-1] )
        delete m_pListLevels[level-1];

    XFListlevelNumber *pLevel = new XFListlevelNumber();
    pLevel->SetNumFmt(fmt);
    pLevel->SetStartValue(start);
    //
    pLevel->SetListlevelType(enumXFListLevelNumber);
    pLevel->SetMinLabelWidth(0.499);
    pLevel->SetIndent(0.501*level);
    pLevel->SetLevel(level);
    m_pListLevels[level-1] = pLevel;
}

void XFListStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("style:name"), GetStyleName() );
    if( !GetParentStyleName().isEmpty() )
        pAttrList->AddAttribute(A2OUSTR("style:parent-style-name"),GetParentStyleName());
    pStrm->StartElement( A2OUSTR("text:list-style") );

    for( int i=0; i<10; i++ )
    {
        XFListLevel *pLevel = m_pListLevels[i];
        if( pLevel )
            pLevel->ToXml(pStrm);
    }

    pStrm->EndElement( A2OUSTR("text:list-style") );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
