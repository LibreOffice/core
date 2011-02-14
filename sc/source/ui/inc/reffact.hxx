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

#ifndef SC_REFFACT_HXX
#define SC_REFFACT_HXX

#include <sfx2/childwin.hxx>

#include "dbfunc.hxx"

#define DECL_WRAPPER(Class) \
    class Class : public SfxChildWindow                                         \
    {                                                                           \
    public:                                                                     \
        Class( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );               \
        SFX_DECL_CHILDWINDOW(Class);                                            \
    };


//==================================================================

DECL_WRAPPER(ScNameDlgWrapper)
DECL_WRAPPER(ScSolverDlgWrapper)
DECL_WRAPPER(ScOptSolverDlgWrapper)
DECL_WRAPPER(ScPivotLayoutWrapper)
DECL_WRAPPER(ScTabOpDlgWrapper)
DECL_WRAPPER(ScFilterDlgWrapper)
DECL_WRAPPER(ScSpecialFilterDlgWrapper)
DECL_WRAPPER(ScDbNameDlgWrapper)
DECL_WRAPPER(ScConsolidateDlgWrapper)
DECL_WRAPPER(ScPrintAreasDlgWrapper)
DECL_WRAPPER(ScCondFormatDlgWrapper)
DECL_WRAPPER(ScColRowNameRangesDlgWrapper)
DECL_WRAPPER(ScFormulaDlgWrapper)
DECL_WRAPPER(ScHighlightChgDlgWrapper)

/*!!! dafuer muss der Funktionsautopilot noch umgebaut werden
DECL_WRAPPER(ScFunctionDlgWrapper)
DECL_WRAPPER(ScEditFunctionDlgWrapper)
DECL_WRAPPER(ScArgumentDlgWrapper)
*/

class ScAcceptChgDlgWrapper: public SfxChildWindow
{
    public:
        ScAcceptChgDlgWrapper(  Window*,
                                sal_uInt16,
                                SfxBindings*,
                                SfxChildWinInfo* );

        SFX_DECL_CHILDWINDOW(Class);

        virtual void ReInitDlg();
};

class ScSimpleRefDlgWrapper: public SfxChildWindow
{
    public:
        ScSimpleRefDlgWrapper(  Window*,
                                sal_uInt16,
                                SfxBindings*,
                                SfxChildWinInfo* );

        SFX_DECL_CHILDWINDOW(Class);

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

//<!--Added by PengYunQuan for Validity Cell Range Picker
class SC_DLLPUBLIC ScValidityRefChildWin : public SfxChildWindow
{
    bool    m_bVisibleLock:1;
    bool    m_bFreeWindowLock:1;
    Window * m_pSavedWndParent;
public:
    ScValidityRefChildWin( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );
    SFX_DECL_CHILDWINDOW(ScValidityRefChildWin);
    ~ScValidityRefChildWin();
    bool    LockVisible( bool bLock ){ bool bVis = m_bVisibleLock; m_bVisibleLock = bLock; return bVis; }
    bool    LockFreeWindow( bool bLock ){ bool bFreeWindow = m_bFreeWindowLock; m_bFreeWindowLock = bLock; return bFreeWindow; }
    void                Hide(){ if( !m_bVisibleLock) SfxChildWindow::Hide(); }
    void                Show( sal_uInt16 nFlags ){ if( !m_bVisibleLock ) SfxChildWindow::Show( nFlags ); }
};
//-->Added by PengYunQuan for Validity Cell Range Picker

//==================================================================


#endif // SC_REFFACT_HXX
