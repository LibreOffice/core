/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sharecontrolfile.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:08:40 $
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

#ifndef _SVT_SHARECONTROLFILE_HXX
#define _SVT_SHARECONTROLFILE_HXX

#include <svtools/svtdllapi.h>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <osl/mutex.hxx>

#define SHARED_OOOUSERNAME_ID   0
#define SHARED_SYSUSERNAME_ID   1
#define SHARED_LOCALHOST_ID     2
#define SHARED_EDITTIME_ID      3
#define SHARED_USERURL_ID       4
#define SHARED_ENTRYSIZE        5

namespace svt {

class SVT_DLLPUBLIC ShareControlFile
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ::rtl::OUString m_aURL;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xInputStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > m_xOutputStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable > m_xSeekable;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XTruncate > m_xTruncate;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > m_aUsersData;

    void OpenStream();
    void Close();
    sal_Bool IsValid()
    {
        return ( m_xFactory.is() && m_xStream.is() && m_xInputStream.is() && m_xOutputStream.is() && m_xSeekable.is() && m_xTruncate.is() );
    }

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > ParseList( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > ParseEntry( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos );
    ::rtl::OUString ParseName( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos );
    ::rtl::OUString EscapeCharacters( const ::rtl::OUString& aSource );
    ::rtl::OUString GetOOOUserName();
    ::rtl::OUString GetCurrentLocalTime();
    ::com::sun::star::uno::Sequence< ::rtl::OUString > GenerateOwnEntry();

public:

    // The constructor will throw exception in case the stream can not be opened
    ShareControlFile( const ::rtl::OUString& aOrigURL, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() );
    ~ShareControlFile();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > GetUsersData();
    void SetUsersDataAndStore( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > >& aUserNames );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > InsertOwnEntry();
    void RemoveEntry( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aOptionalSpecification = ::com::sun::star::uno::Sequence< ::rtl::OUString >() );
    void RemoveFile();
};

}

#endif

