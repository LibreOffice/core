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

        /// shows the panel window
        virtual void Show() = 0;

        /// hides the panel window
        virtual void Hide() = 0;

        /// sets the position of the panel window
        virtual void SetPosSizePixel( const Rectangle& i_rPanelPlayground ) = 0;

        /// sets the focus to the panel window
        virtual void GrabFocus() = 0;

        /** determines whether the panel window, or any of its children, currently has the focus

            Effectively, an implementation simply needs to redelegate this to its panel window's HasChildPathFocus
            method.
        */
        virtual bool HasFocus() const = 0;

        /** release any resources associated with the panel.

            In particular, implementations should destroy the VCL window which implements the panel window.
        */
        virtual void Dispose() = 0;

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
