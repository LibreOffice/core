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
#ifndef DBAUI_VERTSPLITVIEW_HXX
#define DBAUI_VERTSPLITVIEW_HXX

#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif

namespace dbaui
{
    //==================================================================
    class OSplitterView : public Window
    {
        Splitter*                           m_pSplitter;
        Window*                             m_pLeft;
        Window*                             m_pRight;
        sal_Bool                            m_bVertical;

        void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
        DECL_LINK( SplitHdl, Splitter* );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OSplitterView(Window* _pParent,sal_Bool _bVertical = sal_True);
        virtual ~OSplitterView();
        // window overloads
        virtual void GetFocus();

        void setSplitter(Splitter* _pSplitter);
        void set(Window* _pRight,Window* _pLeft = NULL);
        virtual void Resize();
    };
}
#endif // DBAUI_VERTSPLITVIEW_HXX

