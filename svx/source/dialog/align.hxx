/*************************************************************************
 *
 *  $RCSfile: align.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:49:15 $
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
#ifndef _SVX_ALIGN_HXX
#define _SVX_ALIGN_HXX

// include ---------------------------------------------------------------


#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#include "worient.hxx" //@ 12.09.97

#ifndef _SVX_FRMDIRLBOX_HXX
#include "frmdirlbox.hxx"
#endif
#include "flagsdef.hxx"
// defines ---------------------------------------------------------------

//CHINA001 #define WBA_NO_ORIENTATION   ((USHORT)0x0001)
//CHINA001 #define WBA_NO_LINEBREAK ((USHORT)0x0002)
//CHINA001 #define WBA_NO_HORIZONTAL    ((USHORT)0x0004)
//CHINA001 #define WBA_NO_LEFTINDENT    ((USHORT)0x0008)
//CHINA001 #define WBA_NO_VERTICAL      ((USHORT)0x0010)
//CHINA001 #define WBA_NO_GRIDLINES ((USHORT)0x0020)
//CHINA001 #define WBA_NO_HYPHENATION   ((USHORT)0x0040)

// class SvxAlignmentTabPage ---------------------------------------------

/*
{k:\svx\prototyp\dialog\align.bmp}
    [Beschreibung]
    Mit dieser TabPage k"onnen Textausrichtungsattribute eingestellt werden

    [Items]
    <SvxHorJustifyItem>:        <SID_ATTR_ALIGN_HOR_JUSTIFY>
    <SfxUInt16Item>             <SID_ATTR_ALIGN_INDENT>
    <SvxVerJustifyItem>:        <SID_ATTR_ALIGN_VER_JUSTIFY>
    <SvxOrientationItem>:       <SID_ATTR_ALIGN_ORIENTATION>
    <SvxMarginItem>:            <SID_ATTR_ALIGN_MARGIN>
    <SfxBoolItem>:              <SID_ATTR_ALIGN_LINEBREAK>
    <SfxBoolItem>:              <SID_ATTR_ALIGN_HYPHENATION>
*/

class SvxAlignmentTabPage : public SfxTabPage
{
private:
                        // Controls
    FixedLine           aFlAlignment;
    FixedText           aFtHorAlign;
    ListBox             aLbHorAlign;
    FixedText           aFtIndent;
    MetricField         aEdIndent;
    FixedText           aFtVerAlign;
    ListBox             aLbVerAlign;

    SvxWinOrientation   aWinOrient; //@ 12.09.97
    TriStateBox         aBtnAsianVert;

    FixedLine           aFlSpace;
    FixedText           aFtLeftSpace;
    MetricField         aEdLeftSpace;
    FixedText           aFtRightSpace;
    MetricField         aEdRightSpace;
    FixedText           aFtTopSpace;
    MetricField         aEdTopSpace;
    FixedText           aFtBottomSpace;
    MetricField         aEdBottomSpace;

    FixedLine           aFlWrap;
    TriStateBox         aBtnWrap;
    TriStateBox         aBtnHyphen;
    FixedText           aFtTextFlow;
    svx::FrameDirectionListBox aLbFrameDir;

    BOOL                bHyphenDisabled;

    SvxAlignmentTabPage( Window* pParent, const SfxItemSet& rCoreSet );

    void FillForLockMode();

    /** @return  Pointer to an item, if it is DEFAULT or SET; NULL, if it is DONTCARE. */
    const SfxPoolItem*  GetUniqueItem( const SfxItemSet& rCoreSet, sal_uInt16 nSlotId ) const;

#ifdef _SVX_ALIGN_CXX
    // Handler
    DECL_LINK( HorAlignSelectHdl_Impl, ListBox * );
    DECL_LINK( WrapClickHdl_Impl, TriStateBox * );
    DECL_LINK( TxtStackedClickHdl_Impl, void * );
    void EnableHyphen_Impl();
#endif

    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
public:
    ~SvxAlignmentTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static USHORT*      GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet );

    void                SetFlags( USHORT nFlags );
    virtual void        PageCreated (SfxAllItemSet aSet); //add CHINA001
};


#endif

