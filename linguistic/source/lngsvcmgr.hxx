/*************************************************************************
 *
 *  $RCSfile: lngsvcmgr.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-28 11:37:57 $
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

#ifndef _LINGUISTIC_LNGSVCMGR_HXX_
#define _LINGUISTIC_LNGSVCMGR_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase4.hxx> // helper for implementations

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>  //OMultiTypeInterfaceContainerHelper
#endif


#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC2_XLINGUSERVICEMANAGER_HPP_
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XAVAILABLELOCALES_HPP_
#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#endif

#include <vcl/timer.hxx>

#include "misc.hxx"
#include "defs.hxx"

class SpellCheckerDispatcher;
class HyphenatorDispatcher;
class ThesaurusDispatcher;
class SvcInfoArray;
class LngSvcMgrListenerHelper;

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XLinguServiceEventBroadcaster;
    class XSpellChecker;
    class XHyphenator;
    class XThesaurus;
} } } }

///////////////////////////////////////////////////////////////////////////


class LngSvcMgr :
    public cppu::WeakImplHelper4
    <
        com::sun::star::linguistic2::XLinguServiceManager,
        com::sun::star::linguistic2::XAvailableLocales,
        com::sun::star::lang::XComponent,
        com::sun::star::lang::XServiceInfo
    >
{
    ::cppu::OInterfaceContainerHelper                   aEvtListeners;

    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker >  xSpellDsp;
    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    xHyphDsp;
    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XThesaurus >     xThesDsp;

    com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener >        xListenerHelper;

    com::sun::star::uno::Sequence<
        com::sun::star::lang::Locale >                  aAvailSpellLocales;
    com::sun::star::uno::Sequence<
        com::sun::star::lang::Locale >                  aAvailHyphLocales;
    com::sun::star::uno::Sequence<
        com::sun::star::lang::Locale >                  aAvailThesLocales;

    SpellCheckerDispatcher *                            pSpellDsp;
    HyphenatorDispatcher *                              pHyphDsp;
    ThesaurusDispatcher *                               pThesDsp;

    LngSvcMgrListenerHelper *                           pListenerHelper;

    SvcInfoArray *                                      pAvailSpellSvcs;
    SvcInfoArray *                                      pAvailHyphSvcs;
    SvcInfoArray *                                      pAvailThesSvcs;

    Timer aSaveTimer;

    BOOL bDisposing;
    BOOL bIsModified;
    BOOL bHasAvailSpellLocales;
    BOOL bHasAvailHyphLocales;
    BOOL bHasAvailThesLocales;

    // disallow copy-constructor and assignment-operator for now
    LngSvcMgr(const LngSvcMgr &);
    LngSvcMgr & operator = (const LngSvcMgr &);

    void    GetAvailableSpellSvcs_Impl();
    void    GetAvailableHyphSvcs_Impl();
    void    GetAvailableThesSvcs_Impl();
    void    GetListenerHelper_Impl();
    void    GetSpellCheckerDsp_Impl();
    void    GetHyphenatorDsp_Impl();
    void    GetThesaurusDsp_Impl();

    void    SetCfgServiceLists( SpellCheckerDispatcher &rSpellDsp );
    void    SetCfgServiceLists( HyphenatorDispatcher &rHyphDsp );
    void    SetCfgServiceLists( ThesaurusDispatcher &rThesDsp );

    BOOL    SaveCfgSvcs( const String &rServiceName );

    DECL_LINK( TimeOut, Timer* );

public:
    LngSvcMgr();
    virtual ~LngSvcMgr();

    // XLinguServiceManager
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XSpellChecker > SAL_CALL
        getSpellChecker()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenator > SAL_CALL
        getHyphenator()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XThesaurus > SAL_CALL
        getThesaurus()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        addLinguServiceManagerListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XEventListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        removeLinguServiceManagerListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XEventListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getAvailableServices(
                const ::rtl::OUString& rServiceName,
                const ::com::sun::star::lang::Locale& rLocale )
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setConfiguredServices(
                const ::rtl::OUString& rServiceName,
                const ::com::sun::star::lang::Locale& rLocale,
                const ::com::sun::star::uno::Sequence<
                    ::rtl::OUString >& rServiceImplNames )
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getConfiguredServices(
                const ::rtl::OUString& rServiceName,
                const ::com::sun::star::lang::Locale& rLocale )
            throw(::com::sun::star::uno::RuntimeException);

    // XAvailableLocales
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::lang::Locale > SAL_CALL
        getAvailableLocales(
                const ::rtl::OUString& rServiceName )
            throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL
        dispose()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        addEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XEventListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        removeEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XEventListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
        getImplementationName()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        supportsService( const ::rtl::OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw(::com::sun::star::uno::RuntimeException);


    static inline ::rtl::OUString
        getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
        getSupportedServiceNames_Static() throw();

    BOOL    AddLngSvcEvtBroadcaster(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );
    BOOL    RemoveLngSvcEvtBroadcaster(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );
};


inline ::rtl::OUString LngSvcMgr::getImplementationName_Static()
{
    return A2OU( "com.sun.star.lingu2.LngSvcMgr" );
}


///////////////////////////////////////////////////////////////////////////

#endif

