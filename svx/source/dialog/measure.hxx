/*************************************************************************
 *
 *  $RCSfile: measure.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 18:36:36 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_MEASURE_HXX
#define _SVX_MEASURE_HXX

// include ---------------------------------------------------------------

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SVX_DLGCTRL_HXX
#include "dlgctrl.hxx"
#endif
#ifndef _SVX_MEASCTRL_HXX
#include "measctrl.hxx"
#endif

class SdrView;

/*************************************************************************
|*
|* Dialog zum "Andern von TextAttributen
|*
\************************************************************************/

class SvxMeasurePage : public SvxTabPage
{
private:

    FixedLine           aFlLine;
    FixedText           aFtLineDist;
    MetricField         aMtrFldLineDist;
    FixedText           aFtHelplineOverhang;
    MetricField         aMtrFldHelplineOverhang;
    FixedText           aFtHelplineDist;
    MetricField         aMtrFldHelplineDist;
    FixedText           aFtHelpline1Len;
    MetricField         aMtrFldHelpline1Len;
    FixedText           aFtHelpline2Len;
    MetricField         aMtrFldHelpline2Len;
    TriStateBox         aTsbBelowRefEdge;
    FixedText           aFtDecimalPlaces;
    MetricField         aMtrFldDecimalPlaces;

    FixedLine           aFlLabel;
    FixedText           aFtPosition;
    SvxRectCtl          aCtlPosition;
    TriStateBox         aTsbAutoPosV;
    TriStateBox         aTsbAutoPosH;
    TriStateBox         aTsbShowUnit;
    ListBox             aLbUnit;
    TriStateBox         aTsbParallel;

    SvxXMeasurePreview  aCtlPreview;

    FixedLine           aFlVert;

    const SfxItemSet&   rOutAttrs;
    SfxItemSet          aAttrSet;
    const SdrView*      pView;
    SfxMapUnit          eUnit;

    BOOL                bPositionModified;

#ifdef _SVX_MEASURE_CXX
    void                FillUnitLB();

    DECL_LINK( ClickAutoPosHdl_Impl, void * );
    DECL_LINK( ChangeAttrHdl_Impl, void * );
#endif

public:

    SvxMeasurePage( Window* pWindow, const SfxItemSet& rInAttrs );
    ~SvxMeasurePage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static  USHORT*     GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );

    virtual void        PointChanged( Window* pWindow, RECT_POINT eRP );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated (SfxAllItemSet aSet); //add CHINA001
};

/*************************************************************************
|*
|* Von SfxSingleTabDialog abgeleitet, um vom Control "uber virtuelle Methode
|* benachrichtigt werden zu k"onnen.
|*
\************************************************************************/

class SvxMeasureDialog : public SfxSingleTabDialog
{
public:
    SvxMeasureDialog( Window* pParent, const SfxItemSet& rAttr,
                       const SdrView* pView );
    ~SvxMeasureDialog();
};


#endif // _SVX_MEASURE_HXX

