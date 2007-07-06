/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mimeconfighelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 10:18:58 $
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

#ifndef _COMPHELPER_MIMECONFIGHELPER_HXX_
#define _COMPHELPER_MIMECONFIGHELPER_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_VERBDESCRIPTOR_HPP_
#include <com/sun/star/embed/VerbDescriptor.hpp>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif


namespace comphelper {

class COMPHELPER_DLLPUBLIC MimeConfigurationHelper
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xConfigProvider;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xObjectConfig;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xVerbsConfig;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xMediaTypeConfig;

public:

    MimeConfigurationHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );


    static ::rtl::OUString GetStringClassIDRepresentation( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID );

    static ::com::sun::star::uno::Sequence< sal_Int8 > GetSequenceClassIDRepresentation( const ::rtl::OUString& aClassID );


    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                                            GetConfigurationByPath( const ::rtl::OUString& aPath );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetObjConfiguration();

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetVerbsConfiguration();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetMediaTypeConfiguration();


    ::rtl::OUString GetDocServiceNameFromFilter( const ::rtl::OUString& aFilterName );

    ::rtl::OUString GetDocServiceNameFromMediaType( const ::rtl::OUString& aMediaType );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjPropsFromConfigEntry(
                        const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xObjectProps );

    sal_Bool GetVerbByShortcut( const ::rtl::OUString& aVerbShortcut,
                                ::com::sun::star::embed::VerbDescriptor& aDescriptor );

    ::rtl::OUString GetExplicitlyRegisteredObjClassID( const ::rtl::OUString& aMediaType );


    // retrieving object description from configuration
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByStringClassID(
                                                                const ::rtl::OUString& aStringClassID );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByClassID(
                                                const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByMediaType(
                                                const ::rtl::OUString& aMediaType );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByFilter(
                                                const ::rtl::OUString& aFilterName );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByDocumentName(
                                                const ::rtl::OUString& aDocumentName );

    // retrieving object factory from configuration
    ::rtl::OUString GetFactoryNameByStringClassID( const ::rtl::OUString& aStringClassID );
    ::rtl::OUString GetFactoryNameByClassID( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID );
    ::rtl::OUString GetFactoryNameByDocumentName( const ::rtl::OUString& aDocName );
    ::rtl::OUString GetFactoryNameByMediaType( const ::rtl::OUString& aMediaType );

    // typedetection related
    ::rtl::OUString UpdateMediaDescriptorWithFilterName(
                        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr,
                        sal_Bool bIgnoreType );
    ::rtl::OUString UpdateMediaDescriptorWithFilterName(
                        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr,
                        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aObject );
    sal_Bool AddFilterNameCheckOwnFile(
                        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr );

    ::rtl::OUString GetDefaultFilterFromServiceName( const ::rtl::OUString& aServName, sal_Int32 nVersion );

    static sal_Bool ClassIDsEqual( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID1,
                        const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID2 );
    static ::com::sun::star::uno::Sequence< sal_Int8 > GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 );

    ::com::sun::star::uno::Sequence<sal_Int8> GetSequenceClassIDFromObjectName(const ::rtl::OUString& _sObjectName) ;

};

}

#endif // _COMPHELPER_MIMECONFIGHELPER_HXX_

