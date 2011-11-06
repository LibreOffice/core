/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _MS_FILTERTRACER_HXX
#define _MS_FILTERTRACER_HXX

#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <xmloff/attrlist.hxx>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/XTextSearch.hpp>
#include <com/sun/star/util/SearchResult.hpp>
#include <com/sun/star/util/logging/XLogger.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include "filter/msfilter/msfilterdllapi.h"

class FilterConfigItem;
class MSFILTER_DLLPUBLIC MSFilterTracer
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
