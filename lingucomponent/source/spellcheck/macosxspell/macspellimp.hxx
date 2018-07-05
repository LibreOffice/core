/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _MACSPELLIMP_H_
#define _MACSPELLIMP_H_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase6.hxx> // helper for implementations

#ifdef MACOSX
#include <premac.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#include <postmac.h>
#endif
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <tools/table.hxx>

#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>

#include <lingutil.hxx>

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;


///////////////////////////////////////////////////////////////////////////


class MacSpellChecker :
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
    Sequence< Locale >      aSuppLocales;
    rtl_TextEncoding *      aDEncs;
    Locale *                aDLocs;
    OUString *              aDNames;
    sal_Int32               numdict;
    NSSpellChecker *        macSpell;
    int                     macTag;   //unique tag for this doc

    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    Reference< XPropertyChangeListener >    xPropHelper;
    linguistic::PropertyHelper_Spell *      pPropHelper;
    sal_Bool                                    bDisposing;

    // disallow copy-constructor and assignment-operator for now
    MacSpellChecker(const MacSpellChecker &);
    MacSpellChecker & operator = (const MacSpellChecker &);

    linguistic::PropertyHelper_Spell &  GetPropHelper_Impl();
    linguistic::PropertyHelper_Spell &  GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

    sal_Int16   GetSpellFailure( const OUString &rWord, const Locale &rLocale );
    Reference< XSpellAlternatives > GetProposals( const OUString &rWord, const Locale &rLocale );

public:
    MacSpellChecker();
    virtual ~MacSpellChecker();

    // XSupportedLocales (for XSpellChecker)
    virtual Sequence< Locale > SAL_CALL getLocales() throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) throw(RuntimeException);

    // XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const OUString& rWord, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException);
    virtual Reference< XSpellAlternatives > SAL_CALL spell( const OUString& rWord, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException);

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL addLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL removeLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException);

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const Locale& rLocale ) throw(RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& rArguments ) throw(Exception, RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw(RuntimeException);
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException);
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);


    static inline OUString  getImplementationName_Static() throw();
    static Sequence< OUString > getSupportedServiceNames_Static() throw();
};

inline OUString MacSpellChecker::getImplementationName_Static() throw()
{
    return A2OU( "org.openoffice.lingu.MacOSXSpellChecker" );
}



///////////////////////////////////////////////////////////////////////////

#endif

