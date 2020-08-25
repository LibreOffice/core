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

#ifndef INCLUDED_XMLOFF_XMLSTYLE_HXX
#define INCLUDED_XMLOFF_XMLSTYLE_HXX

#include <rtl/ref.hxx>
#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/families.hxx>
#include <memory>

class SvXMLStylesContext_Impl;
class SvXMLImportPropertyMapper;
class SvXMLTokenMap;

namespace com::sun::star {
namespace container { class XNameContainer; }
namespace style { class XAutoStyleFamily; }
}

enum XMLStyleStylesElemTokens
{
    XML_TOK_STYLE_STYLE,
    XML_TOK_STYLE_PAGE_MASTER,
    XML_TOK_TEXT_LIST_STYLE,
    XML_TOK_TEXT_OUTLINE,
    XML_TOK_STYLES_GRADIENTSTYLES,
    XML_TOK_STYLES_HATCHSTYLES,
    XML_TOK_STYLES_BITMAPSTYLES,
    XML_TOK_STYLES_TRANSGRADIENTSTYLES,
    XML_TOK_STYLES_MARKERSTYLES,
    XML_TOK_STYLES_DASHSTYLES,
    XML_TOK_TEXT_NOTE_CONFIG,
    XML_TOK_TEXT_BIBLIOGRAPHY_CONFIG,
    XML_TOK_TEXT_LINENUMBERING_CONFIG,
    XML_TOK_STYLE_DEFAULT_STYLE,
    XML_TOK_STYLE_DEFAULT_PAGE_LAYOUT  //text grid enhancement
};

class XMLOFF_DLLPUBLIC SvXMLStyleContext : public SvXMLImportContext
{
    OUString     maName;
    OUString     maDisplayName;
    OUString     maAutoName;
    OUString     maParentName;// Will be moved to XMLPropStyle soon!!!!
    OUString     maFollow;    // Will be moved to XMLPropStyle soon!!!!
    bool         mbHidden;

    XmlStyleFamily mnFamily;

    bool         mbValid : 1; // Set this to false in CreateAndInsert
                              // if the style shouldn't be processed
                              // by Finish() or si somehow invalid.
    bool         mbNew : 1;   // Set this to false in CreateAnsInsert
                              // if the style is already existing.
    bool         mbDefaultStyle : 1;

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue );

    void SetFamily( XmlStyleFamily nSet ) { mnFamily = nSet; }
    void SetAutoName( const OUString& rName ) { maAutoName = rName; }

public:


    SvXMLStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
              XmlStyleFamily nFamily=XmlStyleFamily::DATA_STYLE,
              bool bDefaultStyle = false );

    SvXMLStyleContext( SvXMLImport& rImport,
            sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList,
            XmlStyleFamily nFamily=XmlStyleFamily::DATA_STYLE,
            bool bDefaultStyle = false );

    // Fast-parser constructor
    SvXMLStyleContext( SvXMLImport& rImport,
              XmlStyleFamily nFamily=XmlStyleFamily::DATA_STYLE,
              bool bDefaultStyle = false );

    virtual ~SvXMLStyleContext() override;

    virtual void StartElement(
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) final override;

    virtual void SAL_CALL startFastElement(
            sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override;

    const OUString&  GetName() const { return maName; }
    const OUString&  GetDisplayName() const { return maDisplayName.getLength() ? maDisplayName : maName; }
    const OUString&  GetAutoName() const { return maAutoName; }
    const OUString&  GetParentName() const { return maParentName; }
    const OUString&  GetFollow() const { return maFollow; }

    XmlStyleFamily GetFamily() const { return mnFamily; }

    bool IsValid() const { return mbValid; }
    void SetValid( bool b ) { mbValid = b; }

    bool IsNew() const { return mbNew; }
    void SetNew( bool b ) { mbNew = b; }

    bool IsHidden() const { return mbHidden; }

    // This method is called for every default style
    virtual void SetDefaults();

    // This method is called for every style. It must create it and insert
    // it into the document.
    virtual void CreateAndInsert( bool bOverwrite );

    // This method is called for every style. It must create it and insert
    // it into the document if this hasn't happened already in CreateAndInsert().
    virtual void CreateAndInsertLate( bool bOverwrite );

    // This method is called for every style after all styles have been
    // inserted into the document.
    virtual void Finish( bool bOverwrite );

    bool IsDefaultStyle() const { return mbDefaultStyle; }

    /** if this method returns true, its parent styles context
        should not add it to its container.<br>
        Transient styles can't be accessed from its
        parent SvXMLStylesContext after they are imported and
        the methods CreateAndInsert(), CreateAndInsertLate()
        and Finish() will not be called.
        The default return value is false
    */
    virtual bool IsTransient() const;
};

class XMLOFF_DLLPUBLIC SvXMLStylesContext : public SvXMLImportContext
{
    std::unique_ptr<SvXMLStylesContext_Impl> mpImpl;
    std::unique_ptr<SvXMLTokenMap>           mpStyleStylesElemTokenMap;


    css::uno::Reference< css::container::XNameContainer > mxParaStyles;

    css::uno::Reference< css::container::XNameContainer > mxTextStyles;

    css::uno::Reference< css::style::XAutoStyleFamily > mxParaAutoStyles;

    css::uno::Reference< css::style::XAutoStyleFamily > mxTextAutoStyles;

    rtl::Reference < SvXMLImportPropertyMapper > mxParaImpPropMapper;
    rtl::Reference < SvXMLImportPropertyMapper > mxTextImpPropMapper;
    rtl::Reference < SvXMLImportPropertyMapper > mxShapeImpPropMapper;
    mutable rtl::Reference < SvXMLImportPropertyMapper > mxChartImpPropMapper;
    mutable rtl::Reference < SvXMLImportPropertyMapper > mxPageImpPropMapper;

    SAL_DLLPRIVATE const SvXMLTokenMap& GetStyleStylesElemTokenMap();

    SvXMLStylesContext(SvXMLStylesContext const &) = delete;
    SvXMLStylesContext& operator =(SvXMLStylesContext const &) = delete;

protected:

    sal_uInt32 GetStyleCount() const;
    SvXMLStyleContext *GetStyle( sal_uInt32 i );
    const SvXMLStyleContext *GetStyle( sal_uInt32 i ) const;

    virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLStyleContext *CreateStyleChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( XmlStyleFamily nFamily,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( XmlStyleFamily nFamily,
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList );

    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList );

    virtual bool InsertStyleFamily( XmlStyleFamily nFamily ) const;

public:

    SvXMLStylesContext( SvXMLImport& rImport,
        bool bAutomatic = false );

    virtual ~SvXMLStylesContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}

    // Create child element.
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    // Create child element.
    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    // Override this method to insert styles into the document.
    virtual void EndElement() override;

    // This allows to add an SvXMLStyleContext to this context from extern
    void AddStyle(SvXMLStyleContext& rNew);

    const SvXMLStyleContext *FindStyleChildContext(
                                      XmlStyleFamily nFamily,
                                      const OUString& rName,
                                      bool bCreateIndex = false ) const;
    static XmlStyleFamily GetFamily( const OUString& rFamily );
    virtual rtl::Reference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                        XmlStyleFamily nFamily ) const;

    virtual css::uno::Reference< css::container::XNameContainer >
        GetStylesContainer( XmlStyleFamily nFamily ) const;
    virtual OUString GetServiceName( XmlStyleFamily nFamily ) const;

    css::uno::Reference< css::style::XAutoStyleFamily >
        GetAutoStyles( XmlStyleFamily nFamily ) const;
    void CopyAutoStylesToDoc();
    void CopyStylesToDoc( bool bOverwrite, bool bFinish = true );
    void FinishStyles( bool bOverwrite );

    // This method must be called to release the references to all styles
    // that are stored in the context.
    void dispose();
    bool IsAutomaticStyle() const;
};

#endif // INCLUDED_XMLOFF_XMLSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
