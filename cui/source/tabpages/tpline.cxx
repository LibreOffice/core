/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include <editeng/sizeitem.hxx>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>

#include <strings.hrc>
#include <svx/colorbox.hxx>
#include <svx/xattr.hxx>
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <cuitabline.hxx>
#include <dlgname.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/svxgrahicitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dialoghelper.hxx>
#include <svx/ofaitem.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/svdmodel.hxx>
#include <svx/numvset.hxx>
#include <editeng/numitem.hxx>
#include <editeng/svxenum.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/brushitem.hxx>
#include <svx/gallery.hxx>
#include <sfx2/opengrf.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>
#include <vcl/settings.hxx>
#include <cuitabarea.hxx>

#define MAX_BMP_WIDTH   16
#define MAX_BMP_HEIGHT  16

using namespace com::sun::star;

// static ----------------------------------------------------------------

const sal_uInt16 SvxLineTabPage::pLineRanges[] =
{
    XATTR_LINETRANSPARENCE,
    XATTR_LINETRANSPARENCE,
    SID_ATTR_LINE_STYLE,
    SID_ATTR_LINE_ENDCENTER,
    0
};

SvxLineTabPage::SvxLineTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "cui/ui/linetabpage.ui", "LineTabPage", &rInAttrs)
    , m_pSymbolList(nullptr)
    , m_bNewSize(false)
    , m_nSymbolType(SVX_SYMBOLTYPE_UNKNOWN) // unknown respectively unchanged
    , m_pSymbolAttr(nullptr)
    , m_bLastWidthModified(false)
    , m_aSymbolLastSize(Size(0,0))
    , m_bSymbols(false)
    , m_rOutAttrs(rInAttrs)
    , m_bObjSelected(false)
    , m_aXLineAttr(rInAttrs.GetPool())
    , m_rXLSet(m_aXLineAttr.GetItemSet())
    , m_pnLineEndListState(nullptr)
    , m_pnDashListState(nullptr)
    , m_pnColorListState(nullptr)
    , m_nPageType(PageType::Area)
    , m_nDlgType(0)
    , m_pPosDashLb(nullptr)
    , m_pPosLineEndLb(nullptr)
    , m_xBoxColor(m_xBuilder->weld_widget("boxCOLOR"))
    , m_xLbLineStyle(new SvxLineLB(m_xBuilder->weld_combo_box("LB_LINE_STYLE")))
    , m_xLbColor(new ColorListBox(m_xBuilder->weld_menu_button("LB_COLOR"), pParent.GetFrameWeld()))
    , m_xBoxWidth(m_xBuilder->weld_widget("boxWIDTH"))
    , m_xMtrLineWidth(m_xBuilder->weld_metric_spin_button("MTR_FLD_LINE_WIDTH", FieldUnit::CM))
    , m_xBoxTransparency(m_xBuilder->weld_widget("boxTRANSPARENCY"))
    , m_xMtrTransparent(m_xBuilder->weld_metric_spin_button("MTR_LINE_TRANSPARENT", FieldUnit::PERCENT))
    , m_xFlLineEnds(m_xBuilder->weld_widget("FL_LINE_ENDS"))
    , m_xBoxArrowStyles(m_xBuilder->weld_widget("boxARROW_STYLES"))
    , m_xLbStartStyle(new SvxLineEndLB(m_xBuilder->weld_combo_box("LB_START_STYLE")))
    , m_xBoxStart(m_xBuilder->weld_widget("boxSTART"))
    , m_xMtrStartWidth(m_xBuilder->weld_metric_spin_button("MTR_FLD_START_WIDTH", FieldUnit::CM))
    , m_xTsbCenterStart(m_xBuilder->weld_check_button("TSB_CENTER_START"))
    , m_xBoxEnd(m_xBuilder->weld_widget("boxEND"))
    , m_xLbEndStyle(new SvxLineEndLB(m_xBuilder->weld_combo_box("LB_END_STYLE")))
    , m_xMtrEndWidth(m_xBuilder->weld_metric_spin_button("MTR_FLD_END_WIDTH", FieldUnit::CM))
    , m_xTsbCenterEnd(m_xBuilder->weld_check_button("TSB_CENTER_END"))
    , m_xCbxSynchronize(m_xBuilder->weld_check_button("CBX_SYNCHRONIZE"))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, "CTL_PREVIEW", m_aCtlPreview))
    , m_xFLEdgeStyle(m_xBuilder->weld_widget("FL_EDGE_STYLE"))
    , m_xGridEdgeCaps(m_xBuilder->weld_widget("gridEDGE_CAPS"))
    , m_xLBEdgeStyle(m_xBuilder->weld_combo_box("LB_EDGE_STYLE"))
    , m_xLBCapStyle(m_xBuilder->weld_combo_box("LB_CAP_STYLE")) // LineCaps
    , m_xFlSymbol(m_xBuilder->weld_widget("FL_SYMBOL_FORMAT")) //#58425# Symbols on a line (e.g. StarChart)
    , m_xGridIconSize(m_xBuilder->weld_widget("gridICON_SIZE"))
    , m_xSymbolMB(m_xBuilder->weld_menu_button("MB_SYMBOL_BITMAP"))
    , m_xSymbolWidthMF(m_xBuilder->weld_metric_spin_button("MF_SYMBOL_WIDTH", FieldUnit::CM))
    , m_xSymbolHeightMF(m_xBuilder->weld_metric_spin_button("MF_SYMBOL_HEIGHT", FieldUnit::CM))
    , m_xSymbolRatioCB(m_xBuilder->weld_check_button("CB_SYMBOL_RATIO"))
{
    // This Page requires ExchangeSupport
    SetExchangeSupport();

    // Metric set
    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FieldUnit::M:
        case FieldUnit::KM:
            eFUnit = FieldUnit::MM;
            [[fallthrough]]; // we now have mm
        case FieldUnit::MM:
            m_xMtrLineWidth->set_increments(50, 500, FieldUnit::NONE);
            m_xMtrStartWidth->set_increments(50, 500, FieldUnit::NONE);
            m_xMtrEndWidth->set_increments(50, 500, FieldUnit::NONE);
            break;

            case FieldUnit::INCH:
            m_xMtrLineWidth->set_increments(2, 20, FieldUnit::NONE);
            m_xMtrStartWidth->set_increments(2, 20, FieldUnit::NONE);
            m_xMtrEndWidth->set_increments(2, 20, FieldUnit::NONE);
            break;
            default: ;// prevent warning
    }
    SetFieldUnit( *m_xMtrLineWidth, eFUnit );
    SetFieldUnit( *m_xMtrStartWidth, eFUnit );
    SetFieldUnit( *m_xMtrEndWidth, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = m_rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    m_ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    m_xLbLineStyle->connect_changed(LINK(this, SvxLineTabPage, ClickInvisibleHdl_Impl));
    m_xLbColor->SetSelectHdl( LINK( this, SvxLineTabPage, ChangePreviewListBoxHdl_Impl ) );
    m_xMtrLineWidth->connect_value_changed(LINK(this, SvxLineTabPage, ChangePreviewModifyHdl_Impl));
    m_xMtrTransparent->connect_value_changed(LINK( this, SvxLineTabPage, ChangeTransparentHdl_Impl));

    m_xLbStartStyle->connect_changed(LINK(this, SvxLineTabPage, ChangeStartListBoxHdl_Impl));
    m_xLbEndStyle->connect_changed(LINK(this, SvxLineTabPage, ChangeEndListBoxHdl_Impl));
    m_xMtrStartWidth->connect_value_changed(LINK(this, SvxLineTabPage, ChangeStartModifyHdl_Impl));
    m_xMtrEndWidth->connect_value_changed(LINK( this, SvxLineTabPage, ChangeEndModifyHdl_Impl));
    m_xTsbCenterStart->connect_clicked(LINK(this, SvxLineTabPage, ChangeStartClickHdl_Impl));
    m_xTsbCenterEnd->connect_clicked(LINK(this, SvxLineTabPage, ChangeEndClickHdl_Impl));

    Link<weld::ComboBox&,void> aEdgeStyle = LINK(this, SvxLineTabPage, ChangeEdgeStyleHdl_Impl);
    m_xLBEdgeStyle->connect_changed(aEdgeStyle);

    // LineCaps
    Link<weld::ComboBox&,void> aCapStyle = LINK(this, SvxLineTabPage, ChangeCapStyleHdl_Impl);
    m_xLBCapStyle->connect_changed(aCapStyle);

    // Symbols on a line (eg star charts), MB-handler set
    m_xSymbolMB->connect_selected(LINK(this, SvxLineTabPage, GraphicHdl_Impl));
    m_xSymbolMB->connect_toggled(LINK(this, SvxLineTabPage, MenuCreateHdl_Impl));
    m_xSymbolWidthMF->connect_value_changed(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    m_xSymbolHeightMF->connect_value_changed(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    m_xSymbolRatioCB->connect_toggled(LINK(this, SvxLineTabPage, RatioHdl_Impl));

    m_xSymbolRatioCB->set_active(true);
    ShowSymbolControls(false);

    m_nActLineWidth = -1;
}

void SvxLineTabPage::ShowSymbolControls(bool bOn)
{
    // Symbols on a line (e.g. StarCharts), symbol-enable controls

    m_bSymbols=bOn;
    m_xFlSymbol->set_visible(bOn);
    m_aCtlPreview.ShowSymbol(bOn);
}

SvxLineTabPage::~SvxLineTabPage()
{
    disposeOnce();
}

void SvxLineTabPage::dispose()
{
    m_xCtlPreview.reset();
    m_xLbEndStyle.reset();
    m_xLbStartStyle.reset();
    m_xLbColor.reset();
    m_xLbLineStyle.reset();
    m_aGalleryBrushItems.clear();
    m_aSymbolBrushItems.clear();

    SfxTabPage::dispose();
}

void SvxLineTabPage::Construct()
{
    FillListboxes();
}

void SvxLineTabPage::FillListboxes()
{
    // Line styles
    auto nOldSelect = m_xLbLineStyle->get_active();
    // aLbLineStyle.FillStyles();
    m_xLbLineStyle->Fill( m_pDashList );
    m_xLbLineStyle->set_active( nOldSelect );

    // Line end style
    OUString sNone( SvxResId( RID_SVXSTR_NONE ) );
    nOldSelect = m_xLbStartStyle->get_active();
    m_xLbStartStyle->clear();
    m_xLbStartStyle->append_text(sNone);
    m_xLbStartStyle->Fill(m_pLineEndList);
    m_xLbStartStyle->set_active(nOldSelect);
    nOldSelect = m_xLbEndStyle->get_active();
    m_xLbEndStyle->clear();
    m_xLbEndStyle->append_text(sNone);
    m_xLbEndStyle->Fill(m_pLineEndList, false);
    m_xLbEndStyle->set_active(nOldSelect);
}

void SvxLineTabPage::ActivatePage( const SfxItemSet& rSet )
{
    const CntUInt16Item* pPageTypeItem = rSet.GetItem<CntUInt16Item>(SID_PAGE_TYPE, false);
    if (pPageTypeItem)
        SetPageType(static_cast<PageType>(pPageTypeItem->GetValue()));
    if( m_nDlgType == 0 && m_pDashList.is() )
    {
        sal_Int32 nPos;
        sal_Int32 nCount;

        // Dash list
        if( ( *m_pnDashListState & ChangeType::MODIFIED ) ||
            ( *m_pnDashListState & ChangeType::CHANGED ) )
        {
            if( *m_pnDashListState & ChangeType::CHANGED )
                m_pDashList = static_cast<SvxLineTabDialog*>(GetDialogController() )->GetNewDashList();

            *m_pnDashListState = ChangeType::NONE;

            // Style list
            nPos = m_xLbLineStyle->get_active();

            m_xLbLineStyle->clear();
            m_xLbLineStyle->append_text(SvxResId(RID_SVXSTR_INVISIBLE));
            m_xLbLineStyle->append_text(SvxResId(RID_SVXSTR_SOLID));
            m_xLbLineStyle->Fill(m_pDashList);
            nCount = m_xLbLineStyle->get_count();

            if ( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                m_xLbLineStyle->set_active(0);
            else
                m_xLbLineStyle->set_active(nPos);
        }

        INetURLObject   aDashURL( m_pDashList->GetPath() );

        aDashURL.Append( m_pDashList->GetName() );
        DBG_ASSERT( aDashURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
        // LineEnd list
        if( ( *m_pnLineEndListState & ChangeType::MODIFIED ) || ( *m_pnLineEndListState & ChangeType::CHANGED ) )
        {
            if( *m_pnLineEndListState & ChangeType::CHANGED )
                m_pLineEndList = static_cast<SvxLineTabDialog*>(GetDialogController())->GetNewLineEndList();

            *m_pnLineEndListState = ChangeType::NONE;

            nPos = m_xLbLineStyle->get_active();
            OUString sNone(SvxResId(RID_SVXSTR_NONE));
            m_xLbStartStyle->clear();
            m_xLbStartStyle->append_text(sNone);

            m_xLbStartStyle->Fill( m_pLineEndList );
            nCount = m_xLbStartStyle->get_count();
            if( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                m_xLbStartStyle->set_active(0);
            else
                m_xLbStartStyle->set_active(nPos);

            m_xLbEndStyle->clear();
            m_xLbEndStyle->append_text(sNone);

            m_xLbEndStyle->Fill( m_pLineEndList, false );
            nCount = m_xLbEndStyle->get_count();

            if( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                m_xLbEndStyle->set_active(0);
            else
                m_xLbEndStyle->set_active(nPos);
        }
        INetURLObject aLineURL( m_pLineEndList->GetPath() );

        aLineURL.Append( m_pLineEndList->GetName() );
        DBG_ASSERT( aLineURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
        // Evaluate if another TabPage set another fill type
        if( m_xLbLineStyle->get_active() != 0 )
        {
            if( m_nPageType == PageType::Hatch ) // 1
            {
                m_xLbLineStyle->set_active(*m_pPosDashLb + 2); // +2 due to SOLID and INVISIBLE
                ChangePreviewHdl_Impl( nullptr );
            }
            if( m_nPageType == PageType::Bitmap )
            {
                m_xLbStartStyle->set_active(*m_pPosLineEndLb + 1);// +1 due to SOLID
                m_xLbEndStyle->set_active(*m_pPosLineEndLb + 1);// +1 due to SOLID
                ChangePreviewHdl_Impl( nullptr );
            }
        }

            // ColorList
        if( *m_pnColorListState != ChangeType::NONE )
        {
            ChangePreviewHdl_Impl( nullptr );
        }

        m_nPageType = PageType::Area;
    }
    // Page does not yet exist in the ctor, that's why we do it here!

    else if (m_nDlgType == 1101) // nNoArrowNoShadowDlg from chart2/source/controller/dialogs/dlg_ObjectProperties.cxx
    {
        m_xFlLineEnds->hide();
        m_xFLEdgeStyle->hide();
    }
}


DeactivateRC SvxLineTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( m_nDlgType == 0 ) // Line dialog
    {
        m_nPageType = PageType::Gradient; // possibly for extensions
        *m_pPosDashLb = m_xLbLineStyle->get_active() - 2;// First entry SOLID!!!
        sal_Int32 nPos = m_xLbStartStyle->get_active();
        if (nPos != -1)
            nPos--;
        *m_pPosLineEndLb = nPos;
    }

    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxLineTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    const SfxPoolItem* pOld = nullptr;
    sal_Int32  nPos;
    bool    bModified = false;

    // To prevent modifications to the list, we do not set other page's items.
    if( m_nDlgType != 0 || m_nPageType != PageType::Hatch )
    {
        nPos = m_xLbLineStyle->get_active();
        if( nPos != -1 &&
            m_xLbLineStyle->get_value_changed_from_saved() )
        {
            std::unique_ptr<XLineStyleItem> pStyleItem;

            if( nPos == 0 )
                pStyleItem.reset(new XLineStyleItem( drawing::LineStyle_NONE ));
            else if( nPos == 1 )
                pStyleItem.reset(new XLineStyleItem( drawing::LineStyle_SOLID ));
            else
            {
                pStyleItem.reset(new XLineStyleItem( drawing::LineStyle_DASH ));

                // For added security
                if( m_pDashList->Count() > static_cast<long>( nPos - 2 ) )
                {
                    XLineDashItem aDashItem( m_xLbLineStyle->get_active_text(),
                                        m_pDashList->GetDash( nPos - 2 )->GetDash() );
                    pOld = GetOldItem( *rAttrs, XATTR_LINEDASH );
                    if ( !pOld || !( *static_cast<const XLineDashItem*>(pOld) == aDashItem ) )
                    {
                        rAttrs->Put( aDashItem );
                        bModified = true;
                    }
                }
            }
            pOld = GetOldItem( *rAttrs, XATTR_LINESTYLE );
            if ( !pOld || !( *static_cast<const XLineStyleItem*>(pOld) == *pStyleItem ) )
            {
                rAttrs->Put( *pStyleItem );
                bModified = true;
            }
        }
    }
    // Line width
    // GetSavedValue() returns OUString!
    if( m_xMtrLineWidth->get_value_changed_from_saved() )
    {
        XLineWidthItem aItem( GetCoreValue( *m_xMtrLineWidth, m_ePoolUnit ) );
        pOld = GetOldItem( *rAttrs, XATTR_LINEWIDTH );
        if ( !pOld || !( *static_cast<const XLineWidthItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }
    // Width line start
    if( m_xMtrStartWidth->get_value_changed_from_saved() )
    {
        XLineStartWidthItem aItem( GetCoreValue( *m_xMtrStartWidth, m_ePoolUnit ) );
        pOld = GetOldItem( *rAttrs, XATTR_LINESTARTWIDTH );
        if ( !pOld || !( *static_cast<const XLineStartWidthItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }
    // Width line end
    if( m_xMtrEndWidth->get_value_changed_from_saved() )
    {
        XLineEndWidthItem aItem( GetCoreValue( *m_xMtrEndWidth, m_ePoolUnit ) );
        pOld = GetOldItem( *rAttrs, XATTR_LINEENDWIDTH );
        if ( !pOld || !( *static_cast<const XLineEndWidthItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    // Line color
    {
        NamedColor aColor = m_xLbColor->GetSelectedEntry();
        XLineColorItem aItem(aColor.second, aColor.first);
        pOld = GetOldItem( *rAttrs, XATTR_LINECOLOR );
        if ( !pOld || !( *static_cast<const XLineColorItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    if( m_nDlgType != 0 || m_nPageType != PageType::Bitmap )
    {
        // Line start
        nPos = m_xLbStartStyle->get_active();
        if( nPos != -1 && m_xLbStartStyle->get_value_changed_from_saved() )
        {
            std::unique_ptr<XLineStartItem> pItem;
            if( nPos == 0 )
                pItem.reset(new XLineStartItem());
            else if( m_pLineEndList->Count() > static_cast<long>( nPos - 1 ) )
                pItem.reset(new XLineStartItem( m_xLbStartStyle->get_active_text(), m_pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
            pOld = GetOldItem( *rAttrs, XATTR_LINESTART );
            if( pItem && ( !pOld || !( *static_cast<const XLineEndItem*>(pOld) == *pItem ) ) )
            {
                rAttrs->Put( *pItem );
                bModified = true;
            }
        }
        // Line end
        nPos = m_xLbEndStyle->get_active();
        if( nPos != -1 && m_xLbEndStyle->get_value_changed_from_saved() )
        {
            std::unique_ptr<XLineEndItem> pItem;
            if( nPos == 0 )
                pItem.reset(new XLineEndItem());
            else if( m_pLineEndList->Count() > static_cast<long>( nPos - 1 ) )
                pItem.reset(new XLineEndItem( m_xLbEndStyle->get_active_text(), m_pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
            pOld = GetOldItem( *rAttrs, XATTR_LINEEND );
            if( pItem &&
                ( !pOld || !( *static_cast<const XLineEndItem*>(pOld) == *pItem ) ) )
            {
                rAttrs->Put( *pItem );
                bModified = true;
            }
        }
    }

    // Centered line end
    TriState eState = m_xTsbCenterStart->get_state();
    if( m_xTsbCenterStart->get_state_changed_from_saved() )
    {
        XLineStartCenterItem aItem( eState != TRISTATE_FALSE );
        pOld = GetOldItem( *rAttrs, XATTR_LINESTARTCENTER );
        if ( !pOld || !( *static_cast<const XLineStartCenterItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }
    eState = m_xTsbCenterEnd->get_state();
    if( m_xTsbCenterEnd->get_state_changed_from_saved() )
    {
        XLineEndCenterItem aItem( eState != TRISTATE_FALSE );
        pOld = GetOldItem( *rAttrs, XATTR_LINEENDCENTER );
        if ( !pOld || !( *static_cast<const XLineEndCenterItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    // Transparency
    sal_uInt16 nVal = m_xMtrTransparent->get_value(FieldUnit::PERCENT);
    if( m_xMtrTransparent->get_value_changed_from_saved() )
    {
        XLineTransparenceItem aItem( nVal );
        pOld = GetOldItem( *rAttrs, XATTR_LINETRANSPARENCE );
        if ( !pOld || !( *static_cast<const XLineTransparenceItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    nPos = m_xLBEdgeStyle->get_active();
    if (nPos != -1 && m_xLBEdgeStyle->get_value_changed_from_saved())
    {
        std::unique_ptr<XLineJointItem> pNew;

        switch(nPos)
        {
            case 0: // Rounded, default
            {
                pNew.reset(new XLineJointItem(css::drawing::LineJoint_ROUND));
                break;
            }
            case 1: // - none -
            {
                pNew.reset(new XLineJointItem(css::drawing::LineJoint_NONE));
                break;
            }
            case 2: // Miter
            {
                pNew.reset(new XLineJointItem(css::drawing::LineJoint_MITER));
                break;
            }
            case 3: // Bevel
            {
                pNew.reset(new XLineJointItem(css::drawing::LineJoint_BEVEL));
                break;
            }
        }

        if(pNew)
        {
            pOld = GetOldItem( *rAttrs, XATTR_LINEJOINT );

            if(!pOld || !(*static_cast<const XLineJointItem*>(pOld) == *pNew))
            {
                rAttrs->Put( *pNew );
                bModified = true;
            }
        }
    }

    // LineCaps
    nPos = m_xLBCapStyle->get_active();
    if (nPos != -1 && m_xLBCapStyle->get_value_changed_from_saved())
    {
        std::unique_ptr<XLineCapItem> pNew;

        switch(nPos)
        {
            case 0: // Butt (=Flat), default
            {
                pNew.reset(new XLineCapItem(css::drawing::LineCap_BUTT));
                break;
            }
            case 1: // Round
            {
                pNew.reset(new XLineCapItem(css::drawing::LineCap_ROUND));
                break;
            }
            case 2: // Square
            {
                pNew.reset(new XLineCapItem(css::drawing::LineCap_SQUARE));
                break;
            }
        }

        if(pNew)
        {
            pOld = GetOldItem( *rAttrs, XATTR_LINECAP );

            if(!pOld || !(*static_cast<const XLineCapItem*>(pOld) == *pNew))
            {
                rAttrs->Put( *pNew );
                bModified = true;
            }
        }
    }

    if(m_nSymbolType!=SVX_SYMBOLTYPE_UNKNOWN || m_bNewSize)
    {
        // Was set by selection or the size is different
        SvxSizeItem  aSItem(rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE),m_aSymbolSize);
        const SfxPoolItem* pSOld = GetOldItem( *rAttrs, rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE) );
        m_bNewSize  = pSOld ? *static_cast<const SvxSizeItem *>(pSOld) != aSItem : m_bNewSize ;
        if(m_bNewSize)
        {
            rAttrs->Put(aSItem);
            bModified=true;
        }

        SfxInt32Item aTItem(rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE),m_nSymbolType);
        const SfxPoolItem* pTOld = GetOldItem( *rAttrs, rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE) );
        bool bNewType = pTOld == nullptr || *static_cast<const SfxInt32Item*>(pTOld) != aTItem;
        if(bNewType && m_nSymbolType==SVX_SYMBOLTYPE_UNKNOWN)
            bNewType=false; // a small fix, type wasn't set -> don't create a type item after all!
        if(bNewType)
        {
            rAttrs->Put(aTItem);
            bModified=true;
        }

        if(m_nSymbolType!=SVX_SYMBOLTYPE_NONE)
        {
            SvxBrushItem aBItem(m_aSymbolGraphic,GPOS_MM,rAttrs->GetPool()->GetWhich(SID_ATTR_BRUSH));
            const SfxPoolItem* pBOld = GetOldItem( *rAttrs, rAttrs->GetPool()->GetWhich(SID_ATTR_BRUSH) );
            bool bNewBrush =
                pBOld == nullptr || *static_cast<const SvxBrushItem*>(pBOld) != aBItem;
            if(bNewBrush)
            {
                rAttrs->Put(aBItem);
                bModified=true;
            }
        }
    }
    rAttrs->Put (CntUInt16Item(SID_PAGE_TYPE, static_cast<sal_uInt16>(m_nPageType)));
    return bModified;
}


void SvxLineTabPage::FillXLSet_Impl()
{
    sal_Int32 nPos;

    if (m_xLbLineStyle->get_active() == -1)
    {
        m_rXLSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
    }
    else if (m_xLbLineStyle->get_active() == 0)
        m_rXLSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
    else if (m_xLbLineStyle->get_active() == 1)
        m_rXLSet.Put( XLineStyleItem( drawing::LineStyle_SOLID ) );
    else
    {
        m_rXLSet.Put( XLineStyleItem( drawing::LineStyle_DASH ) );

        nPos = m_xLbLineStyle->get_active();
        if (nPos != -1)
        {
            m_rXLSet.Put( XLineDashItem( m_xLbLineStyle->get_active_text(),
                            m_pDashList->GetDash( nPos - 2 )->GetDash() ) );
        }
    }

    nPos = m_xLbStartStyle->get_active();
    if (nPos != -1)
    {
        if( nPos == 0 )
            m_rXLSet.Put( XLineStartItem() );
        else
            m_rXLSet.Put( XLineStartItem( m_xLbStartStyle->get_active_text(),
                        m_pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }
    nPos = m_xLbEndStyle->get_active();
    if (nPos != -1)
    {
        if( nPos == 0 )
            m_rXLSet.Put( XLineEndItem() );
        else
            m_rXLSet.Put( XLineEndItem( m_xLbEndStyle->get_active_text(),
                        m_pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }

    nPos = m_xLBEdgeStyle->get_active();
    if (nPos != -1)
    {
        switch(nPos)
        {
            case 0: // Rounded, default
            {
                m_rXLSet.Put(XLineJointItem(css::drawing::LineJoint_ROUND));
                break;
            }
            case 1: // - none -
            {
                m_rXLSet.Put(XLineJointItem(css::drawing::LineJoint_NONE));
                break;
            }
            case 2: // Miter
            {
                m_rXLSet.Put(XLineJointItem(css::drawing::LineJoint_MITER));
                break;
            }
            case 3: // Bevel
            {
                m_rXLSet.Put(XLineJointItem(css::drawing::LineJoint_BEVEL));
                break;
            }
        }
    }

    // LineCaps
    nPos = m_xLBCapStyle->get_active();
    if (nPos != -1)
    {
        switch(nPos)
        {
            case 0: // Butt (=Flat), default
            {
                m_rXLSet.Put(XLineCapItem(css::drawing::LineCap_BUTT));
                break;
            }
            case 1: // Round
            {
                m_rXLSet.Put(XLineCapItem(css::drawing::LineCap_ROUND));
                break;
            }
            case 2: // Square
            {
                m_rXLSet.Put(XLineCapItem(css::drawing::LineCap_SQUARE));
                break;
            }
        }
    }

    m_rXLSet.Put( XLineStartWidthItem( GetCoreValue( *m_xMtrStartWidth, m_ePoolUnit ) ) );
    m_rXLSet.Put( XLineEndWidthItem( GetCoreValue( *m_xMtrEndWidth, m_ePoolUnit ) ) );

    m_rXLSet.Put( XLineWidthItem( GetCoreValue( *m_xMtrLineWidth, m_ePoolUnit ) ) );
    NamedColor aColor = m_xLbColor->GetSelectedEntry();
    m_rXLSet.Put(XLineColorItem(aColor.second, aColor.first));

    // Centered line end
    if( m_xTsbCenterStart->get_state() == TRISTATE_TRUE )
        m_rXLSet.Put( XLineStartCenterItem( true ) );
    else if( m_xTsbCenterStart->get_state() == TRISTATE_FALSE )
        m_rXLSet.Put( XLineStartCenterItem( false ) );

    if( m_xTsbCenterEnd->get_state() == TRISTATE_TRUE )
        m_rXLSet.Put( XLineEndCenterItem( true ) );
    else if( m_xTsbCenterEnd->get_state() == TRISTATE_FALSE )
        m_rXLSet.Put( XLineEndCenterItem( false ) );

    // Transparency
    sal_uInt16 nVal = m_xMtrTransparent->get_value(FieldUnit::PERCENT);
    m_rXLSet.Put( XLineTransparenceItem( nVal ) );

    m_aCtlPreview.SetLineAttributes(m_aXLineAttr.GetItemSet());
}


void SvxLineTabPage::Reset( const SfxItemSet* rAttrs )
{
    drawing::LineStyle  eXLS; // drawing::LineStyle_NONE, drawing::LineStyle_SOLID, drawing::LineStyle_DASH

    // Line style
    const SfxPoolItem *pPoolItem;
    long nSymType=SVX_SYMBOLTYPE_UNKNOWN;
    bool bPrevSym=false;
    bool bEnable=true;
    bool bIgnoreGraphic=false;
    bool bIgnoreSize=false;
    if(rAttrs->GetItemState(rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE),true,&pPoolItem) == SfxItemState::SET)
    {
        nSymType=static_cast<const SfxInt32Item *>(pPoolItem)->GetValue();
    }

    if(nSymType == SVX_SYMBOLTYPE_AUTO)
    {
        m_aSymbolGraphic=m_aAutoSymbolGraphic;
        m_aSymbolSize=m_aSymbolLastSize=m_aAutoSymbolGraphic.GetPrefSize();
        bPrevSym=true;
    }
    else if(nSymType == SVX_SYMBOLTYPE_NONE)
    {
        bEnable=false;
        bIgnoreGraphic=true;
        bIgnoreSize=true;
    }
    else if(nSymType >= 0)
    {
        ScopedVclPtrInstance< VirtualDevice > pVDev;
        pVDev->SetMapMode(MapMode(MapUnit::Map100thMM));

        std::unique_ptr<SdrModel> pModel(
            new SdrModel(nullptr, nullptr, true));
        pModel->GetItemPool().FreezeIdRanges();
        SdrPage* pPage = new SdrPage( *pModel, false );
        pPage->SetSize(Size(1000,1000));
        pModel->InsertPage( pPage, 0 );
        {
        std::unique_ptr<SdrView> pView(new SdrView( *pModel, pVDev ));
        pView->hideMarkHandles();
        pView->ShowSdrPage(pPage);
        SdrObject *pObj=nullptr;
        size_t nSymTmp = static_cast<size_t>(nSymType);
        if(m_pSymbolList)
        {
            if(m_pSymbolList->GetObjCount())
            {
                nSymTmp %= m_pSymbolList->GetObjCount(); // Treat list as cyclic!
                pObj=m_pSymbolList->GetObj(nSymTmp);
                if(pObj)
                {
                    // directly clone to target SdrModel
                    pObj = pObj->CloneSdrObject(*pModel);

                    if(m_pSymbolAttr)
                    {
                        pObj->SetMergedItemSet(*m_pSymbolAttr);
                    }
                    else
                    {
                        pObj->SetMergedItemSet(m_rOutAttrs);
                    }

                    pPage->NbcInsertObject(pObj);

                    // Generate invisible square to give all symbol types a
                    // bitmap size, which is independent from specific glyph
                    SdrObject* pInvisibleSquare(m_pSymbolList->GetObj(0));

                    // directly clone to target SdrModel
                    pInvisibleSquare = pInvisibleSquare->CloneSdrObject(*pModel);

                    pPage->NbcInsertObject(pInvisibleSquare);
                    pInvisibleSquare->SetMergedItem(XFillTransparenceItem(100));
                    pInvisibleSquare->SetMergedItem(XLineTransparenceItem(100));

                    pView->MarkAll();
                    GDIMetaFile aMeta(pView->GetMarkedObjMetaFile());

                    m_aSymbolGraphic=Graphic(aMeta);
                    m_aSymbolSize=pObj->GetSnapRect().GetSize();
                    m_aSymbolGraphic.SetPrefSize(pInvisibleSquare->GetSnapRect().GetSize());
                    m_aSymbolGraphic.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
                    bPrevSym=true;
                    bEnable=true;
                    bIgnoreGraphic=true;

                    pView->UnmarkAll();
                    pInvisibleSquare=pPage->RemoveObject(1);
                    SdrObject::Free( pInvisibleSquare);
                    pObj=pPage->RemoveObject(0);
                    SdrObject::Free( pObj );
                }
            }
        }
        }
    }
    if(rAttrs->GetItemState(rAttrs->GetPool()->GetWhich(SID_ATTR_BRUSH),true,&pPoolItem) == SfxItemState::SET)
    {
        const Graphic* pGraphic = static_cast<const SvxBrushItem *>(pPoolItem)->GetGraphic();
        if( pGraphic )
        {
            if(!bIgnoreGraphic)
            {
                m_aSymbolGraphic=*pGraphic;
            }
            if(!bIgnoreSize)
            {
                m_aSymbolSize=OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
                                                        pGraphic->GetPrefMapMode(),
                                                        MapMode(MapUnit::Map100thMM));
            }
            bPrevSym=true;
        }
    }

    if(rAttrs->GetItemState(rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE),true,&pPoolItem) == SfxItemState::SET)
    {
        m_aSymbolSize = static_cast<const SvxSizeItem *>(pPoolItem)->GetSize();
    }

    m_xGridIconSize->set_sensitive(bEnable);

    if(bPrevSym)
    {
        SetMetricValue(*m_xSymbolWidthMF,  m_aSymbolSize.Width(), m_ePoolUnit);
        SetMetricValue(*m_xSymbolHeightMF, m_aSymbolSize.Height(),m_ePoolUnit);
        m_aCtlPreview.SetSymbol(&m_aSymbolGraphic,m_aSymbolSize);
        m_aSymbolLastSize=m_aSymbolSize;
    }

    if( rAttrs->GetItemState( XATTR_LINESTYLE ) != SfxItemState::DONTCARE )
    {
        eXLS = rAttrs->Get( XATTR_LINESTYLE ).GetValue();

        switch( eXLS )
        {
            case drawing::LineStyle_NONE:
                m_xLbLineStyle->set_active(0);
                break;
            case drawing::LineStyle_SOLID:
                m_xLbLineStyle->set_active(1);
                break;

            case drawing::LineStyle_DASH:
                m_xLbLineStyle->set_active(-1);
                m_xLbLineStyle->set_active_text(rAttrs->Get( XATTR_LINEDASH ).GetName());
                break;

            default:
                break;
        }
    }
    else
    {
        m_xLbLineStyle->set_active(-1);
    }

    // Line strength
    if( rAttrs->GetItemState( XATTR_LINEWIDTH ) != SfxItemState::DONTCARE )
    {
        SetMetricValue( *m_xMtrLineWidth, rAttrs->Get( XATTR_LINEWIDTH ).GetValue(), m_ePoolUnit );
    }
    else
        m_xMtrLineWidth->set_text("");

    // Line color
    m_xLbColor->SetNoSelection();

    if ( rAttrs->GetItemState( XATTR_LINECOLOR ) != SfxItemState::DONTCARE )
    {
        Color aCol = rAttrs->Get( XATTR_LINECOLOR ).GetColorValue();
        m_xLbColor->SelectEntry( aCol );
    }

    // Line start
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINESTART ) == SfxItemState::DEFAULT )
    {
        m_xLbStartStyle->set_sensitive(false);
    }
    else if( rAttrs->GetItemState( XATTR_LINESTART ) != SfxItemState::DONTCARE )
    {
        // #86265# select entry using list and polygon, not string
        bool bSelected(false);
        const basegfx::B2DPolyPolygon& rItemPolygon = rAttrs->Get(XATTR_LINESTART).GetLineStartValue();

        for(long a(0);!bSelected &&  a < m_pLineEndList->Count(); a++)
        {
            const XLineEndEntry* pEntry = m_pLineEndList->GetLineEnd(a);
            const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();

            if(rItemPolygon == rEntryPolygon)
            {
                // select this entry
                m_xLbStartStyle->set_active(a + 1);
                bSelected = true;
            }
        }

        if(!bSelected)
            m_xLbStartStyle->set_active(0);
    }
    else
    {
        m_xLbStartStyle->set_active(-1);
    }

    // Line end
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINEEND ) == SfxItemState::DEFAULT )
    {
        m_xLbEndStyle->set_sensitive(false);
    }
    else if( rAttrs->GetItemState( XATTR_LINEEND ) != SfxItemState::DONTCARE )
    {
        // #86265# select entry using list and polygon, not string
        bool bSelected(false);
        const basegfx::B2DPolyPolygon& rItemPolygon = rAttrs->Get(XATTR_LINEEND).GetLineEndValue();

        for(long a(0);!bSelected &&  a < m_pLineEndList->Count(); a++)
        {
            const XLineEndEntry* pEntry = m_pLineEndList->GetLineEnd(a);
            const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();

            if(rItemPolygon == rEntryPolygon)
            {
                // select this entry
                m_xLbEndStyle->set_active(a + 1);
                bSelected = true;
            }
        }

        if(!bSelected)
            m_xLbEndStyle->set_active(0);
    }
    else
    {
        m_xLbEndStyle->set_active(-1);
    }

    // Line start strength
    if( m_bObjSelected &&  rAttrs->GetItemState( XATTR_LINESTARTWIDTH ) == SfxItemState::DEFAULT )
    {
        m_xMtrStartWidth->set_sensitive(false);
    }
    else if( rAttrs->GetItemState( XATTR_LINESTARTWIDTH ) != SfxItemState::DONTCARE )
    {
        SetMetricValue( *m_xMtrStartWidth,
                        rAttrs->Get( XATTR_LINESTARTWIDTH ).GetValue(),
                        m_ePoolUnit );
    }
    else
        m_xMtrStartWidth->set_text( "" );

    // Line end strength
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINEENDWIDTH ) == SfxItemState::DEFAULT )
    {
        m_xMtrEndWidth->set_sensitive(false);
    }
    else if( rAttrs->GetItemState( XATTR_LINEENDWIDTH ) != SfxItemState::DONTCARE )
    {
        SetMetricValue( *m_xMtrEndWidth,
                        rAttrs->Get( XATTR_LINEENDWIDTH ).GetValue(),
                        m_ePoolUnit );
    }
    else
        m_xMtrEndWidth->set_text("");

    // Centered line end (start)
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINESTARTCENTER ) == SfxItemState::DEFAULT )
    {
        m_xTsbCenterStart->set_sensitive(false);
    }
    else if( rAttrs->GetItemState( XATTR_LINESTARTCENTER ) != SfxItemState::DONTCARE )
    {
        if( rAttrs->Get( XATTR_LINESTARTCENTER ).GetValue() )
            m_xTsbCenterStart->set_state(TRISTATE_TRUE);
        else
            m_xTsbCenterStart->set_state(TRISTATE_FALSE);
    }
    else
    {
        m_xTsbCenterStart->set_state(TRISTATE_INDET);
    }

    // Centered line end (end)
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINEENDCENTER ) == SfxItemState::DEFAULT )
    {
        m_xTsbCenterEnd->set_sensitive(false);
    }
    else if( rAttrs->GetItemState( XATTR_LINEENDCENTER ) != SfxItemState::DONTCARE )
    {
        if( rAttrs->Get( XATTR_LINEENDCENTER ).GetValue() )
            m_xTsbCenterEnd->set_state(TRISTATE_TRUE);
        else
            m_xTsbCenterEnd->set_state(TRISTATE_FALSE);
    }
    else
    {
        m_xTsbCenterEnd->set_state(TRISTATE_INDET);
    }

    // Transparency
    if( rAttrs->GetItemState( XATTR_LINETRANSPARENCE ) != SfxItemState::DONTCARE )
    {
        sal_uInt16 nTransp = rAttrs->Get( XATTR_LINETRANSPARENCE ).GetValue();
        m_xMtrTransparent->set_value(nTransp, FieldUnit::PERCENT);
        ChangeTransparentHdl_Impl(*m_xMtrTransparent);
    }
    else
        m_xMtrTransparent->set_text( "" );

    if( !m_xLbStartStyle->get_sensitive()  &&
        !m_xLbEndStyle->get_sensitive()    &&
        !m_xMtrStartWidth->get_sensitive() &&
        !m_xMtrEndWidth->get_sensitive()   &&
        !m_xTsbCenterStart->get_sensitive()&&
        !m_xTsbCenterEnd->get_sensitive() )
    {
        m_xCbxSynchronize->set_sensitive(false);
        m_xFlLineEnds->set_sensitive(false);
    }

    // Synchronize
    // We get the value from the INI file now
    OUString aStr = GetUserData();
    m_xCbxSynchronize->set_active(aStr.toInt32() != 0);

    if(m_bObjSelected && SfxItemState::DEFAULT == rAttrs->GetItemState(XATTR_LINEJOINT))
    {
//         maFTEdgeStyle.set_sensitive(false);
        m_xLBEdgeStyle->set_sensitive(false);
    }
    else if(SfxItemState::DONTCARE != rAttrs->GetItemState(XATTR_LINEJOINT))
    {
        const css::drawing::LineJoint eLineJoint = rAttrs->Get(XATTR_LINEJOINT).GetValue();

        switch(eLineJoint)
        {
            case css::drawing::LineJoint::LineJoint_MAKE_FIXED_SIZE: // fallback to round, unused value
            case css::drawing::LineJoint_ROUND : m_xLBEdgeStyle->set_active(0); break;
            case css::drawing::LineJoint_NONE : m_xLBEdgeStyle->set_active(1); break;
            case css::drawing::LineJoint_MIDDLE : // fallback to mitre, unused value
            case css::drawing::LineJoint_MITER : m_xLBEdgeStyle->set_active(2); break;
            case css::drawing::LineJoint_BEVEL : m_xLBEdgeStyle->set_active(3); break;
        }
    }
    else
    {
        m_xLBEdgeStyle->set_active(-1);
    }

    // fdo#43209
    if(m_bObjSelected && SfxItemState::DEFAULT == rAttrs->GetItemState(XATTR_LINECAP))
    {
        m_xLBCapStyle->set_sensitive(false);
    }
    else if(SfxItemState::DONTCARE != rAttrs->GetItemState(XATTR_LINECAP))
    {
        const css::drawing::LineCap eLineCap(rAttrs->Get(XATTR_LINECAP).GetValue());

        switch(eLineCap)
        {
            case css::drawing::LineCap_ROUND: m_xLBCapStyle->set_active(1); break;
            case css::drawing::LineCap_SQUARE : m_xLBCapStyle->set_active(2); break;
            default /*css::drawing::LineCap_BUTT*/: m_xLBCapStyle->set_active(0); break;
        }
    }
    else
    {
        m_xLBCapStyle->set_active(-1);
    }

    // Save values
    m_xLbLineStyle->save_value();
    m_xMtrLineWidth->save_value();
    m_xLbColor->SaveValue();
    m_xLbStartStyle->save_value();
    m_xLbEndStyle->save_value();
    m_xMtrStartWidth->save_value();
    m_xMtrEndWidth->save_value();
    m_xTsbCenterStart->save_state();
    m_xTsbCenterEnd->save_state();
    m_xMtrTransparent->save_value();

    m_xLBEdgeStyle->save_value();

    // LineCaps
    m_xLBCapStyle->save_value();

    ClickInvisibleHdl_Impl();

    ChangePreviewHdl_Impl( nullptr );
}

VclPtr<SfxTabPage> SvxLineTabPage::Create(TabPageParent pParent,
                                          const SfxItemSet* rAttrs)
{
    return VclPtr<SvxLineTabPage>::Create(pParent, *rAttrs);
}

IMPL_LINK_NOARG(SvxLineTabPage, ChangePreviewListBoxHdl_Impl, ColorListBox&, void)
{
    ChangePreviewHdl_Impl(nullptr);
}

IMPL_LINK(SvxLineTabPage, ChangePreviewModifyHdl_Impl, weld::MetricSpinButton&, rEdit, void)
{
    ChangePreviewHdl_Impl(&rEdit);
}

void SvxLineTabPage::ChangePreviewHdl_Impl(const weld::MetricSpinButton* pCntrl)
{
    if (pCntrl == m_xMtrLineWidth.get())
    {
        // Line width and start end width
        sal_Int32 nNewLineWidth = GetCoreValue( *m_xMtrLineWidth, m_ePoolUnit );
        if(m_nActLineWidth == -1)
        {
            // Don't initialize yet, get the start value
            const SfxPoolItem* pOld = GetOldItem( m_rXLSet, XATTR_LINEWIDTH );
            sal_Int32 nStartLineWidth = 0;
            if(pOld)
                nStartLineWidth = static_cast<const XLineWidthItem *>(pOld)->GetValue();
            m_nActLineWidth = nStartLineWidth;
        }

        if(m_nActLineWidth != nNewLineWidth)
        {
            // Adapt start/end width
            sal_Int32 nValAct = GetCoreValue( *m_xMtrStartWidth, m_ePoolUnit );
            sal_Int32 nValNew = nValAct + (((nNewLineWidth - m_nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( *m_xMtrStartWidth, nValNew, m_ePoolUnit );

            nValAct = GetCoreValue( *m_xMtrEndWidth, m_ePoolUnit );
            nValNew = nValAct + (((nNewLineWidth - m_nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( *m_xMtrEndWidth, nValNew, m_ePoolUnit );
        }

        // Remember current value
        m_nActLineWidth = nNewLineWidth;
    }

    FillXLSet_Impl();
    m_aCtlPreview.Invalidate();

    // Make transparency accessible accordingly
    if( m_xLbLineStyle->get_active() == 0 ) // invisible
    {
        m_xBoxTransparency->set_sensitive(false);
    }
    else
    {
        m_xBoxTransparency->set_sensitive(true);
    }

    const bool bHasLineStyle = m_xLbLineStyle->get_active() !=0;
    const bool bHasLineStart = m_xLbStartStyle->get_active() != 0;

    m_xBoxStart->set_sensitive(bHasLineStart && bHasLineStyle);

    const bool bHasLineEnd = m_xLbEndStyle->get_active() != 0;

    m_xBoxEnd->set_sensitive(bHasLineEnd && bHasLineStyle);
}

IMPL_LINK_NOARG(SvxLineTabPage, ChangeStartClickHdl_Impl, weld::Button&, void)
{
    if (m_xCbxSynchronize->get_active())
        m_xTsbCenterEnd->set_state(m_xTsbCenterStart->get_state());
    ChangePreviewHdl_Impl(nullptr);
}

IMPL_LINK_NOARG(SvxLineTabPage, ChangeStartListBoxHdl_Impl, weld::ComboBox&, void)
{
    if (m_xCbxSynchronize->get_active())
        m_xLbEndStyle->set_active(m_xLbStartStyle->get_active());

    ChangePreviewHdl_Impl(nullptr);
}

IMPL_LINK_NOARG(SvxLineTabPage, ChangeStartModifyHdl_Impl, weld::MetricSpinButton&, void)
{
    if (m_xCbxSynchronize->get_active())
        m_xMtrEndWidth->set_value(m_xMtrStartWidth->get_value(FieldUnit::NONE), FieldUnit::NONE);

    ChangePreviewHdl_Impl(nullptr);
}

IMPL_LINK_NOARG(SvxLineTabPage, ChangeEdgeStyleHdl_Impl, weld::ComboBox&, void)
{
    ChangePreviewHdl_Impl( nullptr );
}

// fdo#43209
IMPL_LINK_NOARG(SvxLineTabPage, ChangeCapStyleHdl_Impl, weld::ComboBox&, void)
{
    ChangePreviewHdl_Impl( nullptr );
}

IMPL_LINK_NOARG(SvxLineTabPage, ClickInvisibleHdl_Impl, weld::ComboBox&, void)
{
    ClickInvisibleHdl_Impl();
}

void SvxLineTabPage::ClickInvisibleHdl_Impl()
{
    if( m_xLbLineStyle->get_active() == 0 ) // invisible
    {
        if(!m_bSymbols)
            m_xBoxColor->set_sensitive(false);

        m_xBoxWidth->set_sensitive(false);

        if( m_xFlLineEnds->get_sensitive() )
        {
            m_xBoxStart->set_sensitive(false);
            m_xBoxArrowStyles->set_sensitive(false);
            m_xGridEdgeCaps->set_sensitive(false);
        }
    }
    else
    {
        m_xBoxColor->set_sensitive(true);
        m_xBoxWidth->set_sensitive(true);

        if (m_xFlLineEnds->get_sensitive())
        {
            m_xBoxArrowStyles->set_sensitive(true);
            m_xGridEdgeCaps->set_sensitive(true);
        }
    }
    ChangePreviewHdl_Impl( nullptr );
}

IMPL_LINK_NOARG(SvxLineTabPage, ChangeEndClickHdl_Impl, weld::Button&, void)
{
    if (m_xCbxSynchronize->get_active())
        m_xTsbCenterStart->set_state(m_xTsbCenterEnd->get_state());

    ChangePreviewHdl_Impl(nullptr);
}

IMPL_LINK_NOARG(SvxLineTabPage, ChangeEndListBoxHdl_Impl, weld::ComboBox&, void)
{
    if (m_xCbxSynchronize->get_active())
        m_xLbStartStyle->set_active(m_xLbEndStyle->get_active());

    ChangePreviewHdl_Impl(nullptr);
}

IMPL_LINK_NOARG(SvxLineTabPage, ChangeEndModifyHdl_Impl, weld::MetricSpinButton&, void)
{
    if (m_xCbxSynchronize->get_active())
        m_xMtrStartWidth->set_value(m_xMtrEndWidth->get_value(FieldUnit::NONE), FieldUnit::NONE);

    ChangePreviewHdl_Impl(nullptr);
}

IMPL_LINK_NOARG(SvxLineTabPage, ChangeTransparentHdl_Impl, weld::MetricSpinButton&, void)
{
    sal_uInt16 nVal = m_xMtrTransparent->get_value(FieldUnit::PERCENT);

    m_rXLSet.Put(XLineTransparenceItem(nVal));

    FillXLSet_Impl();

    m_aCtlPreview.Invalidate();
}

void SvxLineTabPage::FillUserData()
{
    // Write the synched value to the INI file
    OUString aStrUserData = OUString::boolean(m_xCbxSynchronize->get_active());
    SetUserData( aStrUserData );
}

// #58425# Symbols on a list (e.g. StarChart)
// Handler for the symbol selection's popup menu (NumMenueButton)
// The following link originates from SvxNumOptionsTabPage
IMPL_LINK_NOARG(SvxLineTabPage, MenuCreateHdl_Impl, weld::ToggleButton&, void)
{
    ScopedVclPtrInstance< VirtualDevice > pVD;

    // Initialize popup
    if (!m_xGalleryMenu)
    {
        m_xGalleryMenu = m_xBuilder->weld_menu("gallerysubmenu");
        weld::WaitObject aWait(GetDialogFrameWeld());
        // Get gallery entries
        GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, m_aGrfNames);

        sal_uInt32 i = 0;
        for (auto const& grfName : m_aGrfNames)
        {
            const OUString *pUIName = &grfName;

            // Convert URL encodings to UI characters (e.g. %20 for spaces)
            OUString aPhysicalName;
            if (osl::FileBase::getSystemPathFromFileURL(grfName, aPhysicalName)
                == osl::FileBase::E_None)
            {
                pUIName = &aPhysicalName;
            }

            SvxBmpItemInfo* pInfo = new SvxBmpItemInfo;
            pInfo->pBrushItem.reset(new SvxBrushItem(grfName, "", GPOS_AREA, SID_ATTR_BRUSH));
            pInfo->sItemId = "gallery" + OUString::number(i);
            m_aGalleryBrushItems.emplace_back(pInfo);
            const Graphic* pGraphic = pInfo->pBrushItem->GetGraphic();

            if(pGraphic)
            {
                BitmapEx aBitmap(pGraphic->GetBitmapEx());
                Size aSize(aBitmap.GetSizePixel());
                if(aSize.Width()  > MAX_BMP_WIDTH || aSize.Height() > MAX_BMP_HEIGHT)
                {
                    bool bWidth = aSize.Width() > aSize.Height();
                    double nScale = bWidth ?
                                        double(MAX_BMP_WIDTH) / static_cast<double>(aSize.Width()):
                                        double(MAX_BMP_HEIGHT) / static_cast<double>(aSize.Height());
                    aBitmap.Scale(nScale, nScale);

                }
                pVD->SetOutputSizePixel(aBitmap.GetSizePixel());
                pVD->DrawBitmapEx(Point(), aBitmap);
                m_xGalleryMenu->append(pInfo->sItemId, *pUIName, *pVD);
            }
            else
            {
                m_xGalleryMenu->append(pInfo->sItemId, *pUIName);
            }
            ++i;
        }

        if (m_aGrfNames.empty())
            m_xSymbolMB->set_item_sensitive("gallery", false);
    }

    if (!m_xSymbolsMenu && m_pSymbolList)
    {
        m_xSymbolsMenu = m_xBuilder->weld_menu("symbolssubmenu");
        ScopedVclPtrInstance< VirtualDevice > pVDev;
        pVDev->SetMapMode(MapMode(MapUnit::Map100thMM));
        std::unique_ptr<SdrModel> pModel(
            new SdrModel(nullptr, nullptr, true));
        pModel->GetItemPool().FreezeIdRanges();
        // Page
        SdrPage* pPage = new SdrPage( *pModel, false );
        pPage->SetSize(Size(1000,1000));
        pModel->InsertPage( pPage, 0 );
        {
            // 3D View
            std::unique_ptr<SdrView> pView(new SdrView( *pModel, pVDev ));
            pView->hideMarkHandles();
            pView->ShowSdrPage(pPage);

            // Generate invisible square to give all symbols a
            // bitmap size, which is independent from specific glyph
            SdrObject *pInvisibleSquare=m_pSymbolList->GetObj(0);

            // directly clone to target SdrModel
            pInvisibleSquare = pInvisibleSquare->CloneSdrObject(*pModel);

            pPage->NbcInsertObject(pInvisibleSquare);
            pInvisibleSquare->SetMergedItem(XFillTransparenceItem(100));
            pInvisibleSquare->SetMergedItem(XLineTransparenceItem(100));

            for(size_t i=0;; ++i)
            {
                SdrObject *pObj=m_pSymbolList->GetObj(i);
                if(pObj==nullptr)
                    break;

                // directly clone to target SdrModel
                pObj = pObj->CloneSdrObject(*pModel);

                m_aGrfNames.emplace_back("");
                pPage->NbcInsertObject(pObj);
                if(m_pSymbolAttr)
                {
                    pObj->SetMergedItemSet(*m_pSymbolAttr);
                }
                else
                {
                    pObj->SetMergedItemSet(m_rOutAttrs);
                }
                pView->MarkAll();
                BitmapEx aBitmapEx(pView->GetMarkedObjBitmapEx());
                GDIMetaFile aMeta(pView->GetMarkedObjMetaFile());
                pView->UnmarkAll();
                pObj=pPage->RemoveObject(1);
                SdrObject::Free(pObj);

                SvxBmpItemInfo* pInfo = new SvxBmpItemInfo;
                pInfo->pBrushItem.reset(new SvxBrushItem(Graphic(aMeta), GPOS_AREA, SID_ATTR_BRUSH));
                pInfo->sItemId = "symbol" + OUString::number(i);
                m_aSymbolBrushItems.emplace_back(pInfo);

                Size aSize(aBitmapEx.GetSizePixel());
                if(aSize.Width() > MAX_BMP_WIDTH || aSize.Height() > MAX_BMP_HEIGHT)
                {
                    bool bWidth = aSize.Width() > aSize.Height();
                    double nScale = bWidth ?
                                        double(MAX_BMP_WIDTH) / static_cast<double>(aSize.Width()):
                                        double(MAX_BMP_HEIGHT) / static_cast<double>(aSize.Height());
                    aBitmapEx.Scale(nScale, nScale);
                }
                pVD->SetOutputSizePixel(aBitmapEx.GetSizePixel());
                pVD->DrawBitmapEx(Point(), aBitmapEx);
                m_xSymbolsMenu->append(pInfo->sItemId, "", *pVD);
            }
            pInvisibleSquare=pPage->RemoveObject(0);
            SdrObject::Free(pInvisibleSquare);

            if (m_aGrfNames.empty())
                m_xSymbolMB->set_item_sensitive("symbols", false);
        }
    }
}

// #58425# Symbols on a list (e.g. StarChart)
// Handler for menu button
IMPL_LINK(SvxLineTabPage, GraphicHdl_Impl, const OString&, rIdent, void)
{
    const Graphic* pGraphic = nullptr;
    Graphic aGraphic;
    bool bResetSize = false;
    bool bEnable = true;
    long nPreviousSymbolType = m_nSymbolType;

    OString sNumber;
    if (rIdent.startsWith("gallery", &sNumber))
    {
        SvxBmpItemInfo* pInfo = m_aGalleryBrushItems[sNumber.toUInt32()].get();
        pGraphic = pInfo->pBrushItem->GetGraphic();
        m_nSymbolType = SVX_SYMBOLTYPE_BRUSHITEM;
    }
    else if (rIdent.startsWith("symbol", &sNumber))
    {
        m_nSymbolType = sNumber.toUInt32();
        SvxBmpItemInfo* pInfo = m_aSymbolBrushItems[m_nSymbolType].get();
        pGraphic = pInfo->pBrushItem->GetGraphic();
    }
    else if (rIdent == "automatic")
    {
        pGraphic=&m_aAutoSymbolGraphic;
        m_aAutoSymbolGraphic.SetPrefSize( Size(253,253) );
        m_nSymbolType=SVX_SYMBOLTYPE_AUTO;
    }
    else if (rIdent == "none")
    {
        m_nSymbolType=SVX_SYMBOLTYPE_NONE;
        pGraphic=nullptr;
        bEnable = false;
    }
    else if (rIdent == "file")
    {
        SvxOpenGraphicDialog aGrfDlg(CuiResId(RID_SVXSTR_EDIT_GRAPHIC), GetDialogFrameWeld());
        aGrfDlg.EnableLink(false);
        aGrfDlg.AsLink(false);
        if( !aGrfDlg.Execute() )
        {
            // Remember selected filters
            if( !aGrfDlg.GetGraphic(aGraphic) )
            {
                m_nSymbolType=SVX_SYMBOLTYPE_BRUSHITEM;
                pGraphic = &aGraphic;
                bResetSize = true;
            }
        }
        if( !pGraphic )
            return;
    }

    if (pGraphic)
    {
        Size aSize = SvxNumberFormat::GetGraphicSizeMM100(pGraphic);
        aSize = OutputDevice::LogicToLogic(aSize, MapMode(MapUnit::Map100thMM), MapMode(m_ePoolUnit));
        m_aSymbolGraphic=*pGraphic;
        if( bResetSize )
        {
            m_aSymbolSize=aSize;
        }
        else if( nPreviousSymbolType == SVX_SYMBOLTYPE_BRUSHITEM )
        {   //#i31097# Data Point Symbol size changes when a different symbol is chosen(maoyg)
            if( m_aSymbolSize.Width() != m_aSymbolSize.Height() )
            {
                aSize.setWidth( static_cast<long>( m_aSymbolSize.Width() + m_aSymbolSize.Height() )/2 );
                aSize.setHeight( static_cast<long>( m_aSymbolSize.Width() + m_aSymbolSize.Height() )/2 );
                m_aSymbolSize = aSize;
            }
        }
        m_aCtlPreview.SetSymbol(&m_aSymbolGraphic,m_aSymbolSize);
    }
    else
    {
        m_aSymbolGraphic=Graphic();
        m_aCtlPreview.SetSymbol(nullptr,m_aSymbolSize);
        bEnable = false;
    }
    m_aSymbolLastSize=m_aSymbolSize;
    SetMetricValue(*m_xSymbolWidthMF,  m_aSymbolSize.Width(), m_ePoolUnit);
    SetMetricValue(*m_xSymbolHeightMF, m_aSymbolSize.Height(), m_ePoolUnit);

    m_xGridIconSize->set_sensitive(bEnable);
    m_aCtlPreview.Invalidate();
}

IMPL_LINK( SvxLineTabPage, SizeHdl_Impl, weld::MetricSpinButton&, rField, void)
{
    m_bNewSize = true;
    bool bWidth = &rField == m_xSymbolWidthMF.get();
    m_bLastWidthModified = bWidth;
    bool bRatio = m_xSymbolRatioCB->get_active();
    long nWidthVal = static_cast<long>(m_xSymbolWidthMF->denormalize(m_xSymbolWidthMF->get_value(FieldUnit::MM_100TH)));
    long nHeightVal= static_cast<long>(m_xSymbolHeightMF->denormalize(m_xSymbolHeightMF->get_value(FieldUnit::MM_100TH)));
    nWidthVal = OutputDevice::LogicToLogic(nWidthVal,MapUnit::Map100thMM, m_ePoolUnit );
    nHeightVal = OutputDevice::LogicToLogic(nHeightVal,MapUnit::Map100thMM, m_ePoolUnit);
    m_aSymbolSize = Size(nWidthVal,nHeightVal);
    double fSizeRatio = double(1);

    if(bRatio)
    {
        if (m_aSymbolLastSize.Height() && m_aSymbolLastSize.Width())
            fSizeRatio = static_cast<double>(m_aSymbolLastSize.Width()) / m_aSymbolLastSize.Height();
    }

    if (bWidth)
    {
        long nDelta = nWidthVal - m_aSymbolLastSize.Width();
        m_aSymbolSize.setWidth( nWidthVal );
        if (bRatio)
        {
            m_aSymbolSize.setHeight( m_aSymbolLastSize.Height() + static_cast<long>(static_cast<double>(nDelta) / fSizeRatio) );
            m_aSymbolSize.setHeight( OutputDevice::LogicToLogic( m_aSymbolSize.Height(), m_ePoolUnit, MapUnit::Map100thMM ) );
//TODO            m_xSymbolHeightMF->SetUserValue(m_xSymbolHeightMF->normalize(m_aSymbolSize.Height()), FieldUnit::MM_100TH);
            m_xSymbolHeightMF->set_value(m_xSymbolHeightMF->normalize(m_aSymbolSize.Height()), FieldUnit::MM_100TH);
        }
    }
    else
    {
        long nDelta = nHeightVal - m_aSymbolLastSize.Height();
        m_aSymbolSize.setHeight( nHeightVal );
        if (bRatio)
        {
            m_aSymbolSize.setWidth( m_aSymbolLastSize.Width() + static_cast<long>(static_cast<double>(nDelta) * fSizeRatio) );
            m_aSymbolSize.setWidth( OutputDevice::LogicToLogic( m_aSymbolSize.Width(), m_ePoolUnit, MapUnit::Map100thMM ) );
//TODO            m_xSymbolWidthMF->SetUserValue(m_xSymbolWidthMF->normalize(m_aSymbolSize.Width()), FieldUnit::MM_100TH);
            m_xSymbolWidthMF->set_value(m_xSymbolWidthMF->normalize(m_aSymbolSize.Width()), FieldUnit::MM_100TH);
        }
    }
    m_aCtlPreview.ResizeSymbol(m_aSymbolSize);
    m_aSymbolLastSize=m_aSymbolSize;
}

IMPL_LINK(SvxLineTabPage, RatioHdl_Impl, weld::ToggleButton&, rBox, void)
{
    if (rBox.get_active())
    {
        if (m_bLastWidthModified)
            SizeHdl_Impl(*m_xSymbolWidthMF);
        else
            SizeHdl_Impl(*m_xSymbolHeightMF);
    }
}

void SvxLineTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        FillListboxes();
    }
}

void SvxLineTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SvxDashListItem* pDashListItem = aSet.GetItem<SvxDashListItem>(SID_DASH_LIST, false);
    const SvxLineEndListItem* pLineEndListItem = aSet.GetItem<SvxLineEndListItem>(SID_LINEEND_LIST, false);
    const SfxUInt16Item* pPageTypeItem = aSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    const SfxUInt16Item* pDlgTypeItem = aSet.GetItem<SfxUInt16Item>(SID_DLG_TYPE, false);
    const OfaPtrItem* pSdrObjListItem = aSet.GetItem<OfaPtrItem>(SID_OBJECT_LIST, false);
    const SfxTabDialogItem* pSymbolAttrItem = aSet.GetItem<SfxTabDialogItem>(SID_ATTR_SET, false);
    const SvxGraphicItem* pGraphicItem = aSet.GetItem<SvxGraphicItem>(SID_GRAPHIC, false);

    if (pDashListItem)
        SetDashList(pDashListItem->GetDashList());
    if (pLineEndListItem)
        SetLineEndList(pLineEndListItem->GetLineEndList());
    if (pPageTypeItem)
        SetPageType(static_cast<PageType>(pPageTypeItem->GetValue()));
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    Construct();

    if(pSdrObjListItem) //symbols
    {
        ShowSymbolControls(true);
        m_pSymbolList = static_cast<SdrObjList*>(pSdrObjListItem->GetValue());
        if (pSymbolAttrItem)
            m_pSymbolAttr = new SfxItemSet(pSymbolAttrItem->GetItemSet());
        if(pGraphicItem)
            m_aAutoSymbolGraphic = pGraphicItem->GetGraphic();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
