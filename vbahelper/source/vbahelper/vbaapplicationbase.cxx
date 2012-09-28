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

#include "vbahelper/vbaapplicationbase.hxx"
#include <sal/macros.h>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/awt/XWindow2.hpp>

#include <boost/unordered_map.hpp>
#include <filter/msfilter/msvbahelper.hxx>
#include <tools/datetime.hxx>

#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbuno.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/vbahelper.hxx>

#include "vbacommandbars.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define OFFICEVERSION "11.0"

// ====VbaTimerInfo==================================
typedef ::std::pair< OUString, ::std::pair< double, double > > VbaTimerInfo;

// ====VbaTimer==================================
class VbaTimer
{
    Timer m_aTimer;
    VbaTimerInfo m_aTimerInfo;
    ::rtl::Reference< VbaApplicationBase > m_xBase;

    // the following declarations are here to prevent the usage of them
    VbaTimer( const VbaTimer& );
    VbaTimer& operator=( const VbaTimer& );

public:
    VbaTimer()
    {}

    virtual ~VbaTimer()
    {
        m_aTimer.Stop();
    }

    static double GetNow()
    {
        Date aDateNow( Date::SYSTEM );
        Time aTimeNow( Time::SYSTEM );
         Date aRefDate( 1,1,1900 );
        long nDiffDays = (long)(aDateNow - aRefDate);
        nDiffDays += 2; // Change VisualBasic: 1.Jan.1900 == 2

        long nDiffSeconds = aTimeNow.GetHour() * 3600 + aTimeNow.GetMin() * 60 + aTimeNow.GetSec();
        return (double)nDiffDays + ((double)nDiffSeconds)/(double)(24*3600);
    }

    static sal_Int32 GetTimerMiliseconds( double nFrom, double nTo )
    {
        double nResult = nTo - nFrom;
        if ( nResult > 0 )
            nResult *= 24*3600*1000;
        else
            nResult = 50;

        return (sal_Int32) nResult;
    }

    void Start( const ::rtl::Reference< VbaApplicationBase > xBase, const OUString& aFunction, double nFrom, double nTo )
    {
        if ( !xBase.is() || aFunction.isEmpty() )
            throw uno::RuntimeException( "Unexpected arguments!" , uno::Reference< uno::XInterface >() );

        m_xBase = xBase;
        m_aTimerInfo = VbaTimerInfo( aFunction, ::std::pair< double, double >( nFrom, nTo ) );
        m_aTimer.SetTimeoutHdl( LINK( this, VbaTimer, MacroCallHdl ) );
        m_aTimer.SetTimeout( GetTimerMiliseconds( GetNow(), nFrom ) );
        m_aTimer.Start();
    }

    DECL_LINK( MacroCallHdl, void* );
};

IMPL_LINK_NOARG(VbaTimer, MacroCallHdl)
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

    // mast be the last call in the method since it deletes the timer
    try
    {
        m_xBase->OnTime( uno::makeAny( m_aTimerInfo.second.first ), m_aTimerInfo.first, uno::makeAny( m_aTimerInfo.second.second ), uno::makeAny( sal_False ) );
    } catch( uno::Exception& )
    {}

    return 0;
}

// ====VbaTimerInfoHash==================================
struct VbaTimerInfoHash
{
    size_t operator()( const VbaTimerInfo& rTimerInfo ) const
    {
        return (size_t)rTimerInfo.first.hashCode()
             + (size_t)rtl_str_hashCode_WithLength( (char*)&rTimerInfo.second.first, sizeof( double ) )
             + (size_t)rtl_str_hashCode_WithLength( (char*)&rTimerInfo.second.second, sizeof( double ) );
    }
};

// ====VbaTimerHashMap==================================
typedef ::boost::unordered_map< VbaTimerInfo, VbaTimer*, VbaTimerInfoHash, ::std::equal_to< VbaTimerInfo > > VbaTimerHashMap;

// ====VbaApplicationBase_Impl==================================
struct VbaApplicationBase_Impl
{
    VbaTimerHashMap m_aTimerHash;
    sal_Bool mbVisible;

    inline VbaApplicationBase_Impl() : mbVisible( sal_True ) {}

    virtual ~VbaApplicationBase_Impl()
    {
        // remove the remaining timers
        for ( VbaTimerHashMap::iterator aIter = m_aTimerHash.begin();
              aIter != m_aTimerHash.end();
              ++aIter )
        {
            delete aIter->second;
            aIter->second = NULL;
        }
    }
};

// ====VbaApplicationBase==================================
VbaApplicationBase::VbaApplicationBase( const uno::Reference< uno::XComponentContext >& xContext )
                    : ApplicationBase_BASE( uno::Reference< XHelperInterface >(), xContext )
                    , m_pImpl( new VbaApplicationBase_Impl )
{
}

VbaApplicationBase::~VbaApplicationBase()
{
    delete m_pImpl;
}

sal_Bool SAL_CALL
VbaApplicationBase::getScreenUpdating() throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    return !xModel->hasControllersLocked();
}

void SAL_CALL
VbaApplicationBase::setScreenUpdating(sal_Bool bUpdate) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    // #163808# use helper from module "basic" to lock all documents of this application
    ::basic::vba::lockControllersOfAllDocuments( xModel, !bUpdate );
}

sal_Bool SAL_CALL
VbaApplicationBase::getDisplayStatusBar() throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );

    if( xProps.is() ){
        uno::Reference< frame::XLayoutManager > xLayoutManager( xProps->getPropertyValue( "LayoutManager"), uno::UNO_QUERY_THROW );
        OUString url( "private:resource/statusbar/statusbar" );
        if( xLayoutManager.is() && xLayoutManager->isElementVisible( url ) ){
            return sal_True;
        }
    }
    return sal_False;
}

void SAL_CALL
VbaApplicationBase::setDisplayStatusBar(sal_Bool bDisplayStatusBar) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );

    if( xProps.is() ){
        uno::Reference< frame::XLayoutManager > xLayoutManager( xProps->getPropertyValue( "LayoutManager" ), uno::UNO_QUERY_THROW );
        OUString url( "private:resource/statusbar/statusbar" );
        if( xLayoutManager.is() ){
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
    }
    return;
}

::sal_Bool SAL_CALL VbaApplicationBase::getInteractive()
    throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< awt::XWindow2 > xWindow( xFrame->getContainerWindow(), uno::UNO_QUERY_THROW );

    return xWindow->isEnabled();
}

void SAL_CALL VbaApplicationBase::setInteractive( ::sal_Bool bInteractive )
    throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    // #163808# use helper from module "basic" to enable/disable all container windows of all documents of this application
    ::basic::vba::enableContainerWindowsOfAllDocuments( xModel, bInteractive );
}

sal_Bool SAL_CALL VbaApplicationBase::getVisible() throw (uno::RuntimeException)
{
    return m_pImpl->mbVisible;    // dummy implementation
}

void SAL_CALL VbaApplicationBase::setVisible( sal_Bool bVisible ) throw (uno::RuntimeException)
{
    m_pImpl->mbVisible = bVisible;  // dummy implementation
}


void SAL_CALL
VbaApplicationBase::OnKey( const OUString& Key, const uno::Any& Procedure ) throw (uno::RuntimeException)
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
VbaApplicationBase::CommandBars( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< XCommandBars > xCommandBars( new ScVbaCommandBars( this, mxContext, uno::Reference< container::XIndexAccess >(), getCurrentDocument() ) );
    if( aIndex.hasValue() )
        return uno::makeAny( xCommandBars->Item( aIndex, uno::Any() ) );
    return uno::makeAny( xCommandBars );
}

OUString SAL_CALL
VbaApplicationBase::getVersion() throw (uno::RuntimeException)
{
    return OUString(OFFICEVERSION);
}

uno::Any SAL_CALL VbaApplicationBase::Run( const OUString& MacroName, const uno::Any& varg1, const uno::Any& varg2, const uno::Any& varg3, const uno::Any& varg4, const uno::Any& varg5, const uno::Any& varg6, const uno::Any& varg7, const uno::Any& varg8, const uno::Any& varg9, const uno::Any& varg10, const uno::Any& varg11, const uno::Any& varg12, const uno::Any& varg13, const uno::Any& varg14, const uno::Any& varg15, const uno::Any& varg16, const uno::Any& varg17, const uno::Any& varg18, const uno::Any& varg19, const uno::Any& varg20, const uno::Any& varg21, const uno::Any& varg22, const uno::Any& varg23, const uno::Any& varg24, const uno::Any& varg25, const uno::Any& varg26, const uno::Any& varg27, const uno::Any& varg28, const uno::Any& varg29, const uno::Any& varg30 ) throw (uno::RuntimeException)
{
    OUString aMacroName = MacroName.trim();
    if (0 == aMacroName.indexOf('!'))
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
    if( aMacroInfo.mbFound )
    {
        // handle the arguments
        const uno::Any* aArgsPtrArray[] = { &varg1, &varg2, &varg3, &varg4, &varg5, &varg6, &varg7, &varg8, &varg9, &varg10, &varg11, &varg12, &varg13, &varg14, &varg15, &varg16, &varg17, &varg18, &varg19, &varg20, &varg21, &varg22, &varg23, &varg24, &varg25, &varg26, &varg27, &varg28, &varg29, &varg30 };

        int nArg = sizeof( aArgsPtrArray ) / sizeof( aArgsPtrArray[0] );
        uno::Sequence< uno::Any > aArgs( nArg );

        const uno::Any** pArg = aArgsPtrArray;
        const uno::Any** pArgEnd = ( aArgsPtrArray + nArg );

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
    else
    {
        throw uno::RuntimeException( "The macro doesn't exist" , uno::Reference< uno::XInterface >() );
    }
}

void SAL_CALL VbaApplicationBase::OnTime( const uno::Any& aEarliestTime, const OUString& aFunction, const uno::Any& aLatestTime, const uno::Any& aSchedule )
    throw ( uno::RuntimeException )
{
    if ( aFunction.isEmpty() )
        throw uno::RuntimeException( "Unexpected function name!" , uno::Reference< uno::XInterface >() );

    double nEarliestTime = 0;
    double nLatestTime = 0;
    if ( !( aEarliestTime >>= nEarliestTime )
      || ( aLatestTime.hasValue() && !( aLatestTime >>= nLatestTime ) ) )
        throw uno::RuntimeException( "Only double is supported as time for now!" , uno::Reference< uno::XInterface >() );

    sal_Bool bSetTimer = sal_True;
    aSchedule >>= bSetTimer;

    VbaTimerInfo aTimerIndex( aFunction, ::std::pair< double, double >( nEarliestTime, nLatestTime ) );

    VbaTimerHashMap::iterator aIter = m_pImpl->m_aTimerHash.find( aTimerIndex );
    if ( aIter != m_pImpl->m_aTimerHash.end() )
    {
        delete aIter->second;
        aIter->second = NULL;
        m_pImpl->m_aTimerHash.erase( aIter );
    }

    if ( bSetTimer )
    {
        VbaTimer* pTimer = new VbaTimer;
        m_pImpl->m_aTimerHash[ aTimerIndex ] = pTimer;
        pTimer->Start( this, aFunction, nEarliestTime, nLatestTime );
    }
}

float SAL_CALL VbaApplicationBase::CentimetersToPoints( float _Centimeters ) throw (uno::RuntimeException)
{
    // i cm = 28.35 points
    static const float rate = 28.35f;
    return ( _Centimeters * rate );
}

uno::Any SAL_CALL VbaApplicationBase::getVBE() throw (uno::RuntimeException)
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
    return OUString("VbaApplicationBase");
}

uno::Sequence<OUString>
VbaApplicationBase::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.VbaApplicationBase";
    }
    return aServiceNames;
}

void SAL_CALL VbaApplicationBase::Undo()
    throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    dispatchRequests( xModel, ".uno:Undo" );
}

void VbaApplicationBase::Quit() throw (uno::RuntimeException)
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
