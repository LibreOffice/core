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

#ifndef _XIMPSTYLE_HXX
#define _XIMPSTYLE_HXX

#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include "ximppage.hxx"
#include <xmloff/xmlstyle.hxx>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <vector>

class SvNumberFormatter;
class SvXMLNumFmtHelper;

// special style:style context inside style:page-master context

class SdXMLPageMasterStyleContext: public SvXMLStyleContext
{
    sal_Int32                   mnBorderBottom;
    sal_Int32                   mnBorderLeft;
    sal_Int32                   mnBorderRight;
    sal_Int32                   mnBorderTop;
    sal_Int32                   mnWidth;
    sal_Int32                   mnHeight;
    com::sun::star::view::PaperOrientation meOrientation;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    TYPEINFO();

    SdXMLPageMasterStyleContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    virtual ~SdXMLPageMasterStyleContext();

    sal_Int32 GetBorderBottom() const { return mnBorderBottom; }
    sal_Int32 GetBorderLeft() const { return mnBorderLeft; }
    sal_Int32 GetBorderRight() const { return mnBorderRight; }
    sal_Int32 GetBorderTop() const { return mnBorderTop; }
    sal_Int32 GetWidth() const { return mnWidth; }
    sal_Int32 GetHeight() const { return mnHeight; }
    com::sun::star::view::PaperOrientation GetOrientation() const { return meOrientation; }
};

// style:page-master context

class SdXMLPageMasterContext: public SvXMLStyleContext
{
    OUString               msName;
    SdXMLPageMasterStyleContext*mpPageMasterStyle;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    TYPEINFO();

    SdXMLPageMasterContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    virtual ~SdXMLPageMasterContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

    const OUString& GetName() const { return msName; }
    const SdXMLPageMasterStyleContext* GetPageMasterStyle() const { return mpPageMasterStyle; }
};

// style:masterpage context

class SdXMLMasterPageContext: public SdXMLGenericPageContext
{
    OUString               msPageMasterName;
    OUString               msName;
    OUString               msDisplayName;
    OUString               msStyleName;

public:
    TYPEINFO();

    SdXMLMasterPageContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLMasterPageContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

    const OUString& GetPageMasterName() const { return msPageMasterName; }
    const OUString& GetEncodedName() const { return msName; }
    const OUString& GetDisplayName() const { return msDisplayName; }

};
typedef ::std::vector< SdXMLMasterPageContext* > ImpMasterPageList;

// presentation:placeholder context

class SdXMLPresentationPlaceholderContext: public SvXMLImportContext
{
    OUString               msName;
    sal_Int32                   mnX;
    sal_Int32                   mnY;
    sal_Int32                   mnWidth;
    sal_Int32                   mnHeight;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    SdXMLPresentationPlaceholderContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    virtual ~SdXMLPresentationPlaceholderContext();

    const OUString& GetName() const { return msName; }
    sal_Int32 GetX() const { return mnX; }
    sal_Int32 GetY() const { return mnY; }
    sal_Int32 GetWidth() const { return mnWidth; }
    sal_Int32 GetHeight() const { return mnHeight; }
};
typedef ::std::vector< SdXMLPresentationPlaceholderContext* > ImpPlaceholderList;

// style:presentation-page-layout context

class SdXMLPresentationPageLayoutContext: public SvXMLStyleContext
{
    OUString               msName;
    ImpPlaceholderList          maList;
    sal_uInt16                  mnTypeId;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    TYPEINFO();

    SdXMLPresentationPageLayoutContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    virtual ~SdXMLPresentationPageLayoutContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();
    sal_uInt16 GetTypeId() const { return mnTypeId; }
};

// office:styles context

class SdXMLStylesContext : public SvXMLStylesContext
{
    UniReference< SvXMLImportPropertyMapper > xPresImpPropMapper;
    sal_Bool                    mbIsAutoStyle;
    SvXMLNumFmtHelper*          mpNumFmtHelper;
    SvNumberFormatter*          mpNumFormatter;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

    void ImpSetGraphicStyles() const;
    void ImpSetCellStyles() const;
    void ImpSetGraphicStyles( com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& xPageStyles,
        sal_uInt16 nFamily, const OUString& rPrefix) const;

protected:
    virtual SvXMLStyleContext* CreateStyleChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);

    virtual SvXMLStyleContext *CreateStyleStyleChildContext(
        sal_uInt16 nFamily,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);

    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    TYPEINFO();

    SdXMLStylesContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
        sal_Bool bIsAutoStyle);
    virtual ~SdXMLStylesContext();

    virtual sal_uInt16 GetFamily( const OUString& rFamily ) const;
    virtual void EndElement();
    virtual UniReference< SvXMLImportPropertyMapper > GetImportPropertyMapper(sal_uInt16 nFamily) const;

    void SetMasterPageStyles(SdXMLMasterPageContext& rMaster) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > getPageLayouts() const;
};

// office:master-styles context
//
class SdXMLMasterStylesContext : public SvXMLImportContext
{
    ImpMasterPageList           maMasterPageList;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    TYPEINFO();

    SdXMLMasterStylesContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName);
    virtual ~SdXMLMasterStylesContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

    const ImpMasterPageList& GetMasterPageList() const { return maMasterPageList; }
};

// <pres:header-decl>, <pres:footer-decl> and <pres:date-time-decl>

class SdXMLHeaderFooterDeclContext : public SvXMLStyleContext
{
public:
    SdXMLHeaderFooterDeclContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual sal_Bool IsTransient() const;
    virtual void EndElement();
    virtual void Characters( const OUString& rChars );

private:
    OUString maStrName;
    OUString maStrText;
    OUString maStrDateTimeFormat;
    sal_Bool        mbFixed;
};

#endif  //  _XIMPSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
