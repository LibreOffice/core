/*************************************************************************
 *
 *  $RCSfile: HStorageMap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-09 12:16:07 $
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
#ifndef CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX
#define CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
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
        typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >, TStreamMap> TStoragePair;
        DECLARE_STL_USTRINGACCESS_MAP(TStoragePair,TStorages);
        /** contains all storages so far accessed.
        */
        class StorageContainer
        {
        public:
            static ::rtl::OUString registerStorage(const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>& _xStorage,const ::rtl::OUString& _sURL);
            static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> getRegisteredStorage(const ::rtl::OUString& _sKey);
            static void revokeStorage(const ::rtl::OUString& _sKey);

            static TStreamMap::mapped_type registerStream(JNIEnv * env,jstring name, jstring key,sal_Int32 _nMode);
            static void revokeStream(JNIEnv * env,jstring name, jstring key);
            static TStreamMap::mapped_type getRegisteredStream( JNIEnv * env, jstring name, jstring key);

            static ::rtl::OUString jstring2ustring(JNIEnv * env, jstring jstr);
            static ::rtl::OUString removeURLPrefix(const ::rtl::OUString& _sURL);
        };
    //........................................................................
    }   // namespace hsqldb
    //........................................................................
//........................................................................
}   // namespace connectivity
//........................................................................
#endif // CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX

