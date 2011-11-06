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


#ifndef _CUI_SRCHDLG_HXX
#define _CUI_SRCHDLG_HXX

// include ---------------------------------------------------------------


#include <svtools/stdctrl.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>
#include <svl/svarray.hxx>
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#include <svtools/svmedit.hxx>
#include <svl/srchdefs.hxx>

class SvxJSearchOptionsPage;

class SvxJSearchOptionsDialog : public SfxSingleTabDialog
{
    sal_Int32                   nInitialTlFlags;
    SvxJSearchOptionsPage  *pPage;

    // disallow copy-constructor and assignment-operator for now
    SvxJSearchOptionsDialog( const SvxJSearchOptionsDialog & );
    SvxJSearchOptionsDialog & operator == ( const SvxJSearchOptionsDialog & );

public:
    SvxJSearchOptionsDialog( Window *pParent,
                            const SfxItemSet& rOptionsSet,
                            sal_Int32 nInitialFlags  );
    virtual ~SvxJSearchOptionsDialog();

    // Window
    virtual void    Activate();

    sal_Int32           GetTransliterationFlags() const;
};

#endif
