/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documen6.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 10:52:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "scitems.hxx"
#include <svx/scripttypeitem.hxx>

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "document.hxx"
#include "cell.hxx"
#include "cellform.hxx"
#include "patattr.hxx"
#include "scrdata.hxx"
#include "poolhelp.hxx"

using namespace com::sun::star;

#define SC_BREAKITER_SERVICE    "com.sun.star.i18n.BreakIterator"

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
        uno::Reference< uno::XInterface > xInterface = xServiceManager->createInstance(
                            ::rtl::OUString::createFromAscii( SC_BREAKITER_SERVICE ) );
        pScriptTypeData->xBreakIter = uno::Reference< i18n::XBreakIterator >( xInterface, uno::UNO_QUERY );
        DBG_ASSERT( pScriptTypeData->xBreakIter.is(), "can't get BreakIterator" );
    }
    return pScriptTypeData->xBreakIter;
}

BOOL ScDocument::HasStringWeakCharacters( const String& rString )
{
    BYTE nRet = 0;
    if (rString.Len())
    {
        uno::Reference<i18n::XBreakIterator> xBreakIter = GetBreakIterator();
        if ( xBreakIter.is() )
        {
            rtl::OUString aText = rString;
            sal_Int32 nLen = aText.getLength();

            sal_Int32 nPos = 0;
            do
            {
                sal_Int16 nType = xBreakIter->getScriptType( aText, nPos );
                if ( nType == i18n::ScriptType::WEAK )
                    return TRUE;                            // found

                nPos = xBreakIter->endOfScript( aText, nPos, nType );
            }
            while ( nPos >= 0 && nPos < nLen );
        }
    }

    return FALSE;       // none found
}

BYTE ScDocument::GetStringScriptType( const String& rString )
{

    BYTE nRet = 0;
    if (rString.Len())
    {
        uno::Reference<i18n::XBreakIterator> xBreakIter = GetBreakIterator();
        if ( xBreakIter.is() )
        {
            rtl::OUString aText = rString;
            sal_Int32 nLen = aText.getLength();

            sal_Int32 nPos = 0;
            do
            {
                sal_Int16 nType = xBreakIter->getScriptType( aText, nPos );
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
                nPos = xBreakIter->endOfScript( aText, nPos, nType );
            }
            while ( nPos >= 0 && nPos < nLen );
        }
    }
    return nRet;
}

BYTE ScDocument::GetCellScriptType( ScBaseCell* pCell, ULONG nNumberFormat )
{
    if ( !pCell )
        return 0;       // empty

    BYTE nStored = pCell->GetScriptType();
    if ( nStored != SC_SCRIPTTYPE_UNKNOWN )         // stored value valid?
        return nStored;                             // use stored value

    String aStr;
    Color* pColor;
    ScCellFormat::GetString( pCell, nNumberFormat, aStr, &pColor, *xPoolHelper->GetFormTable() );

    BYTE nRet = GetStringScriptType( aStr );

    pCell->SetScriptType( nRet );       // store for later calls

    return nRet;
}

BYTE ScDocument::GetScriptType( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell )
{
    // if cell is not passed, take from document

    if (!pCell)
    {
        pCell = GetCell( ScAddress( nCol, nRow, nTab ) );
        if ( !pCell )
            return 0;       // empty
    }

    // if script type is set, don't have to get number formats

    BYTE nStored = pCell->GetScriptType();
    if ( nStored != SC_SCRIPTTYPE_UNKNOWN )         // stored value valid?
        return nStored;                             // use stored value

    // include number formats from conditional formatting

    const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
    if (!pPattern) return 0;
    const SfxItemSet* pCondSet = NULL;
    if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
        pCondSet = GetCondResult( nCol, nRow, nTab );

    ULONG nFormat = pPattern->GetNumberFormat( xPoolHelper->GetFormTable(), pCondSet );
    return GetCellScriptType( pCell, nFormat );
}


