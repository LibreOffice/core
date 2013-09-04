/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "LineWidthControl.hxx"
#include "LinePropertyPanel.hrc"
#include "LinePropertyPanel.hxx"

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <unotools/viewoptions.hxx>
#include <svx/xlnwtit.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "svx/sidebar/PopupContainer.hxx"


namespace svx { namespace sidebar {

LineWidthControl::LineWidthControl (
    Window* pParent,
    LinePropertyPanel& rPanel)
    : svx::sidebar::PopupControl(pParent,SVX_RES(RID_POPUPPANEL_LINEPAGE_WIDTH)),
      mrLinePropertyPanel(rPanel),
      mpBindings(NULL),
      maVSWidth( this, SVX_RES(VS_WIDTH)),
      maFTCus( this, SVX_RES(FT_CUSTOME)),
      maFTWidth( this, SVX_RES(FT_LINE_WIDTH)),
      maMFWidth( this, SVX_RES(MF_WIDTH)),
      meMapUnit(SFX_MAPUNIT_TWIP),
      rStr(NULL),
      mstrPT(SVX_RESSTR(STR_PT)),
      mnCustomWidth(0),
      mbCustom(false),
      mbColseByEdit(false),
      mnTmpCusomWidth(0),
      mbVSFocus(true),
      maIMGCus(SVX_RES(IMG_WIDTH_CUSTOM)),
      maIMGCusGray(SVX_RES(IMG_WIDTH_CUSTOM_GRAY))
{
    Initialize();
    FreeResource();
    mpBindings = mrLinePropertyPanel.GetBindings();
}




LineWidthControl::~LineWidthControl (void)
{
    delete[] rStr;
}




void LineWidthControl::Paint(const Rectangle& rect)
{
    svx::sidebar::PopupControl::Paint(rect);

    Color aOldLineColor = GetLineColor();
    Color aOldFillColor = GetFillColor();

    Point aPos( LogicToPixel( Point( CUSTOM_X, CUSTOM_Y), MAP_APPFONT ));
    Size aSize( LogicToPixel( Size(  CUSTOM_W, CUSTOM_H ), MAP_APPFONT ));
    Rectangle aRect( aPos, aSize );
    aRect.Left() -= 1;
    aRect.Top() -= 1;
    aRect.Right() += 1;
    aRect.Bottom() += 1;

    Color aLineColor(189,201,219);
    if(!GetSettings().GetStyleSettings().GetHighContrastMode())
        SetLineColor(aLineColor);
    else
        SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
    SetFillColor(COL_TRANSPARENT);
    DrawRect(aRect);

    SetLineColor(aOldLineColor);
    SetFillColor(aOldFillColor);
}




void LineWidthControl::Initialize()
{
    maVSWidth.SetStyle( maVSWidth.GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT  );// WB_NAMEFIELD | WB_ITEMBORDER |WB_DOUBLEBORDER | WB_NONEFIELD |
    //for high contrast wj
    if(GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        maVSWidth.SetColor(GetSettings().GetStyleSettings().GetMenuColor());
    //  maBorder.SetBackground(GetSettings().GetStyleSettings().GetMenuColor());
        maFTWidth.SetBackground(GetSettings().GetStyleSettings().GetMenuColor());
    }
    else
    {
        maVSWidth.SetColor(COL_WHITE);
    //  maBorder.SetBackground(Wallpaper(COL_WHITE));
        maFTWidth.SetBackground(Wallpaper(COL_WHITE));
    }

    sal_Int64 nFirst= maMFWidth.Denormalize( maMFWidth.GetFirst( FUNIT_TWIP ) );
    sal_Int64 nLast = maMFWidth.Denormalize( maMFWidth.GetLast( FUNIT_TWIP ) );
    sal_Int64 nMin = maMFWidth.Denormalize( maMFWidth.GetMin( FUNIT_TWIP ) );
    sal_Int64 nMax = maMFWidth.Denormalize( maMFWidth.GetMax( FUNIT_TWIP ) );
    maMFWidth.SetSpinSize( 10 );
    maMFWidth.SetUnit( FUNIT_POINT );
    if( maMFWidth.GetDecimalDigits() > 1 )
         maMFWidth.SetDecimalDigits( 1 );
    maMFWidth.SetFirst( maMFWidth.Normalize( nFirst ), FUNIT_TWIP );
    maMFWidth.SetLast( maMFWidth.Normalize( nLast ), FUNIT_TWIP );
    maMFWidth.SetMin( maMFWidth.Normalize( nMin ), FUNIT_TWIP );
    maMFWidth.SetMax( maMFWidth.Normalize( nMax ), FUNIT_TWIP );

    rStr = new XubString[9];
    //modify,
    rStr[0] = OUString("05");
    rStr[1] = OUString("08");
    rStr[2] = OUString("10");
    rStr[3] = OUString("15");
    rStr[4] = OUString("23");
    rStr[5] = OUString("30");
    rStr[6] = OUString("45");
    rStr[7] = OUString("60");
    rStr[8] = SVX_RESSTR(STR_WIDTH_LAST_CUSTOM);

    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    for(int i = 0; i <= 7 ; i++)
    {
        rStr[i] = rStr[i].Insert(cSep, 1);//Modify
        rStr[i].Append(mstrPT);
    }
    //end

    for(sal_uInt16 i = 1 ; i <= 9 ; i++)
    {
        maVSWidth.InsertItem(i);
        maVSWidth.SetItemText(i, rStr[i-1]);
    }
    maVSWidth.SetUnit(rStr);
    maVSWidth.SetItemData(1,(void*)5);
    maVSWidth.SetItemData(2,(void*)8);
    maVSWidth.SetItemData(3,(void*)10);
    maVSWidth.SetItemData(4,(void*)15);
    maVSWidth.SetItemData(5,(void*)23);
    maVSWidth.SetItemData(6,(void*)30);
    maVSWidth.SetItemData(7,(void*)45);
    maVSWidth.SetItemData(8,(void*)60);
    maVSWidth.SetImage(maIMGCusGray);

    maVSWidth.SetSelItem(0);
    Link aLink =  LINK( this, LineWidthControl, VSSelectHdl ) ;
    maVSWidth.SetSelectHdl(aLink);
    aLink = LINK(this, LineWidthControl, MFModifyHdl);
    maMFWidth.SetModifyHdl(aLink);

    maVSWidth.StartSelection();
    maVSWidth.Show();
}




void LineWidthControl::GetFocus()
{
    if(!mbVSFocus)
        maMFWidth.GrabFocus();
    else
        maVSWidth.GrabFocus();
}




ValueSet& LineWidthControl::GetValueSet()
{
    return maVSWidth;
}




void LineWidthControl::SetWidthSelect( long lValue, bool bValuable, SfxMapUnit eMapUnit)
{
    mbVSFocus = true;
    maVSWidth.SetSelItem(0);
    mbColseByEdit = false;
    meMapUnit = eMapUnit;
    SvtViewOptions aWinOpt( E_WINDOW, SIDEBAR_LINE_WIDTH_GLOBAL_VALUE );
    if ( aWinOpt.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;

        OUString aWinData( aTmp );
        mnCustomWidth = aWinData.toInt32();
        mbCustom = true;
        maVSWidth.SetImage(maIMGCus);
        maVSWidth.SetCusEnable(true);

        OUString aStrTip( OUString::number( (double)mnCustomWidth / 10));
        aStrTip += mstrPT;
        maVSWidth.SetItemText(9, aStrTip);
    }
    else
    {
        mbCustom = false;
        maVSWidth.SetImage(maIMGCusGray);
        maVSWidth.SetCusEnable(false);
        //modify
        //String aStrTip(String(SVX_RES(STR_WIDTH_LAST_CUSTOM)));
        //maVSWidth.SetItemText(9, aStrTip);
        maVSWidth.SetItemText(9, rStr[8]);
    }

    if(bValuable)
    {
        sal_Int64 nVal = OutputDevice::LogicToLogic( lValue, (MapUnit)eMapUnit, MAP_100TH_MM );
        nVal = maMFWidth.Normalize( nVal );
        maMFWidth.SetValue( nVal, FUNIT_100TH_MM );
    }
    else
    {
        maMFWidth.SetText( "" );
    }

    MapUnit eOrgUnit = (MapUnit)eMapUnit;
    MapUnit ePntUnit( MAP_TWIP );
    lValue = LogicToLogic( lValue , eOrgUnit, ePntUnit );

    OUString strCurrValue = maMFWidth.GetText();
    sal_uInt16 i = 0;
    for(; i < 8; i++)
        if(strCurrValue == rStr[i])
        {
            maVSWidth.SetSelItem(i+1);
            break;
        }
    if (i>=8)
    {
        mbVSFocus = false;
        maVSWidth.SetSelItem(0);
    }
    maVSWidth.Format();
    maVSWidth.StartSelection();
}




IMPL_LINK(LineWidthControl, VSSelectHdl, void *, pControl)
{
    if(pControl == &maVSWidth)
    {
        sal_uInt16 iPos = maVSWidth.GetSelectItemId();
        if(iPos >= 1 && iPos <= 8)
        {
            sal_IntPtr nVal = LogicToLogic((sal_IntPtr)maVSWidth.GetItemData( iPos ) , MAP_POINT, (MapUnit)meMapUnit);
            nVal = maMFWidth.Denormalize(nVal);
            XLineWidthItem aWidthItem( nVal );
            mpBindings->GetDispatcher()->Execute(SID_ATTR_LINE_WIDTH, SFX_CALLMODE_RECORD, &aWidthItem, 0L);
            mrLinePropertyPanel.SetWidthIcon(iPos);
            mrLinePropertyPanel.SetWidth(nVal);
            mbColseByEdit = false;
            mnTmpCusomWidth = 0;
        }
        else if(iPos == 9)
        {//last custom
            //modified
            if(mbCustom)
            {
                long nVal = LogicToLogic(mnCustomWidth , MAP_POINT, (MapUnit)meMapUnit);
                nVal = maMFWidth.Denormalize(nVal);
                XLineWidthItem aWidthItem( nVal );
                mpBindings->GetDispatcher()->Execute(SID_ATTR_LINE_WIDTH, SFX_CALLMODE_RECORD, &aWidthItem, 0L);
                mrLinePropertyPanel.SetWidth(nVal);
                mbColseByEdit = false;
                mnTmpCusomWidth = 0;
            }
            else
            {
                maVSWidth.SetNoSelection();     //add , set no selection and keep the last select item
                maVSWidth.Format();
                Invalidate();
                maVSWidth.StartSelection();
            }
            //modify end
        }
        if((iPos >= 1 && iPos <= 8) || (iPos == 9 && mbCustom)) //add
            mrLinePropertyPanel.EndLineWidthPopupMode();
    }
    return( 0L );
}




IMPL_LINK(LineWidthControl, MFModifyHdl, void *, pControl)
{
    if(pControl == &maMFWidth)
    {
        if(maVSWidth.GetSelItem())
        {
            maVSWidth.SetSelItem(0);
            maVSWidth.Format();
            Invalidate();
            maVSWidth.StartSelection();
        }
        long nTmp = static_cast<long>(maMFWidth.GetValue());
        long nVal = LogicToLogic( nTmp, MAP_POINT, (MapUnit)meMapUnit );
        sal_Int32 nNewWidth = (short)maMFWidth.Denormalize( nVal );
        XLineWidthItem aWidthItem(nNewWidth);
        mpBindings->GetDispatcher()->Execute(SID_ATTR_LINE_WIDTH, SFX_CALLMODE_RECORD, &aWidthItem, 0L);

        mbColseByEdit = true;
        mnTmpCusomWidth = nTmp;
        /*for(sal_uInt16 i = 0; i < 8; i++)
        {
            if(nTmp == (sal_Int32)maVSWidth.GetItemData(i))
            {
                mbColseByEdit = false;
                break;
            }
        }*/

    }
    return( 0L );
}




bool LineWidthControl::IsCloseByEdit()
{
    return mbColseByEdit;
}




long LineWidthControl::GetTmpCustomWidth()
{
    return mnTmpCusomWidth;
}



} } // end of namespace svx::sidebar

// eof
