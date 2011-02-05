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

#include "vbacommandbars.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define OFFICEVERSION "11.0"

// ====VbaTimerInfo==================================
typedef ::std::pair< ::rtl::OUString, ::std::pair< double, double > > VbaTimerInfo;

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
        Date aDateNow;
        Time aTimeNow;
         Date aRefDate( 1,1,1900 );
        long nDiffDays = (long)(aDateNow - aRefDate);
        nDiffDays += 2; // Anpassung VisualBasic: 1.Jan.1900 == 2

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

    void Start( const ::rtl::Reference< VbaApplicationBase > xBase, const ::rtl::OUString& aFunction, double nFrom, double nTo )
    {
        if ( !xBase.is() || !aFunction.getLength() )
            throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected arguments!" ) ), uno::Reference< uno::XInterface >() );

        m_xBase = xBase;
        m_aTimerInfo = VbaTimerInfo( aFunction, ::std::pair< double, double >( nFrom, nTo ) );
        m_aTimer.SetTimeoutHdl( LINK( this, VbaTimer, MacroCallHdl ) );
        m_aTimer.SetTimeout( GetTimerMiliseconds( GetNow(), nFrom ) );
        m_aTimer.Start();
    }

    DECL_LINK( MacroCallHdl, void* );
};

IMPL_LINK( VbaTimer, MacroCallHdl, void*, EMPTYARG )
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
              aIter++ )
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
    if( bUpdate != xModel->hasControllersLocked() )
        return;
    if (bUpdate)
        xModel->unlockControllers();
    else
        xModel->lockControllers();
}

sal_Bool SAL_CALL
VbaApplicationBase::getDisplayStatusBar() throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );

    if( xProps.is() ){
        uno::Reference< frame::XLayoutManager > xLayoutManager( xProps->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LayoutManager")) ), uno::UNO_QUERY_THROW );
        rtl::OUString url(RTL_CONSTASCII_USTRINGPARAM( "private:resource/statusbar/statusbar" ));
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
        uno::Reference< frame::XLayoutManager > xLayoutManager( xProps->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LayoutManager")) ), uno::UNO_QUERY_THROW );
        rtl::OUString url(RTL_CONSTASCII_USTRINGPARAM( "private:resource/statusbar/statusbar" ));
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
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< awt::XWindow > xWindow( xFrame->getContainerWindow(), uno::UNO_SET_THROW );

    xWindow->setEnable( bInteractive );
}

sal_Bool SAL_CALL VbaApplicationBase::getVisible() throw (uno::RuntimeException)
{
    return m_pImpl->mbVisible;    // dummy implementation
}

void SAL_CALL VbaApplicationBase::setVisible( sal_Bool bVisible ) throw (uno::RuntimeException)
{
    m_pImpl->mbVisible = bVisible;  // dummy implementation
}

uno::Any SAL_CALL
VbaApplicationBase::CommandBars( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< XCommandBars > xCommandBars( new ScVbaCommandBars( this, mxContext, uno::Reference< container::XIndexAccess >(), getCurrentDocument() ) );
    if( aIndex.hasValue() )
        return uno::makeAny( xCommandBars->Item( aIndex, uno::Any() ) );
    return uno::makeAny( xCommandBars );
}

::rtl::OUString SAL_CALL
VbaApplicationBase::getVersion() throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(OFFICEVERSION));
}

uno::Any SAL_CALL VbaApplicationBase::Run( const ::rtl::OUString& MacroName, const uno::Any& varg1, const uno::Any& varg2, const uno::Any& varg3, const uno::Any& varg4, const uno::Any& varg5, const uno::Any& varg6, const uno::Any& varg7, const uno::Any& varg8, const uno::Any& varg9, const uno::Any& varg10, const uno::Any& varg11, const uno::Any& varg12, const uno::Any& varg13, const uno::Any& varg14, const uno::Any& varg15, const uno::Any& varg16, const uno::Any& varg17, const uno::Any& varg18, const uno::Any& varg19, const uno::Any& varg20, const uno::Any& varg21, const uno::Any& varg22, const uno::Any& varg23, const uno::Any& varg24, const uno::Any& varg25, const uno::Any& varg26, const uno::Any& varg27, const uno::Any& varg28, const uno::Any& varg29, const uno::Any& varg30 ) throw (uno::RuntimeException)
{
    ::rtl::OUString sSeparator(RTL_CONSTASCII_USTRINGPARAM("/"));
    ::rtl::OUString sMacroSeparator(RTL_CONSTASCII_USTRINGPARAM("!"));
    ::rtl::OUString sMacro_only_Name;
    sal_Int32 Position_MacroSeparator = MacroName.indexOf(sMacroSeparator);

    uno::Reference< frame::XModel > aMacroDocumentModel;
    if (-1 != Position_MacroSeparator)
    {
        uno::Reference< container::XEnumerationAccess > xComponentEnumAccess;
        uno::Reference< lang::XMultiComponentFactory > xServiceManager = mxContext->getServiceManager();
        try
        {
            uno::Reference< frame::XDesktop > xDesktop (xServiceManager->createInstanceWithContext( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" )),mxContext ), uno::UNO_QUERY_THROW );
            xComponentEnumAccess = xDesktop->getComponents();
        }
        catch(uno::Exception&)
        {
        }

        //rem look for the name of the document in the cmpoonents collection
        uno::Reference < container::XEnumeration > xEnum = xComponentEnumAccess->createEnumeration();

        // iterate through the collection by name
        while (xEnum->hasMoreElements())
        {
            // get the next element as a UNO Any
            uno::Any aComponentHelper = xEnum->nextElement();
            uno::Reference <frame::XModel> xDocModel( aComponentHelper, uno::UNO_QUERY_THROW );

            // get the name of the sheet from its XNamed interface
            ::rtl::OUString  aName = xDocModel->getURL();


            if (aName.match(MacroName.copy(0,Position_MacroSeparator-1),aName.lastIndexOf(sSeparator)+1))
            {
                aMacroDocumentModel = xDocModel;
                sMacro_only_Name = MacroName.copy(Position_MacroSeparator+1);
            }
        }
    }
    else
    {
        aMacroDocumentModel = getCurrentDocument();
        sMacro_only_Name = MacroName.copy(0);
    }


    // search the global tempalte
    VBAMacroResolvedInfo aMacroInfo = resolveVBAMacro( getSfxObjShell( aMacroDocumentModel ), sMacro_only_Name, sal_True );
    if( aMacroInfo.IsResolved() )
    {
        // handle the arguments
        const uno::Any* aArgsPtrArray[] = { &varg1, &varg2, &varg3, &varg4, &varg5, &varg6, &varg7, &varg8, &varg9, &varg10, &varg11, &varg12, &varg13, &varg14, &varg15, &varg16, &varg17, &varg18, &varg19, &varg20, &varg21, &varg22, &varg23, &varg24, &varg25, &varg26, &varg27, &varg28, &varg29, &varg30 };

        int nArg = SAL_N_ELEMENTS( aArgsPtrArray );
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
        executeMacro( aMacroInfo.MacroDocContext(), aMacroInfo.ResolvedMacro(), aArgs, aRet, aDummyCaller );

        return aRet;
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("The macro doesn't exist") ), uno::Reference< uno::XInterface >() );
    }
}

void SAL_CALL VbaApplicationBase::OnTime( const uno::Any& aEarliestTime, const ::rtl::OUString& aFunction, const uno::Any& aLatestTime, const uno::Any& aSchedule )
    throw ( uno::RuntimeException )
{
    if ( !aFunction.getLength() )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected function name!" ) ), uno::Reference< uno::XInterface >() );

    double nEarliestTime = 0;
    double nLatestTime = 0;
    if ( !( aEarliestTime >>= nEarliestTime )
      || ( aLatestTime.hasValue() && !( aLatestTime >>= nLatestTime ) ) )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Only double is supported as time for now!" ) ), uno::Reference< uno::XInterface >() );

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
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[ 0 ] <<= uno::Reference< XHelperInterface >( this );
        aArgs[ 1 ] <<= getCurrentDocument();
        uno::Reference< lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_SET_THROW );
        uno::Reference< uno::XInterface > xVBE = xServiceManager->createInstanceWithArgumentsAndContext(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBE" ) ), aArgs, mxContext );
        return uno::Any( xVBE );
    }
    catch( uno::Exception& )
    {
    }
    return uno::Any();
}

uno::Any SAL_CALL
VbaApplicationBase::getVBProjects() throw (uno::RuntimeException)
{
    try // return empty object on error
    {
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[ 0 ] <<= uno::Reference< XHelperInterface >( this );
        aArgs[ 1 ] <<= getCurrentDocument();
        uno::Reference< lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_SET_THROW );
        uno::Reference< uno::XInterface > xVBProjects = xServiceManager->createInstanceWithArgumentsAndContext(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBProjects" ) ), aArgs, mxContext );
        return uno::Any( xVBProjects );
    }
    catch( uno::Exception& )
    {
    }
    return uno::Any();
}

rtl::OUString&
VbaApplicationBase::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("VbaApplicationBase") );
    return sImplName;
}

uno::Sequence<rtl::OUString>
VbaApplicationBase::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.VbaApplicationBase" ) );
    }
    return aServiceNames;
}

void SAL_CALL VbaApplicationBase::Undo()
    throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    dispatchRequests( xModel, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Undo" ) ) );
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
