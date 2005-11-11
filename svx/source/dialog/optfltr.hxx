/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optfltr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-11 13:55:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_SIMPTABL_HXX //autogen
#include "simptabl.hxx"
#endif



class OfaMSFilterTabPage : public SfxTabPage
{
    FixedLine       aMSWordGB;
    CheckBox        aWBasicCodeCB;
    CheckBox        aWBasicStgCB;
    FixedLine       aMSExcelGB;
    CheckBox        aEBasicCodeCB;
    CheckBox        aEBasicStgCB;
    FixedLine       aMSPPointGB;
    CheckBox        aPBasicCodeCB;
    CheckBox        aPBasicStgCB;

    OfaMSFilterTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaMSFilterTabPage();
public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

class OfaMSFilterTabPage2 : public SfxTabPage
{
    class MSFltrSimpleTable : public SvxSimpleTable
    {
        void            CheckEntryPos(ULONG nPos, USHORT nCol, BOOL bChecked);
        SvButtonState   GetCheckButtonState( SvLBoxEntry*, USHORT nCol ) const;
        void            SetCheckButtonState( SvLBoxEntry*, USHORT nCol, SvButtonState );
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
    Bitmap aChkunBmp, aChkchBmp, aChkchhiBmp,
           aChkunhiBmp, aChktriBmp, aChktrihiBmp;
    String sHeader1, sHeader2;
    String sChgToFromMath,
           sChgToFromWriter,
           sChgToFromCalc,
           sChgToFromImpress;
    SvLBoxButtonData*   pCheckButtonData;

    OfaMSFilterTabPage2( Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaMSFilterTabPage2();

    void            InsertEntry( const String& _rTxt, sal_Int32 _nType );
    SvLBoxEntry*    GetEntry4Type( sal_Int32 _nType ) const;

public:

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};


#endif //


