/*************************************************************************
 *
 *  $RCSfile: HStorageMap.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-06-27 08:24:51 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#include "hsqldb/HStorageMap.hxx"

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONBROADCASTER_HPP_
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#include <osl/thread.h>

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
                m_xStream = NULL;
                m_xSeek = NULL;
                if ( m_xInputStream.is() )
                {
                    m_xInputStream->closeInput();
                    m_xInputStream = NULL;
                }
                if ( m_xOutputStream.is() )
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
                        e;
                        OSL_ENSURE(0,"Could not dispose OutputStream");
                    }
                    m_xOutputStream = NULL;
                }
            }
            catch(Exception& )
            {
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
        Reference< XStream> StreamHelper::getStream()
        {
            return m_xStream;
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
            const char *    cstr;
            rtl_uString *   ustr    = NULL;
            cstr    = env->GetStringUTFChars(jstr, NULL);
            if (JNI_FALSE != env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_ENSURE(0,"ExceptionClear");
            }
            rtl_uString_newFromAscii(&ustr, cstr);
            env->ReleaseStringUTFChars(jstr, cstr);
            if (JNI_FALSE != env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_ENSURE(0,"ExceptionClear");
            }
            return ustr ? ::rtl::OUString(ustr,SAL_NO_ACQUIRE) : ::rtl::OUString();
        }

        // -----------------------------------------------------------------------------
        ::rtl::OUString StorageContainer::registerStorage(const Reference< XStorage>& _xStorage,const ::rtl::OUString& _sURL)
        {
            OSL_ENSURE(_xStorage.is(),"Storage is NULL!");
            TStorages& rMap = lcl_getStorageMap();
            // check if the storage is already in our map
            TStorages::iterator aFind = ::std::find_if(rMap.begin(),rMap.end(),
                                        ::std::compose1(
                                            ::std::bind2nd(::std::equal_to<Reference<XStorage> >(),_xStorage)
                                            ,::std::compose1(::std::select1st<TStorageURLPair>(),::std::compose1(::std::select1st<TStorages::mapped_type>(),::std::select2nd<TStorages::value_type>())))
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
                                        ::std::compose1(
                                            ::std::bind2nd(::std::equal_to<Reference<XStorage> >(),_xStorage)
                                            ,::std::compose1(::std::select1st<TStorageURLPair>(),::std::compose1(::std::select1st<TStorages::mapped_type>(),::std::select2nd<TStorages::value_type>())))
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
/*                              if ( _nMode == ElementModes::READ )
                                    _nMode |= ElementModes::SEEKABLE;
*/
                                pHelper.reset(new StreamHelper(aStoragePair.first.first->openStreamElement(sName,_nMode)));
                            }
                            catch(Exception& )
                            {
                                ::rtl::OUString sName = removeOldURLPrefix(sOrgName);
                                pHelper.reset(new StreamHelper(aStoragePair.first.first->openStreamElement(sName,_nMode)));
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
