/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hlmailtp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:15:20 $
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
#ifndef _SVX_TABPAGE_MAIL_HYPERLINK_HXX
#define _SVX_TABPAGE_MAIL_HYPERLINK_HXX

#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - Mail & News
|*
\************************************************************************/

class SvxHyperlinkMailTp : public SvxHyperlinkTabPageBase
{
private:
    FixedLine           maGrpMailNews;
    RadioButton         maRbtMail;
    RadioButton         maRbtNews;
    FixedText           maFtReceiver;
    SvxHyperURLBox      maCbbReceiver;
    ImageButton         maBtAdrBook;
    FixedText           maFtSubject;
    Edit                maEdSubject;

    DECL_LINK (Click_SmartProtocol_Impl, void * );      // Radiobutton clicked: Type EMAIl or News
    DECL_LINK (ClickAdrBookHdl_Impl  , void * );        // Button : Address book
    DECL_LINK (ModifiedReceiverHdl_Impl, void * );      // Combobox "receiver" modified

    void    SetScheme( const String& aScheme );
    void    RemoveImproperProtocol(const String& aProperScheme);
    String  GetSchemeFromButtons() const;
    INetProtocol GetSmartProtocolFromButtons() const;

    String  CreateAbsoluteURL() const;

protected:
    virtual void FillDlgFields     ( String& aStrURL );
    virtual void GetCurentItemData ( String& aStrURL, String& aStrName,
                                     String& aStrIntName, String& aStrFrame,
                                     SvxLinkInsertMode& eMode );

public:
    SvxHyperlinkMailTp ( Window *pParent, const SfxItemSet& rItemSet);
    ~SvxHyperlinkMailTp ();

    static  IconChoicePage* Create( Window* pWindow, const SfxItemSet& rItemSet );

    virtual void        SetInitFocus();
};


#endif // _SVX_TABPAGE_MAIL_HYPERLINK_HXX
