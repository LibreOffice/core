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

#ifndef _SFX_FLTLST_HXX
#define _SFX_FLTLST_HXX

//*****************************************************************************************************************
//  includes
//*****************************************************************************************************************
#include <fcontnr.hxx>

#include <cppuhelper/implbase1.hxx>

#include <rtl/ustring.hxx>

#include <osl/mutex.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/util/XFlushable.hpp>

#include <com/sun/star/util/XFlushListener.hpp>

#include <com/sun/star/document/XEventListener.hpp>

#include <com/sun/star/lang/EventObject.hpp>

#include <com/sun/star/uno/RuntimeException.hpp>
namespace binfilter {

//*****************************************************************************************************************
//  declarations
//*****************************************************************************************************************
class SfxFilterListener : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XFlushListener >
{
    // member
    private:
        ::osl::Mutex                                                            m_aMutex      ;
        ::rtl::OUString                                                         m_sFactory    ;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushable >  m_xTypeCache  ;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushable >  m_xFilterCache;
        SfxFilterContainer*                                                     m_pContainer  ;

    // c++ interface
    public:
              SfxFilterListener( const ::rtl::OUString&    sFactory   ,
                                       SfxFilterContainer* pContainer );
             ~SfxFilterListener(                                      );

    // uno interface
    public:
        // XFlushListener
        virtual void SAL_CALL flushed( const ::com::sun::star::lang::EventObject& aSource ) throw( ::com::sun::star::uno::RuntimeException );
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aSource ) throw( ::com::sun::star::uno::RuntimeException );

};  // SfxFilterListener

}//end of namespace binfilter
#endif // _SFX_FLTLST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
