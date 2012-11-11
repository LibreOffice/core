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

#include "scitems.hxx"
#include <editeng/scripttypeitem.hxx>

#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include "document.hxx"
#include "cell.hxx"
#include "cellform.hxx"
#include "patattr.hxx"
#include "scrdata.hxx"
#include "poolhelp.hxx"
#include "attrib.hxx"

using namespace com::sun::star;

//
//  this file is compiled with exceptions enabled
//  put functions here that need exceptions!
//

// -----------------------------------------------------------------------

const uno::Reference< i18n::XBreakIterator >& ScDocument::GetBreakIterator()
{
    if ( !pScriptTypeData )
        pScriptTypeData = new ScScriptTypeData;
    if ( !pScriptTypeData->xBreakIter.is() )
    {
        pScriptTypeData->xBreakIter = i18n::BreakIterator::create( comphelper::getComponentContext(xServiceManager) );
    }
    return pScriptTypeData->xBreakIter;
}

bool ScDocument::HasStringWeakCharacters( const rtl::OUString& rString )
{
    if (!rString.isEmpty())
    {
        uno::Reference<i18n::XBreakIterator> xBreakIter = GetBreakIterator();
        if ( xBreakIter.is() )
        {
            sal_Int32 nLen = rString.getLength();

            sal_Int32 nPos = 0;
            do
            {
                sal_Int16 nType = xBreakIter->getScriptType( rString, nPos );
                if ( nType == i18n::ScriptType::WEAK )
                    return true;                            // found

                nPos = xBreakIter->endOfScript( rString, nPos, nType );
            }
            while ( nPos >= 0 && nPos < nLen );
        }
    }

    return false;       // none found
}

sal_uInt8 ScDocument::GetStringScriptType( const rtl::OUString& rString )
{

    sal_uInt8 nRet = 0;
    if (!rString.isEmpty())
    {
        uno::Reference<i18n::XBreakIterator> xBreakIter = GetBreakIterator();
        if ( xBreakIter.is() )
        {
            sal_Int32 nLen = rString.getLength();

            sal_Int32 nPos = 0;
            do
            {
                sal_Int16 nType = xBreakIter->getScriptType( rString, nPos );
                switch ( nType )
                {
                    case i18n::ScriptType::LATIN:
                        nRet |= SCRIPTTYPE_LATIN;
                        break;
                    case i18n::ScriptType::ASIAN:
                        nRet |= SCRIPTTYPE_ASIAN;
                        break;
                    case i18n::ScriptType::COMPLEX:
                        nRet |= SCRIPTTYPE_COMPLEX;
                        break;
                    // WEAK is ignored
                }
                nPos = xBreakIter->endOfScript( rString, nPos, nType );
            }
            while ( nPos >= 0 && nPos < nLen );
        }
    }
    return nRet;
}

sal_uInt8 ScDocument::GetCellScriptType( ScBaseCell* pCell, sal_uLong nNumberFormat )
{
    if ( !pCell )
        return 0;       // empty

    sal_uInt8 nStored = pCell->GetScriptType();
    if ( nStored != SC_SCRIPTTYPE_UNKNOWN )         // stored value valid?
        return nStored;                             // use stored value

    rtl::OUString aStr;
    Color* pColor;
    ScCellFormat::GetString( pCell, nNumberFormat, aStr, &pColor, *xPoolHelper->GetFormTable() );

    sal_uInt8 nRet = GetStringScriptType( aStr );

    pCell->SetScriptType( nRet );       // store for later calls

    return nRet;
}

sal_uInt8 ScDocument::GetScriptType( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell )
{
    // if cell is not passed, take from document

    if (!pCell)
    {
        pCell = GetCell( ScAddress( nCol, nRow, nTab ) );
        if ( !pCell )
            return 0;       // empty
    }

    // if script type is set, don't have to get number formats

    sal_uInt8 nStored = pCell->GetScriptType();
    if ( nStored != SC_SCRIPTTYPE_UNKNOWN )         // stored value valid?
        return nStored;                             // use stored value

    // include number formats from conditional formatting

    const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
    if (!pPattern) return 0;
    const SfxItemSet* pCondSet = NULL;
    if ( !((const ScCondFormatItem&)pPattern->GetItem(ATTR_CONDITIONAL)).GetCondFormatData().empty() )
        pCondSet = GetCondResult( nCol, nRow, nTab );

    sal_uLong nFormat = pPattern->GetNumberFormat( xPoolHelper->GetFormTable(), pCondSet );
    return GetCellScriptType( pCell, nFormat );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
