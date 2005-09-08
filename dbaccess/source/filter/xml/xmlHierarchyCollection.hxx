/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlHierarchyCollection.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:07:32 $
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
#ifndef DBA_XMLHIERARCHYCOLLECTION_HXX
#define DBA_XMLHIERARCHYCOLLECTION_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif


namespace dbaxml
{
    class ODBFilter;
    class OXMLHierarchyCollection : public SvXMLImportContext
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xParentContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xContainer;
        ::rtl::OUString     m_sName;
        ::rtl::OUString     m_sCollectionServiceName;
        ::rtl::OUString     m_sComponentServiceName;

        ODBFilter& GetOwnImport();
    public:

        OXMLHierarchyCollection( ODBFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                    ,const ::rtl::OUString& _sCollectionServiceName
                    ,const ::rtl::OUString& _sComponentServiceName
                    );
        OXMLHierarchyCollection( ODBFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xContainer
                    );
        virtual ~OXMLHierarchyCollection();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    };
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLHIERARCHYCOLLECTION_HXX
