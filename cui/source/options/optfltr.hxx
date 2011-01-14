/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

        void            CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, sal_Bool bChecked);
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


