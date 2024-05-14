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

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "vbacontrols.hxx"
#include "vbacontrol.hxx"
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <o3tl/safeint.hxx>
#include <unordered_map>
#include <utility>

using namespace com::sun::star;
using namespace ooo::vba;


typedef  std::unordered_map< OUString, sal_Int32 > ControlIndexMap;

namespace {

class ControlArrayWrapper : public ::cppu::WeakImplHelper< container::XNameAccess, container::XIndexAccess >
{
    uno::Reference< awt::XControlContainer > mxDialog;
    uno::Sequence< OUString > msNames;
    std::vector< uno::Reference< awt::XControl > > mControls;
    ControlIndexMap mIndices;

private:
    void SetArrayElementTo( const uno::Reference< awt::XControl >& xCtrl, sal_Int32 nIndex )
    {
        // initialize the element with specified index to the control
        if ( xCtrl.is() )
        {
            if ( nIndex == -1 )
                nIndex = msNames.getLength();

            if ( nIndex >= msNames.getLength() )
                msNames.realloc( nIndex );

            msNames.getArray()[ nIndex ] = getControlName( xCtrl );
            mControls.push_back( xCtrl );
            mIndices[ msNames[ nIndex ] ] = nIndex;
        }
    }
public:
    explicit ControlArrayWrapper( const uno::Reference< awt::XControl >& xDialog )
    {
        try
        {
            mxDialog.set( xDialog, uno::UNO_QUERY_THROW );
            uno::Sequence< uno::Reference< awt::XControl > > sXControls = mxDialog->getControls();

            msNames.realloc( sXControls.getLength() );
            for ( sal_Int32 i = 0; i < sXControls.getLength(); ++i )
                SetArrayElementTo( sXControls[ i ], i );
        }
        catch (const uno::Exception&)
        {
            // accept the case when the dialog already does not exist
            // in this case the wrapper should work in dummy mode
        }
    }

    static OUString getControlName( const uno::Reference< awt::XControl >& xCtrl )
    {
        if ( !xCtrl.is() )
            throw uno::RuntimeException();

        uno::Reference< beans::XPropertySet > xProp( xCtrl->getModel(), uno::UNO_QUERY_THROW );
        OUString sName;
        xProp->getPropertyValue( u"Name"_ustr ) >>= sName;
        return sName;
    }


    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return cppu::UnoType<awt::XControl>::get();
    }

    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return ( !mControls.empty() );
    }

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        return getByIndex( mIndices[ aName ] );
    }

    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        return msNames;
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        ControlIndexMap::iterator it = mIndices.find( aName );
        return it != mIndices.end();
    }

    // XElementAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override
    {
        return mControls.size();
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        if ( Index < 0 || o3tl::make_unsigned(Index) >= mControls.size() )
            throw lang::IndexOutOfBoundsException();
        return uno::Any( mControls[ Index ] );
    }
};


class ControlsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<uno::XComponentContext > m_xContext;
    uno::Reference<container::XIndexAccess > m_xIndexAccess;
    uno::Reference<awt::XControl > m_xDlg;
    uno::Reference< frame::XModel > m_xModel;
    double mfOffsetX;
    double mfOffsetY;
    sal_Int32 nIndex;

public:

    ControlsEnumWrapper(
        uno::Reference< uno::XComponentContext > xContext,
        uno::Reference< container::XIndexAccess > xIndexAccess,
        uno::Reference< awt::XControl > xDlg,
        uno::Reference< frame::XModel > xModel,
        double fOffsetX, double fOffsetY ) :
    m_xContext(std::move( xContext)),
    m_xIndexAccess(std::move( xIndexAccess )),
    m_xDlg(std::move( xDlg )),
    m_xModel(std::move( xModel )),
    mfOffsetX( fOffsetX ),
    mfOffsetY( fOffsetY ),
    nIndex( 0 ) {}

    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( nIndex < m_xIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( nIndex < m_xIndexAccess->getCount() )
        {
            uno::Reference< awt::XControl > xControl;
            m_xIndexAccess->getByIndex( nIndex++ ) >>= xControl;

            uno::Reference< msforms::XControl > xVBAControl;
            if ( xControl.is() && m_xDlg.is() )
                xVBAControl = ScVbaControlFactory::createUserformControl( m_xContext, xControl, m_xDlg, m_xModel, mfOffsetX, mfOffsetY );
            return uno::Any( xVBAControl );
        }
        throw container::NoSuchElementException();
    }

};

}

static uno::Reference<container::XIndexAccess >
lcl_controlsWrapper( const uno::Reference< awt::XControl >& xDlg )
{
    return new ControlArrayWrapper( xDlg );
}

ScVbaControls::ScVbaControls(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const css::uno::Reference< awt::XControl >& xDialog,
        uno::Reference< frame::XModel > xModel,
        double fOffsetX, double fOffsetY ) :
    ControlsImpl_BASE( xParent, xContext, lcl_controlsWrapper( xDialog  ) ),
    mxDialog( xDialog ),
    mxModel(std::move( xModel )),
    mfOffsetX( fOffsetX ),
    mfOffsetY( fOffsetY )
{
}

uno::Reference< container::XEnumeration >
ScVbaControls::createEnumeration()
{
    uno::Reference< container::XEnumeration > xEnum( new ControlsEnumWrapper( mxContext, m_xIndexAccess, mxDialog, mxModel, mfOffsetX, mfOffsetY ) );
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
ScVbaControls::Move( double cx, double cy )
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
{
    uno::Any aResult;
    OUString aComServiceName;

    try
    {
        if ( !mxDialog.is() )
            throw uno::RuntimeException();

        uno::Reference< awt::XControl > xNewControl;
        uno::Reference< lang::XMultiServiceFactory > xModelFactory( mxDialog->getModel(), uno::UNO_QUERY_THROW );

        uno::Reference< container::XNameContainer > xDialogContainer( xModelFactory, uno::UNO_QUERY_THROW );

        Object >>= aComServiceName;

        // TODO: Support Before and After?
        OUString aNewName;
        StringKey >>= aNewName;
        if ( aNewName.isEmpty() )
        {
            aNewName = aComServiceName;
            if ( aNewName.isEmpty() )
                aNewName = "Control";

            sal_Int32 nInd = 0;
            while( xDialogContainer->hasByName( aNewName ) && (nInd < SAL_MAX_INT32) )
            {
                aNewName = aComServiceName + OUString::number( nInd++ );
            }
        }

        double fDefWidth = 72.0, fDefHeight = 18.0;
        if ( !aComServiceName.isEmpty() )
        {
            // create a UNO control model based on the passed control type
            uno::Reference< awt::XControlModel > xNewModel;
            bool bFontSupport = false;
            bool bNativeAX = false;
            if( aComServiceName.equalsIgnoreAsciiCase( "Forms.CommandButton.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlButtonModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 24.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.Label.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlFixedTextModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.Image.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlImageControlModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 72.0;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.CheckBox.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlCheckBoxModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 108.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.OptionButton.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlRadioButtonModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 108.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.TextBox.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlEditModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.ListBox.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlListBoxModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.ComboBox.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlComboBoxModel"_ustr ), uno::UNO_QUERY_THROW );
                uno::Reference< beans::XPropertySet > xProps( xNewModel, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue( u"Dropdown"_ustr , uno::Any( true ) );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.ToggleButton.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlButtonModel"_ustr ), uno::UNO_QUERY_THROW );
                uno::Reference< beans::XPropertySet > xProps( xNewModel, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue( u"Toggle"_ustr , uno::Any( true ) );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.Frame.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlGroupBoxModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 216.0; fDefHeight = 144.0;
                bFontSupport = true;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.SpinButton.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlSpinButtonModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 12.75; fDefHeight = 25.5;
            }
            else if( aComServiceName.equalsIgnoreAsciiCase( "Forms.ScrollBar.1" ) )
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.awt.UnoControlScrollBarModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 12.75; fDefHeight = 63.8;
            }
            else
            {
                xNewModel.set( xModelFactory->createInstance( u"com.sun.star.custom.awt.UnoControlSystemAXContainerModel"_ustr ), uno::UNO_QUERY_THROW );
                fDefWidth = 72.0; fDefHeight = 18.0;
                bNativeAX = true;
            }

            // need to set a few font properties to get rid of the default DONT_KNOW values
            if( bFontSupport )
            {
                uno::Reference< beans::XPropertySet > xModelProps( xNewModel, uno::UNO_QUERY_THROW );
                xModelProps->setPropertyValue( u"FontName"_ustr , uno::Any( u"Tahoma"_ustr ) );
                xModelProps->setPropertyValue( u"FontHeight"_ustr , uno::Any( float( 8.0 ) ) );
                xModelProps->setPropertyValue( u"FontWeight"_ustr , uno::Any( awt::FontWeight::NORMAL ) );
                xModelProps->setPropertyValue( u"FontSlant"_ustr , uno::Any( awt::FontSlant_NONE ) );
                xModelProps->setPropertyValue( u"FontUnderline"_ustr , uno::Any( awt::FontUnderline::NONE ) );
                xModelProps->setPropertyValue( u"FontStrikeout"_ustr , uno::Any( awt::FontStrikeout::NONE ) );
            }

            xDialogContainer->insertByName( aNewName, uno::Any( xNewModel ) );
            uno::Reference< awt::XControlContainer > xControlContainer( mxDialog, uno::UNO_QUERY_THROW );
            xNewControl = xControlContainer->getControl( aNewName );

            if( bNativeAX ) try
            {
                uno::Reference< script::XInvocation > xControlInvoke( xNewControl, uno::UNO_QUERY_THROW );

                uno::Sequence< uno::Any > aArgs{ uno::Any(aComServiceName) };
                uno::Sequence< sal_Int16 > aOutIDDummy;
                uno::Sequence< uno::Any > aOutDummy;
                xControlInvoke->invoke( u"SOAddAXControl"_ustr , aArgs, aOutIDDummy, aOutDummy );
            }
            catch (const uno::Exception&)
            {
                xDialogContainer->removeByName( aNewName );
                throw;
            }
        }

        if ( !xNewControl.is() )
            throw uno::RuntimeException();

        UpdateCollectionIndex( lcl_controlsWrapper( mxDialog  ) );
        aResult <<= xNewControl;
        aResult = createCollectionObject( aResult );
        uno::Reference< msforms::XControl > xVBAControl( aResult, uno::UNO_QUERY_THROW );
        if( fDefWidth > 0.0 )
            xVBAControl->setWidth( fDefWidth );
        if( fDefHeight > 0.0 )
            xVBAControl->setHeight( fDefHeight );
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException( u"Can not create AXControl!"_ustr,
                uno::Reference< uno::XInterface >(),
                anyEx );
    }

    return aResult;
}

void SAL_CALL ScVbaControls::Remove( const uno::Any& StringKeyOrIndex )
{
    try
    {
        OUString aControlName;
        sal_Int32 nIndex = -1;
        if ( !mxDialog.is() )
            throw uno::RuntimeException();

        uno::Reference< lang::XMultiServiceFactory > xModelFactory( mxDialog->getModel(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameContainer > xDialogContainer( xModelFactory, uno::UNO_QUERY_THROW );

        if ( StringKeyOrIndex >>= aControlName )
        {
            if ( aControlName.isEmpty() )
                throw uno::RuntimeException();
        }
        else if ( StringKeyOrIndex >>= nIndex )
        {
            if (nIndex >= 0 && nIndex < m_xIndexAccess->getCount() )
                throw uno::RuntimeException();
        }
        else
            throw uno::RuntimeException();

        uno::Reference< awt::XControl > xControl;
        if ( !aControlName.isEmpty() )
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

        if ( aControlName.isEmpty() )
            aControlName = ControlArrayWrapper::getControlName( xControl );

        xDialogContainer->removeByName( aControlName );
        xControl->dispose();
    }
    catch (const uno::RuntimeException&)
    {
        // the exceptions are not rethrown, impossibility to find or remove the control is currently not reported
        // since in most cases it means just that the controls is already not there, the VBA seems to do it in the same way

        // throw;
    }
    catch (const uno::Exception&)
    {
        // throw lang::WrappedTargetException("Can not create AXControl!",
        //         uno::Reference< uno::XInterface >(),
        //         uno::makeAny( e ) );
    }
}


uno::Type
ScVbaControls::getElementType()
{
    return cppu::UnoType<ooo::vba::msforms::XControl>::get();
}

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaControls, u"ooo.vba.msforms.Controls"_ustr )
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
