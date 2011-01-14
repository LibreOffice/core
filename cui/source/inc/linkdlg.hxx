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

#ifndef _LINKDLG_HXX
#define _LINKDLG_HXX

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>

#include <svtools/svmedit.hxx>  // MultiLineEdit
#include <svtools/svtabbx.hxx>  // MultiLineEdit

/********************** SvUpdateLinksDialog ******************************
*************************************************************************/
namespace sfx2
{
    class LinkManager;
    class SvBaseLink;
}

class SvBaseLinksDlg : public ModalDialog
{
    using Window::SetType;

    FixedText aFtFiles;
    FixedText aFtLinks;
    FixedText aFtType;
    FixedText aFtStatus;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    PushButton aPbUpdateNow;
    PushButton aPbOpenSource;
    PushButton aPbChangeSource;
    PushButton aPbBreakLink;
    FixedText aFtFiles2;
    FixedText aFtSource2;
    FixedText aFtType2;
    FixedText aFtUpdate;
    RadioButton aRbAutomatic;
    RadioButton aRbManual;
    FixedText aFtFullFileName;
    FixedText aFtFullSourceName;
    FixedText aFtFullTypeName;
    String aStrAutolink;
    String aStrManuallink;
    String aStrBrokenlink;
    String aStrGraphiclink;
    String aStrButtonclose;
    String aStrCloselinkmsg;
    String aStrCloselinkmsgMulti;
    String aStrWaitinglink;
    sfx2::LinkManager*  pLinkMgr;
    sal_Bool            bHtmlMode;
    SvTabListBox aTbLinks;
    Timer aUpdateTimer;

#if _SOLAR__PRIVATE
    DECL_LINK( LinksSelectHdl, SvTabListBox * );
    DECL_LINK( LinksDoubleClickHdl, SvTabListBox * );
    DECL_LINK( AutomaticClickHdl, RadioButton * );
    DECL_LINK( ManualClickHdl, RadioButton * );
    DECL_LINK( UpdateNowClickHdl, PushButton * );
    DECL_LINK( OpenSourceClickHdl, PushButton * );
    DECL_LINK( ChangeSourceClickHdl, PushButton * );
    DECL_LINK( BreakLinkClickHdl, PushButton * );
    DECL_LINK( UpdateWaitingHdl, Timer * );
    DECL_LINK( EndEditHdl, sfx2::SvBaseLink* );
    sfx2::SvBaseLink* GetSelEntry( sal_uInt16* pPos );
    String ImplGetStateStr( const sfx2::SvBaseLink& );
    void SetType( sfx2::SvBaseLink& rLink, sal_uInt16 nPos, sal_uInt16 nType );
    void InsertEntry( const sfx2::SvBaseLink& rLink, sal_uInt16 nPos = LISTBOX_APPEND, sal_Bool bSelect = sal_False);
#endif

    void StartUpdateTimer()         { aUpdateTimer.Start(); }

    SvTabListBox&     Links()       { return aTbLinks; }
    FixedText&      FileName()      { return aFtFullFileName; }
    FixedText&      SourceName()    { return aFtFullSourceName; }
    FixedText&      TypeName()      { return aFtFullTypeName; }
    RadioButton&    Automatic()     { return aRbAutomatic; }
    RadioButton&    Manual()        { return aRbManual; }
    PushButton&     UpdateNow()     { return aPbUpdateNow; }
    PushButton&     OpenSource()    { return aPbOpenSource; }
    PushButton&     ChangeSource()  { return aPbChangeSource; }
    PushButton&     BreakLink()     { return aPbBreakLink; }

    String&         Autolink()      { return aStrAutolink; }
    String&         Manuallink()    { return aStrManuallink; }
    String&         Brokenlink()    { return aStrBrokenlink; }
    String&         Graphiclink()   { return aStrGraphiclink; }
    String&         Buttonclose()   { return aStrButtonclose; }
    String&         Closelinkmsg()  { return aStrCloselinkmsg; }
    String&         CloselinkmsgMulti() { return aStrCloselinkmsgMulti; }
    String&         Waitinglink()   { return aStrWaitinglink; }
    void SetManager( sfx2::LinkManager* );

public:
    SvBaseLinksDlg( Window * pParent, sfx2::LinkManager*, sal_Bool bHtml = sal_False );
    ~SvBaseLinksDlg();
    void SetActLink( sfx2::SvBaseLink * pLink );
};

#endif // _LINKDLG_HXX
