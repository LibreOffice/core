/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: supplang.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:30:11 $
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
#include "precompiled_linguistic.hxx"

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

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


