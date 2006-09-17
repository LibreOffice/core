/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chinese_translation_unodialog.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:08:56 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "chinese_translation_unodialog.hxx"
#include "chinese_translationdialog.hxx"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
// header for define RET_CANCEL
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

// header for class OImplementationId
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif

//.............................................................................
namespace textconversiondlgs
{
//.............................................................................
using namespace ::com::sun::star;

#define SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.linguistic2.ChineseTranslationDialog")
#define SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.linguistic2.ChineseTranslationDialog")

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)

ChineseTranslation_UnoDialog::ChineseTranslation_UnoDialog( const uno::Reference< uno::XComponentContext >& xContext )
                    : m_xCC( xContext )
                    , m_xParentWindow( 0 )
                    , m_pDialog( 0 )
                    , m_bDisposed(sal_False)
                    , m_bInDispose(sal_False)
                    , m_aContainerMutex()
                    , m_aDisposeEventListeners(m_aContainerMutex)
{
}

ChineseTranslation_UnoDialog::~ChineseTranslation_UnoDialog()
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    impl_DeleteDialog();
}

void ChineseTranslation_UnoDialog::impl_DeleteDialog()
{
    if( m_pDialog )
    {
        if(m_pDialog->IsInExecute())
            m_pDialog->EndDialog(RET_CANCEL);
        delete m_pDialog;
        m_pDialog = 0;
    }
}
//-------------------------------------------------------------------------
// lang::XServiceInfo

::rtl::OUString SAL_CALL ChineseTranslation_UnoDialog::getImplementationName() throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

::rtl::OUString ChineseTranslation_UnoDialog::getImplementationName_Static()
{
    return SERVICE_IMPLEMENTATION_NAME;
}

sal_Bool SAL_CALL ChineseTranslation_UnoDialog::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString* pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
    {
        if( pArray[ i ] == ServiceName )
            return sal_True;
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChineseTranslation_UnoDialog::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< rtl::OUString > ChineseTranslation_UnoDialog::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = SERVICE_NAME;
    return aSNS;
}

//-------------------------------------------------------------------------
// ui::dialogs::XExecutableDialog

void SAL_CALL ChineseTranslation_UnoDialog::setTitle( const ::rtl::OUString& _rTitle ) throw(uno::RuntimeException)
{
    //not implemented - fell free to do so, if you do need this
}

//-------------------------------------------------------------------------
void SAL_CALL ChineseTranslation_UnoDialog::initialize( const uno::Sequence< uno::Any >& aArguments ) throw(uno::Exception, uno::RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    if( m_bDisposed || m_bInDispose )
        return;

    const uno::Any* pArguments = aArguments.getConstArray();
    for(sal_Int32 i=0; i<aArguments.getLength(); ++i, ++pArguments)
    {
        beans::PropertyValue aProperty;
        if(*pArguments >>= aProperty)
        {
            if( aProperty.Name.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "ParentWindow" ) ) == 0 )
            {
                aProperty.Value >>= m_xParentWindow;
            }
        }
    }
}

//-------------------------------------------------------------------------
sal_Int16 SAL_CALL ChineseTranslation_UnoDialog::execute() throw(uno::RuntimeException)
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        if( m_bDisposed || m_bInDispose )
            return nRet;

        if( !m_pDialog )
        {
            Window* pParent = NULL;
            if( m_xParentWindow.is() )
            {
                VCLXWindow* pImplementation = VCLXWindow::GetImplementation(m_xParentWindow);
                if (pImplementation)
                    pParent = pImplementation->GetWindow();
            }
            uno::Reference< XComponent > xComp( this );
            m_pDialog = new ChineseTranslationDialog( pParent );
        }
        if( !m_pDialog )
            return nRet;
        nRet = m_pDialog->Execute();
        if(nRet==RET_OK)
           nRet=ui::dialogs::ExecutableDialogResults::OK;
    }
    return nRet;
}

//-------------------------------------------------------------------------
// lang::XComponent

void SAL_CALL ChineseTranslation_UnoDialog::dispose() throw (uno::RuntimeException)
{
    lang::EventObject aEvt;
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        if( m_bDisposed || m_bInDispose )
            return;
        m_bInDispose = true;

        impl_DeleteDialog();
        m_xParentWindow = 0;
        m_bDisposed = true;

        aEvt.Source = static_cast< XComponent * >( this );
    }
    if( m_aDisposeEventListeners.getLength() )
        m_aDisposeEventListeners.disposeAndClear( aEvt );
}

void SAL_CALL ChineseTranslation_UnoDialog::addEventListener( const uno::Reference< lang::XEventListener > & xListener ) throw (uno::RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    if( m_bDisposed || m_bInDispose )
        return;
    m_aDisposeEventListeners.addInterface( xListener );
}

void SAL_CALL ChineseTranslation_UnoDialog::removeEventListener( const uno::Reference< lang::XEventListener > & xListener ) throw (uno::RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    if( m_bDisposed || m_bInDispose )
        return;
    m_aDisposeEventListeners.removeInterface( xListener );
}

//-------------------------------------------------------------------------
// XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL ChineseTranslation_UnoDialog::getPropertySetInfo(  ) throw (uno::RuntimeException)
{
    return 0;
}
void SAL_CALL ChineseTranslation_UnoDialog::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only read only properties
    throw beans::PropertyVetoException();
}
uno::Any SAL_CALL ChineseTranslation_UnoDialog::getPropertyValue( const ::rtl::OUString& rPropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;

    sal_Bool bDirectionToSimplified = sal_True;
    sal_Bool bUseCharacterVariants = sal_False;
    sal_Bool bTranslateCommonTerms = sal_False;

    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        if( m_bDisposed || m_bInDispose || !m_pDialog )
            return aRet;
        m_pDialog->getSettings( bDirectionToSimplified, bUseCharacterVariants, bTranslateCommonTerms );
    }

    if( rPropertyName.equals( C2U("IsDirectionToSimplified") ) )
    {
        aRet <<= bDirectionToSimplified;
    }
    else if( rPropertyName.equals( C2U("IsUseCharacterVariants") ) )
    {
        aRet <<= bUseCharacterVariants;
    }
    else if( rPropertyName.equals( C2U("IsTranslateCommonTerms") ) )
    {
        aRet <<= bTranslateCommonTerms;
    }
    else
    {
        throw beans::UnknownPropertyException();
    }
    return aRet;

}
void SAL_CALL ChineseTranslation_UnoDialog::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only not bound properties -> ignore listener
}
void SAL_CALL ChineseTranslation_UnoDialog::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only not bound properties -> ignore listener
}
void SAL_CALL ChineseTranslation_UnoDialog::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only not bound properties -> ignore listener
}
void SAL_CALL ChineseTranslation_UnoDialog::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only not bound properties -> ignore listener
}

//.............................................................................
} //end namespace
//.............................................................................
