/*************************************************************************
 *
 *  $RCSfile: impastpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-20 10:15:09 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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
 *  Contributor(s): _______________________________________
 *
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
#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
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
        mnFamily( nFamily ), mpParentList( NULL ),
        mpNameList( NULL ), mnCount( 0 ), mnName( 0 ),
        pCache( 0 )
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

    sal_Bool Add( XMLFamilyData_Impl* pFamilyData, const ::std::vector< XMLPropertyState >& rProperties, ::rtl::OUString& rName );

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
    const ::rtl::OUString       msCDATA;
    const ::rtl::OUString       msWS;
    SvXMLAttributeList*         mpAttrList;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >  mxAttrList;

    XMLFamilyDataList_Impl      maFamilyList;

public:

    SvXMLAutoStylePoolP_Impl();
    ~SvXMLAutoStylePoolP_Impl();

    void AddFamily( sal_Int32 nFamily, const ::rtl::OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > & rMapper,
        const ::rtl::OUString& rStrPrefix, sal_Bool bAsFamily = sal_True );
    void RegisterName( sal_Int32 nFamily, const ::rtl::OUString& rName );

    ::rtl::OUString Add( sal_Int32 nFamily, const ::rtl::OUString& rParent,
                         const ::std::vector< XMLPropertyState >& rProperties,
                           sal_Bool bCache = sal_False );
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
