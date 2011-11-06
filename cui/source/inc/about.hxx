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

#include <tools/resary.hxx>
#include <vcl/button.hxx>
#include <vcl/accel.hxx>
#include <svtools/svmedit.hxx>
#include <tools/list.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>        // SfxModalDialog

DECLARE_LIST( AccelList, Accelerator* )

// class AboutDialog -----------------------------------------------------

class AboutDialog : public SfxModalDialog
{
private:
    OKButton        aOKButton;
    Image           aAppLogo;

    FixedInfo       aVersionText;
    MultiLineEdit   aCopyrightText;
    FixedInfo       aBuildData;

    ResStringArray* pDeveloperAry;
    String          aDevVersionStr;
    String          aAccelStr;
    String          aVersionData;
    String          aCopyrightTextStr;

    AccelList       aAccelList;

    AutoTimer       aTimer;
    long            nOff;
    long            m_nDeltaWidth;
    int             m_nPendingScrolls;

    sal_Bool            bNormal;

protected:
    virtual sal_Bool    Close();
    virtual void    Paint( const Rectangle& );

public:
    AboutDialog( Window* pParent, const ResId& nId );
    ~AboutDialog();

    DECL_LINK( TimerHdl, Timer * );
    DECL_LINK( AccelSelectHdl, Accelerator * );
};

#endif // #ifndef _ABOUT_HXX


