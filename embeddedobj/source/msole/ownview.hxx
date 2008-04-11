/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ownview.hxx,v $
 * $Revision: 1.4 $
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

#ifndef __OWNVIEW_HXX_
#define __OWNVIEW_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase2.hxx>

#include <osl/mutex.hxx>

class OwnView_Impl : public ::cppu::WeakImplHelper2 < ::com::sun::star::util::XCloseListener,
                                                      ::com::sun::star::document::XEventListener >
{
    ::osl::Mutex m_aMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xModel;

    ::rtl::OUString m_aTempFileURL;
    ::rtl::OUString m_aNativeTempURL;

    ::rtl::OUString m_aFilterName;

    sal_Bool m_bBusy;

    sal_Bool m_bUseNative;

private:
    sal_Bool CreateModelFromURL( const ::rtl::OUString& aFileURL );

    sal_Bool CreateModel( sal_Bool bUseNative );

    sal_Bool ReadContentsAndGenerateTempFile( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xStream, sal_Bool bParseHeader );

    void CreateNative();

    ::rtl::OUString GetFilterNameFromExtentionAndInStream(
                                const ::rtl::OUString& aNameWithExtention,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInputStream );

public:
    OwnView_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xStream );
    virtual ~OwnView_Impl();

    sal_Bool Open();

    void Close();

    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
};

#endif

