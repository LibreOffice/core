/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "MutableAttrList.hxx"
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include "ActionMapTypesOASIS.hxx"
#include "AttrTransformerAction.hxx"
#include "TransformerActions.hxx"
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#include "ControlOASISTContext.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLControlOASISTransformerContext, XMLTransformerContext );

XMLControlOASISTransformerContext::XMLControlOASISTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_Bool bCreateControl ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_FORM,
                            ::xmloff::token::GetXMLToken( XML_CONTROL ) ) ),
    m_bCreateControl( bCreateControl )
{
}

XMLControlOASISTransformerContext::~XMLControlOASISTransformerContext()
{
}

void XMLControlOASISTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_FORM_CONTROL_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = 0;
//      GetTransformer().ProcessAttrList( xAttrList, OOO_SHAPE_ACTIONS,
//                                        sal_True );

    XMLMutableAttributeList *pControlMutableAttrList =
        m_bCreateControl ? new XMLMutableAttributeList : 0;
    Reference< XAttributeList > xControlAttrList( pControlMutableAttrList );

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
                    new XMLMutableAttributeList( rAttrList );
                xAttrList = pMutableAttrList;
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_MOVE_TO_ELEM:
                if( m_bCreateControl )
                {
                    pControlMutableAttrList->AddAttribute( rAttrName,
                                                           rAttrValue );
                    pMutableAttrList->RemoveAttributeByIndex( i );
                    --i;
                    --nAttrCount;
                }
                break;
            case XML_ATACTION_RENAME_REMOVE_NAMESPACE_PREFIX:
                {
                    OUString aAttrValue( rAttrValue );
                    sal_uInt16 nValPrefix =
                        static_cast<sal_uInt16>( (*aIter).second.m_nParam2 );
                    GetTransformer().RemoveNamespacePrefix( aAttrValue,
                                                            nValPrefix );
                    OUString aNewAttrQName(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                            (*aIter).second.GetQNamePrefixFromParam1(),
                            ::xmloff::token::GetXMLToken(
                                (*aIter).second.GetQNameTokenFromParam1()) ) );
                    if( m_bCreateControl )
                    {
                        pControlMutableAttrList->AddAttribute( aNewAttrQName,
                                                               aAttrValue );
                        pMutableAttrList->RemoveAttributeByIndex( i );
                        --i;
                        --nAttrCount;
                    }
                    else
                    {
                        pMutableAttrList->RenameAttributeByIndex( i,
                                                              aNewAttrQName );
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                }
            case XML_ATACTION_URI_OASIS:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().ConvertURIToOOo( aAttrValue,
                           static_cast< sal_Bool >((*aIter).second.m_nParam1)) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            default:
                OSL_ENSURE( !this, "unknown action" );
                break;
            }
        }
    }

    if( m_bCreateControl )
        GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                        xControlAttrList );
    XMLTransformerContext::StartElement( xAttrList );
}

void XMLControlOASISTransformerContext::EndElement()
{
    XMLTransformerContext::EndElement();
    if( m_bCreateControl )
        GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}
