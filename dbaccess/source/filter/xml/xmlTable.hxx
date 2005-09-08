/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlTable.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:09:53 $
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
#ifndef DBA_XMLTABLE_HXX
#define DBA_XMLTABLE_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

namespace dbaxml
{
    class ODBFilter;
    class OXMLTable : public SvXMLImportContext
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xParentContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable;
        ::rtl::OUString m_sFilterStatement;
        ::rtl::OUString m_sOrderStatement;
        ::rtl::OUString m_sName;
        ::rtl::OUString m_sSchema;
        ::rtl::OUString m_sCatalog;
        ::rtl::OUString m_sStyleName;
        ::rtl::OUString m_sServiceName;
        sal_Bool        m_bApplyFilter;
        sal_Bool        m_bApplyOrder;


        ODBFilter& GetOwnImport();

        void fillAttributes(    sal_uInt16 nPrfx
                                ,const ::rtl::OUString& _sLocalName
                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                                , ::rtl::OUString& _rsCommand
                                ,::rtl::OUString& _rsTableName
                                ,::rtl::OUString& _rsTableSchema
                                ,::rtl::OUString& _rsTableCatalog
                            );

        virtual void setProperties(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _xProp);
    public:

        OXMLTable( ODBFilter& rImport
                    , sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                    ,const ::rtl::OUString& _sServiceName
                    );
        virtual ~OXMLTable();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
        virtual void EndElement();
    };
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLTABLE_HXX
