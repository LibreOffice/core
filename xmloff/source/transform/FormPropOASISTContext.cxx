/*************************************************************************
 *
 *  $RCSfile: FormPropOASISTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:50:14 $
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

#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
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
#ifndef _XMLOFF_FORMPROPOASISTCONTEXT_HXX
#include "FormPropOASISTContext.hxx"
#endif

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLFormPropOASISTransformerContext,
                XMLRenameElemTransformerContext );

XMLTokenEnum XMLFormPropOASISTransformerContext::GetValueType(
                const OUString& rValue )
{
    XMLTokenEnum eRet = XML_DOUBLE;
    sal_Bool bNeg = sal_False;
    sal_uInt32 nVal = 0;

    sal_Int32 nPos = 0L;
    sal_Int32 nLen = rValue.getLength();

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rValue[nPos] )
        nPos++;

    if( nPos < nLen && sal_Unicode('-') == rValue[nPos] )
    {
        bNeg = sal_True;
        nPos++;
    }

    // get number
    sal_Bool bOverflow = sal_False;
    while( nPos < nLen &&
           sal_Unicode('0') <= rValue[nPos] &&
           sal_Unicode('9') >= rValue[nPos] )
    {
        nVal *= 10;
        nVal += (rValue[nPos] - sal_Unicode('0'));
        bOverflow |= (nVal > (bNeg ? 2147483648UL : 2147483647UL));
        nPos++;
    }

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rValue[nPos] )
        nPos++;

    if( nPos == nLen )
    {
        // It's a integer number
        if( bOverflow )
            eRet = XML_LONG;
        else if( nVal > (bNeg ? 32768UL : 32767UL) )
            eRet = XML_INT;
        else
            eRet = XML_SHORT;
    }

    return eRet;
}

XMLFormPropOASISTransformerContext::XMLFormPropOASISTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        XMLTokenEnum eLocalName ) :
    XMLRenameElemTransformerContext( rImp, rQName, XML_NAMESPACE_FORM,
                                       XML_PROPERTY ),
    m_bIsList( XML_LIST_PROPERTY == eLocalName),
    m_bIsListValue( XML_LIST_VALUE == eLocalName)
{
}

XMLFormPropOASISTransformerContext::~XMLFormPropOASISTransformerContext()
{
}

void XMLFormPropOASISTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_FORM_PROP_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    XMLMutableAttributeList *pMutableAttrList =
        new XMLMutableAttributeList( rAttrList );
    Reference< XAttributeList > xAttrList( pMutableAttrList );

    sal_Int16 nValueTypeAttr = -1;
    OUString aValue;
    sal_Bool bIsVoid = sal_False;
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
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_RENAME:
                if( IsXMLToken( aLocalName, XML_VALUE_TYPE ) )
                {
                    if( IsXMLToken( rAttrValue, XML_FLOAT ) )
                    {
                        nValueTypeAttr = i;
                    }
                    else if( IsXMLToken( rAttrValue, XML_VOID ) )
                    {
                        pMutableAttrList->SetValueByIndex( i,
                                GetXMLToken( XML_SHORT ) );
                        bIsVoid = sal_True;
                    }
                }
                {
                    OUString aNewAttrQName(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                                (*aIter).second.GetQNamePrefixFromParam1(),
                                ::xmloff::token::GetXMLToken(
                                    (*aIter).second.GetQNameTokenFromParam1()) ) );
                    pMutableAttrList->RenameAttributeByIndex( i, aNewAttrQName );
                }
                break;
            case XML_ATACTION_REMOVE:
                if( !IsXMLToken( aLocalName, XML_CURRENCY ) )
                    aValue = rAttrValue;
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
    if( m_bIsList )
    {
        OUString aNewAttrQName(
                GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_FORM,
                    GetXMLToken( XML_PROPERTY_IS_LIST ) ) );
        pMutableAttrList->AddAttribute( aNewAttrQName,
                                        GetXMLToken( XML_TRUE ) );
    }

    if( nValueTypeAttr != -1 )
        pMutableAttrList->SetValueByIndex( nValueTypeAttr,
                                GetXMLToken( GetValueType( aValue ) ) );

    if( !m_bIsListValue )
        XMLRenameElemTransformerContext::StartElement( xAttrList );
    if( !m_bIsList )
    {
        pMutableAttrList =
            new XMLMutableAttributeList;
        xAttrList = pMutableAttrList;
        if( bIsVoid )
        {
            OUString aNewAttrQName(
                GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_FORM, GetXMLToken( XML_PROPERTY_IS_VOID ) ) );
            pMutableAttrList->AddAttribute( aNewAttrQName,
                                        GetXMLToken( XML_TRUE ) );
        }

        OUString aValueElemQName(
            GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_FORM, GetXMLToken( XML_PROPERTY_VALUE ) ) );
        GetTransformer().GetDocHandler()->startElement( aValueElemQName,
                                                        xAttrList );
        GetTransformer().GetDocHandler()->characters( aValue );
        GetTransformer().GetDocHandler()->endElement( aValueElemQName );
    }
}

void XMLFormPropOASISTransformerContext::EndElement()
{
    if( !m_bIsListValue )
        XMLRenameElemTransformerContext::EndElement();
}
