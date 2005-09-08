/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HStorageMap.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:15:15 $
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
#ifndef CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX
#define CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONLISTENER_HPP_
#include <com/sun/star/embed/XTransactionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
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
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream>        getStream();
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

