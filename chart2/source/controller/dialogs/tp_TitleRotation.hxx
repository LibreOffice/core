/*************************************************************************
 *
 *  $RCSfile: tp_TitleRotation.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-08 23:04:12 $
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
#ifndef _CHART2_TP_TITLEROTATION_HXX
#define _CHART2_TP_TITLEROTATION_HXX
/*
#include "SchSfxItemIds.hxx"
// header for enum SvxChartKindError
#include "SchAllDefinesFor_svx_chrtitem.hxx"
#ifndef _SVX_CHRTITEM_HXX //autogen
#include <svx/chrtitem.hxx>
#endif
*/
// header for SfxTabPage
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
// header for SvxWinOrientation
#ifndef WORIENT_HXX //autogen
#include <svx/worient.hxx>
#endif
/*
// header for class ValueSet
#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif
*/
// header for FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
/*
// header for MetricField
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
*/
//.............................................................................
namespace chart
{
//.............................................................................

class SchAlignmentTabPage : public SfxTabPage
{
private:


    //Seit 4/1998 koennen Texte frei gedreht werden: SID_ATTR_ALIGN_DEGREES
    SvxWinOrientation   aWinOrient;

//  CheckBox    aCbxTextOverlap;

//  CheckBox    aCbxTextBreak;
//  FixedLine   aFlTextBreak;

//  RadioButton aRbtSideBySide;
//  RadioButton aRbtUpDown;
//  RadioButton aRbtDownUp;
//  RadioButton aRbtAutoOrder;
//  FixedLine   aFlOrder;
//  OrderMode   eOrderMode;

    const SfxItemSet&   rOutAttrs;

//  DECL_LINK(CheckButtonHdl, Button*);
//  BOOL bTextCanOverlap;
public:
    SchAlignmentTabPage(Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SchAlignmentTabPage();

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rInAttrs);
    virtual BOOL FillItemSet(SfxItemSet& rOutAttrs);
    virtual void Reset(const SfxItemSet& rInAttrs);

//  void SetOrderMode(OrderMode eMode);
//  void SetTextCanOverlap(BOOL b){bTextCanOverlap=b;}
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
