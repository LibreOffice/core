/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_TitleRotation.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:49:52 $
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
#ifndef _CHART2_TP_TITLEROTATION_HXX
#define _CHART2_TP_TITLEROTATION_HXX

// header for SfxTabPage
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

#ifndef SVX_DIALCONTROL_HXX
#include <svx/dialcontrol.hxx>
#endif
#ifndef SVX_WRAPFIELD_HXX
#include <svx/wrapfield.hxx>
#endif
#ifndef SVX_ORIENTHELPER_HXX
#include <svx/orienthelper.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class SchAlignmentTabPage : public SfxTabPage
{
private:
    FixedLine               aFlAlign;
    svx::DialControl        aCtrlDial;
    FixedText               aFtRotate;
    svx::WrapField          aNfRotate;
    TriStateBox             aCbStacked;
    svx::OrientationHelper  aOrientHlp;

public:
    SchAlignmentTabPage(Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SchAlignmentTabPage();

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rInAttrs);
    virtual BOOL FillItemSet(SfxItemSet& rOutAttrs);
    virtual void Reset(const SfxItemSet& rInAttrs);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
