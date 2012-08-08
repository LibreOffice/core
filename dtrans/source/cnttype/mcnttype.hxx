/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifndef _MCNTTYPE_HXX_
#define _MCNTTYPE_HXX_

#include <cppuhelper/compbase1.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>

#include <map>

class CMimeContentType : public
    cppu::WeakImplHelper1< com::sun::star::datatransfer::XMimeContentType >
{
public:
    CMimeContentType( const rtl::OUString& aCntType );

    //-------------------------------------------
    // XMimeContentType
    //-------------------------------------------

    virtual ::rtl::OUString SAL_CALL getMediaType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getMediaSubtype(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFullMediaType(  ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getParameters(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasParameter( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getParameterValue( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

private:
    void SAL_CALL init( const rtl::OUString& aCntType ) throw( com::sun::star::lang::IllegalArgumentException );
    void SAL_CALL getSym( void );
    void SAL_CALL acceptSym( const rtl::OUString& pSymTlb );
    void SAL_CALL skipSpaces( void );
    void SAL_CALL type( void );
    void SAL_CALL subtype( void );
    void SAL_CALL trailer( void );
    rtl::OUString SAL_CALL pName( );
    rtl::OUString SAL_CALL pValue( );
    rtl::OUString SAL_CALL quotedPValue( );
    rtl::OUString SAL_CALL nonquotedPValue( );
    void SAL_CALL comment( void );
    sal_Bool SAL_CALL isInRange( const rtl::OUString& aChr, const rtl::OUString& aRange );

private:
    ::osl::Mutex                             m_aMutex;
    rtl::OUString                            m_MediaType;
    rtl::OUString                            m_MediaSubtype;
    rtl::OUString                            m_ContentType;
    std::map< rtl::OUString, rtl::OUString > m_ParameterMap;
    sal_Int32                                m_nPos;
    rtl::OUString                            m_nxtSym;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
