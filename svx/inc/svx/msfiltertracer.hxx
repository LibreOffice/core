/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msfiltertracer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:00:48 $
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

#ifndef _MS_FILTERTRACER_HXX
#define _MS_FILTERTRACER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_XTEXTSEARCH_HPP_
#include <com/sun/star/util/XTextSearch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHRESULT_HPP_
#include <com/sun/star/util/SearchResult.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_LOGGING_XLOGGER_HPP_
#include <com/sun/star/util/logging/XLogger.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class FilterConfigItem;
class SVX_DLLPUBLIC MSFilterTracer
{
        FilterConfigItem*   mpCfgItem;
        SvXMLAttributeList* mpAttributeList;
        SvStream*           mpStream;
        sal_Bool            mbEnabled;

        ::com::sun::star::util::SearchOptions maSearchOptions;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               mxFilterTracer;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XTextSearch >             mxTextSearch;

        ::com::sun::star::uno::Reference< ::com::sun::star::util::logging::XLogger >        mxLogger;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    mxHandler;

    public :

        /* The first parameter is expecting the path of the configuration file.
           The second parameter can be used to override properties of the configuration
           file. A "DocumentURL" property can be set in the property
           sequence to provide the current url of the document.
           If the "Path" property is not set in the configuration file, then the path
           of the "DocumentURL" property is used instead. If both are not set, the
           application path is used instead.
           If the "Name" property is not set in the configuration file, then the name
           of the "DocumentURL" property is used instead. If both are not set, the
           name "tracer* is used.
           The file extension will always be "log" */

        MSFilterTracer( const ::rtl::OUString& rConfigurationPath,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pConfigData = NULL );
        ~MSFilterTracer();

        void StartTracing();
        void EndTracing();

        inline sal_Bool IsEnabled() const { return mbEnabled; }

        void StartElement( const ::rtl::OUString& rName, ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttribs );
        void EndElement( const ::rtl::OUString& rName );

        void Trace( const ::rtl::OUString& rElementID, const ::rtl::OUString& rMessage );
        void AddAttribute( const ::rtl::OUString &sName , const ::rtl::OUString &sValue );
        void RemoveAttribute( const ::rtl::OUString& sName );
        void ClearAttributes();

        /* initially all properties of the configuration file and the
           properties which are given to the c'tor can be retrieved */
        ::com::sun::star::uno::Any GetProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any* pDefault = NULL ) const;
        void SetProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rProperty );
};

#endif
