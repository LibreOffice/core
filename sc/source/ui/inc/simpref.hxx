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

#ifndef SC_SIMPREF_HXX
#define SC_SIMPREF_HXX

#include <vcl/group.hxx>

#include <vcl/combobox.hxx>

#include <vcl/fixed.hxx>

#include <vcl/morebtn.hxx>
#include "anyrefdg.hxx"
#include "dbdata.hxx"

class ScViewData;
class ScDocument;


//============================================================================

class ScSimpleRefDlg: public ScAnyRefDlg
{
private:
    Link            aCloseHdl;
    Link            aDoneHdl;
    Link            aAbortedHdl;
    Link            aChangeHdl;

    FixedText* m_pFtAssign;
    formula::RefEdit* m_pEdAssign;
    formula::RefButton* m_pRbAssign;

    OKButton*       m_pBtnOk;
    CancelButton*   m_pBtnCancel;

    ScDocument*     pDoc;

    ScRange         theCurArea;
    bool            bCloseFlag;
    bool            bAutoReOpen;
    bool            bCloseOnButtonUp;
    bool            bSingleCell;
    bool            bMultiSelection;

    void            Init();

    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );


protected:

    virtual void    RefInputDone( bool bForced = false );

public:
                    ScSimpleRefDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScViewData*    ptrViewData );
                    ~ScSimpleRefDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual bool    Close();

    void            StartRefInput();

    OUString       GetRefString() const;
    void           SetRefString(const OUString &rStr);
    virtual void   FillInfo(SfxChildWinInfo&) const;

    void            SetCloseHdl( const Link& rLink );
    void            SetUnoLinks( const Link& rDone, const Link& rAbort,
                                const Link& rChange );

    void            SetFlags( bool bSetCloseOnButtonUp, bool bSetSingleCell, bool bSetMultiSelection );

    void            SetAutoReOpen(bool bFlag) {bAutoReOpen=bFlag;}
};



#endif // SC_DBNAMDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
