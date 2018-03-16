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
#include "vbaapplication.hxx"
#include "vbadocument.hxx"
#include <osl/file.hxx>
#include <vbahelper/vbahelper.hxx>
#include "vbawindow.hxx"
#include "vbasystem.hxx"
#include "vbaoptions.hxx"
#include "vbaselection.hxx"
#include "vbadocuments.hxx"
#include "vbaaddins.hxx"
#include "vbadialogs.hxx"
#include <ooo/vba/XConnectionPoint.hpp>
#include <ooo/vba/word/WdEnableCancelKey.hpp>
#include <ooo/vba/word/XApplicationOutgoing.hpp>
#include <basic/sbuno.hxx>
#include <editeng/acorrcfg.hxx>
#include "wordvbahelper.hxx"
#include <docsh.hxx>
#include "vbalistgalleries.hxx"

using namespace ::ooo;
using namespace ::ooo::vba;
using namespace ::com::sun::star;

class SwVbaApplicationOutgoingConnectionPoint : public cppu::WeakImplHelper<XConnectionPoint>
{
private:
    SwVbaApplication* mpApp;

public:
    SwVbaApplicationOutgoingConnectionPoint( SwVbaApplication* pApp );

    // XConnectionPoint
    sal_uInt32 SAL_CALL Advise(const uno::Reference< XSink >& Sink ) override;
    void SAL_CALL Unadvise( sal_uInt32 Cookie ) override;
};

SwVbaApplication::SwVbaApplication( uno::Reference<uno::XComponentContext >& xContext ):
    SwVbaApplication_BASE( xContext )
{
}

SwVbaApplication::~SwVbaApplication()
{
    // FIXME: Sadly this is not the place to do this, this dtor is never called, it seems
    for (auto& i : mvSinks)
    {
        if (i.is())
            i->Call("Quit", uno::Sequence<uno::Any>());
    }
}

sal_uInt32
SwVbaApplication::AddSink( const css::uno::Reference< XSink >& xSink )
{
    mvSinks.push_back(xSink);
    return mvSinks.size();;
}

void
SwVbaApplication::RemoveSink( sal_uInt32 nNumber )
{
    if (nNumber < 1 || nNumber > mvSinks.size())
        return;

    mvSinks[nNumber-1] = uno::Reference< XSink >();
}

OUString SAL_CALL
SwVbaApplication::getName()
{
    return OUString("Microsoft Word" );
}

uno::Reference< word::XDocument > SAL_CALL
SwVbaApplication::getActiveDocument()
{
    return new SwVbaDocument( this, mxContext, getCurrentDocument() );
}

uno::Reference< word::XWindow > SAL_CALL
SwVbaApplication::getActiveWindow()
{
    // #FIXME so far can't determine Parent
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
    return new SwVbaWindow( uno::Reference< XHelperInterface >(), mxContext, xModel, xController );
}

uno::Reference<word::XSystem > SAL_CALL
SwVbaApplication::getSystem()
{
    return uno::Reference< word::XSystem >( new SwVbaSystem( mxContext ) );
}

uno::Reference<word::XOptions > SAL_CALL
SwVbaApplication::getOptions()
{
    return uno::Reference< word::XOptions >( new SwVbaOptions( mxContext ) );
}

uno::Any SAL_CALL
SwVbaApplication::CommandBars( const uno::Any& aIndex )
{
    return VbaApplicationBase::CommandBars( aIndex );
}

uno::Reference< word::XSelection > SAL_CALL
SwVbaApplication::getSelection()
{
    return new SwVbaSelection( this, mxContext, getCurrentDocument() );
}

uno::Any SAL_CALL
SwVbaApplication::Documents( const uno::Any& index )
{
    // FIXME DUMMY just to test calling this somewhere... the dtor is never called
    for (auto& i : mvSinks)
    {
        if (i.is())
            i->Call("Quit", uno::Sequence<uno::Any>());
    }

    uno::Reference< XCollection > xCol( new SwVbaDocuments( this, mxContext ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Any SAL_CALL
SwVbaApplication::Addins( const uno::Any& index )
{
    static uno::Reference< XCollection > xCol( new SwVbaAddins( this, mxContext ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Any SAL_CALL
SwVbaApplication::Dialogs( const uno::Any& index )
{
    uno::Reference< word::XDialogs > xCol( new SwVbaDialogs( this, mxContext, getCurrentDocument() ));
    if ( index.hasValue() )
        return xCol->Item( index );
    return uno::makeAny( xCol );
}

uno::Any SAL_CALL
SwVbaApplication::ListGalleries( const uno::Any& index )
{
    uno::Reference< text::XTextDocument > xTextDoc( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new SwVbaListGalleries( this, mxContext, xTextDoc ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

sal_Bool SAL_CALL SwVbaApplication::getDisplayAutoCompleteTips()
{
    return SvxAutoCorrCfg::Get().IsAutoTextTip();
}

void SAL_CALL SwVbaApplication::setDisplayAutoCompleteTips( sal_Bool _displayAutoCompleteTips )
{
    SvxAutoCorrCfg::Get().SetAutoTextTip( _displayAutoCompleteTips );
}

sal_Int32 SAL_CALL SwVbaApplication::getEnableCancelKey()
{
    // the default value is wdCancelInterrupt in Word
    return word::WdEnableCancelKey::wdCancelInterrupt;
}

void SAL_CALL SwVbaApplication::setEnableCancelKey( sal_Int32/* _enableCancelKey */)
{
    // seems not supported in Writer
}

float SAL_CALL SwVbaApplication::CentimetersToPoints( float Centimeters )
{
    return VbaApplicationBase::CentimetersToPoints( Centimeters );
}

void SAL_CALL SwVbaApplication::ShowMe()
{
    // No idea what we should or could do
}

// XInterfaceWithIID

OUString SAL_CALL
SwVbaApplication::getIID()
{
    return OUString("{82154421-0FBF-11d4-8313-005004526AB4}");
}

uno::Reference< frame::XModel >
SwVbaApplication::getCurrentDocument()
{
    return getCurrentWordDoc( mxContext );
}

// XConnectable

OUString SAL_CALL
SwVbaApplication::GetIIDForClassItselfNotCoclass()
{
    return OUString("{82154423-0FBF-11D4-8313-005004526AB4}");
}

TypeAndIID SAL_CALL
SwVbaApplication::GetConnectionPoint()
{
    TypeAndIID aResult =
        { word::XApplicationOutgoing::static_type(),
          "{82154422-0FBF-11D4-8313-005004526AB4}"
        };

    return aResult;
}

uno::Reference<XConnectionPoint> SAL_CALL
SwVbaApplication::FindConnectionPoint()
{
    uno::Reference<XConnectionPoint> xCP(new SwVbaApplicationOutgoingConnectionPoint(this));
    return xCP;
}

// XHelperInterface

OUString
SwVbaApplication::getServiceImplName()
{
    return OUString("SwVbaApplication");
}

uno::Sequence< OUString >
SwVbaApplication::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Application";
    }
    return aServiceNames;
}

SwVbaApplicationOutgoingConnectionPoint::SwVbaApplicationOutgoingConnectionPoint( SwVbaApplication* pApp ) :
    mpApp(pApp)
{
}

// SwVbaApplicationOutgoingConnectionPoint

// XConnectionPoint
sal_uInt32 SAL_CALL
SwVbaApplicationOutgoingConnectionPoint::Advise( const uno::Reference< XSink >& Sink )
{
    return mpApp->AddSink(Sink);
}

void SAL_CALL
SwVbaApplicationOutgoingConnectionPoint::Unadvise( sal_uInt32 Cookie )
{
    mpApp->RemoveSink( Cookie );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
