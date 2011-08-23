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

#ifndef _RSCSFX_HXX //autogen wg. SfxStyleFamily
#include <rsc/rscsfx.hxx>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _UNIVERSALL_REFERENCE_HXX
#include <bf_xmloff/uniref.hxx>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include <bf_xmloff/xmltkmap.hxx>
#endif
#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif
namespace com { namespace sun { namespace star { namespace container
{
    class XNameContainer;
} } } }
namespace binfilter {

class SvXMLStylesContext_Impl;
class SvXMLUnitConverter;
class SvXMLImportPropertyMapper;



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
    XML_TOK_TEXT_FOOTNOTE_CONFIG,
    XML_TOK_TEXT_ENDNOTE_CONFIG,
    XML_TOK_TEXT_BIBLIOGRAPHY_CONFIG,
    XML_TOK_TEXT_LINENUMBERING_CONFIG,
    XML_TOK_STYLE_DEFAULT_STYLE,
    XML_TOK_STYLE_STYLES_ELEM_END=XML_TOK_UNKNOWN
};

class SvXMLStyleContext : public SvXMLImportContext
{
    ::rtl::OUString		aName;
    ::rtl::OUString		aParent;	// Will be moved to XMLPropStyle soon!!!!
    ::rtl::OUString		aFollow;	// Will be moved to XMLPropStyle soon!!!!

    ::rtl::OUString		aHelpFile;	// Will be removed very soon!!!!

    sal_uInt32			nHelpId;	// Will be removed very soon!!!!
    sal_uInt16			nFamily;

    sal_Bool			bValid : 1;	// Set this to false in CreateAndInsert
                                    // if the style shouldn't be processed
                                    // by Finish() or si somehow invalid.
    sal_Bool			bNew : 1;	// Set this to false in CreateAnsInsert
                                    // if the style is already existing.
    sal_Bool			bDefaultStyle : 1;

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );

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

    const ::rtl::OUString&	GetName() const { return aName; }
    const ::rtl::OUString&	GetParent() const { return aParent; }
    const ::rtl::OUString&	GetFollow() const { return aFollow; }

    const ::rtl::OUString&	GetHelpFile() const { return aHelpFile; }
    sal_uInt32	GetHelpId() const { return nHelpId; }

    sal_uInt16 GetFamily() const { return nFamily; }

    BOOL IsValid() const { return bValid; }
    void SetValid( sal_Bool b ) { bValid = b; }

    BOOL IsNew() const { return bNew; }
    void SetNew( sal_Bool b ) { bNew = b; }

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

    sal_Bool IsDefaultStyle() const { return bDefaultStyle; }

    /** if this method returns true, its parent styles context
        should not add it to its container.<br>
        Transient styles can't be accessed from its
        parent SvXMLStylesContext after theyre imported and
        the methods	CreateAndInsert(), CreateAndInsertLate()
        and Finish() will not be called.
        The default return value is false
    */
    virtual BOOL IsTransient() const;
};

class SvXMLStylesContext : public SvXMLImportContext
{
    const ::rtl::OUString sParaStyleServiceName;
    const ::rtl::OUString sTextStyleServiceName;

    SvXMLStylesContext_Impl	*pImpl;
    SvXMLTokenMap			*pStyleStylesElemTokenMap;

    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > xParaStyles;
    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > xTextStyles;

    UniReference < SvXMLImportPropertyMapper > xParaImpPropMapper;
    UniReference < SvXMLImportPropertyMapper > xTextImpPropMapper;
    UniReference < SvXMLImportPropertyMapper > xShapeImpPropMapper;
    mutable UniReference < SvXMLImportPropertyMapper > xChartImpPropMapper;
    mutable UniReference < SvXMLImportPropertyMapper > xPageImpPropMapper;

    const SvXMLTokenMap& GetStyleStylesElemTokenMap();

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
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

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
                                      BOOL bCreateIndex=sal_False ) const;
    virtual sal_uInt16 GetFamily( const ::rtl::OUString& rFamily ) const;
    virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const;

    virtual ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer >
        GetStylesContainer( sal_uInt16 nFamily ) const;
    virtual ::rtl::OUString GetServiceName( sal_uInt16 nFamily ) const;

    void CopyStylesToDoc( sal_Bool bOverwrite, sal_Bool bFinish=sal_True );
    void FinishStyles( sal_Bool bOverwrite );

    // This method must be called to release the references to all styles
    // that are stored in the context.
    void Clear();
};

class SvXMLUseStylesContext : public SvXMLImportContext
{
public:

    SvXMLUseStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        ::rtl::OUString& rHRef );

    virtual ~SvXMLUseStylesContext();
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLSTYLE_HXX

