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

#pragma once

#include "anyrefdg.hxx"

class ScDocument;

class ScSimpleRefDlg: public ScAnyRefDlgController
{
private:
    Link<const OUString*,void> aCloseHdl;
    Link<const OUString&,void> aDoneHdl;
    Link<const OUString&,void> aAbortedHdl;
    Link<const OUString&,void> aChangeHdl;

    ScRange         theCurArea;
    bool            bCloseFlag;
    bool            bAutoReOpen;
    bool            bCloseOnButtonUp;
    bool            bSingleCell;
    bool            bMultiSelection;

    std::unique_ptr<weld::Label> m_xFtAssign;
    std::unique_ptr<formula::RefEdit> m_xEdAssign;
    std::unique_ptr<formula::RefButton> m_xRbAssign;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    void            Init();

    DECL_LINK( CancelBtnHdl, weld::Button&, void );
    DECL_LINK( OkBtnHdl, weld::Button&, void );

protected:

    virtual void    RefInputDone( bool bForced = false ) override;

public:
                    ScSimpleRefDlg( SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent);
    virtual        ~ScSimpleRefDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;

    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;
    virtual void    Close() override;

    void            StartRefInput();

    void            SetRefString(const OUString &rStr);
    virtual void    FillInfo(SfxChildWinInfo&) const override;

    void            SetCloseHdl( const Link<const OUString*,void>& rLink );
    void            SetUnoLinks( const Link<const OUString&,void>& rDone, const Link<const OUString&,void>& rAbort,
                                const Link<const OUString&,void>& rChange );

    void            SetFlags( bool bSetCloseOnButtonUp, bool bSetSingleCell, bool bSetMultiSelection );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
