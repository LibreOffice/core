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

#ifndef INCLUDED_SC_SOURCE_UI_INC_HIGHRED_HXX
#define INCLUDED_SC_SOURCE_UI_INC_HIGHRED_HXX

#include "anyrefdg.hxx"

#include <svx/ctredlin.hxx>
#include <chgviset.hxx>

class ScViewData;
class ScDocument;


class ScHighlightChgDlg : public ScAnyRefDlgController
{
private:
    ScViewData&             m_rViewData;
    ScDocument&             rDoc;
    ScChangeViewSettings    aChangeViewSet;

    std::unique_ptr<weld::CheckButton> m_xHighlightBox;
    std::unique_ptr<weld::CheckButton> m_xCbAccept;
    std::unique_ptr<weld::CheckButton> m_xCbReject;
    std::unique_ptr<weld::Button> m_xOkButton;

    std::unique_ptr<formula::RefEdit> m_xEdAssign;
    std::unique_ptr<formula::RefButton> m_xRbAssign;

    std::unique_ptr<weld::Container> m_xBox;

    std::unique_ptr<SvxTPFilter> m_xFilterCtr;

    void                    Init();

    DECL_LINK( RefHandle, SvxTPFilter*, void );
    DECL_LINK( HighlightHandle, weld::Button&, void );
    DECL_LINK( OKBtnHdl, weld::Button&, void );

protected:

    virtual void    RefInputDone( bool bForced = false ) override;

public:
    ScHighlightChgDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                      ScViewData& rViewData);

    virtual ~ScHighlightChgDlg() override;

    virtual void    SetActive() override;
    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;
    virtual void    Close() override;
    virtual bool    IsRefInputMode() const override;

};

#endif // INCLUDED_SC_SOURCE_UI_INC_HIGHRED_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
