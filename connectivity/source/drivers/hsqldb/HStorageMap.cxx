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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
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

#define ThrowException(env, type, msg) { \
    env->ThrowNew(env->FindClass(type), msg); }


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
                    catch(DisposedException&)
                    {
                    }
                    catch(const Exception& e)
                    {
                        OSL_UNUSED( e );
                        OSL_ENSURE(0,"Could not dispose OutputStream");
                    }
                    m_xOutputStream.clear();
                }
            }
            catch(Exception& ex)
            {
                OSL_UNUSED( ex );
                OSL_ENSURE(0,"Exception catched!");
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
        ::rtl::OUString lcl_getNextCount()
        {
            static sal_Int32 s_nCount = 0;
            return ::rtl::OUString::valueOf(s_nCount++);
        }
        // -----------------------------------------------------------------------------
        ::rtl::OUString StorageContainer::removeURLPrefix(const ::rtl::OUString& _sURL,const ::rtl::OUString& _sFileURL)
        {
            return _sURL.copy(_sFileURL.getLength()+1);
        }
        // -----------------------------------------------------------------------------
        ::rtl::OUString StorageContainer::removeOldURLPrefix(const ::rtl::OUString& _sURL)
        {
            ::rtl::OUString sRet = _sURL;
#if defined(WIN) || defined(WNT)
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

        ::rtl::OUString StorageContainer::jstring2ustring(JNIEnv * env, jstring jstr)
        {
            if (JNI_FALSE != env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_ENSURE(0,"ExceptionClear");
            }
            ::rtl::OUString aStr;
            if ( jstr )
            {
                jboolean bCopy(sal_True);
                const jchar* pChar = env->GetStringChars(jstr,&bCopy);
                jsize len = env->GetStringLength(jstr);
                aStr = ::rtl::OUString(pChar,len);

                if(bCopy)
                    env->ReleaseStringChars(jstr,pChar);
            }

            if (JNI_FALSE != env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_ENSURE(0,"ExceptionClear");
            }
            return aStr;
        }

        // -----------------------------------------------------------------------------
        ::rtl::OUString StorageContainer::registerStorage(const Reference< XStorage>& _xStorage,const ::rtl::OUString& _sURL)
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
        TStorages::mapped_type StorageContainer::getRegisteredStorage(const ::rtl::OUString& _sKey)
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
        ::rtl::OUString StorageContainer::getRegisteredKey(const Reference< XStorage>& _xStorage)
        {
            ::rtl::OUString sKey;
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
        void StorageContainer::revokeStorage(const ::rtl::OUString& _sKey,const Reference<XTransactionListener>& _xListener)
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
                catch(Exception&)
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
            ::rtl::OUString sKey = jstring2ustring(env,key);
            TStorages::iterator aFind = rMap.find(sKey);
            OSL_ENSURE(aFind != rMap.end(),"Storage could not be found in list!");
            if ( aFind != rMap.end() )
            {
                TStorages::mapped_type aStoragePair = StorageContainer::getRegisteredStorage(sKey);
                OSL_ENSURE(aStoragePair.first.first.is(),"No Storage available!");
                if ( aStoragePair.first.first.is() )
                {
                    ::rtl::OUString sOrgName = StorageContainer::jstring2ustring(env,name);
                    ::rtl::OUString sName = removeURLPrefix(sOrgName,aStoragePair.first.second);
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
                            catch(Exception& )
                            {
                                ::rtl::OUString sStrippedName = removeOldURLPrefix(sOrgName);

                                if ( ((_nMode & ElementModes::WRITE) != ElementModes::WRITE ) )
                                {
                                    sal_Bool bIsStream = sal_True;
                                    try
                                    {
                                       bIsStream = aStoragePair.first.first->isStreamElement(sStrippedName);
                                    }
                                    catch(Exception& )
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
                        catch(Exception& e)
                        {
#if OSL_DEBUG_LEVEL > 0
                            ::rtl::OString sMessage( "[HSQLDB-SDBC] caught an exception while opening a stream\n" );
                            sMessage += "Name: ";
                            sMessage += ::rtl::OString( sName.getStr(), sName.getLength(), osl_getThreadTextEncoding() );
                            sMessage += "\nMode: 0x";
                            if ( _nMode < 16 )
                                sMessage += "0";
                            sMessage += ::rtl::OString::valueOf( _nMode, 16 ).toAsciiUpperCase();
                            OSL_ENSURE( false, sMessage.getStr() );
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
            ::rtl::OString cstr( ::rtl::OUStringToOString(_aException.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
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
