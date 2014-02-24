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
#ifndef CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX
#define CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX

#include <sal/config.h>

#include <map>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <boost/shared_ptr.hpp>
#include <jni.h>

namespace connectivity
{

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


        typedef std::map< OUString, ::boost::shared_ptr<StreamHelper> > TStreamMap;
        typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >, OUString > TStorageURLPair;
        typedef ::std::pair< TStorageURLPair, TStreamMap> TStoragePair;
        typedef std::map<OUString, TStoragePair> TStorages;
        /** contains all storages so far accessed.
        */
        class StorageContainer
        {
        public:
            static OUString registerStorage(const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>& _xStorage,const OUString& _sURL);
            static TStorages::mapped_type getRegisteredStorage(const OUString& _sKey);
            static OUString getRegisteredKey(const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>& _xStorage);
            static void revokeStorage(const OUString& _sKey,const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XTransactionListener>& _xListener);

            static TStreamMap::mapped_type registerStream(JNIEnv * env,jstring name, jstring key,sal_Int32 _nMode);
            static void revokeStream(JNIEnv * env,jstring name, jstring key);
            static TStreamMap::mapped_type getRegisteredStream( JNIEnv * env, jstring name, jstring key);

            static OUString jstring2ustring(JNIEnv * env, jstring jstr);
            static OUString removeURLPrefix(const OUString& _sURL,const OUString& _sFileURL);
            static OUString removeOldURLPrefix(const OUString& _sURL);
            static void throwJavaException(const ::com::sun::star::uno::Exception& _aException,JNIEnv * env);
        };

    }   // namespace hsqldb


}   // namespace connectivity


#endif // CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
