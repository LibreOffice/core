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


#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/BorderLine2.hpp>

#include <comphelper/string.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <svl/style.hxx>
#include <svl/itemset.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include "sdr/properties/textproperties.hxx"
#include "editeng/outlobj.hxx"
#include "editeng/writingmodeitem.hxx"
#include "svx/svdotable.hxx"
#include "svx/svdoutl.hxx"
#include "svx/unoshtxt.hxx"
#include "svx/svdmodel.hxx"

#include "getallcharpropids.hxx"
#include "tableundo.hxx"
#include "cell.hxx"
#include "svx/unoshprp.hxx"
#include "svx/unoshape.hxx"
#include "editeng/editobj.hxx"
#include "editeng/boxitem.hxx"
#include "svx/xflbstit.hxx"
#include "svx/xflbmtit.hxx"
#include <svx/svdpool.hxx>


using ::editeng::SvxBorderLine;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;


static const SvxItemPropertySet* ImplGetSvxCellPropertySet()
{
    // Propertymap fuer einen Outliner Text
    static const SfxItemPropertyMapEntry aSvxCellPropertyMap[] =
    {
        FILL_PROPERTIES
//      { "HasLevels",                    OWN_ATTR_HASLEVELS,             cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY,      0},
        { OUString("Style"),                        OWN_ATTR_STYLE,                 cppu::UnoType< css::style::XStyle >::get(),                                    css::beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString(UNO_NAME_TEXT_WRITINGMODE),      SDRATTR_TEXTDIRECTION,          cppu::UnoType<css::text::WritingMode>::get(),                         0,      0},
        { OUString(UNO_NAME_TEXT_HORZADJUST),       SDRATTR_TEXT_HORZADJUST,        cppu::UnoType<css::drawing::TextHorizontalAdjust>::get(),  0,      0}, \
        { OUString(UNO_NAME_TEXT_LEFTDIST),         SDRATTR_TEXT_LEFTDIST,          cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
        { OUString(UNO_NAME_TEXT_LOWERDIST),        SDRATTR_TEXT_LOWERDIST,         cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
        { OUString(UNO_NAME_TEXT_RIGHTDIST),        SDRATTR_TEXT_RIGHTDIST,         cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
        { OUString(UNO_NAME_TEXT_UPPERDIST),        SDRATTR_TEXT_UPPERDIST,         cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
        { OUString(UNO_NAME_TEXT_VERTADJUST),       SDRATTR_TEXT_VERTADJUST,        cppu::UnoType<css::drawing::TextVerticalAdjust>::get(),    0,      0},\
        { OUString(UNO_NAME_TEXT_WORDWRAP),         SDRATTR_TEXT_WORDWRAP,          cppu::UnoType<bool>::get(),        0,      0}, \

        { OUString("TableBorder"),                  OWN_ATTR_TABLEBORDER,           cppu::UnoType<TableBorder>::get(), 0, 0 }, \
        { OUString("TopBorder"),                    SDRATTR_TABLE_BORDER,           cppu::UnoType<BorderLine>::get(), 0, TOP_BORDER }, \
        { OUString("BottomBorder"),                 SDRATTR_TABLE_BORDER,           cppu::UnoType<BorderLine>::get(), 0, BOTTOM_BORDER }, \
        { OUString("LeftBorder"),                   SDRATTR_TABLE_BORDER,           cppu::UnoType<BorderLine>::get(), 0, LEFT_BORDER }, \
        { OUString("RightBorder"),                  SDRATTR_TABLE_BORDER,           cppu::UnoType<BorderLine>::get(), 0, RIGHT_BORDER }, \

        SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    static SvxItemPropertySet aSvxCellPropertySet( aSvxCellPropertyMap, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aSvxCellPropertySet;
}

namespace
{

class CellTextProvider : public svx::ITextProvider
{
public:
    explicit CellTextProvider(const sdr::table::CellRef& rCell);
    virtual ~CellTextProvider();

private:
    virtual sal_Int32 getTextCount() const override;
    virtual SdrText* getText(sal_Int32 nIndex) const override;

private:
    const sdr::table::CellRef m_xCell;
};

CellTextProvider::CellTextProvider(const sdr::table::CellRef& rCell)
    : m_xCell(rCell)
{
}

CellTextProvider::~CellTextProvider()
{
}

sal_Int32 CellTextProvider::getTextCount() const
{
    return 1;
}

SdrText* CellTextProvider::getText(sal_Int32 nIndex) const
{
    (void) nIndex;
    assert(nIndex == 0);
    return m_xCell.get();
}

}

namespace sdr
{
    namespace properties
    {
        class CellProperties : public TextProperties
        {
        protected:
            // create a new itemset
            SfxItemSet* CreateObjectSpecificItemSet(SfxItemPool& rPool) override;

            const svx::ITextProvider& getTextProvider() const override;

        public:
            // basic constructor
            CellProperties(SdrObject& rObj, sdr::table::Cell* pCell );

            // constructor for copying, but using new object
            CellProperties(const CellProperties& rProps, SdrObject& rObj, sdr::table::Cell* pCell);

            // destructor
            virtual ~CellProperties();

            // Clone() operator, normally just calls the local copy constructor
            BaseProperties& Clone(SdrObject& rObj) const override;

            void ForceDefaultAttributes() override;

            void ItemSetChanged(const SfxItemSet& rSet) override;

            void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem) override;

            void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr) override;

            sdr::table::CellRef mxCell;

        private:
            const CellTextProvider maTextProvider;
        };

        // create a new itemset
        SfxItemSet* CellProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return new SfxItemSet(rPool,

                // range from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range for SdrTableObj
                SDRATTR_TABLE_FIRST, SDRATTR_TABLE_LAST,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0);
        }

        const svx::ITextProvider& CellProperties::getTextProvider() const
        {
            return maTextProvider;
        }

        CellProperties::CellProperties(SdrObject& rObj, sdr::table::Cell* pCell)
        :   TextProperties(rObj)
        ,   mxCell(pCell)
        ,   maTextProvider(mxCell)
        {
        }

        CellProperties::CellProperties(const CellProperties& rProps, SdrObject& rObj, sdr::table::Cell* pCell)
        :   TextProperties(rProps, rObj)
        ,   mxCell( pCell )
        ,   maTextProvider(mxCell)
        {
        }

        CellProperties::~CellProperties()
        {
        }

        BaseProperties& CellProperties::Clone(SdrObject& rObj) const
        {
            OSL_FAIL("CellProperties::Clone(), does not work yet!");
            return *(new CellProperties(*this, rObj,nullptr));
        }

        void CellProperties::ForceDefaultAttributes()
        {
        }

        void CellProperties::ItemSetChanged(const SfxItemSet& rSet )
        {
            SdrTextObj& rObj = static_cast<SdrTextObj&>(GetSdrObject());

            if( mxCell.is() )
            {
                OutlinerParaObject* pParaObj = mxCell->GetEditOutlinerParaObject();

                bool bOwnParaObj = pParaObj != nullptr;

                if( pParaObj == nullptr )
                    pParaObj = mxCell->GetOutlinerParaObject();

                if(pParaObj)
                {
                    // handle outliner attributes
                    Outliner* pOutliner = nullptr;

                    if(mxCell->IsTextEditActive())
                    {
                        pOutliner = rObj.GetTextEditOutliner();
                    }
                    else
                    {
                        pOutliner = &rObj.ImpGetDrawOutliner();
                        pOutliner->SetText(*pParaObj);
                    }

                    sal_Int32 nParaCount(pOutliner->GetParagraphCount());

                    // if the user sets character attributes to the complete
                    // cell we want to remove all hard set character attributes
                    // with same which ids from the text
                    std::vector<sal_uInt16> aCharWhichIds(GetAllCharPropIds(rSet));

                    for(sal_Int32 nPara = 0; nPara < nParaCount; nPara++)
                    {
                        SfxItemSet aSet(pOutliner->GetParaAttribs(nPara));
                        aSet.Put(rSet);

                        for (std::vector<sal_uInt16>::const_iterator aI = aCharWhichIds.begin(); aI != aCharWhichIds.end(); ++aI)
                        {
                            pOutliner->RemoveCharAttribs(nPara, *aI);
                        }

                        pOutliner->SetParaAttribs(nPara, aSet);
                    }

                    if(!mxCell->IsTextEditActive())
                    {
                        if(nParaCount)
                        {
                            // force ItemSet
                            GetObjectItemSet();

                            SfxItemSet aNewSet(pOutliner->GetParaAttribs(0L));
                            mpItemSet->Put(aNewSet);
                        }

                        OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, nParaCount);
                        pOutliner->Clear();

                        mxCell->SetOutlinerParaObject(pTemp);
                    }

                    if( bOwnParaObj )
                        delete pParaObj;
                }
            }

            // call parent
            AttributeProperties::ItemSetChanged(rSet);

            if( mxCell.is() )
                mxCell->notifyModified();
        }

        void CellProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
        {
            if(pNewItem && (SDRATTR_TEXTDIRECTION == nWhich))
            {
                bool bVertical(css::text::WritingMode_TB_RL == static_cast<const SvxWritingModeItem*>(pNewItem)->GetValue());

                sdr::table::SdrTableObj& rObj = static_cast<sdr::table::SdrTableObj&>(GetSdrObject());
                if( rObj.IsVerticalWriting() != bVertical )
                    rObj.SetVerticalWriting(bVertical);

                // Set a cell vertical property
                OutlinerParaObject* pParaObj = mxCell->GetEditOutlinerParaObject();
                if( pParaObj == nullptr )
                    pParaObj = mxCell->GetOutlinerParaObject();
                if(pParaObj)
                {
                    pParaObj->SetVertical(bVertical);
                }

            }

            // call parent
            AttributeProperties::ItemChange( nWhich, pNewItem );
        }

        void CellProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            TextProperties::SetStyleSheet( pNewStyleSheet, bDontRemoveHardAttr );
        }
    } // end of namespace properties
} // end of namespace sdr

namespace sdr { namespace table {


// Cell


rtl::Reference< Cell > Cell::create( SdrTableObj& rTableObj, OutlinerParaObject* pOutlinerParaObject )
{
    rtl::Reference< Cell > xCell( new Cell( rTableObj, pOutlinerParaObject ) );
    if( xCell->mxTable.is() )
    {
        Reference< XEventListener > xListener( xCell.get() );
        xCell->mxTable->addEventListener( xListener );
    }
    return xCell;
}


Cell::Cell( SdrTableObj& rTableObj, OutlinerParaObject* pOutlinerParaObject ) throw(css::uno::RuntimeException)
: SdrText( rTableObj, pOutlinerParaObject )
, SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorSvxPropertySet() )
, mpPropSet( ImplGetSvxCellPropertySet() )
, mpProperties( new sdr::properties::CellProperties( rTableObj, this ) )
, mnCellContentType( CellContentType_EMPTY )
, mfValue( 0.0 )
, mnError( 0 )
, mbMerged( false )
, mnRowSpan( 1 )
, mnColSpan( 1 )
, mxTable( rTableObj.getTable() )
{
    if( rTableObj.GetModel() )
        SetModel( rTableObj.GetModel() );
}


Cell::~Cell() throw()
{
    dispose();
}


void Cell::dispose()
{
    if( mxTable.is() )
    {
        try
        {
            Reference< XEventListener > xThis( this );
            mxTable->removeEventListener( xThis );
        }
        catch( Exception& )
        {
            OSL_FAIL("Cell::dispose(), exception caught!");
        }
        mxTable.clear();
    }

    if( mpProperties )
    {
        delete mpProperties;
        mpProperties = nullptr;
    }
    SetOutlinerParaObject( nullptr );
}


void Cell::SetModel(SdrModel* pNewModel)
{
    SvxTextEditSource* pTextEditSource = dynamic_cast< SvxTextEditSource* >( GetEditSource() );
    if( (GetModel() != pNewModel) || ( pNewModel && !pTextEditSource) )
    {
        if( mpProperties )
        {
            SfxItemPool* pItemPool = mpProperties->GetObjectItemSet().GetPool();

            // test for correct pool in ItemSet; move to new pool if necessary
            if( pNewModel && pItemPool && pItemPool != &pNewModel->GetItemPool())
                mpProperties->MoveToItemPool(pItemPool, &pNewModel->GetItemPool(), pNewModel);
        }

        if( pTextEditSource )
        {
            pTextEditSource->ChangeModel( pNewModel );
        }
        else
        {
            SetEditSource( new SvxTextEditSource( &GetObject(), this ) );
        }

        SetStyleSheet( nullptr, true );
        SdrText::SetModel( pNewModel );
        ForceOutlinerParaObject( OutlinerMode::TextObject );
    }
}


void Cell::merge( sal_Int32 nColumnSpan, sal_Int32 nRowSpan )
{
    if ((mnColSpan != nColumnSpan) || (mnRowSpan != nRowSpan) || mbMerged)
    {
        mnColSpan = nColumnSpan;
        mnRowSpan = nRowSpan;
        mbMerged = false;
        notifyModified();
    }
}


void Cell::mergeContent( const CellRef& xSourceCell )
{
    SdrTableObj& rTableObj = dynamic_cast< SdrTableObj& >( GetObject() );

    if( xSourceCell->hasText() )
    {
        SdrOutliner& rOutliner=rTableObj.ImpGetDrawOutliner();
        rOutliner.SetUpdateMode(true);

        if( hasText() )
        {
            rOutliner.SetText(*GetOutlinerParaObject());
            rOutliner.AddText(*xSourceCell->GetOutlinerParaObject());
        }
        else
        {
            rOutliner.SetText(*xSourceCell->GetOutlinerParaObject());
        }

        SetOutlinerParaObject( rOutliner.CreateParaObject() );
        rOutliner.Clear();
        xSourceCell->SetOutlinerParaObject(rOutliner.CreateParaObject());
        rOutliner.Clear();
        SetStyleSheet( GetStyleSheet(), true );
    }
}


void Cell::cloneFrom( const CellRef& xCell )
{
    if( xCell.is() )
    {
        replaceContentAndFormating( xCell );

        mnCellContentType = xCell->mnCellContentType;

        msFormula = xCell->msFormula;
        mfValue = xCell->mfValue;
        mnError = xCell->mnError;

        mbMerged = xCell->mbMerged;
        mnRowSpan = xCell->mnRowSpan;
        mnColSpan = xCell->mnColSpan;

    }
    notifyModified();
}

void Cell::replaceContentAndFormating( const CellRef& xSourceCell )
{
    if( xSourceCell.is() && mpProperties )
    {
        mpProperties->SetMergedItemSet( xSourceCell->GetObjectItemSet() );
        SetOutlinerParaObject( new OutlinerParaObject(*xSourceCell->GetOutlinerParaObject()) );

        SdrTableObj& rTableObj = dynamic_cast< SdrTableObj& >( GetObject() );
        SdrTableObj& rSourceTableObj = dynamic_cast< SdrTableObj& >( xSourceCell->GetObject() );

        if(rSourceTableObj.GetModel() != rTableObj.GetModel())
        {
            SetStyleSheet( nullptr, true );
        }
    }
}


void Cell::setMerged()
{
    if( !mbMerged )
    {
        mbMerged = true;
        notifyModified();
    }
}


void Cell::copyFormatFrom( const CellRef& xSourceCell )
{
    if( xSourceCell.is() && mpProperties )
    {
        mpProperties->SetMergedItemSet( xSourceCell->GetObjectItemSet() );

        SdrTableObj& rTableObj = dynamic_cast< SdrTableObj& >( GetObject() );
        SdrTableObj& rSourceTableObj = dynamic_cast< SdrTableObj& >( xSourceCell->GetObject() );

        if(rSourceTableObj.GetModel() != rTableObj.GetModel())
        {
            SetStyleSheet( nullptr, true );
        }

        notifyModified();
    }
}


void Cell::notifyModified()
{
    if( mxTable.is() )
        mxTable->setModified( true );
}


// SdrTextShape proxy


bool Cell::IsActiveCell()
{
    bool isActive = false;
    SdrTableObj& rTableObj = dynamic_cast< SdrTableObj& >( GetObject() );
    if( rTableObj.getActiveCell().get() == this )
        isActive = true;

    return isActive;
}

bool Cell::IsTextEditActive()
{
    bool isActive = false;
    SdrTableObj& rTableObj = dynamic_cast< SdrTableObj& >( GetObject() );
    if(rTableObj.getActiveCell().get() == this )
    {
        OutlinerParaObject* pParaObj = rTableObj.GetEditOutlinerParaObject();
        if( pParaObj != nullptr )
        {
            isActive = true;
            delete pParaObj;
        }
    }
    return isActive;
}


bool Cell::hasText() const
{
    OutlinerParaObject* pParaObj = GetOutlinerParaObject();
    if( pParaObj )
    {
        const EditTextObject& rTextObj = pParaObj->GetTextObject();
        if( rTextObj.GetParagraphCount() >= 1 )
        {
            if( rTextObj.GetParagraphCount() == 1 )
            {
                if( rTextObj.GetText(0).isEmpty() )
                    return false;
            }
            return true;
        }
    }

    return false;
}


OutlinerParaObject* Cell::GetEditOutlinerParaObject() const
{
    SdrTableObj& rTableObj = dynamic_cast< SdrTableObj& >( GetObject() );
    if( rTableObj.getActiveCell().get() == this )
        return rTableObj.GetEditOutlinerParaObject();
    return nullptr;
}


void Cell::SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr )
{
    // only allow cell styles for cells
    if( pStyleSheet && pStyleSheet->GetFamily() != SfxStyleFamily::Frame )
        return;

    if( mpProperties && (mpProperties->GetStyleSheet() != pStyleSheet) )
    {
        mpProperties->SetStyleSheet( pStyleSheet, bDontRemoveHardAttr );
    }
}


const SfxItemSet& Cell::GetObjectItemSet()
{
    if( mpProperties )
    {
        return mpProperties->GetObjectItemSet();
    }
    else
    {
        OSL_FAIL("Cell::GetObjectItemSet(), called without properties!");
        return GetObject().GetObjectItemSet();
    }
}

void Cell::SetObjectItem(const SfxPoolItem& rItem)
{
    if( mpProperties )
    {
        mpProperties->SetObjectItem( rItem );
        notifyModified();
    }
}

void Cell::SetMergedItem(const SfxPoolItem& rItem)
{
    SetObjectItem(rItem);
}

SfxStyleSheet* Cell::GetStyleSheet() const
{
    if( mpProperties )
        return mpProperties->GetStyleSheet();
    else
        return nullptr;
}

void Cell::TakeTextAnchorRect(Rectangle& rAnchorRect) const
{
    rAnchorRect.Left() = maCellRect.Left() + GetTextLeftDistance();
    rAnchorRect.Right() = maCellRect.Right() - GetTextRightDistance();
    rAnchorRect.Top() = maCellRect.Top() + GetTextUpperDistance();
    rAnchorRect.Bottom() = maCellRect.Bottom() - GetTextLowerDistance();
}


const SfxItemSet& Cell::GetItemSet() const
{
    return mpProperties->GetObjectItemSet();
}


void Cell::SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, bool bClearAllItems)
{
    if( mpProperties )
    {
        mpProperties->SetMergedItemSetAndBroadcast(rSet, bClearAllItems);
        notifyModified();
    }
}


sal_Int32 Cell::getMinimumWidth()
{
    return GetTextLeftDistance() + GetTextRightDistance() + 100;
}


sal_Int32 Cell::getMinimumHeight()
{
    if( !mpProperties )
        return 0;

    SdrTableObj& rTableObj = dynamic_cast< SdrTableObj& >( GetObject() );
    sal_Int32 nMinimumHeight = 0;

    Rectangle aTextRect;
    TakeTextAnchorRect( aTextRect );
    Size aSize( aTextRect.GetSize() );
    aSize.Height()=0x0FFFFFFF;

    SdrOutliner* pEditOutliner = rTableObj.GetCellTextEditOutliner( *this );
    if(pEditOutliner)
    {
        pEditOutliner->SetMaxAutoPaperSize(aSize);
        nMinimumHeight = pEditOutliner->GetTextHeight()+1;
    }
    else
    {
        Outliner& rOutliner=rTableObj.ImpGetDrawOutliner();
        rOutliner.SetPaperSize(aSize);
        rOutliner.SetUpdateMode(true);
        ForceOutlinerParaObject( OutlinerMode::TextObject );

        if( GetOutlinerParaObject() )
        {
            rOutliner.SetText(*GetOutlinerParaObject());
        }
        nMinimumHeight=rOutliner.GetTextHeight()+1;
        rOutliner.Clear();
    }

    nMinimumHeight += GetTextUpperDistance() + GetTextLowerDistance();
    return nMinimumHeight;
}


long Cell::GetTextLeftDistance() const
{
    return static_cast<const SdrMetricItem&>(GetItemSet().Get(SDRATTR_TEXT_LEFTDIST)).GetValue();
}


long Cell::GetTextRightDistance() const
{
    return static_cast<const SdrMetricItem&>(GetItemSet().Get(SDRATTR_TEXT_RIGHTDIST)).GetValue();
}


long Cell::GetTextUpperDistance() const
{
    return static_cast<const SdrMetricItem&>(GetItemSet().Get(SDRATTR_TEXT_UPPERDIST)).GetValue();
}


long Cell::GetTextLowerDistance() const
{
    return static_cast<const SdrMetricItem&>(GetItemSet().Get(SDRATTR_TEXT_LOWERDIST)).GetValue();
}


SdrTextVertAdjust Cell::GetTextVerticalAdjust() const
{
    return static_cast<const SdrTextVertAdjustItem&>(GetItemSet().Get(SDRATTR_TEXT_VERTADJUST)).GetValue();
}


SdrTextHorzAdjust Cell::GetTextHorizontalAdjust() const
{
    return static_cast<const SdrTextHorzAdjustItem&>(GetItemSet().Get(SDRATTR_TEXT_HORZADJUST)).GetValue();
}


void Cell::SetOutlinerParaObject( OutlinerParaObject* pTextObject )
{
    SdrText::SetOutlinerParaObject( pTextObject );
    maSelection.nStartPara = EE_PARA_MAX_COUNT;

    if( pTextObject == nullptr )
        ForceOutlinerParaObject( OutlinerMode::TextObject );
}


void Cell::AddUndo()
{
    SdrObject& rObj = GetObject();
    if( rObj.IsInserted() && GetModel() && GetModel()->IsUndoEnabled() )
    {
        CellRef xCell( this );
        GetModel()->AddUndo( new CellUndo( &rObj, xCell ) );

        // Undo action for the after-text-edit-ended stack.
        SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(&rObj);
        if (pTableObj && pTableObj->IsTextEditActive())
            pTableObj->AddUndo(new CellUndo(pTableObj, xCell));
    }
}


sdr::properties::TextProperties* Cell::CloneProperties( sdr::properties::TextProperties* pProperties, SdrObject& rNewObj, Cell& rNewCell )
{
    if( pProperties )
        return new sdr::properties::CellProperties( *static_cast<sdr::properties::CellProperties*>(pProperties), rNewObj, &rNewCell );
    else
        return nullptr;
}


sdr::properties::TextProperties* Cell::CloneProperties( SdrObject& rNewObj, Cell& rNewCell )
{
    return CloneProperties(mpProperties,rNewObj,rNewCell);
}


// XInterface


Any SAL_CALL Cell::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    if( rType == cppu::UnoType<XMergeableCell>::get() )
        return Any( Reference< XMergeableCell >( this ) );

    if( rType == cppu::UnoType<XCell>::get() )
        return Any( Reference< XCell >( this ) );

    if( rType == cppu::UnoType<XLayoutConstrains>::get() )
        return Any( Reference< XLayoutConstrains >( this ) );

    if( rType == cppu::UnoType<XEventListener>::get() )
        return Any( Reference< XEventListener >( this ) );

    Any aRet( SvxUnoTextBase::queryAggregation( rType ) );
    if( aRet.hasValue() )
        return aRet;

    return ::cppu::OWeakObject::queryInterface( rType );
}


void SAL_CALL Cell::acquire() throw ()
{
    ::cppu::OWeakObject::acquire();
}


void SAL_CALL Cell::release() throw ()
{
    ::cppu::OWeakObject::release();
}


// XTypeProvider


Sequence< Type > SAL_CALL Cell::getTypes(  ) throw (RuntimeException, std::exception)
{
    Sequence< Type > aTypes( SvxUnoTextBase::getTypes() );

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 2);
    aTypes[nLen++] = cppu::UnoType<XMergeableCell>::get();
    aTypes[nLen++] = cppu::UnoType<XLayoutConstrains>::get();

    return aTypes;
}


Sequence< sal_Int8 > SAL_CALL Cell::getImplementationId(  ) throw (RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XLayoutConstrains
css::awt::Size SAL_CALL Cell::getMinimumSize()
    throw (RuntimeException,
           std::exception)
{
    return css::awt::Size( getMinimumWidth(),  getMinimumHeight() );
}


css::awt::Size SAL_CALL Cell::getPreferredSize()
    throw (RuntimeException,
           std::exception)
{
    return getMinimumSize();
}


css::awt::Size SAL_CALL Cell::calcAdjustedSize( const css::awt::Size& aNewSize ) throw (RuntimeException, std::exception)
{
    return aNewSize;
}


// XMergeableCell


sal_Int32 SAL_CALL Cell::getRowSpan() throw (RuntimeException, std::exception)
{
    return mnRowSpan;
}


sal_Int32 SAL_CALL Cell::getColumnSpan() throw (RuntimeException, std::exception)
{
    return mnColSpan;
}


sal_Bool SAL_CALL Cell::isMerged() throw (RuntimeException, std::exception)
{
    return mbMerged;
}


// XCell


OUString SAL_CALL Cell::getFormula(  ) throw (RuntimeException, std::exception)
{
    return msFormula;
}


void SAL_CALL Cell::setFormula( const OUString& aFormula ) throw (RuntimeException, std::exception)
{
    if( msFormula != aFormula )
    {
        msFormula = aFormula;
    }
}


double SAL_CALL Cell::getValue(  ) throw (RuntimeException, std::exception)
{
    return mfValue;
}


void SAL_CALL Cell::setValue( double nValue ) throw (RuntimeException, std::exception)
{
    if( mfValue != nValue )
    {
        mfValue = nValue;
        mnCellContentType = CellContentType_VALUE;
    }
}


CellContentType SAL_CALL Cell::getType() throw (RuntimeException, std::exception)
{
    return mnCellContentType;
}


sal_Int32 SAL_CALL Cell::getError(  ) throw (RuntimeException, std::exception)
{
    return mnError;
}


// XPropertySet


Any Cell::GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertySimpleEntry* pMap )
{
    Any aAny( SvxItemPropertySet_getPropertyValue( pMap, aSet ) );

    if( pMap->aType != aAny.getValueType() )
    {
        // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
        if( ( pMap->aType == ::cppu::UnoType<sal_Int16>::get()) && aAny.getValueType() == ::cppu::UnoType<sal_Int32>::get() )
        {
            sal_Int32 nValue = 0;
            aAny >>= nValue;
            aAny <<= (sal_Int16)nValue;
        }
        else
        {
            OSL_FAIL("GetAnyForItem() Returnvalue has wrong Type!" );
        }
    }

    return aAny;
}

Reference< XPropertySetInfo > SAL_CALL Cell::getPropertySetInfo() throw(RuntimeException, std::exception)
{
    return mpPropSet->getPropertySetInfo();
}


void SAL_CALL Cell::setPropertyValue( const OUString& rPropertyName, const Any& rValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( (mpProperties == nullptr) || (GetModel() == nullptr) )
        throw DisposedException();

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(rPropertyName);
    if( pMap )
    {
        if( (pMap->nFlags & PropertyAttribute::READONLY ) != 0 )
            throw PropertyVetoException();

        switch( pMap->nWID )
        {
        case OWN_ATTR_STYLE:
        {
            Reference< XStyle > xStyle;
            if( !( rValue >>= xStyle ) )
                throw IllegalArgumentException();

            SfxUnoStyleSheet* pStyle = SfxUnoStyleSheet::getUnoStyleSheet(xStyle);
            SetStyleSheet( pStyle, true );
            return;
        }
        case OWN_ATTR_TABLEBORDER:
        {
            if(rValue.getValueType() != cppu::UnoType<TableBorder>::get())
                break;

            const TableBorder* pBorder = static_cast<const TableBorder*>(rValue.getValue());
            if( pBorder == nullptr )
                break;

            SvxBoxItem aBox( SDRATTR_TABLE_BORDER );
            SvxBoxInfoItem aBoxInfo( SDRATTR_TABLE_BORDER_INNER );
            SvxBorderLine aLine;

            bool bSet = SvxBoxItem::LineToSvxLine(pBorder->TopLine, aLine, false);
            aBox.SetLine(bSet ? &aLine : nullptr, SvxBoxItemLine::TOP);
            aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::TOP, pBorder->IsTopLineValid);

            bSet = SvxBoxItem::LineToSvxLine(pBorder->BottomLine, aLine, false);
            aBox.SetLine(bSet ? &aLine : nullptr, SvxBoxItemLine::BOTTOM);
            aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::BOTTOM, pBorder->IsBottomLineValid);

            bSet = SvxBoxItem::LineToSvxLine(pBorder->LeftLine, aLine, false);
            aBox.SetLine(bSet ? &aLine : nullptr, SvxBoxItemLine::LEFT);
            aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::LEFT, pBorder->IsLeftLineValid);

            bSet = SvxBoxItem::LineToSvxLine(pBorder->RightLine, aLine, false);
            aBox.SetLine(bSet ? &aLine : nullptr, SvxBoxItemLine::RIGHT);
            aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::RIGHT, pBorder->IsRightLineValid);

            bSet = SvxBoxItem::LineToSvxLine(pBorder->HorizontalLine, aLine, false);
            aBoxInfo.SetLine(bSet ? &aLine : nullptr, SvxBoxInfoItemLine::HORI);
            aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI, pBorder->IsHorizontalLineValid);

            bSet = SvxBoxItem::LineToSvxLine(pBorder->VerticalLine, aLine, false);
            aBoxInfo.SetLine(bSet ? &aLine : nullptr, SvxBoxInfoItemLine::VERT);
            aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::VERT, pBorder->IsVerticalLineValid);

            aBox.SetDistance(pBorder->Distance); //TODO
            aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::DISTANCE, pBorder->IsDistanceValid);

            mpProperties->SetObjectItem(aBox);
            mpProperties->SetObjectItem(aBoxInfo);
            return;
        }
        case OWN_ATTR_FILLBMP_MODE:
        {
            BitmapMode eMode;
            if(!(rValue >>= eMode) )
            {
                sal_Int32 nMode = 0;
                if(!(rValue >>= nMode))
                    throw IllegalArgumentException();

                eMode = (BitmapMode)nMode;
            }

            mpProperties->SetObjectItem( XFillBmpStretchItem( eMode == BitmapMode_STRETCH ) );
            mpProperties->SetObjectItem( XFillBmpTileItem( eMode == BitmapMode_REPEAT ) );
            return;
        }
        default:
        {
            SfxItemSet aSet( GetModel()->GetItemPool(), pMap->nWID, pMap->nWID);
            aSet.Put(mpProperties->GetItem(pMap->nWID));

            bool bSpecial = false;

            switch( pMap->nWID )
            {
                case XATTR_FILLBITMAP:
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLFLOATTRANSPARENCE:
                case XATTR_LINEEND:
                case XATTR_LINESTART:
                case XATTR_LINEDASH:
                {
                    if( pMap->nMemberId == MID_NAME )
                    {
                        OUString aApiName;
                        if( rValue >>= aApiName )
                        {
                            if( SvxShape::SetFillAttribute( pMap->nWID, aApiName, aSet, GetModel() ) )
                                bSpecial = true;
                        }
                    }
                }
                break;
            }

            if( !bSpecial )
            {

                if( !SvxUnoTextRangeBase::SetPropertyValueHelper( aSet, pMap, rValue, aSet ))
                {
                    if( aSet.GetItemState( pMap->nWID ) != SfxItemState::SET )
                    {
                        // Default aus ItemPool holen
                        if(SfxItemPool::IsWhich(pMap->nWID))
                            aSet.Put(GetModel()->GetItemPool().GetDefaultItem(pMap->nWID));
                    }

                    if( aSet.GetItemState( pMap->nWID ) == SfxItemState::SET )
                    {
                        SvxItemPropertySet_setPropertyValue( pMap, rValue, aSet );
                    }
                }
            }

            GetModel()->SetChanged();
            mpProperties->SetMergedItemSetAndBroadcast( aSet );
            return;
        }
        }
    }
    throw UnknownPropertyException();
}


Any SAL_CALL Cell::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( (mpProperties == nullptr) || (GetModel() == nullptr) )
        throw DisposedException();

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(PropertyName);
    if( pMap )
    {
        switch( pMap->nWID )
        {
        case OWN_ATTR_STYLE:
        {
            return Any( Reference< XStyle >( dynamic_cast< SfxUnoStyleSheet* >( GetStyleSheet() ) ) );
        }
        case OWN_ATTR_TABLEBORDER:
        {
            const SvxBoxInfoItem& rBoxInfoItem = static_cast<const SvxBoxInfoItem&>(mpProperties->GetItem(SDRATTR_TABLE_BORDER_INNER));
            const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(mpProperties->GetItem(SDRATTR_TABLE_BORDER));

             TableBorder aTableBorder;
            aTableBorder.TopLine                = SvxBoxItem::SvxLineToLine(rBox.GetTop(), false);
            aTableBorder.IsTopLineValid         = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::TOP);
            aTableBorder.BottomLine             = SvxBoxItem::SvxLineToLine(rBox.GetBottom(), false);
            aTableBorder.IsBottomLineValid      = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::BOTTOM);
            aTableBorder.LeftLine               = SvxBoxItem::SvxLineToLine(rBox.GetLeft(), false);
            aTableBorder.IsLeftLineValid        = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::LEFT);
            aTableBorder.RightLine              = SvxBoxItem::SvxLineToLine(rBox.GetRight(), false);
            aTableBorder.IsRightLineValid       = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::RIGHT );
            aTableBorder.HorizontalLine         = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetHori(), false);
            aTableBorder.IsHorizontalLineValid  = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::HORI);
            aTableBorder.VerticalLine           = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetVert(), false);
            aTableBorder.IsVerticalLineValid    = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::VERT);
            aTableBorder.Distance               = rBox.GetDistance();
            aTableBorder.IsDistanceValid        = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::DISTANCE);

            return Any( aTableBorder );
        }
        case OWN_ATTR_FILLBMP_MODE:
        {
            const XFillBmpStretchItem& rStretchItem = static_cast<const XFillBmpStretchItem&>(mpProperties->GetItem(XATTR_FILLBMP_STRETCH));
            const XFillBmpTileItem& rTileItem = static_cast<const XFillBmpTileItem&>(mpProperties->GetItem(XATTR_FILLBMP_TILE));
            if( rTileItem.GetValue() )
            {
                return Any( BitmapMode_REPEAT );
            }
            else if( rStretchItem.GetValue() )
            {
                return Any(  BitmapMode_STRETCH );
            }
            else
            {
                return Any(  BitmapMode_NO_REPEAT );
            }
        }
        default:
        {
            SfxItemSet aSet( GetModel()->GetItemPool(), pMap->nWID, pMap->nWID);
            aSet.Put(mpProperties->GetItem(pMap->nWID));

            Any aAny;
            if(!SvxUnoTextRangeBase::GetPropertyValueHelper( aSet, pMap, aAny ))
            {
                if(!aSet.Count())
                {
                    // Default aus ItemPool holen
                    if(SfxItemPool::IsWhich(pMap->nWID))
                        aSet.Put(GetModel()->GetItemPool().GetDefaultItem(pMap->nWID));
                }

                if( aSet.Count() )
                    aAny = GetAnyForItem( aSet, pMap );
            }

            return aAny;
        }
        }
    }
    throw UnknownPropertyException();
}


void SAL_CALL Cell::addPropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*xListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}


void SAL_CALL Cell::removePropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*aListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}


void SAL_CALL Cell::addVetoableChangeListener( const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}


void SAL_CALL Cell::removeVetoableChangeListener( const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}


// XMultiPropertySet


void SAL_CALL Cell::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw (PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    ::SolarMutexGuard aSolarGuard;

    if( (mpProperties == nullptr) || (GetModel() == nullptr) )
        throw DisposedException();

    const sal_Int32 nCount = aPropertyNames.getLength();

    const OUString* pNames = aPropertyNames.getConstArray();
    const Any* pValues = aValues.getConstArray();

    for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pNames++, pValues++ )
    {
        try
        {
            setPropertyValue( *pNames, *pValues );
        }
        catch( UnknownPropertyException& )
        {
            OSL_FAIL("svx::Cell::setPropertyValues(), unknown property!" );
        }
        catch( Exception& )
        {
            OSL_FAIL("svx::Cell::setPropertyValues(), Exception caught!" );
        }
    }
}


Sequence< Any > SAL_CALL Cell::getPropertyValues( const Sequence< OUString >& aPropertyNames ) throw (RuntimeException, std::exception)
{
    ::SolarMutexGuard aSolarGuard;

    if( (mpProperties == nullptr) || (GetModel() == nullptr) )
        throw DisposedException();

    const sal_Int32 nCount = aPropertyNames.getLength();
    const OUString* pNames = aPropertyNames.getConstArray();

    Sequence< Any > aRet( nCount );
    Any* pValue = aRet.getArray();

    for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pValue++, pNames++ )
    {
        try
        {
            *pValue = getPropertyValue( *pNames );
        }
        catch( UnknownPropertyException& )
        {
            OSL_FAIL("svx::Cell::setPropertyValues(), unknown property!" );
        }
        catch( Exception& )
        {
            OSL_FAIL( "svx::Cell::getPropertyValues(), Exception caught!" );
        }
    }

    return aRet;
}


void SAL_CALL Cell::addPropertiesChangeListener( const Sequence< OUString >& /*aPropertyNames*/, const Reference< XPropertiesChangeListener >& /*xListener*/ ) throw (RuntimeException, std::exception)
{
}


void SAL_CALL Cell::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& /*xListener*/ ) throw (RuntimeException, std::exception)
{
}


void SAL_CALL Cell::firePropertiesChangeEvent( const Sequence< OUString >& /*aPropertyNames*/, const Reference< XPropertiesChangeListener >& /*xListener*/ ) throw (RuntimeException, std::exception)
{
}


// XPropertyState


PropertyState SAL_CALL Cell::getPropertyState( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( (mpProperties == nullptr) || (GetModel() == nullptr) )
        throw DisposedException();

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(PropertyName);

    if( pMap )
    {
        PropertyState eState;
        switch( pMap->nWID )
        {
        case OWN_ATTR_FILLBMP_MODE:
        {
            const SfxItemSet& rSet = mpProperties->GetMergedItemSet();

            const bool bStretch = rSet.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SfxItemState::SET;
            const bool bTile = rSet.GetItemState( XATTR_FILLBMP_TILE, false ) == SfxItemState::SET;
            if( bStretch || bTile )
            {
                eState = PropertyState_DIRECT_VALUE;
            }
            else
            {
                eState = PropertyState_DEFAULT_VALUE;
            }
            break;
        }
        case OWN_ATTR_STYLE:
        {
            return PropertyState_DIRECT_VALUE;
        }
        case OWN_ATTR_TABLEBORDER:
        {
            const SfxItemSet& rSet = mpProperties->GetMergedItemSet();
            if( (rSet.GetItemState( SDRATTR_TABLE_BORDER_INNER, false ) == SfxItemState::DEFAULT) && (rSet.GetItemState( SDRATTR_TABLE_BORDER, false ) == SfxItemState::DEFAULT) )
                return PropertyState_DEFAULT_VALUE;

            return PropertyState_DIRECT_VALUE;
        }
        default:
        {
            const SfxItemSet& rSet = mpProperties->GetMergedItemSet();

            switch( rSet.GetItemState( pMap->nWID, false ) )
            {
            case SfxItemState::READONLY:
            case SfxItemState::SET:
                eState = PropertyState_DIRECT_VALUE;
                break;
            case SfxItemState::DEFAULT:
                eState = PropertyState_DEFAULT_VALUE;
                break;
            default:
                eState = PropertyState_AMBIGUOUS_VALUE;
                break;
            }

            // if a item is set, this doesn't mean we want it :)
            if( ( PropertyState_DIRECT_VALUE == eState ) )
            {
                switch( pMap->nWID )
                {
                // the following items are disabled by changing the
                // fill style or the line style. so there is no need
                // to export items without names which should be empty
                case XATTR_FILLBITMAP:
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_LINEDASH:
                    {
                        const NameOrIndex* pItem = static_cast<const NameOrIndex*>(rSet.GetItem((sal_uInt16)pMap->nWID));
                        if( ( pItem == nullptr ) || pItem->GetName().isEmpty() )
                            eState = PropertyState_DEFAULT_VALUE;
                    }
                    break;

                // #i36115#
                // If e.g. the LineStart is on NONE and thus the string has length 0, it still
                // may be a hard attribute covering the set LineStart of the parent (Style).
                // #i37644#
                // same is for fill float transparency
                case XATTR_LINEEND:
                case XATTR_LINESTART:
                case XATTR_FILLFLOATTRANSPARENCE:
                    {
                        const NameOrIndex* pItem = static_cast<const NameOrIndex*>(rSet.GetItem((sal_uInt16)pMap->nWID));
                        if( pItem == nullptr )
                            eState = PropertyState_DEFAULT_VALUE;
                    }
                    break;
                }
            }
        }
        }
        return eState;
    }
    throw UnknownPropertyException();
}


Sequence< PropertyState > SAL_CALL Cell::getPropertyStates( const Sequence< OUString >& aPropertyName ) throw(UnknownPropertyException, RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( (mpProperties == nullptr) || (GetModel() == nullptr) )
        throw DisposedException();

    const sal_Int32 nCount = aPropertyName.getLength();

    Sequence< PropertyState > aRet( nCount );

    const OUString* pNames = aPropertyName.getConstArray();
    PropertyState* pState = aRet.getArray();

    for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pNames++, pState++ )
    {
        try
        {
            *pState = getPropertyState( *pNames );
        }
        catch( Exception& )
        {
            *pState = PropertyState_AMBIGUOUS_VALUE;
        }
    }

    return aRet;
}


void SAL_CALL Cell::setPropertyToDefault( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( (mpProperties == nullptr) || (GetModel() == nullptr) )
        throw DisposedException();

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(PropertyName);
    if( pMap )
    {
        switch( pMap->nWID )
        {
        case OWN_ATTR_FILLBMP_MODE:
        {
            mpProperties->ClearObjectItem( XATTR_FILLBMP_STRETCH );
            mpProperties->ClearObjectItem( XATTR_FILLBMP_TILE );
            break;
        }
        case OWN_ATTR_STYLE:
            break;

        case OWN_ATTR_TABLEBORDER:
        {
            mpProperties->ClearObjectItem( SDRATTR_TABLE_BORDER_INNER );
            mpProperties->ClearObjectItem( SDRATTR_TABLE_BORDER );
            break;
        }

        default:
        {
            mpProperties->ClearObjectItem( pMap->nWID );
        }
        }

        GetModel()->SetChanged();
        return;
    }
    throw UnknownPropertyException();
}


Any SAL_CALL Cell::getPropertyDefault( const OUString& aPropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( (mpProperties == nullptr) || (GetModel() == nullptr) )
        throw DisposedException();

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(aPropertyName);
    if( pMap )
    {
        switch( pMap->nWID )
        {
        case OWN_ATTR_FILLBMP_MODE:
            return Any(  BitmapMode_NO_REPEAT );

        case OWN_ATTR_STYLE:
        {
            Reference< XStyle > xStyle;
            return Any( xStyle );
        }

        case OWN_ATTR_TABLEBORDER:
        {
            TableBorder aBorder;
            return Any( aBorder );
        }

        default:
        {
            if( SfxItemPool::IsWhich(pMap->nWID) )
            {
                SfxItemSet aSet( GetModel()->GetItemPool(), pMap->nWID, pMap->nWID);
                aSet.Put(GetModel()->GetItemPool().GetDefaultItem(pMap->nWID));
                return GetAnyForItem( aSet, pMap );
            }
        }
        }
    }
    throw UnknownPropertyException();
}


// XMultiPropertyStates


void SAL_CALL Cell::setAllPropertiesToDefault()
    throw (RuntimeException, std::exception)
{
    delete mpProperties;
    mpProperties = new sdr::properties::CellProperties( static_cast< SdrTableObj& >( GetObject() ), this );

    SdrOutliner& rOutliner = GetObject().ImpGetDrawOutliner();

    OutlinerParaObject* pParaObj = GetOutlinerParaObject();
    if( pParaObj )
    {
        rOutliner.SetText(*pParaObj);
        sal_Int32 nParaCount(rOutliner.GetParagraphCount());

        if(nParaCount)
        {
            ESelection aSelection( 0, 0, EE_PARA_ALL, EE_TEXTPOS_ALL);
            rOutliner.RemoveAttribs(aSelection, true, 0);

            OutlinerParaObject* pTemp = rOutliner.CreateParaObject(0, nParaCount);
            rOutliner.Clear();

            SetOutlinerParaObject(pTemp);
        }
    }
}


void SAL_CALL Cell::setPropertiesToDefault( const Sequence< OUString >& aPropertyNames ) throw (UnknownPropertyException, RuntimeException, std::exception)
{
    sal_Int32 nCount = aPropertyNames.getLength();
    const OUString* pName = aPropertyNames.getConstArray();

    while(nCount--)
        setPropertyToDefault( *pName++ );
}


Sequence< Any > SAL_CALL Cell::getPropertyDefaults( const Sequence< OUString >& aPropertyNames ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    sal_Int32 nCount = aPropertyNames.getLength();
    Sequence< Any > aDefaults( nCount );
    Any* pDefaults = aDefaults.getArray();
    const OUString* pName = aPropertyNames.getConstArray();

    while(nCount--)
        *pDefaults++ = getPropertyDefault( *pName++ );

    return aDefaults;
}


// XText


void SAL_CALL Cell::insertTextContent( const Reference< XTextRange >& xRange, const Reference< XTextContent >& xContent, sal_Bool bAbsorb ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    SvxUnoTextBase::insertTextContent( xRange, xContent, bAbsorb );
    notifyModified();
}


void SAL_CALL Cell::removeTextContent( const Reference< XTextContent >& xContent ) throw (NoSuchElementException, RuntimeException, std::exception)
{
    SvxUnoTextBase::removeTextContent( xContent );
    notifyModified();
}


// XSimpleText


Reference< XTextCursor > SAL_CALL Cell::createTextCursor(  ) throw (RuntimeException, std::exception)
{
    return SvxUnoTextBase::createTextCursor();
}


Reference< XTextCursor > SAL_CALL Cell::createTextCursorByRange( const Reference< XTextRange >& aTextPosition ) throw (RuntimeException, std::exception)
{
    return SvxUnoTextBase::createTextCursorByRange( aTextPosition );
}


void SAL_CALL Cell::insertString( const Reference< XTextRange >& xRange, const OUString& aString, sal_Bool bAbsorb ) throw (RuntimeException, std::exception)
{
    SvxUnoTextBase::insertString( xRange, aString, bAbsorb );
    notifyModified();
}


void SAL_CALL Cell::insertControlCharacter( const Reference< XTextRange >& xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    SvxUnoTextBase::insertControlCharacter( xRange, nControlCharacter, bAbsorb );
    notifyModified();
}


// XTextRange


Reference< XText > SAL_CALL Cell::getText(  ) throw (RuntimeException, std::exception)
{
    return SvxUnoTextBase::getText();
}


Reference< XTextRange > SAL_CALL Cell::getStart(  ) throw (RuntimeException, std::exception)
{
    return SvxUnoTextBase::getStart();
}


Reference< XTextRange > SAL_CALL Cell::getEnd(  ) throw (RuntimeException, std::exception)
{
    return SvxUnoTextBase::getEnd();
}


OUString SAL_CALL Cell::getString(  ) throw (RuntimeException, std::exception)
{
    maSelection.nStartPara = EE_PARA_MAX_COUNT;
    return SvxUnoTextBase::getString();
}


void SAL_CALL Cell::setString( const OUString& aString ) throw (RuntimeException, std::exception)
{
    SvxUnoTextBase::setString( aString );
    notifyModified();
}

// XEventListener
void SAL_CALL Cell::disposing( const EventObject& /*Source*/ ) throw (RuntimeException, std::exception)
{
    mxTable.clear();
    dispose();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
