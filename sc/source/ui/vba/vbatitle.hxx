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
#ifndef SC_VBA_TITLE_HXX
#define SC_VBA_TITLE_HXX

#include <vbahelper/vbahelperinterface.hxx>
#include "excelvbahelper.hxx"
#include "vbainterior.hxx"
#include "vbafont.hxx"
#include "vbapalette.hxx"
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XTitle.hpp>
#include <ooo/vba/excel/XCharacters.hpp>
#include <basic/sberrors.hxx>
#include <memory>

template< typename Ifc1 >
class TitleImpl : public InheritedHelperInterfaceImpl< Ifc1 >
{
typedef InheritedHelperInterfaceImpl< Ifc1 > BaseClass;

protected:
    css::uno::Reference< css::drawing::XShape > xTitleShape;
    css::uno::Reference< css::beans::XPropertySet > xShapePropertySet;
    std::auto_ptr<ov::ShapeHelper> oShapeHelper;
    ScVbaPalette m_Palette;
public:
    TitleImpl(  const css::uno::Reference< ov::XHelperInterface >& xParent,   const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& _xTitleShape ) : BaseClass( xParent, xContext ), xTitleShape( _xTitleShape )
    {
        xShapePropertySet.set( xTitleShape, css::uno::UNO_QUERY_THROW );
        oShapeHelper.reset( new ov::ShapeHelper(xTitleShape) );
    }
    css::uno::Reference< ov::excel::XInterior > SAL_CALL Interior(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        // #TODO find out what the proper parent should be
        // leaving as set by the helperapi for the moment
        // #TODO we really need the ScDocument to pass to ScVbaInterior
        // otherwise attemps to access the palette will fail
        return new ScVbaInterior( BaseClass::mxParent, BaseClass::mxContext, xShapePropertySet );
    }
    css::uno::Reference< ov::excel::XFont > SAL_CALL Font(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        // #TODO find out what the proper parent should be
        // leaving as set by the helperapi for the moment
        return new ScVbaFont( BaseClass::mxParent, BaseClass::mxContext, m_Palette, xShapePropertySet );

    }
    void SAL_CALL setText( const ::rtl::OUString& Text ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        try
        {
            xShapePropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("String") ), css::uno::makeAny( Text ));
        }
        catch ( css::uno::Exception& )
        {
            throw css::script::BasicErrorException( rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
        }
    }
    ::rtl::OUString SAL_CALL getText(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        ::rtl::OUString sText;
        try
        {
            xShapePropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("String") ) ) >>= sText;
        }
        catch ( css::uno::Exception& )
        {
            throw css::script::BasicErrorException( rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
        }
        return sText;
    }

    css::uno::Reference< ov::excel::XCharacters > SAL_CALL Characters(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        // #FIXME #TODO the helperapi Characters implementation doesn't
        // seem to do very much, need to know how the existing Characters
        // impl ( that we use for Range ) can be reused
        return  css::uno::Reference< ov::excel::XCharacters > ();
    }

    void SAL_CALL setTop( double Top ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        oShapeHelper->setTop( Top );
    }
    double SAL_CALL getTop(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        return oShapeHelper->getTop();
    }
    void SAL_CALL setLeft( double Left ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        oShapeHelper->setLeft( Left );
    }
    double SAL_CALL getLeft(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        return oShapeHelper->getLeft();
    }
    void SAL_CALL setOrientation( ::sal_Int32 _nOrientation ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        try
        {
            xShapePropertySet->setPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextRotation")), css::uno::makeAny(_nOrientation*100));
        }
        catch (css::uno::Exception& )
        {
            throw css::script::BasicErrorException( rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
        }
    }
    ::sal_Int32 SAL_CALL getOrientation(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        sal_Int32 nSOOrientation = 0;
        try
        {
            xShapePropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextRotation"))) >>= nSOOrientation;
        }
        catch (css::uno::Exception& )
        {
            throw css::script::BasicErrorException( rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
        }
        return static_cast< sal_Int32 >(nSOOrientation / 100) ;
    }
// XHelperInterface
    rtl::OUString& getServiceImplName()
    {
        static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("TitleImpl") );
        return sImplName;
    }
    css::uno::Sequence< rtl::OUString > getServiceNames()
    {
        static css::uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
            aServiceNames.realloc( 1 );
            aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.XTitle" ) );
        }
        return aServiceNames;
    }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
