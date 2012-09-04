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
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include<com/sun/star/view/XSelectionSupplier.hpp>

#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbashaperange.hxx>
#include <vbahelper/vbashape.hxx>
using namespace ::ooo::vba;
using namespace ::com::sun::star;

class VbShapeRangeEnumHelper : public EnumerationHelper_BASE
{
        uno::Reference< XCollection > m_xParent;
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
    VbShapeRangeEnumHelper( const uno::Reference< XCollection >& xParent,  const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xParent( xParent ), m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
        virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }
        virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
        {
                ScVbaShapeRange* pCollectionImpl = dynamic_cast< ScVbaShapeRange* >(m_xParent.get());
                if ( pCollectionImpl && hasMoreElements() )
                    return pCollectionImpl->createCollectionObject(  m_xIndexAccess->getByIndex( nIndex++ ) );
                throw container::NoSuchElementException();
        }

};

ScVbaShapeRange::ScVbaShapeRange( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xShapes, const uno::Reference< drawing::XDrawPage >& xDrawPage, const uno::Reference< frame::XModel >& xModel  ) : ScVbaShapeRange_BASE( xParent, xContext, xShapes ), m_xDrawPage( xDrawPage ), m_xModel( xModel )
{
}

// Methods
void SAL_CALL
ScVbaShapeRange::Select(  ) throw (uno::RuntimeException)
{
    uno::Reference< view::XSelectionSupplier > xSelectSupp( m_xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelectSupp->select( uno::makeAny( getShapes() ) );
}

uno::Reference< msforms::XShape > SAL_CALL
ScVbaShapeRange::Group() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShapeGrouper > xShapeGrouper( m_xDrawPage, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShapeGroup > xShapeGroup( xShapeGrouper->group( getShapes() ), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xShape( xShapeGroup, uno::UNO_QUERY_THROW );
    return uno::Reference< msforms::XShape >( new ScVbaShape( getParent(), mxContext, xShape, getShapes(), m_xModel, office::MsoShapeType::msoGroup ) );
}

uno::Reference< drawing::XShapes >
ScVbaShapeRange::getShapes() throw (uno::RuntimeException)
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


void SAL_CALL
ScVbaShapeRange::IncrementRotation( double Increment ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->IncrementRotation( Increment );
    }
}

void SAL_CALL
ScVbaShapeRange::IncrementLeft( double Increment ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->IncrementLeft( Increment );
    }
}

void SAL_CALL
ScVbaShapeRange::IncrementTop( double Increment ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->IncrementTop( Increment );
    }
}

rtl::OUString SAL_CALL ScVbaShapeRange::getName() throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getName( );
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::setName( const rtl::OUString& _name ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setName( _name );
    }
}

double SAL_CALL ScVbaShapeRange::getHeight() throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getHeight( );
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::setHeight( double _height ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setHeight( _height );
    }
}

double SAL_CALL ScVbaShapeRange::getWidth() throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getWidth( );
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::setWidth( double _width ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setWidth( _width );
    }
}

double SAL_CALL ScVbaShapeRange::getLeft() throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getLeft();
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::setLeft( double _left ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setLeft( _left );
    }
}

double SAL_CALL ScVbaShapeRange::getTop() throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getTop();
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::setTop( double _top ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setTop( _top );
    }
}

uno::Reference< ov::msforms::XLineFormat > SAL_CALL ScVbaShapeRange::getLine() throw (css::uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getLine();
    }
    throw uno::RuntimeException();
}

uno::Reference< ov::msforms::XFillFormat > SAL_CALL ScVbaShapeRange::getFill() throw (css::uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getFill();
    }
    throw uno::RuntimeException();
}

::sal_Bool SAL_CALL ScVbaShapeRange::getLockAspectRatio() throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getLockAspectRatio();
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::setLockAspectRatio( ::sal_Bool _lockaspectratio ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setLockAspectRatio( _lockaspectratio );
    }
}

::sal_Bool SAL_CALL ScVbaShapeRange::getLockAnchor() throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getLockAnchor();
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::setLockAnchor( ::sal_Bool _lockanchor ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setLockAnchor( _lockanchor );
    }
}

::sal_Int32 SAL_CALL ScVbaShapeRange::getRelativeHorizontalPosition() throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getRelativeHorizontalPosition();
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::setRelativeHorizontalPosition( ::sal_Int32 _relativehorizontalposition ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setRelativeHorizontalPosition( _relativehorizontalposition );
    }
}

::sal_Int32 SAL_CALL ScVbaShapeRange::getRelativeVerticalPosition() throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->getRelativeVerticalPosition();
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::setRelativeVerticalPosition( ::sal_Int32 _relativeverticalposition ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->setRelativeVerticalPosition( _relativeverticalposition );
    }
}

uno::Any SAL_CALL ScVbaShapeRange::TextFrame(  ) throw (css::uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->TextFrame();
    }
    throw uno::RuntimeException();
}

uno::Any SAL_CALL ScVbaShapeRange::WrapFormat(  ) throw (css::uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; /* ++index unreachable */ )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xShape->WrapFormat();
    }
    throw uno::RuntimeException();
}

void SAL_CALL ScVbaShapeRange::ZOrder( sal_Int32 ZOrderCmd ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = getCount();
    for ( sal_Int32 index = 1; index <= nLen; ++index )
    {
        uno::Reference< msforms::XShape > xShape( Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
        xShape->ZOrder( ZOrderCmd );
    }
}

uno::Type SAL_CALL
ScVbaShapeRange::getElementType() throw (uno::RuntimeException)
{
    return msforms::XShape::static_type(0);
}

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaShapeRange::createEnumeration() throw (uno::RuntimeException)
{
    return new VbShapeRangeEnumHelper( this, m_xIndexAccess );
}

uno::Any
ScVbaShapeRange:: createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< drawing::XShape > xShape( aSource, uno::UNO_QUERY_THROW );
    // #TODO  #FIXME Shape parent should always be the sheet the shapes belong
    // to
    uno::Reference< msforms::XShape > xVbShape( new ScVbaShape( uno::Reference< XHelperInterface >(), mxContext, xShape, getShapes(), m_xModel, ScVbaShape::getType( xShape ) ) );
        return uno::makeAny( xVbShape );
}

rtl::OUString
ScVbaShapeRange::getServiceImplName()
{
    return rtl::OUString("ScVbaShapeRange");
}

uno::Sequence< rtl::OUString >
ScVbaShapeRange::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( "ooo.vba.msform.ShapeRange"  );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
