/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: thesdta.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:56:45 $
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

#ifndef _LANG_HXX //autogen wg. LANGUAGE_ENGLISH_US
#include <tools/lang.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <misc.hxx>

#include "thesdta.hxx"

//using namespace utl;
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


ThesaurusMeaning::ThesaurusMeaning(const OUString &rText,
                 const OUString &rLookUpText, INT16 nLookUpLang ) :
    aText               (rText),
    aLookUpText         (rLookUpText),
    nLookUpLanguage     (nLookUpLang)
{
}

ThesaurusMeaning::~ThesaurusMeaning()
{
}

OUString SAL_CALL
        ThesaurusMeaning::getMeaning() throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aText;
}

/*
uno::Sequence< OUString > SAL_CALL ThesaurusMeaning::querySynonyms()
        throw(RuntimeException)
{
}
*/

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

