/*************************************************************************
 *
 *  $RCSfile: tp_Scale.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:27 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "tp_Scale.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "Strings.hrc"
#include "SchSfxItemIds.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
// header for class SvxDoubleItem
#define ITEMID_DOUBLE   0
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif
// header for class SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
// header for SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

// header for class WarningBox
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

/*
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

// header for class Table
#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif
*/
// header for class SvNumberformat
#ifndef _ZFORMAT_HXX
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <svtools/zformat.hxx>
#endif
/*
// header for SvNumberFormatter
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#include "schattr.hxx"
#define ITEMID_DOUBLE   0
#ifndef _SVX_CHRTITEM_HXX //autogen
#include <svx/chrtitem.hxx>
#endif

#include "schresid.hxx"
#include "strings.hrc"
#include "chtmodel.hxx"
#include "attrib.hxx"
#include "tpscaley.hxx"
#include "tpscaley.hrc"

#include "chaxis.hxx"

#include <rtl/math.hxx>
*/
//.............................................................................
namespace chart
{
//.............................................................................

SchScaleYAxisTabPage::SchScaleYAxisTabPage(Window* pWindow,const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_SCALE_Y), rInAttrs),

    aFlScale(this, ResId(FL_SCALE_Y)),
    aTxtMin (this, ResId (TXT_MIN)),
    aFmtFldMin(this, ResId(EDT_MIN)),
    aCbxAutoMin(this, ResId(CBX_AUTO_MIN)),
    aTxtMax(this, ResId (TXT_MAX)),
    aFmtFldMax(this, ResId(EDT_MAX)),
    aCbxAutoMax(this, ResId(CBX_AUTO_MAX)),
    aTxtMain (this, ResId (TXT_STEP_MAIN)),
    aFmtFldStepMain(this, ResId(EDT_STEP_MAIN)),
    aCbxAutoStepMain(this, ResId(CBX_AUTO_STEP_MAIN)),
    aTxtHelp (this, ResId (TXT_STEP_HELP)),
//  aFmtFldStepHelp(this, ResId(EDT_STEP_HELP)),
    aMtStepHelp (this, ResId (MT_STEPHELP)),
    aCbxAutoStepHelp(this, ResId(CBX_AUTO_STEP_HELP)),
    aTxtOrigin (this, ResId (TXT_ORIGIN)),
    aFmtFldOrigin(this, ResId(EDT_ORIGIN)),
    aCbxAutoOrigin(this, ResId(CBX_AUTO_ORIGIN)),
    aCbxLogarithm(this, ResId(CBX_LOGARITHM)),

    aFlTicks(this,ResId(FL_TICKS)),
    aCbxTicksInner(this, ResId(CBX_TICKS_INNER)),
    aCbxTicksOuter(this, ResId(CBX_TICKS_OUTER)),

    aFlHelpTicks(this,ResId(FL_HELPTICKS)),
    aCbxHelpTicksInner(this, ResId(CBX_HELPTICKS_INNER)),
    aCbxHelpTicksOuter(this, ResId(CBX_HELPTICKS_OUTER)),

    fMin(0.0),
    fMax(0.0),
    fStepMain(0.0),
    nStepHelp(0),
    fOrigin(0.0),
    nAxisType(0),
    rOutAttrs(rInAttrs),
    pNumFormatter(NULL) //,
//  bLastStepMain(FALSE)
{
    FreeResource();
    SetExchangeSupport();

    aCbxAutoMin.SetClickHdl(LINK(this, SchScaleYAxisTabPage, EnableValueHdl));
    aCbxAutoMax.SetClickHdl(LINK(this, SchScaleYAxisTabPage, EnableValueHdl));
    aCbxAutoStepMain.SetClickHdl(LINK(this, SchScaleYAxisTabPage, EnableValueHdl));
    aCbxAutoStepHelp.SetClickHdl(LINK(this, SchScaleYAxisTabPage, EnableValueHdl));
    aCbxAutoOrigin.SetClickHdl(LINK(this, SchScaleYAxisTabPage, EnableValueHdl));
//  aCbxLogarithm.SetClickHdl(LINK(this, SchScaleYAxisTabPage, HandleLogs));
    const SfxPoolItem *pPoolItem = NULL;
    if (rInAttrs.GetItemState(SCHATTR_AXISTYPE, TRUE, &pPoolItem) == SFX_ITEM_SET)
        nAxisType = (int) ((const SfxInt32Item*)pPoolItem)->GetValue();
}

/*************************************************************************
|*
|*  Werte-Editierfelder je nach Checkbox-Markierung enablen/disablen
|*
*************************************************************************/

IMPL_LINK( SchScaleYAxisTabPage, EnableValueHdl, CheckBox *, pCbx )
{
    if (pCbx == &aCbxAutoMin)
    {
        aFmtFldMin.Enable(!aCbxAutoMin.IsChecked());
    }
    else if (pCbx == &aCbxAutoMax)
    {
        aFmtFldMax.Enable(!aCbxAutoMax.IsChecked());
    }
    else if (pCbx == &aCbxAutoStepMain)
    {
        aFmtFldStepMain.Enable(!aCbxAutoStepMain.IsChecked());
//      bLastStepMain = aCbxAutoStepMain.IsChecked();
    }
    else if (pCbx == &aCbxAutoStepHelp)
    {
        aMtStepHelp.Show ();
        aMtStepHelp.Enable( ! aCbxAutoStepHelp.IsChecked() );

//      if (aCbxLogarithm.IsChecked ())
//      {
// //           aFmtFldStepHelp.Hide ();
//          aMtStepHelp.Show ();
//          aMtStepHelp.Enable(!aCbxAutoStepHelp.IsChecked());
//      }
//      else
//      {
// //           aFmtFldStepHelp.Show ();
//          aMtStepHelp.Hide ();
// //           aFmtFldStepHelp.Enable(!aCbxAutoStepHelp.IsChecked());
//      }
    }
    else if (pCbx == &aCbxAutoOrigin)
    {
        aFmtFldOrigin.Enable(!aCbxAutoOrigin.IsChecked());
    }
    return 0;
}

/*************************************************************************
|*
|* Erzeugung
|*
\*************************************************************************/

SfxTabPage* SchScaleYAxisTabPage::Create(Window* pWindow,const SfxItemSet& rOutAttrs)
{
    return new SchScaleYAxisTabPage(pWindow, rOutAttrs);
}

/*************************************************************************
|*
|* Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
\*************************************************************************/

BOOL SchScaleYAxisTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    DBG_ASSERT( pNumFormatter, "No NumberFormatter available" );

    long nTicks=0;
    long nHelpTicks=0;

    if(aCbxHelpTicksInner.IsChecked())
        nHelpTicks|=CHAXIS_MARK_INNER;
    if(aCbxHelpTicksOuter.IsChecked())
        nHelpTicks|=CHAXIS_MARK_OUTER;
    if(aCbxTicksInner.IsChecked())
        nTicks|=CHAXIS_MARK_INNER;
    if(aCbxTicksOuter.IsChecked())
        nTicks|=CHAXIS_MARK_OUTER;

    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_TICKS,nTicks));
    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_HELPTICKS,nHelpTicks));

    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MIN      ,aCbxAutoMin.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MAX      ,aCbxAutoMax.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP,aCbxAutoStepHelp.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN   ,aCbxAutoOrigin.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM     ,aCbxLogarithm.IsChecked()));
    rOutAttrs.Put(SvxDoubleItem(fMax     , SCHATTR_AXIS_MAX));
    rOutAttrs.Put(SvxDoubleItem(fMin     , SCHATTR_AXIS_MIN));
    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_STEP_HELP, nStepHelp));
    rOutAttrs.Put(SvxDoubleItem(fOrigin  , SCHATTR_AXIS_ORIGIN));

    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN,aCbxAutoStepMain.IsChecked()));
    rOutAttrs.Put(SvxDoubleItem(fStepMain,SCHATTR_AXIS_STEP_MAIN));

    return TRUE;
}

/*************************************************************************
|*
|* Initialisierung
|*
\*************************************************************************/

void SchScaleYAxisTabPage::Reset(const SfxItemSet& rInAttrs)
{
    DBG_ASSERT( pNumFormatter, "No NumberFormatter available" );
    if(!pNumFormatter)
        return;

    const SfxPoolItem *pPoolItem = NULL;

    long nTicks=0,nHelpTicks=0;
    if(rInAttrs.GetItemState(SCHATTR_AXIS_TICKS,TRUE, &pPoolItem)== SFX_ITEM_SET)
        nTicks=((const SfxInt32Item*)pPoolItem)->GetValue();
    if(rInAttrs.GetItemState(SCHATTR_AXIS_HELPTICKS,TRUE, &pPoolItem)== SFX_ITEM_SET)
        nHelpTicks=((const SfxInt32Item*)pPoolItem)->GetValue();

    aCbxHelpTicksInner.Check(nHelpTicks&CHAXIS_MARK_INNER);
    aCbxHelpTicksOuter.Check(nHelpTicks&CHAXIS_MARK_OUTER);
    aCbxTicksInner.Check(nTicks&CHAXIS_MARK_INNER);
    aCbxTicksOuter.Check(nTicks&CHAXIS_MARK_OUTER);

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_MIN,TRUE,&pPoolItem) == SFX_ITEM_SET)
        aCbxAutoMin.Check(((const SfxBoolItem*)pPoolItem)->GetValue());

    if (rInAttrs.GetItemState(SCHATTR_AXIS_MIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        fMin = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        aFmtFldMin.SetValue( fMin );
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_MAX,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoMax.Check(((const SfxBoolItem*)pPoolItem)->GetValue());

    if (rInAttrs.GetItemState(SCHATTR_AXIS_MAX,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        fMax = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        aFmtFldMax.SetValue( fMax );
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_STEP_MAIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoStepMain.Check(((const SfxBoolItem*)pPoolItem)->GetValue());

    if (rInAttrs.GetItemState(SCHATTR_AXIS_STEP_MAIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        fStepMain = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        aFmtFldStepMain.SetValue( fStepMain );
    }
    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_STEP_HELP,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoStepHelp.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_LOGARITHM,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxLogarithm.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_STEP_HELP,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nStepHelp = ((const SfxInt32Item*)pPoolItem)->GetValue();
//      if (aCbxLogarithm.IsChecked ())

        aMtStepHelp.SetValue( nStepHelp );

//      else
//      {
// //           aFmtFldStepHelp.SetValue( fStepHelp );
//      }
    }
    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_ORIGIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoOrigin.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_ORIGIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        fOrigin = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        aFmtFldOrigin.SetValue( fOrigin );
    }

    EnableValueHdl(&aCbxAutoMin);
    EnableValueHdl(&aCbxAutoMax);
    EnableValueHdl(&aCbxAutoStepMain);
    EnableValueHdl(&aCbxAutoStepHelp);
    EnableValueHdl(&aCbxAutoOrigin);
//  CheckLogControls ();
}

/*************************************************************************
|*
|* Prueft die eingegebenen Werte
|*
\*************************************************************************/

int SchScaleYAxisTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if( !pNumFormatter )
    {
        DBG_ERROR( "No NumberFormatter available" );
        return LEAVE_PAGE;
    }

    ULONG nIndex = pNumFormatter->GetStandardIndex(LANGUAGE_SYSTEM);
    Edit* pEdit = NULL;
    USHORT nErrStrId = 0;
    double fDummy;

//  if( CheckLogs ())
//         return KEEP_PAGE;


    fMax = aFmtFldMax.GetValue();
    fMin = aFmtFldMin.GetValue();
    fOrigin = aFmtFldOrigin.GetValue();
    fStepMain = aFmtFldStepMain.GetValue();
//  fStepHelp = aFmtFldStepHelp.GetValue();
    nStepHelp = aMtStepHelp.GetValue();

    //do some reasonable automatic correction of user input if necessary
    if (!aCbxAutoMax.IsChecked() && !aCbxAutoMin.IsChecked() &&
             fMin >= fMax)
    {
        //do not bother the user with error boxes
        //just correct hte error and exchance minimum and maximum
        double fSwap = fMin;
        fMin = fMax;
        fMax = fSwap;
        /*
        pEdit = &aFmtFldMin;
        nErrStrId = STR_MINVALUE_LT_MAXVALUE;
        */
    }
    // check for entries in invalid ranges
    if ( aCbxLogarithm.IsChecked() &&
            ( ( !aCbxAutoMin.IsChecked() && fMin <= 0.0 )
             || ( !aCbxAutoMax.IsChecked() && fMax <= 0.0 ) ) )
    {
        //do not bother the user with error boxes for minimum greater 0
        //just set some suiteable default value
        if( !aCbxAutoMax.IsChecked() && fMax<=0.0 )
            fMax = 100.0;
        if( !aCbxAutoMin.IsChecked() && fMin<=0.0 )
        {
            fMin = 0.1;
            if(!aCbxAutoMax.IsChecked() && fMin>=fMax)
                fMin = log(fMax)/10.0;
        }
        /*
        pEdit = &aFmtFldMin;
        nErrStrId = STR_BAD_LOGARITHM;
        */
    }
    if (!aCbxAutoStepHelp.IsChecked() && nStepHelp <= 0)
    {
        //do not bother the user with error boxes for minimum greater 0
        nStepHelp = 1;
        /*
        pEdit = &aFmtFldStepMain;
        nErrStrId = STR_STEP_GT_ZERO;
        */
    }
    //user often forgets to switch visibility of help tickmarks on
    if( !aCbxAutoStepHelp.IsChecked() && aMtStepHelp.IsModified() && nStepHelp > 1
        && !aCbxHelpTicksInner.IsChecked() && !aCbxHelpTicksOuter.IsChecked() )
        //&& !aCbxHelpTicksInner.IsModified() && !aCbxHelpTicksOuter.IsModified() )
    {
        //check help ticks like main ticks
        if(aCbxTicksInner.IsChecked())
            aCbxHelpTicksInner.Check();
        if(aCbxTicksOuter.IsChecked())
            aCbxHelpTicksOuter.Check();
    }

    //check wich entries need user action

    // check for entries that cannot be parsed for the current number format
    if ( aFmtFldMin.IsModified()
        && !aCbxAutoMin.IsChecked()
        && !pNumFormatter->IsNumberFormat(aFmtFldMin.GetText(), nIndex, fDummy))
    {
        pEdit = &aFmtFldMin;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if (aFmtFldMax.IsModified() && !aCbxAutoMax.IsChecked() &&
             !pNumFormatter->IsNumberFormat(aFmtFldMax.GetText(),
                                            nIndex, fDummy))
    {
        pEdit = &aFmtFldMax;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if (aFmtFldStepMain.IsModified() && !aCbxAutoStepMain.IsChecked() &&
             !pNumFormatter->IsNumberFormat(aFmtFldStepMain.GetText(),
                                            nIndex, fDummy))
    {
        pEdit = &aFmtFldStepMain;
        nErrStrId = STR_INVALID_NUMBER;
    }
    /*
    else if (!GetHelpValue ())
    {
//      pEdit = &aFmtFldStepHelp;
        nErrStrId = STR_INVALID_NUMBER;
    }
    */
    else if (aFmtFldOrigin.IsModified() && !aCbxAutoOrigin.IsChecked() &&
             !pNumFormatter->IsNumberFormat(aFmtFldOrigin.GetText(),
                                            nIndex, fDummy))
    {
        pEdit = &aFmtFldOrigin;
        nErrStrId = STR_INVALID_NUMBER;
    }
    /*
    //do not bother the user with error boxes for origin
    //any values are tolerable
    else if (!aCbxAutoOrigin.IsChecked() && aCbxLogarithm.IsChecked() &&
             fOrigin <= 0.0)
    {
        pEdit = &aFmtFldOrigin;
        nErrStrId = STR_BAD_LOGARITHM;
    }
    */
    else if (!aCbxAutoStepMain.IsChecked() && fStepMain <= 0.0)
    {
        //show error message only if no reasonable value can be found here
        if( !aCbxAutoMin.IsChecked() && !aCbxAutoMax.IsChecked() )
        {
            if(aCbxLogarithm.IsChecked())
                fStepMain = 1.0;
            else
                fStepMain = (fMax - fMin)/2.0;
        }
        else
        {
            pEdit = &aFmtFldStepMain;
            nErrStrId = STR_STEP_GT_ZERO;
        }
    }
    /*
    //do not bother the user with error boxes Main Step
    //the main step is allowed to be greater than the visible range
    else if (!aCbxAutoStepMain.IsChecked() && fStepMain > fabs( fMax - fMin ))
    {
        pEdit = &aFmtFldStepMain;
        nErrStrId = STR_STEPMAIN_LT_MAXVALUE;
    }
    */
    // step may now be anything, independent from main step
//  else if (!aCbxLogarithm.IsChecked () && !aCbxAutoStepHelp.IsChecked() && fStepHelp > fStepMain)
//  {
// //       pEdit = &aFmtFldStepHelp;
//      nErrStrId = STR_STEPHELP_LT_STEPMAIN;
//  }
    /*
    else
    {
        //do not bother the user with error boxes for origin
        //any values are tolerable
        if(( ! aCbxAutoMin.IsChecked() &&
             fOrigin < fMin ) ||
           ( ! aCbxAutoMax.IsChecked() &&
             fMax < fOrigin ))
        {
            nErrStrId = STR_WRONG_ORIGIN;

            if( ! aCbxAutoOrigin.IsChecked ())
                pEdit = &aFmtFldOrigin;
            else if( ! aCbxAutoMin.IsChecked())
                pEdit = &aFmtFldMin;
            else
                pEdit = &aFmtFldMax;
        }
    }
    */

    if( ShowWarning( nErrStrId, pEdit ) )
        return KEEP_PAGE;

    if( pItemSet )
        FillItemSet( *pItemSet );

    return LEAVE_PAGE;
}

/*************************************************************************
|*
|*  pruefe die eingaben fuer logarithmische werte
|*
*************************************************************************/

// IMPL_LINK( SchScaleYAxisTabPage, HandleLogs, CheckBox *, pCbx )
// {
//  CheckLogs ();

//  return 0;
// }

/*************************************************************************
|*
|*  pruefe die eingaben fuer logarithmische werte
|*
*************************************************************************/

// bool SchScaleYAxisTabPage::CheckLogs ()
// {
//     if( !pNumFormatter )
//     {
//         DBG_ERROR( "No NumberFormatter available" );
//         return false;
//     }

//  ULONG  nIndex    = pNumFormatter->GetStandardIndex(LANGUAGE_SYSTEM);
//  Edit   *pEdit    = NULL;
//  USHORT nErrStrId = 0;
//  String aNumStr;
//  double fDummy;

// //   CheckLogControls ();

//  if (!aCbxAutoMin.IsChecked ())
//  {
//      fMin = aFmtFldMin.GetValue();
//      if (!pNumFormatter->IsNumberFormat(aFmtFldMin.GetText(), nIndex, fDummy))
//      {
//          pEdit = &aFmtFldMin;
//          nErrStrId = STR_INVALID_NUMBER;
//      }
//      else if (aCbxLogarithm.IsChecked ())
//           {
//               if (fMin > 1.0)
//               {
//                   double fVal = 1.0;

//                   while (fVal < fMin)
//                       fVal *= 10.0;

//                   if (fVal > fMin) fVal /= 10.0;
//                   fMin = fVal;
//               }
//               else if (fMin > 0.0)
//                    {
//                        double fVal = 1.0;

//                        while (fVal > fMin)
//                        {
//                            fVal /= 10.0;
//                            if(::rtl::math::approxEqual(fVal,fMin))
//                                fMin=fVal; //Double-Ungenauigkeiten (last-bits) ausgleichen
//                        }

//                        fMin = fVal;
//                    }
//                    else fMin = 1.0;

//               aFmtFldMin.SetValue( fMin );
//           }
//  }

//     if (!aCbxAutoMin.IsChecked ())
//  {
//      fMax = aFmtFldMax.GetValue();
//      if( !pNumFormatter->IsNumberFormat(aFmtFldMax.GetText(), nIndex, fDummy))
//      {
//          pEdit = &aFmtFldMax;
//          nErrStrId = STR_INVALID_NUMBER;
//      }
//      else if (aCbxLogarithm.IsChecked ())
//           {
//               if (fMax > 1.0)
//               {
//                   double fVal = 1.0;

//                   while (fVal < fMax)
//                       fVal *= 10.0;

//                   fMax = fVal;
//               }
//               else if (fMax > 0.0)
//                    {
//                        double fVal = 1.0;

//                        while (fVal > fMax)
//                            fVal /= 10.0;

//                        if (fVal < fMax) fVal *= 10.0;
//                        fMax = fVal;
//                    }
//                    else fMax = 1.0;

//               aFmtFldMax.SetValue( fMax );
//           }
//  }

//  BOOL bNewOrigin = FALSE;

//  if (aCbxAutoOrigin.IsChecked())
//  {
//      if (fMax < 0.0)
//      {
//          fOrigin = fMax;
//          bNewOrigin = TRUE;
//      }
//      if (fMin > 0.0)
//      {
//          fOrigin = fMin;
//          bNewOrigin = TRUE;
//      }
//      if ((fMax >= 0.0) && (fMin <= 0.0))
//      {
//          fOrigin = 0.0;
//          bNewOrigin = TRUE;
//      }
//  }

//  if (aCbxLogarithm.IsChecked ())
//  {
//      fOrigin = fMin;
//      bNewOrigin = TRUE;
//  }

//  if( bNewOrigin )
//  {
//         aFmtFldOrigin.SetValue( fOrigin );
//  }

//     return ShowWarning( nErrStrId, pEdit );
// }

/*************************************************************************
|*
|*  pruefe die eingaben fuer logarithmische werte
|*
*************************************************************************/

// bool SchScaleYAxisTabPage::CheckLogControls ()
// {
//     USHORT nErrorId = 0;
//     Edit * pControl = NULL;

//     if( !pNumFormatter )
//     {
//         DBG_ERROR( "No NumberFormatter available" );
//         return false;
//     }

//  if (aCbxLogarithm.IsChecked ())
//  {
//      ULONG nIndex = pNumFormatter->GetStandardIndex(LANGUAGE_SYSTEM);

// //       aFmtFldOrigin.Enable (FALSE);
// //       aCbxAutoOrigin.Enable (FALSE);
// //       aFmtFldStepMain.Enable (FALSE);
// //       bLastStepMain = aCbxAutoStepMain.IsChecked();
// //       aCbxAutoStepMain.Check();
// //       aCbxAutoStepMain.Enable (FALSE);
// //       aFmtFldStepHelp.Enable (FALSE);
// //       aFmtFldStepHelp.Hide ();
// //       aMtStepHelp.Show ();

//      double fNewMin;
//      if (pNumFormatter->IsNumberFormat(aFmtFldMin.GetText(), nIndex, fNewMin ))
//         {
//             if (fNewMin <= 0.0)
//             {
//                 nErrorId = STR_BAD_LOGARITHM;
//                 pControl = & aFmtFldMin;
//             }
//             else
//                 fMin = fNewMin;
//         }
//         else
//         {
//             nErrorId = STR_BAD_FORMAT;
//             pControl = & aFmtFldMin;
//         }

// //         aMtStepHelp.Enable (!aCbxAutoStepHelp.IsChecked ());
//  }
// //   else
// //   {
// //       aCbxAutoStepMain.Check( bLastStepMain );

// //       aCbxAutoOrigin.Enable (TRUE);
// //         aCbxAutoStepMain.Enable (TRUE);

// //       aFmtFldStepHelp.Enable (!aCbxAutoStepHelp.IsChecked ());

// //       aFmtFldStepHelp.Show ();
// //       aMtStepHelp.Hide ();
// //   }

//     aFmtFldOrigin.Enable (!aCbxAutoOrigin.IsChecked ());
//     aFmtFldStepMain.Enable (!aCbxAutoStepMain.IsChecked ());
//     aMtStepHelp.Enable (!aCbxAutoStepHelp.IsChecked ());

//     return ShowWarning( nErrorId, pControl );
// }

/*************************************************************************
|*
|*  pruefe die eingaben fuer logarithmische werte
|*
*************************************************************************/

/*
BOOL SchScaleYAxisTabPage::GetHelpValue ()
{
    if (!aCbxAutoStepHelp.IsChecked())
        nStepHelp = aMtStepHelp.GetValue ();
    return TRUE;

//     if (aCbxLogarithm.IsChecked ())
//  {
//      if (!aCbxAutoStepHelp.IsChecked()) fStepHelp = aMtStepHelp.GetValue ();

//      return TRUE;
//  }
//  else
//  {
//      ULONG nIndex = pNumFormatter->GetStandardIndex(LANGUAGE_SYSTEM);
//      double fDummy;

//      fStepHelp = aFmtFldStepHelp.GetValue();
//      return !(aFmtFldStepHelp.IsModified() && !aCbxAutoStepHelp.IsChecked() &&
//               !pNumFormatter->IsNumberFormat(aFmtFldStepHelp.GetText(),
//                                              nIndex, fDummy));
//  }
}
*/

void SchScaleYAxisTabPage::SetNumFormatter( SvNumberFormatter* pFormatter )
{
    pNumFormatter = pFormatter;
    aFmtFldMax.SetFormatter( pNumFormatter );
    aFmtFldMin.SetFormatter( pNumFormatter );
    aFmtFldStepMain.SetFormatter( pNumFormatter );
//  aFmtFldStepHelp.SetFormatter( pNumFormatter );
    aFmtFldOrigin.SetFormatter( pNumFormatter );
    SetNumFormat();
}

void SchScaleYAxisTabPage::SetNumFormat()
{
    const SfxPoolItem *pPoolItem = NULL;

    if( rOutAttrs.GetItemState( SCHATTR_AXIS_NUMFMT, TRUE, &pPoolItem ) == SFX_ITEM_SET )
    {
        ULONG nFmt = (ULONG)((const SfxInt32Item*)pPoolItem)->GetValue();
        short eType = pNumFormatter->GetType( nFmt );

        // change nFmt to an editable format (without loss of information)
        if( eType == NUMBERFORMAT_CURRENCY )    // for currencies just display decimals
        {
            nFmt = pNumFormatter->GetStandardIndex();
        }
        else
        {
            const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
            if( pFormat )
            {
                LanguageType eLanguage = pFormat->GetLanguage();
                nFmt = pNumFormatter->GetStandardFormat( nFmt, eType, eLanguage );
            }
            // else: format is 'standard'
        }

        aFmtFldMax.SetFormatKey( nFmt );
        aFmtFldMin.SetFormatKey( nFmt );
        aFmtFldOrigin.SetFormatKey( nFmt );

        // for steps use standard format if date or time format is chosen
        if( pNumFormatter &&
            ( eType == NUMBERFORMAT_DATE ||
              eType == NUMBERFORMAT_TIME ||
              eType == NUMBERFORMAT_DATETIME ) )
        {
            nFmt = pNumFormatter->GetStandardIndex();
        }

        aFmtFldStepMain.SetFormatKey( nFmt );
//      aFmtFldStepHelp.SetFormatKey( nFmt );
    }
}

bool SchScaleYAxisTabPage::ShowWarning( USHORT nResIdMessage, Edit * pControl /* = NULL */ )
{
    if( nResIdMessage == 0 )
        return false;

    WarningBox( this, WinBits( WB_OK ), String( SchResId( nResIdMessage ))).Execute();
    if( pControl )
    {
        pControl->GrabFocus();
        pControl->SetSelection( Selection( 0, SELECTION_MAX ));
    }
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................

