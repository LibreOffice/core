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

#ifndef _XIMPPAGE_HXX
#define _XIMPPAGE_HXX

#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <tools/rtti.hxx>
#include "ximpshap.hxx"

// draw:g context (RECURSIVE)

class SdXMLGenericPageContext : public SvXMLImportContext
{
    // the shape group this group is working on
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxShapes;
    ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotationAccess > mxAnnotationAccess;

protected:
    OUString               maPageLayoutName;
    OUString               maUseHeaderDeclName;
    OUString               maUseFooterDeclName;
    OUString               maUseDateTimeDeclName;
    OUString               msNavOrder;

    void SetLocalShapesContext(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rNew)
        { mxShapes = rNew; }

    /** sets the page style on this page */
    void SetStyle( OUString& rStyleName );

    /** sets the presentation layout at this page. It is used for drawing pages and for the handout master */
    void SetLayout();

    /** deletes all shapes on this drawing page */
    void DeleteAllShapes();

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

    /** sets the properties from a page master style with the given name on this contexts page */
    void SetPageMaster( OUString& rsPageMasterName );

    void SetNavigationOrder();

public:
    TYPEINFO();

    SdXMLGenericPageContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLGenericPageContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();

    const com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& GetLocalShapesContext() const
        { return mxShapes; }
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& GetLocalShapesContext()
        { return mxShapes; }
};

#endif  //  _XIMPGROUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
