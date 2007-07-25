/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_DataLabel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-25 08:34:04 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "res_DataLabel.hxx"
#include "res_DataLabel_IDs.hrc"

#include "Strings.hrc"
#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"
//DLNF #include "dlg_NumberFormat.hxx"

// header for class SvxNumberInfoItem
#ifndef _SVX_NUMINF_HXX
#include <svx/numinf.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif
// for SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
// header for SvNumberFormatter
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

namespace
{

void lcl_ReadNumberFormatFromItemSet( const SfxItemSet& rSet, USHORT nValueWhich, USHORT nSourceFormatWhich, ULONG& rnFormatKeyOut, bool& rbSourceFormatOut )
{
    const SfxPoolItem *pItem1 = NULL;
    if( rSet.GetItemState( nValueWhich, TRUE, &pItem1 ) == SFX_ITEM_SET )
    {
        const SfxUInt32Item * pNumItem = dynamic_cast< const SfxUInt32Item * >( pItem1 );
        if( pNumItem )
            rnFormatKeyOut = pNumItem->GetValue();
    }

    const SfxPoolItem *pItem2 = NULL;
    if( rSet.GetItemState( nSourceFormatWhich, TRUE, &pItem2 ) == SFX_ITEM_SET )
    {
        const SfxBoolItem * pBoolItem = dynamic_cast< const SfxBoolItem * >( pItem2 );
        if( pBoolItem )
            rbSourceFormatOut = pBoolItem->GetValue();
    }
}

}//end anonymous namespace

DataLabelResources::DataLabelResources( Window* pWindow, const SfxItemSet& rInAttrs )
    : m_aCbValue(pWindow, SchResId(CB_VALUE)),
    m_aRbNumber(pWindow, SchResId(RB_NUMBER)),
    m_aRbPercent(pWindow, SchResId(RB_PERCENT)),
//DLNF    m_aPB_NumberFormatForValue(pWindow, SchResId(PB_NUMBERFORMAT)),
//DLNF    m_aPB_NumberFormatForPercent(pWindow, SchResId(PB_PERCENT_NUMBERFORMAT)),
    m_aCbText(pWindow, SchResId(CB_TEXT)),
    m_aCbSymbol(pWindow, SchResId(CB_SYMBOL)),
    m_pNumberFormatter(0),
    m_nNumberFormatForValue(0),
    m_nNumberFormatForPercent(11),
    m_bSourceFormatForValue(true),
    m_bSourceFormatForPercent(true),
    m_pWindow(pWindow),
    m_pPool(rInAttrs.GetPool())
{
//DLNF    Size aPBSize( m_aPB_NumberFormatForPercent.GetSizePixel() );
//DLNF    long nMinWidth = ::std::max( m_aPB_NumberFormatForPercent.CalcMinimumSize().getWidth(), m_aPB_NumberFormatForValue.CalcMinimumSize().getWidth() );
//DLNF    aPBSize.setWidth( nMinWidth+20 );//the min with is to small to fit, hm... so add alittle

//DLNF    m_aPB_NumberFormatForValue.SetSizePixel( aPBSize );
//DLNF    m_aPB_NumberFormatForPercent.SetSizePixel( aPBSize );

//DLNF    m_aPB_NumberFormatForValue.SetClickHdl( LINK( this, DataLabelResources, NumberFormatDialogHdl ) );
//DLNF    m_aPB_NumberFormatForPercent.SetClickHdl( LINK( this, DataLabelResources, NumberFormatDialogHdl ) );
    m_aCbValue.SetClickHdl( LINK( this, DataLabelResources, CheckHdl ));
    m_aCbText.SetClickHdl(  LINK( this, DataLabelResources, CheckHdl ));
    m_aCbSymbol.SetClickHdl(  LINK( this, DataLabelResources, CheckHdl ));

//DLNF    lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue );
//DLNF    lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent );
}

DataLabelResources::~DataLabelResources()
{
}

void DataLabelResources::SetNumberFormatter( SvNumberFormatter* pFormatter )
{
    m_pNumberFormatter = pFormatter;
}
//DLNF
/*
IMPL_LINK( DataLabelResources, NumberFormatDialogHdl, PushButton *, pButton )
{
    if( !m_pPool || !m_pNumberFormatter )
    {
        DBG_ERROR("Missing item pool or number formatter");
        return 1;
    }

    if( pButton == &m_aPB_NumberFormatForValue && !m_aRbNumber.IsChecked())
        m_aRbNumber.Check();
    else if( pButton == &m_aPB_NumberFormatForPercent && !m_aRbPercent.IsChecked())
        m_aRbPercent.Check();

    SfxItemSet aNumberSet = NumberFormatDialog::CreateEmptyItemSetForNumberFormatDialog( *m_pPool );
    aNumberSet.Put (SvxNumberInfoItem( m_pNumberFormatter, (const USHORT)SID_ATTR_NUMBERFORMAT_INFO));

    bool bPercent = ( pButton == &m_aPB_NumberFormatForPercent );

    ULONG& rnFormatKey = bPercent ? m_nNumberFormatForPercent : m_nNumberFormatForValue;
    bool& rUseSourceFormat = bPercent ? m_bSourceFormatForPercent : m_bSourceFormatForValue;

    aNumberSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, rnFormatKey ));
    aNumberSet.Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, rUseSourceFormat ));

    NumberFormatDialog aDlg(m_pWindow, aNumberSet);
    if( bPercent )
        aDlg.SetText( String( SchResId( STR_DLG_NUMBERFORMAT_FOR_PERCENTAGE_VALUE ) ) );
    if( RET_OK == aDlg.Execute() )
    {
        const SfxItemSet* pResult = aDlg.GetOutputItemSet();
        if( pResult )
            lcl_ReadNumberFormatFromItemSet( *pResult, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, rnFormatKey, rUseSourceFormat );
    }
    return 0;
}
*/
IMPL_LINK( DataLabelResources, CheckHdl, CheckBox*, pBox )
{
    if( pBox == &m_aCbValue || pBox == &m_aCbText )
    {
        m_aCbValue.EnableTriState( FALSE );
        m_aCbText.EnableTriState( FALSE );
        m_aCbSymbol.EnableTriState( FALSE );
    }
    else if( pBox == &m_aCbSymbol )
        m_aCbSymbol.EnableTriState( FALSE );

    EnableControls();

    return 0;
}

void DataLabelResources::EnableControls()
{
    m_aCbSymbol.Enable( m_aCbValue.IsChecked() || m_aCbText.IsChecked() );

    m_aRbPercent.Enable( m_aCbValue.IsChecked() );
    m_aRbNumber.Enable( m_aCbValue.IsChecked() );

//DLNF    m_aPB_NumberFormatForValue.Enable( m_pNumberFormatter && m_aCbValue.IsChecked() );
//DLNF    m_aPB_NumberFormatForPercent.Enable( m_pNumberFormatter && m_aCbValue.IsChecked() );
}

BOOL DataLabelResources::FillItemSet( SfxItemSet& rOutAttrs ) const
{
    BOOL bText = m_aCbText.IsChecked();
    SvxChartDataDescr eDescr;

    if( m_aCbValue.IsChecked() )
    {
        if( m_aRbNumber.IsChecked() )
        {
            eDescr = (bText? CHDESCR_TEXTANDVALUE : CHDESCR_VALUE);
//DLNF            rOutAttrs.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, m_nNumberFormatForValue ));
//DLNF            rOutAttrs.Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, m_bSourceFormatForValue ));
        }
        else
        {
            eDescr = (bText? CHDESCR_TEXTANDPERCENT : CHDESCR_PERCENT);
//DLNF            rOutAttrs.Put( SfxUInt32Item( SCHATTR_PERCENT_NUMBERFORMAT_VALUE, m_nNumberFormatForPercent ));
//DLNF            rOutAttrs.Put( SfxBoolItem( SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_bSourceFormatForPercent ));
        }
    }
    else if( m_aCbText.IsChecked() )
        eDescr = CHDESCR_TEXT;
    else
        eDescr = CHDESCR_NONE;

    if( m_aCbValue.GetState()!= STATE_DONTKNOW || m_aCbText.GetState()!= STATE_DONTKNOW )
    {
        rOutAttrs.Put(SvxChartDataDescrItem(eDescr, SCHATTR_DATADESCR_DESCR));
        rOutAttrs.Put(SfxBoolItem( SCHATTR_DATADESCR_SHOW_SYM, m_aCbSymbol.IsChecked()) );
    }
    return TRUE;
}

void DataLabelResources::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;

    // default state
    m_aRbNumber.Enable( FALSE );
    m_aRbPercent.Enable( FALSE );
    m_aCbSymbol.Enable( FALSE );

    m_aCbValue.EnableTriState( FALSE );
    m_aCbText.EnableTriState( FALSE );
    m_aCbSymbol.EnableTriState( FALSE );

    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_SHOW_SYM, TRUE, &pPoolItem) == SFX_ITEM_SET )
        m_aCbSymbol.Check( ((const SfxBoolItem*)pPoolItem)->GetValue() );
    else
    {
        m_aCbSymbol.EnableTriState( TRUE );
        m_aCbSymbol.SetState( STATE_DONTKNOW );
    }

    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_DESCR,
                              TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        switch( ((const SvxChartDataDescrItem*)pPoolItem)->GetValue() )
        {
            case CHDESCR_VALUE:
                m_aCbValue.Check();
                m_aRbNumber.Check();
                break;

            case CHDESCR_PERCENT:
                m_aCbValue.Check();
                m_aRbPercent.Check();
                break;

            case CHDESCR_TEXT:
                m_aCbText.Check();
                break;

            case CHDESCR_TEXTANDPERCENT:
                m_aCbText.Check();
                m_aCbValue.Check();
                m_aRbPercent.Check();
                break;

            case CHDESCR_TEXTANDVALUE:
                m_aCbText.Check();
                m_aCbValue.Check();
                m_aRbNumber.Check();
                break;

            default:
                break;
        }
    }
    else
    {
        m_aCbText.EnableTriState( TRUE );
        m_aCbText.SetState( STATE_DONTKNOW );

        m_aCbValue.EnableTriState( TRUE );
        m_aCbValue.SetState( STATE_DONTKNOW );
    }
    if( !m_aRbNumber.IsChecked() && !m_aRbPercent.IsChecked() )
        m_aRbNumber.Check();

//DLNF    lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue );
//DLNF    lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent );

    EnableControls();
}

//.............................................................................
} //namespace chart
//.............................................................................
