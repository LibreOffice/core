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

#ifndef INCLUDED_DTRANS_SOURCE_CNTTYPE_MCNTTYPE_HXX
#define INCLUDED_DTRANS_SOURCE_CNTTYPE_MCNTTYPE_HXX

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
    CMimeContentType( const OUString& aCntType );

    // XMimeContentType

    virtual OUString SAL_CALL getMediaType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getMediaSubtype(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getFullMediaType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getParameters(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasParameter( const OUString& aName )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getParameterValue( const OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    void SAL_CALL init( const OUString& aCntType ) throw( com::sun::star::lang::IllegalArgumentException );
    void SAL_CALL getSym( void );
    void SAL_CALL acceptSym( const OUString& pSymTlb );
    void SAL_CALL skipSpaces( void );
    void SAL_CALL type( void );
    void SAL_CALL subtype( void );
    void SAL_CALL trailer( void );
    OUString SAL_CALL pName( );
    OUString SAL_CALL pValue( );
    OUString SAL_CALL quotedPValue( );
    OUString SAL_CALL nonquotedPValue( );
    void SAL_CALL comment( void );
    bool SAL_CALL isInRange( const OUString& aChr, const OUString& aRange );

private:
    ::osl::Mutex                             m_aMutex;
    OUString                            m_MediaType;
    OUString                            m_MediaSubtype;
    OUString                            m_ContentType;
    std::map< OUString, OUString > m_ParameterMap;
    sal_Int32                                m_nPos;
    OUString                            m_nxtSym;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
