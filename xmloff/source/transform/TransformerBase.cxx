/*************************************************************************
 *
 *  $RCSfile: TransformerBase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2004-07-22 11:55:04 $
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

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XCHARACTERCLASSIFICATION_HPP_
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_UNICODETYPE_HPP_
#include <com/sun/star/i18n/UnicodeType.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_IGNORETCONTEXT_HXX
#include "IgnoreTContext.hxx"
#endif
#ifndef _XMLOFF_RENAMEELEMTCONTEXT_HXX
#include "RenameElemTContext.hxx"
#endif
#ifndef _XMLOFF_PROCATTRTCONTEXT_HXX
#include "ProcAttrTContext.hxx"
#endif
#ifndef _XMLOFF_PROCADDATTRTCONTEXT_HXX
#include "ProcAddAttrTContext.hxx"
#endif
#ifndef _XMLOFF_MERGEELEMTCONTEXT_HXX
#include "MergeElemTContext.hxx"
#endif
#ifndef _XMLOFF_CREATEELEMTCONTEXT_HXX
#include "CreateElemTContext.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_ELEMTRANSFORMERACTION_HXX
#include "ElemTransformerAction.hxx"
#endif
#ifndef _XMLOFF_ATTRTRANSFORMERACTION_HXX
#include "AttrTransformerAction.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERTOKENMAP_HXX
#include "TransformerTokenMap.hxx"
#endif

#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _XMLOFF_TCONTEXTVECTOR_HXX
#include "TContextVector.hxx"
#endif

#include <vector>
#include <hash_map>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::osl;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::xml::sax;

// -----------------------------------------------------------------------------

XMLTransformerContext *XMLTransformerBase::CreateContext( USHORT nPrefix,
    const OUString& rLocalName, const OUString& rQName )
{
    XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
    XMLTransformerActions::const_iterator aIter =
        GetElemActions().find( aKey );

    if( !(aIter == GetElemActions().end()) )
    {
        sal_uInt32 nActionType = (*aIter).second.m_nActionType;
        if( (nActionType & XML_ETACTION_USER_DEFINED) != 0 )
        {
            XMLTransformerContext *pContext =
                CreateUserDefinedContext( (*aIter).second,
                                    rQName );
            OSL_ENSURE( pContext && !pContext->IsPersistent(),
                        "unknown or not persistent action" );
            return pContext;
        }

        switch( nActionType )
        {
        case XML_ETACTION_COPY_CONTENT:
            return new XMLIgnoreTransformerContext( *this, rQName, sal_False,
                                                sal_False );
            break;
        case XML_ETACTION_COPY:
            return new XMLTransformerContext( *this, rQName );
            break;
        case XML_ETACTION_RENAME_ELEM:
            return new XMLRenameElemTransformerContext( *this, rQName,
                    (*aIter).second.GetQNamePrefixFromParam1(),
                    (*aIter).second.GetQNameTokenFromParam1() );
        case XML_ETACTION_RENAME_ELEM_ADD_ATTR:
            return new XMLRenameElemTransformerContext( *this, rQName,
                    (*aIter).second.GetQNamePrefixFromParam1(),
                    (*aIter).second.GetQNameTokenFromParam1(),
                    (*aIter).second.GetQNamePrefixFromParam2(),
                    (*aIter).second.GetQNameTokenFromParam2(),
                       static_cast< XMLTokenEnum >( (*aIter).second.m_nParam3 ) );
            break;
        case XML_ETACTION_RENAME_ELEM_PROC_ATTRS:
            return new XMLProcAttrTransformerContext( *this, rQName,
                    (*aIter).second.GetQNamePrefixFromParam1(),
                    (*aIter).second.GetQNameTokenFromParam1(),
                       static_cast< sal_uInt16 >( (*aIter).second.m_nParam2 ) );
            break;
        case XML_ETACTION_RENAME_ELEM_ADD_PROC_ATTR:
            return new XMLProcAddAttrTransformerContext( *this, rQName,
                    (*aIter).second.GetQNamePrefixFromParam1(),
                    (*aIter).second.GetQNameTokenFromParam1(),
                       static_cast< sal_uInt16 >(
                        (*aIter).second.m_nParam3  >> 16 ),
                    (*aIter).second.GetQNamePrefixFromParam2(),
                    (*aIter).second.GetQNameTokenFromParam2(),
                       static_cast< XMLTokenEnum >(
                        (*aIter).second.m_nParam3 & 0xffff ) );
            break;
        case XML_ETACTION_RENAME_ELEM_COND:
            {
                const XMLTransformerContext *pCurrent = GetCurrentContext();
                if( pCurrent->HasQName(
                            (*aIter).second.GetQNamePrefixFromParam2(),
                            (*aIter).second.GetQNameTokenFromParam2() ) )
                    return new XMLRenameElemTransformerContext( *this, rQName,
                            (*aIter).second.GetQNamePrefixFromParam1(),
                            (*aIter).second.GetQNameTokenFromParam1() );
            }
            break;
        case XML_ETACTION_RENAME_ELEM_PROC_ATTRS_COND:
            {
                const XMLTransformerContext *pCurrent = GetCurrentContext();
                if( pCurrent->HasQName(
                            (*aIter).second.GetQNamePrefixFromParam3(),
                            (*aIter).second.GetQNameTokenFromParam3() ) )
                    return new XMLProcAttrTransformerContext( *this, rQName,
                            (*aIter).second.GetQNamePrefixFromParam1(),
                            (*aIter).second.GetQNameTokenFromParam1(),
                            static_cast< sal_uInt16 >( (*aIter).second.m_nParam2 ) );
                else
                    return new XMLProcAttrTransformerContext( *this, rQName,
                            static_cast< sal_uInt16 >( (*aIter).second.m_nParam2 ) );
            }
            break;
        case XML_ETACTION_PROC_ATTRS:
            return new XMLProcAttrTransformerContext( *this, rQName,
                       static_cast< sal_uInt16 >( (*aIter).second.m_nParam1 ) );
        case XML_ETACTION_PROC_ATTRS_COND:
            {
                const XMLTransformerContext *pCurrent = GetCurrentContext();
                if( pCurrent->HasQName(
                            (*aIter).second.GetQNamePrefixFromParam1(),
                            (*aIter).second.GetQNameTokenFromParam1() ) )
                    return new XMLProcAttrTransformerContext( *this, rQName,
                            static_cast< sal_uInt16 >( (*aIter).second.m_nParam2 ) );
            }
            break;
        case XML_ETACTION_MOVE_ATTRS_TO_ELEMS:
            return new XMLCreateElemTransformerContext( *this, rQName,
                       static_cast< sal_uInt16 >( (*aIter).second.m_nParam1 ) );
        case XML_ETACTION_MOVE_ELEMS_TO_ATTRS:
            return new XMLMergeElemTransformerContext( *this, rQName,
                       static_cast< sal_uInt16 >( (*aIter).second.m_nParam1 ) );
        default:
            OSL_ENSURE( !this, "unknown action" );
            break;
        }
    }

    // default is copying
    return new XMLTransformerContext( *this, rQName );
}

XMLTransformerActions *XMLTransformerBase::GetUserDefinedActions( sal_uInt16 )
{
    return 0;
}

XMLTransformerBase::XMLTransformerBase( XMLTransformerActionInit *pInit,
                                    ::xmloff::token::XMLTokenEnum *pTKMapInit )
    throw () :
    m_pNamespaceMap( new SvXMLNamespaceMap ),
    m_pReplaceNamespaceMap( new SvXMLNamespaceMap ),
    m_pContexts( new XMLTransformerContextVector ),
    m_pElemActions( new XMLTransformerActions( pInit ) ),
    m_pTokenMap( new XMLTransformerTokenMap( pTKMapInit ) )
{
    GetNamespaceMap().Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO), XML_NAMESPACE_FO );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_DC), GetXMLToken(XML_N_DC), XML_NAMESPACE_DC );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG),  XML_NAMESPACE_SVG );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_MATH), GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_DOM), GetXMLToken(XML_N_DOM), XML_NAMESPACE_DOM );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_OOOW), GetXMLToken(XML_N_OOOW), XML_NAMESPACE_OOOW );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_OOOC), GetXMLToken(XML_N_OOOC), XML_NAMESPACE_OOOC );
}

XMLTransformerBase::~XMLTransformerBase() throw ()
{
    ResetTokens();

    delete m_pNamespaceMap;
    delete m_pReplaceNamespaceMap;
    delete m_pContexts;
    delete m_pElemActions;
}

void SAL_CALL XMLTransformerBase::startDocument( void )
    throw( SAXException, RuntimeException )
{
    m_xHandler->startDocument();
}

void SAL_CALL XMLTransformerBase::endDocument( void )
    throw( SAXException, RuntimeException)
{
    m_xHandler->endDocument();
}

void SAL_CALL XMLTransformerBase::startElement( const OUString& rName,
                                         const Reference< XAttributeList >& rAttrList )
    throw(SAXException, RuntimeException)
{
    SvXMLNamespaceMap *pRewindMap = 0;

    // Process namespace attributes. This must happen before creating the
    // context, because namespace decaration apply to the element name itself.
    XMLMutableAttributeList *pMutableAttrList = 0;
    Reference< XAttributeList > xAttrList( rAttrList );
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        if( ( rAttrName.getLength() >= 5 ) &&
            ( rAttrName.compareTo( GetXMLToken(XML_XMLNS), 5 ) == 0 ) &&
            ( rAttrName.getLength() == 5 || ':' == rAttrName[5] ) )
        {
            if( !pRewindMap )
            {
                pRewindMap = m_pNamespaceMap;
                m_pNamespaceMap = new SvXMLNamespaceMap( *m_pNamespaceMap );
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );

            OUString aPrefix( ( rAttrName.getLength() == 5 )
                                 ? OUString()
                                 : rAttrName.copy( 6 ) );
            sal_uInt16 nKey = m_pNamespaceMap->Add( aPrefix, rAttrValue );

            const OUString& rRepName = m_pReplaceNamespaceMap->GetNameByKey( nKey );
            if( rRepName.getLength() )
            {
                if( !pMutableAttrList )
                {
                    pMutableAttrList = new XMLMutableAttributeList( xAttrList );
                    xAttrList = pMutableAttrList;
                }

                pMutableAttrList->SetValueByIndex( i, rRepName );
            }
        }
    }

    // Get element's namespace and local name.
    OUString aLocalName;
    sal_uInt16 nPrefix =
        m_pNamespaceMap->GetKeyByAttrName( rName, &aLocalName );

    // If there are contexts already, call a CreateChildContext at the topmost
    // context. Otherwise, create a default context.
    ::rtl::Reference < XMLTransformerContext > xContext;
    if( !m_pContexts->empty() )
    {
        xContext = m_pContexts->back()->CreateChildContext( nPrefix,
                                                          aLocalName,
                                                          rName,
                                                          xAttrList );
    }
    else
    {
        xContext = CreateContext( nPrefix, aLocalName, rName );
    }

    OSL_ENSURE( xContext.is(), "XMLTransformerBase::startElement: missing context" );
    if( !xContext.is() )
        xContext = new XMLTransformerContext( *this, rName );

    // Remeber old namespace map.
    if( pRewindMap )
        xContext->SetRewindMap( pRewindMap );

    // Call a startElement at the new context.
    xContext->StartElement( xAttrList );

    // Push context on stack.
    m_pContexts->push_back( xContext );
}

void SAL_CALL XMLTransformerBase::endElement( const OUString& rName )
    throw(SAXException, RuntimeException)
{
    if( !m_pContexts->empty() )
    {
        // Get topmost context and remove it from the stack.
        ::rtl::Reference< XMLTransformerContext > xContext = m_pContexts->back();
        m_pContexts->pop_back();

#ifndef PRODUCT
        OSL_ENSURE( xContext->GetQName() == rName,
                "XMLTransformerBase::endElement: popped context has wrong lname" );
#endif

        // Call a EndElement at the current context.
        xContext->EndElement();

        // Get a namespace map to rewind.
        SvXMLNamespaceMap *pRewindMap = xContext->GetRewindMap();

        // Delete the current context.
        xContext = 0;

        // Rewind a namespace map.
        if( pRewindMap )
        {
            delete m_pNamespaceMap;
            m_pNamespaceMap = pRewindMap;
        }
    }
}

void SAL_CALL XMLTransformerBase::characters( const OUString& rChars )
    throw(SAXException, RuntimeException)
{
    if( !m_pContexts->empty() )
    {
        m_pContexts->back()->Characters( rChars );
    }
}

void SAL_CALL XMLTransformerBase::ignorableWhitespace( const OUString& rWhitespaces )
    throw(SAXException, RuntimeException)
{
    m_xHandler->ignorableWhitespace( rWhitespaces );
}

void SAL_CALL XMLTransformerBase::processingInstruction( const OUString& rTarget,
                                       const OUString& rData )
    throw(SAXException, RuntimeException)
{
    m_xHandler->processingInstruction( rTarget, rData );
}

void SAL_CALL XMLTransformerBase::setDocumentLocator( const Reference< XLocator >& rLocator )
    throw(SAXException, RuntimeException)
{
    m_xLocator = rLocator;
}

// XExtendedDocumentHandler
void SAL_CALL XMLTransformerBase::startCDATA( void ) throw(SAXException, RuntimeException)
{
    if( m_xExtHandler.is() )
        m_xExtHandler->startCDATA();
}

void SAL_CALL XMLTransformerBase::endCDATA( void ) throw(RuntimeException)
{
    if( m_xExtHandler.is() )
        m_xExtHandler->endCDATA();
}

void SAL_CALL XMLTransformerBase::comment( const OUString& rComment )
    throw(SAXException, RuntimeException)
{
    if( m_xExtHandler.is() )
        m_xExtHandler->comment( rComment );
}

void SAL_CALL XMLTransformerBase::allowLineBreak( void )
    throw(SAXException, RuntimeException)
{
    if( m_xExtHandler.is() )
        m_xExtHandler->allowLineBreak();
}

void SAL_CALL XMLTransformerBase::unknown( const OUString& rString )
    throw(SAXException, RuntimeException)
{
    if( m_xExtHandler.is() )
        m_xExtHandler->unknown( rString );
}

// XInitialize
void SAL_CALL XMLTransformerBase::initialize( const Sequence< Any >& aArguments )
    throw(Exception, RuntimeException)
{
    const sal_Int32 nAnyCount = aArguments.getLength();
    const Any* pAny = aArguments.getConstArray();

    for( sal_Int32 nIndex = 0; nIndex < nAnyCount; nIndex++, pAny++ )
    {
        Reference<XInterface> xValue;
        *pAny >>= xValue;

        // document handler
        Reference<XDocumentHandler> xTmpDocHandler(
            xValue, UNO_QUERY );
        if( xTmpDocHandler.is() )
            m_xHandler = xTmpDocHandler;

        // property set to transport data across
        Reference<XPropertySet> xTmpPropertySet(
            xValue, UNO_QUERY );
        if( xTmpPropertySet.is() )
            m_xPropSet = xTmpPropertySet;
    }

    if( m_xPropSet.is() )
    {
        Any aAny;
        OUString sRelPath, sName;
        Reference< XPropertySetInfo > xPropSetInfo =
            m_xPropSet->getPropertySetInfo();
        OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("StreamRelPath" ) );
        if( xPropSetInfo->hasPropertyByName(sPropName) )
        {
            aAny = m_xPropSet->getPropertyValue(sPropName);
            aAny >>= sRelPath;
        }
        sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("StreamName" ) );
        if( xPropSetInfo->hasPropertyByName(sPropName) )
        {
            aAny = m_xPropSet->getPropertyValue(sPropName);
            aAny >>= sName;
        }
        if( sName.getLength() )
        {
            m_aExtPathPrefix = OUString( RTL_CONSTASCII_USTRINGPARAM("../" ) );
            if( sRelPath.getLength() )
            {
                OUString sTmp = m_aExtPathPrefix;
                sal_Int32 nPos = 0;
                do
                {
                    m_aExtPathPrefix += sTmp;
                    nPos = sRelPath.indexOf( '/', nPos + 1 );
                }
                while( -1 != nPos );
            }

        }
    }
}

XMLMutableAttributeList *XMLTransformerBase::ProcessAttrList(
        Reference< XAttributeList >& rAttrList, sal_uInt16 nActionMap,
           sal_Bool bClone  )
{
    XMLMutableAttributeList *pMutableAttrList = 0;
    XMLTransformerActions *pActions = GetUserDefinedActions( nActionMap );
    OSL_ENSURE( pActions, "go no actions" );
    if( pActions )
    {
        sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            const OUString& rAttrName = rAttrList->getNameByIndex( i );
            const OUString& rAttrValue = rAttrList->getValueByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                           &aLocalName );

            XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
            XMLTransformerActions::const_iterator aIter =
                    pActions->find( aKey );
            if( !(aIter == pActions->end() ) )
            {
                if( !pMutableAttrList )
                {
                    pMutableAttrList = new XMLMutableAttributeList( rAttrList,
                                                                    bClone );
                    rAttrList = pMutableAttrList;
                }

                sal_uInt32 nAction = (*aIter).second.m_nActionType;
                sal_Bool bRename = sal_False;
                switch( nAction )
                {
                case XML_ATACTION_RENAME:
                    bRename = sal_True;
                    break;
                case XML_ATACTION_COPY:
                    break;
                case XML_ATACTION_REMOVE:
                case XML_ATACTION_STYLE_DISPLAY_NAME:
                    pMutableAttrList->RemoveAttributeByIndex( i );
                    --i;
                    --nAttrCount;
                    break;
                case XML_ATACTION_RENAME_IN2INCH:
                    bRename = sal_True;
                case XML_ATACTION_IN2INCH:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( ReplaceSingleInWithInch( aAttrValue ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_INS2INCHS:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( ReplaceInWithInch( aAttrValue ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_INCH2IN:
                    bRename = sal_True;
                case XML_ATACTION_INCH2IN:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( ReplaceSingleInchWithIn( aAttrValue ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_INCHS2INS:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( ReplaceInchWithIn( aAttrValue ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_DECODE_STYLE_NAME_REF:
                    bRename = sal_True;
                case XML_ATACTION_DECODE_STYLE_NAME:
                case XML_ATACTION_DECODE_STYLE_NAME_REF:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( DecodeStyleName(aAttrValue) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_ENCODE_STYLE_NAME:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( EncodeStyleName(aAttrValue) )
                        {
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                            OUString aNewAttrQName(
                                GetNamespaceMap().GetQNameByKey(
                                    nPrefix,
                                ::xmloff::token::GetXMLToken(
                                XML_DISPLAY_NAME ) ) );
                            pMutableAttrList->AddAttribute( aNewAttrQName,
                                                            rAttrValue );
                        }
                    }
                    break;
                case XML_ATACTION_RENAME_ENCODE_STYLE_NAME_REF:
                    bRename = sal_True;
                case XML_ATACTION_ENCODE_STYLE_NAME_REF:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( EncodeStyleName(aAttrValue) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_NEG_PERCENT:
                    bRename = sal_True;
                case XML_ATACTION_NEG_PERCENT:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( NegPercent( aAttrValue ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_ADD_NAMESPACE_PREFIX:
                    bRename = sal_True;
                case XML_ATACTION_ADD_NAMESPACE_PREFIX:
                    {
                        OUString aAttrValue( rAttrValue );
                        sal_uInt16 nValPrefix =
                            static_cast<sal_uInt16>(
                                    bRename ? (*aIter).second.m_nParam2
                                            : (*aIter).second.m_nParam1);
                        if( AddNamespacePrefix( aAttrValue, nValPrefix ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_ADD_APP_NAMESPACE_PREFIX:
                    {
                        OUString aAttrValue( rAttrValue );
                        sal_uInt16 nValPrefix =
                            static_cast<sal_uInt16>((*aIter).second.m_nParam1);
                        if( IsXMLToken( GetClass(), XML_SPREADSHEET  ) )
                            nValPrefix = XML_NAMESPACE_OOOC;
                        else if( IsXMLToken( GetClass(), XML_TEXT  ) )
                            nValPrefix = XML_NAMESPACE_OOOW;
                        if( AddNamespacePrefix( aAttrValue, nValPrefix ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_REMOVE_NAMESPACE_PREFIX:
                    bRename = sal_True;
                case XML_ATACTION_REMOVE_NAMESPACE_PREFIX:
                    {
                        OUString aAttrValue( rAttrValue );
                        sal_uInt16 nValPrefix =
                            static_cast<sal_uInt16>(
                                    bRename ? (*aIter).second.m_nParam2
                                            : (*aIter).second.m_nParam1);
                        if( RemoveNamespacePrefix( aAttrValue, nValPrefix ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_REMOVE_ANY_NAMESPACE_PREFIX:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( RemoveNamespacePrefix( aAttrValue ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_URI_OOO:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( ConvertURIToOASIS( aAttrValue,
                            static_cast< sal_Bool >((*aIter).second.m_nParam1)))
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_URI_OASIS:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( ConvertURIToOOo( aAttrValue,
                            static_cast< sal_Bool >((*aIter).second.m_nParam1)))
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                default:
                    OSL_ENSURE( !this, "unknown action" );
                    break;
                }

                if( bRename )
                {
                    OUString aNewAttrQName(
                        GetNamespaceMap().GetQNameByKey(
                            (*aIter).second.GetQNamePrefixFromParam1(),
                            ::xmloff::token::GetXMLToken(
                                (*aIter).second.GetQNameTokenFromParam1()) ) );
                    pMutableAttrList->RenameAttributeByIndex( i,
                                                              aNewAttrQName );
                }
            }
        }
    }

    return pMutableAttrList;
}

sal_Bool XMLTransformerBase::ReplaceSingleInchWithIn( OUString& rValue )
{
    sal_Bool bRet = sal_False;
    sal_Int32 nPos = rValue.getLength();
    while( nPos && rValue[nPos-1] <= ' ' )
        --nPos;
    if( nPos > 2 &&
        ('c'==rValue[nPos-2] || 'C'==rValue[nPos-2]) &&
        ('h'==rValue[nPos-1] || 'H'==rValue[nPos-1]) )
    {
        rValue =rValue.copy( 0, nPos-2 );
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool XMLTransformerBase::ReplaceInchWithIn( OUString& rValue )
{
    sal_Bool bRet = sal_False;
    sal_Int32 nPos = 1;
    while( nPos < rValue.getLength()-3 )
    {
        sal_Unicode c = rValue[nPos];
        if( 'i'==c || 'I'==c )
        {
            c = rValue[nPos-1];
            if( (c >= '0' && c <= '9') || '.' == c )
            {
                c = rValue[nPos+1];
                if( 'n'==c || 'N'==c )
                {
                    c = rValue[nPos+2];
                    if( 'c'==c || 'C'==c )
                    {
                        c = rValue[nPos+3];
                        if( 'h'==c || 'H'==c )
                        {
                            rValue = rValue.replaceAt( nPos,
                                4, GetXMLToken(XML_UNIT_INCH) );
                            nPos += 2;
                            bRet = sal_True;
                            continue;
                        }
                    }
                }
            }
        }
        ++nPos;
    }

    return bRet;
}

sal_Bool XMLTransformerBase::ReplaceSingleInWithInch( OUString& rValue )
{
    sal_Bool bRet = sal_False;

    sal_Int32 nPos = rValue.getLength();
    while( nPos && rValue[nPos-1] <= ' ' )
        --nPos;
    if( nPos > 2 &&
        ('i'==rValue[nPos-2] ||
            'I'==rValue[nPos-2]) &&
        ('n'==rValue[nPos-1] ||
            'N'==rValue[nPos-1]) )
    {
        nPos -= 2;
        rValue = rValue.replaceAt( nPos, rValue.getLength() - nPos,
                                           GetXMLToken(XML_INCH) );
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool XMLTransformerBase::ReplaceInWithInch( OUString& rValue )
{
    sal_Bool bRet = sal_False;
    sal_Int32 nPos = 1;
    while( nPos < rValue.getLength()-1 )
    {
        sal_Unicode c = rValue[nPos];
        if( 'i'==c || 'I'==c )
        {
            c = rValue[nPos-1];
            if( (c >= '0' && c <= '9') || '.' == c )
            {
                c = rValue[nPos+1];
                if( 'n'==c || 'N'==c )
                {
                    rValue = rValue.replaceAt( nPos,
                                    2, GetXMLToken(XML_INCH) );
                    nPos += 4;
                    bRet = sal_True;
                    continue;
                }
            }
        }
        ++nPos;
    }

    return bRet;
}

sal_Bool XMLTransformerBase::EncodeStyleName( OUString& rName ) const
{
    static sal_Char aHexTab[] = "0123456789abcdef";

    sal_Bool bEncoded = sal_False;

    sal_Int32 nLen = rName.getLength();
    OUStringBuffer aBuffer( nLen );

    for( xub_StrLen i = 0; i < nLen; i++ )
    {
        sal_Unicode c = rName[i];
        sal_Bool bValidChar = sal_False;
        if( c < 0x00ffU )
        {
            bValidChar =
                (c >= 0x0041 && c <= 0x005a) ||
                (c >= 0x0061 && c <= 0x007a) ||
                (c >= 0x00c0 && c <= 0x00d6) ||
                (c >= 0x00d8 && c <= 0x00f6) ||
                (c >= 0x00f8 && c <= 0x00ff) ||
                ( i > 0 && ( (c >= 0x0030 && c <= 0x0039) ||
                             c == 0x00b7 || c == '-' || c == '.') );
        }
        else
        {
            if( (c >= 0xf900U && c <= 0xfffeU) ||
                 (c >= 0x20ddU && c <= 0x20e0U))
            {
                bValidChar = sal_False;
            }
            else if( (c >= 0x02bbU && c <= 0x02c1U) || c == 0x0559 ||
                     c == 0x06e5 || c == 0x06e6 )
            {
                bValidChar = sal_True;
            }
            else if( c == 0x0387 )
            {
                bValidChar = i > 0;
            }
            else
            {
                if( !xCharClass.is() )
                {
                    Reference< XMultiServiceFactory > xFactory =
                        comphelper::getProcessServiceFactory();
                    if( xFactory.is() )
                    {
                        try
                        {
                            const_cast < XMLTransformerBase * >(this)
                                ->xCharClass =
                                    Reference < XCharacterClassification >(
                                xFactory->createInstance(
                                    OUString::createFromAscii(
                        "com.sun.star.i18n.CharacterClassification_Unicode") ),
                                UNO_QUERY );

                            OSL_ENSURE( xCharClass.is(),
                    "can't instantiate character clossification component" );
                        }
                        catch( com::sun::star::uno::Exception& )
                        {
                        }
                    }
                }
                if( xCharClass.is() )
                {
                    sal_Int16 nType = xCharClass->getType( rName, i );

                    switch( nType )
                    {
                    case UnicodeType::UPPERCASE_LETTER:     // Lu
                    case UnicodeType::LOWERCASE_LETTER:     // Ll
                    case UnicodeType::TITLECASE_LETTER:     // Lt
                    case UnicodeType::OTHER_LETTER:         // Lo
                    case UnicodeType::LETTER_NUMBER:        // Nl
                        bValidChar = sal_True;
                        break;
                    case UnicodeType::NON_SPACING_MARK:     // Ms
                    case UnicodeType::ENCLOSING_MARK:       // Me
                    case UnicodeType::COMBINING_SPACING_MARK:   //Mc
                    case UnicodeType::MODIFIER_LETTER:      // Lm
                    case UnicodeType::DECIMAL_DIGIT_NUMBER: // Nd
                        bValidChar = i > 0;
                        break;
                    }
                }
            }
        }
        if( bValidChar )
        {
            aBuffer.append( c );
        }
        else
        {
            aBuffer.append( static_cast< sal_Unicode >( '_' ) );
            if( c > 0x0fff )
                aBuffer.append( static_cast< sal_Unicode >(
                            aHexTab[ (c >> 12) & 0x0f ]  ) );
            if( c > 0x00ff )
                aBuffer.append( static_cast< sal_Unicode >(
                        aHexTab[ (c >> 8) & 0x0f ] ) );
            if( c > 0x000f )
                aBuffer.append( static_cast< sal_Unicode >(
                        aHexTab[ (c >> 4) & 0x0f ] ) );
            aBuffer.append( static_cast< sal_Unicode >(
                        aHexTab[ c & 0x0f ] ) );
            aBuffer.append( static_cast< sal_Unicode >( '_' ) );
            bEncoded = sal_True;
        }
    }

    if( bEncoded )
        rName = aBuffer.makeStringAndClear();
    return bEncoded;
}

sal_Bool XMLTransformerBase::DecodeStyleName( OUString& rName )
{
    sal_Bool bEncoded = sal_False;

    sal_Int32 nLen = rName.getLength();
    OUStringBuffer aBuffer( nLen );

    sal_Bool bWithinHex = sal_False;
    sal_Unicode cEnc = 0;
    for( xub_StrLen i = 0; i < nLen; i++ )
    {
        sal_Unicode c = rName[i];
        if( '_' == c )
        {
            if( bWithinHex )
            {
                aBuffer.append( cEnc );
                cEnc = 0;
            }
            else
            {
                bEncoded = sal_True;
            }
            bWithinHex = !bWithinHex;
        }
        else if( bWithinHex )
        {
            sal_Unicode cDigit;
            if( c >= '0' && c <= '9' )
            {
                cDigit = c - '0';
            }
            else if( c >= 'a' && c <= 'f' )
            {
                cDigit = c - 'a' + 10;
            }
            else if( c >= 'A' && c <= 'F' )
            {
                cDigit = c - 'A' + 10;
            }
            else
            {
                // error
                bEncoded = sal_False;
                break;
            }
            cEnc = (cEnc << 4) + cDigit;
        }
        else
        {
            aBuffer.append( c );
        }
    }

    if( bEncoded )
        rName = aBuffer.makeStringAndClear();
    return bEncoded;
}

sal_Bool XMLTransformerBase::NegPercent( OUString& rValue )
{
    sal_Bool bRet = sal_False;
    sal_Bool bNeg = sal_False;
    double nVal = 0;

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
    while( nPos < nLen &&
           sal_Unicode('0') <= rValue[nPos] &&
           sal_Unicode('9') >= rValue[nPos] )
    {
        // TODO: check overflow!
        nVal *= 10;
        nVal += (rValue[nPos] - sal_Unicode('0'));
        nPos++;
    }
    double nDiv = 1.;
    if( nPos < nLen && sal_Unicode('.') == rValue[nPos] )
    {
        nPos++;

        while( nPos < nLen &&
               sal_Unicode('0') <= rValue[nPos] &&
               sal_Unicode('9') >= rValue[nPos] )
        {
            // TODO: check overflow!
            nDiv *= 10;
            nVal += ( static_cast<double>(rValue[nPos] - sal_Unicode('0')) / nDiv );
            nPos++;
        }
    }

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rValue[nPos] )
        nPos++;

    if( nPos < nLen &&  sal_Unicode('%') == rValue[nPos] )
    {
        if( bNeg )
               nVal = -nVal;
        nVal += .5;

        sal_Int32 nIntVal = 100 - static_cast<sal_Int32>( nVal );

        OUStringBuffer aNewValBuffer;
        aNewValBuffer.append( nIntVal );
        aNewValBuffer.append( sal_Unicode('%' ) );

        rValue = aNewValBuffer.makeStringAndClear();
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool XMLTransformerBase::AddNamespacePrefix( ::rtl::OUString& rName,
                             sal_uInt16 nPrefix ) const
{
    rName = GetNamespaceMap().GetQNameByKey( nPrefix, rName );
    return sal_True;
}

sal_Bool XMLTransformerBase::RemoveNamespacePrefix( ::rtl::OUString& rName,
                            sal_uInt16 nPrefixOnly ) const
{
    OUString aLocalName;
    sal_uInt16 nPrefix =
        GetNamespaceMap().GetKeyByAttrName( rName, &aLocalName );
    sal_Bool bRet = XML_NAMESPACE_UNKNOWN != nPrefix &&
                    (USHRT_MAX == nPrefixOnly || nPrefix == nPrefixOnly);
    if( bRet )
        rName = aLocalName;

    return bRet;
}

sal_Bool XMLTransformerBase::ConvertURIToOASIS( ::rtl::OUString& rURI,
                                        sal_Bool bSupportPackage ) const
{
    sal_Bool bRet = sal_False;
    if( m_aExtPathPrefix.getLength() && rURI.getLength() )
    {
        sal_Bool bRel = sal_False;
        switch( rURI[0] )
        {
        case '#':
            // no rel path, but
            // for package URIs, the '#' has to be removed
            if( bSupportPackage )
            {
                rURI = rURI.copy( 1 );
                bRet = sal_True;
            }
            break;
        case '/':
            // no rel path; nothing to do
            break;
        case '.':
            // a rel path; to keep URI simple, remove './', if there
            bRel = sal_True;
            if( rURI.getLength() > 1 && '/' == rURI[1] )
            {
                rURI = rURI.copy( 2 );
                bRet = sal_True;
            }
            break;
        default:
            // check for a RFC2396 schema
            {
                bRel = sal_True;
                sal_Int32 nPos = 1;
                sal_Int32 nLen = rURI.getLength();
                while( nPos < nLen )
                {
                    switch( rURI[nPos] )
                    {
                    case '/':
                        // a relative path segement
                        nPos = nLen;    // leave loop
                        break;
                    case ':':
                        // a schema
                        bRel = sal_False;
                        nPos = nLen;    // leave loop
                        break;
                    default:
                        // we don't care about any other characters
                        break;
                    }
                    ++nPos;
                }
            }
        }

        if( bRel )
        {
            OUString sTmp( m_aExtPathPrefix );
            sTmp += rURI;
            rURI = sTmp;
            bRet = sal_True;
        }
    }

    return bRet;
}

sal_Bool XMLTransformerBase::ConvertURIToOOo( ::rtl::OUString& rURI,
                                        sal_Bool bSupportPackage ) const
{
    sal_Bool bRet = sal_False;
    if( rURI.getLength() )
    {
        sal_Bool bPackage = sal_False;
        switch( rURI[0] )
        {
        case '/':
            // no rel path; nothing to to
            break;
        case '.':
            // a rel path
            if( 0 == rURI.compareTo( m_aExtPathPrefix,
                                     m_aExtPathPrefix.getLength() ) )
            {
                // an external URI; remove '../'
                rURI = rURI.copy( m_aExtPathPrefix.getLength() );
                bRet = sal_True;
            }
            else
            {
                bPackage = sal_True;
            }
            break;
        default:
            // check for a RFC2396 schema
            {
                bPackage = sal_True;
                sal_Int32 nPos = 1;
                sal_Int32 nLen = rURI.getLength();
                while( nPos < nLen )
                {
                    switch( rURI[nPos] )
                    {
                    case '/':
                        // a relative path segement within the package
                        nPos = nLen;    // leave loop
                        break;
                    case ':':
                        // a schema
                        bPackage = sal_False;
                        nPos = nLen;    // leave loop
                        break;
                    default:
                        // we don't care about any other characters
                        break;
                    }
                    ++nPos;
                }
            }
        }

        if( bPackage && bSupportPackage )
        {
            OUString sTmp( OUString::valueOf( sal_Unicode( '#' ) ) );
            sTmp += rURI;
            rURI = sTmp;
            bRet = sal_True;
        }
    }

    return bRet;
}

XMLTokenEnum XMLTransformerBase::GetToken( const OUString& rStr ) const
{
    XMLTransformerTokenMap::const_iterator aIter =
        m_pTokenMap->find( rStr );
    if( aIter == m_pTokenMap->end() )
        return XML_TOKEN_END;
    else
        return (*aIter).second;
}



const XMLTransformerContext *XMLTransformerBase::GetCurrentContext() const
{
    OSL_ENSURE( !m_pContexts->empty(), "empty stack" );


    return m_pContexts->empty() ? 0 : m_pContexts->back().get();
}
