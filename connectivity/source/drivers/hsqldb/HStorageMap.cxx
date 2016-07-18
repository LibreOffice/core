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

#include "hsqldb/HStorageMap.hxx"
#include <comphelper/types.hxx>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <uno/mapping.hxx>
#include <algorithm>

namespace connectivity
{

    namespace hsqldb
    {

        using namespace ::com::sun::star::uno;
        using namespace ::com::sun::star::lang;
        using namespace ::com::sun::star::embed;
        using namespace ::com::sun::star::io;

        StreamHelper::StreamHelper(const Reference< XStream>& _xStream)
            : m_xStream(_xStream)
        {
        }

        StreamHelper::~StreamHelper()
        {
            try
            {
                m_xStream.clear();
                m_xSeek.clear();
                if ( m_xInputStream.is() )
                {
                    m_xInputStream->closeInput();
                    m_xInputStream.clear();
                }
                // this is done implicity by the closing of the input stream
                else if ( m_xOutputStream.is() )
                {
                    m_xOutputStream->closeOutput();
                    try
                    {
                        ::comphelper::disposeComponent(m_xOutputStream);
                    }
                    catch(const DisposedException&)
                    {
                    }
                    catch(const Exception&)
                    {
                        OSL_FAIL("Could not dispose OutputStream");
                    }
                    m_xOutputStream.clear();
                }
            }
            catch(const Exception&)
            {
                OSL_FAIL("Exception caught!");
            }
        }

        Reference< XInputStream> const & StreamHelper::getInputStream()
        {
            if ( !m_xInputStream.is() )
                m_xInputStream = m_xStream->getInputStream();
            return m_xInputStream;
        }

        Reference< XOutputStream> const & StreamHelper::getOutputStream()
        {
            if ( !m_xOutputStream.is() )
                m_xOutputStream = m_xStream->getOutputStream();
            return m_xOutputStream;
        }

        Reference< XSeekable> const & StreamHelper::getSeek()
        {
            if ( !m_xSeek.is() )
                m_xSeek.set(m_xStream,UNO_QUERY);
            return m_xSeek;
        }

        css::uno::Reference<css::embed::XStorage> StorageData::mapStorage()
            const
        {
            css::uno::Environment env(css::uno::Environment::getCurrent());
            if (!(env.is() && storageEnvironment.is())) {
                throw css::uno::RuntimeException("cannot get environments");
            }
            if (env.get() == storageEnvironment.get()) {
                return storage;
            } else {
                css::uno::Mapping map(storageEnvironment, env);
                if (!map.is()) {
                    throw css::uno::RuntimeException("cannot get mapping");
                }
                css::uno::Reference<css::embed::XStorage> mapped;
                map.mapInterface(
                    reinterpret_cast<void **>(&mapped), storage.get(),
                    cppu::UnoType<css::embed::XStorage>::get());
                return mapped;
            }
        }

        TStorages& lcl_getStorageMap()
        {
            static TStorages s_aMap;
            return s_aMap;
        }

        OUString lcl_getNextCount()
        {
            static sal_Int32 s_nCount = 0;
            return OUString::number(s_nCount++);
        }

        OUString StorageContainer::removeURLPrefix(const OUString& _sURL,const OUString& _sFileURL)
        {
            return _sURL.copy(_sFileURL.getLength()+1);
        }

        OUString StorageContainer::removeOldURLPrefix(const OUString& _sURL)
        {
            OUString sRet = _sURL;
#if defined(_WIN32)
            sal_Int32 nIndex = sRet.lastIndexOf('\\');
#else
            sal_Int32 nIndex = sRet.lastIndexOf('/');
#endif
            if ( nIndex != -1 )
            {
                sRet = _sURL.copy(nIndex+1);
            }
            return sRet;

        }
        /*****************************************************************************/
        /* convert jstring to rtl_uString */

        OUString StorageContainer::jstring2ustring(JNIEnv * env, jstring jstr)
        {
            if (env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_FAIL("ExceptionClear");
            }
            OUString aStr;
            if ( jstr )
            {
                jboolean bCopy(true);
                const jchar* pChar = env->GetStringChars(jstr,&bCopy);
                jsize len = env->GetStringLength(jstr);
                aStr = OUString(
                    reinterpret_cast<sal_Unicode const *>(pChar), len);

                if(bCopy)
                    env->ReleaseStringChars(jstr,pChar);
            }

            if (env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_FAIL("ExceptionClear");
            }
            return aStr;
        }


        OUString StorageContainer::registerStorage(const Reference< XStorage>& _xStorage,const OUString& _sURL)
        {
            OSL_ENSURE(_xStorage.is(),"Storage is NULL!");
            TStorages& rMap = lcl_getStorageMap();
            // check if the storage is already in our map
            TStorages::const_iterator aFind = ::std::find_if(rMap.begin(),rMap.end(),
                [&_xStorage] (const TStorages::value_type& storage) {
                    return storage.second.mapStorage() == _xStorage;
                });

            if ( aFind == rMap.end() )
            {
                aFind = rMap.insert(TStorages::value_type(lcl_getNextCount(), {_xStorage, css::uno::Environment::getCurrent(), _sURL, TStreamMap()})).first;
            }

            return aFind->first;
        }

        TStorages::mapped_type StorageContainer::getRegisteredStorage(const OUString& _sKey)
        {
            TStorages::mapped_type aRet;
            TStorages& rMap = lcl_getStorageMap();
            TStorages::const_iterator aFind = rMap.find(_sKey);
            OSL_ENSURE(aFind != rMap.end(),"Storage could not be found in list!");
            if ( aFind != rMap.end() )
                aRet = aFind->second;

            return aRet;
        }

        OUString StorageContainer::getRegisteredKey(const Reference< XStorage>& _xStorage)
        {
            OUString sKey;
            OSL_ENSURE(_xStorage.is(),"Storage is NULL!");
            TStorages& rMap = lcl_getStorageMap();
            // check if the storage is already in our map
            TStorages::const_iterator aFind = ::std::find_if(rMap.begin(),rMap.end(),
                [&_xStorage] (const TStorages::value_type& storage) {
                    return storage.second.mapStorage() == _xStorage;
                });

            if ( aFind != rMap.end() )
                sKey = aFind->first;
            return sKey;
        }

        void StorageContainer::revokeStorage(const OUString& _sKey,const Reference<XTransactionListener>& _xListener)
        {
            TStorages& rMap = lcl_getStorageMap();
            TStorages::iterator aFind = rMap.find(_sKey);
            if ( aFind != rMap.end() )
            {
                try
                {
                    if ( _xListener.is() )
                    {
                        Reference<XTransactionBroadcaster> xBroad(aFind->second.mapStorage(),UNO_QUERY);
                        if ( xBroad.is() )
                            xBroad->removeTransactionListener(_xListener);
                        Reference<XTransactedObject> xTrans(aFind->second.mapStorage(),UNO_QUERY);
                        if ( xTrans.is() )
                            xTrans->commit();
                    }
                }
                catch(const Exception&)
                {
                }
                rMap.erase(aFind);
            }
        }

        TStreamMap::mapped_type StorageContainer::registerStream(JNIEnv * env,jstring name, jstring key,sal_Int32 _nMode)
        {
            TStreamMap::mapped_type pHelper;
            TStorages& rMap = lcl_getStorageMap();
            OUString sKey = jstring2ustring(env,key);
            TStorages::iterator aFind = rMap.find(sKey);
            OSL_ENSURE(aFind != rMap.end(),"Storage could not be found in list!");
            if ( aFind != rMap.end() )
            {
                TStorages::mapped_type aStoragePair = StorageContainer::getRegisteredStorage(sKey);
                auto storage = aStoragePair.mapStorage();
                OSL_ENSURE(storage.is(),"No Storage available!");
                if ( storage.is() )
                {
                    OUString sOrgName = StorageContainer::jstring2ustring(env,name);
                    OUString sName = removeURLPrefix(sOrgName,aStoragePair.url);
                    TStreamMap::iterator aStreamFind = aFind->second.streams.find(sName);
                    OSL_ENSURE( aStreamFind == aFind->second.streams.end(),"A Stream was already registered for this object!");
                    if ( aStreamFind != aFind->second.streams.end() )
                    {
                        pHelper = aStreamFind->second;
                    }
                    else
                    {
                        try
                        {
                            try
                            {
                                pHelper.reset(new StreamHelper(storage->openStreamElement(sName,_nMode)));
                            }
                            catch(const Exception&)
                            {
                                OUString sStrippedName = removeOldURLPrefix(sOrgName);

                                if ( ((_nMode & ElementModes::WRITE) != ElementModes::WRITE ) )
                                {
                                    bool bIsStream = true;
                                    try
                                    {
                                       bIsStream = storage->isStreamElement(sStrippedName);
                                    }
                                    catch(const Exception&)
                                    {
                                        bIsStream = false;
                                    }
                                    if ( !bIsStream )
                                        return pHelper; // readonly file without data stream
                                }
                                pHelper.reset( new StreamHelper(storage->openStreamElement( sStrippedName, _nMode ) ) );
                            }
                            aFind->second.streams.insert(TStreamMap::value_type(sName,pHelper));
                        }
                        catch(const Exception& e)
                        {
#if OSL_DEBUG_LEVEL > 0
                            OString sMessage( "[HSQLDB-SDBC] caught an exception while opening a stream\n" );
                            sMessage += "Name: ";
                            sMessage += OString( sName.getStr(), sName.getLength(), osl_getThreadTextEncoding() );
                            sMessage += "\nMode: 0x";
                            if ( _nMode < 16 )
                                sMessage += "0";
                            sMessage += OString::number( _nMode, 16 ).toAsciiUpperCase();
                            OSL_FAIL( sMessage.getStr() );
#endif
                            StorageContainer::throwJavaException(e,env);
                        }
                    }
                }
            }
            return pHelper;
        }

        void StorageContainer::revokeStream( JNIEnv * env,jstring name, jstring key)
        {
            TStorages& rMap = lcl_getStorageMap();
            TStorages::iterator aFind = rMap.find(jstring2ustring(env,key));
            OSL_ENSURE(aFind != rMap.end(),"Storage could not be found in list!");
            if ( aFind != rMap.end() )
                aFind->second.streams.erase(removeURLPrefix(jstring2ustring(env,name),aFind->second.url));
        }

        TStreamMap::mapped_type StorageContainer::getRegisteredStream( JNIEnv * env,jstring name, jstring key)
        {
            TStreamMap::mapped_type  pRet;
            TStorages& rMap = lcl_getStorageMap();
            TStorages::const_iterator aFind = rMap.find(jstring2ustring(env,key));
            OSL_ENSURE(aFind != rMap.end(),"Storage could not be found in list!");
            if ( aFind != rMap.end() )
            {
                TStreamMap::const_iterator aStreamFind = aFind->second.streams.find(removeURLPrefix(jstring2ustring(env,name),aFind->second.url));
                if ( aStreamFind != aFind->second.streams.end() )
                    pRet = aStreamFind->second;
            }

            return pRet;
        }

        void StorageContainer::throwJavaException(const Exception& _aException,JNIEnv * env)
        {
            if (env->ExceptionCheck())
                env->ExceptionClear();
            OString cstr( OUStringToOString(_aException.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
            env->ThrowNew(env->FindClass("java/io/IOException"), cstr.getStr());
        }

    }   // namespace hsqldb


}
// namespace connectivity


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
