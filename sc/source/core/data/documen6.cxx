/*************************************************************************
 *
 *  $RCSfile: documen6.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-23 20:23:25 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "scitems.hxx"
#include <svx/scripttypeitem.hxx>

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "document.hxx"
#include "cell.hxx"
#include "cellform.hxx"

using namespace com::sun::star;

#define SC_BREAKITER_SERVICE    "com.sun.star.i18n.BreakIterator"

//
//  this file is compiled with exceptions enabled
//  put functions here that need exceptions!
//

// -----------------------------------------------------------------------

BYTE ScDocument::GetCellScriptType( ScBaseCell* pCell, ULONG nNumberFormat )
{
    if ( !pCell )
        return 0;       // empty

    BYTE nStored = pCell->GetScriptType();
    if ( nStored != SC_SCRIPTTYPE_UNKNOWN )         // stored value valid?
        return nStored;                             // use stored value

    BYTE nRet = 0;

    String aStr;
    Color* pColor;
    ScCellFormat::GetString( pCell, nNumberFormat, aStr, &pColor, *pFormTable );

    if (aStr.Len())
    {
        //! keep the BreakIterator as member

        uno::Reference<uno::XInterface> xInterface = xServiceManager->createInstance(
                            rtl::OUString::createFromAscii( SC_BREAKITER_SERVICE ) );
        uno::Reference<i18n::XBreakIterator> xBreakIter( xInterface, uno::UNO_QUERY );
        if ( xBreakIter.is() )
        {
            rtl::OUString aText = aStr;
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

    pCell->SetScriptType( nRet );       // store for later calls

    return nRet;
}

BYTE ScDocument::GetScriptType( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell )
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

    ULONG nFormat = ((const SfxUInt32Item*)GetEffItem( nCol, nRow, nTab, ATTR_VALUE_FORMAT ))->GetValue();
    return GetCellScriptType( pCell, nFormat );
}


