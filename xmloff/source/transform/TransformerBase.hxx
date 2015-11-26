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

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <xmloff/xmltoken.hxx>

#include "Transformer.hxx"

namespace com { namespace sun { namespace star {
    namespace i18n { class XCharacterClassification; }
}}}

class SvXMLNamespaceMap;
class XMLTransformerContext;
class XMLTransformerContextVector;
class XMLTransformerActions;
struct XMLTransformerActionInit;
struct TransformerAction_Impl;
class XMLMutableAttributeList;
class XMLTransformerTokenMap;

const sal_uInt16 INVALID_ACTIONS = 0xffff;

class XMLTransformerBase : public XMLTransformer
{
    friend class XMLTransformerContext;

    css::uno::Reference< css::xml::sax::XLocator >                    m_xLocator;
    css::uno::Reference< css::xml::sax::XDocumentHandler >            m_xHandler;     // the handlers
    css::uno::Reference< css::xml::sax::XExtendedDocumentHandler >    m_xExtHandler;
    css::uno::Reference< css::beans::XPropertySet >                   m_xPropSet;
    css::uno::Reference< css::i18n::XCharacterClassification >        xCharClass;

    OUString m_aExtPathPrefix;
    OUString m_aClass;

    SvXMLNamespaceMap           *m_pNamespaceMap;
    SvXMLNamespaceMap           *m_pReplaceNamespaceMap;
    XMLTransformerContextVector *m_pContexts;
    XMLTransformerActions       *m_pElemActions;
    XMLTransformerTokenMap      *m_pTokenMap;

protected:
    css::uno::Reference< css::frame::XModel >     mxModel;

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    XMLTransformerContext *CreateContext( sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      const OUString& rQName );

public:
    XMLTransformerBase( XMLTransformerActionInit *pInit=nullptr,
                           ::xmloff::token::XMLTokenEnum *pTKMapInit=nullptr ) throw();
    virtual ~XMLTransformerBase() throw();

    // css::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endDocument()
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL startElement(const OUString& aName,
                              const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endElement(const OUString& aName)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL characters(const OUString& aChars)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                       const OUString& aData)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator > & xLocator)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    // css::xml::sax::XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA() throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endCDATA() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL comment(const OUString& sComment)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL allowLineBreak()
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL unknown(const OUString& sString)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // C++
    const css::uno::Reference< css::xml::sax::XDocumentHandler > & GetDocHandler() { return m_xHandler; }

    const css::uno::Reference< css::beans::XPropertySet > & GetPropertySet() { return m_xPropSet; }


    SvXMLNamespaceMap& GetNamespaceMap() { return *m_pNamespaceMap; }
    const SvXMLNamespaceMap& GetNamespaceMap() const { return *m_pNamespaceMap; }
    SvXMLNamespaceMap& GetReplaceNamespaceMap() { return *m_pReplaceNamespaceMap; }

    XMLTransformerActions& GetElemActions() { return *m_pElemActions; }
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

    bool AddNamespacePrefix( OUString& rName,
                                 sal_uInt16 nPrefix ) const;
    bool RemoveNamespacePrefix( OUString& rName,
                                    sal_uInt16 nPrefixOnly=0xffffU ) const;

    bool ConvertURIToOASIS( OUString& rURI,
                                bool bSupportPackage=false ) const;
    bool ConvertURIToOOo( OUString& rURI,
                                bool bSupportPackage=false ) const;

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
    inline void SetClass( const OUString& r ) { m_aClass = r; }
    inline const OUString& GetClass() const { return m_aClass; }

    bool isWriter() const;

};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
