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


#ifndef _SVX_OPTPATH_HXX
#define _SVX_OPTPATH_HXX

// include ---------------------------------------------------------------

#include <sfx2/tabdlg.hxx>
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifdef _SVX_OPTPATH_CXX
#include <svtools/headbar.hxx>
#else
class HeaderBar;
#endif
#include "ControlFocusHelper.hxx"

#ifndef _COM_SUN_STAR_UI_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#include <svtools/dialogclosedlistener.hxx>

// forward ---------------------------------------------------------------

class SvTabListBox;
namespace svx
{
    class OptHeaderTabListBox;
}
struct OptPath_Impl;
class SvxPathTabPage;

// define ----------------------------------------------------------------

#define SfxPathTabPage SvxPathTabPage

// class SvxPathTabPage --------------------------------------------------

class SvxPathTabPage : public SfxTabPage
{
private:
    FixedLine           aStdBox;
    FixedText           aTypeText;
    FixedText           aPathText;
    SvxControlFocusHelper aPathCtrl;
    PushButton          aStandardBtn;
    PushButton          aPathBtn;

    HeaderBar*                  pHeaderBar;
    ::svx::OptHeaderTabListBox* pPathBox;
    OptPath_Impl*               pImpl;

    ::com::sun::star::uno::Reference< ::svt::DialogClosedListener > xDialogListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFolderPicker > xFolderPicker;

#ifdef _SVX_OPTPATH_CXX
    void        ChangeCurrentEntry( const String& _rFolder );

    DECL_LINK(  PathHdl_Impl, PushButton * );
    DECL_LINK(  StandardHdl_Impl, PushButton * );

    DECL_LINK(  PathSelect_Impl, svx::OptHeaderTabListBox * );
    DECL_LINK(  HeaderSelect_Impl, HeaderBar * );
    DECL_LINK(  HeaderEndDrag_Impl, HeaderBar * );

    DECL_LINK( DialogClosedHdl, ::com::sun::star::ui::dialogs::DialogClosedEvent* );

    void        GetPathList( sal_uInt16 _nPathHandle, String& _rInternalPath,
                             String& _rUserPath, String& _rWritablePath, sal_Bool& _rReadOnly );
    void        SetPathList( sal_uInt16 _nPathHandle,
                             const String& _rUserPath, const String& _rWritablePath );
#endif

public:
    SvxPathTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxPathTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        FillUserData();
};

#endif

