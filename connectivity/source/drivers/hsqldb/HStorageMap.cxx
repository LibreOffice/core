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
#include "diagnose_ex.h"
#include <osl/thread.h>

#include <o3tl/compat_functional.hxx>

//........................................................................
namespace connectivity
{
//........................................................................
    namespace hsqldb
    {
    //........................................................................
        using namespace ::com::sun::star::uno;
        using namespace ::com::sun::star::lang;
        using namespace ::com::sun::star::embed;
        using namespace ::com::sun::star::io;

        StreamHelper::StreamHelper(const Reference< XStream>& _xStream)
            : m_xStream(_xStream)
        {
        }
        // -----------------------------------------------------------------------------
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
        // -----------------------------------------------------------------------------
        Reference< XInputStream> StreamHelper::getInputStream()
        {
            if ( !m_xInputStream.is() )
                m_xInputStream = m_xStream->getInputStream();
            return m_xInputStream;
        }
        // -----------------------------------------------------------------------------
        Reference< XOutputStream> StreamHelper::getOutputStream()
        {
            if ( !m_xOutputStream.is() )
                m_xOutputStream = m_xStream->getOutputStream();
            return m_xOutputStream;
        }
        // -----------------------------------------------------------------------------
        Reference< XSeekable> StreamHelper::getSeek()
        {
            if ( !m_xSeek.is() )
                m_xSeek.set(m_xStream,UNO_QUERY);
            return m_xSeek;
        }
        // -----------------------------------------------------------------------------
        TStorages& lcl_getStorageMap()
        {
            static TStorages s_aMap;
            return s_aMap;
        }
        // -----------------------------------------------------------------------------
        OUString lcl_getNextCount()
        {
            static sal_Int32 s_nCount = 0;
            return OUString::number(s_nCount++);
        }
        // -----------------------------------------------------------------------------
        OUString StorageContainer::removeURLPrefix(const OUString& _sURL,const OUString& _sFileURL)
        {
            return _sURL.copy(_sFileURL.getLength()+1);
        }
        // -----------------------------------------------------------------------------
        OUString StorageContainer::removeOldURLPrefix(const OUString& _sURL)
        {
            OUString sRet = _sURL;
#if defined(WNT)
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
            if (JNI_FALSE != env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_FAIL("ExceptionClear");
            }
            OUString aStr;
            if ( jstr )
            {
                jboolean bCopy(sal_True);
                const jchar* pChar = env->GetStringChars(jstr,&bCopy);
                jsize len = env->GetStringLength(jstr);
                aStr = OUString(pChar,len);

                if(bCopy)
                    env->ReleaseStringChars(jstr,pChar);
            }

            if (JNI_FALSE != env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_FAIL("ExceptionClear");
            }
            return aStr;
        }

        // -----------------------------------------------------------------------------
        OUString StorageContainer::registerStorage(const Reference< XStorage>& _xStorage,const OUString& _sURL)
        {
            OSL_ENSURE(_xStorage.is(),"Storage is NULL!");
            TStorages& rMap = lcl_getStorageMap();
            // check if the storage is already in our map
            TStorages::iterator aFind = ::std::find_if(rMap.begin(),rMap.end(),
                                        ::o3tl::compose1(
                                            ::std::bind2nd(::std::equal_to<Reference<XStorage> >(),_xStorage)
                                            ,::o3tl::compose1(::o3tl::select1st<TStorageURLPair>(),::o3tl::compose1(::o3tl::select1st<TStorages::mapped_type>(),::o3tl::select2nd<TStorages::value_type>())))
                    );
            if ( aFind == rMap.end() )
            {
                aFind = rMap.insert(TStorages::value_type(lcl_getNextCount(),TStorages::mapped_type(TStorageURLPair(_xStorage,_sURL),TStreamMap()))).first;
            }

            return aFind->first;
        }
        // -----------------------------------------------------------------------------
        TStorages::mapped_type StorageContainer::getRegisteredStorage(const OUString& _sKey)
        {
            TStorages::mapped_type aRet;
            TStorages& rMap = lcl_getStorageMap();
            TStorages::iterator aFind = rMap.find(_sKey);
            OSL_ENSURE(aFind != rMap.end(),"Storage could not be found in list!");
            if ( aFind != rMap.end() )
                aRet = aFind->second;

            return aRet;
        }
        // -----------------------------------------------------------------------------
        OUString StorageContainer::getRegisteredKey(const Reference< XStorage>& _xStorage)
        {
            OUString sKey;
            OSL_ENSURE(_xStorage.is(),"Storage is NULL!");
            TStorages& rMap = lcl_getStorageMap();
            // check if the storage is already in our map
            TStorages::iterator aFind = ::std::find_if(rMap.begin(),rMap.end(),
                                        ::o3tl::compose1(
                                            ::std::bind2nd(::std::equal_to<Reference<XStorage> >(),_xStorage)
                                            ,::o3tl::compose1(::o3tl::select1st<TStorageURLPair>(),::o3tl::compose1(::o3tl::select1st<TStorages::mapped_type>(),::o3tl::select2nd<TStorages::value_type>())))
                    );
            if ( aFind != rMap.end() )
                sKey = aFind->first;
            return sKey;
        }
        // -----------------------------------------------------------------------------
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
                        Reference<XTransactionBroadcaster> xBroad(aFind->second.first.first,UNO_QUERY);
                        if ( xBroad.is() )
                            xBroad->removeTransactionListener(_xListener);
                        Reference<XTransactedObject> xTrans(aFind->second.first.first,UNO_QUERY);
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
        // -----------------------------------------------------------------------------
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
                OSL_ENSURE(aStoragePair.first.first.is(),"No Storage available!");
                if ( aStoragePair.first.first.is() )
                {
                    OUString sOrgName = StorageContainer::jstring2ustring(env,name);
                    OUString sName = removeURLPrefix(sOrgName,aStoragePair.first.second);
                    TStreamMap::iterator aStreamFind = aFind->second.second.find(sName);
                    OSL_ENSURE( aStreamFind == aFind->second.second.end(),"A Stream was already registered for this object!");
                    if ( aStreamFind != aFind->second.second.end() )
                    {
                        pHelper = aStreamFind->second;
                    }
                    else
                    {
                        try
                        {
                            try
                            {
                                pHelper.reset(new StreamHelper(aStoragePair.first.first->openStreamElement(sName,_nMode)));
                            }
                            catch(const Exception&)
                            {
                                OUString sStrippedName = removeOldURLPrefix(sOrgName);

                                if ( ((_nMode & ElementModes::WRITE) != ElementModes::WRITE ) )
                                {
                                    sal_Bool bIsStream = sal_True;
                                    try
                                    {
                                       bIsStream = aStoragePair.first.first->isStreamElement(sStrippedName);
                                    }
                                    catch(const Exception&)
                                    {
                                        bIsStream = sal_False;
                                    }
                                    if ( !bIsStream )
                                        return pHelper; // readonly file without data stream
                                }
                                pHelper.reset( new StreamHelper(aStoragePair.first.first->openStreamElement( sStrippedName, _nMode ) ) );
                            }
                            aFind->second.second.insert(TStreamMap::value_type(sName,pHelper));
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
        // -----------------------------------------------------------------------------
        void StorageContainer::revokeStream( JNIEnv * env,jstring name, jstring key)
        {
            TStorages& rMap = lcl_getStorageMap();
            TStorages::iterator aFind = rMap.find(jstring2ustring(env,key));
            OSL_ENSURE(aFind != rMap.end(),"Storage could not be found in list!");
            if ( aFind != rMap.end() )
                aFind->second.second.erase(removeURLPrefix(jstring2ustring(env,name),aFind->second.first.second));
        }
        // -----------------------------------------------------------------------------
        TStreamMap::mapped_type StorageContainer::getRegisteredStream( JNIEnv * env,jstring name, jstring key)
        {
            TStreamMap::mapped_type  pRet;
            TStorages& rMap = lcl_getStorageMap();
            TStorages::iterator aFind = rMap.find(jstring2ustring(env,key));
            OSL_ENSURE(aFind != rMap.end(),"Storage could not be found in list!");
            if ( aFind != rMap.end() )
            {
                TStreamMap::iterator aStreamFind = aFind->second.second.find(removeURLPrefix(jstring2ustring(env,name),aFind->second.first.second));
                if ( aStreamFind != aFind->second.second.end() )
                    pRet = aStreamFind->second;
            }

            return pRet;
        }
        // -----------------------------------------------------------------------------
        void StorageContainer::throwJavaException(const Exception& _aException,JNIEnv * env)
        {
            if (JNI_FALSE != env->ExceptionCheck())
                env->ExceptionClear();
            OString cstr( OUStringToOString(_aException.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
            env->ThrowNew(env->FindClass("java/io/IOException"), cstr.getStr());
        }
    //........................................................................
    }   // namespace hsqldb
    //........................................................................
//........................................................................
}
// namespace connectivity
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
