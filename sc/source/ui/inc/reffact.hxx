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

#ifndef SC_REFFACT_HXX
#define SC_REFFACT_HXX

#include <sfx2/childwin.hxx>

#include "dbfunc.hxx"

#define DECL_WRAPPER_WITHID(Class) \
    class Class : public SfxChildWindow                                         \
    {                                                                           \
    public:                                                                     \
        Class( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );           \
        SFX_DECL_CHILDWINDOW_WITHID(Class);                                     \
    };

//==================================================================

DECL_WRAPPER_WITHID(ScNameDlgWrapper)
DECL_WRAPPER_WITHID(ScNameDefDlgWrapper)
DECL_WRAPPER_WITHID(ScSolverDlgWrapper)
DECL_WRAPPER_WITHID(ScOptSolverDlgWrapper)
DECL_WRAPPER_WITHID(ScXMLSourceDlgWrapper)
DECL_WRAPPER_WITHID(ScPivotLayoutWrapper)
DECL_WRAPPER_WITHID(ScTabOpDlgWrapper)
DECL_WRAPPER_WITHID(ScFilterDlgWrapper)
DECL_WRAPPER_WITHID(ScSpecialFilterDlgWrapper)
DECL_WRAPPER_WITHID(ScDbNameDlgWrapper)
DECL_WRAPPER_WITHID(ScConsolidateDlgWrapper)
DECL_WRAPPER_WITHID(ScPrintAreasDlgWrapper)
DECL_WRAPPER_WITHID(ScColRowNameRangesDlgWrapper)
DECL_WRAPPER_WITHID(ScFormulaDlgWrapper)
DECL_WRAPPER_WITHID(ScHighlightChgDlgWrapper)

DECL_WRAPPER_WITHID(ScRandomNumberGeneratorDialogWrapper)

class ScAcceptChgDlgWrapper: public SfxChildWindow
{
    public:
        ScAcceptChgDlgWrapper(  Window*,
                                sal_uInt16,
                                SfxBindings*,
                                SfxChildWinInfo* );

        SFX_DECL_CHILDWINDOW_WITHID(Class);

        virtual void ReInitDlg();
};

class ScSimpleRefDlgWrapper: public SfxChildWindow
{
    public:
        ScSimpleRefDlgWrapper(  Window*,
                                sal_uInt16,
                                SfxBindings*,
                                SfxChildWinInfo* );

        SFX_DECL_CHILDWINDOW_WITHID(Class);

        static void     SetDefaultPosSize(Point aPos, Size aSize, sal_Bool bSet=sal_True);
        virtual String  GetRefString();
        virtual void    SetRefString(const String& rStr);
        void            SetCloseHdl( const Link& rLink );
        void            SetUnoLinks( const Link& rDone, const Link& rAbort,
                                        const Link& rChange );
        void            SetFlags( sal_Bool bCloseOnButtonUp, sal_Bool bSingleCell, sal_Bool bMultiSelection );
        static void     SetAutoReOpen(sal_Bool bFlag);

        void            StartRefInput();
};

class SC_DLLPUBLIC ScValidityRefChildWin : public SfxChildWindow
{
    bool    m_bVisibleLock:1;
    bool    m_bFreeWindowLock:1;
    Window * m_pSavedWndParent;
public:
    ScValidityRefChildWin( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );
    SFX_DECL_CHILDWINDOW_WITHID(ScValidityRefChildWin);
    ~ScValidityRefChildWin();
    bool    LockVisible( bool bLock ){ bool bVis = m_bVisibleLock; m_bVisibleLock = bLock; return bVis; }
    bool    LockFreeWindow( bool bLock ){ bool bFreeWindow = m_bFreeWindowLock; m_bFreeWindowLock = bLock; return bFreeWindow; }
    void                Hide(){ if( !m_bVisibleLock) SfxChildWindow::Hide(); }
    void                Show( sal_uInt16 nFlags ){ if( !m_bVisibleLock ) SfxChildWindow::Show( nFlags ); }
};

//==================================================================


#endif // SC_REFFACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
