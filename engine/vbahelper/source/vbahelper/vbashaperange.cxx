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
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <ooo/vba/office/MsoShapeType.hpp>

#include <utility>
#include <vbahelper/vbashaperange.hxx>
#include <vbahelper/vbashape.hxx>
#include <rtl/ref.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class VbShapeRangeEnumHelper : public EnumerationHelper_BASE
{
        rtl::Reference< ScVbaShapeRange > m_xParent;
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
    VbShapeRangeEnumHelper( rtl::Reference< ScVbaShapeRange > xParent, uno::Reference< container::XIndexAccess > xIndexAccess ) : m_xParent(std::move( xParent )), m_xIndexAccess(std::move( xIndexAccess )), nIndex( 0 ) {}
        virtual bool hasMoreElements(  ) override
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }
        virtual cpo::uno::Any nextElement(  ) override
        {
                ScVbaShapeRange* pCollectionImpl = m_xParent.get();
                if ( pCollectionImpl && hasMoreElements() )
                    return pCollectionImpl->createCollectionObject(  m_xIndexAccess->getByIndex( nIndex++ ) );
                throw container::NoSuchElementException();
        }

};

}

ScVbaShapeRange::ScVbaShapeRange( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< cpo::uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xShapes, uno::Reference< drawing::XDrawPage >  xDrawPage, uno::Reference< frame::XModel > xModel  ) : ScVbaShapeRange_BASE( xParent, xContext, xShapes ), m_xDrawPage(std::move( xDrawPage )), m_xModel(std::move( xModel ))
{
}

// Methods
void
ScVbaShapeRange::Select(  )
{
    uno::Reference< view::XSelectionSupplier > xSelectSupp( m_xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelectSupp->select( cpo::uno::Any( getShapes() ) );
}

uno::Reference< msforms::XShape >
ScVbaShapeRange::Group()
{
    uno::Reference< drawing::XShapeGrouper > xShapeGrouper( m_xDrawPage, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShapeGroup > xShapeGroup( xShapeGrouper->group( getShapes() ), uno::UNO_SET_THROW );
    uno::Reference< drawing::XShape > xShape( xShapeGroup, uno::UNO_QUERY_THROW );
    return uno::Reference< msforms::XShape >( new ScVbaShape( getParent(), mxContext, xShape, getShapes(), m_xModel, office::MsoShapeType::msoGroup ) );
}

uno::Reference< drawing::XShapes > const &
ScVbaShapeRange::getShapes()
{
    if ( !m_xShapes.is() )
    {
        m_xShapes.set( drawing::ShapeCollection::create(mxContext) );
        sal_Int32 nLen = m_xIndexAccess->getCount();
        for ( sal_Int32 index = 0; index < nLen; ++index )
            m_xShapes->add( uno::Reference< drawing::XShape >( m_xIndexAccess->getByIndex( index ), uno::UNO_QUERY_THROW ) );

    }
    return m_xShapes;
}


void
ScVbaShapeRange::IncrementRotation( double Increment )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->IncrementRotation( Increment );
    }
}

void
ScVbaShapeRange::IncrementLeft( double Increment )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->IncrementLeft( Increment );
    }
}

void
ScVbaShapeRange::IncrementTop( double Increment )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->IncrementTop( Increment );
    }
}

OUString ScVbaShapeRange::getName()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getName( );
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::setName( const OUString& _name )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setName( _name );
    }
}

double ScVbaShapeRange::getHeight()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getHeight( );
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::setHeight( double _height )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setHeight( _height );
    }
}

double ScVbaShapeRange::getWidth()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getWidth( );
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::setWidth( double _width )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setWidth( _width );
    }
}

double ScVbaShapeRange::getLeft()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getLeft();
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::setLeft( double _left )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setLeft( _left );
    }
}

double ScVbaShapeRange::getTop()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getTop();
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::setTop( double _top )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setTop( _top );
    }
}

uno::Reference< ov::msforms::XLineFormat > ScVbaShapeRange::getLine()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getLine();
    }
    throw cpo::uno::RuntimeException();
}

uno::Reference< ov::msforms::XFillFormat > ScVbaShapeRange::getFill()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getFill();
    }
    throw cpo::uno::RuntimeException();
}

bool ScVbaShapeRange::getLockAspectRatio()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getLockAspectRatio();
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::setLockAspectRatio( bool _lockaspectratio )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setLockAspectRatio( _lockaspectratio );
    }
}

bool ScVbaShapeRange::getLockAnchor()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getLockAnchor();
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::setLockAnchor( bool _lockanchor )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setLockAnchor( _lockanchor );
    }
}

::sal_Int32 ScVbaShapeRange::getRelativeHorizontalPosition()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getRelativeHorizontalPosition();
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::setRelativeHorizontalPosition( ::sal_Int32 _relativehorizontalposition )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setRelativeHorizontalPosition( _relativehorizontalposition );
    }
}

::sal_Int32 ScVbaShapeRange::getRelativeVerticalPosition()
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getRelativeVerticalPosition();
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::setRelativeVerticalPosition( ::sal_Int32 _relativeverticalposition )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setRelativeVerticalPosition( _relativeverticalposition );
    }
}

cpo::uno::Any ScVbaShapeRange::TextFrame(  )
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->TextFrame();
    }
    throw cpo::uno::RuntimeException();
}

cpo::uno::Any ScVbaShapeRange::WrapFormat(  )
{
    sal_Int32 nLen = getCount();
    sal_Int32 index = 1;
    if (index <= nLen)
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->WrapFormat();
    }
    throw cpo::uno::RuntimeException();
}

void ScVbaShapeRange::ZOrder( sal_Int32 ZOrderCmd )
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( cpo::uno::Any( index ), cpo::uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->ZOrder( ZOrderCmd );
    }
}

cpo::uno::Type
ScVbaShapeRange::getElementType()
{
    return cppu::UnoType<msforms::XShape>::get();
}

uno::Reference< container::XEnumeration >
ScVbaShapeRange::createEnumeration()
{
    return new VbShapeRangeEnumHelper( this, m_xIndexAccess );
}

cpo::uno::Any
ScVbaShapeRange:: createCollectionObject( const cpo::uno::Any& aSource )
{
    uno::Reference< drawing::XShape > xShape( aSource, uno::UNO_QUERY_THROW );
    // #TODO  #FIXME Shape parent should always be the sheet the shapes belong
    // to
    uno::Reference< msforms::XShape > xVbShape( new ScVbaShape( uno::Reference< XHelperInterface >(), mxContext, xShape, getShapes(), m_xModel, ScVbaShape::getType( xShape ) ) );
    return cpo::uno::Any( xVbShape );
}

OUString
ScVbaShapeRange::getServiceImplName()
{
    return u"ScVbaShapeRange"_ustr;
}

cpo::uno::Sequence< OUString >
ScVbaShapeRange::getServiceNames()
{
    static cpo::uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msform.ShapeRange"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
