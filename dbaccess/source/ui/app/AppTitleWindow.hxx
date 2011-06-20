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
#ifndef DBAUI_TITLE_WINDOW_HXX
#define DBAUI_TITLE_WINDOW_HXX

#include <vcl/fixed.hxx>

namespace dbaui
{
    class OTitleWindow : public Window
    {
        FixedText   m_aSpace1;
        FixedText   m_aSpace2;
        FixedText   m_aTitle;
        Window*     m_pChild;
        sal_Bool        m_bShift;
        void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OTitleWindow(Window* _pParent,sal_uInt16 _nTitleId,WinBits _nBits,sal_Bool _bShift = sal_True);
        virtual ~OTitleWindow();

        // window overloads
        virtual void Resize();
        virtual void GetFocus();

        /** sets the child window which should be displayed below the title. It will be destroyed at the end.
            @param  _pChild
                The child window.
        */
        void setChildWindow(Window* _pChild);

        /** gets the child window.

            @return
                The child winodw.
        */
        inline Window* getChildWindow() const { return m_pChild; }

        /** sets the title text out of the resource
            @param  _nTitleId
                The resource id of the title text.
        */
        void setTitle(sal_uInt16 _nTitleId);


        /** Gets the min Width in Pixel which is needed to display the whole

            @return
                the min width
        */
        long GetWidthPixel() const;
    };
// .............................................................
} // namespace dbaui
// .............................................................
#endif // DBAUI_TITLE_WINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
