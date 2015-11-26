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

#include <vcl/morebtn.hxx>
#include <vcl/combobox.hxx>
#include <vcl/group.hxx>
#include <svtools/headbar.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svtabbx.hxx>

#include "rangenam.hxx"
#include "anyrefdg.hxx"

#include <vcl/lstbox.hxx>

#include <svx/ctredlin.hxx>
#include "chgtrack.hxx"
#include "chgviset.hxx"

class ScViewData;
class ScDocument;


class ScHighlightChgDlg : public ScAnyRefDlg
{
private:
    VclPtr<CheckBox>               m_pHighlightBox;
    VclPtr<SvxTPFilter>            m_pFilterCtr;
    VclPtr<CheckBox>               m_pCbAccept;
    VclPtr<CheckBox>               m_pCbReject;
    VclPtr<OKButton>               m_pOkButton;

    VclPtr<formula::RefEdit>       m_pEdAssign;
    VclPtr<formula::RefButton>     m_pRbAssign;

    ScViewData*             pViewData;
    ScDocument*             pDoc;
    ScChangeViewSettings    aChangeViewSet;

    void                    Init();

    DECL_LINK_TYPED( RefHandle, SvxTPFilter*, void );
    DECL_LINK_TYPED( HighlightHandle, Button*, void );
    DECL_LINK_TYPED( OKBtnHdl, Button*, void );

protected:

    virtual void    RefInputDone( bool bForced = false ) override;

public:
                    ScHighlightChgDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                               ScViewData*      ptrViewData);

                    virtual ~ScHighlightChgDlg();
    virtual void    dispose() override;

    virtual void    SetActive() override;
    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual bool    Close() override;
    virtual bool    IsRefInputMode() const override;

};

#endif // INCLUDED_SC_SOURCE_UI_INC_HIGHRED_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
