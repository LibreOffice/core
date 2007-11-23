/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_DataLabel.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 11:47:59 $
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
#ifndef CHART2_RES_DATALABEL_HXX
#define CHART2_RES_DATALABEL_HXX

#include "res_TextSeparator.hxx"
// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class SfxItemSet
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

class SvNumberFormatter;

//.............................................................................
namespace chart
{
//.............................................................................

class DataLabelResources
{
public:
    DataLabelResources( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~DataLabelResources();

    BOOL FillItemSet(SfxItemSet& rOutAttrs) const;
    void Reset(const SfxItemSet& rInAttrs);

    void SetNumberFormatter( SvNumberFormatter* pFormatter );

private:
    CheckBox            m_aCBNumber;
    PushButton          m_aPB_NumberFormatForValue;
    CheckBox            m_aCBPercent;
    PushButton          m_aPB_NumberFormatForPercent;
    CheckBox            m_aCBCategory;
    CheckBox            m_aCBSymbol;

    TextSeparatorResources  m_aSeparatorResources;

    FixedText           m_aFT_LabelPlacement;
    ListBox             m_aLB_LabelPlacement;

    ::std::map< sal_Int32, USHORT > m_aPlacementToListBoxMap;
    ::std::map< USHORT, sal_Int32 > m_aListBoxToPlacementMap;

    SvNumberFormatter*  m_pNumberFormatter;
    bool                m_bNumberFormatMixedState;
    bool                m_bPercentFormatMixedState;
    ULONG               m_nNumberFormatForValue;
    ULONG               m_nNumberFormatForPercent;

    bool                m_bSourceFormatMixedState;
    bool                m_bPercentSourceMixedState;
    bool                m_bSourceFormatForValue;
    bool                m_bSourceFormatForPercent;

    Window*             m_pWindow;
    SfxItemPool*        m_pPool;

    DECL_LINK(NumberFormatDialogHdl, PushButton * );
    DECL_LINK(CheckHdl, CheckBox* );
    void EnableControls();
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
