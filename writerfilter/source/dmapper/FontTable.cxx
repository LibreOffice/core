/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <FontTable.hxx>
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#endif
#include <vector>
#include <stdio.h>

namespace writerfilter {
namespace dmapper
{

struct FontTable_Impl
{
    std::vector< FontEntry > aFontEntries;
    FontEntry::Pointer_t pCurrentEntry;
    FontTable_Impl() {}
};

FontTable::FontTable() :
    m_pImpl( new FontTable_Impl )
{
}

FontTable::~FontTable()
{
    delete m_pImpl;
}

void FontTable::attribute(Id Name, Value & val)
{
    OSL_ENSURE( m_pImpl->pCurrentEntry, "current entry has to be set here");
    if(!m_pImpl->pCurrentEntry)
        return ;
    int nIntValue = val.getInt();
    ::rtl::OUString sValue = val.getString();
    switch(Name)
    {
        case NS_rtf::LN_CBFFNM1:
            m_pImpl->pCurrentEntry->sFontName1 = sValue;
        break;
        case NS_rtf::LN_PRQ:
            m_pImpl->pCurrentEntry->nPitchRequest = static_cast<sal_Int16>( nIntValue );
        break;
        case NS_rtf::LN_FTRUETYPE:
            m_pImpl->pCurrentEntry->bTrueType = nIntValue == 1 ? true : false;
        break;
        case NS_rtf::LN_UNUSED1_3: //unused
        case NS_rtf::LN_FF: //unused
        case NS_rtf::LN_UNUSED1_7: //unused
        break;
        case NS_rtf::LN_WWEIGHT:
            m_pImpl->pCurrentEntry->nBaseWeight = nIntValue;
        break;
        case NS_rtf::LN_CHS:
            m_pImpl->pCurrentEntry->nTextEncoding = nIntValue;
        break;
        case NS_rtf::LN_IXCHSZALT:
        break;
        case NS_rtf::LN_PANOSE:
            m_pImpl->pCurrentEntry->sPanose += sValue;
        break;
        case NS_rtf::LN_FS:
            m_pImpl->pCurrentEntry->sFontSignature += sValue;
        break;
        case NS_rtf::LN_F:
        break;
        case NS_rtf::LN_ALTFONTNAME:
            m_pImpl->pCurrentEntry->sAlternativeFont = sValue;
        break;
        case NS_rtf::LN_XSZFFN:
        case NS_ooxml::LN_CT_Font_name:
            m_pImpl->pCurrentEntry->sFontName = sValue;
        break;
        default:
        {
            //----> debug
            int nVal = val.getInt();
            ++nVal;
            //<---- debug
        }
    }
}

void FontTable::sprm(Sprm& rSprm)
{
    OSL_ENSURE( m_pImpl->pCurrentEntry, "current entry has to be set here");
    if(!m_pImpl->pCurrentEntry)
        return ;
    sal_uInt32 nSprmId = rSprm.getId();
    (void)nSprmId;

    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    (void)nIntValue;
    rtl::OUString sStringValue = pValue->getString();
}

void FontTable::entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
    //create a new font entry
    OSL_ENSURE( !m_pImpl->pCurrentEntry, "current entry has to be NULL here");
    m_pImpl->pCurrentEntry.reset(new FontEntry);
    ref->resolve(*this);
    //append it to the table
    m_pImpl->aFontEntries.push_back( *m_pImpl->pCurrentEntry );
    m_pImpl->pCurrentEntry.reset();
}

void FontTable::startSectionGroup()
{
}

void FontTable::endSectionGroup()
{
}

void FontTable::startParagraphGroup()
{
}

void FontTable::endParagraphGroup()
{
}

void FontTable::startCharacterGroup()
{
}

void FontTable::endCharacterGroup()
{
}

void FontTable::text(const sal_uInt8*, size_t )
{
}

void FontTable::utext(const sal_uInt8* , size_t)
{
}

void FontTable::props(writerfilter::Reference<Properties>::Pointer_t)
{
}

void FontTable::table(Id, writerfilter::Reference<Table>::Pointer_t)
{
}

void FontTable::substream(Id, ::writerfilter::Reference<Stream>::Pointer_t)
{
}

void FontTable::info(const string& )
{
}

void FontTable::startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > )
{
}

void FontTable::endShape( )
{
}

const FontEntry::Pointer_t FontTable::getFontEntry(sal_uInt32 nIndex)
{
    FontEntry::Pointer_t pRet;
    if(m_pImpl->aFontEntries.size() > nIndex)
    {
        pRet.reset(&m_pImpl->aFontEntries[nIndex]);
    }
    return pRet;
}

sal_uInt32 FontTable::size()
{
    return m_pImpl->aFontEntries.size();
}

}//namespace dmapper
}//namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
