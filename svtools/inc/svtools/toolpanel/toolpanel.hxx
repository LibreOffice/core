/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef SVT_TOOLPANEL_HXX
#define SVT_TOOLPANEL_HXX

#include "svtools/svtdllapi.h"
#include "svtools/toolpanel/refbase.hxx"

#include <rtl/ustring.hxx>
#include <vcl/image.hxx>

#include <boost/noncopyable.hpp>

class Rectangle;
class Window;
namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
} } } }

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= IToolPanel
    //====================================================================
    /** abstract interface for a single tool panel
    */
    class SVT_DLLPUBLIC IToolPanel : public ::rtl::IReference
    {
    public:
        /// retrieves the display name of the panel
        virtual ::rtl::OUString GetDisplayName() const = 0;

        /// retrieves the image associated with the panel, if any
        virtual Image GetImage() const = 0;

        /// retrieves the help ID associated with the panel, if any.
        virtual rtl::OString GetHelpID() const = 0;

        /** activates the panel

            Usually, this means the panel's Window is created (if not previosly done so) and shown.

            @param i_rParentWindow
                the parent window to anchor the panel window at. Subsequent calls to the Activate
                method will always get the same parent window. The complete area of this window is
                available, and should be used, for the panel window.
        */
        virtual void Activate( Window& i_rParentWindow ) = 0;

        /** deactivates the panel

            There are different ways how an implementation could deactivate a panel. The easiest way
            would be to simply hide the associated Window. Alternatively, you could completely destroy it,
            or decide to cache it by re-parenting it to another (temporary, invisible) window.
        */
        virtual void Deactivate() = 0;

        /** sets a new size for the panel's Window

            The panel window is always expected to be positioned at (0,0), relative to the parent window
            which was passed to the Activate member. Resizing the panel window is necessary when the size of
            this parent window changes. Effectively, this method is a means of convenience, to relief panel
            implementations from reacting on size changes of their parent window themselves.
        */
        virtual void SetSizePixel( const Size& i_rPanelWindowSize ) = 0;

        /// sets the focus to the panel window
        virtual void GrabFocus() = 0;

        /** release any resources associated with the panel.

            In particular, implementations should ultimately destroy the VCL window which implements the panel
            window. No subsequent calls to any other method will happen after Destroy has been called.
        */
        virtual void Dispose() = 0;

        /** creates an XAccessible for the tool panel

            Implementations are allowed to create a new instance each time this method is called, the caller
            is responsible for caching the XAccessible implementation, if this is desired.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                    CreatePanelAccessible(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                    ) = 0;

        virtual ~IToolPanel()
        {
        }
    };

    typedef ::rtl::Reference< IToolPanel >  PToolPanel;

    //====================================================================
    //= ToolPanelBase
    //====================================================================
    /** base class for tool panel implementations, adding ref count implementation to the IToolPanel interface,
        but still being abstract
    */
    class SVT_DLLPUBLIC ToolPanelBase   :public IToolPanel
                                        ,public RefBase
                                        ,public ::boost::noncopyable
    {
    protected:
        ToolPanelBase();
        ~ToolPanelBase();

    public:
        DECLARE_IREFERENCE()

    private:
        oslInterlockedCount m_refCount;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVT_TOOLPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
