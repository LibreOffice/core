/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simpref.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-05 09:45:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

#ifndef SC_DBCOLECT_HXX
#include "dbcolect.hxx"
#endif

#ifndef _SC_EXPFTEXT_HXX
#include "expftext.hxx"
#endif

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

    void            SetFlags( BOOL bSetCloseOnButtonUp );
    void            SetSingleCell( BOOL bFlag );

    void            SetAutoReOpen(BOOL bFlag) {bAutoReOpen=bFlag;}

    static  String  GetLastRefString();
};



#endif // SC_DBNAMDLG_HXX

