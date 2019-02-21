/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <sax/tools/converter.hxx>
#include <comphelper/processfactory.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include "IgnoreTContext.hxx"
#include "RenameElemTContext.hxx"
#include "ProcAttrTContext.hxx"
#include "ProcAddAttrTContext.hxx"
#include "MergeElemTContext.hxx"
#include "CreateElemTContext.hxx"
#include "MutableAttrList.hxx"
#include "TransformerActions.hxx"
#include "ElemTransformerAction.hxx"
#include "PropertyActionsOOo.hxx"
#include "TransformerTokenMap.hxx"

#include "TransformerBase.hxx"
#include <xmloff/xmlimp.hxx>

using namespace ::osl;
using namespace ::xmloff::token;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::xml::sax;

namespace
{
bool lcl_ConvertAttr( OUString & rOutAttribute, sal_Int32 nParam )
{
    bool bResult = false;
    enum XMLTokenEnum eTokenToRename =
        static_cast< enum XMLTokenEnum >( nParam & 0xffff );
    if( eTokenToRename != XML_TOKEN_INVALID &&
        IsXMLToken( rOutAttribute, eTokenToRename ))
    {
        enum XMLTokenEnum eReplacementToken =
            static_cast< enum XMLTokenEnum >( nParam >> 16 );
        rOutAttribute = GetXMLToken( eReplacementToken );
        bResult = true;
    }
    return bResult;
}
} // anonymous namespace

XMLTransformerContext *XMLTransformerBase::CreateContext( sal_uInt16 nPrefix,
    const OUString& rLocalName, const OUString& rQName )
{
    XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
    XMLTransformerActions::const_iterator aIter =
        GetElemActions().find( aKey );

    if( aIter != GetElemActions().end() )
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
            return new XMLIgnoreTransformerContext( *this, rQName, false,
                                                false );
        case XML_ETACTION_COPY:
            return new XMLTransformerContext( *this, rQName );
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
        case XML_ETACTION_RENAME_ELEM_PROC_ATTRS:
            return new XMLProcAttrTransformerContext( *this, rQName,
                    (*aIter).second.GetQNamePrefixFromParam1(),
                    (*aIter).second.GetQNameTokenFromParam1(),
                       static_cast< sal_uInt16 >( (*aIter).second.m_nParam2 ) );
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
            OSL_ENSURE( false, "unknown action" );
            break;
        }
    }

    // default is copying
    return new XMLTransformerContext( *this, rQName );
}

XMLTransformerActions *XMLTransformerBase::GetUserDefinedActions( sal_uInt16 )
{
    return nullptr;
}

XMLTransformerBase::XMLTransformerBase( XMLTransformerActionInit const *pInit,
                                    ::xmloff::token::XMLTokenEnum const *pTKMapInit )
    throw () :
    m_pNamespaceMap( new SvXMLNamespaceMap ),
    m_ElemActions( pInit ),
    m_TokenMap( pTKMapInit )
{
    GetNamespaceMap().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_DC), GetXMLToken(XML_N_DC), XML_NAMESPACE_DC );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_MATH), GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_DOM), GetXMLToken(XML_N_DOM), XML_NAMESPACE_DOM );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_OOOW), GetXMLToken(XML_N_OOOW), XML_NAMESPACE_OOOW );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_OOOC), GetXMLToken(XML_N_OOOC), XML_NAMESPACE_OOOC );
}

XMLTransformerBase::~XMLTransformerBase() throw ()
{
}

void SAL_CALL XMLTransformerBase::startDocument()
{
    m_xHandler->startDocument();
}

void SAL_CALL XMLTransformerBase::endDocument()
{
    m_xHandler->endDocument();
}

void SAL_CALL XMLTransformerBase::startElement( const OUString& rName,
                                         const Reference< XAttributeList >& rAttrList )
{
    std::unique_ptr<SvXMLNamespaceMap> pRewindMap;

    // Process namespace attributes. This must happen before creating the
    // context, because namespace declaration apply to the element name itself.
    XMLMutableAttributeList *pMutableAttrList = nullptr;
    Reference< XAttributeList > xAttrList( rAttrList );
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        if( ( rAttrName.getLength() >= 5 ) &&
            ( rAttrName.startsWith( GetXMLToken(XML_XMLNS) ) ) &&
            ( rAttrName.getLength() == 5 || ':' == rAttrName[5] ) )
        {
            if( !pRewindMap )
            {
                pRewindMap = std::move(m_pNamespaceMap);
                m_pNamespaceMap.reset( new SvXMLNamespaceMap( *pRewindMap ) );
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );

            OUString aPrefix( ( rAttrName.getLength() == 5 )
                                 ? OUString()
                                 : rAttrName.copy( 6 ) );
            // Add namespace, but only if it is known.
            sal_uInt16 nKey = m_pNamespaceMap->AddIfKnown( aPrefix, rAttrValue );
            // If namespace is unknown, try to match a name with similar
            // TC Id an version
            if( XML_NAMESPACE_UNKNOWN == nKey  )
            {
                OUString aTestName( rAttrValue );
                if( SvXMLNamespaceMap::NormalizeOasisURN( aTestName ) )
                    nKey = m_pNamespaceMap->AddIfKnown( aPrefix, aTestName );
            }
            // If that namespace is not known, too, add it as unknown
            if( XML_NAMESPACE_UNKNOWN == nKey  )
                nKey = m_pNamespaceMap->Add( aPrefix, rAttrValue );

            const OUString& rRepName = m_vReplaceNamespaceMap.GetNameByKey( nKey );
            if( !rRepName.isEmpty() )
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
    if( !m_vContexts.empty() )
    {
        xContext = m_vContexts.back()->CreateChildContext( nPrefix,
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

    // Remember old namespace map.
    if( pRewindMap )
        xContext->PutRewindMap( std::move(pRewindMap) );

    // Push context on stack.
    m_vContexts.push_back( xContext );

    // Call a startElement at the new context.
    xContext->StartElement( xAttrList );
}

void SAL_CALL XMLTransformerBase::endElement( const OUString&
#if OSL_DEBUG_LEVEL > 0
rName
#endif
)
{
    if( !m_vContexts.empty() )
    {
        // Get topmost context
        ::rtl::Reference< XMLTransformerContext > xContext = m_vContexts.back();

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( xContext->GetQName() == rName,
                "XMLTransformerBase::endElement: popped context has wrong lname" );
#endif

        // Call a EndElement at the current context.
        xContext->EndElement();

        // and remove it from the stack.
        m_vContexts.pop_back();

        // Get a namespace map to rewind.
        std::unique_ptr<SvXMLNamespaceMap> pRewindMap = xContext->TakeRewindMap();

        // Delete the current context.
        xContext = nullptr;

        // Rewind a namespace map.
        if( pRewindMap )
        {
            m_pNamespaceMap = std::move( pRewindMap );
        }
    }
}

void SAL_CALL XMLTransformerBase::characters( const OUString& rChars )
{
    if( !m_vContexts.empty() )
    {
        m_vContexts.back()->Characters( rChars );
    }
}

void SAL_CALL XMLTransformerBase::ignorableWhitespace( const OUString& rWhitespaces )
{
    m_xHandler->ignorableWhitespace( rWhitespaces );
}

void SAL_CALL XMLTransformerBase::processingInstruction( const OUString& rTarget,
                                       const OUString& rData )
{
    m_xHandler->processingInstruction( rTarget, rData );
}

void SAL_CALL XMLTransformerBase::setDocumentLocator( const Reference< XLocator >& )
{
}

// XExtendedDocumentHandler
void SAL_CALL XMLTransformerBase::startCDATA()
{
}

void SAL_CALL XMLTransformerBase::endCDATA()
{
}

void SAL_CALL XMLTransformerBase::comment( const OUString& /*rComment*/ )
{
}

void SAL_CALL XMLTransformerBase::allowLineBreak()
{
}

void SAL_CALL XMLTransformerBase::unknown( const OUString& /*rString*/ )
{
}

// XInitialize
void SAL_CALL XMLTransformerBase::initialize( const Sequence< Any >& aArguments )
{
    const sal_Int32 nAnyCount = aArguments.getLength();
    const Any* pAny = aArguments.getConstArray();

    for( sal_Int32 nIndex = 0; nIndex < nAnyCount; nIndex++, pAny++ )
    {
        // use isAssignableFrom instead of comparing the types to
        // allow XExtendedDocumentHandler instead of XDocumentHandler (used in
        // writeOasis2OOoLibraryElement in sfx2).
        // The Any shift operator can't be used to query the type because it
        // uses queryInterface, and the model also has a XPropertySet interface.

        // document handler
        if( cppu::UnoType<XDocumentHandler>::get().isAssignableFrom( pAny->getValueType() ) )
        {
            m_xHandler.set( *pAny, UNO_QUERY );
        // Type change to avoid crashing of dynamic_cast
            if (SvXMLImport *pFastHandler = dynamic_cast<SvXMLImport*>(
                                uno::Reference< XFastDocumentHandler >( m_xHandler, uno::UNO_QUERY ).get() ) )
                m_xHandler.set( new SvXMLLegacyToFastDocHandler( pFastHandler ) );
        }

        // property set to transport data across
        if( cppu::UnoType<XPropertySet>::get().isAssignableFrom( pAny->getValueType() ) )
            m_xPropSet.set( *pAny, UNO_QUERY );

        // xmodel
        if( cppu::UnoType<css::frame::XModel>::get().isAssignableFrom( pAny->getValueType() ) )
            mxModel.set( *pAny, UNO_QUERY );
    }

    if( m_xPropSet.is() )
    {
        Any aAny;
        OUString sRelPath, sName;
        Reference< XPropertySetInfo > xPropSetInfo =
            m_xPropSet->getPropertySetInfo();
        OUString sPropName( "StreamRelPath"  );
        if( xPropSetInfo->hasPropertyByName(sPropName) )
        {
            aAny = m_xPropSet->getPropertyValue(sPropName);
            aAny >>= sRelPath;
        }
        sPropName = "StreamName";
        if( xPropSetInfo->hasPropertyByName(sPropName) )
        {
            aAny = m_xPropSet->getPropertyValue(sPropName);
            aAny >>= sName;
        }
        if( !sName.isEmpty() )
        {
            m_aExtPathPrefix = "../";

            // If there is a rel path within a package, then append
            // additional '../'. If the rel path contains an ':', then it is
            // an absolute URI (or invalid URI, because zip files don't
            // permit ':'), and it will be ignored.
            if( !sRelPath.isEmpty() )
            {
                sal_Int32 nColPos = sRelPath.indexOf( ':' );
                OSL_ENSURE( -1 == nColPos,
                            "StreamRelPath contains ':', absolute URI?" );

                if( -1 == nColPos )
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

    assert(m_xHandler.is()); // can't do anything without that
}

static sal_Int16 lcl_getUnit( const OUString& rValue )
{
    if( rValue.endsWithIgnoreAsciiCase( "cm" ) )
        return util::MeasureUnit::CM;
    else if ( rValue.endsWithIgnoreAsciiCase( "mm" ) )
        return util::MeasureUnit::MM;
    else
        return util::MeasureUnit::INCH;
}

XMLMutableAttributeList *XMLTransformerBase::ProcessAttrList(
        Reference< XAttributeList >& rAttrList, sal_uInt16 nActionMap,
           bool bClone  )
{
    XMLMutableAttributeList *pMutableAttrList = nullptr;
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
            if( aIter != pActions->end() )
            {
                if( !pMutableAttrList )
                {
                    pMutableAttrList = new XMLMutableAttributeList( rAttrList,
                                                                    bClone );
                    rAttrList = pMutableAttrList;
                }

                sal_uInt32 nAction = (*aIter).second.m_nActionType;
                bool bRename = false;
                switch( nAction )
                {
                case XML_ATACTION_RENAME:
                    bRename = true;
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
                    bRename = true;
                    [[fallthrough]];
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
                    bRename = true;
                    [[fallthrough]];
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
                case XML_ATACTION_TWIPS2IN:
                    {
                        OUString aAttrValue( rAttrValue );

                        XMLTransformerBase::ReplaceSingleInchWithIn( aAttrValue );
                        if( isWriter() )
                        {
                            sal_Int16 const nDestUnit = lcl_getUnit(aAttrValue);

                            // convert twips value to inch
                            sal_Int32 nMeasure;
                            if (::sax::Converter::convertMeasure(nMeasure,
                                    aAttrValue))
                            {

                                // #i13778#,#i36248# apply correct twip-to-1/100mm
                                nMeasure = static_cast<sal_Int32>( nMeasure >= 0
                                                        ? ((nMeasure*127+36)/72)
                                                        : ((nMeasure*127-36)/72) );

                                OUStringBuffer aBuffer;
                                ::sax::Converter::convertMeasure(aBuffer,
                                        nMeasure, util::MeasureUnit::MM_100TH,
                                        nDestUnit );
                                aAttrValue = aBuffer.makeStringAndClear();
                            }
                        }

                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_DECODE_STYLE_NAME_REF:
                    bRename = true;
                    [[fallthrough]];
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
                    bRename = true;
                    [[fallthrough]];
                case XML_ATACTION_ENCODE_STYLE_NAME_REF:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( EncodeStyleName(aAttrValue) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_NEG_PERCENT:
                    bRename = true;
                    [[fallthrough]];
                case XML_ATACTION_NEG_PERCENT:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( NegPercent( aAttrValue ) )
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_ADD_NAMESPACE_PREFIX:
                    bRename = true;
                    [[fallthrough]];
                case XML_ATACTION_ADD_NAMESPACE_PREFIX:
                    {
                        OUString aAttrValue( rAttrValue );
                        sal_uInt16 nValPrefix =
                            static_cast<sal_uInt16>(
                                    bRename ? (*aIter).second.m_nParam2
                                            : (*aIter).second.m_nParam1);
                        AddNamespacePrefix( aAttrValue, nValPrefix );
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
                        AddNamespacePrefix( aAttrValue, nValPrefix );
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_REMOVE_NAMESPACE_PREFIX:
                    bRename = true;
                    [[fallthrough]];
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
                            static_cast< bool >((*aIter).second.m_nParam1)))
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_URI_OASIS:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( ConvertURIToOOo( aAttrValue,
                            static_cast< bool >((*aIter).second.m_nParam1)))
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_ATTRIBUTE:
                    {
                        OUString aAttrValue( rAttrValue );
                        RenameAttributeValue(
                            aAttrValue,
                            (*aIter).second.m_nParam1,
                            (*aIter).second.m_nParam2,
                            (*aIter).second.m_nParam3 );
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RNG2ISO_DATETIME:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( ConvertRNGDateTimeToISO( aAttrValue ))
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_RNG2ISO_DATETIME:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( ConvertRNGDateTimeToISO( aAttrValue ))
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                        bRename = true;
                    }
                    break;
                case XML_ATACTION_IN2TWIPS:
                    {
                        OUString aAttrValue( rAttrValue );
                        XMLTransformerBase::ReplaceSingleInWithInch( aAttrValue );

                        if( isWriter() )
                        {
                            sal_Int16 const nDestUnit = lcl_getUnit(aAttrValue);

                            // convert inch value to twips and export as faked inch
                            sal_Int32 nMeasure;
                            if (::sax::Converter::convertMeasure(nMeasure,
                                    aAttrValue))
                            {

                                // #i13778#,#i36248#/ apply correct 1/100mm-to-twip conversion
                                nMeasure = static_cast<sal_Int32>( nMeasure >= 0
                                                        ? ((nMeasure*72+63)/127)
                                                        : ((nMeasure*72-63)/127) );

                                OUStringBuffer aBuffer;
                                ::sax::Converter::convertMeasure( aBuffer,
                                        nMeasure, util::MeasureUnit::MM_100TH,
                                        nDestUnit );
                                aAttrValue = aBuffer.makeStringAndClear();
                            }
                        }

                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_SVG_WIDTH_HEIGHT_OOO:
                    {
                        OUString aAttrValue( rAttrValue );
                        ReplaceSingleInchWithIn( aAttrValue );

                        sal_Int16 const nDestUnit = lcl_getUnit( aAttrValue );

                        sal_Int32 nMeasure;
                        if (::sax::Converter::convertMeasure(nMeasure,
                                    aAttrValue))
                        {

                            if( nMeasure > 0 )
                                nMeasure -= 1;
                            else if( nMeasure < 0 )
                                nMeasure += 1;


                            OUStringBuffer aBuffer;
                            ::sax::Converter::convertMeasure(aBuffer, nMeasure,
                                   util::MeasureUnit::MM_100TH, nDestUnit);
                            aAttrValue = aBuffer.makeStringAndClear();
                        }

                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_SVG_WIDTH_HEIGHT_OASIS:
                    {
                        OUString aAttrValue( rAttrValue );
                        ReplaceSingleInWithInch( aAttrValue );

                        sal_Int16 const nDestUnit = lcl_getUnit( aAttrValue );

                        sal_Int32 nMeasure;
                        if (::sax::Converter::convertMeasure(nMeasure,
                                aAttrValue))
                        {

                            if( nMeasure > 0 )
                                nMeasure += 1;
                            else if( nMeasure < 0 )
                                nMeasure -= 1;


                            OUStringBuffer aBuffer;
                            ::sax::Converter::convertMeasure(aBuffer, nMeasure,
                                    util::MeasureUnit::MM_100TH, nDestUnit );
                            aAttrValue = aBuffer.makeStringAndClear();
                        }

                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                    break;
                case XML_ATACTION_DECODE_ID:
                    {
                        const sal_Int32 nLen = rAttrValue.getLength();
                        OUStringBuffer aBuffer;

                        sal_Int32 pos;
                        for( pos = 0; pos < nLen; pos++ )
                        {
                            sal_Unicode c = rAttrValue[pos];
                            if( (c >= '0') && (c <= '9') )
                                aBuffer.append( c );
                            else
                                aBuffer.append( static_cast<sal_Int32>(c) );
                        }

                        pMutableAttrList->SetValueByIndex( i, aBuffer.makeStringAndClear() );
                    }
                    break;
                // #i50322# - special handling for the
                // transparency of writer background graphics.
                case XML_ATACTION_WRITER_BACK_GRAPHIC_TRANSPARENCY:
                    {
                        // determine, if it's the transparency of a document style
                        XMLTransformerContext* pFirstContext = m_vContexts[0].get();
                        OUString aFirstContextLocalName;
                        /* sal_uInt16 nFirstContextPrefix = */
                        GetNamespaceMap().GetKeyByAttrName( pFirstContext->GetQName(),
                                                                &aFirstContextLocalName );
                        bool bIsDocumentStyle(
                            ::xmloff::token::IsXMLToken( aFirstContextLocalName,
                                                         XML_DOCUMENT_STYLES ) );
                        // no conversion of transparency value for document
                        // styles, because former OpenOffice.org version writes
                        // writes always a transparency value of 100% and doesn't
                        // read the value. Thus, it's interpreted as 0%
                        if ( !bIsDocumentStyle )
                        {
                            OUString aAttrValue( rAttrValue );
                            NegPercent(aAttrValue);
                            pMutableAttrList->SetValueByIndex( i, aAttrValue );
                        }
                        bRename = true;
                    }
                    break;
                case XML_ATACTION_SHAPEID:
                {
                    OUString sNewValue( "shape"  );
                    sNewValue += rAttrValue;
                    pMutableAttrList->SetValueByIndex( i, sNewValue );
                    break;
                }

                default:
                    OSL_ENSURE( false, "unknown action" );
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

bool XMLTransformerBase::ReplaceSingleInchWithIn( OUString& rValue )
{
    bool bRet = false;
    sal_Int32 nPos = rValue.getLength();
    while( nPos && rValue[nPos-1] <= ' ' )
        --nPos;
    if( nPos > 2 &&
        ('c'==rValue[nPos-2] || 'C'==rValue[nPos-2]) &&
        ('h'==rValue[nPos-1] || 'H'==rValue[nPos-1]) )
    {
        rValue =rValue.copy( 0, nPos-2 );
        bRet = true;
    }

    return bRet;
}

bool XMLTransformerBase::ReplaceInchWithIn( OUString& rValue )
{
    bool bRet = false;
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
                            bRet = true;
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

bool XMLTransformerBase::ReplaceSingleInWithInch( OUString& rValue )
{
    bool bRet = false;

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
        bRet = true;
    }

    return bRet;
}

bool XMLTransformerBase::ReplaceInWithInch( OUString& rValue )
{
    bool bRet = false;
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
                    bRet = true;
                    continue;
                }
            }
        }
        ++nPos;
    }

    return bRet;
}

bool XMLTransformerBase::EncodeStyleName( OUString& rName ) const
{
    static const sal_Char aHexTab[] = "0123456789abcdef";

    bool bEncoded = false;

    sal_Int32 nLen = rName.getLength();
    OUStringBuffer aBuffer( nLen );

    for( sal_Int32 i = 0; i < nLen; i++ )
    {
        sal_Unicode c = rName[i];
        bool bValidChar = false;
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
                bValidChar = false;
            }
            else if( (c >= 0x02bbU && c <= 0x02c1U) || c == 0x0559 ||
                     c == 0x06e5 || c == 0x06e6 )
            {
                bValidChar = true;
            }
            else if( c == 0x0387 )
            {
                bValidChar = i > 0;
            }
            else
            {
                if( !xCharClass.is() )
                {
                    const_cast < XMLTransformerBase * >(this)
                        ->xCharClass = CharacterClassification::create( comphelper::getProcessComponentContext() );
                }
                sal_Int16 nType = xCharClass->getType( rName, i );

                switch( nType )
                {
                case UnicodeType::UPPERCASE_LETTER:     // Lu
                case UnicodeType::LOWERCASE_LETTER:     // Ll
                case UnicodeType::TITLECASE_LETTER:     // Lt
                case UnicodeType::OTHER_LETTER:         // Lo
                case UnicodeType::LETTER_NUMBER:        // Nl
                    bValidChar = true;
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
        if( bValidChar )
        {
            aBuffer.append( c );
        }
        else
        {
            aBuffer.append( '_' );
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
            aBuffer.append( '_' );
            bEncoded = true;
        }
    }

    if( aBuffer.getLength() > (1<<15)-1 )
        bEncoded = false;

    if( bEncoded )
        rName = aBuffer.makeStringAndClear();
    return bEncoded;
}

bool XMLTransformerBase::DecodeStyleName( OUString& rName )
{
    bool bEncoded = false;

    sal_Int32 nLen = rName.getLength();
    OUStringBuffer aBuffer( nLen );

    bool bWithinHex = false;
    sal_Unicode cEnc = 0;
    for( sal_Int32 i = 0; i < nLen; i++ )
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
                bEncoded = true;
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
                bEncoded = false;
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

bool XMLTransformerBase::NegPercent( OUString& rValue )
{
    bool bRet = false;
    bool bNeg = false;
    double nVal = 0;

    sal_Int32 nPos = 0;
    sal_Int32 nLen = rValue.getLength();

    // skip white space
    while( nPos < nLen && ' ' == rValue[nPos] )
        nPos++;

    if( nPos < nLen && '-' == rValue[nPos] )
    {
        bNeg = true;
        nPos++;
    }

    // get number
    while( nPos < nLen &&
           '0' <= rValue[nPos] &&
           '9' >= rValue[nPos] )
    {
        // TODO: check overflow!
        nVal *= 10;
        nVal += (rValue[nPos] - '0');
        nPos++;
    }
    if( nPos < nLen && '.' == rValue[nPos] )
    {
        nPos++;
        double nDiv = 1.;

        while( nPos < nLen &&
               '0' <= rValue[nPos] &&
               '9' >= rValue[nPos] )
        {
            // TODO: check overflow!
            nDiv *= 10;
            nVal += ( static_cast<double>(rValue[nPos] - '0') / nDiv );
            nPos++;
        }
    }

    // skip white space
    while( nPos < nLen && ' ' == rValue[nPos] )
        nPos++;

    if( nPos < nLen && '%' == rValue[nPos] )
    {
        if( bNeg )
               nVal = -nVal;
        nVal += .5;

        sal_Int32 nIntVal = 100 - static_cast<sal_Int32>( nVal );

        rValue = OUString::number(nIntVal) + "%";

        bRet = true;
    }

    return bRet;
}

void XMLTransformerBase::AddNamespacePrefix( OUString& rName,
                             sal_uInt16 nPrefix ) const
{
    rName = GetNamespaceMap().GetQNameByKey( nPrefix, rName, false );
}

bool XMLTransformerBase::RemoveNamespacePrefix( OUString& rName,
                            sal_uInt16 nPrefixOnly ) const
{
    OUString aLocalName;
    sal_uInt16 nPrefix =
        GetNamespaceMap().GetKeyByAttrName_( rName, &aLocalName );
    bool bRet = XML_NAMESPACE_UNKNOWN != nPrefix &&
                    (USHRT_MAX == nPrefixOnly || nPrefix == nPrefixOnly);
    if( bRet )
        rName = aLocalName;

    return bRet;
}

bool XMLTransformerBase::ConvertURIToOASIS( OUString& rURI,
                                        bool bSupportPackage ) const
{
    bool bRet = false;
    if( !m_aExtPathPrefix.isEmpty() && !rURI.isEmpty() )
    {
        bool bRel = false;
        switch( rURI[0] )
        {
        case '#':
            // no rel path, but
            // for package URIs, the '#' has to be removed
            if( bSupportPackage )
            {
                rURI = rURI.copy( 1 );
                bRet = true;
            }
            break;
        case '/':
            // no rel path; nothing to do
            break;
        case '.':
            // a rel path; to keep URI simple, remove './', if there
            bRel = true;
            if( rURI.getLength() > 1 && '/' == rURI[1] )
            {
                rURI = rURI.copy( 2 );
                bRet = true;
            }
            break;
        default:
            // check for a RFC2396 schema
            {
                bRel = true;
                sal_Int32 nPos = 1;
                sal_Int32 nLen = rURI.getLength();
                while( nPos < nLen )
                {
                    switch( rURI[nPos] )
                    {
                    case '/':
                        // a relative path segment
                        nPos = nLen;    // leave loop
                        break;
                    case ':':
                        // a schema
                        bRel = false;
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
            bRet = true;
        }
    }

    return bRet;
}

bool XMLTransformerBase::ConvertURIToOOo( OUString& rURI,
                                        bool bSupportPackage ) const
{
    bool bRet = false;
    if( !rURI.isEmpty() )
    {
        bool bPackage = false;
        switch( rURI[0] )
        {
        case '/':
            // no rel path; nothing to do
            break;
        case '.':
            // a rel path
            if( rURI.startsWith( m_aExtPathPrefix ) )
            {
                // an external URI; remove '../'
                rURI = rURI.copy( m_aExtPathPrefix.getLength() );
                bRet = true;
            }
            else
            {
                bPackage = true;
            }
            break;
        default:
            // check for a RFC2396 schema
            {
                bPackage = true;
                sal_Int32 nPos = 1;
                sal_Int32 nLen = rURI.getLength();
                while( nPos < nLen )
                {
                    switch( rURI[nPos] )
                    {
                    case '/':
                        // a relative path segment within the package
                        nPos = nLen;    // leave loop
                        break;
                    case ':':
                        // a schema
                        bPackage = false;
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
            OUString sTmp( '#' );
            if( rURI.startsWith( "./" ) )
                rURI = rURI.copy( 2 );
            sTmp += rURI;
            rURI = sTmp;
            bRet = true;
        }
    }

    return bRet;
}

bool XMLTransformerBase::RenameAttributeValue(
    OUString& rOutAttributeValue,
    sal_Int32 nParam1,
    sal_Int32 nParam2,
    sal_Int32 nParam3 )
{
    return ( lcl_ConvertAttr( rOutAttributeValue, nParam1) ||
             lcl_ConvertAttr( rOutAttributeValue, nParam2) ||
             lcl_ConvertAttr( rOutAttributeValue, nParam3) );
}

// static
bool XMLTransformerBase::ConvertRNGDateTimeToISO( OUString& rDateTime )
{
    if( !rDateTime.isEmpty() &&
        rDateTime.indexOf( '.' ) != -1 )
    {
        rDateTime = rDateTime.replace( '.', ',');
        return true;
    }

    return false;
}

XMLTokenEnum XMLTransformerBase::GetToken( const OUString& rStr ) const
{
    XMLTransformerTokenMap::const_iterator aIter =
        m_TokenMap.find( rStr );
    if( aIter == m_TokenMap.end() )
        return XML_TOKEN_END;
    else
        return (*aIter).second;
}


const XMLTransformerContext *XMLTransformerBase::GetCurrentContext() const
{
    OSL_ENSURE( !m_vContexts.empty(), "empty stack" );


    return m_vContexts.empty() ? nullptr : m_vContexts.back().get();
}

const XMLTransformerContext *XMLTransformerBase::GetAncestorContext(
                                                        sal_uInt32 n ) const
{
    auto nSize = m_vContexts.size();

    OSL_ENSURE( nSize > n + 2 , "invalid context" );

    return nSize > n + 2 ? m_vContexts[nSize - (n + 2)].get() : nullptr;
}

bool XMLTransformerBase::isWriter() const
{
    Reference< XServiceInfo > xSI( mxModel, UNO_QUERY );
    return  xSI.is() &&
        (   xSI->supportsService("com.sun.star.text.TextDocument") ||
            xSI->supportsService("com.sun.star.text.WebDocument") ||
            xSI->supportsService("com.sun.star.text.GlobalDocument") );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
