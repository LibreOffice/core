/*************************************************************************
 *
 *  $RCSfile: pszctrl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-23 09:32:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#include <limits.h>

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#pragma hdrstop

#define _SVX_PSZCTRL_CXX

#include "pszctrl.hxx"

#define PAINT_OFFSET    5

#define ITEMID_SIZE     SID_ATTR_SIZE
#include "sizeitem.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"
#include "stbctrls.h"

#include "dialogs.hrc"

// -----------------------------------------------------------------------

/*  [Beschreibung]

    Funktion, mit der ein metrischer Wert in textueller Darstellung
    umgewandelt wird.

    nVal ist hier der metrische Wert in der Einheit eUnit.

    [Querverweise]

    <SvxPosSizeStatusBarControl::Paint(const UserDrawEvent&)>
*/

String GetMetricStr_Impl( long nVal, SfxMapUnit eUnit )
{
    // Applikations-Metrik besorgen und setzen
    FieldUnit eOutUnit = GetModuleFieldUnit( NULL );
    FieldUnit eInUnit = FUNIT_TWIP;

    if ( SFX_MAPUNIT_100TH_MM == eUnit )
        eInUnit = FUNIT_100TH_MM;

    String sMetric;
    char cSep = GetpApp()->GetAppInternational().GetNumDecimalSep();
    long nConvVal = MetricField::ConvertValue( nVal * 100, 0L, 0,
                                               eInUnit, eOutUnit );

    if ( nConvVal < 0 && ( nConvVal / 100 == 0 ) )
        sMetric += '-';
    sMetric += String::CreateFromInt32( nConvVal / 100 );

    if( FUNIT_NONE != eOutUnit )
    {
        sMetric += cSep;
        long nFract = nConvVal % 100;

        if ( nFract < 0 )
            nFract *= -1;
        if ( nFract < 10 )
            sMetric += '0';
        sMetric += String::CreateFromInt32( nFract );
    }

    return sMetric;
}

// -----------------------------------------------------------------------

SFX_IMPL_STATUSBAR_CONTROL(SvxPosSizeStatusBarControl, SvxSizeItem);

// class FunctionPopup_Impl ----------------------------------------------

class FunctionPopup_Impl : public PopupMenu
{
public:
    FunctionPopup_Impl( USHORT nCheck );

    USHORT          GetSelected() const { return nSelected; }

private:
    USHORT          nSelected;

    virtual void    Select();
};

// -----------------------------------------------------------------------

FunctionPopup_Impl::FunctionPopup_Impl( USHORT nCheck ) :
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
    BOOL    bTime;      // Zeit und Datum anzeigen? (nichts anzeigen (#65302#))
    BOOL    bSize;      // Gr"o/se anzeigen?
    BOOL    bTable;     // Tabellenindex anzeigen?
    BOOL    bHasMenu;   // StarCalc Popup-Menue anzeigen?
    USHORT  nFunction;  // selektierte StarCalc Funktion
    Image   aPosImage;  // Image f"ur die Positionsanzeige
    Image   aSizeImage; // Image f"ur die Gr"o/senanzeige
};

// class SvxPosSizeStatusBarControl ------------------------------------------

/*  [Beschreibung]

    Ctor():
    Anlegen einer Impl-Klassen-Instanz, Default die Zeitanzeige enablen,
    Images fu"r die Position und Gro"sse laden.
*/

SvxPosSizeStatusBarControl::SvxPosSizeStatusBarControl( USHORT nId,
                                                        StatusBar& rStb,
                                                        SfxBindings& rBind ) :
    SfxStatusBarControl( nId, rStb, rBind ),

    aPosForwarder( SID_ATTR_POSITION, *this ),
    aTableForwarder( SID_TABLE_CELL, *this ),
    aFuncForwarder( SID_PSZ_FUNCTION, *this ),
    pImp( new SvxPosSizeStatusBarControl_Impl )

{
    pImp->bTime = TRUE;
    pImp->bSize = FALSE;
    pImp->bTable = FALSE;
    pImp->bHasMenu = FALSE;
    pImp->nFunction = 0;
    pImp->aPosImage = Image( ResId( RID_SVXBMP_POSITION, DIALOG_MGR() ) );
    pImp->aSizeImage = Image( ResId( RID_SVXBMP_SIZE, DIALOG_MGR() ) );
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
    Zeit        TRUE        FALSE           FALSE           FALSE
    Position    FALSE                                       FALSE
    Gro"sse     FALSE                       TRUE            FALSE
    Text        FALSE                       FALSE           TRUE

    Ein anderes Item bewirkt einen Assert, die Zeitanzeige wird enabled.
*/

void SvxPosSizeStatusBarControl::StateChanged( USHORT nSID, SfxItemState eState,
                                               const SfxPoolItem* pState )
{
    // da Kombi-Controller, immer die aktuelle Id als HelpId setzen
    // gecachten HelpText vorher l"oschen
    GetStatusBar().SetHelpText( GetId(), String() );
    GetStatusBar().SetHelpId( GetId(), nSID );

    if ( nSID == SID_PSZ_FUNCTION )
    {
        if ( eState == SFX_ITEM_AVAILABLE )
        {
            pImp->bHasMenu = TRUE;
            if ( pState && pState->ISA(SfxUInt16Item) )
                pImp->nFunction = ((const SfxUInt16Item*)pState)->GetValue();
        }
        else
            pImp->bHasMenu = FALSE;
    }
    else if ( SFX_ITEM_AVAILABLE != eState )
    {
        // Datum und Zeit anzeigen
        pImp->bTime = TRUE;
        pImp->bSize = FALSE;
        pImp->bTable = FALSE;
    }
    else if ( pState->ISA( SfxPointItem ) )
    {
        // Position anzeigen
        pImp->aPos = ( (SfxPointItem*)pState )->GetValue();
        pImp->bTime = FALSE;
        pImp->bTable = FALSE;
    }
    else if ( pState->ISA( SvxSizeItem ) )
    {
        // Groesse anzeigen
        pImp->aSize = ( (SvxSizeItem*)pState )->GetSize();
        pImp->bSize = TRUE;
        pImp->bTime = FALSE;
        pImp->bTable = FALSE;
    }
    else if ( pState->ISA( SfxStringItem ) )
    {
        // String anzeigen (Tabellen-Zelle oder anderes)
        pImp->aStr = ( (SfxStringItem*)pState )->GetValue();
        pImp->bTable = TRUE;
        pImp->bTime = FALSE;
        pImp->bSize = FALSE;
    }
    else
    {
        DBG_ERRORFILE( "invalid item type" );
        // trotzdem Datum und Zeit anzeigen
        pImp->bTime = TRUE;
        pImp->bSize = FALSE;
        pImp->bTable = FALSE;
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
        USHORT nSelect = pImp->nFunction;
        if (!nSelect)
            nSelect = PSZ_FUNC_NONE;
        FunctionPopup_Impl aMenu( nSelect );
        if ( aMenu.Execute( &GetStatusBar(),
                            GetStatusBar().OutputToScreenPixel( rCEvt.GetMousePosPixel() ) ) )
        {
            nSelect = aMenu.GetSelected();
            if (nSelect)
            {
                if (nSelect == PSZ_FUNC_NONE)
                    nSelect = 0;

                SfxUInt16Item aItem( SID_PSZ_FUNCTION, nSelect );
                GetBindings().GetDispatcher()->Execute( SID_PSZ_FUNCTION, SFX_CALLMODE_RECORD, &aItem, 0L );
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

    if ( pImp->bTime )
    {
        // PB: Datum und Uhrzeit nicht mehr ausgeben (#65302#)
        pDev->DrawRect( rRect );
    }
    else if ( !pImp->bTable )
    {
        // Position fuer Size-Anzeige berechnen
        long nSizePosX =
            rRect.Left() + rRect.GetWidth() / 2 + PAINT_OFFSET;
        // Metric besorgen
        SfxMapUnit eUnit = SFX_MAPUNIT_TWIP;
        SfxObjectShell* pSh = SfxObjectShell::Current();
        if ( pSh )
            eUnit = pSh->GetPool().GetMetric( SID_ATTR_SIZE );

        // Position zeichnen
        Point aPnt = rRect.TopLeft();
        aPnt.Y() = aItemPos.Y();
        aPnt.X() += PAINT_OFFSET;
        pDev->DrawImage( aPnt, pImp->aPosImage );
        aPnt.X() += pImp->aPosImage.GetSizePixel().Width();
        aPnt.X() += PAINT_OFFSET;
        String aStr = GetMetricStr_Impl( pImp->aPos.X(), eUnit );
        aStr.AppendAscii(" / ");
        aStr += GetMetricStr_Impl( pImp->aPos.Y(), eUnit );
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
            aStr = GetMetricStr_Impl( pImp->aSize.Width(), eUnit );
            aStr.AppendAscii(" x ");
            aStr += GetMetricStr_Impl( pImp->aSize.Height(), eUnit );
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

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

// -----------------------------------------------------------------------

ULONG SvxPosSizeStatusBarControl::GetDefItemWidth(const StatusBar& rStb)
{
    Image aTmpPosImage( ResId( RID_SVXBMP_POSITION, DIALOG_MGR() ) );
    Image aTmpSizeImage( ResId( RID_SVXBMP_SIZE, DIALOG_MGR() ) );

    ULONG nWidth=PAINT_OFFSET+aTmpPosImage.GetSizePixel().Width();
    nWidth+=PAINT_OFFSET+aTmpSizeImage.GetSizePixel().Width();
    nWidth+=2*(PAINT_OFFSET+rStb.GetTextWidth(String::CreateFromAscii("XXXX,XX / XXXX,XX")));

    return nWidth;
}


