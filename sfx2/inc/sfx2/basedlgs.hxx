/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basedlgs.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 15:46:46 $
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
#ifndef _BASEDLGS_HXX
#define _BASEDLGS_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif
#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

class TabPage;
class SfxTabPage;
class SfxBindings;
class SfxChildWindow;
struct SfxChildWinInfo;
class SfxItemSet;
class SfxItemPool;
class OKButton;
class CancelButton;
class HelpButton;
class Button;
class FixedLine;

// class SfxModalDefParentHelper -----------------------------------------

class SfxModalDefParentHelper
{
private:
    Window *pOld;

public:
    SfxModalDefParentHelper(Window* pWindow);
    ~SfxModalDefParentHelper();
};

// class SfxModalDialog --------------------------------------------------

class SFX2_DLLPUBLIC SfxModalDialog: public ModalDialog
{
    sal_uInt32          nUniqId;
    String              aExtraData;
    Timer               aTimer;

    SAL_DLLPRIVATE SfxModalDialog(SfxModalDialog &); // not defined
    SAL_DLLPRIVATE void operator =(SfxModalDialog &); // not defined

//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_LINK( TimerHdl_Impl, Timer* );
//#endif

    SAL_DLLPRIVATE void SetDialogData_Impl();
    SAL_DLLPRIVATE void GetDialogData_Impl();
    SAL_DLLPRIVATE void init();

protected:
    SfxModalDialog(Window *pParent, const ResId &);
    SfxModalDialog(Window* pParent, sal_uInt32 nUniqueId,
                   WinBits nWinStyle = WB_STDMODAL);
    ~SfxModalDialog();

    String&     GetExtraData()      { return aExtraData; }
    sal_uInt32  GetUniqId() const   { return nUniqId; }
};

// class SfxModelessDialog --------------------------------------------------
class SfxModelessDialog_Impl;
class SFX2_DLLPUBLIC SfxModelessDialog: public ModelessDialog
{
    SfxBindings*            pBindings;
    Size                    aSize;
    SfxModelessDialog_Impl* pImp;

    SAL_DLLPRIVATE SfxModelessDialog(SfxModelessDialog &); // not defined
    SAL_DLLPRIVATE void operator =(SfxModelessDialog &); // not defined

protected:
                            SfxModelessDialog( SfxBindings*, SfxChildWindow*,
                                Window*, const ResId& );
                            SfxModelessDialog( SfxBindings*, SfxChildWindow*,
                                Window*, WinBits nWinStyle = WB_STDMODELESS );
                            ~SfxModelessDialog();
    virtual BOOL            Close();
    virtual void            Resize();
    virtual void            Move();
    virtual void            StateChanged( StateChangedType nStateChange );

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo* pInfo);
    virtual long            Notify( NotifyEvent& rNEvt );
    SfxBindings&            GetBindings()
                            { return *pBindings; }

    DECL_LINK( TimerHdl, Timer* );

};

// class SfxFloatingWindow --------------------------------------------------
class SfxFloatingWindow_Impl;
class SFX2_DLLPUBLIC SfxFloatingWindow: public FloatingWindow
{
    SfxBindings*            pBindings;
    Size                    aSize;
    SfxFloatingWindow_Impl* pImp;

    SAL_DLLPRIVATE SfxFloatingWindow(SfxFloatingWindow &); // not defined
    SAL_DLLPRIVATE void operator =(SfxFloatingWindow &); // not defined

protected:
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              Window* pParent,
                                              WinBits nWinBits=WB_STDMODELESS);
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              Window* pParent,
                                              const ResId& rResId);
                            ~SfxFloatingWindow();

    virtual void            StateChanged( StateChangedType nStateChange );
    virtual BOOL            Close();
    virtual void            Resize();
    virtual void            Move();
    virtual long            Notify( NotifyEvent& rNEvt );
    SfxBindings&            GetBindings()
                            { return *pBindings; }

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo* pInfo);

    DECL_LINK( TimerHdl, Timer* );

};

// class SfxSingleTabDialog --------------------------------------------------

struct SingleTabDlgImpl
{
    TabPage*        m_pTabPage;
    SfxTabPage*     m_pSfxPage;
    FixedLine*      m_pLine;

    SingleTabDlgImpl() : m_pTabPage( NULL ), m_pSfxPage( NULL ), m_pLine( NULL ) {}
};

typedef USHORT* (*GetTabPageRanges)(); // liefert internationale Which-Werte

class SFX2_DLLPUBLIC SfxSingleTabDialog : public SfxModalDialog
{
public:
    SfxSingleTabDialog( Window* pParent, const SfxItemSet& rOptionsSet, USHORT nUniqueId );
    SfxSingleTabDialog( Window* pParent, USHORT nUniqueId, const SfxItemSet* pInSet = 0 );

    virtual             ~SfxSingleTabDialog();

    void                SetPage( TabPage* pNewPage );
    void                SetTabPage( SfxTabPage* pTabPage,
                                    GetTabPageRanges pRangesFunc = 0 );
    SfxTabPage*         GetTabPage() const { return pImpl->m_pSfxPage; }

    const USHORT*       GetInputRanges( const SfxItemPool& rPool );
    void                SetInputSet( const SfxItemSet* pInSet )
                            { pOptions = pInSet; }
    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }
    OKButton*           GetOKButton() const { return pOKBtn; }
    CancelButton*       GetCancelButton() const { return pCancelBtn; }

private:
    GetTabPageRanges    fnGetRanges;
    USHORT*             pRanges;

    OKButton*           pOKBtn;
    CancelButton*       pCancelBtn;
    HelpButton*         pHelpBtn;

    SingleTabDlgImpl*   pImpl;
    const SfxItemSet*   pOptions;
    SfxItemSet*         pOutSet;

    DECL_DLLPRIVATE_LINK( OKHdl_Impl, Button * );
};

#endif

