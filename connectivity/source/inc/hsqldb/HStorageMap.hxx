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
#ifndef CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX
#define CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <boost/shared_ptr.hpp>
#include <comphelper/stl_types.hxx>
#include <jni.h>
//........................................................................
namespace connectivity
{
//........................................................................
    namespace hsqldb
    {
        class StreamHelper
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream>        m_xStream;
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable>      m_xSeek;
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream>  m_xOutputStream;
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>   m_xInputStream;
        public:
            StreamHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream>& _xStream);
            ~StreamHelper();

            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>   getInputStream();
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream>  getOutputStream();
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable>      getSeek();
        };


        DECLARE_STL_USTRINGACCESS_MAP(::boost::shared_ptr<StreamHelper>,TStreamMap);
        typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >, ::rtl::OUString > TStorageURLPair;
        typedef ::std::pair< TStorageURLPair, TStreamMap> TStoragePair;
        DECLARE_STL_USTRINGACCESS_MAP(TStoragePair,TStorages);
        /** contains all storages so far accessed.
        */
        class StorageContainer
        {
        public:
            static ::rtl::OUString registerStorage(const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>& _xStorage,const ::rtl::OUString& _sURL);
            static TStorages::mapped_type getRegisteredStorage(const ::rtl::OUString& _sKey);
            static ::rtl::OUString getRegisteredKey(const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>& _xStorage);
            static void revokeStorage(const ::rtl::OUString& _sKey,const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XTransactionListener>& _xListener);

            static TStreamMap::mapped_type registerStream(JNIEnv * env,jstring name, jstring key,sal_Int32 _nMode);
            static void revokeStream(JNIEnv * env,jstring name, jstring key);
            static TStreamMap::mapped_type getRegisteredStream( JNIEnv * env, jstring name, jstring key);

            static ::rtl::OUString jstring2ustring(JNIEnv * env, jstring jstr);
            static ::rtl::OUString removeURLPrefix(const ::rtl::OUString& _sURL,const ::rtl::OUString& _sFileURL);
            static ::rtl::OUString removeOldURLPrefix(const ::rtl::OUString& _sURL);
            static void throwJavaException(const ::com::sun::star::uno::Exception& _aException,JNIEnv * env);
        };
    //........................................................................
    }   // namespace hsqldb
    //........................................................................
//........................................................................
}   // namespace connectivity

//........................................................................
#endif // CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
