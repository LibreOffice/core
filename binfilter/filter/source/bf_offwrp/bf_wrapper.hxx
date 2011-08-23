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

#ifndef _BF_WRAPPER_HXX
#define _BF_WRAPPER_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <osl/mutex.hxx>
#include <bf_sfx2/sfxuno.hxx>


#ifdef _MSC_VER
#pragma hdrstop
#endif
namespace binfilter {
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class OfficeApplication;

class bf_OfficeWrapper : public ::cppu::WeakImplHelper3< XInitialization, XComponent, XServiceInfo >
{
    OfficeApplication*									pApp;
    ::osl::Mutex										aMutex;
    ::cppu::OInterfaceContainerHelper					aListeners;

public:
    bf_OfficeWrapper( const Reference < XMultiServiceFactory >& xFactory );
    virtual ~bf_OfficeWrapper();

    SFX_DECL_XSERVICEINFO

    static Reference< XSingleServiceFactory > GetWrapperFactory( Reference< XMultiServiceFactory > & xSMgr );
    static ::rtl::OUString  GetImplementationName_static();

    // XComponent
    virtual void SAL_CALL dispose() throw ( RuntimeException );
    virtual void SAL_CALL addEventListener( const Reference< XEventListener > & aListener) throw ( RuntimeException );
    virtual void SAL_CALL removeEventListener(const Reference< XEventListener > & aListener) throw ( RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw( Exception );
};

}//end of namespace binfilter
#endif // _BF_WRAPPER_HXX

// eof
