/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpcontentidentifier.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:23:36 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _FTP_FTPCONTENTIDENTIFIER_HXX_
#define _FTP_FTPCONTENTIDENTIFIER_HXX_

#include <vector>
#include "curl.hxx"
#include <curl/easy.h>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#include "ftpdirp.hxx"
#include "ftpurl.hxx"


namespace ftp {


    class FTPContentProvider;


    class FTPContentIdentifier
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::ucb::XContentIdentifier
    {
    public:

        FTPContentIdentifier(const rtl::OUString& ident);

        ~FTPContentIdentifier();

        // XInterface

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();

        // XTypeProvider

        virtual
        com::sun::star::uno::Sequence<com::sun::star::uno::Type> SAL_CALL
        getTypes(
        )
            throw(
                com::sun::star::uno::RuntimeException
            );

        virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(
        )
            throw(
                com::sun::star::uno::RuntimeException
            );


        // XContentIdentifier

        virtual ::rtl::OUString SAL_CALL
        getContentIdentifier(
        )
            throw (
                com::sun::star::uno::RuntimeException
            );

        virtual ::rtl::OUString SAL_CALL
        getContentProviderScheme(
        )
            throw (
                ::com::sun::star::uno::RuntimeException
            );


    private:

        rtl::OUString m_ident;
    };

}


#endif
