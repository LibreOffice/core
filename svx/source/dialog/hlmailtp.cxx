/*************************************************************************
 *
 *  $RCSfile: hlmailtp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-23 11:52:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

#include <comphelper/processfactory.hxx>

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXINIPROP_HXX
#include <svtools/iniprop.hxx>
#endif

//#include <usr/ustring.hxx>
#include "hlmailtp.hxx"
#include "hyperdlg.hrc"

using namespace ::rtl;
using namespace ::com::sun::star;

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHyperlinkMailTp::SvxHyperlinkMailTp ( Window *pParent, const SfxItemSet& rItemSet)
:   maGrpMailNews   ( this, ResId (GRP_MAILNEWS) ),
    maRbtMail       ( this, ResId (RB_LINKTYP_MAIL) ),
    maRbtNews       ( this, ResId (RB_LINKTYP_NEWS) ),
    maFtReceiver    ( this, ResId (FT_RECEIVER) ),
    maCbbReceiver   ( this, INET_PROT_MAILTO ),
    maFtSubject     ( this, ResId (FT_SUBJECT) ),
    maEdSubject     ( this, ResId (ED_SUBJECT) ),
    maBtAdrBook     ( this, ResId (BTN_ADRESSBOOK) ),
    SvxHyperlinkTabPageBase ( pParent, SVX_RES( RID_SVXPAGE_HYPERLINK_MAIL ),
                              rItemSet )
{
    InitStdControls();
    FreeResource();

    // Init URL-Box (pos&size, Open-Handler)
    maCbbReceiver.SetPosSizePixel ( LogicToPixel( Point( 54, 25 ), MAP_APPFONT ),
                                    LogicToPixel( Size ( 176, 60), MAP_APPFONT ) );

    maCbbReceiver.Show();
    maCbbReceiver.SetHelpId( HID_HYPERDLG_MAIL_PATH );

    SetExchangeSupport ();

    // set defaults
    maRbtMail.Check ();

    // overload handlers
    maRbtMail.SetClickHdl        ( LINK ( this, SvxHyperlinkMailTp, ClickTypeEMailHdl_Impl ) );
    maRbtNews.SetClickHdl        ( LINK ( this, SvxHyperlinkMailTp, ClickTypeNewsHdl_Impl ) );
    maBtAdrBook.SetClickHdl      ( LINK ( this, SvxHyperlinkMailTp, ClickAdrBookHdl_Impl ) );

    maCbbReceiver.SetLoseFocusHdl( LINK ( this, SvxHyperlinkMailTp, LostFocusReceiverHdl_Impl ) );
    maCbbReceiver.SetModifyHdl   ( LINK ( this, SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl) );
}

SvxHyperlinkMailTp::~SvxHyperlinkMailTp ()
{
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/

void SvxHyperlinkMailTp::FillDlgFields ( String& aStrURL )
{
    INetURLObject aURL ( aStrURL );
    String aStrScheme, aStrSubject;

    // Protocoll
    INetProtocol eProtocol = aURL.GetProtocol ();
    switch ( eProtocol )
    {
        case INET_PROT_MAILTO :
            maRbtMail.Check ();
            maRbtNews.Check (FALSE);

            maFtSubject.Enable();
            maEdSubject.Enable();

            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( INET_MAILTO_SCHEME ) );

            break;
        case INET_PROT_NEWS :
            maRbtMail.Check (FALSE);
            maRbtNews.Check ();

            maFtSubject.Disable();
            maEdSubject.Disable();

            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( INET_NEWS_SCHEME ) );

            break;
        default:
            maRbtMail.Check ();
            maRbtNews.Check (FALSE);

            break;
    }

    if ( aStrScheme != aEmptyStr )
    {
        xub_StrLen nPos = aURL.GetMainURL().Search ( aStrScheme, 0 ) + aStrScheme.Len();
        String aStrURLc ( aURL.GetMainURL() );

        if ( eProtocol == INET_PROT_MAILTO )
        {
            // Find mail-subject
            String aStrTmp ( aStrURLc );

            const sal_Char sSubject[] = "subject";
            nPos = aStrTmp.ToLowerAscii().SearchAscii( sSubject, 0 );
            nPos = aStrTmp.Search( sal_Unicode( '=' ), nPos );

            if ( nPos != STRING_NOTFOUND )
                aStrSubject = aStrURLc.Copy( nPos+1, aStrURLc.Len() );

            nPos = aStrURLc.Search ( sal_Unicode( '?' ), 0);

            aStrURLc = aStrURLc.Copy( 0, ( nPos == STRING_NOTFOUND ?
                                               aStrURLc.Len() : nPos ) );

            maEdSubject.SetText ( aStrSubject );
        }
        else
        {
            maEdSubject.SetText (aEmptyStr);
            maEdSubject.SetText ( aEmptyStr );
        }

        maCbbReceiver.SetText ( aStrURLc );
    }
    else
    {
        maCbbReceiver.SetText ( aEmptyStr );
        maEdSubject.SetText ( aEmptyStr );
    }
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkMailTp::GetCurentItemData ( String& aStrURL, String& aStrName,
                                             String& aStrIntName, String& aStrFrame,
                                             SvxLinkInsertMode& eMode )
{
    String aStrScheme;
    String aStrParam;

    const sal_Char sMailtoScheme[] = INET_MAILTO_SCHEME;
    const sal_Char sNewsScheme[]   = INET_NEWS_SCHEME;

    // get data from dialog-controls
    aStrURL = maCbbReceiver.GetText();

    if ( maRbtMail.IsChecked() && aStrURL.SearchAscii( sMailtoScheme ) != 0 )
    {
        aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( INET_MAILTO_SCHEME ) );

    } else if ( maRbtNews.IsChecked() && aStrURL.SearchAscii( sNewsScheme ) != 0 )
    {
        aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( INET_NEWS_SCHEME ) );
    }

    if ( maRbtMail.IsChecked() )
    {
        if ( maEdSubject.GetText() != aEmptyStr )
        {
            aStrParam = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "subject=" ) );
            aStrParam += maEdSubject.GetText();
        }
    }

    UniString aStrTmp( aStrScheme );
    aStrTmp.Append( aStrURL );
    INetURLObject aURL ( aStrTmp );
    aURL.SetParam (aStrParam);

    // get data from standard-fields
    aStrIntName = mpEdText->GetText();
    aStrName    = mpEdIndication->GetText();
    aStrFrame   = mpCbbFrame->GetText();
    eMode       = (SvxLinkInsertMode) (mpLbForm->GetSelectEntryPos()+1);
    if( IsHTMLDoc() )
        eMode = (SvxLinkInsertMode) ( UINT16(eMode) | HLINK_HTMLMODE );

    if ( aStrURL != aEmptyStr )
        aStrURL     = aURL.GetMainURL();

    if ( aStrName == aEmptyStr )
        aStrName = aStrURL;
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

IconChoicePage* SvxHyperlinkMailTp::Create( Window* pWindow, const SfxItemSet& rItemSet )
{
    return( new SvxHyperlinkMailTp( pWindow, rItemSet ) );
}

/*************************************************************************
|*
|* Activate / Deactivate Tabpage
|*
|************************************************************************/

void SvxHyperlinkMailTp::ActivatePage( const SfxItemSet& rItemSet )
{
    ///////////////////////////////////////
    // Set dialog-fields from input-itemset
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // standard-fields
        FillStandardDlgFields (pHyperlinkItem);

        mbNewName = ( pHyperlinkItem->GetName() == aEmptyStr );
    }
}

int SvxHyperlinkMailTp::DeactivatePage( SfxItemSet* pSet )
{
    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_GETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    pSet->Put (aItem);

    return( LEAVE_PAGE );
}

/*************************************************************************
|*
|* Fill output-ItemSet
|*
|************************************************************************/

BOOL SvxHyperlinkMailTp::FillItemSet( SfxItemSet& rOut)
{
    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_SETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    rOut.Put (aItem);

    return TRUE;
}

/*************************************************************************
|*
|* reset dialog-fields
|*
|************************************************************************/

void SvxHyperlinkMailTp::Reset( const SfxItemSet& rItemSet)
{
    ///////////////////////////////////////
    // Set dialog-fields from create-itemset
    maStrInitURL = aEmptyStr;

    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // set dialog-fields
        FillStandardDlgFields (pHyperlinkItem);

        // set all other fields
        FillDlgFields ( (String&)pHyperlinkItem->GetURL() );

        // Store initial URL
        maStrInitURL = pHyperlinkItem->GetURL();

        mbNewName = ( pHyperlinkItem->GetName() == aEmptyStr );
    }
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkMailTp::SetInitFocus()
{
    maCbbReceiver.GrabFocus();
}

/*************************************************************************
|*
|* Change Scheme-String
|*
|************************************************************************/

void SvxHyperlinkMailTp::ChangeScheme ( String& aStrURL, String aStrNewScheme )
{
    INetURLObject aURL ( aStrURL );
    String aStrScheme;

    // set protocoll-radiobuttons
    INetProtocol aProtocol = aURL.GetProtocol ();
    switch ( aProtocol )
    {
        case INET_PROT_MAILTO :
            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( INET_MAILTO_SCHEME ) );
            break;
        case INET_PROT_NEWS :
            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( INET_NEWS_SCHEME ) );
            break;
    }

    if ( aStrScheme != aEmptyStr )
    {
        aStrURL = aStrNewScheme;
        aStrURL += aStrURL.Erase ( 0, aStrScheme.Len() );
    }
}

/*************************************************************************
|*
|* Click on radiobutton : Type EMail
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, ClickTypeEMailHdl_Impl, void *, EMPTYARG )
{
    maFtSubject.Enable();
    maEdSubject.Enable();

    String aStrURL ( maCbbReceiver.GetText() );
    ChangeScheme ( aStrURL, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM ( INET_MAILTO_SCHEME ) ) );
    maCbbReceiver.SetText ( aStrURL );

    ModifiedReceiverHdl_Impl (NULL);

    return( 0L );
}

/*************************************************************************
|*
|* Click on radiobutton : Type News
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, ClickTypeNewsHdl_Impl, void *, EMPTYARG )
{
    maFtSubject.Disable();
    maEdSubject.Disable();

    String aStrURL ( maCbbReceiver.GetText() );
    ChangeScheme ( aStrURL, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM ( INET_NEWS_SCHEME ) ) );
    maCbbReceiver.SetText ( aStrURL );

    ModifiedReceiverHdl_Impl (NULL);

    return( 0L );
}

/*************************************************************************
|*
|* Contens of editfield "receiver" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl, void *, EMPTYARG )
{
    const sal_Char sMailtoScheme[] = INET_MAILTO_SCHEME;
    const sal_Char sNewsScheme[]   = INET_NEWS_SCHEME;

    String aStrCurrentReceiver( maCbbReceiver.GetText() );

    if ( mbNewName )
        mpEdIndication->SetText ( aStrCurrentReceiver );

    // changed scheme ? - Then change radiobutton-settings
    if( aStrCurrentReceiver.SearchAscii( sMailtoScheme ) == 0 && !maRbtMail.IsChecked() )
    {
        maRbtMail.Check();
        maRbtNews.Check(FALSE);
        maFtSubject.Enable();
        maEdSubject.Enable();
    }
    else if( aStrCurrentReceiver.SearchAscii( sNewsScheme ) == 0 && !maRbtNews.IsChecked() )
    {
        maRbtMail.Check(FALSE);
        maRbtNews.Check();
        maFtSubject.Disable();
        maEdSubject.Disable();
    }

    return( 0L );
}

/*************************************************************************
|*
|* Combobox Receiver lost the focus
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, LostFocusReceiverHdl_Impl, void *, EMPTYARG )
{
    const sal_Char sMailtoScheme[] = INET_MAILTO_SCHEME;
    const sal_Char sNewsScheme[]   = INET_NEWS_SCHEME;

    String aStrURL ( maCbbReceiver.GetText() );
    String aStrScheme;

    if ( maRbtMail.IsChecked() && aStrURL.SearchAscii( sMailtoScheme ) != 0 )
    {
        aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( INET_MAILTO_SCHEME ) );
    } else if ( maRbtNews.IsChecked() && aStrURL.SearchAscii( sNewsScheme ) != 0 )
    {
        aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( INET_NEWS_SCHEME ) );
    }

    if ( aStrURL != aEmptyStr )
    {
        String aStrTarget ( aStrScheme );
        aStrTarget += aStrURL;
        maCbbReceiver.SetText ( aStrTarget );
    }

    ModifiedReceiverHdl_Impl (NULL);

    return (0L);
}

/*************************************************************************
|*
|* Click on imagebutton : addressbook
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, ClickAdrBookHdl_Impl, void *, EMPTYARG )
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    uno::Reference< frame::XDispatchProvider > xProv( pViewFrame->GetFrame()->GetFrameInterface(), uno::UNO_QUERY );
    if ( xProv.is() )
    {
        SfxAppIniManagerProperty aProp;
        GetpApp()->Property( aProp );
        if( !aProp.GetIniManager() )
            return ( 0L );

        String aAddressBook = aProp.GetIniManager()->Get( SFX_KEY_ADDRESSBOOK );
        INetURLObject aObj;
        aObj.SetSmartProtocol( INET_PROT_FILE );
        aObj.SetURL( aAddressBook.GetToken( 0, sal_Unicode( ';' ) ) );

        String aMark( RTL_CONSTASCII_USTRINGPARAM( "db:Table;" ) );
        aMark += aAddressBook.GetToken( 1, sal_Unicode( ';' ) );
        aObj.SetMark( aMark );

        util::URL aURL;
        aURL.Complete = ::rtl::OUString( aObj.GetMainURL() );

        uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
        if( xFactory.is() )
        {
            uno::Reference< util::XURLTransformer > xTrans( xFactory->createInstance
                ( OUString::createFromAscii( "com.sun.star.util.URLTransformer" ) ),
                                  uno::UNO_QUERY);
            xTrans->parseStrict( aURL );

            uno::Reference< frame::XDispatch > aDisp = xProv->queryDispatch( aURL,
                                                        OUString::createFromAscii( "_beamer" ),
                                                        frame::FrameSearchFlag::GLOBAL |
                                                        frame::FrameSearchFlag::CREATE );
            if ( aDisp.is() )
            {
                uno::Sequence< beans::PropertyValue > aArgs(1);
                beans::PropertyValue* pArg = aArgs.getArray();
                pArg[0].Name = DEFINE_CONST_UNICODE("Referer");
                pArg[0].Value = uno::makeAny( OUString( DEFINE_CONST_UNICODE("private:user") ) );
                aDisp->dispatch( aURL, aArgs );
            }
        }
    }

    return( 0L );
}


