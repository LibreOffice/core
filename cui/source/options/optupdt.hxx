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



#ifndef _SVX_OPTUPDT_HXX
#define _SVX_OPTUPDT_HXX

// include ---------------------------------------------------------------

#include <sfx2/tabdlg.hxx>
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <com/sun/star/container/XNameReplace.hpp>

// class SvxPathTabPage --------------------------------------------------

class SvxOnlineUpdateTabPage : public SfxTabPage
{
private:
    FixedLine           m_aOptionsLine;
    CheckBox            m_aAutoCheckCheckBox;
    RadioButton         m_aEveryDayButton;
    RadioButton         m_aEveryWeekButton;
    RadioButton         m_aEveryMonthButton;
    PushButton          m_aCheckNowButton;
    CheckBox            m_aAutoDownloadCheckBox;
    FixedText           m_aDestPathLabel;
    FixedText           m_aDestPath;
    PushButton          m_aChangePathButton;
    FixedText           m_aLastChecked;
    rtl::OUString       m_aNeverChecked;
    rtl::OUString       m_aLastCheckedTemplate;

    DECL_LINK(  FileDialogHdl_Impl, PushButton* ) ;
    DECL_LINK(  CheckNowHdl_Impl, PushButton* ) ;
    DECL_LINK(  AutoCheckHdl_Impl, CheckBox* ) ;

    void        CalcButtonWidth();

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > m_xUpdateAccess;

    void                    UpdateLastCheckedText();

public:
    SvxOnlineUpdateTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxOnlineUpdateTabPage();

    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );

    virtual sal_Bool            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );
    virtual void            FillUserData();
};


#endif

