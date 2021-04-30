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

#ifndef INCLUDED_VCL_SOURCE_GRAPHIC_DESCRIPTOR_HXX
#define INCLUDED_VCL_SOURCE_GRAPHIC_DESCRIPTOR_HXX

#include <comphelper/propertysethelper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/weakagg.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <vcl/graph.hxx>

#define MIMETYPE_BMP        "image/x-MS-bmp"
#define MIMETYPE_GIF        "image/gif"
#define MIMETYPE_JPG        "image/jpeg"
#define MIMETYPE_PCD        "image/x-photo-cd"
#define MIMETYPE_PCX        "image/x-pcx"
#define MIMETYPE_PNG        "image/png"
#define MIMETYPE_TIF        "image/tiff"
#define MIMETYPE_XBM        "image/x-xbitmap"
#define MIMETYPE_XPM        "image/x-xpixmap"
#define MIMETYPE_PBM        "image/x-portable-bitmap"
#define MIMETYPE_PGM        "image/x-portable-graymap"
#define MIMETYPE_PPM        "image/x-portable-pixmap"
#define MIMETYPE_RAS        "image/x-cmu-raster"
#define MIMETYPE_TGA        "image/x-targa"
#define MIMETYPE_PSD        "image/vnd.adobe.photoshop"
#define MIMETYPE_EPS        "image/x-eps"
#define MIMETYPE_DXF        "image/vnd.dxf"
#define MIMETYPE_MET        "image/x-met"
#define MIMETYPE_PCT        "image/x-pict"
#define MIMETYPE_SVM        "image/x-svm"
#define MIMETYPE_WMF        "image/x-wmf"
#define MIMETYPE_EMF        "image/x-emf"
#define MIMETYPE_SVG        "image/svg+xml"
#define MIMETYPE_PDF        "application/pdf"
#define MIMETYPE_VCLGRAPHIC "image/x-vclgraphic"

namespace comphelper { class PropertySetInfo; }
namespace com::sun::star::io { class XInputStream; }

class Graphic;

namespace unographic {

class GraphicDescriptor : public ::cppu::OWeakAggObject,
                          public css::lang::XServiceInfo,
                          public css::lang::XTypeProvider,
                          public ::comphelper::PropertySetHelper
{
public:

    GraphicDescriptor();
    virtual ~GraphicDescriptor() noexcept override;

    void init( const ::Graphic& rGraphic );
    void init( const OUString& rURL );
    void init( const css::uno::Reference< css::io::XInputStream >& rxIStm, const OUString& rURL );

    static rtl::Reference<::comphelper::PropertySetInfo> createPropertySetInfo();

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

protected:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // PropertySetHelper
     virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const css::uno::Any* pValues ) override;
     virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, css::uno::Any* pValue ) override;

private:

    const ::Graphic*        mpGraphic;
    GraphicType             meType;
    OUString                maMimeType;
    Size                    maSizePixel;
    Size                    maSize100thMM;
    sal_uInt16              mnBitsPerPixel;
    bool                    mbTransparent;

    GraphicDescriptor( const GraphicDescriptor& rDescriptor ) = delete;

    GraphicDescriptor& operator=( const GraphicDescriptor& ) = delete;

    void implCreate( SvStream& rIStm, const OUString* pPath );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
