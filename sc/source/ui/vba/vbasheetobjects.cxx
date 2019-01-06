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

#include "vbasheetobjects.hxx"
#include <vector>
#include <rtl/math.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "vbasheetobject.hxx"
#include <cppuhelper/implbase.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

namespace {

template< typename Type >
bool lclGetProperty( Type& orValue, const uno::Reference< beans::XPropertySet >& rxPropSet, const OUString& rPropName )
{
    try
    {
        return rxPropSet->getPropertyValue( rPropName ) >>= orValue;
    }
    catch( uno::Exception& )
    {
    }
    return false;
}

/** Rounds the passed value to a multiple of 0.75 and converts it to 1/100 mm.

    @throws uno::RuntimeException
*/
double lclPointsToHmm( const uno::Any& rPoints )
{
    return PointsToHmm( ::rtl::math::approxFloor( rPoints.get< double >() / 0.75 ) * 0.75 );
}

} // namespace

// Base implementations

/** Container for a specific type of drawing object in a spreadsheet.

    Derived classes provide all required functionality specific to the type of
    shapes covered by the container.
 */
class ScVbaObjectContainer : public ::cppu::WeakImplHelper< container::XIndexAccess >
{
public:
    /// @throws uno::RuntimeException
    explicit ScVbaObjectContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        const uno::Type& rVbaType );

    /** Returns the VBA helper interface of the VBA collection object. */
    const uno::Reference< XHelperInterface >& getParent() const { return mxParent; }
    /** Returns the component context of the VBA collection object. */
    const uno::Reference< uno::XComponentContext >& getContext() const { return mxContext; }
    /** Returns the VBA type information of the objects in this container. */
    const uno::Type& getVbaType() const { return maVbaType; }

    /** Collects all shapes supported by this instance and inserts them into
        the internal shape vector.

        @throws uno::RuntimeException
    */
    void collectShapes();
    /** Creates and returns a new UNO shape.

        @throws uno::RuntimeException
    */
    uno::Reference< drawing::XShape > createShape( const awt::Point& rPos, const awt::Size& rSize );
    /** Inserts the passed shape into the draw page and into this container, and returns its index in the draw page.

        @throws uno::RuntimeException
    */
    sal_Int32 insertShape( const uno::Reference< drawing::XShape >& rxShape );
    /** Creates and returns a new VBA implementation object for the passed shape.

        @throws uno::RuntimeException
    */
    ::rtl::Reference< ScVbaSheetObjectBase > createVbaObject( const uno::Reference< drawing::XShape >& rxShape );
    /** Creates and returns a new VBA implementation object for the passed shape in an Any.

        @throws uno::RuntimeException
    */
    uno::Any createCollectionObject( const uno::Any& rSource );
    /** Returns the VBA implementation object with the specified name.

        @throws uno::RuntimeException
    */
    uno::Any getItemByStringIndex( const OUString& rIndex );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

protected:
    /** Derived classes return true, if the passed shape is supported by the instance. */
    virtual bool implPickShape( const uno::Reference< drawing::XShape >& rxShape ) const = 0;
    /** Derived classes create and return a new VBA implementation object for the passed shape.

        @throws uno::RuntimeException
    */
    virtual ScVbaSheetObjectBase* implCreateVbaObject( const uno::Reference< drawing::XShape >& rxShape ) = 0;
    /** Derived classes return the service name of the UNO shape. */
    virtual OUString implGetShapeServiceName() const = 0;

    /** Returns the shape name via 'Name' property of the UNO shape. May be overwritten.

        @throws uno::RuntimeException
    */
    virtual OUString implGetShapeName( const uno::Reference< drawing::XShape >& rxShape ) const;
    /** Is called when a new UNO shape has been created but not yet inserted into the drawing page.

        @throws uno::RuntimeException
    */
    virtual void implOnShapeCreated( const uno::Reference< drawing::XShape >& rxShape );

protected:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    uno::Reference< lang::XMultiServiceFactory > mxFactory;
    uno::Reference< drawing::XShapes > mxShapes;

private:
    typedef ::std::vector< uno::Reference< drawing::XShape > > ShapeVector;
    const uno::Type maVbaType;
    ShapeVector maShapes;
};

ScVbaObjectContainer::ScVbaObjectContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        const uno::Type& rVbaType ) :
    mxParent( rxParent ),
    mxContext( rxContext ),
    mxModel( rxModel, uno::UNO_SET_THROW ),
    mxFactory( rxModel, uno::UNO_QUERY_THROW ),
    maVbaType( rVbaType )
{
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupp( rxSheet, uno::UNO_QUERY_THROW );
    mxShapes.set( xDrawPageSupp->getDrawPage(), uno::UNO_QUERY_THROW );
}

void ScVbaObjectContainer::collectShapes()
{
    maShapes.clear();
    for( sal_Int32 nIndex = 0, nCount = mxShapes->getCount(); nIndex < nCount; ++nIndex )
    {
        uno::Reference< drawing::XShape > xShape( mxShapes->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
        if( implPickShape( xShape ) )
            maShapes.push_back( xShape );
    }
}

uno::Reference< drawing::XShape > ScVbaObjectContainer::createShape( const awt::Point& rPos, const awt::Size& rSize )
{
    uno::Reference< drawing::XShape > xShape( mxFactory->createInstance( implGetShapeServiceName() ), uno::UNO_QUERY_THROW );
    xShape->setPosition( rPos );
    xShape->setSize( rSize );
    implOnShapeCreated( xShape );
    return xShape;
}

sal_Int32 ScVbaObjectContainer::insertShape( const uno::Reference< drawing::XShape >& rxShape )
{
    mxShapes->add( rxShape );
    maShapes.push_back( rxShape );
    return mxShapes->getCount() - 1;
}

::rtl::Reference< ScVbaSheetObjectBase > ScVbaObjectContainer::createVbaObject(
    const uno::Reference< drawing::XShape >& rxShape )
{
    return implCreateVbaObject( rxShape );
}

uno::Any ScVbaObjectContainer::createCollectionObject( const uno::Any& rSource )
{
    uno::Reference< drawing::XShape > xShape( rSource, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XSheetObject > xSheetObject( implCreateVbaObject( xShape ) );
    return uno::Any( xSheetObject );
}

uno::Any ScVbaObjectContainer::getItemByStringIndex( const OUString& rIndex )
{
    auto aIt = std::find_if(maShapes.begin(), maShapes.end(),
        [&rIndex, this](const ShapeVector::value_type& rxShape) { return rIndex == implGetShapeName( rxShape ); });
    if (aIt != maShapes.end())
        return createCollectionObject( uno::Any( *aIt ) );
    throw uno::RuntimeException();
}

// XIndexAccess

sal_Int32 SAL_CALL ScVbaObjectContainer::getCount()
{
    return static_cast< sal_Int32 >( maShapes.size() );
}

uno::Any SAL_CALL ScVbaObjectContainer::getByIndex( sal_Int32 nIndex )
{
    if( (0 <= nIndex) && (nIndex < getCount()) )
        return uno::Any( maShapes[ static_cast< size_t >( nIndex ) ] );
    throw lang::IndexOutOfBoundsException();
}

// XElementAccess

uno::Type SAL_CALL ScVbaObjectContainer::getElementType()
{
    return cppu::UnoType<drawing::XShape>::get();
}

sal_Bool SAL_CALL ScVbaObjectContainer::hasElements()
{
    return !maShapes.empty();
}

// private

OUString ScVbaObjectContainer::implGetShapeName( const uno::Reference< drawing::XShape >& rxShape ) const
{
    uno::Reference< beans::XPropertySet > xPropSet( rxShape, uno::UNO_QUERY_THROW );
    return xPropSet->getPropertyValue( "Name" ).get< OUString >();
}

void ScVbaObjectContainer::implOnShapeCreated( const uno::Reference< drawing::XShape >& /*rxShape*/ )
{
}

class ScVbaObjectEnumeration : public SimpleEnumerationBase
{
public:
    explicit ScVbaObjectEnumeration( const ScVbaObjectContainerRef& rxContainer );
    virtual uno::Any createCollectionObject( const uno::Any& rSource ) override;

private:
    ScVbaObjectContainerRef mxContainer;
};

ScVbaObjectEnumeration::ScVbaObjectEnumeration( const ScVbaObjectContainerRef& rxContainer ) :
    SimpleEnumerationBase( rxContainer.get() ),
    mxContainer( rxContainer )
{
}

uno::Any ScVbaObjectEnumeration::createCollectionObject( const uno::Any& rSource )
{
    return mxContainer->createCollectionObject( rSource );
}

ScVbaSheetObjectsBase::ScVbaSheetObjectsBase( const ScVbaObjectContainerRef& rxContainer ) :
    ScVbaSheetObjects_BASE( rxContainer->getParent(), rxContainer->getContext(), rxContainer.get() ),
    mxContainer( rxContainer )
{
    mxContainer->collectShapes();
}

ScVbaSheetObjectsBase::~ScVbaSheetObjectsBase()
{
}

void ScVbaSheetObjectsBase::collectShapes()
{
    mxContainer->collectShapes();
}

// XEnumerationAccess

uno::Reference< container::XEnumeration > SAL_CALL ScVbaSheetObjectsBase::createEnumeration()
{
    return new ScVbaObjectEnumeration( mxContainer );
}

// XElementAccess

uno::Type SAL_CALL ScVbaSheetObjectsBase::getElementType()
{
    return mxContainer->getVbaType();
}

// ScVbaCollectionBase

uno::Any ScVbaSheetObjectsBase::createCollectionObject( const uno::Any& rSource )
{
    return mxContainer->createCollectionObject( rSource );
}

uno::Any ScVbaSheetObjectsBase::getItemByStringIndex( const OUString& rIndex )
{
    return mxContainer->getItemByStringIndex( rIndex );
}

// Graphic object containers supporting ooo.vba.excel.XGraphicObject

ScVbaGraphicObjectsBase::ScVbaGraphicObjectsBase( const ScVbaObjectContainerRef& rxContainer ) :
    ScVbaGraphicObjects_BASE( rxContainer )
{
}

// XGraphicObjects

uno::Any SAL_CALL ScVbaGraphicObjectsBase::Add( const uno::Any& rLeft, const uno::Any& rTop, const uno::Any& rWidth, const uno::Any& rHeight )
{
    /*  Extract double values from passed Anys (the lclPointsToHmm() helper
        function will throw a RuntimeException on any error), and convert from
        points to 1/100 mm. */
    awt::Point aPos( static_cast<sal_Int32>(lclPointsToHmm( rLeft )),  static_cast<sal_Int32>(lclPointsToHmm( rTop )) );
    awt::Size aSize( static_cast<sal_Int32>(lclPointsToHmm( rWidth )), static_cast<sal_Int32>(lclPointsToHmm( rHeight )) );
    // TODO: translate coordinates for RTL sheets
    if( (aPos.X < 0) || (aPos.Y < 0) || (aSize.Width <= 0) || (aSize.Height <= 0) )
        throw uno::RuntimeException();

    // create the UNO shape
    uno::Reference< drawing::XShape > xShape( mxContainer->createShape( aPos, aSize ), uno::UNO_SET_THROW );
    sal_Int32 nIndex = mxContainer->insertShape( xShape );

    // create and return the VBA object
    ::rtl::Reference< ScVbaSheetObjectBase > xVbaObject = mxContainer->createVbaObject( xShape );
    xVbaObject->setDefaultProperties( nIndex );
    return uno::Any( uno::Reference< excel::XSheetObject >( xVbaObject.get() ) );
}

// Drawing controls

class ScVbaControlContainer : public ScVbaObjectContainer
{
public:
    /// @throws uno::RuntimeException
    explicit ScVbaControlContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        const uno::Type& rVbaType,
        const OUString& rModelServiceName,
        sal_Int16 /* css::form::FormComponentType */ eType );

protected:
    /// @throws uno::RuntimeException
    uno::Reference< container::XIndexContainer > const & createForm();

    virtual bool implPickShape( const uno::Reference< drawing::XShape >& rxShape ) const override;
    virtual OUString implGetShapeServiceName() const override;
    virtual bool implCheckProperties( const uno::Reference< beans::XPropertySet >& rxModelProps ) const;
    virtual OUString implGetShapeName( const uno::Reference< drawing::XShape >& rxShape ) const override;
    virtual void implOnShapeCreated( const uno::Reference< drawing::XShape >& rxShape ) override;

protected:
    uno::Reference< container::XIndexContainer > mxFormIC;
    OUString const maModelServiceName;
    sal_Int16 /* css::form::FormComponentType */ meType;
};

ScVbaControlContainer::ScVbaControlContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        const uno::Type& rVbaType,
        const OUString& rModelServiceName,
        sal_Int16 /* css::form::FormComponentType */ eType ) :
    ScVbaObjectContainer( rxParent, rxContext, rxModel, rxSheet, rVbaType ),
    maModelServiceName( rModelServiceName ),
    meType( eType )
{
}

uno::Reference< container::XIndexContainer > const & ScVbaControlContainer::createForm()
{
    if( !mxFormIC.is() )
    {
        uno::Reference< form::XFormsSupplier > xFormsSupp( mxShapes, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameContainer > xFormsNC( xFormsSupp->getForms(), uno::UNO_SET_THROW );
        OUString aFormName = "Standard";
        if( xFormsNC->hasByName( aFormName ) )
        {
            mxFormIC.set( xFormsNC->getByName( aFormName ), uno::UNO_QUERY_THROW );
        }
        else
        {
            uno::Reference< form::XForm > xForm( mxFactory->createInstance( "com.sun.star.form.component.Form" ), uno::UNO_QUERY_THROW );
            xFormsNC->insertByName( aFormName, uno::Any( xForm ) );
            mxFormIC.set( xForm, uno::UNO_QUERY_THROW );
        }
    }
    return mxFormIC;
}

bool ScVbaControlContainer::implPickShape( const uno::Reference< drawing::XShape >& rxShape ) const
{
    try
    {
        uno::Reference< drawing::XControlShape > xControlShape( rxShape, uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xModelProps( xControlShape->getControl(), uno::UNO_QUERY_THROW );
        sal_Int16 nClassId = -1;
        return lclGetProperty( nClassId, xModelProps, "ClassId" ) &&
            (nClassId == meType) && implCheckProperties( xModelProps );
    }
    catch( uno::Exception& )
    {
    }
    return false;
}

OUString ScVbaControlContainer::implGetShapeServiceName() const
{
    return OUString( "com.sun.star.drawing.ControlShape" );
}

bool ScVbaControlContainer::implCheckProperties( const uno::Reference< beans::XPropertySet >& /*rxModelProps*/ ) const
{
    return true;
}

OUString ScVbaControlContainer::implGetShapeName( const uno::Reference< drawing::XShape >& rxShape ) const
{
    uno::Reference< drawing::XControlShape > xControlShape( rxShape, uno::UNO_QUERY_THROW );
    return uno::Reference< container::XNamed >( xControlShape->getControl(), uno::UNO_QUERY_THROW )->getName();
}

void ScVbaControlContainer::implOnShapeCreated( const uno::Reference< drawing::XShape >& rxShape )
{
    // passed shape must be a control shape
    uno::Reference< drawing::XControlShape > xControlShape( rxShape, uno::UNO_QUERY_THROW );

    // create the UNO control model
    uno::Reference< form::XFormComponent > xFormComponent( mxFactory->createInstance( maModelServiceName ), uno::UNO_QUERY_THROW );
    uno::Reference< awt::XControlModel > xControlModel( xFormComponent, uno::UNO_QUERY_THROW );

    // insert the control model into the form and the shape
    createForm();
    mxFormIC->insertByIndex( mxFormIC->getCount(), uno::Any( xFormComponent ) );
    xControlShape->setControl( xControlModel );
}

// Push button

class ScVbaButtonContainer : public ScVbaControlContainer
{
    bool mbOptionButtons;
public:
    /// @throws uno::RuntimeException
    explicit ScVbaButtonContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        bool bOptionButtons);

protected:
    virtual ScVbaSheetObjectBase* implCreateVbaObject( const uno::Reference< drawing::XShape >& rxShape ) override;
    virtual bool implCheckProperties( const uno::Reference< beans::XPropertySet >& rxModelProps ) const override;
};

ScVbaButtonContainer::ScVbaButtonContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        bool bOptionButtons ) :
    ScVbaControlContainer(
        rxParent, rxContext, rxModel, rxSheet,
        cppu::UnoType<excel::XButton>::get(),
        ( bOptionButtons ?
          OUString( "com.sun.star.form.component.RadioButton" ) :
          OUString( "com.sun.star.form.component.CommandButton" ) ),
        ( bOptionButtons ?
          form::FormComponentType::RADIOBUTTON :
          form::FormComponentType::COMMANDBUTTON) ),
    mbOptionButtons(bOptionButtons)
{
}

ScVbaSheetObjectBase* ScVbaButtonContainer::implCreateVbaObject( const uno::Reference< drawing::XShape >& rxShape )
{
    uno::Reference< drawing::XControlShape > xControlShape( rxShape, uno::UNO_QUERY_THROW );
    return new ScVbaButton( mxParent, mxContext, mxModel, createForm(), xControlShape );
}

bool ScVbaButtonContainer::implCheckProperties( const uno::Reference< beans::XPropertySet >& rxModelProps ) const
{
    if (mbOptionButtons)
        return true;

    // do not insert toggle buttons into the 'Buttons' collection
    bool bToggle = false;
    return lclGetProperty( bToggle, rxModelProps, "Toggle" ) && !bToggle;
}

ScVbaButtons::ScVbaButtons(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        bool bOptionButtons) :
    ScVbaGraphicObjectsBase( new ScVbaButtonContainer( rxParent, rxContext, rxModel, rxSheet, bOptionButtons ) )
{
}

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaButtons, "ooo.vba.excel.Buttons" )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
