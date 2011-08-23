/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SFX_BINDETECT_HXX
#define _SFX_BINDETECT_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <bf_sfx2/sfxuno.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace lang
            {
                class XMultiServiceFactory;
            }
            namespace beans
            {
                struct PropertyValue;
            }
        }
    }
}

#define REFERENCE ::com::sun::star::uno::Reference
#define SEQUENCE ::com::sun::star::uno::Sequence
#define RUNTIME_EXCEPTION ::com::sun::star::uno::RuntimeException

namespace binfilter {

class BinFilterDetect : public ::cppu::WeakImplHelper2< ::com::sun::star::document::XExtendedFilterDetection, ::com::sun::star::lang::XServiceInfo >
{
    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > m_xFact;


public:
                            BinFilterDetect( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
    virtual                 ~BinFilterDetect();

    SFX_DECL_XSERVICEINFO

    //----------------------------------------------------------------------------------
    // XExtendedFilterDetect
    //----------------------------------------------------------------------------------
    virtual ::rtl::OUString SAL_CALL detect( SEQUENCE< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( RUNTIME_EXCEPTION );
};

}//end of namespace binfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
