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
#ifndef INCLUDED_OLEHANDLER_HXX
#define INCLUDED_OLEHANDLER_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <resourcemodel/WW8ResourceModel.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>

#include <com/sun/star/drawing/XShape.hpp>

namespace com{ namespace sun{ namespace star{
    namespace embed{
        class XEmbeddedObject;
    }
    namespace graphic{
        class XGraphic;
    }
    namespace io{
        class XInputStream;
    }
    namespace text{
        class XTextDocument;
    }
    namespace uno{
        class XComponentContext;
    }
}}}
namespace writerfilter {
namespace dmapper
{
//class PropertyMap;
/** Handler for OLE objects
 */
class WRITERFILTER_DLLPRIVATE OLEHandler : public Properties
{
    ::rtl::OUString     m_sObjectType;
    ::rtl::OUString     m_sProgId;
    ::rtl::OUString     m_sShapeId;
    ::rtl::OUString     m_sDrawAspect;
    ::rtl::OUString     m_sObjectId;
    ::rtl::OUString     m_sr_id;

    sal_Int32                   m_nDxaOrig;
    sal_Int32                   m_nDyaOrig;
    sal_Int32                   m_nWrapMode;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > m_xShape;

    ::com::sun::star::awt::Size m_aShapeSize;
    ::com::sun::star::awt::Point m_aShapePosition;

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > m_xReplacement;
    
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xInputStream;
public:
    OLEHandler();
    virtual ~OLEHandler();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    inline ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getShape( ) { return m_xShape; };

    inline bool isOLEObject( ) { return m_xInputStream.is( ); };

    ::rtl::OUString copyOLEOStream( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument > xTextDocument );

    ::com::sun::star::awt::Size     getSize() const { return m_aShapeSize;}
    ::com::sun::star::awt::Point    getPosition() const { return m_aShapePosition;}
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > 
                                    getReplacement() const { return m_xReplacement; }

};
typedef boost::shared_ptr< OLEHandler >  OLEHandlerPtr;
}}

#endif //
