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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBATITLE_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBATITLE_HXX

#include <vbahelper/vbahelperinterface.hxx>
#include "vbainterior.hxx"
#include "vbafont.hxx"
#include "vbapalette.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <basic/sberrors.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }

template< typename... Ifc >
class TitleImpl : public InheritedHelperInterfaceImpl< Ifc... >
{
typedef InheritedHelperInterfaceImpl< Ifc... > BaseClass;

    css::uno::Reference< css::drawing::XShape > xTitleShape;
    css::uno::Reference< css::beans::XPropertySet > xShapePropertySet;
    std::unique_ptr<ov::ShapeHelper> oShapeHelper;
    ScVbaPalette const m_Palette;
public:
    TitleImpl(  const css::uno::Reference< ov::XHelperInterface >& xParent,
                const css::uno::Reference< css::uno::XComponentContext >& xContext,
                const css::uno::Reference< css::drawing::XShape >& _xTitleShape )
        : BaseClass( xParent, xContext ), xTitleShape( _xTitleShape ), m_Palette(nullptr)
    {
        xShapePropertySet.set( xTitleShape, css::uno::UNO_QUERY_THROW );
        oShapeHelper.reset( new ov::ShapeHelper(xTitleShape) );
    }
    css::uno::Reference< ov::excel::XInterior > SAL_CALL Interior(  ) override
    {
        // #TODO find out what the proper parent should be
        // leaving as set by the helperapi for the moment
        // #TODO we really need the ScDocument to pass to ScVbaInterior
        // otherwise attempts to access the palette will fail
        return new ScVbaInterior( BaseClass::mxParent, BaseClass::mxContext, xShapePropertySet );
    }
    css::uno::Reference< ov::excel::XFont > SAL_CALL Font(  ) override
    {
        // #TODO find out what the proper parent should be
        // leaving as set by the helperapi for the moment
        return new ScVbaFont( BaseClass::mxParent, BaseClass::mxContext, m_Palette, xShapePropertySet );

    }
    void SAL_CALL setText( const OUString& Text ) override
    {
        try
        {
            xShapePropertySet->setPropertyValue("String", css::uno::makeAny( Text ));
        }
        catch ( css::uno::Exception& )
        {
            throw css::script::BasicErrorException( OUString(), css::uno::Reference< css::uno::XInterface >(), sal_uInt32(ERRCODE_BASIC_METHOD_FAILED), OUString() );
        }
    }
    OUString SAL_CALL getText(  ) override
    {
        OUString sText;
        try
        {
            xShapePropertySet->getPropertyValue("String") >>= sText;
        }
        catch ( css::uno::Exception& )
        {
            throw css::script::BasicErrorException( OUString(), css::uno::Reference< css::uno::XInterface >(), sal_uInt32(ERRCODE_BASIC_METHOD_FAILED), OUString() );
        }
        return sText;
    }

    void SAL_CALL setTop( double Top ) override
    {
        oShapeHelper->setTop( Top );
    }
    double SAL_CALL getTop(  ) override
    {
        return oShapeHelper->getTop();
    }
    void SAL_CALL setLeft( double Left ) override
    {
        oShapeHelper->setLeft( Left );
    }
    double SAL_CALL getLeft(  ) override
    {
        return oShapeHelper->getLeft();
    }
    void SAL_CALL setOrientation( ::sal_Int32 _nOrientation ) override
    {
        try
        {
            xShapePropertySet->setPropertyValue("TextRotation", css::uno::makeAny(_nOrientation*100));
        }
        catch (css::uno::Exception& )
        {
            throw css::script::BasicErrorException( OUString(), css::uno::Reference< css::uno::XInterface >(), sal_uInt32(ERRCODE_BASIC_METHOD_FAILED), OUString() );
        }
    }
    ::sal_Int32 SAL_CALL getOrientation(  ) override
    {
        sal_Int32 nSOOrientation = 0;
        try
        {
            xShapePropertySet->getPropertyValue("TextRotation") >>= nSOOrientation;
        }
        catch (css::uno::Exception& )
        {
            throw css::script::BasicErrorException( OUString(), css::uno::Reference< css::uno::XInterface >(), sal_uInt32(ERRCODE_BASIC_METHOD_FAILED), OUString() );
        }
        return static_cast< sal_Int32 >(nSOOrientation / 100) ;
    }
// XHelperInterface
    OUString getServiceImplName() override
    {
        return OUString("TitleImpl");
    }
    css::uno::Sequence< OUString > getServiceNames() override
    {
        static css::uno::Sequence< OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
            aServiceNames.realloc( 1 );
            aServiceNames[ 0 ] = "ooo.vba.excel.XTitle";
        }
        return aServiceNames;
    }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
