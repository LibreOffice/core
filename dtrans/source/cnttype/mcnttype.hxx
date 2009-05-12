/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mcnttype.hxx,v $
 * $Revision: 1.7 $
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


#ifndef _MCNTTYPE_HXX_
#define _MCNTTYPE_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <cppuhelper/compbase1.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>

#include <map>

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

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

