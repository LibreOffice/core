/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterCanvasFactory.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "precompiled_sd.hxx"

#include "presenter/PresenterCanvasFactory.hxx"
#include "PresenterCanvas.hxx"

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/WindowClass.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/drawing/CanvasFeature.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/compbase2.hxx>
#include <rtl/ref.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sd { namespace presenter {

//===== PresenterCanvasFactory::SharedWindowContainer =========================

namespace {
    class SharedWindowDescriptor
    {
    public:
        Reference<awt::XWindow> mxSharedWindow;
        Reference<rendering::XCanvas> mxSharedCanvas;
    };
}

class PresenterCanvasFactory::SharedWindowContainer
    : public ::std::vector<SharedWindowDescriptor>
{
public:
    iterator FindDescriptor (const Reference<awt::XWindow>& rxWindow)
    {
        for (iterator iDescriptor=begin(); iDescriptor!=end(); ++iDescriptor)
            if (iDescriptor->mxSharedWindow == rxWindow)
                return iDescriptor;
        return end();
    }
};




//===== PresenterCanvasFactory ================================================

class PresenterCanvasFactory::Deleter
{
public:
    void operator() (const PresenterCanvasFactory* pObject) { delete pObject; }
};


::boost::shared_ptr<PresenterCanvasFactory> PresenterCanvasFactory::mpInstance;


::boost::shared_ptr<PresenterCanvasFactory> PresenterCanvasFactory::Instance (void)
{
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());
    if (mpInstance.get() == NULL)
    {
        mpInstance.reset(new PresenterCanvasFactory(), PresenterCanvasFactory::Deleter());
    }

    return mpInstance;
}




void PresenterCanvasFactory::AddSharedWindow (
    const Reference<awt::XWindow>& rxWindow,
    const Reference<rendering::XCanvas>& rxCanvas)
{
    SharedWindowDescriptor aDescriptor;

    if (mpSharedWindows->FindDescriptor(rxWindow) != mpSharedWindows->end())
        return;

    aDescriptor.mxSharedWindow = rxWindow;
    aDescriptor.mxSharedCanvas = rxCanvas;

    // Store the new shared window only when both the window and the canvas
    // are present.
    if (aDescriptor.mxSharedCanvas.is() && aDescriptor.mxSharedCanvas.is())
        mpSharedWindows->push_back(aDescriptor);
}




void PresenterCanvasFactory::RemoveSharedWindow (const Reference<awt::XWindow>& rxWindow)
{
    SharedWindowContainer::iterator iDescriptor = mpSharedWindows->FindDescriptor(rxWindow);
    if (iDescriptor != mpSharedWindows->end())
    {
        mpSharedWindows->erase(iDescriptor);
    }
}




Reference<rendering::XCanvas> PresenterCanvasFactory::GetCanvas (
    const css::uno::Reference<css::awt::XWindow>& rxSharedWindow,
    const css::uno::Reference<css::awt::XWindow>& rxWindow,
    const sal_Int16 nRequestedCanvasFeatures,
    const rtl::OUString& rsCanvasServiceName)
{
    (void)nRequestedCanvasFeatures;

    Reference<rendering::XCanvas> xCanvas;

    if (rxSharedWindow.is() && rsCanvasServiceName.getLength()==0)
    {
        OSL_ASSERT(rxSharedWindow.is());
        xCanvas = CreateSharedCanvas(rxSharedWindow, rxWindow);
    }
    else
    {
        xCanvas = CreateCanvas(rxWindow, rsCanvasServiceName);
    }

    return xCanvas;
}




Reference<rendering::XCanvas> PresenterCanvasFactory::CreateSharedCanvas (
    const css::uno::Reference<css::awt::XWindow>& rxSharedWindow,
    const css::uno::Reference<css::awt::XWindow>& rxWindow) const
{
    // A shared window is given.  Look it up and determine which canvas
    // to return.
    SharedWindowContainer::iterator iDescriptor (
        mpSharedWindows->FindDescriptor(rxSharedWindow));
    if (iDescriptor != mpSharedWindows->end())
    {
        if (rxWindow == iDescriptor->mxSharedWindow || ! rxWindow.is())
        {
            // A shared window itself is given.  Return the previously
            // created canvas.
            return Reference<rendering::XCanvas>(iDescriptor->mxSharedCanvas, UNO_QUERY);
        }
        else
        {
            // A true child window is given.  Create a canvas wrapper.
            return new PresenterCanvas(
                Reference<rendering::XCanvas>(iDescriptor->mxSharedCanvas, UNO_QUERY),
                iDescriptor->mxSharedWindow,
                rxWindow);
        }
    }

    return NULL;
}




Reference<rendering::XCanvas> PresenterCanvasFactory::CreateCanvasForSprite (
    const css::uno::Reference<css::awt::XWindow>& rxSharedWindow,
    const css::uno::Reference<css::awt::XWindow>& rxWindow) const
{
    OSL_ASSERT(rxSharedWindow.is());
    (void)rxWindow.is();

    SharedWindowContainer::iterator iDescriptor (
        mpSharedWindows->FindDescriptor(rxSharedWindow));
    if (iDescriptor != mpSharedWindows->end())
    {
        OSL_ASSERT(iDescriptor->mxSharedCanvas.is());
        Reference<rendering::XSpriteCanvas> xSpriteCanvas(iDescriptor->mxSharedCanvas, UNO_QUERY);
        if (xSpriteCanvas.is())
        {
            Reference<rendering::XCustomSprite> xSprite (
                xSpriteCanvas->createCustomSprite(geometry::RealSize2D(10,10)));
            if (xSprite.is())
            {
                return xSprite->getContentCanvas();
            }
        }
    }
    return NULL;
}




Reference<rendering::XCanvas> PresenterCanvasFactory::CreateCanvas (
    const css::uno::Reference<css::awt::XWindow>& rxWindow,
    const rtl::OUString& rsCanvasServiceName) const
{
    // No shared window is given or an explicit canvas service name is
    // specified.  Create a new canvas.
    ::Window* pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow != NULL)
    {
        Sequence<Any> aArg (5);

        // common: first any is VCL pointer to window (for VCL canvas)
        aArg[0] = makeAny(reinterpret_cast<sal_Int64>(pWindow));
        aArg[1] = Any();
        aArg[2] = makeAny(::com::sun::star::awt::Rectangle());
        aArg[3] = makeAny(sal_False);
        aArg[4] = makeAny(rxWindow);

        Reference<lang::XMultiServiceFactory> xFactory (::comphelper::getProcessServiceFactory());
        return Reference<rendering::XCanvas>(
            xFactory->createInstanceWithArguments(
                rsCanvasServiceName.getLength()>0
                    ? rsCanvasServiceName
                        : OUString::createFromAscii("com.sun.star.rendering.VCLCanvas"),
                aArg),
            UNO_QUERY);
    }

    return NULL;
}




Reference<rendering::XCanvas> PresenterCanvasFactory::GetSharedCanvas (
    const Reference<awt::XWindow>& rxSharedWindow)
{
    SharedWindowContainer::iterator iDescriptor = mpSharedWindows->FindDescriptor(rxSharedWindow);
    if (iDescriptor != mpSharedWindows->end())
        return Reference<rendering::XCanvas>(iDescriptor->mxSharedCanvas, UNO_QUERY);
    else
        return NULL;
}




Reference<rendering::XCanvas> PresenterCanvasFactory::GetSharedCanvas (
    const Reference<rendering::XCanvas>& rxCanvas)
{
    PresenterCanvas* pCanvas = dynamic_cast<PresenterCanvas*>(rxCanvas.get());
    if (pCanvas != NULL)
        return pCanvas->GetSharedCanvas();
    else
        return NULL;
}




PresenterCanvasFactory::PresenterCanvasFactory (void)
    : mpSharedWindows(new SharedWindowContainer())
{
}




PresenterCanvasFactory::~PresenterCanvasFactory (void)
{
    mpSharedWindows.reset();
}




} } // end of namespace ::sd::presenter
