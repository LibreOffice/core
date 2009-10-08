/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: simpref.hxx,v $
 * $Revision: 1.6.32.1 $
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

#ifndef SC_SIMPREF_HXX
#define SC_SIMPREF_HXX

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#include "anyrefdg.hxx"
#include "dbcolect.hxx"
#include "expftext.hxx"

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

    FixedText       aFtAssign;
    ScRefEdit       aEdAssign;
    ScRefButton     aRbAssign;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    BOOL            bRefInputMode;

    ScRange         theCurArea;
    BOOL            bCloseFlag;
    BOOL            bAutoReOpen;
    BOOL            bCloseOnButtonUp;
    BOOL            bSingleCell;
    BOOL            bMultiSelection;

    void            Init();

    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );


protected:

    virtual void    RefInputDone( BOOL bForced = FALSE );

public:
                    ScSimpleRefDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScViewData*    ptrViewData );
                    ~ScSimpleRefDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual BOOL    IsRefInputMode() const;
    virtual void    SetActive();
    virtual BOOL    Close();

    void            StartRefInput();

    virtual String  GetRefString() const;
    virtual void    SetRefString(const String &rStr);
    virtual void    FillInfo(SfxChildWinInfo&) const;

    void            SetCloseHdl( const Link& rLink );
    void            SetUnoLinks( const Link& rDone, const Link& rAbort,
                                const Link& rChange );

    void            SetFlags( BOOL bSetCloseOnButtonUp, BOOL bSetSingleCell, BOOL bSetMultiSelection );

    void            SetAutoReOpen(BOOL bFlag) {bAutoReOpen=bFlag;}
};



#endif // SC_DBNAMDLG_HXX

