/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_DataLabel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-25 08:34:15 $
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
    CheckBox            m_aCbValue;
    RadioButton         m_aRbNumber;
    RadioButton         m_aRbPercent;
//DLNF    PushButton          m_aPB_NumberFormatForValue;
//DLNF    PushButton          m_aPB_NumberFormatForPercent;
    CheckBox            m_aCbText;
    CheckBox            m_aCbSymbol;

    SvNumberFormatter*  m_pNumberFormatter;
    ULONG               m_nNumberFormatForValue;
    ULONG               m_nNumberFormatForPercent;
    bool                m_bSourceFormatForValue;
    bool                m_bSourceFormatForPercent;

    Window*             m_pWindow;
    SfxItemPool*        m_pPool;

//DLNF    DECL_LINK(NumberFormatDialogHdl, PushButton * );
    DECL_LINK(CheckHdl, CheckBox* );
    void EnableControls();
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
