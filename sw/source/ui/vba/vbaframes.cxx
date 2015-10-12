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
#include "vbaframes.hxx"
#include "vbaframe.hxx"
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include "wordvbahelper.hxx"
#include <cppuhelper/implbase.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class FramesEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
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
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return ( nCurrentPos < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
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
    return cppu::UnoType<word::XFrame>::get();
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

OUString
SwVbaFrames::getServiceImplName()
{
    return OUString("SwVbaFrames");
}

css::uno::Sequence<OUString>
SwVbaFrames::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.Frames";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
