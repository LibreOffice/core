/*************************************************************************
 *
 *  $RCSfile: hlmailtp.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:00:56 $
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

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

#include <comphelper/processfactory.hxx>

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

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
    // Set HC bitmaps and disable display of bitmap names.
    maBtAdrBook.SetModeImage( Image( ResId( IMG_ADRESSBOOK_HC ) ), BMP_COLOR_HIGHCONTRAST );
    maBtAdrBook.EnableTextDisplay (FALSE);

    InitStdControls();
    FreeResource();

    // Init URL-Box (pos&size, Open-Handler)
    maCbbReceiver.SetPosSizePixel ( LogicToPixel( Point( COL_2, 25 ), MAP_APPFONT ),
                                    LogicToPixel( Size ( 176 - COL_DIFF, 60), MAP_APPFONT ) );

    maCbbReceiver.Show();
    maCbbReceiver.SetHelpId( HID_HYPERDLG_MAIL_PATH );

    SetExchangeSupport ();

    // set defaults
    maRbtMail.Check ();

    // overload handlers
    maRbtMail.SetClickHdl        ( LINK ( this, SvxHyperlinkMailTp, Click_SmartProtocol_Impl ) );
    maRbtNews.SetClickHdl        ( LINK ( this, SvxHyperlinkMailTp, Click_SmartProtocol_Impl ) );
    maBtAdrBook.SetClickHdl      ( LINK ( this, SvxHyperlinkMailTp, ClickAdrBookHdl_Impl ) );
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
    const sal_Char sMailtoScheme[] = INET_MAILTO_SCHEME;

    INetURLObject aURL( aStrURL );
    String aStrScheme = GetSchemeFromURL( aStrURL );

    // set URL-field and additional controls
    if ( aStrScheme != aEmptyStr )
    {
        String aStrURLc ( aStrURL );
        // set additional controls for EMail:
        if ( aStrScheme.SearchAscii( sMailtoScheme ) == 0 )
        {
            // Find mail-subject
            String aStrSubject, aStrTmp ( aStrURLc );

            const sal_Char sSubject[] = "subject";
            xub_StrLen nPos = aStrTmp.ToLowerAscii().SearchAscii( sSubject, 0 );
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
        }

        maCbbReceiver.SetText ( aStrURLc );
    }
    else
    {
        maCbbReceiver.SetText ( aEmptyStr );
        maEdSubject.SetText ( aEmptyStr );
    }

    SetScheme( aStrScheme );
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
    aStrURL = CreateAbsoluteURL();
    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

String SvxHyperlinkMailTp::CreateAbsoluteURL() const
{
    String aStrURL = maCbbReceiver.GetText();
    String aScheme = GetSchemeFromURL(aStrURL);

    INetURLObject aURL(aStrURL);

    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        aURL.ConcatData(GetSmartProtocolFromButtons(), String(), String(),
                        String(), 0, maCbbReceiver.GetText(),
                        INetURLObject::ENCODE_ALL);

        if( aURL.GetProtocol() == INET_PROT_NOT_VALID
            && aScheme.Len() == 0 )
        {
            //try wether this might be a relative link to the local fileystem
            aURL.SetSmartURL( SvtPathOptions().GetWorkPath() );
            if( !aURL.hasFinalSlash() )
                aURL.setFinalSlash();
            aURL.Append( aStrURL );
        }
    }

    // subject for EMail-url
    if( aURL.GetProtocol() == INET_PROT_MAILTO )
    {
        if ( maEdSubject.GetText() != aEmptyStr )
        {
            String aQuery = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "subject=" ) );
            aQuery.Append( maEdSubject.GetText() );
            aURL.SetParam(aQuery);
        }
    }

    if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        return aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );

    return aEmptyStr;
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
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkMailTp::SetInitFocus()
{
    maCbbReceiver.GrabFocus();
}

/*************************************************************************
|************************************************************************/

void SvxHyperlinkMailTp::SetScheme( const String& aScheme )
{
    //if  aScheme is empty or unknown the default beaviour is like it where MAIL
    const sal_Char sNewsScheme[]   = INET_NEWS_SCHEME;

    BOOL bMail = aScheme.SearchAscii( sNewsScheme ) != 0;

    //update protocol button selection:
    maRbtMail.Check(bMail);
    maRbtNews.Check(!bMail);

    //update target:
    RemoveImproperProtocol(aScheme);
    maCbbReceiver.SetSmartProtocol( GetSmartProtocolFromButtons() );

    //show/hide  special fields for MAIL:
    maFtSubject.Enable(bMail);
    maEdSubject.Enable(bMail);
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkMailTp::RemoveImproperProtocol(const String& aProperScheme)
{
    String aStrURL ( maCbbReceiver.GetText() );
    if ( aStrURL != aEmptyStr )
    {
        String aStrScheme = GetSchemeFromURL( aStrURL );
        if ( aStrScheme != aEmptyStr && aStrScheme != aProperScheme )
        {
            aStrURL.Erase ( 0, aStrScheme.Len() );
            maCbbReceiver.SetText ( aStrURL );
        }
    }
}

String SvxHyperlinkMailTp::GetSchemeFromButtons() const
{
    if( maRbtNews.IsChecked() )
    {
        return String::CreateFromAscii( INET_NEWS_SCHEME );
    }
    return String::CreateFromAscii( INET_MAILTO_SCHEME );
}

INetProtocol SvxHyperlinkMailTp::GetSmartProtocolFromButtons() const
{
    if( maRbtNews.IsChecked() )
    {
        return INET_PROT_NEWS;
    }
    return INET_PROT_MAILTO;
}

/*************************************************************************
|*
|* Click on radiobutton : Type EMail
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, Click_SmartProtocol_Impl, void *, EMPTYARG )
{
    String aScheme = GetSchemeFromButtons();
    SetScheme( aScheme );
    return( 0L );
}

/*************************************************************************
|*
|* Contens of editfield "receiver" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl, void *, EMPTYARG )
{
    String aScheme = GetSchemeFromURL( maCbbReceiver.GetText() );
    if(aScheme.Len()!=0)
        SetScheme( aScheme );

    return( 0L );
}

/*************************************************************************
|*
|* Click on imagebutton : addressbook
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, ClickAdrBookHdl_Impl, void *, EMPTYARG )
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if( pViewFrame )
    {
        SfxItemPool &rPool = pViewFrame->GetPool();
        SfxRequest aReq(SID_VIEW_DATA_SOURCE_BROWSER, 0, rPool);
        pViewFrame->ExecuteSlot( aReq, sal_True );
    }


/*  uno::Reference< frame::XDispatchProvider > xProv( pViewFrame->GetFrame()->GetFrameInterface(), uno::UNO_QUERY );
    if ( xProv.is() )
    {
!!! (pb) we need a new config item here
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
        aURL.Complete = ::rtl::OUString( aObj.GetMainURL( INetURLObject::NO_DECODE ) );

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
*/

    return( 0L );
}


