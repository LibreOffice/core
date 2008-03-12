/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartOASISTContext.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:12:44 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_CHARTOASISTCONTEXT_HXX
#include "ChartOASISTContext.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_ACTIONMAPTYPESOASIS_HXX
#include "ActionMapTypesOASIS.hxx"
#endif
#ifndef _XMLOFF_ATTRTRANSFORMERACTION_HXX
#include "AttrTransformerAction.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

// -----------------------------------------------------------------------------

TYPEINIT1( XMLChartOASISTransformerContext, XMLTransformerContext );

XMLChartOASISTransformerContext::XMLChartOASISTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLChartOASISTransformerContext::~XMLChartOASISTransformerContext()
{
}

void XMLChartOASISTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_CHART_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    OUString aAddInName;
    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = 0;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );
        XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( !(aIter == pActions->end() ) )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList =
                        new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_IN2INCH:
                {
                    OUString aAttrValue( rAttrValue );
                    if( XMLTransformerBase::ReplaceSingleInWithInch(
                                aAttrValue ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_DECODE_STYLE_NAME_REF:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().DecodeStyleName(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_REMOVE_ANY_NAMESPACE_PREFIX:
                OSL_ENSURE( IsXMLToken( aLocalName, XML_CLASS ),
                               "unexpected class token" );
                {
                    OUString aChartClass;
                    sal_uInt16 nValuePrefix =
                        GetTransformer().GetNamespaceMap().GetKeyByAttrName(
                            rAttrValue,
                            &aChartClass );
                    if( XML_NAMESPACE_CHART == nValuePrefix )
                    {
                        pMutableAttrList->SetValueByIndex( i, aChartClass );
                    }
                    else if ( XML_NAMESPACE_OOO == nValuePrefix )
                    {
                        pMutableAttrList->SetValueByIndex( i,
                                                GetXMLToken(XML_ADD_IN ) );
                        aAddInName = aChartClass;
                    }
                }
                break;
            default:
                OSL_ENSURE( !this, "unknown action" );
                break;
            }
        }
    }

    if( aAddInName.getLength() > 0 )
    {
        OUString aAttrQName( GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_CHART,
                                GetXMLToken( XML_ADD_IN_NAME ) ) );
        pMutableAttrList->AddAttribute( aAttrQName, aAddInName );
    }

    XMLTransformerContext::StartElement( xAttrList );
}
