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
#ifndef _SW_TYPEDETECT_HXX
#define _SW_TYPEDETECT_HXX

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>

class SfxMedium;

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace uno
            {
                class Any;
            }
            namespace lang
            {
                class XMultiServiceFactory;
            }
            namespace frame
            {
                class XFrame;
            }
            namespace beans
            {
                struct PropertyValue;
            }
        }
    }
}

#include <sfx2/sfxuno.hxx>
class SfxMedium;
class SfxFilter;

#define REFERENCE ::com::sun::star::uno::Reference
#define SEQUENCE ::com::sun::star::uno::Sequence
#define RUNTIME_EXCEPTION ::com::sun::star::uno::RuntimeException

class SwFilterDetect : public ::cppu::WeakImplHelper2< ::com::sun::star::document::XExtendedFilterDetection, ::com::sun::star::lang::XServiceInfo >
{
    static sal_uLong    DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter );
    static sal_uLong    GlobDetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter );
public:
                            SwFilterDetect( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
    virtual                 ~SwFilterDetect();

    SFX_DECL_XSERVICEINFO

    //----------------------------------------------------------------------------------
    // XExtendedFilterDetect
    //----------------------------------------------------------------------------------
    virtual ::rtl::OUString SAL_CALL detect( SEQUENCE< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( RUNTIME_EXCEPTION );
};

#endif
