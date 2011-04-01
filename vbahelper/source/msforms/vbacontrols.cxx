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

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>

#include "vbacontrols.hxx"
#include "vbacontrol.hxx"
#include <cppuhelper/implbase2.hxx>
#include <ooo/vba/XControlProvider.hpp>
#include <hash_map>

using namespace com::sun::star;
using namespace ooo::vba;


typedef ::cppu::WeakImplHelper2< container::XNameAccess, container::XIndexAccess > ArrayWrapImpl;

typedef  std::hash_map< rtl::OUString, sal_Int32, ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString >  > ControlIndexMap;
typedef  std::vector< uno::Reference< awt::XControl > > ControlVec;

class ControlArrayWrapper : public ArrayWrapImpl
{
    uno::Reference< awt::XControlContainer > mxDialog;
    uno::Sequence< ::rtl::OUString > msNames;
    ControlVec mControls;
    ControlIndexMap mIndices;

private:
    void SetArrayElementTo( const uno::Reference< awt::XControl >& xCtrl, sal_Int32 nIndex = -1 )
    {
        // initialize the element with specified index to the control
        if ( xCtrl.is() )
        {
            if ( nIndex == -1 )
                nIndex = msNames.getLength();

            if ( nIndex >= msNames.getLength() )
                msNames.realloc( nIndex );

            msNames[ nIndex ] = getControlName( xCtrl );
            mControls.push_back( xCtrl );
            mIndices[ msNames[ nIndex ] ] = nIndex;
        }
    }

public:
    ControlArrayWrapper( const uno::Reference< awt::XControl >& xDialog )
    {
        try
        {
            mxDialog.set( xDialog, uno::UNO_QUERY_THROW );
            uno::Sequence< uno::Reference< awt::XControl > > sXControls = mxDialog->getControls();

            msNames.realloc( sXControls.getLength() );
            for ( sal_Int32 i = 0; i < sXControls.getLength(); ++i )
                SetArrayElementTo( sXControls[ i ], i );
        }
        catch( uno::Exception& )
        {
            // accept the case when the dialog already does not exist
            // in this case the wrapper should work in dummy mode
        }
    }

    static rtl::OUString getControlName( const uno::Reference< awt::XControl >& xCtrl )
    {
        if ( !xCtrl.is() )
            throw uno::RuntimeException();

        uno::Reference< beans::XPropertySet > xProp( xCtrl->getModel(), uno::UNO_QUERY_THROW );
        rtl::OUString sName;
        xProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ) ) >>= sName;
        return sName;
    }


    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return awt::XControl::static_type(0);
    }

    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return ( mControls.size() > 0 );
    }

    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        return getByIndex( mIndices[ aName ] );
    }

    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        return msNames;
    }

    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException)
    {
        ControlIndexMap::iterator it = mIndices.find( aName );
        return it != mIndices.end();
    }

    // XElementAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (css::uno::RuntimeException)
    {
        return mControls.size();
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
    {
        if ( Index < 0 || Index >= static_cast< sal_Int32 >( mControls.size() ) )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( mControls[ Index ] );
    }
};


class ControlsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<XHelperInterface > m_xParent;
    uno::Reference<uno::XComponentContext > m_xContext;
    uno::Reference<container::XIndexAccess > m_xIndexAccess;
    uno::Reference<awt::XControl > m_xDlg;
    uno::Reference< frame::XModel > m_xModel;
    double mfOffsetX;
    double mfOffsetY;
    sal_Int32 nIndex;

public:

    ControlsEnumWrapper(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< container::XIndexAccess >& xIndexAccess,
        const uno::Reference< awt::XControl >& xDlg,
        const uno::Reference< frame::XModel >& xModel,
        double fOffsetX, double fOffsetY ) :
    m_xParent( xParent ),
    m_xContext( xContext),
    m_xIndexAccess( xIndexAccess ),
    m_xDlg( xDlg ),
    m_xModel( xModel ),
    mfOffsetX( fOffsetX ),
    mfOffsetY( fOffsetY ),
    nIndex( 0 ) {}

    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex < m_xIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( nIndex < m_xIndexAccess->getCount() )
        {
            uno::Reference< awt::XControl > xControl;
            m_xIndexAccess->getByIndex( nIndex++ ) >>= xControl;

            uno::Reference< msforms::XControl > xVBAControl;
            if ( xControl.is() && m_xDlg.is() )
                xVBAControl = ScVbaControlFactory::createUserformControl( m_xContext, xControl, m_xDlg, m_xModel, mfOffsetX, mfOffsetY );
            return uno::makeAny( xVBAControl );
        }
        throw container::NoSuchElementException();
    }

};


uno::Reference<container::XIndexAccess >
lcl_controlsWrapper( const uno::Reference< awt::XControl >& xDlg )
{
    return new ControlArrayWrapper( xDlg );
}

ScVbaControls::ScVbaControls(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const css::uno::Reference< awt::XControl >& xDialog,
        const uno::Reference< frame::XModel >& xModel,
        double fOffsetX, double fOffsetY ) :
    ControlsImpl_BASE( xParent, xContext, lcl_controlsWrapper( xDialog  ) ),
    mxDialog( xDialog ),
    mxModel( xModel ),
    mfOffsetX( fOffsetX ),
    mfOffsetY( fOffsetY )
{
}

uno::Reference< container::XEnumeration >
ScVbaControls::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumeration > xEnum( new ControlsEnumWrapper( mxParent, mxContext, m_xIndexAccess, mxDialog, mxModel, mfOffsetX, mfOffsetY ) );
    if ( !xEnum.is() )
        throw uno::RuntimeException();
    return xEnum;
}

uno::Any
ScVbaControls::createCollectionObject( const css::uno::Any& aSource )
{
    // Create control from awt::XControl
    uno::Reference< awt::XControl > xControl( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< msforms::XControl > xVBAControl = ScVbaControlFactory::createUserformControl( mxContext, xControl, mxDialog, mxModel, mfOffsetX, mfOffsetY );
    return uno::Any( xVBAControl );
}

void SAL_CALL
ScVbaControls::Move( double cx, double cy ) throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumeration > xEnum( createEnumeration() );
    while ( xEnum->hasMoreElements() )
    {
        uno::Reference< msforms::XControl > xControl( xEnum->nextElement(), uno::UNO_QUERY_THROW );
        xControl->setLeft( xControl->getLeft() + cx );
        xControl->setTop( xControl->getTop() + cy );
    }
}

uno::Any SAL_CALL ScVbaControls::Add( const uno::Any& Object, const uno::Any& StringKey, const uno::Any& /*Before*/, const uno::Any& /*After*/ )
    throw (uno::RuntimeException)
{
    uno::Any aResult;
    ::rtl::OUString aComServiceName;

    try
    {
        if ( !mxDialog.is() )
            throw uno::RuntimeException();

        uno::Reference< awt::XControl > xNewControl;
        uno::Reference< lang::XMultiServiceFactory > xModelFactory( mxDialog->getModel(), uno::UNO_QUERY_THROW );

        uno::Reference< container::XNameContainer > xDialogContainer( xModelFactory, uno::UNO_QUERY_THROW );

        Object >>= aComServiceName;

        // TODO: Support Before and After?
        ::rtl::OUString aNewName;
        StringKey >>= aNewName;
        if ( !aNewName.getLength() )
        {
            aNewName = aComServiceName;
            if ( !aNewName.getLength() )
                aNewName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Control" ) );

            sal_Int32 nInd = 0;
            while( xDialogContainer->hasByName( aNewName ) && (nInd < SAL_MAX_INT32) )
            {
                aNewName = aComServiceName;
                aNewName += ::rtl::OUString::valueOf( nInd++ );
            }
        }

        double fDefWidth = 72.0, fDefHeight = 18.0;
        if ( aComServiceName.getLength() )
        {
            // create a UNO control model based on the passed control type
            uno::Reference< awt::XControlModel > xNewModel;
            bool bFontSupport = false;
            bool bNativeAX = false;
            if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.CommandButton.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlButtonModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 24.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.Label.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedTextModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.Image.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlImageControlModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 72.0;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.CheckBox.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlCheckBoxModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 108.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.OptionButton.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlRadioButtonModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 108.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.TextBox.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlEditModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.ListBox.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlListBoxModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.ComboBox.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlComboBoxModel" ) ) ), uno::UNO_QUERY_THROW );
                uno::Reference< beans::XPropertySet > xProps( xNewModel, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Dropdown" ) ), uno::Any( true ) );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.ToggleButton.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlButtonModel" ) ) ), uno::UNO_QUERY_THROW );
                uno::Reference< beans::XPropertySet > xProps( xNewModel, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Toggle" ) ), uno::Any( true ) );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.Frame.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlGroupBoxModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 216.0; fDefHeight = 144.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.SpinButton.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlSpinButtonModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 12.75; fDefHeight = 25.5;
            }
            else if( aComServiceName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Forms.ScrollBar.1" ) ) )
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlScrollBarModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 12.75; fDefHeight = 63.8;
            }
            else
            {
                xNewModel.set( xModelFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.custom.awt.UnoControlSystemAXContainerModel" ) ) ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bNativeAX = true;
            }

            // need to set a few font properties to get rid of the default DONT_KNOW values
            if( bFontSupport )
            {
                uno::Reference< beans::XPropertySet > xModelProps( xNewModel, uno::UNO_QUERY_THROW );
                xModelProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontName" ) ), uno::Any( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Tahoma" ) ) ) );
                xModelProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontHeight" ) ), uno::Any( float( 8.0 ) ) );
                xModelProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontWeight" ) ), uno::Any( awt::FontWeight::NORMAL ) );
                xModelProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontSlant" ) ), uno::Any( awt::FontSlant_NONE ) );
                xModelProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontUnderline" ) ), uno::Any( awt::FontUnderline::NONE ) );
                xModelProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontStrikeout" ) ), uno::Any( awt::FontStrikeout::NONE ) );
            }

            xDialogContainer->insertByName( aNewName, uno::makeAny( xNewModel ) );
            uno::Reference< awt::XControlContainer > xControlContainer( mxDialog, uno::UNO_QUERY_THROW );
            xNewControl = xControlContainer->getControl( aNewName );

            if( bNativeAX ) try
            {
                uno::Reference< script::XInvocation > xControlInvoke( xNewControl, uno::UNO_QUERY_THROW );

                uno::Sequence< uno::Any > aArgs( 1 );
                aArgs[0] <<= aComServiceName;
                uno::Sequence< sal_Int16 > aOutIDDummy;
                uno::Sequence< uno::Any > aOutDummy;
                xControlInvoke->invoke( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SOAddAXControl" ) ), aArgs, aOutIDDummy, aOutDummy );
            }
            catch( uno::Exception& )
            {
                xDialogContainer->removeByName( aNewName );
                throw;
            }
        }

        if ( xNewControl.is() )
        {
            UpdateCollectionIndex( lcl_controlsWrapper( mxDialog  ) );
            aResult <<= xNewControl;
            aResult = createCollectionObject( aResult );
            uno::Reference< msforms::XControl > xVBAControl( aResult, uno::UNO_QUERY_THROW );
            if( fDefWidth > 0.0 )
                xVBAControl->setWidth( fDefWidth );
            if( fDefHeight > 0.0 )
                xVBAControl->setHeight( fDefHeight );
        }
        else
            throw uno::RuntimeException();
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& e )
    {
        throw lang::WrappedTargetException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Can not create AXControl!" ) ),
                uno::Reference< uno::XInterface >(),
                uno::makeAny( e ) );
    }

    return aResult;
}

void SAL_CALL ScVbaControls::Remove( const uno::Any& StringKeyOrIndex )
    throw (uno::RuntimeException)
{
    ::rtl::OUString aControlName;
    sal_Int32 nIndex = -1;

    try
    {
        if ( !mxDialog.is() )
            throw uno::RuntimeException();

        uno::Reference< lang::XMultiServiceFactory > xModelFactory( mxDialog->getModel(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameContainer > xDialogContainer( xModelFactory, uno::UNO_QUERY_THROW );

        if ( !( ( StringKeyOrIndex >>= aControlName ) && aControlName.getLength() )
          && !( ( StringKeyOrIndex >>= nIndex ) && nIndex >= 0 && nIndex < m_xIndexAccess->getCount() ) )
            throw uno::RuntimeException();

        uno::Reference< awt::XControl > xControl;
        if ( aControlName.getLength() )
        {
            uno::Reference< awt::XControlContainer > xControlContainer( mxDialog, uno::UNO_QUERY_THROW );
            xControl = xControlContainer->getControl( aControlName );
        }
        else
        {
            m_xIndexAccess->getByIndex( nIndex ) >>= xControl;
        }

        if ( !xControl.is() )
            throw uno::RuntimeException();

        if ( !aControlName.getLength() )
            aControlName = ControlArrayWrapper::getControlName( xControl );

        xDialogContainer->removeByName( aControlName );
        xControl->dispose();
    }
    catch( uno::RuntimeException& )
    {
        // the exceptions are not rethrown, impossibility to find or remove the control is currently not reported
        // since in most cases it means just that the controls is already not there, the VBA seems to do it in the same way

        // throw;
    }
    catch( uno::Exception& e )
    {
        // throw lang::WrappedTargetException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Can not create AXControl!" ) ),
        //         uno::Reference< uno::XInterface >(),
        //         uno::makeAny( e ) );
    }
}


uno::Type
ScVbaControls::getElementType() throw (uno::RuntimeException)
{
    return ooo::vba::msforms::XControl::static_type(0);
}

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaControls, "ooo.vba.msforms.Controls" )
