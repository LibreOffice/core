/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <utility>
#include <vbahelper/vbahelperinterface.hxx>
#include "vbainterior.hxx"
#include "vbafont.hxx"
#include "vbapalette.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <basic/sberrors.hxx>
#include <memory>

template< typename... Ifc >
class TitleImpl : public InheritedHelperInterfaceImpl< Ifc... >
{
typedef InheritedHelperInterfaceImpl< Ifc... > BaseClass;

    cpo::uno::Reference< css::drawing::XShape > xTitleShape;
    cpo::uno::Reference< css::beans::XPropertySet > xShapePropertySet;
    ov::ShapeHelper maShapeHelper;
    ScVbaPalette m_Palette;
public:
    TitleImpl(  const cpo::uno::Reference< ov::XHelperInterface >& xParent,
                const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
                cpo::uno::Reference< css::drawing::XShape >  _xTitleShape )
        : BaseClass( xParent, xContext ),
        xTitleShape(std::move( _xTitleShape )),
        xShapePropertySet( xTitleShape, cpo::uno::UNO_QUERY_THROW ),
        maShapeHelper( xTitleShape ),
        m_Palette(nullptr)
    {
    }
    cpo::uno::Reference< ov::excel::XInterior > Interior(  ) override
    {
        // #TODO find out what the proper parent should be
        // leaving as set by the helperapi for the moment
        // #TODO we really need the ScDocument to pass to ScVbaInterior
        // otherwise attempts to access the palette will fail
        return new ScVbaInterior( BaseClass::mxParent, BaseClass::mxContext, xShapePropertySet );
    }
    cpo::uno::Reference< ov::excel::XFont > Font(  ) override
    {
        // #TODO find out what the proper parent should be
        // leaving as set by the helperapi for the moment
        return new ScVbaFont( BaseClass::mxParent, BaseClass::mxContext, m_Palette, xShapePropertySet );

    }
    void setText( const OUString& Text ) override
    {
        try
        {
            xShapePropertySet->setPropertyValue(u"String"_ustr, cpo::uno::Any( Text ));
        }
        catch ( cpo::uno::Exception& )
        {
            throw css::script::BasicErrorException( OUString(), cpo::uno::Reference< cpo::uno::XInterface >(), sal_uInt32(ERRCODE_BASIC_METHOD_FAILED), OUString() );
        }
    }
    OUString getText(  ) override
    {
        OUString sText;
        try
        {
            xShapePropertySet->getPropertyValue(u"String"_ustr) >>= sText;
        }
        catch ( cpo::uno::Exception& )
        {
            throw css::script::BasicErrorException( OUString(), cpo::uno::Reference< cpo::uno::XInterface >(), sal_uInt32(ERRCODE_BASIC_METHOD_FAILED), OUString() );
        }
        return sText;
    }

    void setTop( double Top ) override
    {
        maShapeHelper.setTop( Top );
    }
    double getTop(  ) override
    {
        return maShapeHelper.getTop();
    }
    void setLeft( double Left ) override
    {
        maShapeHelper.setLeft( Left );
    }
    double getLeft(  ) override
    {
        return maShapeHelper.getLeft();
    }
    void setOrientation( ::sal_Int32 _nOrientation ) override
    {
        try
        {
            xShapePropertySet->setPropertyValue(u"TextRotation"_ustr, cpo::uno::Any(_nOrientation*100));
        }
        catch (cpo::uno::Exception& )
        {
            throw css::script::BasicErrorException( OUString(), cpo::uno::Reference< cpo::uno::XInterface >(), sal_uInt32(ERRCODE_BASIC_METHOD_FAILED), OUString() );
        }
    }
    ::sal_Int32 getOrientation(  ) override
    {
        sal_Int32 nSOOrientation = 0;
        try
        {
            xShapePropertySet->getPropertyValue(u"TextRotation"_ustr) >>= nSOOrientation;
        }
        catch (cpo::uno::Exception& )
        {
            throw css::script::BasicErrorException( OUString(), cpo::uno::Reference< cpo::uno::XInterface >(), sal_uInt32(ERRCODE_BASIC_METHOD_FAILED), OUString() );
        }
        return static_cast< sal_Int32 >(nSOOrientation / 100) ;
    }
// XHelperInterface
    OUString getServiceImplName() override
    {
        return u"TitleImpl"_ustr;
    }
    cpo::uno::Sequence< OUString > getServiceNames() override
    {
        static const cpo::uno::Sequence< OUString > aServiceNames{ u"ooo.vba.excel.XTitle"_ustr };
        return aServiceNames;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
