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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SIMPREF_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SIMPREF_HXX

#include <vcl/group.hxx>

#include <vcl/combobox.hxx>

#include <vcl/fixed.hxx>

#include <vcl/morebtn.hxx>
#include "anyrefdg.hxx"
#include "dbdata.hxx"

class ScDocument;

class ScSimpleRefDlg: public ScAnyRefDlg
{
private:
    Link<const OUString*,void> aCloseHdl;
    Link<const OUString&,void> aDoneHdl;
    Link<const OUString&,void> aAbortedHdl;
    Link<const OUString&,void> aChangeHdl;

    VclPtr<FixedText> m_pFtAssign;
    VclPtr<formula::RefEdit> m_pEdAssign;
    VclPtr<formula::RefButton> m_pRbAssign;

    VclPtr<OKButton>       m_pBtnOk;
    VclPtr<CancelButton>   m_pBtnCancel;

    ScRange         theCurArea;
    bool            bCloseFlag;
    bool            bAutoReOpen;
    bool            bCloseOnButtonUp;
    bool            bSingleCell;
    bool            bMultiSelection;

    void            Init();

    DECL_LINK_TYPED( CancelBtnHdl, Button*, void );
    DECL_LINK_TYPED( OkBtnHdl, Button*, void );

protected:

    virtual void    RefInputDone( bool bForced = false ) SAL_OVERRIDE;

public:
                    ScSimpleRefDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent);
    virtual        ~ScSimpleRefDlg();
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;

    virtual bool    IsRefInputMode() const SAL_OVERRIDE;
    virtual void    SetActive() SAL_OVERRIDE;
    virtual bool    Close() SAL_OVERRIDE;

    void            StartRefInput();

    void            SetRefString(const OUString &rStr);
    virtual void    FillInfo(SfxChildWinInfo&) const SAL_OVERRIDE;

    void            SetCloseHdl( const Link<const OUString*,void>& rLink );
    void            SetUnoLinks( const Link<const OUString&,void>& rDone, const Link<const OUString&,void>& rAbort,
                                const Link<const OUString&,void>& rChange );

    void            SetFlags( bool bSetCloseOnButtonUp, bool bSetSingleCell, bool bSetMultiSelection );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_SIMPREF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
