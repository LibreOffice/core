/*************************************************************************
 *
 *  $RCSfile: ximpstyl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:04 $
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

#ifndef _XIMPSTYLE_HXX
#define _XIMPSTYLE_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _SDXMLIMP_HXX
#include "sdxmlimp.hxx"
#endif

#ifndef _XIMPGROUP_HXX
#include "ximpgrp.hxx"
#endif

#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

#ifndef _COM_SUN_STAR_VIEW_PAPERORIENTATION_HPP_
#include <com/sun/star/view/PaperOrientation.hpp>
#endif

//////////////////////////////////////////////////////////////////////////////
// style:page-master context

class SdXMLPageMasterContext: public SvXMLStyleContext
{
    sal_Int32                   mnBorderBottom;
    sal_Int32                   mnBorderLeft;
    sal_Int32                   mnBorderRight;
    sal_Int32                   mnBorderTop;
    sal_Int32                   mnWidth;
    sal_Int32                   mnHeight;
    com::sun::star::view::PaperOrientation meOrientation;
    rtl::OUString               msName;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    TYPEINFO();

    SdXMLPageMasterContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    virtual ~SdXMLPageMasterContext();

    const rtl::OUString& GetName() const { return msName; }
    sal_Int32 GetBorderBottom() const { return mnBorderBottom; }
    sal_Int32 GetBorderLeft() const { return mnBorderLeft; }
    sal_Int32 GetBorderRight() const { return mnBorderRight; }
    sal_Int32 GetBorderTop() const { return mnBorderTop; }
    sal_Int32 GetWidth() const { return mnWidth; }
    sal_Int32 GetHeight() const { return mnHeight; }
    com::sun::star::view::PaperOrientation GetOrientation() const { return meOrientation; }
};

//////////////////////////////////////////////////////////////////////////////
// style:masterpage context

class SdXMLMasterPageContext: public SdXMLGroupShapeContext
{
    rtl::OUString               msPageMasterName;
    rtl::OUString               msName;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    TYPEINFO();

    SdXMLMasterPageContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLMasterPageContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

    const rtl::OUString& GetPageMasterName() const { return msPageMasterName; }
    const rtl::OUString& GetName() const { return msName; }
};
DECLARE_LIST(ImpMasterPageList, SdXMLMasterPageContext*);

//////////////////////////////////////////////////////////////////////////////
// presentation:placeholder context

class SdXMLPresentationPlaceholderContext: public SvXMLImportContext
{
    rtl::OUString               msName;
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
        const rtl::OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    virtual ~SdXMLPresentationPlaceholderContext();

    const rtl::OUString& GetName() const { return msName; }
    sal_Int32 GetX() const { return mnX; }
    sal_Int32 GetY() const { return mnY; }
    sal_Int32 GetWidth() const { return mnWidth; }
    sal_Int32 GetHeight() const { return mnHeight; }
};
DECLARE_LIST(ImpPlaceholderList, SdXMLPresentationPlaceholderContext*);

//////////////////////////////////////////////////////////////////////////////
// style:presentation-page-layout context

class SdXMLPresentationPageLayoutContext: public SvXMLStyleContext
{
    rtl::OUString               msName;
    ImpPlaceholderList          maList;
    sal_uInt16                  mnTypeId;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    TYPEINFO();

    SdXMLPresentationPageLayoutContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    virtual ~SdXMLPresentationPageLayoutContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();
    sal_uInt16 GetTypeId() const { return mnTypeId; }
};

//////////////////////////////////////////////////////////////////////////////
// office:styles context

class SdXMLStylesContext : public SvXMLStylesContext
{
    UniReference< SvXMLImportPropertyMapper > xPresImpPropMapper;
    sal_Bool                    mbIsAutoStyle;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

    void ImpSetGraphicStyles() const;
    void ImpSetGraphicStyles( com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& xPageStyles,
        sal_uInt16 nFamily, const UniString& rPrefix) const;

protected:
    virtual SvXMLStyleContext* CreateStyleChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);

    virtual SvXMLStyleContext *CreateStyleStyleChildContext(
        sal_uInt16 nFamily,
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);

public:
    TYPEINFO();

    SdXMLStylesContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
        sal_Bool bIsAutoStyle);
    virtual ~SdXMLStylesContext();

    virtual sal_uInt16 GetFamily( const rtl::OUString& rFamily ) const;
    virtual void EndElement();
    virtual UniReference< SvXMLImportPropertyMapper > GetImportPropertyMapper(sal_uInt16 nFamily) const;

    void SetMasterPageStyles(SdXMLMasterPageContext& rMaster) const;
};

//////////////////////////////////////////////////////////////////////////////
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
        const rtl::OUString& rLName);
    virtual ~SdXMLMasterStylesContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

    const ImpMasterPageList& GetMasterPageList() const { return maMasterPageList; }
};


#endif  //  _XIMPSTYLE_HXX
