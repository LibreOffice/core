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
#include <oox/helper/helper.hxx>
#include "vbasheetobject.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::ooo::vba;

// ============================================================================

namespace {

template< typename Type >
inline bool lclGetProperty( Type& orValue, const uno::Reference< beans::XPropertySet >& rxPropSet, const OUString& rPropName )
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

/** Rounds the passed value to a multiple of 0.75 and converts it to 1/100 mm. */
inline double lclPointsToHmm( const uno::Any& rPoints ) throw (uno::RuntimeException)
{
    return PointsToHmm( ::rtl::math::approxFloor( rPoints.get< double >() / 0.75 ) * 0.75 );
}

} // namespace

// ============================================================================
// Base implementations
// ============================================================================

/** Container for a specific type of drawing object in a spreadsheet.

    Derived classes provide all required functionality specific to the type of
    shapes covered by the container.
 */
class ScVbaObjectContainer : public ::cppu::WeakImplHelper1< container::XIndexAccess >
{
public:
    explicit ScVbaObjectContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        const uno::Type& rVbaType ) throw (uno::RuntimeException);

    /** Returns the VBA helper interface of the VBA collection object. */
    inline const uno::Reference< XHelperInterface >& getParent() const { return mxParent; }
    /** Returns the component context of the VBA collection object. */
    inline const uno::Reference< uno::XComponentContext >& getContext() const { return mxContext; }
    /** Returns the VBA type information of the objects in this container. */
    inline const uno::Type& getVbaType() const { return maVbaType; }

    /** Collects all shapes supported by this instance and inserts them into
        the internal shape vector. */
    void collectShapes() throw (uno::RuntimeException);
    /** Creates and returns a new UNO shape. */
    uno::Reference< drawing::XShape > createShape( const awt::Point& rPos, const awt::Size& rSize ) throw (uno::RuntimeException);
    /** Inserts the passed shape into the draw page and into this container, and returns its index in the draw page. */
    sal_Int32 insertShape( const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException);
    /** Creates and returns a new VBA implementation object for the passed shape. */
    ::rtl::Reference< ScVbaSheetObjectBase > createVbaObject( const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException);
    /** Creates and returns a new VBA implementation object for the passed shape in an Any. */
    uno::Any createCollectionObject( const uno::Any& rSource ) throw (uno::RuntimeException);
    /** Returns the VBA implementation object with the specified name. */
    uno::Any getItemByStringIndex( const OUString& rIndex ) throw (uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException);
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException);

protected:
    /** Derived classes return true, if the passed shape is supported by the instance. */
    virtual bool implPickShape( const uno::Reference< drawing::XShape >& rxShape ) const = 0;
    /** Derived classes create and return a new VBA implementation object for the passed shape. */
    virtual ScVbaSheetObjectBase* implCreateVbaObject( const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException) = 0;
    /** Derived classes return the service name of the UNO shape. */
    virtual OUString implGetShapeServiceName() const = 0;

    /** Returns the shape name via 'Name' property of the UNO shape. May be overwritten. */
    virtual OUString implGetShapeName( const uno::Reference< drawing::XShape >& rxShape ) const throw (uno::RuntimeException);
    /** Is called when a new UNO shape has been created but not yet inserted into the drawing page. */
    virtual void implOnShapeCreated( const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException);

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

// ----------------------------------------------------------------------------

ScVbaObjectContainer::ScVbaObjectContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        const uno::Type& rVbaType ) throw (uno::RuntimeException) :
    mxParent( rxParent ),
    mxContext( rxContext ),
    mxModel( rxModel, uno::UNO_SET_THROW ),
    mxFactory( rxModel, uno::UNO_QUERY_THROW ),
    maVbaType( rVbaType )
{
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupp( rxSheet, uno::UNO_QUERY_THROW );
    mxShapes.set( xDrawPageSupp->getDrawPage(), uno::UNO_QUERY_THROW );
}

void ScVbaObjectContainer::collectShapes() throw (uno::RuntimeException)
{
    maShapes.clear();
    for( sal_Int32 nIndex = 0, nCount = mxShapes->getCount(); nIndex < nCount; ++nIndex )
    {
        uno::Reference< drawing::XShape > xShape( mxShapes->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
        if( implPickShape( xShape ) )
            maShapes.push_back( xShape );
    }
}

uno::Reference< drawing::XShape > ScVbaObjectContainer::createShape( const awt::Point& rPos, const awt::Size& rSize ) throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( mxFactory->createInstance( implGetShapeServiceName() ), uno::UNO_QUERY_THROW );
    xShape->setPosition( rPos );
    xShape->setSize( rSize );
    implOnShapeCreated( xShape );
    return xShape;
}

sal_Int32 ScVbaObjectContainer::insertShape( const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException)
{
    mxShapes->add( rxShape );
    maShapes.push_back( rxShape );
    return mxShapes->getCount() - 1;
}

::rtl::Reference< ScVbaSheetObjectBase > ScVbaObjectContainer::createVbaObject(
    const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException)
{
    return implCreateVbaObject( rxShape );
}

uno::Any ScVbaObjectContainer::createCollectionObject( const uno::Any& rSource ) throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( rSource, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XSheetObject > xSheetObject( implCreateVbaObject( xShape ) );
    return uno::Any( xSheetObject );
}

uno::Any ScVbaObjectContainer::getItemByStringIndex( const OUString& rIndex ) throw (uno::RuntimeException)
{
    for( ShapeVector::iterator aIt = maShapes.begin(), aEnd = maShapes.end(); aIt != aEnd; ++aIt )
        if( rIndex == implGetShapeName( *aIt ) )
            return createCollectionObject( uno::Any( *aIt ) );
    throw uno::RuntimeException();
}

// XIndexAccess

sal_Int32 SAL_CALL ScVbaObjectContainer::getCount() throw (uno::RuntimeException)
{
    return static_cast< sal_Int32 >( maShapes.size() );
}

uno::Any SAL_CALL ScVbaObjectContainer::getByIndex( sal_Int32 nIndex )
        throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( (0 <= nIndex) && (nIndex < getCount()) )
        return uno::Any( maShapes[ static_cast< size_t >( nIndex ) ] );
    throw lang::IndexOutOfBoundsException();
}

// XElementAccess

uno::Type SAL_CALL ScVbaObjectContainer::getElementType() throw (uno::RuntimeException)
{
    return drawing::XShape::static_type( 0 );
}

sal_Bool SAL_CALL ScVbaObjectContainer::hasElements() throw (uno::RuntimeException)
{
    return !maShapes.empty();
}

// private

OUString ScVbaObjectContainer::implGetShapeName( const uno::Reference< drawing::XShape >& rxShape ) const throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xPropSet( rxShape, uno::UNO_QUERY_THROW );
    return xPropSet->getPropertyValue( "Name" ).get< OUString >();
}

void ScVbaObjectContainer::implOnShapeCreated( const uno::Reference< drawing::XShape >& /*rxShape*/ ) throw (uno::RuntimeException)
{
}

// ============================================================================

class ScVbaObjectEnumeration : public SimpleEnumerationBase
{
public:
    explicit ScVbaObjectEnumeration( const ScVbaObjectContainerRef& rxContainer );
    virtual uno::Any createCollectionObject( const uno::Any& rSource );

private:
    ScVbaObjectContainerRef mxContainer;
};

// ----------------------------------------------------------------------------

ScVbaObjectEnumeration::ScVbaObjectEnumeration( const ScVbaObjectContainerRef& rxContainer ) :
    SimpleEnumerationBase( rxContainer->getParent(), rxContainer->getContext(), rxContainer.get() ),
    mxContainer( rxContainer )
{
}

uno::Any ScVbaObjectEnumeration::createCollectionObject( const uno::Any& rSource )
{
    return mxContainer->createCollectionObject( rSource );
}

// ============================================================================

ScVbaSheetObjectsBase::ScVbaSheetObjectsBase( const ScVbaObjectContainerRef& rxContainer ) throw (css::uno::RuntimeException) :
    ScVbaSheetObjects_BASE( rxContainer->getParent(), rxContainer->getContext(), rxContainer.get() ),
    mxContainer( rxContainer )
{
    mxContainer->collectShapes();
}

ScVbaSheetObjectsBase::~ScVbaSheetObjectsBase()
{
}

void ScVbaSheetObjectsBase::collectShapes() throw (uno::RuntimeException)
{
    mxContainer->collectShapes();
}

// XEnumerationAccess

uno::Reference< container::XEnumeration > SAL_CALL ScVbaSheetObjectsBase::createEnumeration() throw (uno::RuntimeException)
{
    return new ScVbaObjectEnumeration( mxContainer );
}

// XElementAccess

uno::Type SAL_CALL ScVbaSheetObjectsBase::getElementType() throw (uno::RuntimeException)
{
    return mxContainer->getVbaType();
}

// ScVbaCollectionBase

uno::Any ScVbaSheetObjectsBase::createCollectionObject( const uno::Any& rSource )
{
    return mxContainer->createCollectionObject( rSource );
}

uno::Any ScVbaSheetObjectsBase::getItemByStringIndex( const OUString& rIndex ) throw (uno::RuntimeException)
{
    return mxContainer->getItemByStringIndex( rIndex );
}

// ============================================================================
// Graphic object containers supporting ooo.vba.excel.XGraphicObject
// ============================================================================

ScVbaGraphicObjectsBase::ScVbaGraphicObjectsBase( const ScVbaObjectContainerRef& rxContainer ) throw (uno::RuntimeException) :
    ScVbaGraphicObjects_BASE( rxContainer )
{
}

// XGraphicObjects

uno::Any SAL_CALL ScVbaGraphicObjectsBase::Add( const uno::Any& rLeft, const uno::Any& rTop, const uno::Any& rWidth, const uno::Any& rHeight ) throw (uno::RuntimeException)
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

// ============================================================================
// Drawing controls
// ============================================================================

class ScVbaControlContainer : public ScVbaObjectContainer
{
public:
    explicit ScVbaControlContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        const uno::Type& rVbaType,
        const OUString& rModelServiceName,
        sal_Int16 nComponentType ) throw (uno::RuntimeException);

protected:
    uno::Reference< container::XIndexContainer > createForm() throw (uno::RuntimeException);

    virtual bool implPickShape( const uno::Reference< drawing::XShape >& rxShape ) const;
    virtual OUString implGetShapeServiceName() const;
    virtual bool implCheckProperties( const uno::Reference< beans::XPropertySet >& rxModelProps ) const;
    virtual OUString implGetShapeName( const uno::Reference< drawing::XShape >& rxShape ) const throw (uno::RuntimeException);
    virtual void implOnShapeCreated( const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException);

protected:
    uno::Reference< container::XIndexContainer > mxFormIC;
    OUString maModelServiceName;
    sal_Int16 mnComponentType;
};

// ----------------------------------------------------------------------------

ScVbaControlContainer::ScVbaControlContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet,
        const uno::Type& rVbaType,
        const OUString& rModelServiceName,
        sal_Int16 nComponentType ) throw (uno::RuntimeException) :
    ScVbaObjectContainer( rxParent, rxContext, rxModel, rxSheet, rVbaType ),
    maModelServiceName( rModelServiceName ),
    mnComponentType( nComponentType )
{
}

uno::Reference< container::XIndexContainer > ScVbaControlContainer::createForm() throw (uno::RuntimeException)
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
            (nClassId == mnComponentType) && implCheckProperties( xModelProps );
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

OUString ScVbaControlContainer::implGetShapeName( const uno::Reference< drawing::XShape >& rxShape ) const throw (uno::RuntimeException)
{
    uno::Reference< drawing::XControlShape > xControlShape( rxShape, uno::UNO_QUERY_THROW );
    return uno::Reference< container::XNamed >( xControlShape->getControl(), uno::UNO_QUERY_THROW )->getName();
}

void ScVbaControlContainer::implOnShapeCreated( const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException)
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

// ============================================================================
// Push button
// ============================================================================

class ScVbaButtonContainer : public ScVbaControlContainer
{
public:
    explicit ScVbaButtonContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet ) throw (uno::RuntimeException);

protected:
    virtual ScVbaSheetObjectBase* implCreateVbaObject( const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException);
    virtual bool implCheckProperties( const uno::Reference< beans::XPropertySet >& rxModelProps ) const;
};

// ----------------------------------------------------------------------------

ScVbaButtonContainer::ScVbaButtonContainer(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet ) throw (uno::RuntimeException) :
    ScVbaControlContainer(
        rxParent, rxContext, rxModel, rxSheet,
        excel::XButton::static_type( 0 ),
        "com.sun.star.form.component.CommandButton",
        form::FormComponentType::COMMANDBUTTON )
{
}

ScVbaSheetObjectBase* ScVbaButtonContainer::implCreateVbaObject( const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException)
{
    uno::Reference< drawing::XControlShape > xControlShape( rxShape, uno::UNO_QUERY_THROW );
    return new ScVbaButton( mxParent, mxContext, mxModel, createForm(), xControlShape );
}

bool ScVbaButtonContainer::implCheckProperties( const uno::Reference< beans::XPropertySet >& rxModelProps ) const
{
    // do not insert toggle buttons into the 'Buttons' collection
    bool bToggle = false;
    return lclGetProperty( bToggle, rxModelProps, "Toggle" ) && !bToggle;
}

// ============================================================================

ScVbaButtons::ScVbaButtons(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< sheet::XSpreadsheet >& rxSheet ) throw (uno::RuntimeException) :
    ScVbaGraphicObjectsBase( new ScVbaButtonContainer( rxParent, rxContext, rxModel, rxSheet ) )
{
}

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaButtons, "ooo.vba.excel.Buttons" )

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
