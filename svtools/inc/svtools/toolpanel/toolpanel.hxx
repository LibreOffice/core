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
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVT_TOOLPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
