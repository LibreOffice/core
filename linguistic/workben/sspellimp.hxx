/*************************************************************************
 *
 *  $RCSfile: sspellimp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:46 $
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

#ifndef _LINGU2_SPELLIMP_HXX_
#define _LINGU2_SPELLIMP_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase6.hxx> // helper for implementations

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEDISPLAYNAME_HPP_
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#endif
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif

#include "misc.hxx"
#include "sprophelp.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;


#define A2OU(x) ::rtl::OUString::createFromAscii( x )

///////////////////////////////////////////////////////////////////////////


class SpellChecker :
    public cppu::WeakImplHelper6
    <
        XSpellChecker,
        XLinguServiceEventBroadcaster,
        XInitialization,
        XComponent,
        XServiceInfo,
        XServiceDisplayName
    >
{
    Sequence< Locale >                      aSuppLocales;
    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    Reference< XPropertyChangeListener >    xPropHelper;
    PropertyHelper_Spell *                  pPropHelper;
    BOOL                                    bDisposing;

    // disallow copy-constructor and assignment-operator for now
    SpellChecker(const SpellChecker &);
    SpellChecker & operator = (const SpellChecker &);

    PropertyHelper_Spell &  GetPropHelper_Impl();
    PropertyHelper_Spell &  GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

    INT16   GetSpellFailure( const OUString &rWord, const Locale &rLocale );
    Reference< XSpellAlternatives >
            GetProposals( const OUString &rWord, const Locale &rLocale );

public:
    SpellChecker();
    virtual ~SpellChecker();

    // XSupportedLocales (for XSpellChecker)
    virtual Sequence< Locale > SAL_CALL
        getLocales()
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLocale( const Locale& rLocale )
            throw(RuntimeException);

    // XSpellChecker
    virtual sal_Bool SAL_CALL
        isValid( const OUString& rWord, const Locale& rLocale,
                const PropertyValues& rProperties )
            throw(IllegalArgumentException,
                  RuntimeException);
    virtual Reference< XSpellAlternatives > SAL_CALL
        spell( const OUString& rWord, const Locale& rLocale,
                const PropertyValues& rProperties )
            throw(IllegalArgumentException,
                  RuntimeException);

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL
        addLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxLstnr )
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        removeLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxLstnr )
            throw(RuntimeException);

    // XServiceDisplayName
    virtual OUString SAL_CALL
        getServiceDisplayName( const Locale& rLocale )
            throw(RuntimeException);

    // XInitialization
    virtual void SAL_CALL
        initialize( const Sequence< Any >& rArguments )
            throw(Exception, RuntimeException);

    // XComponent
    virtual void SAL_CALL
        dispose()
            throw(RuntimeException);
    virtual void SAL_CALL
        addEventListener( const Reference< XEventListener >& rxListener )
            throw(RuntimeException);
    virtual void SAL_CALL
        removeEventListener( const Reference< XEventListener >& rxListener )
            throw(RuntimeException);

    ////////////////////////////////////////////////////////////
    // Service specific part
    //

    // XServiceInfo
    virtual OUString SAL_CALL
        getImplementationName()
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        supportsService( const OUString& rServiceName )
            throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw(RuntimeException);


    static inline OUString
        getImplementationName_Static() throw();
    static Sequence< OUString >
        getSupportedServiceNames_Static() throw();
};

inline OUString SpellChecker::getImplementationName_Static() throw()
{
    return A2OU( "com.sun.star.lingu.examples.SpellChecker" );
}


///////////////////////////////////////////////////////////////////////////

#endif

