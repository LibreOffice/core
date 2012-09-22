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


#ifndef _ABOUT_HXX
#define _ABOUT_HXX

// include ---------------------------------------------------------------

#include <vcl/button.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/fixedhyper.hxx>
#include <tools/list.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>        // SfxModalDialog

// class AboutDialog -----------------------------------------------------

class AboutDialog : public SfxModalDialog
{
private:
    OKButton            maOKButton;
    PushButton          maReadmeButton;
    FixedInfo           maVersionText;
    MultiLineEdit       maBuildInfoEdit;
    MultiLineEdit       maCopyrightEdit;
    svt::FixedHyperlink maCreditsLink;

    String              maCopyrightTextStr;
    rtl::OUString       maVersionData;

    Image               maMainLogo;
    Image               maAppLogo;

    void                InitControls();
    void                ApplyStyleSettings();
    void                LayoutControls( Size& aDlgSize );

    const rtl::OUString GetBuildId() const;
    const rtl::OUString GetBuildVersionString() const;
    Point               maAppLogoPos;
    Point               maMainLogoPos;

    DECL_DLLPRIVATE_LINK(OpenLinkHdl_Impl, svt::FixedHyperlink* );
    DECL_DLLPRIVATE_LINK(ShowReadme_Impl, PushButton* );

protected:
    virtual sal_Bool    Close();
    virtual void        Paint( const Rectangle& );

public:
    AboutDialog( Window* pParent, const ResId& nId );
    ~AboutDialog();

};

#endif // #ifndef _ABOUT_HXX


