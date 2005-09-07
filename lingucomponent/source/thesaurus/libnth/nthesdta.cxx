/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nthesdta.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:44:00 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include "nthesdta.hxx"
#include <linguistic/misc.hxx>

// #include "lngsvcmgr.hxx"


using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

Meaning::Meaning(
#if 0
            const OUString &rTerm, INT16 nLang,
            const PropertyHelper_Thes &rHelper ) :
#else
            const OUString &rTerm, INT16 nLang) :
#endif

    aTerm       (rTerm),
    nLanguage   (nLang),
    aSyn        ( Sequence< OUString >(1) )

{
#if 0
    // this is for future use by a german thesaurus when one exists
    bIsGermanPreReform = rHelper.IsGermanPreReform;
#endif
}


Meaning::~Meaning()
{
}


OUString SAL_CALL Meaning::getMeaning()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aTerm;
}


Sequence< OUString > SAL_CALL Meaning::querySynonyms()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
        return aSyn;
}


void Meaning::SetSynonyms( const Sequence< OUString > &rSyn )
{
        MutexGuard      aGuard( GetLinguMutex() );
        aSyn = rSyn;
}

void Meaning::SetMeaning( const OUString &rTerm )
{
        MutexGuard      aGuard( GetLinguMutex() );
        aTerm = rTerm;
}

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic






