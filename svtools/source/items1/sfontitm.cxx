/*************************************************************************
 *
 *  $RCSfile: sfontitm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:18:38 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif

#ifndef _SFONTITM_HXX
#include <sfontitm.hxx>
#endif

//============================================================================
//
//  class SfxFontItem
//
//============================================================================

TYPEINIT1(SfxFontItem, SfxPoolItem);

//============================================================================
// virtual
int SfxFontItem::operator ==(const SfxPoolItem & rItem) const
{
    const SfxFontItem * pFontItem = PTR_CAST(SfxFontItem, &rItem);
    return pFontItem && m_bHasFont == pFontItem->m_bHasFont
           && m_bHasColor == pFontItem->m_bHasColor
           && m_bHasFillColor == pFontItem->m_bHasFillColor
           && (!m_bHasColor || m_aColor == pFontItem->m_aColor)
           && (!m_bHasFillColor || m_aFillColor == pFontItem->m_aFillColor)
           && (!m_bHasFont || m_bKerning == pFontItem->m_bKerning
                              && m_bShadow == pFontItem->m_bShadow
                              && m_bOutline == pFontItem->m_bOutline
                              && m_bWordLine == pFontItem->m_bWordLine
                              && m_nOrientation == pFontItem->m_nOrientation
                              && m_nStrikeout == pFontItem->m_nStrikeout
                              && m_nUnderline == pFontItem->m_nUnderline
                              && m_nItalic == pFontItem->m_nItalic
                              && m_nWidthType == pFontItem->m_nWidthType
                              && m_nWeight == pFontItem->m_nWeight
                              && m_nPitch == pFontItem->m_nPitch
                              && m_nFamily == pFontItem->m_nFamily
                              && m_nLanguage == pFontItem->m_nLanguage
                              && m_nCharSet == pFontItem->m_nCharSet
                              && m_aFillColor == pFontItem->m_aFillColor
                              && m_aColor == pFontItem->m_aColor
                              && m_aSize == pFontItem->m_aSize
                              && m_aStyleName == pFontItem->m_aStyleName
                              && m_aName == pFontItem->m_aName);
}

//============================================================================
// virtual
SfxPoolItem * SfxFontItem::Create(SvStream & rStream, USHORT) const
{
    VersionCompat aItemCompat(rStream, STREAM_READ);
    SfxFontItem * pItem = new SfxFontItem(Which());
    {
        VersionCompat aFontCompat(rStream, STREAM_READ);
        readByteString(rStream, pItem->m_aName);
        readByteString(rStream, pItem->m_aStyleName);
        rStream >> pItem->m_aSize;
        sal_Int16 nCharSet = 0;
        rStream >> nCharSet;
        pItem->m_nCharSet = rtl_TextEncoding(nCharSet);
        rStream >> pItem->m_nFamily >> pItem->m_nPitch >> pItem->m_nWeight
                >> pItem->m_nUnderline >> pItem->m_nStrikeout
                >> pItem->m_nItalic;
        sal_Int16 nLanguage = 0;
        rStream >> nLanguage;
        pItem->m_nLanguage = LanguageType(nLanguage);
        rStream >> pItem->m_nWidthType >> pItem->m_nOrientation;
        sal_Int8 nWordLine = 0;
        rStream >> nWordLine;
        pItem->m_bWordLine = nWordLine != 0;
        sal_Int8 nOutline = 0;
        rStream >> nOutline;
        pItem->m_bOutline = nOutline != 0;
        sal_Int8 nShadow = 0;
        rStream >> nShadow;
        pItem->m_bShadow = nShadow != 0;
        sal_Int8 nKerning = 0;
        rStream >> nKerning;
        pItem->m_bKerning = nKerning != 0;
    }
    pItem->m_aColor.Read(rStream, TRUE);
    pItem->m_aFillColor.Read(rStream, TRUE);
    sal_Int16 nFlags = 0;
    rStream >> nFlags;
    pItem->m_bHasFont = (nFlags & 4) != 0;
    pItem->m_bHasColor = (nFlags & 1) != 0;
    pItem->m_bHasFillColor = (nFlags & 2) != 0;
    return pItem;
}

//============================================================================
// virtual
SvStream & SfxFontItem::Store(SvStream & rStream, USHORT) const
{
    VersionCompat aItemCompat(rStream, STREAM_WRITE, 1);
    {
        VersionCompat aFontCompat(rStream, STREAM_WRITE, 1);
        writeByteString(rStream, m_aName);
        writeByteString(rStream, m_aStyleName);
        rStream << m_aSize << sal_Int16(m_nCharSet)
                << m_nFamily << m_nPitch << m_nWeight << m_nUnderline
                << m_nStrikeout << m_nItalic << sal_Int16(m_nLanguage)
                << m_nWidthType << m_nOrientation << sal_Int8(m_bWordLine)
                << sal_Int8(m_bOutline) << sal_Int8(m_bShadow)
                << sal_Int8(m_bKerning);
    }
    SAL_CONST_CAST(Color &, m_aColor).Write(rStream, TRUE);
    SAL_CONST_CAST(Color &, m_aFillColor).Write(rStream, TRUE);
    rStream << sal_Int16(m_bHasFont << 2 | m_bHasColor
                          | m_bHasFillColor << 1);
    return rStream;
}

