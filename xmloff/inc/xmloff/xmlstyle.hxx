/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLOFF_XMLSTYLE_HXX
#define _XMLOFF_XMLSTYLE_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <rsc/rscsfx.hxx>
#include <tools/rtti.hxx>
#include <xmloff/uniref.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlictxt.hxx>

class SvXMLStylesContext_Impl;
class SvXMLUnitConverter;
class SvXMLImportPropertyMapper;

namespace com { namespace sun { namespace star {
namespace container { class XNameContainer; }
namespace style { class XAutoStyleFamily; }
} } }

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
    XML_TOK_STYLE_DEFAULT_PAGE_LAYOUT,  //text grid enhancement
    XML_TOK_STYLE_STYLES_ELEM_END=XML_TOK_UNKNOWN
};

class XMLOFF_DLLPUBLIC SvXMLStyleContext : public SvXMLImportContext
{
    ::rtl::OUString     maName;
    ::rtl::OUString     maDisplayName;
    ::rtl::OUString     maAutoName;
    ::rtl::OUString     maParentName;// Will be moved to XMLPropStyle soon!!!!
    ::rtl::OUString     maFollow;   // Will be moved to XMLPropStyle soon!!!!

    ::rtl::OUString     maHelpFile; // Will be removed very soon!!!!

    sal_uInt32          mnHelpId;   // Will be removed very soon!!!!
    sal_uInt16          mnFamily;

    sal_Bool            mbValid : 1;    // Set this to false in CreateAndInsert
                                    // if the style shouldn't be processed
                                    // by Finish() or si somehow invalid.
    sal_Bool            mbNew : 1;  // Set this to false in CreateAnsInsert
                                    // if the style is already existing.
    sal_Bool            mbDefaultStyle : 1;

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );

    void SetFamily( sal_uInt16 nSet ) { mnFamily = nSet; }
    void SetAutoName( const ::rtl::OUString& rName ) { maAutoName = rName; }

public:

    TYPEINFO();

    SvXMLStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
    sal_uInt16 nFamily=0, sal_Bool bDefaultStyle = sal_False );

    virtual ~SvXMLStyleContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    const ::rtl::OUString&  GetName() const { return maName; }
    const ::rtl::OUString&  GetDisplayName() const { return maDisplayName.getLength() ? maDisplayName : maName; }
    const ::rtl::OUString&  GetAutoName() const { return maAutoName; }
    const ::rtl::OUString&  GetParentName() const { return maParentName; }
    const ::rtl::OUString&  GetFollow() const { return maFollow; }

    const ::rtl::OUString&  GetHelpFile() const { return maHelpFile; }
    sal_uInt32  GetHelpId() const { return mnHelpId; }

    sal_uInt16 GetFamily() const { return mnFamily; }

    sal_Bool IsValid() const { return mbValid; }
    void SetValid( sal_Bool b ) { mbValid = b; }

    sal_Bool IsNew() const { return mbNew; }
    void SetNew( sal_Bool b ) { mbNew = b; }

    // This method is called for every default style
    virtual void SetDefaults();

    // This method is called for every style. It must create it and insert
    // it into the document.
    virtual void CreateAndInsert( sal_Bool bOverwrite );

    // This method is called for every style. It must create it and insert
    // it into the document if this hasn't happened already in CreateAndInsert().
    virtual void CreateAndInsertLate( sal_Bool bOverwrite );

    // This method is called fpr every style after all styles have been
    // inserted into the document.
    virtual void Finish( sal_Bool bOverwrite );

    sal_Bool IsDefaultStyle() const { return mbDefaultStyle; }

    /** if this method returns true, its parent styles context
        should not add it to its container.<br>
        Transient styles can't be accessed from its
        parent SvXMLStylesContext after theyre imported and
        the methods CreateAndInsert(), CreateAndInsertLate()
        and Finish() will not be called.
        The default return value is false
    */
    virtual sal_Bool IsTransient() const;
};

class XMLOFF_DLLPUBLIC SvXMLStylesContext : public SvXMLImportContext
{
    const ::rtl::OUString msParaStyleServiceName;
    const ::rtl::OUString msTextStyleServiceName;

    SvXMLStylesContext_Impl *mpImpl;
    SvXMLTokenMap           *mpStyleStylesElemTokenMap;


    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > mxParaStyles;

    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > mxTextStyles;

    ::com::sun::star::uno::Reference <
                    ::com::sun::star::style::XAutoStyleFamily > mxParaAutoStyles;

    ::com::sun::star::uno::Reference <
                    ::com::sun::star::style::XAutoStyleFamily > mxTextAutoStyles;

    UniReference < SvXMLImportPropertyMapper > mxParaImpPropMapper;
    UniReference < SvXMLImportPropertyMapper > mxTextImpPropMapper;
    UniReference < SvXMLImportPropertyMapper > mxShapeImpPropMapper;
    mutable UniReference < SvXMLImportPropertyMapper > mxChartImpPropMapper;
    mutable UniReference < SvXMLImportPropertyMapper > mxPageImpPropMapper;

    SAL_DLLPRIVATE const SvXMLTokenMap& GetStyleStylesElemTokenMap();

    SAL_DLLPRIVATE SvXMLStylesContext(SvXMLStylesContext &); // not defined
    SAL_DLLPRIVATE void operator =(SvXMLStylesContext &); // not defined

protected:

    sal_uInt32 GetStyleCount() const;
    SvXMLStyleContext *GetStyle( sal_uInt32 i );
    const SvXMLStyleContext *GetStyle( sal_uInt32 i ) const;

    virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( sal_uInt16 nFamily,
        sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual sal_Bool InsertStyleFamily( sal_uInt16 nFamily ) const;

public:
    TYPEINFO();

    SvXMLStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        sal_Bool bAutomatic = sal_False );

    virtual ~SvXMLStylesContext();

    // Create child element.
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    // This method must be overloaded to insert styles into the document.
    virtual void EndElement();

    // This allows to add an SvXMLStyleContext to this context from extern
    void AddStyle(SvXMLStyleContext& rNew);

    const SvXMLStyleContext *FindStyleChildContext(
                                      sal_uInt16 nFamily,
                                      const ::rtl::OUString& rName,
                                      sal_Bool bCreateIndex=sal_False ) const;
    virtual sal_uInt16 GetFamily( const ::rtl::OUString& rFamily ) const;
    virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const;

    virtual ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer >
        GetStylesContainer( sal_uInt16 nFamily ) const;
    virtual ::rtl::OUString GetServiceName( sal_uInt16 nFamily ) const;

       virtual ::com::sun::star::uno::Reference < ::com::sun::star::style::XAutoStyleFamily >
        GetAutoStyles( sal_uInt16 nFamily ) const;
    void CopyAutoStylesToDoc();
    void CopyStylesToDoc( sal_Bool bOverwrite, sal_Bool bFinish=sal_True );
    void FinishStyles( sal_Bool bOverwrite );

    // This method must be called to release the references to all styles
    // that are stored in the context.
    void Clear();
    sal_Bool IsAutomaticStyle() const;
};

#endif  //  _XMLOFF_XMLSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
