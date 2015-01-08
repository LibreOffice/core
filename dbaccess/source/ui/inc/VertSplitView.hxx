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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_VERTSPLITVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_VERTSPLITVIEW_HXX

#include <vcl/window.hxx>

class Splitter;

namespace dbaui
{
    class OSplitterView : public vcl::Window
    {
        Splitter*                           m_pSplitter;
        vcl::Window*                             m_pLeft;
        vcl::Window*                             m_pRight;
        bool                            m_bVertical;

        void ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
        DECL_LINK( SplitHdl, Splitter* );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt) SAL_OVERRIDE;
    public:
        OSplitterView(vcl::Window* _pParent,bool _bVertical = true);
        virtual ~OSplitterView();
        // Window overrides
        virtual void GetFocus() SAL_OVERRIDE;

        void setSplitter(Splitter* _pSplitter);
        void set(vcl::Window* _pRight,Window* _pLeft = NULL);
        virtual void Resize() SAL_OVERRIDE;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_VERTSPLITVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
