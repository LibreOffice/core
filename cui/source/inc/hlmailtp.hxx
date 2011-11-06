/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
