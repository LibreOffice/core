/*************************************************************************
 *
 *  $RCSfile: tdoc_storage.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 17:40:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

#ifndef INCLUDED_TDOC_STORAGE_HXX
#define INCLUDED_TDOC_STORAGE_HXX

#include <map>

#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "com/sun/star/embed/XStorage.hpp"

namespace tdoc_ucp {

    enum StorageAccessMode
    {
        READ,       // storage might be writable as well
        READ_ONLY,  // storage must not be writable
        READ_WRITE_NOCREATE,
        READ_WRITE_CREATE
    };

    class Storage;
    class OfficeDocumentsManager;

    class StorageElementFactory : public salhelper::SimpleReferenceObject
    {
    public:
        StorageElementFactory(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory > & xSMgr,
            const rtl::Reference< OfficeDocumentsManager > & xDocsMgr );
        ~StorageElementFactory();

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        createTemporaryStorage()
            throw ( com::sun::star::uno::Exception,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        createStorage( const rtl::OUString & rUri, StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
        createInputStream( const rtl::OUString & rUri,
                           const rtl::OUString & rPassword )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::packages::WrongPasswordException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >
        createOutputStream( const rtl::OUString & rUri,
                            const rtl::OUString & rPassword )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::packages::WrongPasswordException,
                    com::sun::star::uno::RuntimeException );
    private:
        friend class Storage;

        void releaseElement( Storage * pElement ) SAL_THROW(());

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        queryParentStorage( const rtl::OUString & rUri,
                            StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        queryStorage( const com::sun::star::uno::Reference<
                        com::sun::star::embed::XStorage > & xParentStorage,
                      const rtl::OUString & rUri,
                      StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XStream >
        queryStream( const com::sun::star::uno::Reference<
                        com::sun::star::embed::XStorage > & xParentStorage,
                     const rtl::OUString & rPassword,
                     const rtl::OUString & rUri,
                     StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::packages::WrongPasswordException,
                    com::sun::star::uno::RuntimeException );

        struct ltstrbool
        {
            bool operator()(
                const std::pair< rtl::OUString, bool > & s1,
                const std::pair< rtl::OUString, bool > & s2 ) const
            {
                if ( s1.first < s2.first )
                    return true;
                else if ( s1.first == s2.first )
                    return ( !s1.second && s2.second );
                else
                    return false;
            }
        };

        // key: pair< storageuri, iswritable >
        typedef std::map<
            std::pair< rtl::OUString, bool >, Storage *, ltstrbool > StorageMap;

        StorageMap m_aMap;
        osl::Mutex m_aMutex;
        rtl::Reference< OfficeDocumentsManager > m_xDocsMgr;
        com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    };

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_STORAGE_HXX */
