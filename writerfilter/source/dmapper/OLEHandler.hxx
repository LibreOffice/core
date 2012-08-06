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
#ifndef INCLUDED_OLEHANDLER_HXX
#define INCLUDED_OLEHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>

#include <com/sun/star/drawing/XShape.hpp>

namespace com{ namespace sun{ namespace star{
    namespace graphic{
        class XGraphic;
    }
    namespace io{
        class XInputStream;
    }
    namespace text{
        class XTextDocument;
    }
}}}
namespace writerfilter {
namespace dmapper
{
/** Handler for OLE objects
 */
class WRITERFILTER_DLLPRIVATE OLEHandler : public LoggedProperties
{
    OUString     m_sObjectType;
    OUString     m_sProgId;
    OUString     m_sShapeId;
    OUString     m_sDrawAspect;
    OUString     m_sObjectId;
    OUString     m_sr_id;

    sal_Int32                   m_nDxaOrig;
    sal_Int32                   m_nDyaOrig;
    sal_Int32                   m_nWrapMode;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > m_xShape;

    ::com::sun::star::awt::Size m_aShapeSize;
    ::com::sun::star::awt::Point m_aShapePosition;

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > m_xReplacement;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xInputStream;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    OLEHandler();
    virtual ~OLEHandler();

    inline ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getShape( ) { return m_xShape; };

    inline bool isOLEObject( ) { return m_xInputStream.is( ); };

    OUString copyOLEOStream( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument > xTextDocument );

    ::com::sun::star::awt::Size     getSize() const { return m_aShapeSize;}
    ::com::sun::star::awt::Point    getPosition() const { return m_aShapePosition;}
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                                    getReplacement() const { return m_xReplacement; }

};
typedef boost::shared_ptr< OLEHandler >  OLEHandlerPtr;
}}

#endif //

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
