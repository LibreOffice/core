/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impastpl.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 15:27:04 $
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

#ifndef _XMLOFF_XMLASTPL_IMPL_HXX
#define _XMLOFF_XMLASTPL_IMPL_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _CNTRSRT_HXX
#include <svtools/cntnrsrt.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#include <vector>
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include <maptype.hxx>
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include <xmlexppr.hxx>
#endif

class SvXMLAutoStylePoolP;
class SvXMLAutoStylePoolParentsP_Impl;
class SvXMLAutoStylePoolNamesP_Impl;
class SvXMLAttributeList;
class SvXMLExportPropertyMapper;
class SvXMLExport;

#define MAX_CACHE_SIZE 65536

///////////////////////////////////////////////////////////////////////////////
//
// Implementationclass for stylefamily-information
//

typedef ::rtl::OUString *OUStringPtr;
DECLARE_LIST( SvXMLAutoStylePoolCache_Impl, OUStringPtr )

class XMLFamilyData_Impl
{
public:
    SvXMLAutoStylePoolCache_Impl        *pCache;
    sal_uInt32                          mnFamily;
    ::rtl::OUString                     maStrFamilyName;
    UniReference < SvXMLExportPropertyMapper >  mxMapper;

    SvXMLAutoStylePoolParentsP_Impl*    mpParentList;
    SvXMLAutoStylePoolNamesP_Impl*      mpNameList;
    sal_uInt32                          mnCount;
    sal_uInt32                          mnName;
    ::rtl::OUString                     maStrPrefix;
    sal_Bool                            bAsFamily;

public:
    XMLFamilyData_Impl( sal_Int32 nFamily, const ::rtl::OUString& rStrName,
            const UniReference < SvXMLExportPropertyMapper > &  rMapper,
            const ::rtl::OUString& rStrPrefix, sal_Bool bAsFamily = sal_True );

    XMLFamilyData_Impl( sal_Int32 nFamily ) :
        pCache( 0 ),
        mnFamily( nFamily ), mpParentList( NULL ),
        mpNameList( NULL ), mnCount( 0 ), mnName( 0 )

    {}
    ~XMLFamilyData_Impl();

    friend int XMLFamilyDataSort_Impl( const XMLFamilyData_Impl& r1, const XMLFamilyData_Impl& r2 );

    void ClearEntries();
};

DECLARE_CONTAINER_SORT( XMLFamilyDataList_Impl, XMLFamilyData_Impl )

///////////////////////////////////////////////////////////////////////////////
//
//
//

DECLARE_CONTAINER_SORT_DEL( SvXMLAutoStylePoolNamesP_Impl,
                           ::rtl::OUString )

///////////////////////////////////////////////////////////////////////////////
//
// Properties of a pool
//

class SvXMLAutoStylePoolPropertiesP_Impl
{
    ::rtl::OUString                     msName;
    ::std::vector< XMLPropertyState >   maProperties;
    sal_uInt32                          mnPos;

public:

    SvXMLAutoStylePoolPropertiesP_Impl( XMLFamilyData_Impl* pFamilyData, const ::std::vector< XMLPropertyState >& rProperties );

    ~SvXMLAutoStylePoolPropertiesP_Impl()
    {
    }

    const ::rtl::OUString& GetName() const { return msName; }
    const ::std::vector< XMLPropertyState >& GetProperties() const { return maProperties; }
    sal_uInt32 GetPos() const { return mnPos; }
};

typedef SvXMLAutoStylePoolPropertiesP_Impl *SvXMLAutoStylePoolPropertiesPPtr;
DECLARE_LIST( SvXMLAutoStylePoolPropertiesPList_Impl, SvXMLAutoStylePoolPropertiesPPtr )

///////////////////////////////////////////////////////////////////////////////
//
// Parents of AutoStylePool's
//

class SvXMLAutoStylePoolParentP_Impl
{
    ::rtl::OUString                         msParent;
    SvXMLAutoStylePoolPropertiesPList_Impl  maPropertiesList;

public:

    SvXMLAutoStylePoolParentP_Impl( const ::rtl::OUString & rParent ) :
        msParent( rParent )
    {
    }

    ~SvXMLAutoStylePoolParentP_Impl();

    sal_Bool Add( XMLFamilyData_Impl* pFamilyData, const ::std::vector< XMLPropertyState >& rProperties, ::rtl::OUString& rName, bool bDontSeek = false );

    ::rtl::OUString Find( const XMLFamilyData_Impl* pFamilyData, const ::std::vector< XMLPropertyState >& rProperties ) const;

    const ::rtl::OUString& GetParent() const { return msParent; }

    const SvXMLAutoStylePoolPropertiesPList_Impl& GetPropertiesList() const
    {
        return maPropertiesList;
    }
};

DECLARE_CONTAINER_SORT_DEL( SvXMLAutoStylePoolParentsP_Impl,
                            SvXMLAutoStylePoolParentP_Impl )

///////////////////////////////////////////////////////////////////////////////
//
// Implementationclass of SvXMLAutoStylePool
//

class SvXMLAutoStylePoolP_Impl
{
    SvXMLExport& rExport;

    XMLFamilyDataList_Impl      maFamilyList;

public:

    SvXMLAutoStylePoolP_Impl( SvXMLExport& rExport );
    ~SvXMLAutoStylePoolP_Impl();

    SvXMLExport& GetExport() const { return rExport; }

    void AddFamily( sal_Int32 nFamily, const ::rtl::OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > & rMapper,
        const ::rtl::OUString& rStrPrefix, sal_Bool bAsFamily = sal_True );
    void RegisterName( sal_Int32 nFamily, const ::rtl::OUString& rName );
    void GetRegisteredNames(
        com::sun::star::uno::Sequence<sal_Int32>& aFamilies,
        com::sun::star::uno::Sequence<rtl::OUString>& aNames );

//  ::rtl::OUString Add( sal_Int32 nFamily, const ::rtl::OUString& rParent,
//                       const ::std::vector< XMLPropertyState >& rProperties,
//                       sal_Bool bCache = sal_False );
    sal_Bool Add( ::rtl::OUString& rName, sal_Int32 nFamily,
                const ::rtl::OUString& rParent,
                const ::std::vector< XMLPropertyState >& rProperties,
                sal_Bool bCache = sal_False,
                bool bDontSeek = false );
    ::rtl::OUString AddToCache( sal_Int32 nFamily,
                                const ::rtl::OUString& rParent );
    ::rtl::OUString Find( sal_Int32 nFamily, const ::rtl::OUString& rParent,
                          const ::std::vector< XMLPropertyState >& rProperties ) const;

    ::rtl::OUString FindAndRemoveCached( sal_Int32 nFamily ) const;

    void exportXML( sal_Int32 nFamily,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const SvXMLAutoStylePoolP *pAntiImpl) const;

    void ClearEntries();
};

struct SvXMLAutoStylePoolPExport_Impl
{
    const ::rtl::OUString                   *mpParent;
    const SvXMLAutoStylePoolPropertiesP_Impl    *mpProperties;
};

#endif
