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

#ifndef SC_HIGHRED_HXX
#define SC_HIGHRED_HXX

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

#ifndef FLT_DATE_BEFORE
#define FLT_DATE_BEFORE     0
#define FLT_DATE_SINCE      1
#define FLT_DATE_EQUAL      2
#define FLT_DATE_NOTEQUAL   3
#define FLT_DATE_BETWEEN    4
#define FLT_DATE_SAVE       5
#endif



class ScHighlightChgDlg : public ScAnyRefDlg
{
private:
    CheckBox*               m_pHighlightBox;
    SvxTPFilter*            m_pFilterCtr;
    CheckBox*               m_pCbAccept;
    CheckBox*               m_pCbReject;
    OKButton*               m_pOkButton;

    formula::RefEdit*       m_pEdAssign;
    formula::RefButton*     m_pRbAssign;

    ScViewData*             pViewData;
    ScDocument*             pDoc;
    ScRangeName             aLocalRangeName;
    Selection               theCurSel;
    ScRangeList             aRangeList;
    ScChangeViewSettings    aChangeViewSet;

    void                    Init();

    DECL_LINK( RefHandle, SvxTPFilter* );
    DECL_LINK(HighlightHandle, CheckBox*);
    DECL_LINK(OKBtnHdl, PushButton*);


protected:

    virtual void    RefInputDone( bool bForced = false );

public:
                    ScHighlightChgDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData);

                    ~ScHighlightChgDlg();

    virtual void    SetActive();
    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual bool    Close();
    virtual bool    IsRefInputMode() const;

};


#endif // SC_NAMEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
