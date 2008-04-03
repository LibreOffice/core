/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterBitmapContainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:55:29 $
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

#include "PresenterBitmapContainer.hxx"
#include "PresenterComponent.hxx"
#include "PresenterConfigurationAccess.hxx"

#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <boost/bind.hpp>
#include <map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::std;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))


namespace sdext { namespace presenter {

//===== PresenterBitmapContainer ==============================================

PresenterBitmapContainer::PresenterBitmapContainer (
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const ::rtl::OUString& rsConfigurationBase)
    : maIconContainer()
{
    LoadButtons(rxComponentContext, rxCanvas, rsConfigurationBase);
}




PresenterBitmapContainer::~PresenterBitmapContainer (void)
{
    maIconContainer.clear();
}




PresenterBitmapContainer::BitmapSet PresenterBitmapContainer::GetButtons (
    const OUString& rsName) const
{
    BitmapContainer::const_iterator iSet (maIconContainer.find(rsName));
    if (iSet != maIconContainer.end())
        return iSet->second;
    else
        return BitmapSet();
}




void PresenterBitmapContainer::LoadButtons (
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const ::rtl::OUString& rsConfigurationBase)
{
    if ( ! rxCanvas.is())
        return;

    try
    {
        // Get access to the configuration.
        PresenterConfigurationAccess aConfiguration (
            rxComponentContext,
            A2S("org.openoffice.Office.extension.PresenterScreen"),
            PresenterConfigurationAccess::READ_ONLY);
        Reference<container::XNameAccess> xBitmapList (
            aConfiguration.GetConfigurationNode(rsConfigurationBase),
            UNO_QUERY_THROW);

        // Determine the base path of the bitmaps.
        Reference<deployment::XPackageInformationProvider> xInformationProvider (
            rxComponentContext->getValueByName(OUString::createFromAscii(
                "/singletons/com.sun.star.deployment.PackageInformationProvider")),
            UNO_QUERY_THROW);
        OUString sLocation;
        if (xInformationProvider.is())
            sLocation = xInformationProvider->getPackageLocation(gsExtensionIdentifier);
        sLocation += A2S("/");

        // Create an object that is able to load the bitmaps in a format that is
        // supported by the canvas.
        Reference<lang::XMultiComponentFactory> xFactory (
            rxComponentContext->getServiceManager(), UNO_QUERY);
        if ( ! xFactory.is())
            return;
        Reference<drawing::XPresenterHelper> xBitmapLoader(
            xFactory->createInstanceWithContext(
                A2S("com.sun.star.drawing.PresenterHelper"),
                rxComponentContext),
            UNO_QUERY_THROW);

        // Load all button bitmaps.
        if (xBitmapList.is())
        {
            vector<OUString> aProperties (4);
            aProperties[0] = A2S("Name");
            aProperties[1] = A2S("NormalFileName");
            aProperties[2] = A2S("MouseOverFileName");
            aProperties[3] = A2S("ButtonDownFileName");
            PresenterConfigurationAccess::ForAll(
                xBitmapList,
                aProperties,
                ::boost::bind(&PresenterBitmapContainer::ProcessBitmap, this,
                    _1,
                    _2,
                    sLocation,
                    rxCanvas,
                    xBitmapLoader));
        }
    }
    catch (Exception&)
    {
        OSL_ASSERT(false);
    }
}




void PresenterBitmapContainer::ProcessBitmap (
    const OUString& rsKey,
    const vector<Any>& rValues,
    const OUString& rsLocation,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const Reference<drawing::XPresenterHelper>& rxBitmapLoader)
{
    (void)rsKey;
    OSL_ASSERT(rxCanvas.is());
    OSL_ASSERT(rxBitmapLoader.is());

    if (rValues.size() != 4)
        return;

    OUString sName;
    rValues[0] >>= sName;

    BitmapSet aIconSet;
    try
    {
        OUString sNormalIconName;
        if (rValues[1] >>= sNormalIconName)
            aIconSet.mxNormalIcon = rxBitmapLoader->loadBitmap(
                rsLocation + sNormalIconName,
                rxCanvas);
    }
    catch (Exception&) {}
    try
    {
        OUString sMouseOverIconName;
        if (rValues[2] >>= sMouseOverIconName)
            aIconSet.mxMouseOverIcon = rxBitmapLoader->loadBitmap(
                rsLocation + sMouseOverIconName,
                rxCanvas);
    }
    catch (Exception&) {}
    try
    {
        OUString sButtonDownIconName;
        if (rValues[3] >>= sButtonDownIconName)
            aIconSet.mxButtonDownIcon = rxBitmapLoader->loadBitmap(
                rsLocation + sButtonDownIconName,
                rxCanvas);
    }
    catch (Exception&) {}

    aIconSet.Update(rxCanvas->getDevice());

    maIconContainer.insert(BitmapContainer::value_type(sName,aIconSet));
}




//===== PresenterBitmapContainer::BitmapSet ===================================

void PresenterBitmapContainer::BitmapSet::Update (
    const Reference<rendering::XGraphicDevice>& rxDevice)
{
    if (mxNormalIcon.is() && ! mxMouseOverIcon.is())
    {
        const geometry::IntegerSize2D aSize (mxNormalIcon->getSize());
        Reference<rendering::XBitmap> xHighlight(rxDevice->createCompatibleBitmap(aSize));
        if (xHighlight.is())
        {
            Reference<rendering::XBitmapCanvas> xHighlightCanvas (xHighlight->queryBitmapCanvas());
            if (xHighlightCanvas.is())
            {
                double aColor[] = {0,0,0,128};
                xHighlightCanvas->drawBitmapModulated(
                    mxNormalIcon,
                    rendering::ViewState(geometry::AffineMatrix2D(1,0,0,0,1,0), NULL),
                    rendering::RenderState(geometry::AffineMatrix2D(1,0,0,0,1,0), NULL,
                        Sequence<double>(aColor,4),rendering::CompositeOperation::SOURCE));
                mxMouseOverIcon = xHighlight;
            }
        }
    }
}


} } // end of namespace ::sdext::presenter
