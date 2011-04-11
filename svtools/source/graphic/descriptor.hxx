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

#ifndef _GOODIES_DESCRIPTOR_HXX
#define _GOODIES_DESCRIPTOR_HXX

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

using namespace com::sun::star;

namespace comphelper { class PropertySetInfo; }
namespace com { namespace sun { namespace star { namespace io { class XInputStream; } } } }

class Graphic;

namespace unographic {

// -------------------
// - GraphicProvider -
// -------------------

class GraphicDescriptor : public ::cppu::OWeakAggObject,
                          public ::com::sun::star::lang::XServiceInfo,
                          public ::com::sun::star::lang::XTypeProvider,
                          public ::comphelper::PropertySetHelper
{
public:

    GraphicDescriptor();
    ~GraphicDescriptor() throw();

     void init( const ::Graphic& rGraphic ) throw();
    void init( const ::rtl::OUString& rURL ) throw();
    void init( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxIStm, const ::rtl::OUString& rURL ) throw();

    bool isValid() const;

    static ::rtl::OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();

protected:

    static ::comphelper::PropertySetInfo* createPropertySetInfo();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // PropertySetHelper
        virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
        virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

private:

    const ::Graphic*        mpGraphic;
    GraphicType             meType;
    ::rtl::OUString         maMimeType;
    Size                    maSizePixel;
    Size                    maSize100thMM;
    sal_uInt16              mnBitsPerPixel;
    bool                    mbTransparent;
    bool                    mbAlpha;
    bool                    mbAnimated;

    GraphicDescriptor( const GraphicDescriptor& rDescriptor );

    GraphicDescriptor& operator=( const GraphicDescriptor& );

    void implCreate( SvStream& rIStm, const ::rtl::OUString* pPath );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
