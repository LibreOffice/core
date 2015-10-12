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

#ifndef INCLUDED_SVTOOLS_SOURCE_GRAPHIC_DESCRIPTOR_HXX
#define INCLUDED_SVTOOLS_SOURCE_GRAPHIC_DESCRIPTOR_HXX

#include <comphelper/propertysethelper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

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
#define MIMETYPE_SGF        "image/x-sgf"
#define MIMETYPE_SVM        "image/x-svm"
#define MIMETYPE_WMF        "image/x-wmf"
#define MIMETYPE_SGV        "image/x-sgv"
#define MIMETYPE_EMF        "image/x-emf"
#define MIMETYPE_SVG        "image/svg+xml"
#define MIMETYPE_VCLGRAPHIC "image/x-vclgraphic"

namespace comphelper { class PropertySetInfo; }
namespace com { namespace sun { namespace star { namespace io { class XInputStream; } } } }

class Graphic;

namespace unographic {

class GraphicDescriptor : public ::cppu::OWeakAggObject,
                          public ::com::sun::star::lang::XServiceInfo,
                          public ::com::sun::star::lang::XTypeProvider,
                          public ::comphelper::PropertySetHelper
{
public:

    GraphicDescriptor();
    virtual ~GraphicDescriptor() throw();

    void init( const ::Graphic& rGraphic );
    void init( const OUString& rURL );
    void init( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxIStm, const OUString& rURL );

    static OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

protected:

    static ::comphelper::PropertySetInfo* createPropertySetInfo();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // PropertySetHelper
        virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException ) override;
        virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

private:

    const ::Graphic*        mpGraphic;
    GraphicType             meType;
    OUString         maMimeType;
    Size                    maSizePixel;
    Size                    maSize100thMM;
    sal_uInt16              mnBitsPerPixel;
    bool                    mbTransparent;
    bool                    mbAlpha;
    bool                    mbAnimated;

    GraphicDescriptor( const GraphicDescriptor& rDescriptor ) = delete;

    GraphicDescriptor& operator=( const GraphicDescriptor& ) = delete;

    void implCreate( SvStream& rIStm, const OUString* pPath );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
