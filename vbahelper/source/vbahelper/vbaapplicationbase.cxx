/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <vbahelper/vbaapplicationbase.hxx>
#include <sal/macros.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/awt/XWindow2.hpp>

#include <filter/msfilter/msvbahelper.hxx>
#include <rtl/ref.hxx>
#include <tools/datetime.hxx>
#include <vcl/timer.hxx>
#include <vcl/svapp.hxx>

#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/vbahelper.hxx>

#include <comphelper/asyncquithandler.hxx>

#include "vbacommandbars.hxx"

#include <boost/functional/hash.hpp>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define OFFICEVERSION "11.0"

typedef ::std::pair< OUString, ::std::pair< double, double > > VbaTimerInfo;

namespace {

class VbaTimer
{
    Timer m_aTimer;
    VbaTimerInfo m_aTimerInfo;
    ::rtl::Reference< VbaApplicationBase > m_xBase;

public:
    VbaTimer()
    {}

    ~VbaTimer()
    {
        m_aTimer.Stop();
    }

    VbaTimer(const VbaTimer&) = delete;
    VbaTimer& operator=(const VbaTimer&) = delete;

    static double GetNow()
    {
        DateTime aNow( DateTime::SYSTEM );
        Date aRefDate( 1,1,1900 );
        tools::Long nDiffDays = aNow - aRefDate;
        nDiffDays += 2; // Change VisualBasic: 1.Jan.1900 == 2

        tools::Long nDiffSeconds = aNow.GetHour() * 3600 + aNow.GetMin() * 60 + aNow.GetSec();
        return static_cast<double>(nDiffDays) + static_cast<double>(nDiffSeconds)/double(24*3600);
    }

    static sal_Int32 GetTimerMilliseconds( double nFrom, double nTo )
    {
        double nResult = nTo - nFrom;
        if ( nResult > 0 )
            nResult *= 24*3600*1000;
        else
            nResult = 50;

        return static_cast<sal_Int32>(nResult);
    }

    void Start( const ::rtl::Reference< VbaApplicationBase >& xBase, const OUString& aFunction, double nFrom, double nTo )
    {
        if ( !xBase.is() || aFunction.isEmpty() )
            throw uno::RuntimeException( "Unexpected arguments!" );

        m_xBase = xBase;
        m_aTimerInfo = VbaTimerInfo( aFunction, ::std::pair< double, double >( nFrom, nTo ) );
        m_aTimer.SetInvokeHandler( LINK( this, VbaTimer, MacroCallHdl ) );
        m_aTimer.SetTimeout( GetTimerMilliseconds( GetNow(), nFrom ) );
        m_aTimer.Start();
    }

    DECL_LINK( MacroCallHdl, Timer*, void );
};

}

IMPL_LINK_NOARG(VbaTimer, MacroCallHdl, Timer *, void)
{
    if ( m_aTimerInfo.second.second == 0 || GetNow() < m_aTimerInfo.second.second )
    {
        uno::Any aDummyArg;
        try
        {
            m_xBase->Run( m_aTimerInfo.first, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg, aDummyArg );
        }
        catch( uno::Exception& )
        {}
    }

    // must be the last call in the method since it deletes the timer
    try
    {
        m_xBase->OnTime( uno::makeAny( m_aTimerInfo.second.first ), m_aTimerInfo.first, uno::makeAny( m_aTimerInfo.second.second ), uno::makeAny( false ) );
    } catch( uno::Exception& )
    {}
}

namespace {

struct VbaTimerInfoHash
{
    size_t operator()( const VbaTimerInfo& rTimerInfo ) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, rTimerInfo.first.hashCode());
        boost::hash_combine(seed, rTimerInfo.second.first);
        boost::hash_combine(seed, rTimerInfo.second.second);
        return seed;
    }
};

}

typedef std::unordered_map< VbaTimerInfo, std::unique_ptr<VbaTimer>, VbaTimerInfoHash > VbaTimerHashMap;

struct VbaApplicationBase_Impl final
{
    VbaTimerHashMap m_aTimerHash;
    bool mbVisible;
    OUString msCaption;

    VbaApplicationBase_Impl() : mbVisible( true ) {}
};

VbaApplicationBase::VbaApplicationBase( const uno::Reference< uno::XComponentContext >& xContext )
                    : ApplicationBase_BASE( uno::Reference< XHelperInterface >(), xContext )
                    , m_pImpl( new VbaApplicationBase_Impl )
{
}

VbaApplicationBase::~VbaApplicationBase()
{
}

sal_Bool SAL_CALL
VbaApplicationBase::getScreenUpdating()
{
    uno::Reference< frame::XModel > xModel = getCurrentDocument();
    if (!xModel.is())
        return true;
    return !xModel->hasControllersLocked();
}

void SAL_CALL
VbaApplicationBase::setScreenUpdating(sal_Bool bUpdate)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    // #163808# use helper from module "basic" to lock all documents of this application
    ::basic::vba::lockControllersOfAllDocuments( xModel, !bUpdate );
}

sal_Bool SAL_CALL
VbaApplicationBase::getDisplayStatusBar()
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_SET_THROW );
    uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );

    uno::Reference< frame::XLayoutManager > xLayoutManager( xProps->getPropertyValue( "LayoutManager"), uno::UNO_QUERY_THROW );
    if( xLayoutManager->isElementVisible( "private:resource/statusbar/statusbar" ) ){
        return true;
    }
    return false;
}

void SAL_CALL
VbaApplicationBase::setDisplayStatusBar(sal_Bool bDisplayStatusBar)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_SET_THROW );
    uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );

    uno::Reference< frame::XLayoutManager > xLayoutManager( xProps->getPropertyValue( "LayoutManager" ), uno::UNO_QUERY_THROW );
    OUString url( "private:resource/statusbar/statusbar" );
    if( bDisplayStatusBar && !xLayoutManager->isElementVisible( url ) ){
        if( !xLayoutManager->showElement( url ) )
            xLayoutManager->createElement( url );
        return;
    }
    else if( !bDisplayStatusBar && xLayoutManager->isElementVisible( url ) ){
        xLayoutManager->hideElement( url );
        return;
    }
}

sal_Bool SAL_CALL VbaApplicationBase::getInteractive()
{
    uno::Reference< frame::XModel > xModel = getCurrentDocument();
    if (!xModel.is())
        return true;

    uno::Reference< frame::XController > xController( xModel->getCurrentController() );
    if (!xController.is())
        return true;

    uno::Reference< frame::XFrame > xFrame( xController->getFrame() );
    if (!xFrame.is())
        return true;

    uno::Reference< awt::XWindow2 > xWindow( xFrame->getContainerWindow(), uno::UNO_QUERY );
    if (!xWindow.is())
        return true;

    return xWindow->isEnabled();
}

void SAL_CALL VbaApplicationBase::setInteractive( sal_Bool bInteractive )
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    // #163808# use helper from module "basic" to enable/disable all container windows of all documents of this application
    ::basic::vba::enableContainerWindowsOfAllDocuments( xModel, bInteractive );
}

sal_Bool SAL_CALL VbaApplicationBase::getVisible()
{
    return m_pImpl->mbVisible;    // dummy implementation
}

void SAL_CALL VbaApplicationBase::setVisible( sal_Bool bVisible )
{
    m_pImpl->mbVisible = bVisible;  // dummy implementation
}

OUString SAL_CALL VbaApplicationBase::getCaption()
{
    SbMethod* pMeth = StarBASIC::GetActiveMethod();
    if (!pMeth)
    {
        // When called from Automation clients, we don't even try, as there doesn't seem to be any
        // good way to get at the actual "caption" (title) of the application's window (any of them,
        // if there are several). We just keep a copy of a fake caption in the VbaApplicationBase_Impl.
        return m_pImpl->msCaption;
    }

    // No idea if this code, which uses APIs that apparently are related to StarBasic (check
    // getCurrentDoc() in vbahelper.cxx), actually works any better.
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_SET_THROW );
    return xFrame->getName();
}

void SAL_CALL VbaApplicationBase::setCaption( const OUString& sCaption )
{
    // See comments in getCaption().

    SbMethod* pMeth = StarBASIC::GetActiveMethod();
    if (!pMeth)
    {
        m_pImpl->msCaption = sCaption;
        return;
    }

    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_SET_THROW );
    xFrame->setName( sCaption );
}

void SAL_CALL
VbaApplicationBase::OnKey( const OUString& Key, const uno::Any& Procedure )
{
    // parse the Key & modifiers
    awt::KeyEvent aKeyEvent = parseKeyEvent( Key );
    OUString MacroName;
    Procedure >>= MacroName;
    uno::Reference< frame::XModel > xModel;
    SbMethod* pMeth = StarBASIC::GetActiveMethod();
    if ( pMeth )
    {
        SbModule* pMod = dynamic_cast< SbModule* >( pMeth->GetParent() );
        if ( pMod )
            xModel = StarBASIC::GetModelFromBasic( pMod );
    }

    if ( !xModel.is() )
        xModel = getCurrentDocument();

    applyShortCutKeyBinding( xModel, aKeyEvent, MacroName );
}

uno::Any SAL_CALL
VbaApplicationBase::CommandBars( const uno::Any& aIndex )
{
    uno::Reference< XCommandBars > xCommandBars( new ScVbaCommandBars( this, mxContext, uno::Reference< container::XIndexAccess >(), getCurrentDocument() ) );
    if( aIndex.hasValue() )
        return xCommandBars->Item( aIndex, uno::Any() );
    return uno::makeAny( xCommandBars );
}

OUString SAL_CALL
VbaApplicationBase::getVersion()
{
    return OFFICEVERSION;
}

uno::Any SAL_CALL VbaApplicationBase::Run( const OUString& MacroName, const uno::Any& varg1, const uno::Any& varg2, const uno::Any& varg3, const uno::Any& varg4, const uno::Any& varg5, const uno::Any& varg6, const uno::Any& varg7, const uno::Any& varg8, const uno::Any& varg9, const uno::Any& varg10, const uno::Any& varg11, const uno::Any& varg12, const uno::Any& varg13, const uno::Any& varg14, const uno::Any& varg15, const uno::Any& varg16, const uno::Any& varg17, const uno::Any& varg18, const uno::Any& varg19, const uno::Any& varg20, const uno::Any& varg21, const uno::Any& varg22, const uno::Any& varg23, const uno::Any& varg24, const uno::Any& varg25, const uno::Any& varg26, const uno::Any& varg27, const uno::Any& varg28, const uno::Any& varg29, const uno::Any& varg30 )
{
    OUString aMacroName = MacroName.trim();
    if( aMacroName.startsWith("!") )
        aMacroName = aMacroName.copy(1).trim();

    uno::Reference< frame::XModel > xModel;
    SbMethod* pMeth = StarBASIC::GetActiveMethod();
    if ( pMeth )
    {
        SbModule* pMod = dynamic_cast< SbModule* >( pMeth->GetParent() );
        if ( pMod )
            xModel = StarBASIC::GetModelFromBasic( pMod );
    }

    if ( !xModel.is() )
        xModel = getCurrentDocument();

    MacroResolvedInfo aMacroInfo = resolveVBAMacro( getSfxObjShell( xModel ), aMacroName );
    if( !aMacroInfo.mbFound )
    {
        throw uno::RuntimeException( "The macro doesn't exist" );
    }

    // handle the arguments
    const uno::Any* aArgsPtrArray[] = { &varg1, &varg2, &varg3, &varg4, &varg5, &varg6, &varg7, &varg8, &varg9, &varg10, &varg11, &varg12, &varg13, &varg14, &varg15, &varg16, &varg17, &varg18, &varg19, &varg20, &varg21, &varg22, &varg23, &varg24, &varg25, &varg26, &varg27, &varg28, &varg29, &varg30 };

    int nArg = SAL_N_ELEMENTS( aArgsPtrArray );
    uno::Sequence< uno::Any > aArgs( nArg );

    const uno::Any** pArg = aArgsPtrArray;
    const uno::Any** pArgEnd = aArgsPtrArray + nArg;

    sal_Int32 nArgProcessed = 0;

    for ( ; pArg != pArgEnd; ++pArg, ++nArgProcessed )
        aArgs[ nArgProcessed ] =  **pArg;

    // resize array to position of last param with value
    aArgs.realloc( nArgProcessed + 1 );

    uno::Any aRet;
    uno::Any aDummyCaller;
    executeMacro( aMacroInfo.mpDocContext, aMacroInfo.msResolvedMacro, aArgs, aRet, aDummyCaller );

    return aRet;
}

void SAL_CALL VbaApplicationBase::OnTime( const uno::Any& aEarliestTime, const OUString& aFunction, const uno::Any& aLatestTime, const uno::Any& aSchedule )
{
    if ( aFunction.isEmpty() )
        throw uno::RuntimeException( "Unexpected function name!" );

    double nEarliestTime = 0;
    double nLatestTime = 0;
    if ( !( aEarliestTime >>= nEarliestTime )
      || ( aLatestTime.hasValue() && !( aLatestTime >>= nLatestTime ) ) )
        throw uno::RuntimeException( "Only double is supported as time for now!" );

    bool bSetTimer = true;
    aSchedule >>= bSetTimer;

    VbaTimerInfo aTimerIndex( aFunction, ::std::pair< double, double >( nEarliestTime, nLatestTime ) );

    VbaTimerHashMap::iterator aIter = m_pImpl->m_aTimerHash.find( aTimerIndex );
    if ( aIter != m_pImpl->m_aTimerHash.end() )
    {
        m_pImpl->m_aTimerHash.erase( aIter );
    }

    if ( bSetTimer )
    {
        VbaTimer* pTimer = new VbaTimer;
        m_pImpl->m_aTimerHash[ aTimerIndex ].reset(pTimer);
        pTimer->Start( this, aFunction, nEarliestTime, nLatestTime );
    }
}

float SAL_CALL VbaApplicationBase::CentimetersToPoints( float Centimeters )
{
    // i cm = 28.35 points
    static const float rate = 28.35f;
    return ( Centimeters * rate );
}

uno::Any SAL_CALL VbaApplicationBase::getVBE()
{
    try // return empty object on error
    {
        // "VBE" object does not have a parent, but pass document model to be able to determine application type
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[ 0 ] <<= getCurrentDocument();
        uno::Reference< lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_SET_THROW );
        uno::Reference< uno::XInterface > xVBE = xServiceManager->createInstanceWithArgumentsAndContext(
            "ooo.vba.vbide.VBE" , aArgs, mxContext );
        return uno::Any( xVBE );
    }
    catch( const uno::Exception& )
    {
    }
    return uno::Any();
}

OUString
VbaApplicationBase::getServiceImplName()
{
    return "VbaApplicationBase";
}

uno::Sequence<OUString>
VbaApplicationBase::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.VbaApplicationBase"
    };
    return aServiceNames;
}

void SAL_CALL VbaApplicationBase::Undo()
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    dispatchRequests( xModel, ".uno:Undo" );
}

void VbaApplicationBase::Quit()
{
    // need to stop basic
    SbMethod* pMeth = StarBASIC::GetActiveMethod();
    if ( pMeth )
    {
        SbModule* pMod = dynamic_cast< SbModule* >( pMeth->GetParent() );
        if ( pMod )
        {
            StarBASIC* pBasic = dynamic_cast< StarBASIC* >( pMod->GetParent() );
            if ( pBasic )
                pBasic->QuitAndExitApplication();
        }
    }
    else
    {
        // This is the case of a call from an (OLE) Automation client.

        // When an Automation client itself asks the process to quit, it should obey it.
        AsyncQuitHandler::instance().SetForceQuit();

        // TODO: Probably we should just close any document windows open by the "application"
        // (Writer or Calc) the call being handled is for. And only then, if no document windows
        // are left open, quit the actual LibreOffice application.

        Application::PostUserEvent( LINK( &AsyncQuitHandler::instance(), AsyncQuitHandler, OnAsyncQuit ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
