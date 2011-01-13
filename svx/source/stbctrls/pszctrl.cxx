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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <limits.h>
#include <tools/shl.hxx>
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include <vcl/image.hxx>
#include <svl/stritem.hxx>
#include <svl/ptitem.hxx>
#include <svl/itempool.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/intitem.hxx>
#define _SVX_PSZCTRL_CXX

#include "svx/pszctrl.hxx"

#define PAINT_OFFSET    5

#include <editeng/sizeitem.hxx>
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include "stbctrls.h"
#include "sfx2/module.hxx"

#include <svx/dialogs.hrc>
#include <unotools/localedatawrapper.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

// -----------------------------------------------------------------------

/*  [Beschreibung]

    Funktion, mit der ein metrischer Wert in textueller Darstellung
    umgewandelt wird.

    nVal ist hier der metrische Wert in der Einheit eUnit.

    [Querverweise]

    <SvxPosSizeStatusBarControl::Paint(const UserDrawEvent&)>
*/

String SvxPosSizeStatusBarControl::GetMetricStr_Impl( long nVal )
{
    // Applikations-Metrik besorgen und setzen
    FieldUnit eOutUnit = SfxModule::GetCurrentFieldUnit();
    FieldUnit eInUnit = FUNIT_100TH_MM;

    String sMetric;
    const sal_Unicode cSep = Application::GetSettings().GetLocaleDataWrapper().getNumDecimalSep().GetChar(0);
    sal_Int64 nConvVal = MetricField::ConvertValue( nVal * 100, 0L, 0, eInUnit, eOutUnit );

    if ( nConvVal < 0 && ( nConvVal / 100 == 0 ) )
        sMetric += '-';
    sMetric += String::CreateFromInt64( nConvVal / 100 );

    if( FUNIT_NONE != eOutUnit )
    {
        sMetric += cSep;
        sal_Int64 nFract = nConvVal % 100;

        if ( nFract < 0 )
            nFract *= -1;
        if ( nFract < 10 )
            sMetric += '0';
        sMetric += String::CreateFromInt64( nFract );
    }

    return sMetric;
}

// -----------------------------------------------------------------------

SFX_IMPL_STATUSBAR_CONTROL(SvxPosSizeStatusBarControl, SvxSizeItem);

// class FunctionPopup_Impl ----------------------------------------------

class FunctionPopup_Impl : public PopupMenu
{
public:
    FunctionPopup_Impl( sal_uInt16 nCheck );

    sal_uInt16          GetSelected() const { return nSelected; }

private:
    sal_uInt16          nSelected;

    virtual void    Select();
};

// -----------------------------------------------------------------------

FunctionPopup_Impl::FunctionPopup_Impl( sal_uInt16 nCheck ) :
    PopupMenu( ResId( RID_SVXMNU_PSZ_FUNC, DIALOG_MGR() ) ),
    nSelected( 0 )
{
    if (nCheck)
        CheckItem( nCheck );
}

// -----------------------------------------------------------------------

void FunctionPopup_Impl::Select()
{
    nSelected = GetCurItemId();
}

// struct SvxPosSizeStatusBarControl_Impl --------------------------------

struct SvxPosSizeStatusBarControl_Impl

/*  [Beschreibung]

    Diese Implementations-Struktur der Klasse SvxPosSizeStatusBarControl
    dient der Entkopplung von "Anderungen vom exportierten Interface sowie
    der Verringerung von extern sichtbaren Symbolen.

    Eine Instanz exisitiert pro SvxPosSizeStatusBarControl-Instanz
    f"ur deren Laufzeit.
*/

{
    Point   aPos;       // g"ultig, wenn eine Position angezeigt wird
    Size    aSize;      // g"ultig, wenn eine Gr"o/se angezeigt wird
    String  aStr;       // g"ultig, wenn ein Text angezeigt wird
    sal_Bool    bPos;       // show position
    sal_Bool    bSize;      // Gr"o/se anzeigen?
    sal_Bool    bTable;     // Tabellenindex anzeigen?
    sal_Bool    bHasMenu;   // StarCalc Popup-Menue anzeigen?
    sal_uInt16  nFunction;  // selektierte StarCalc Funktion
    Image   aPosImage;  // Image f"ur die Positionsanzeige
    Image   aSizeImage; // Image f"ur die Gr"o/senanzeige
};

// class SvxPosSizeStatusBarControl ------------------------------------------

/*  [Beschreibung]

    Ctor():
    Anlegen einer Impl-Klassen-Instanz, Default die Zeitanzeige enablen,
    Images fu"r die Position und Gro"sse laden.
*/

#define STR_POSITION ".uno:Position"
#define STR_TABLECELL ".uno:StateTableCell"
#define STR_FUNC ".uno:StatusBarFunc"

SvxPosSizeStatusBarControl::SvxPosSizeStatusBarControl( sal_uInt16 _nSlotId,
                                                        sal_uInt16 _nId,
                                                        StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    pImp( new SvxPosSizeStatusBarControl_Impl )
{
    pImp->bPos = sal_False;
    pImp->bSize = sal_False;
    pImp->bTable = sal_False;
    pImp->bHasMenu = sal_False;
    pImp->nFunction = 0;
    pImp->aPosImage = Image( ResId( RID_SVXBMP_POSITION, DIALOG_MGR() ) );
    pImp->aSizeImage = Image( ResId( RID_SVXBMP_SIZE, DIALOG_MGR() ) );

    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( STR_POSITION )));         // SID_ATTR_POSITION
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( STR_TABLECELL )));   // SID_TABLE_CELL
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( STR_FUNC )));    // SID_PSZ_FUNCTION
}

// -----------------------------------------------------------------------

/*  [Beschreibung]

    Dtor():
    Pointer auf die Impl-Klasse lo"schen, damit der Timer gestoppt wird.
*/

SvxPosSizeStatusBarControl::~SvxPosSizeStatusBarControl()
{
    delete pImp;
}

// -----------------------------------------------------------------------

/*  [Beschreibung]

    SID_PSZ_FUNCTION aktiviert das Popup-Menue fuer Calc, ansonsten:

    Statusbenachrichtigung;
    Je nach Item-Typ wird eine bestimmte Anzeige enabled, die anderen disabled.

                NULL/Void   SfxPointItem    SvxSizeItem     SfxStringItem
    ------------------------------------------------------------------------
    Zeit        sal_True        sal_False           sal_False           FALSE
    Position    sal_False                                       FALSE
    Gro"sse     FALSE                       TRUE            FALSE
    Text        sal_False                       sal_False           TRUE

    Ein anderes Item bewirkt einen Assert, die Zeitanzeige wird enabled.
*/

void SvxPosSizeStatusBarControl::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                               const SfxPoolItem* pState )
{
    // da Kombi-Controller, immer die aktuelle Id als HelpId setzen
    // gecachten HelpText vorher l"oschen
    GetStatusBar().SetHelpText( GetId(), String() );

    switch ( nSID )
    {
        case SID_ATTR_POSITION : GetStatusBar().SetHelpId( GetId(), STR_POSITION ); break;
        case SID_TABLE_CELL: GetStatusBar().SetHelpId( GetId(), STR_TABLECELL ); break;
        case SID_PSZ_FUNCTION: GetStatusBar().SetHelpId( GetId(), STR_FUNC ); break;
        default: break;
    }

    if ( nSID == SID_PSZ_FUNCTION )
    {
        if ( eState == SFX_ITEM_AVAILABLE )
        {
            pImp->bHasMenu = sal_True;
            if ( pState && pState->ISA(SfxUInt16Item) )
                pImp->nFunction = ((const SfxUInt16Item*)pState)->GetValue();
        }
        else
            pImp->bHasMenu = sal_False;
    }
    else if ( SFX_ITEM_AVAILABLE != eState )
    {
        // #i34458# don't switch to empty display before an empty state was
        // notified for all display types

        if ( nSID == SID_TABLE_CELL )
            pImp->bTable = sal_False;
        else if ( nSID == SID_ATTR_POSITION )
            pImp->bPos = sal_False;
        else if ( nSID == GetSlotId() )     // controller is registered for SID_ATTR_SIZE
            pImp->bSize = sal_False;
        else
        {
            DBG_ERRORFILE("unknown slot id");
        }
    }
    else if ( pState->ISA( SfxPointItem ) )
    {
        // Position anzeigen
        pImp->aPos = ( (SfxPointItem*)pState )->GetValue();
        pImp->bPos = sal_True;
        pImp->bTable = sal_False;
    }
    else if ( pState->ISA( SvxSizeItem ) )
    {
        // Groesse anzeigen
        pImp->aSize = ( (SvxSizeItem*)pState )->GetSize();
        pImp->bSize = sal_True;
        pImp->bTable = sal_False;
    }
    else if ( pState->ISA( SfxStringItem ) )
    {
        // String anzeigen (Tabellen-Zelle oder anderes)
        pImp->aStr = ( (SfxStringItem*)pState )->GetValue();
        pImp->bTable = sal_True;
        pImp->bPos = sal_False;
        pImp->bSize = sal_False;
    }
    else
    {
        DBG_ERRORFILE( "invalid item type" );
        // trotzdem Datum und Zeit anzeigen
        pImp->bPos = sal_False;
        pImp->bSize = sal_False;
        pImp->bTable = sal_False;
    }

    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );

    //  nur Strings auch als Text an der StatusBar setzen, damit Tip-Hilfe
    //  funktioniert, wenn der Text zu lang ist.
    String aText;
    if ( pImp->bTable )
        aText = pImp->aStr;
    GetStatusBar().SetItemText( GetId(), aText );
}

// -----------------------------------------------------------------------

/*  [Beschreibung]

    Popup-Menue ausfuehren, wenn per Status enabled
*/

void SvxPosSizeStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU && pImp->bHasMenu )
    {
        sal_uInt16 nSelect = pImp->nFunction;
        if (!nSelect)
            nSelect = PSZ_FUNC_NONE;
        FunctionPopup_Impl aMenu( nSelect );
        if ( aMenu.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel() ) )
        {
            nSelect = aMenu.GetSelected();
            if (nSelect)
            {
                if (nSelect == PSZ_FUNC_NONE)
                    nSelect = 0;

                ::com::sun::star::uno::Any a;
                SfxUInt16Item aItem( SID_PSZ_FUNCTION, nSelect );

                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
                aArgs[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StatusBarFunc" ));
                aItem.QueryValue( a );
                aArgs[0].Value = a;

                execute( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:StatusBarFunc" )), aArgs );
//              GetBindings().GetDispatcher()->Execute( SID_PSZ_FUNCTION, SFX_CALLMODE_RECORD, &aItem, 0L );
            }
        }
    }
    else
        SfxStatusBarControl::Command( rCEvt );
}

// -----------------------------------------------------------------------

/*  [Beschreibung]

    Je nach enableden Anzeigentyp, wird der Wert angezeigt. Vorher wird
    das Rectangle u"bermalt (gelo"scht).
*/

void SvxPosSizeStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    OutputDevice* pDev = rUsrEvt.GetDevice();
    DBG_ASSERT( pDev, "no OutputDevice on UserDrawEvent" );
    const Rectangle& rRect = rUsrEvt.GetRect();
    StatusBar& rBar = GetStatusBar();
    Point aItemPos = rBar.GetItemTextPos( GetId() );
    Color aOldLineColor = pDev->GetLineColor();
    Color aOldFillColor = pDev->GetFillColor();
    pDev->SetLineColor();
    pDev->SetFillColor( pDev->GetBackground().GetColor() );

    if ( pImp->bPos || pImp->bSize )
    {
        // Position fuer Size-Anzeige berechnen
        long nSizePosX =
            rRect.Left() + rRect.GetWidth() / 2 + PAINT_OFFSET;
        // Position zeichnen
        Point aPnt = rRect.TopLeft();
        aPnt.Y() = aItemPos.Y();
        aPnt.X() += PAINT_OFFSET;
        pDev->DrawImage( aPnt, pImp->aPosImage );
        aPnt.X() += pImp->aPosImage.GetSizePixel().Width();
        aPnt.X() += PAINT_OFFSET;
        String aStr = GetMetricStr_Impl( pImp->aPos.X());
        aStr.AppendAscii(" / ");
        aStr += GetMetricStr_Impl( pImp->aPos.Y());
        pDev->DrawRect(
            Rectangle( aPnt, Point( nSizePosX, rRect.Bottom() ) ) );
        pDev->DrawText( aPnt, aStr );

        // falls verf"ugbar, Gr"osse zeichnen
        aPnt.X() = nSizePosX;

        if ( pImp->bSize )
        {
            pDev->DrawImage( aPnt, pImp->aSizeImage );
            aPnt.X() += pImp->aSizeImage.GetSizePixel().Width();
            Point aDrwPnt = aPnt;
            aPnt.X() += PAINT_OFFSET;
            aStr = GetMetricStr_Impl( pImp->aSize.Width() );
            aStr.AppendAscii(" x ");
            aStr += GetMetricStr_Impl( pImp->aSize.Height() );
            pDev->DrawRect( Rectangle( aDrwPnt, rRect.BottomRight() ) );
            pDev->DrawText( aPnt, aStr );
        }
        else
            pDev->DrawRect( Rectangle( aPnt, rRect.BottomRight() ) );
    }
    else if ( pImp->bTable )
    {
        pDev->DrawRect( rRect );
        pDev->DrawText( Point(
            rRect.Left() + rRect.GetWidth() / 2 - pDev->GetTextWidth( pImp->aStr ) / 2,
            aItemPos.Y() ), pImp->aStr );
    }
    else
    {
        // Empty display if neither size nor table position are available.
        // Date/Time are no longer used (#65302#).
        pDev->DrawRect( rRect );
    }

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

// -----------------------------------------------------------------------

sal_uIntPtr SvxPosSizeStatusBarControl::GetDefItemWidth(const StatusBar& rStb)
{
    Image aTmpPosImage( ResId( RID_SVXBMP_POSITION, DIALOG_MGR() ) );
    Image aTmpSizeImage( ResId( RID_SVXBMP_SIZE, DIALOG_MGR() ) );

    sal_uIntPtr nWidth=PAINT_OFFSET+aTmpPosImage.GetSizePixel().Width();
    nWidth+=PAINT_OFFSET+aTmpSizeImage.GetSizePixel().Width();
    nWidth+=2*(PAINT_OFFSET+rStb.GetTextWidth(String::CreateFromAscii("XXXX,XX / XXXX,XX")));

    return nWidth;
}


