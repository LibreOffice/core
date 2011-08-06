/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVTOOLS_HELPAGENTWIDNOW_HXX_
#define _SVTOOLS_HELPAGENTWIDNOW_HXX_

#include "svtools/svtdllapi.h"
#include <vcl/floatwin.hxx>
#include <vcl/image.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= IHelpAgentCallback
    //====================================================================
    class IHelpAgentCallback
    {
    public:
        virtual void helpRequested() = 0;
        virtual void closeAgent() = 0;
    };

    //====================================================================
    //= HelpAgentWindow
    //====================================================================
    class SVT_DLLPUBLIC HelpAgentWindow : public FloatingWindow
    {
    protected:
        Window*                 m_pCloser;
        IHelpAgentCallback*     m_pCallback;
        Size                    m_aPreferredSize;
        Image                   m_aPicture;

    public:
        HelpAgentWindow( Window* _pParent );
        ~HelpAgentWindow();

        /// returns the preferred size of the window
        const Size& getPreferredSizePixel() const { return m_aPreferredSize; }

        // callback handler maintainance
        void                setCallback(IHelpAgentCallback* _pCB) { m_pCallback = _pCB; }
        IHelpAgentCallback* getCallback() const { return m_pCallback; }

    protected:
        virtual void Resize();
        virtual void Paint( const Rectangle& rRect );
        virtual void MouseButtonUp( const MouseEvent& rMEvt );

        DECL_LINK( OnButtonClicked, Window* );

    private:
        SVT_DLLPRIVATE Size implOptimalButtonSize( const Image& _rButtonImage );
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif  // _SVTOOLS_HELPAGENTWIDNOW_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
