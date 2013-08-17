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
#ifndef DBAUI_VERTSPLITVIEW_HXX
#define DBAUI_VERTSPLITVIEW_HXX

#include <vcl/window.hxx>

class Splitter;

namespace dbaui
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
