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

#ifndef _SDXMLIMP_IMPL_HXX
#define _SDXMLIMP_IMPL_HXX

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <xmloff/xmltkmap.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

#include <vector>
#include <xmloff/xmlimp.hxx>

//////////////////////////////////////////////////////////////////////////////

enum SdXMLDocElemTokenMap
{
    XML_TOK_DOC_FONTDECLS,
    XML_TOK_DOC_STYLES,
    XML_TOK_DOC_AUTOSTYLES,
    XML_TOK_DOC_MASTERSTYLES,
    XML_TOK_DOC_META,
    XML_TOK_DOC_BODY,
    XML_TOK_DOC_SCRIPT,
    XML_TOK_DOC_SETTINGS,
    XML_TOK_OFFICE_END = XML_TOK_UNKNOWN
};

enum SdXMLBodyElemTokenMap
{
    XML_TOK_BODY_PAGE,
    XML_TOK_BODY_SETTINGS,
    XML_TOK_BODY_HEADER_DECL,
    XML_TOK_BODY_FOOTER_DECL,
    XML_TOK_BODY_DATE_TIME_DECL
};

enum SdXMLStylesElemTokenMap
{
    XML_TOK_STYLES_MASTER_PAGE,
    XML_TOK_STYLES_STYLE,
    XML_TOK_STYLES_PAGE_MASTER,
    XML_TOK_STYLES_PRESENTATION_PAGE_LAYOUT
};

enum SdXMLAutoStylesElemTokenMap
{
    XML_TOK_AUTOSTYLES_STYLE
};

enum SdXMLMasterPageElemTokenMap
{
    XML_TOK_MASTERPAGE_STYLE,
    XML_TOK_MASTERPAGE_NOTES
};

enum SdXMLMasterPageAttrTokenMap
{
    XML_TOK_MASTERPAGE_NAME,
    XML_TOK_MASTERPAGE_DISPLAY_NAME,
    XML_TOK_MASTERPAGE_PAGE_MASTER_NAME,
    XML_TOK_MASTERPAGE_STYLE_NAME,
    XML_TOK_MASTERPAGE_PAGE_LAYOUT_NAME,
    XML_TOK_MASTERPAGE_USE_HEADER_NAME,
    XML_TOK_MASTERPAGE_USE_FOOTER_NAME,
    XML_TOK_MASTERPAGE_USE_DATE_TIME_NAME
};

enum SdXMLPageMasterAttrTokenMap
{
    XML_TOK_PAGEMASTER_NAME
};

enum SdXMLPageMasterStyleAttrTokenMap
{
    XML_TOK_PAGEMASTERSTYLE_MARGIN_TOP,
    XML_TOK_PAGEMASTERSTYLE_MARGIN_BOTTOM,
    XML_TOK_PAGEMASTERSTYLE_MARGIN_LEFT,
    XML_TOK_PAGEMASTERSTYLE_MARGIN_RIGHT,
    XML_TOK_PAGEMASTERSTYLE_PAGE_WIDTH,
    XML_TOK_PAGEMASTERSTYLE_PAGE_HEIGHT,
    XML_TOK_PAGEMASTERSTYLE_PAGE_ORIENTATION
};

enum SdXMLDocStyleAttrTokenMap
{
    XML_TOK_DOCSTYLE_NAME,
    XML_TOK_DOCSTYLE_FAMILY,
    XML_TOK_DOCSTYLE_PARENT_STYLE_NAME,
    XML_TOK_DOCSTYLE_AUTOMATIC
};

enum SdXMLDocStyleElemTokenMap
{
    XML_TOK_DOCSTYLE_PROPERTIES,
    XML_TOK_DOCSTYLE_PRESENTATION_PLACEHOLDER
};

enum SdXMLDrawPageAttrTokenMap
{
    XML_TOK_DRAWPAGE_NAME,
    XML_TOK_DRAWPAGE_STYLE_NAME,
    XML_TOK_DRAWPAGE_MASTER_PAGE_NAME,
    XML_TOK_DRAWPAGE_PAGE_LAYOUT_NAME,
    XML_TOK_DRAWPAGE_DRAWID,
    XML_TOK_DRAWPAGE_XMLID,
    XML_TOK_DRAWPAGE_HREF,
    XML_TOK_DRAWPAGE_USE_HEADER_NAME,
    XML_TOK_DRAWPAGE_USE_FOOTER_NAME,
    XML_TOK_DRAWPAGE_USE_DATE_TIME_NAME
};

enum SdXMLDrawPageElemTokenMap
{
    XML_TOK_DRAWPAGE_NOTES,
    XML_TOK_DRAWPAGE_PAR,
    XML_TOK_DRAWPAGE_SEQ
};

enum SdXMLPresentationPlaceholderAttrTokenMap
{
    XML_TOK_PRESENTATIONPLACEHOLDER_OBJECTNAME,
    XML_TOK_PRESENTATIONPLACEHOLDER_X,
    XML_TOK_PRESENTATIONPLACEHOLDER_Y,
    XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH,
    XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT
};

//////////////////////////////////////////////////////////////////////////////

struct SdXMLltXDrawPageId
{
  bool operator()(sal_Int32 nId1, sal_Int32 nId2 ) const
  {
    return nId1 < nId2;
  }
};

typedef std::map< sal_Int32, com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >, SdXMLltXDrawPageId > DrawPageIdMap;

//////////////////////////////////////////////////////////////////////////////

class SvXMLUnitConverter;
class SvXMLTokenMap;
class SdXMLMasterStylesContext;

//////////////////////////////////////////////////////////////////////////////

struct DateTimeDeclContextImpl
{
    rtl::OUString maStrText;
    sal_Bool mbFixed;
    rtl::OUString maStrDateTimeFormat;

    DateTimeDeclContextImpl() : mbFixed(sal_True) {}
};

DECLARE_STL_USTRINGACCESS_MAP( ::rtl::OUString, HeaderFooterDeclMap );
DECLARE_STL_USTRINGACCESS_MAP( DateTimeDeclContextImpl, DateTimeDeclMap );

//////////////////////////////////////////////////////////////////////////////

class SdXMLImport: public SvXMLImport
{
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > mxDocStyleFamilies;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocMasterPages;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocDrawPages;
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > mxPageLayouts;

    // contexts for Style and AutoStyle import
    SdXMLMasterStylesContext*   mpMasterStylesContext;

    // token map lists
    SvXMLTokenMap*              mpDocElemTokenMap;
    SvXMLTokenMap*              mpBodyElemTokenMap;
    SvXMLTokenMap*              mpStylesElemTokenMap;
    SvXMLTokenMap*              mpMasterPageElemTokenMap;
    SvXMLTokenMap*              mpMasterPageAttrTokenMap;
    SvXMLTokenMap*              mpPageMasterAttrTokenMap;
    SvXMLTokenMap*              mpPageMasterStyleAttrTokenMap;
    SvXMLTokenMap*              mpDrawPageAttrTokenMap;
    SvXMLTokenMap*              mpDrawPageElemTokenMap;
    SvXMLTokenMap*              mpPresentationPlaceholderAttrTokenMap;

    sal_uInt16                  mnStyleFamilyMask;

    sal_Int32                   mnNewPageCount;
    sal_Int32                   mnNewMasterPageCount;

    sal_Bool                    mbIsDraw;
    sal_Bool                    mbLoadDoc;
    sal_Bool                    mbPreview;

    DrawPageIdMap               maDrawPageIds;

    ::rtl::OUString             msPageLayouts;
    ::rtl::OUString             msPreview;

    HeaderFooterDeclMap         maHeaderDeclsMap;
    HeaderFooterDeclMap         maFooterDeclsMap;
    DateTimeDeclMap             maDateTimeDeclsMap;

protected:
    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext(sal_uInt16 nPrefix,
      const rtl::OUString& rLocalName,
      const com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XAttributeList>& xAttrList);

public:
    // #110680#
    SdXMLImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        sal_Bool bIsDraw, sal_uInt16 nImportFlags = IMPORT_ALL );
    ~SdXMLImport() throw ();

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    virtual void SetViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aViewProps);
    virtual void SetConfigurationSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aConfigProps);

    // namespace office
    // NB: in contrast to other CreateFooContexts, this particular one handles
    //     the root element (i.e. office:document-meta)
    SvXMLImportContext* CreateMetaContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    SvXMLImportContext* CreateScriptContext( const ::rtl::OUString& rLocalName );
    SvXMLImportContext* CreateBodyContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    SvXMLStylesContext* CreateStylesContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    SvXMLStylesContext* CreateAutoStylesContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    SvXMLImportContext* CreateMasterStylesContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    SvXMLImportContext *CreateFontDeclsContext(const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    // Styles and AutoStyles contexts
    const SdXMLMasterStylesContext* GetMasterStylesContext() const { return mpMasterStylesContext; }

    sal_uInt16 GetStyleFamilyMask() const { return mnStyleFamilyMask; }
    sal_Bool IsStylesOnlyMode() const { return !mbLoadDoc; }

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetBodyElemTokenMap();
    const SvXMLTokenMap& GetStylesElemTokenMap();
    const SvXMLTokenMap& GetMasterPageElemTokenMap();
    const SvXMLTokenMap& GetMasterPageAttrTokenMap();
    const SvXMLTokenMap& GetPageMasterAttrTokenMap();
    const SvXMLTokenMap& GetPageMasterStyleAttrTokenMap();
    const SvXMLTokenMap& GetDrawPageAttrTokenMap();
    const SvXMLTokenMap& GetDrawPageElemTokenMap();
    const SvXMLTokenMap& GetPresentationPlaceholderAttrTokenMap();

    // export local parameters concerning page access and similar
    const com::sun::star::uno::Reference<
        com::sun::star::container::XNameAccess >& GetLocalDocStyleFamilies() const { return mxDocStyleFamilies; }
    const com::sun::star::uno::Reference<
        com::sun::star::container::XIndexAccess >& GetLocalMasterPages() const { return mxDocMasterPages; }
    const com::sun::star::uno::Reference<
        com::sun::star::container::XIndexAccess >& GetLocalDrawPages() const { return mxDocDrawPages; }

    sal_Int32 GetNewPageCount() const { return mnNewPageCount; }
    void IncrementNewPageCount() { mnNewPageCount++; }
    sal_Int32 GetNewMasterPageCount() const { return mnNewMasterPageCount; }
    void IncrementNewMasterPageCount() { mnNewMasterPageCount++; }

    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > getPageLayouts() const { return mxPageLayouts; }

    sal_Bool IsDraw() const { return mbIsDraw; }
    sal_Bool IsImpress() const { return !mbIsDraw; }

    // #80365#
    virtual void SetStatistics(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue> & i_rStats);

    sal_Bool IsPreview() const { return mbPreview; }

    // XServiceInfo ( : SvXMLExport )
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    void AddHeaderDecl( const ::rtl::OUString& rName, const ::rtl::OUString& rText );
    void AddFooterDecl( const ::rtl::OUString& rName, const ::rtl::OUString& rText );
    void AddDateTimeDecl( const ::rtl::OUString& rName, const ::rtl::OUString& rText, sal_Bool bFixed, const ::rtl::OUString& rDateTimeFormat );

    ::rtl::OUString GetHeaderDecl( const ::rtl::OUString& rName ) const;
    ::rtl::OUString GetFooterDecl( const ::rtl::OUString& rName ) const;
    ::rtl::OUString GetDateTimeDecl( const ::rtl::OUString& rName, sal_Bool& rbFixed, ::rtl::OUString& rDateTimeFormat );

};

#endif  //  _SDXMLIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
