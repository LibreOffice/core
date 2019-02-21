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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERBASE_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERBASE_HXX

#include <sal/config.h>

#include <vector>
#include <memory>

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <rtl/ref.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>

#include "Transformer.hxx"
#include "TransformerActions.hxx"
#include "TransformerTokenMap.hxx"

namespace com { namespace sun { namespace star {
    namespace i18n { class XCharacterClassification; }
}}}

class SvXMLNamespaceMap;
class XMLTransformerContext;
class XMLTransformerActions;
struct XMLTransformerActionInit;
struct TransformerAction_Impl;
class XMLMutableAttributeList;

const sal_uInt16 INVALID_ACTIONS = 0xffff;

class XMLTransformerBase : public XMLTransformer
{
    friend class XMLTransformerContext;

    css::uno::Reference< css::xml::sax::XDocumentHandler >            m_xHandler;     // the handlers
    css::uno::Reference< css::beans::XPropertySet >                   m_xPropSet;
    css::uno::Reference< css::i18n::XCharacterClassification >        xCharClass;

    OUString m_aExtPathPrefix;
    OUString m_aClass;

    std::unique_ptr<SvXMLNamespaceMap> m_pNamespaceMap;
    SvXMLNamespaceMap           m_vReplaceNamespaceMap;
    std::vector<rtl::Reference<XMLTransformerContext>> m_vContexts;
    XMLTransformerActions       m_ElemActions;
    XMLTransformerTokenMap const m_TokenMap;

protected:
    css::uno::Reference< css::frame::XModel >     mxModel;

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    XMLTransformerContext *CreateContext( sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      const OUString& rQName );

public:
    XMLTransformerBase( XMLTransformerActionInit const *pInit,
                           ::xmloff::token::XMLTokenEnum const *pTKMapInit ) throw();
    virtual ~XMLTransformerBase() throw() override;

    // css::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;
    virtual void SAL_CALL startElement(const OUString& aName,
                              const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs) override;
    virtual void SAL_CALL endElement(const OUString& aName) override;
    virtual void SAL_CALL characters(const OUString& aChars) override;
    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) override;
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                       const OUString& aData) override;
    virtual void SAL_CALL setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator > & xLocator) override;

    // css::xml::sax::XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA() override;
    virtual void SAL_CALL endCDATA() override;
    virtual void SAL_CALL comment(const OUString& sComment) override;
    virtual void SAL_CALL allowLineBreak() override;
    virtual void SAL_CALL unknown(const OUString& sString) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // C++
    const css::uno::Reference< css::xml::sax::XDocumentHandler > & GetDocHandler() { return m_xHandler; }

    const css::uno::Reference< css::beans::XPropertySet > & GetPropertySet() { return m_xPropSet; }


    SvXMLNamespaceMap& GetNamespaceMap() { return *m_pNamespaceMap; }
    const SvXMLNamespaceMap& GetNamespaceMap() const { return *m_pNamespaceMap; }
    SvXMLNamespaceMap& GetReplaceNamespaceMap() { return m_vReplaceNamespaceMap; }

    XMLTransformerActions& GetElemActions() { return m_ElemActions; }
    virtual XMLTransformerActions *GetUserDefinedActions( sal_uInt16 n );
    virtual XMLTransformerContext *CreateUserDefinedContext(
                                      const TransformerAction_Impl& rAction,
                                      const OUString& rQName,
                                         bool bPersistent=false ) = 0;
    virtual OUString GetEventName( const OUString& rName,
                                             bool bForm = false ) = 0;


    XMLMutableAttributeList *ProcessAttrList( css::uno::Reference< css::xml::sax::XAttributeList >& rAttrList,
                         sal_uInt16 nActionMap, bool bClone );

    static bool ReplaceSingleInchWithIn( OUString& rValue );
    static bool ReplaceSingleInWithInch( OUString& rValue );
    static bool ReplaceInchWithIn( OUString& rValue );
    static bool ReplaceInWithInch( OUString& rValue );

    bool EncodeStyleName( OUString& rName ) const;
    static bool DecodeStyleName( OUString& rName );
    static bool NegPercent( OUString& rValue );

    void AddNamespacePrefix( OUString& rName,
                                 sal_uInt16 nPrefix ) const;
    bool RemoveNamespacePrefix( OUString& rName,
                                    sal_uInt16 nPrefixOnly=0xffffU ) const;

    bool ConvertURIToOASIS( OUString& rURI,
                                bool bSupportPackage ) const;
    bool ConvertURIToOOo( OUString& rURI,
                                bool bSupportPackage ) const;

    /** renames the given rOutAttributeValue if one of the parameters contains a
        matching token in its lower 16 bits.  The value is converted to the
        token that is given in the upper 16 bits of the matching parameter.
     */
    static bool RenameAttributeValue( OUString& rOutAttributeValue,
                                   sal_Int32 nParam1,
                                   sal_Int32 nParam2,
                                   sal_Int32 nParam3 );

    /** converts the '.' that separates fractions of seconds in a dateTime
        string into a ',' that was used in the OOo format

        @param rDateTime
            A dateTime string that will be parsed and changed in case a match
            was found.
        @return <TRUE/> if the given string was changed
     */
    static bool ConvertRNGDateTimeToISO( OUString& rDateTime );

    ::xmloff::token::XMLTokenEnum GetToken( const OUString& rStr ) const;

    const XMLTransformerContext *GetCurrentContext() const;
    const XMLTransformerContext *GetAncestorContext( sal_uInt32 i ) const;

    // C++
    void SetClass( const OUString& r ) { m_aClass = r; }
    const OUString& GetClass() const { return m_aClass; }

    bool isWriter() const;

};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
