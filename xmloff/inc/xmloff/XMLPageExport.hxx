/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLPageExport.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:16:19 $
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

#ifndef _XMLOFF_XMLPAGEEXPORT_HXX
#define _XMLOFF_XMLPAGEEXPORT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif
#ifndef _UNIVERSALL_REFERENCE_HXX
#include <xmloff/uniref.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

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

