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

#ifndef SC_ADDINLIS_HXX
#define SC_ADDINLIS_HXX

#ifndef _SC_ADIASYNC_HXX
#include "adiasync.hxx"			// for ScAddInDocs PtrArr
#endif

#ifndef _COM_SUN_STAR_SHEET_XRESULTLISTENER_HPP_
#include <com/sun/star/sheet/XResultListener.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_XVOLATILERESULT_HPP_
#include <com/sun/star/sheet/XVolatileResult.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
namespace binfilter {



class ScDocument;


class ScAddInListener : public cppu::WeakImplHelper2<
                            ::com::sun::star::sheet::XResultListener,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxBroadcaster
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XVolatileResult> xVolRes;
    ::com::sun::star::uno::Any	aResult;
    ScAddInDocs*				pDocs;			// documents where this is used

    static List					aAllListeners;

                            // always allocated via CreateListener
                            ScAddInListener(
                                ::com::sun::star::uno::Reference<
                                    ::com::sun::star::sheet::XVolatileResult> xVR,
                                ScDocument* pD );

public:
    virtual					~ScAddInListener();

                            // create Listener and put it into global list
    static ScAddInListener*	CreateListener(
                                ::com::sun::star::uno::Reference<
                                    ::com::sun::star::sheet::XVolatileResult> xVR,
                                ScDocument* pDoc );

    static ScAddInListener*	Get( ::com::sun::star::uno::Reference<
                                    ::com::sun::star::sheet::XVolatileResult> xVR );
    static void				RemoveDocument( ScDocument* pDocument );

    BOOL					HasDocument( ScDocument* pDoc ) const	{ return pDocs->Seek_Entry( pDoc ); }
    void					AddDocument( ScDocument* pDoc )			{ pDocs->Insert( pDoc ); }
    const ::com::sun::star::uno::Any& GetResult() const				{ return aResult; }


                            // XResultListener
    virtual void SAL_CALL	modified( const ::com::sun::star::sheet::ResultEvent& aEvent )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XEventListener
    virtual void SAL_CALL	disposing( const ::com::sun::star::lang::EventObject& Source )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};


} //namespace binfilter
#endif

