/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column:100 -*- */
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

#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

#include "vbaapplication.hxx"
#include "vbadocument.hxx"
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <vbahelper/vbahelper.hxx>
#include "vbawindow.hxx"
#include "vbasystem.hxx"
#include "vbaoptions.hxx"
#include "vbaselection.hxx"
#include "vbadocuments.hxx"
#include "vbaaddins.hxx"
#include "vbamailmerge.hxx"
#include "vbadialogs.hxx"
#include <ooo/vba/XConnectionPoint.hpp>
#include <ooo/vba/word/WdEnableCancelKey.hpp>
#include <ooo/vba/word/WdWindowState.hpp>
#include <ooo/vba/word/XApplicationOutgoing.hpp>
#include <ooo/vba/word/XBookmarks.hpp>
#include <basic/sbuno.hxx>
#include <editeng/acorrcfg.hxx>
#include "wordvbahelper.hxx"
#include <docsh.hxx>
#include <swdll.hxx>
#include <swmodule.hxx>
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

class SwWordBasic : public cppu::WeakImplHelper<word::XWordBasic>
{
private:
    SwVbaApplication* mpApp;

public:
    SwWordBasic( SwVbaApplication* pApp );

    // XWordBasic
    virtual sal_Int32 SAL_CALL getMailMergeMainDocumentType() override;
    virtual void SAL_CALL setMailMergeMainDocumentType( sal_Int32 _mailmergemaindocumenttype ) override;

    virtual void SAL_CALL FileOpen( const OUString& Name, const uno::Any& ConfirmConversions, const uno::Any& ReadOnly, const uno::Any& AddToMru, const uno::Any& PasswordDoc, const uno::Any& PasswordDot, const uno::Any& Revert, const uno::Any& WritePasswordDoc, const uno::Any& WritePasswordDot ) override;
    virtual OUString SAL_CALL WindowName() override;
    virtual sal_Bool SAL_CALL ExistingBookmark( const OUString& Name ) override;
    virtual void SAL_CALL MailMergeOpenDataSource(const OUString& Name, const css::uno::Any& Format,
                                                  const css::uno::Any& ConfirmConversions, const css::uno::Any& ReadOnly,
                                                  const css::uno::Any& LinkToSource, const css::uno::Any& AddToRecentFiles,
                                                  const css::uno::Any& PasswordDocument, const css::uno::Any& PasswordTemplate,
                                                  const css::uno::Any& Revert, const css::uno::Any& WritePasswordDocument,
                                                  const css::uno::Any& WritePasswordTemplate, const css::uno::Any& Connection,
                                                  const css::uno::Any& SQLStatement, const css::uno::Any& SQLStatement1,
                                                  const css::uno::Any& OpenExclusive, const css::uno::Any& SubType) override;
};

SwVbaApplication::SwVbaApplication( uno::Reference<uno::XComponentContext >& xContext ):
    SwVbaApplication_BASE( xContext )
{
}

SwVbaApplication::~SwVbaApplication()
{
}

sal_uInt32
SwVbaApplication::AddSink( const uno::Reference< XSink >& xSink )
{
    {
        SolarMutexGuard aGuard;
        SwGlobals::ensure();
    }
    // No harm in potentially calling this several times
    SW_MOD()->RegisterAutomationApplicationEventsCaller( uno::Reference< XSinkCaller >(this) );
    mvSinks.push_back(xSink);
    return mvSinks.size();
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

SwVbaWindow *
SwVbaApplication::getActiveSwVbaWindow()
{
    // #FIXME so far can't determine Parent
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
    return new SwVbaWindow( uno::Reference< XHelperInterface >(), mxContext, xModel, xController );
}

uno::Reference< css::uno::XComponentContext > const &
SwVbaApplication::getContext()
{
    return mxContext;
}

uno::Reference< word::XWindow > SAL_CALL
SwVbaApplication::getActiveWindow()
{
    return getActiveSwVbaWindow();
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

uno::Reference< word::XWordBasic > SAL_CALL
SwVbaApplication::getWordBasic()
{
    uno::Reference< word::XWordBasic > xWB( new SwWordBasic( this ) );
    return xWB;
}

uno::Any SAL_CALL
SwVbaApplication::Documents( const uno::Any& index )
{
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

sal_Int32 SAL_CALL SwVbaApplication::getWindowState()
{
    auto xWindow = getActiveWindow();
    if (xWindow.is())
    {
        uno::Any aState = xWindow->getWindowState();
        sal_Int32 nState;
        if (aState >>= nState)
            return nState;
    }

    return word::WdWindowState::wdWindowStateNormal; // ?
}

void SAL_CALL SwVbaApplication::setWindowState( sal_Int32 _windowstate )
{
    auto xWindow = getActiveWindow();
    if (xWindow.is())
    {
        uno::Any aState;
        aState <<= _windowstate;
        xWindow->setWindowState( aState );
    }
}

sal_Int32 SAL_CALL SwVbaApplication::getWidth()
{
    auto pWindow = getActiveSwVbaWindow();
    return pWindow->getWidth();
}

void SAL_CALL SwVbaApplication::setWidth( sal_Int32 _width )
{
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setWidth( _width );
}

sal_Int32 SAL_CALL SwVbaApplication::getHeight()
{
    auto pWindow = getActiveSwVbaWindow();
    return pWindow->getHeight();
}

void SAL_CALL SwVbaApplication::setHeight( sal_Int32 _height )
{
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setHeight( _height );
}

sal_Int32 SAL_CALL SwVbaApplication::getLeft()
{
    auto pWindow = getActiveSwVbaWindow();
    return pWindow->getLeft();
}

void SAL_CALL SwVbaApplication::setLeft( sal_Int32 _left )
{
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setLeft( _left );
}

sal_Int32 SAL_CALL SwVbaApplication::getTop()
{
    auto pWindow = getActiveSwVbaWindow();
    return pWindow->getTop();
}

void SAL_CALL SwVbaApplication::setTop( sal_Int32 _top )
{
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setTop( _top );
}

OUString SAL_CALL SwVbaApplication::getStatusBar()
{
    return OUString("");
}

uno::Any SAL_CALL SwVbaApplication::getCustomizationContext()
{
    return uno::Any(); // ???
}

void SAL_CALL SwVbaApplication::setCustomizationContext( const uno::Any& _customizationcontext )
{
    (void) _customizationcontext;
    // ???
}

void SAL_CALL SwVbaApplication::setStatusBar( const OUString& _statusbar )
{
    // ScVbaAppSettings::setStatusBar() also uses the XStatusIndicator to show this, so maybe that is OK?
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY );
    if (xModel.is())
    {
        uno::Reference< task::XStatusIndicatorSupplier > xStatusIndicatorSupplier( xModel->getCurrentController(), uno::UNO_QUERY );
        if (xStatusIndicatorSupplier.is())
        {
            uno::Reference< task::XStatusIndicator > xStatusIndicator( xStatusIndicatorSupplier->getStatusIndicator(), uno::UNO_QUERY );
            if (xStatusIndicator.is())
                xStatusIndicator->start( _statusbar, 100 );
        }
    }

    // Yes, we intentionally use the "extensions.olebridge" tag here even if this is sw. We
    // interpret setting the StatusBar property as a request from an Automation client to display
    // the string in LibreOffice's debug output, and all other generic Automation support debug
    // output (in extensions/source/ole) uses that tag. If the check for "cross-module" or mixed log
    // areas in compilerplugins/clang/sallogareas.cxx is re-activated, this will have to be added as
    // a special case.

    SAL_INFO("extensions.olebridge", "Client debug output: " << _statusbar);
}

float SAL_CALL SwVbaApplication::CentimetersToPoints( float Centimeters )
{
    return VbaApplicationBase::CentimetersToPoints( Centimeters );
}

void SAL_CALL SwVbaApplication::ShowMe()
{
    // No idea what we should or could do
}

void SAL_CALL SwVbaApplication::Resize( sal_Int32 Width, sal_Int32 Height )
{
    // Have to do it like this as the Width and Height are hidden away in the ooo::vba::XWindowBase
    // which ooo::vba::word::XApplication does not inherit from. SwVbaWindow, however, does inherit
    // from XWindowBase. Ugh.
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setWidth( Width );
    pWindow->setHeight( Height );
}

void SAL_CALL SwVbaApplication::Move( sal_Int32 Left, sal_Int32 Top )
{
    // See comment in Resize().
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setLeft( Left );
    pWindow->setTop( Top );
}

// XInterfaceWithIID

OUString SAL_CALL
SwVbaApplication::getIID()
{
    return OUString("{82154421-0FBF-11d4-8313-005004526AB4}");
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

uno::Reference< frame::XModel >
SwVbaApplication::getCurrentDocument()
{
    return getCurrentWordDoc( mxContext );
}

// XSinkCaller

void SAL_CALL
SwVbaApplication::CallSinks( const OUString& Method, uno::Sequence< uno::Any >& Arguments )
{
    for (auto& i : mvSinks)
    {
        if (i.is())
            i->Call(Method, Arguments);
    }
}

// SwVbaApplicationOutgoingConnectionPoint

SwVbaApplicationOutgoingConnectionPoint::SwVbaApplicationOutgoingConnectionPoint( SwVbaApplication* pApp ) :
    mpApp(pApp)
{
}

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

// SwWordBasic

SwWordBasic::SwWordBasic( SwVbaApplication* pApp ) :
    mpApp(pApp)
{
}

// XWordBasic
sal_Int32 SAL_CALL
SwWordBasic::getMailMergeMainDocumentType()
{
    return SwVbaMailMerge::get( mpApp->getParent(), mpApp->getContext() )->getMainDocumentType();
}

// XWordBasic
void SAL_CALL
SwWordBasic::setMailMergeMainDocumentType( sal_Int32 _mailmergemaindocumenttype )
{
    SwVbaMailMerge::get( mpApp->getParent(), mpApp->getContext() )->setMainDocumentType( _mailmergemaindocumenttype );
}

void SAL_CALL
SwWordBasic::FileOpen( const OUString& Name, const uno::Any& ConfirmConversions, const uno::Any& ReadOnly, const uno::Any& AddToMru, const uno::Any& PasswordDoc, const uno::Any& PasswordDot, const uno::Any& Revert, const uno::Any& WritePasswordDoc, const uno::Any& WritePasswordDot )
{
    uno::Any aDocuments = mpApp->Documents( uno::Any() );

    uno::Reference<word::XDocuments> rDocuments;

    if (aDocuments >>= rDocuments)
        rDocuments->Open( Name, ConfirmConversions, ReadOnly, AddToMru, PasswordDoc, PasswordDot, Revert, WritePasswordDoc, WritePasswordDot, uno::Any(), uno::Any(), uno::Any(), uno::Any(), uno::Any(), uno::Any(), uno::Any() );
}

OUString SAL_CALL
SwWordBasic::WindowName()
{
    return mpApp->getActiveSwVbaWindow()->getCaption();
}

sal_Bool SAL_CALL
SwWordBasic::ExistingBookmark( const OUString& Name )
{
    uno::Reference< word::XBookmarks > xBookmarks( mpApp->getActiveDocument()->Bookmarks( uno::Any() ), uno::UNO_QUERY );
    return xBookmarks.is() && xBookmarks->Exists( Name );
}

void SAL_CALL
SwWordBasic::MailMergeOpenDataSource( const OUString& Name, const css::uno::Any& Format,
                                      const css::uno::Any& ConfirmConversions, const css::uno::Any& ReadOnly,
                                      const css::uno::Any& LinkToSource, const css::uno::Any& AddToRecentFiles,
                                      const css::uno::Any& PasswordDocument, const css::uno::Any& PasswordTemplate,
                                      const css::uno::Any& Revert, const css::uno::Any& WritePasswordDocument,
                                      const css::uno::Any& WritePasswordTemplate, const css::uno::Any& Connection,
                                      const css::uno::Any& SQLStatement, const css::uno::Any& SQLStatement1,
                                      const css::uno::Any& OpenExclusive, const css::uno::Any& SubType )
{
    mpApp->getActiveDocument()->getMailMerge()->OpenDataSource( Name, Format, ConfirmConversions, ReadOnly,
                                                                LinkToSource, AddToRecentFiles,
                                                                PasswordDocument, PasswordTemplate,
                                                                Revert, WritePasswordDocument,
                                                                WritePasswordTemplate, Connection,
                                                                SQLStatement, SQLStatement1,
                                                                OpenExclusive, SubType );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
