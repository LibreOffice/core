/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "DummyXShape.hxx"
#include <rtl/ustring.hxx>

#include <algorithm>

using namespace com::sun::star;

namespace chart {

namespace dummy {

DummyXShape::DummyXShape():
    mpParent(NULL)
{
}

OUString DummyXShape::getName()
    throw(uno::RuntimeException)
{
    return maName;
}

void DummyXShape::setName( const OUString& rName )
    throw(uno::RuntimeException)
{
    maName = rName;
}

awt::Point DummyXShape::getPosition()
    throw(uno::RuntimeException)
{
    return maPosition;
}

void DummyXShape::setPosition( const awt::Point& rPoint )
    throw(uno::RuntimeException)
{
    maPosition = rPoint;
}

awt::Size DummyXShape::getSize()
    throw(uno::RuntimeException)
{
    return maSize;
}

void DummyXShape::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    maSize = rSize;
}

OUString DummyXShape::getShapeType()
    throw(uno::RuntimeException)
{
    return OUString("dummy shape");
}
uno::Reference< beans::XPropertySetInfo > DummyXShape::getPropertySetInfo()
    throw(uno::RuntimeException)
{
    return uno::Reference< beans::XPropertySetInfo >();
}

void DummyXShape::setPropertyValue( const OUString& rName, const uno::Any& )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            uno::RuntimeException)
{
    SAL_DEBUG("DummyXShape::setProperty: " << rName << " " << "Any");
}

uno::Any DummyXShape::getPropertyValue( const OUString& rName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SAL_DEBUG("DummyXShape::getPropertyValue: " << rName);
    return uno::Any();
}

void DummyXShape::addPropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void DummyXShape::removePropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void DummyXShape::addVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void DummyXShape::removeVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void DummyXShape::setPropertyValues( const uno::Sequence< OUString >& ,
        const uno::Sequence< uno::Any >& )
throw (beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException, uno::RuntimeException)
{
}

uno::Sequence< uno::Any > DummyXShape::getPropertyValues(
        const uno::Sequence< OUString >& )
    throw (uno::RuntimeException)
{
    return uno::Sequence< uno::Any >();
}

    void DummyXShape::addPropertiesChangeListener( const uno::Sequence< OUString >& , const uno::Reference< beans::XPropertiesChangeListener >& ) throw (uno::RuntimeException)
{
}

    void DummyXShape::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& ) throw (uno::RuntimeException)
{
}

void DummyXShape::firePropertiesChangeEvent( const uno::Sequence< OUString >& ,
        const uno::Reference< beans::XPropertiesChangeListener >& )
    throw (uno::RuntimeException)
{
}

OUString DummyXShape::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString("DummyXShape");
}

namespace {

uno::Sequence< OUString > listSupportedServices()
{
    static uno::Sequence< OUString > aSupportedServices;
    if(aSupportedServices.getLength() == 0)
    {
        aSupportedServices.realloc(3);
        aSupportedServices[0] = "com.sun.star.drawing.Shape";
        aSupportedServices[1] = "com.sun.star.container.Named";
        aSupportedServices[2] = "com.sun.star.beans.PropertySet";
    }

    return aSupportedServices;
}

}

uno::Sequence< OUString > DummyXShape::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    return listSupportedServices();
}

sal_Bool DummyXShape::supportsService( const OUString& rServiceName )
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSupportedServices = listSupportedServices();
    for(sal_Int32 i = 0; i < aSupportedServices.getLength(); ++i)
    {
        if(aSupportedServices[i] == rServiceName)
            return true;
    }

    return false;
}

uno::Reference< uno::XInterface > DummyXShape::getParent()
    throw(uno::RuntimeException)
{
    return mxParent;
}

void DummyXShape::setParent( const uno::Reference< uno::XInterface >& xParent )
    throw(lang::NoSupportException, uno::RuntimeException)
{
    mxParent = xParent;
}

DummyChart* DummyXShape::getRootShape()
{
    assert(mxParent.is());
    DummyXShape* pParent = dynamic_cast<DummyXShape*>(mxParent.get());
    assert(pParent);
    return pParent->getRootShape();
}

DummyChart* DummyChart::getRootShape()
{
    return this;
}

uno::Any DummyXShapes::queryInterface( const uno::Type& rType )
    throw(uno::RuntimeException)
{
    return DummyXShape::queryInterface(rType);
}

void DummyXShapes::acquire()
    throw()
{
    DummyXShape::acquire();
}

void DummyXShapes::release()
    throw()
{
    DummyXShape::release();
}

void DummyXShapes::add( const uno::Reference< drawing::XShape>& xShape )
    throw(uno::RuntimeException)
{
    DummyXShape* pChild = dynamic_cast<DummyXShape*>(xShape.get());
    assert(pChild);
    maUNOShapes.push_back(xShape);
    pChild->setParent(static_cast< ::cppu::OWeakObject* >( this ));
    maShapes.push_back(pChild);
}

void DummyXShapes::remove( const uno::Reference< drawing::XShape>& xShape )
    throw(uno::RuntimeException)
{
    std::vector< uno::Reference<drawing::XShape> >::iterator itr = std::find(maUNOShapes.begin(), maUNOShapes.end(), xShape);

    DummyXShape* pChild = dynamic_cast<DummyXShape*>((*itr).get());
    std::vector< DummyXShape* >::iterator itrShape = std::find(maShapes.begin(), maShapes.end(), pChild);
    if(itrShape != maShapes.end())
        maShapes.erase(itrShape);

    if(itr != maUNOShapes.end())
        maUNOShapes.erase(itr);
}

uno::Type DummyXShapes::getElementType()
    throw(uno::RuntimeException)
{
    return ::getCppuType(( const uno::Reference< drawing::XShape >*)0);
}

sal_Bool DummyXShapes::hasElements()
    throw(uno::RuntimeException)
{
    return !maUNOShapes.empty();
}

sal_Int32 DummyXShapes::getCount()
    throw(uno::RuntimeException)
{
    return maUNOShapes.size();
}

uno::Any DummyXShapes::getByIndex(sal_Int32 nIndex)
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException,
            uno::RuntimeException)
{
    uno::Any aShape;
    aShape <<= maUNOShapes[nIndex];
    return aShape;
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
