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


#ifndef _SVX_RECOVER_HXX
#define _SVX_RECOVER_HXX

#include <layout/layout.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <layout/core/box.hxx>
#include <layout/layout-pre.hxx>

class SvxRecoverDialog : public SfxModalDialog
{
private:
    FixedImage aHeaderImage;
    FixedText aHeaderText;
    FixedLine aHeaderLine;
    FixedText aRecoverText;
    FixedText aTextAdvanced;

    CheckBox aCheckBoxDoc;
    FixedImage aImageDoc;
    FixedText aTextDoc;
    CheckBox aCheckBoxSheet;
    FixedImage aImageSheet;
    FixedText aTextSheet;
    CheckBox aCheckBoxDraw;
    FixedImage aImageDraw;
    FixedText aTextDraw;
    CheckBox aCheckBoxPresent;
    FixedImage aImagePresent;
    FixedText aTextPresent;
    AdvancedButton aButtonAdvanced;

    FixedText aProgressText;
    ProgressBar aProgressBar;
    CheckBox aCheckBoxLogFile;
    OKButton aOKBtn;
    CancelButton aCancelBtn;
    HelpButton aHelpBtn;

public:
#if TEST_LAYOUT
    SvxRecoverDialog( Window* pParent );
#else /* !TEST_LAYOUT */
    SvxRecoverDialog( Window* pParent, const SfxItemSet& rCoreSet );
#endif /* !TEST_LAYOUT */
    ~SvxRecoverDialog();
};

#include <layout/layout-post.hxx>

#endif

