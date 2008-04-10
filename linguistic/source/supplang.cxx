/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: supplang.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_linguistic.hxx"
#include <tools/string.hxx>
#include <tools/fsys.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <tools/debug.hxx>
#include <svtools/pathoptions.hxx>
#include <unotools/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/ucbhelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/XDesktop.hpp>

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/XDictionary1.hpp>
#include <com/sun/star/linguistic2/DictionaryType.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>

#include "misc.hxx"
#include "lngprops.hxx"
#include "supplang.hxx"

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

INT16 & LanguageState::GetState( ModuleName eModule )
{
    if ( MOD_SPELL == eModule )
        return nSpellState;
    else if ( MOD_HYPH == eModule )
        return nHyphState;
    else
    {
        DBG_ASSERT( MOD_THES == eModule, "unexpected argument" );
        return nThesState;
    }
}


LanguageState * GetLanguageState( INT16 nLanguage, LanguageState *pArray )
{
    if (!pArray)
        return 0;

    LanguageState   *pState = pArray;
    INT16 nLang;
    while ((nLang = pState->nLanguage) != LANGUAGE_NONE  &&  nLang != nLanguage)
        ++pState;
    return nLang == LANGUAGE_NONE ? 0 : pState;
}

///////////////////////////////////////////////////////////////////////////

SuppLanguages::SuppLanguages( LanguageState *pState, USHORT nCount)
{
    for (USHORT i = 0;  i < nCount;  ++i)
        aLanguages.Insert( pState[i].nLanguage, new INT16( LANG_QUERY ) );
}


SuppLanguages::~SuppLanguages()
{
    INT16 *pItem = aLanguages.First();
    while (pItem)
    {
        INT16 *pTmp = pItem;
        pItem = aLanguages.Next();
        delete pTmp;
    }
}


BOOL SuppLanguages::HasLanguage( INT16 nLanguage ) const
{
    BOOL bRes = FALSE;
    SuppLanguages *pThis = (SuppLanguages *) this;
    INT16 *pEntry = pThis->aLanguages.Seek( nLanguage );
    if (pEntry)
    {
        INT16 nState = *pEntry;
        if (LANG_QUERY == nState)
        {
            nState = CheckLanguage( nLanguage );
            pThis->aLanguages.Replace( nLanguage, new INT16( nState ) );
        }
        bRes = LANG_OK == nState;
    }
    return bRes;
}


const Sequence< INT16 > SuppLanguages::GetLanguages() const
{
    INT32 nLen = aLanguages.Count();
    Sequence< INT16 > aRes( nLen );
    INT16 *pRes = aRes.getArray();
    USHORT nCnt = 0;
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        INT16 nLanguage = sal::static_int_cast< INT16 >(aLanguages.GetObjectKey( i ));
        if (HasLanguage( nLanguage ))
            pRes[ nCnt++ ] = nLanguage;
    }
    aRes.realloc( nCnt );
    return aRes;
}

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic


