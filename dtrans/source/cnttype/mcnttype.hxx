/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mcnttype.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:59:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _MCNTTYPE_HXX_
#define _MCNTTYPE_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPE_HPP_
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#endif

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

