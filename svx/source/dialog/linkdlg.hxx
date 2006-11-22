/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linkdlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:35:39 $
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
    class SvLinkManager;
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
    sfx2::SvLinkManager*  pLinkMgr;
    BOOL            bHtmlMode;
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
    sfx2::SvBaseLink* GetSelEntry( USHORT* pPos );
    String ImplGetStateStr( const sfx2::SvBaseLink& );
    void SetType( sfx2::SvBaseLink& rLink, USHORT nPos, USHORT nType );
    void InsertEntry( const sfx2::SvBaseLink& rLink, USHORT nPos = LISTBOX_APPEND, sal_Bool bSelect = sal_False);
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
    void SetManager( sfx2::SvLinkManager* );

public:
    SvBaseLinksDlg( Window * pParent, sfx2::SvLinkManager*, BOOL bHtml = FALSE );
    ~SvBaseLinksDlg();
    void SetActLink( sfx2::SvBaseLink * pLink );
};

#endif // _LINKDLG_HXX
