/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#include "vbaframes.hxx"
#include "vbaframe.hxx"
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include "wordvbahelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

typedef ::cppu::WeakImplHelper1< container::XEnumeration > FramesEnumeration_Base;
class FramesEnumeration : public FramesEnumeration_Base
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< container::XIndexAccess> mxIndexAccess;
    uno::Reference< frame::XModel > mxModel;
    sal_Int32 nCurrentPos;
public:
    FramesEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess,  const uno::Reference< frame::XModel >& xModel  ) throw ( uno::RuntimeException ) : mxParent( xParent ), mxContext( xContext), mxIndexAccess( xIndexAccess ), mxModel( xModel ), nCurrentPos(0)
    {
    }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nCurrentPos < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        uno::Reference< text::XTextFrame > xTextFrame( mxIndexAccess->getByIndex( nCurrentPos++ ), uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< word::XFrame > ( new SwVbaFrame( mxParent, mxContext, mxModel, xTextFrame ) ) );
    }

};

SwVbaFrames::SwVbaFrames( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xFrames, const uno::Reference< frame::XModel >& xModel ): SwVbaFrames_BASE( xParent, xContext, xFrames ), mxModel( xModel )
{
    mxFramesSupplier.set( mxModel, uno::UNO_QUERY_THROW );
}
// XEnumerationAccess
uno::Type
SwVbaFrames::getElementType() throw (uno::RuntimeException)
{
    return word::XFrame::static_type(0);
}

uno::Reference< container::XEnumeration >
SwVbaFrames::createEnumeration() throw (uno::RuntimeException)
{
    return new FramesEnumeration( this, mxContext,m_xIndexAccess, mxModel );
}

uno::Any
SwVbaFrames::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< text::XTextFrame > xTextFrame( aSource, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XFrame > ( new SwVbaFrame( this, mxContext, mxModel, xTextFrame ) ) );
}

rtl::OUString&
SwVbaFrames::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaFrames") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaFrames::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Frames") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
