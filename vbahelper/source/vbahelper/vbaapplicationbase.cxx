/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "vbahelper/vbaapplicationbase.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include "vbacommandbars.hxx"
#include <svx/msvbahelper.hxx>

// start basic includes
#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbuno.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
// end basic includes
using namespace com::sun::star;
using namespace ooo::vba;

#define OFFICEVERSION "11.0"

VbaApplicationBase::VbaApplicationBase( const uno::Reference< uno::XComponentContext >& xContext )
                    : ApplicationBase_BASE( uno::Reference< XHelperInterface >(), xContext )
{
}

VbaApplicationBase::~VbaApplicationBase()
{
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

void SAL_CALL VbaApplicationBase::Run( const ::rtl::OUString& MacroName, const uno::Any& varg1, const uno::Any& varg2, const uno::Any& varg3, const uno::Any& varg4, const uno::Any& varg5, const uno::Any& varg6, const uno::Any& varg7, const uno::Any& varg8, const uno::Any& varg9, const uno::Any& varg10, const uno::Any& varg11, const uno::Any& varg12, const uno::Any& varg13, const uno::Any& varg14, const uno::Any& varg15, const uno::Any& varg16, const uno::Any& varg17, const uno::Any& varg18, const uno::Any& varg19, const uno::Any& varg20, const uno::Any& varg21, const uno::Any& varg22, const uno::Any& varg23, const uno::Any& varg24, const uno::Any& varg25, const uno::Any& varg26, const uno::Any& varg27, const uno::Any& varg28, const uno::Any& varg29, const uno::Any& varg30 ) throw (uno::RuntimeException)
{
    VBAMacroResolvedInfo aMacroInfo = resolveVBAMacro( getSfxObjShell( getCurrentDocument() ), MacroName );
    if( aMacroInfo.IsResolved() )
    {
        // handle the arguments
        const uno::Any* aArgsPtrArray[] = { &varg1, &varg2, &varg3, &varg4, &varg5, &varg6, &varg7, &varg8, &varg9, &varg10, &varg11, &varg12, &varg13, &varg14, &varg15, &varg16, &varg17, &varg18, &varg19, &varg20, &varg21, &varg22, &varg23, &varg24, &varg25, &varg26, &varg27, &varg28, &varg29, &varg30 };

        int nArg = sizeof( aArgsPtrArray ) / sizeof( aArgsPtrArray[0] );
        uno::Sequence< uno::Any > aArgs( nArg );

        const uno::Any** pArg = aArgsPtrArray;
        const uno::Any** pArgEnd = ( aArgsPtrArray + nArg );

        sal_Int32 nLastArgWithValue = 0;
        sal_Int32 nArgProcessed = 0;

        for ( ; pArg != pArgEnd; ++pArg, ++nArgProcessed )
        {
            aArgs[ nArgProcessed ] =  **pArg;
            if( (*pArg)->hasValue() )
                nLastArgWithValue = nArgProcessed;
        }

        // resize array to position of last param with value
        aArgs.realloc( nArgProcessed + 1 );

        uno::Any aRet;
        uno::Any aDummyCaller;
        executeMacro( aMacroInfo.MacroDocContext(), aMacroInfo.ResolvedMacro(), aArgs, aRet, aDummyCaller );
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("The macro doesn't exist") ), uno::Reference< uno::XInterface >() );
    }
}

float SAL_CALL VbaApplicationBase::CentimetersToPoints( float _Centimeters ) throw (uno::RuntimeException)
{
    // i cm = 28.35 points
    static const float rate = 28.35f;
    return ( _Centimeters * rate );
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
