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


#ifndef _OFA_OPTFLTR_HXX
#define _OFA_OPTFLTR_HXX

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/simptabl.hxx>



class OfaMSFilterTabPage : public SfxTabPage
{
    FixedLine       aMSWordGB;
    CheckBox        aWBasicCodeCB;
    CheckBox        aWBasicStgCB;
    FixedLine       aMSExcelGB;
    CheckBox        aEBasicCodeCB;
    CheckBox        aEBasicExectblCB;
    CheckBox        aEBasicStgCB;
    FixedLine       aMSPPointGB;
    CheckBox        aPBasicCodeCB;
    CheckBox        aPBasicStgCB;

    OfaMSFilterTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaMSFilterTabPage();

    DECL_LINK( LoadExcelBasicCheckHdl_Impl, CheckBox* );
public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

class OfaMSFilterTabPage2 : public SfxTabPage
{
    class MSFltrSimpleTable : public SvxSimpleTable
    {
        using SvTreeListBox::GetCheckButtonState;
        using SvTreeListBox::SetCheckButtonState;
        using SvxSimpleTable::SetTabs;

        void            CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, bool bChecked);
        SvButtonState   GetCheckButtonState( SvLBoxEntry*, sal_uInt16 nCol ) const;
        void            SetCheckButtonState( SvLBoxEntry*, sal_uInt16 nCol, SvButtonState );
    protected:
        virtual void    SetTabs();
        virtual void    HBarClick();
        virtual void    KeyInput( const KeyEvent& rKEvt );

    public:
        MSFltrSimpleTable(Window* pParent, const ResId& rResId ) :
            SvxSimpleTable( pParent, rResId ){}
    };

    MSFltrSimpleTable aCheckLB;
    FixedText aHeader1FT, aHeader2FT;
    String sHeader1, sHeader2;
    String sChgToFromMath,
           sChgToFromWriter,
           sChgToFromCalc,
           sChgToFromImpress;
    SvLBoxButtonData*   pCheckButtonData;

    OfaMSFilterTabPage2( Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaMSFilterTabPage2();

    void            InsertEntry( const String& _rTxt, sal_IntPtr _nType );
    SvLBoxEntry*    GetEntry4Type( sal_IntPtr _nType ) const;

public:

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};


#endif //


