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



#ifndef _XMLOFF_XMLPAGEEXPORT_HXX
#define _XMLOFF_XMLPAGEEXPORT_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <rtl/ustring.hxx>
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#include <xmloff/attrlist.hxx>
#include <xmloff/uniref.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>

namespace com { namespace sun { namespace star {
    namespace style { class XStyle; }
    namespace container { class XIndexReplace; class XNameAccess;}
    namespace beans { class XPropertySet; }
} } }

class SvXMLExport;
class XMLPropertyHandlerFactory;
class XMLPropertySetMapper;
class SvXMLExportPropertyMapper;

//______________________________________________________________________________

struct XMLPageExportNameEntry
{
    ::rtl::OUString         sPageMasterName;
    ::rtl::OUString         sStyleName;
};

//______________________________________________________________________________

class XMLOFF_DLLPUBLIC XMLPageExport : public UniRefBase
{
    SvXMLExport& rExport;

    const ::rtl::OUString sIsPhysical;
    const ::rtl::OUString sFollowStyle;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess > xPageStyles;

    ::std::vector< XMLPageExportNameEntry > aNameVector;
    SAL_DLLPRIVATE sal_Bool findPageMasterName( const ::rtl::OUString& rStyleName, ::rtl::OUString& rPMName ) const;

    UniReference < XMLPropertyHandlerFactory > xPageMasterPropHdlFactory;
    UniReference < XMLPropertySetMapper > xPageMasterPropSetMapper;
    UniReference < SvXMLExportPropertyMapper > xPageMasterExportPropMapper;

protected:

    SvXMLExport& GetExport() { return rExport; }

    virtual void collectPageMasterAutoStyle(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                ::rtl::OUString& rPageMasterName );

    virtual void exportMasterPageContent(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                 sal_Bool bAutoStyles );

    sal_Bool exportStyle(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::style::XStyle >& rStyle,
                sal_Bool bAutoStyles );

    void exportStyles( sal_Bool bUsed, sal_Bool bAutoStyles );

public:
    XMLPageExport( SvXMLExport& rExp );
    ~XMLPageExport();

    void    collectAutoStyles( sal_Bool bUsed )     { exportStyles( bUsed, sal_True ); }
    void    exportAutoStyles();
    void    exportMasterStyles( sal_Bool bUsed )    { exportStyles( bUsed, sal_False ); }

    //text grid enhancement for better CJK support
    void exportDefaultStyle();
};

#endif  //  _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX

