/*************************************************************************
 *
 *  $RCSfile: xmlTable.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:23:12 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBA_XMLTABLE_HXX
#include "xmlTable.hxx"
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef DBA_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif
#ifndef DBA_XMLHIERARCHYCOLLECTION_HXX
#include "xmlHierarchyCollection.hxx"
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#include <comphelper/namecontainer.hxx>
#endif

namespace dbaxml
{
    using namespace ::rtl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::xml::sax;

OXMLTable::OXMLTable( ODBFilter& _rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                ,const ::rtl::OUString& _sServiceName
                ) :
    SvXMLImportContext( _rImport, nPrfx, _sLocalName )
    ,m_xParentContainer(_xParentContainer)
    ,m_bApplyFilter(sal_False)
    ,m_bApplyOrder(sal_False)
    ,m_sServiceName(_sServiceName)
{
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = GetOwnImport().GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = GetOwnImport().GetQueryElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    sal_Bool bAutoEnabled = sal_False;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_QUERY_NAME:
                m_sName = sValue;
                break;
            case XML_TOK_CATALOG_NAME:
                m_sCatalog = sValue;
                break;
            case XML_TOK_SCHEMA_NAME:
                m_sSchema = sValue;
                break;
            case XML_TOK_STYLE_NAME:
                m_sStyleName = sValue;
                break;
            case XML_TOK_APPLY_FILTER:
                m_bApplyFilter = sValue.equalsAscii("true");
                break;
            case XML_TOK_APPLY_ORDER:
                m_bApplyOrder = sValue.equalsAscii("true");
                break;
        }
    }
    Sequence< Any > aArguments(2);
    PropertyValue aValue;
    // set as folder
    aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
    aValue.Value <<= m_sName;
    aArguments[0] <<= aValue;
    //parent
    aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parent"));
    aValue.Value <<= m_xParentContainer;
    aArguments[1] <<= aValue;
    m_xTable.set(GetOwnImport().getORB()->createInstanceWithArguments(m_sServiceName,aArguments),UNO_QUERY);
}
// -----------------------------------------------------------------------------

OXMLTable::~OXMLTable()
{
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLTable::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetQueryElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_FILTER_STATEMENT:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                ::rtl::OUString s1,s2,s3;
                fillAttributes(nPrefix, rLocalName,xAttrList,m_sFilterStatement,s1,s2,s3);
            }
            break;
        case XML_TOK_ORDER_STATEMENT:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                ::rtl::OUString s1,s2,s3;
                fillAttributes(nPrefix, rLocalName,xAttrList,m_sOrderStatement,s1,s2,s3);
            }
            break;

        case XML_TOK_COLUMNS:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference< XColumnsSupplier > xColumnsSup(m_xTable,UNO_QUERY);
                Reference< XNameAccess > xColumns;
                if ( xColumnsSup.is() )
                {
                    xColumns = xColumnsSup->getColumns();
                }
                pContext = new OXMLHierarchyCollection( GetOwnImport(), nPrefix, rLocalName ,xColumns);
            }
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
ODBFilter& OXMLTable::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
void OXMLTable::setProperties(Reference< XPropertySet > & _xProp )
{
    try
    {
        if ( _xProp.is() )
        {
            _xProp->setPropertyValue(PROPERTY_APPLYFILTER,makeAny(m_bApplyFilter));
            _xProp->setPropertyValue(PROPERTY_FILTER,makeAny(m_sFilterStatement));

            if ( _xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_APPLYORDER) )
                _xProp->setPropertyValue(PROPERTY_APPLYORDER,makeAny(m_bApplyOrder));
            _xProp->setPropertyValue(PROPERTY_ORDER,makeAny(m_sOrderStatement));
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"OXMLTable::EndElement -> exception catched");
    }
}
// -----------------------------------------------------------------------------
void OXMLTable::EndElement()
{
    Reference<XNameContainer> xNameContainer(m_xParentContainer,UNO_QUERY);
    if ( xNameContainer.is() )
    {
        try
        {
            if ( m_xTable.is() )
            {
                setProperties(m_xTable);

                if ( m_sStyleName.getLength() )
                {
                    const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
                    if ( pAutoStyles )
                    {
                        OTableStyleContext* pAutoStyle = PTR_CAST(OTableStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_TABLE,m_sStyleName));
                        if ( pAutoStyle )
                        {
                            pAutoStyle->FillPropertySet(m_xTable);
                        }
                    }
                }

                xNameContainer->insertByName(m_sName,makeAny(m_xTable));
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"OXMLQuery::EndElement -> exception catched");
        }
    }

}
// -----------------------------------------------------------------------------
void OXMLTable::fillAttributes(sal_uInt16 nPrfx
                                ,const ::rtl::OUString& _sLocalName
                                ,const Reference< XAttributeList > & _xAttrList
                                , ::rtl::OUString& _rsCommand
                                ,::rtl::OUString& _rsTableName
                                ,::rtl::OUString& _rsTableSchema
                                ,::rtl::OUString& _rsTableCatalog
                                )
{
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = GetOwnImport().GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = GetOwnImport().GetQueryElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COMMAND:
                _rsCommand = sValue;
                break;
            case XML_TOK_CATALOG_NAME:
                _rsTableCatalog = sValue;
                break;
            case XML_TOK_SCHEMA_NAME:
                _rsTableSchema = sValue;
                break;
            case XML_TOK_QUERY_NAME:
                _rsTableName = sValue;
                break;
        }
    }
}
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
