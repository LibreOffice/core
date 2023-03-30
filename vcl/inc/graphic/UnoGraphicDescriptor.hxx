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

inline constexpr OUStringLiteral MIMETYPE_BMP = u"image/x-MS-bmp";
inline constexpr OUStringLiteral MIMETYPE_GIF = u"image/gif";
inline constexpr OUStringLiteral MIMETYPE_JPG = u"image/jpeg";
inline constexpr OUStringLiteral MIMETYPE_PCD = u"image/x-photo-cd";
inline constexpr OUStringLiteral MIMETYPE_PCX = u"image/x-pcx";
inline constexpr OUStringLiteral MIMETYPE_PNG = u"image/png";
inline constexpr OUStringLiteral MIMETYPE_TIF = u"image/tiff";
inline constexpr OUStringLiteral MIMETYPE_XBM = u"image/x-xbitmap";
inline constexpr OUStringLiteral MIMETYPE_XPM = u"image/x-xpixmap";
inline constexpr OUStringLiteral MIMETYPE_PBM = u"image/x-portable-bitmap";
inline constexpr OUStringLiteral MIMETYPE_PGM = u"image/x-portable-graymap";
inline constexpr OUStringLiteral MIMETYPE_PPM = u"image/x-portable-pixmap";
inline constexpr OUStringLiteral MIMETYPE_RAS = u"image/x-cmu-raster";
inline constexpr OUStringLiteral MIMETYPE_TGA = u"image/x-targa";
inline constexpr OUStringLiteral MIMETYPE_PSD = u"image/vnd.adobe.photoshop";
inline constexpr OUStringLiteral MIMETYPE_EPS = u"image/x-eps";
inline constexpr OUStringLiteral MIMETYPE_DXF = u"image/vnd.dxf";
inline constexpr OUStringLiteral MIMETYPE_MET = u"image/x-met";
inline constexpr OUStringLiteral MIMETYPE_PCT = u"image/x-pict";
inline constexpr OUStringLiteral MIMETYPE_SVM = u"image/x-svm";
inline constexpr OUStringLiteral MIMETYPE_WMF = u"image/x-wmf";
inline constexpr OUStringLiteral MIMETYPE_EMF = u"image/x-emf";
inline constexpr OUStringLiteral MIMETYPE_SVG = u"image/svg+xml";
inline constexpr OUStringLiteral MIMETYPE_PDF = u"application/pdf";
inline constexpr OUStringLiteral MIMETYPE_WEBP = u"image/webp";
inline constexpr OUStringLiteral MIMETYPE_VCLGRAPHIC = u"image/x-vclgraphic";

namespace comphelper { class PropertySetInfo; }
namespace com::sun::star::io { class XInputStream; }

class Graphic;

namespace unographic {

class GraphicDescriptor : public ::cppu::OWeakObject,
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
