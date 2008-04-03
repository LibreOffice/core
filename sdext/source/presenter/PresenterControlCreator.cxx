/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterControlCreator.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:57:02 $
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

#include "PresenterControlCreator.hxx"
#include "PresenterHelper.hxx"
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sdext { namespace presenter {

PresenterControlCreator::PresenterControlCreator (
    const Reference<uno::XComponentContext>& rxContext,
    const Reference<awt::XWindow>& rxParentWindow,
    const sal_Int32 nToolBarHeight)
    : mxComponentContext(rxContext),
      mxControlContainerWindow(),
      mxControlContainer(),
      mxModelNameContainer(),
      mxControlFactory(),
      mnControlCount(0),
      mnToolBarHeight(nToolBarHeight)
{
    if (CreateContainer())
        CreateContainerWindow(rxParentWindow);
}




PresenterControlCreator::~PresenterControlCreator (void)
{
}




sal_Int32 PresenterControlCreator::GetToolBarHeight (void) const
{
    return mnToolBarHeight;
}




Reference<awt::XWindow> PresenterControlCreator::GetContainerWindow (void) const
{
    return mxControlContainerWindow;
}




Reference<awt::XControlContainer> PresenterControlCreator::GetContainer (void) const
{
    return mxControlContainer;
}




bool PresenterControlCreator::CreateContainer (void)
{
    try
    {
        Reference<lang::XMultiComponentFactory> xFactory (
            mxComponentContext->getServiceManager(), UNO_QUERY_THROW);

        mxControlContainer = Reference<awt::XControlContainer>(
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.awt.UnoControlContainer"),
                mxComponentContext),
            UNO_QUERY_THROW);

        Reference<awt::XControlModel> xControlModel (
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.awt.UnoControlDialogModel"),
                mxComponentContext),
            UNO_QUERY_THROW);
        mxModelNameContainer = Reference<container::XNameContainer>(
            xControlModel, UNO_QUERY_THROW);

        Reference<awt::XControl> xControl (mxControlContainer, UNO_QUERY_THROW);
        xControl->setModel(xControlModel);

        mxControlFactory = Reference<lang::XMultiServiceFactory>(
            xControlModel, UNO_QUERY_THROW);

        return true;
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }

    return false;
}




void PresenterControlCreator::CreateContainerWindow (
    const Reference<awt::XWindow>& rxParentWindow)
{
    try
    {
        Reference<awt::XControl> xControl (mxControlContainer, UNO_QUERY_THROW);
        Reference<awt::XWindowPeer> xContainerPeer (rxParentWindow, UNO_QUERY_THROW);
        Reference<lang::XMultiComponentFactory> xFactory (
            mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
        Reference<awt::XToolkit> xToolkit (
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.awt.Toolkit"),
                mxComponentContext),
            UNO_QUERY_THROW);
        xControl->createPeer(xToolkit, xContainerPeer);

        mxControlContainerWindow = Reference<awt::XWindow>(mxControlContainer, UNO_QUERY_THROW);
        mxControlContainerWindow->setEnable(sal_True);
        mxControlContainerWindow->setVisible(sal_True);
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}




Reference<awt::XControl> PresenterControlCreator::AddButton (
    const OUString& rsName,
    const OUString& rsImageName,
    const Property pUserProperties[])
{
    Property aButtonDefaultProperties[] = {
        {(sal_Char*)"PushButtonType", Any(awt::PushButtonType_STANDARD)},
        {NULL, Any()}
    };
    Reference<awt::XControl> xControl(
        AddControl(
            rsName,
            OUString::createFromAscii("com.sun.star.awt.UnoControlButtonModel"),
            OUString::createFromAscii("com.sun.star.awt.UnoControlButton"),
            aButtonDefaultProperties,
            pUserProperties));
    if (xControl.is())
    {
        Reference<awt::XButton> xButton (xControl, UNO_QUERY_THROW);
        if (xButton.is())
            xButton->setActionCommand(rsName);


        Reference<graphic::XGraphic> xGraphic (
            PresenterHelper::GetGraphic(mxComponentContext, rsImageName));
        if (xGraphic.is())
        {
            Property aButtonGraphicProperties[] = {
                {(sal_Char*)"Graphic", Any(xGraphic)},
                {(sal_Char*)"Label", Any(OUString())},
                {NULL, Any()}
            };
            SetModelProperties (
                Reference<beans::XPropertySet>(xControl->getModel(), UNO_QUERY),
                aButtonGraphicProperties);
        }
    }

    return xControl;
}




Reference<awt::XControl> PresenterControlCreator::AddLabel (
    const OUString& rsName,
    const OUString& rsDefaultText,
    const Property pUserProperties[])
{
    Property aLabelDefaultProperties[] = {
        {(sal_Char*)"Label", Any(rsDefaultText)},
        {(sal_Char*)"VerticalAlign", Any(style::VerticalAlignment_MIDDLE)},
        {NULL, Any()}
    };
    Reference<awt::XControl> xLabel(
        AddControl(
            rsName,
            OUString::createFromAscii("com.sun.star.awt.UnoControlFixedTextModel"),
            OUString::createFromAscii("com.sun.star.awt.UnoControlFixedText"),
            aLabelDefaultProperties,
            pUserProperties));
    if (xLabel.is())
    {
        Reference<awt::XFixedText> xText (xLabel, UNO_QUERY);
        if (xText.is())
            xText->setAlignment(1);
    }
    return xLabel;
}




Reference<awt::XControl> PresenterControlCreator::AddEdit (
    const OUString& rsName,
    const Property pUserProperties[])
{
    Property aEditDefaultProperties[] = {
        {NULL, Any()}
    };
    Reference<awt::XControl> xControl(
        AddControl(
            rsName,
            OUString::createFromAscii("com.sun.star.awt.UnoControlEditModel"),
            OUString::createFromAscii("com.sun.star.awt.UnoControlEdit"),
            aEditDefaultProperties,
            pUserProperties));
    return xControl;
}




Reference<awt::XControl> PresenterControlCreator::AddControl (
    const OUString& rsName,
    const OUString& rsModelServiceName,
    const OUString& rsControlServiceName,
    const Property pControlDefaultProperties[],
    const Property pUserProperties[])
{
    Reference<awt::XControl> xControl;
    Property aPropertyDefaults[] = {
        {(sal_Char*)"Enabled", Any(sal_True)},
        {(sal_Char*)"Width", Any((sal_Int16)mnToolBarHeight)},
        {(sal_Char*)"Height", Any((sal_Int16)mnToolBarHeight)},
        {(sal_Char*)"Label", Any(rsName)},
        {(sal_Char*)"PositionX", Any((sal_Int32)0)},
        {(sal_Char*)"PositionY", Any((sal_Int16)0)},
        {(sal_Char*)"Step", Any((sal_Int16)0)},
        {(sal_Char*)"TabIndex", Any((sal_Int16)mnControlCount)},
        {(sal_Char*)"Name", Any(rsName)},
        {NULL, Any()}
    };
    try
    {
        // Create the model for the control.
        Reference<awt::XControlModel> xControlModel (
            mxControlFactory->createInstance(rsModelServiceName),
            UNO_QUERY_THROW);
        Reference<beans::XPropertySet> xPropertySet (xControlModel, UNO_QUERY_THROW);
        SetModelProperties(xPropertySet, aPropertyDefaults);
        SetModelProperties(xPropertySet, pControlDefaultProperties);
        SetModelProperties(xPropertySet, pUserProperties);
        mxModelNameContainer->insertByName(rsName, Any(xControlModel));

        // Create the control for the control.
        Reference<lang::XMultiComponentFactory> xFactory (
            mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
        xControl = Reference<awt::XControl> (
            xFactory->createInstanceWithContext(rsControlServiceName, mxComponentContext),
            UNO_QUERY_THROW);

        // Combine model and control.
        xControl->setModel(xControlModel);

        // Add the new control to the parent container.
        mxControlContainer->addControl(rsName, xControl);

        Reference<awt::XWindow> xWindow (xControl, UNO_QUERY_THROW);
        xWindow->setEnable(sal_True);
        xWindow->setVisible(sal_True);

        mnControlCount += 1;
    }
    catch (Exception& rException)
    {
        (void)rException;
        OSL_TRACE("caught exception in PresenterControlCreator::AddControl()");
    }

    return xControl;
}




void PresenterControlCreator::SetModelProperties (
    const Reference<beans::XPropertySet>& rxSet,
    const Property pProperties[])
{
    if ( ! rxSet.is())
        return;
    if (pProperties == NULL)
        return;

    for (sal_Int32 nIndex=0; pProperties[nIndex].msName!=NULL; ++nIndex)
    {
        try
        {
            rxSet->setPropertyValue(
                OUString::createFromAscii(pProperties[nIndex].msName),
                pProperties[nIndex].maValue);
        }
        catch (Exception& rException)
        {
            (void)rException;
            OSL_TRACE("caught exception in PresenterControlCreator::SetModelProperties() for property %s",
                pProperties[nIndex].msName);
        }
    }
}




} } // end of namespace ::sdext::presenter
