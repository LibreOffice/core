/*************************************************************************
 *
 *  $RCSfile: descriptor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 09:49:56 $
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

#ifndef _GOODIES_DESCRIPTOR_HXX
#define _GOODIES_DESCRIPTOR_HXX

#ifndef _COMPHELPER_PROPERTYSETHELPER_HXX_
#include <comphelper/propertysethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#include <comphelper/propertysetinfo.hxx>
#include <vcl/graph.hxx>

#define MIMETYPE_BMP        "image/bmp"
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
#define MIMETYPE_PSD        "image/x-photoshop"
#define MIMETYPE_EPS        "image/x-eps"
#define MIMETYPE_DXF        "image/x-dxf"
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
    void init( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxIStm ) throw();

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
    sal_Int8                mnBitsPerPixel;
    bool                    mbTransparent;
    bool                    mbAlpha;
    bool                    mbAnimated;

    GraphicDescriptor( const GraphicDescriptor& rDescriptor );

    GraphicDescriptor& operator=( const GraphicDescriptor& );

    void implCreate( SvStream& rIStm, const String* pPath );
};

}

#endif
