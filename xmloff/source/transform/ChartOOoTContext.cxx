/*************************************************************************
 *
 *  $RCSfile: ChartOOoTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:44:12 $
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

#ifndef _XMLOFF_CHARTOOOTCONTEXT_HXX
#include "ChartOOoTContext.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_ACTIONMAPTYPESOOo_HXX
#include "ActionMapTypesOOo.hxx"
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

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

// -----------------------------------------------------------------------------

TYPEINIT1( XMLChartOOoTransformerContext, XMLTransformerContext );

XMLChartOOoTransformerContext::XMLChartOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLChartOOoTransformerContext::~XMLChartOOoTransformerContext()
{
}

void XMLChartOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_CHART_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    sal_Int16 nClassName = -1;
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
            case XML_ATACTION_INCH2IN:
                {
                    OUString aAttrValue( rAttrValue );
                    if( XMLTransformerBase::ReplaceSingleInchWithIn(
                                aAttrValue ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_ENCODE_STYLE_NAME_REF:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().EncodeStyleName(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_ADD_NAMESPACE_PREFIX:
                OSL_ENSURE( ::xmloff::token::IsXMLToken( aLocalName, XML_CLASS ),
                               "unexpected class token" );
                if( ::xmloff::token::IsXMLToken( rAttrValue, XML_ADD_IN ) )
                {
                    nClassName = -1;
                }
                else
                {
                    OUString aAttrValue( rAttrValue );
                    sal_uInt16 nValPrefix =
                        static_cast<sal_uInt16>((*aIter).second.m_nParam1);
                    if( GetTransformer().AddNamespacePrefix( aAttrValue,
                                                             nValPrefix ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_REMOVE:
                OSL_ENSURE( ::xmloff::token::IsXMLToken( aLocalName, XML_ADD_IN_NAME ),
                               "unexpected class token" );
                aAddInName = rAttrValue;
                pMutableAttrList->RemoveAttributeByIndex( i );
                --i;
                --nAttrCount;
                break;
            default:
                OSL_ENSURE( !this, "unknown action" );
                break;
            }
        }
    }

    if( nClassName != -1 && aAddInName.getLength() > 0 )
    {
        GetTransformer().AddNamespacePrefix( aAddInName, XML_NAMESPACE_OOO );
        pMutableAttrList->SetValueByIndex( nClassName, aAddInName );
    }

    XMLTransformerContext::StartElement( xAttrList );
}
