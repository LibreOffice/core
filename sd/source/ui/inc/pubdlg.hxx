/*************************************************************************
 *
 *  $RCSfile: pubdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-26 20:23:12 $
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


#ifndef _SD_PUBDLG_HXX
#define _SD_PUBDLG_HXX

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SD_RESLTN_HXX
#include "resltn.hxx"       // enum PublishingResolution
#endif

#ifndef INC_ASSCLASS
#include "assclass.hxx"
#endif

#define NOOFPAGES 6

enum HtmlPublishMode { PUBLISH_HTML, PUBLISH_FRAMES, PUBLISH_WEBCAST, PUBLISH_KIOSK };

class SfxItemSet;

class FixedText;
class RadioButton;
class ListBox;
class ComboBox;
class Edit;
class MultiLineEdit;
class ValueSet;
class SdHtmlAttrPreview;
class List;
class SdPublishingDesign;

// *********************************************************************
// Html-Export Autopilot
// *********************************************************************

class SdPublishingDlg : public ModalDialog
{
private:
    // page 1 controls
    FixedBitmap*    pPage1_Bmp;
    GroupBox*       pPage1_Titel;
    RadioButton*    pPage1_NewDesign;
    RadioButton*    pPage1_OldDesign;
    ListBox*        pPage1_Designs;
    PushButton*     pPage1_DelDesign;

    // page 2 controls
    FixedBitmap*    pPage2_Bmp;
    GroupBox*       pPage2_Titel;
    RadioButton*    pPage2_Standard;
    RadioButton*    pPage2_Frames;
    RadioButton*    pPage2_Kiosk;
    RadioButton*    pPage2_WebCast;
    FixedBitmap*    pPage2_Standard_FB;
    FixedBitmap*    pPage2_Frames_FB;
    FixedBitmap*    pPage2_Kiosk_FB;
    FixedBitmap*    pPage2_WebCast_FB;

    GroupBox*       pPage2_Titel_Html;
    CheckBox*       pPage2_Content;
    CheckBox*       pPage2_Notes;

    GroupBox*       pPage2_Titel_WebCast;
    RadioButton*    pPage2_ASP;
    RadioButton*    pPage2_PERL;
    FixedText*      pPage2_URL_txt;
    Edit*           pPage2_URL;
    FixedText*      pPage2_CGI_txt;
    Edit*           pPage2_CGI;
    FixedText*      pPage2_Index_txt;
    Edit*           pPage2_Index;

    GroupBox*       pPage2_Titel_Kiosk;
    RadioButton*    pPage2_ChgDefault;
    RadioButton*    pPage2_ChgAuto;
    FixedText*      pPage2_Duration_txt;
    TimeField*      pPage2_Duration;
    CheckBox*       pPage2_Endless;

    // page 3 controls
    FixedBitmap*    pPage3_Bmp;
    GroupBox*       pPage3_Titel1;
    RadioButton*    pPage3_Gif;
    RadioButton*    pPage3_Jpg;
    FixedText*      pPage3_Quality_txt;
    ComboBox*       pPage3_Quality;
    GroupBox*       pPage3_Titel2;
    RadioButton*    pPage3_Resolution_1;
    RadioButton*    pPage3_Resolution_2;
    RadioButton*    pPage3_Resolution_3;
    GroupBox*       pPage3_Titel3;
    CheckBox*       pPage3_SldSound;
    // page 4 controls

    FixedBitmap*    pPage4_Bmp;
    GroupBox*       pPage4_Titel1;
    FixedText*      pPage4_Author_txt;
    Edit*           pPage4_Author;
    FixedText*      pPage4_Email_txt;
    Edit*           pPage4_Email;
    FixedText*      pPage4_WWW_txt;
    Edit*           pPage4_WWW;
    FixedText*      pPage4_Titel2;
    MultiLineEdit*  pPage4_Misc;
    CheckBox*       pPage4_Download;
//-/    CheckBox*       pPage4_Created;

    // page 5 controls

    FixedBitmap*    pPage5_Bmp;
    GroupBox*       pPage5_Titel;
    CheckBox*       pPage5_TextOnly;
    ValueSet*       pPage5_Buttons;

    // page 6 controls

    FixedBitmap*    pPage6_Bmp;
    GroupBox*       pPage6_Titel;
    RadioButton*    pPage6_Default;
    RadioButton*    pPage6_User;
    PushButton*     pPage6_Back;
    PushButton*     pPage6_Text;
    PushButton*     pPage6_Link;
    PushButton*     pPage6_VLink;
    PushButton*     pPage6_ALink;
    RadioButton*    pPage6_DocColors;
    SdHtmlAttrPreview*  pPage6_Preview;

//  CheckBox*       pPage6_Sound;

    // standard controls
    OKButton        aFinishButton;
    CancelButton    aCancelButton;
    HelpButton      aHelpButton;
    PushButton      aNextPageButton;
    PushButton      aLastPageButton;

    Assistent       aAssistentFunc;

    BOOL            m_bImpress;
    BOOL            m_bButtonsDirty;

    void SetDefaults();
    void CreatePages();
    void RemovePages();

    Color m_aBackColor, m_aTextColor, m_aLinkColor;
    Color m_aVLinkColor, m_aALinkColor;

    void    ChangePage();
    void    UpdatePage();

    List*   m_pDesignList;
    BOOL    m_bDesignListDirty;
    SdPublishingDesign* m_pDesign;
    BOOL    Load();
    BOOL    Save();

    void    GetDesign( SdPublishingDesign* pDesign );
    void    SetDesign( SdPublishingDesign* pDesign );

    void    LoadPreviewButtons();

    DECL_LINK( FinishHdl, OKButton * );
    DECL_LINK( NextPageHdl, PushButton * );
    DECL_LINK( LastPageHdl, PushButton * );

    DECL_LINK( DesignHdl, RadioButton * );
    DECL_LINK( DesignSelectHdl, ListBox * );
    DECL_LINK( DesignDeleteHdl, PushButton * );
    DECL_LINK( BaseHdl, RadioButton * );
    DECL_LINK( ContentHdl, RadioButton * );
    DECL_LINK( GfxFormatHdl, RadioButton * );
    DECL_LINK( ResolutionHdl, RadioButton * );
    DECL_LINK( ButtonsHdl, ValueSet* );
    DECL_LINK( ColorHdl, PushButton * );
    DECL_LINK( WebServerHdl, RadioButton * );
    DECL_LINK( SlideChgHdl, RadioButton* );

public:

    SdPublishingDlg(Window* pWindow, DocumentType eDocType);
    ~SdPublishingDlg();

    void FillItemSet( SfxItemSet& aSet );
};

#endif // _SD_PUBDLG_HXX

