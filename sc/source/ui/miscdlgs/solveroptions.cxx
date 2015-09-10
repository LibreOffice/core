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

#include "solveroptions.hxx"
#include "scresid.hxx"
#include "global.hxx"
#include "miscuno.hxx"
#include "solverutil.hxx"

#include <rtl/math.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <svtools/treelistentry.hxx>

#include <algorithm>

#include <com/sun/star/sheet/Solver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;

/// Helper for sorting properties
struct ScSolverOptionsEntry
{
    sal_Int32       nPosition;
    OUString   aDescription;

    ScSolverOptionsEntry() : nPosition(0) {}

    bool operator< (const ScSolverOptionsEntry& rOther) const
    {
        return (ScGlobal::GetCollator()->compareString( aDescription, rOther.aDescription ) < 0);
    }
};

class ScSolverOptionsString : public SvLBoxString
{
    bool        mbIsDouble;
    double      mfDoubleValue;
    sal_Int32   mnIntValue;

public:
    ScSolverOptionsString( SvTreeListEntry* pEntry, sal_uInt16 nFlags, const OUString& rStr ) :
        SvLBoxString( pEntry, nFlags, rStr ),
        mbIsDouble( false ),
        mfDoubleValue( 0.0 ),
        mnIntValue( 0 ) {}

    bool      IsDouble() const        { return mbIsDouble; }
    double    GetDoubleValue() const  { return mfDoubleValue; }
    sal_Int32 GetIntValue() const     { return mnIntValue; }

    void      SetDoubleValue( double fNew ) { mbIsDouble = true; mfDoubleValue = fNew; }
    void      SetIntValue( sal_Int32 nNew ) { mbIsDouble = false; mnIntValue = nNew; }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) SAL_OVERRIDE;
};

void ScSolverOptionsString::Paint(const Point& rPos, SvTreeListBox& /*rDev*/, vcl::RenderContext& rRenderContext,
                                  const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    //! move position? (SvxLinguTabPage: aPos.X() += 20)
    OUString aNormalStr(GetText());
    aNormalStr += ":";
    rRenderContext.DrawText(rPos, aNormalStr);

    Point aNewPos(rPos);
    aNewPos.X() += rRenderContext.GetTextWidth(aNormalStr);
    vcl::Font aOldFont(rRenderContext.GetFont());
    vcl::Font aFont(aOldFont);
    aFont.SetWeight(WEIGHT_BOLD);

    OUString sTxt(' ');
    if (mbIsDouble)
        sTxt += rtl::math::doubleToUString(mfDoubleValue,
            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
            ScGlobal::GetpLocaleData()->getNumDecimalSep()[0], true );
    else
        sTxt += OUString::number(mnIntValue);
    rRenderContext.SetFont(aFont);
    rRenderContext.DrawText(aNewPos, sTxt);

    rRenderContext.SetFont(aOldFont);
}

ScSolverOptionsDialog::ScSolverOptionsDialog( vcl::Window* pParent,
                        const uno::Sequence<OUString>& rImplNames,
                        const uno::Sequence<OUString>& rDescriptions,
                        const OUString& rEngine,
                        const uno::Sequence<beans::PropertyValue>& rProperties )
    : ModalDialog(pParent, "SolverOptionsDialog",
        "modules/scalc/ui/solveroptionsdialog.ui")
    , mpCheckButtonData(NULL)
    , maImplNames(rImplNames)
    , maDescriptions(rDescriptions)
    , maEngine(rEngine)
    , maProperties(rProperties)
{
    get(m_pLbEngine, "engine");
    get(m_pLbSettings, "settings");
    get(m_pBtnEdit, "edit");

    m_pLbEngine->SetSelectHdl( LINK( this, ScSolverOptionsDialog, EngineSelectHdl ) );

    m_pBtnEdit->SetClickHdl( LINK( this, ScSolverOptionsDialog, ButtonHdl ) );

    m_pLbSettings->SetStyle( m_pLbSettings->GetStyle()|WB_CLIPCHILDREN|WB_FORCE_MAKEVISIBLE );
    m_pLbSettings->SetHighlightRange();

    m_pLbSettings->SetSelectHdl( LINK( this, ScSolverOptionsDialog, SettingsSelHdl ) );
    m_pLbSettings->SetDoubleClickHdl( LINK( this, ScSolverOptionsDialog, SettingsDoubleClickHdl ) );

    sal_Int32 nSelect = -1;
    sal_Int32 nImplCount = maImplNames.getLength();
    for (sal_Int32 nImpl=0; nImpl<nImplCount; ++nImpl)
    {
        OUString aImplName( maImplNames[nImpl] );
        OUString aDescription( maDescriptions[nImpl] );   // user-visible descriptions in list box
        m_pLbEngine->InsertEntry( aDescription );
        if ( aImplName == maEngine )
            nSelect = nImpl;
    }
    if ( nSelect < 0 )                  // no (valid) engine given
    {
        if ( nImplCount > 0 )
        {
            maEngine = maImplNames[0];  // use first implementation
            nSelect = 0;
        }
        else
            maEngine.clear();
        maProperties.realloc(0);        // don't use options from different engine
    }
    if ( nSelect >= 0 )                 // select in list box
        m_pLbEngine->SelectEntryPos( static_cast<sal_uInt16>(nSelect) );

    if ( !maProperties.getLength() )
        ReadFromComponent();            // fill maProperties from component (using maEngine)
    FillListBox();                      // using maProperties
}

ScSolverOptionsDialog::~ScSolverOptionsDialog()
{
    disposeOnce();
}

void ScSolverOptionsDialog::dispose()
{
    delete mpCheckButtonData;
    m_pLbEngine.clear();
    m_pLbSettings.clear();
    m_pBtnEdit.clear();
    ModalDialog::dispose();
}

const uno::Sequence<beans::PropertyValue>& ScSolverOptionsDialog::GetProperties()
{
    // update maProperties from list box content
    // order of entries in list box and maProperties is the same
    sal_Int32 nEntryCount = maProperties.getLength();
    SvTreeList* pModel = m_pLbSettings->GetModel();
    if ( nEntryCount == (sal_Int32)pModel->GetEntryCount() )
    {
        for (sal_Int32 nEntryPos=0; nEntryPos<nEntryCount; ++nEntryPos)
        {
            uno::Any& rValue = maProperties[nEntryPos].Value;
            SvTreeListEntry* pEntry = pModel->GetEntry(nEntryPos);

            bool bHasData = false;
            sal_uInt16 nItemCount = pEntry->ItemCount();
            for (sal_uInt16 nItemPos=0; nItemPos<nItemCount && !bHasData; ++nItemPos)
            {
                SvLBoxItem& rItem = pEntry->GetItem( nItemPos );
                ScSolverOptionsString* pStringItem = dynamic_cast<ScSolverOptionsString*>(&rItem);
                if ( pStringItem )
                {
                    if ( pStringItem->IsDouble() )
                        rValue <<= pStringItem->GetDoubleValue();
                    else
                        rValue <<= pStringItem->GetIntValue();
                    bHasData = true;
                }
            }
            if ( !bHasData )
                ScUnoHelpFunctions::SetBoolInAny( rValue,
                                    m_pLbSettings->GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
        }
    }
    else
    {
        OSL_FAIL( "wrong count" );
    }

    return maProperties;
}

void ScSolverOptionsDialog::FillListBox()
{
    // get property descriptions, sort by them

    uno::Reference<sheet::XSolverDescription> xDesc( ScSolverUtil::GetSolver( maEngine ), uno::UNO_QUERY );
    sal_Int32 nCount = maProperties.getLength();
    std::vector<ScSolverOptionsEntry> aDescriptions( nCount );
    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
    {
        OUString aPropName( maProperties[nPos].Name );
        OUString aVisName;
        if ( xDesc.is() )
            aVisName = xDesc->getPropertyDescription( aPropName );
        if ( aVisName.isEmpty() )
            aVisName = aPropName;
        aDescriptions[nPos].nPosition = nPos;
        aDescriptions[nPos].aDescription = aVisName;
    }
    std::sort( aDescriptions.begin(), aDescriptions.end() );

    // also update maProperties to the order of descriptions

    uno::Sequence<beans::PropertyValue> aNewSeq;
    aNewSeq.realloc( nCount );
    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
        aNewSeq[nPos] = maProperties[ aDescriptions[nPos].nPosition ];
    maProperties = aNewSeq;

    // fill the list box

    m_pLbSettings->SetUpdateMode(false);
    m_pLbSettings->Clear();

    if (!mpCheckButtonData)
        mpCheckButtonData = new SvLBoxButtonData(m_pLbSettings);

    SvTreeList* pModel = m_pLbSettings->GetModel();
    SvTreeListEntry* pEntry = NULL;

    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
    {
        OUString aVisName = aDescriptions[nPos].aDescription;

        uno::Any aValue = maProperties[nPos].Value;
        uno::TypeClass eClass = aValue.getValueTypeClass();
        if ( eClass == uno::TypeClass_BOOLEAN )
        {
            // check box entry
            pEntry = new SvTreeListEntry;
            std::unique_ptr<SvLBoxButton> pButton(new SvLBoxButton(
                pEntry, SvLBoxButtonKind_enabledCheckbox, 0, mpCheckButtonData));
            if ( ScUnoHelpFunctions::GetBoolFromAny( aValue ) )
                pButton->SetStateChecked();
            else
                pButton->SetStateUnchecked();
            pEntry->AddItem(std::move(pButton));
            pEntry->AddItem(std::unique_ptr<SvLBoxContextBmp>(
                new SvLBoxContextBmp(pEntry, 0, Image(), Image(), false)));
            pEntry->AddItem(std::unique_ptr<SvLBoxString>(
                new SvLBoxString( pEntry, 0, aVisName)));
        }
        else
        {
            // value entry
            pEntry = new SvTreeListEntry;
            pEntry->AddItem(std::unique_ptr<SvLBoxString>(
                new SvLBoxString(pEntry, 0, ""))); // empty column
            pEntry->AddItem(std::unique_ptr<SvLBoxContextBmp>(
                new SvLBoxContextBmp(pEntry, 0, Image(), Image(), false)));
            std::unique_ptr<ScSolverOptionsString> pItem(
                new ScSolverOptionsString(pEntry, 0, aVisName));
            if ( eClass == uno::TypeClass_DOUBLE )
            {
                double fDoubleValue = 0.0;
                if ( aValue >>= fDoubleValue )
                    pItem->SetDoubleValue( fDoubleValue );
            }
            else
            {
                sal_Int32 nIntValue = 0;
                if ( aValue >>= nIntValue )
                    pItem->SetIntValue( nIntValue );
            }
            pEntry->AddItem(std::move(pItem));
        }
        pModel->Insert( pEntry );
    }

    m_pLbSettings->SetUpdateMode(true);
}

void ScSolverOptionsDialog::ReadFromComponent()
{
    maProperties = ScSolverUtil::GetDefaults( maEngine );
}

void ScSolverOptionsDialog::EditOption()
{
    SvTreeListEntry* pEntry = m_pLbSettings->GetCurEntry();
    if (pEntry)
    {
        sal_uInt16 nItemCount = pEntry->ItemCount();
        for (sal_uInt16 nPos=0; nPos<nItemCount; ++nPos)
        {
            SvLBoxItem& rItem = pEntry->GetItem( nPos );
            ScSolverOptionsString* pStringItem = dynamic_cast<ScSolverOptionsString*>(&rItem);
            if ( pStringItem )
            {
                if ( pStringItem->IsDouble() )
                {
                    ScopedVclPtrInstance< ScSolverValueDialog > aValDialog( this );
                    aValDialog->SetOptionName( pStringItem->GetText() );
                    aValDialog->SetValue( pStringItem->GetDoubleValue() );
                    if ( aValDialog->Execute() == RET_OK )
                    {
                        pStringItem->SetDoubleValue( aValDialog->GetValue() );
                        m_pLbSettings->InvalidateEntry( pEntry );
                    }
                }
                else
                {
                    ScopedVclPtrInstance< ScSolverIntegerDialog > aIntDialog( this );
                    aIntDialog->SetOptionName( pStringItem->GetText() );
                    aIntDialog->SetValue( pStringItem->GetIntValue() );
                    if ( aIntDialog->Execute() == RET_OK )
                    {
                        pStringItem->SetIntValue( aIntDialog->GetValue() );
                        m_pLbSettings->InvalidateEntry( pEntry );
                    }
                }
            }
        }
    }
}

IMPL_LINK_TYPED( ScSolverOptionsDialog, ButtonHdl, Button*, pBtn, void )
{
    if (pBtn == m_pBtnEdit)
        EditOption();
}

IMPL_LINK_NOARG_TYPED(ScSolverOptionsDialog, SettingsDoubleClickHdl, SvTreeListBox*, bool)
{
    EditOption();
    return false;
}

IMPL_LINK_NOARG(ScSolverOptionsDialog, EngineSelectHdl)
{
    sal_uInt16 nSelectPos = m_pLbEngine->GetSelectEntryPos();
    if ( nSelectPos < maImplNames.getLength() )
    {
        OUString aNewEngine( maImplNames[nSelectPos] );
        if ( aNewEngine != maEngine )
        {
            maEngine = aNewEngine;
            ReadFromComponent();            // fill maProperties from component (using maEngine)
            FillListBox();                  // using maProperties
        }
    }
    return 0;
}

IMPL_LINK_NOARG_TYPED(ScSolverOptionsDialog, SettingsSelHdl, SvTreeListBox*, void)
{
    bool bCheckbox = false;

    SvTreeListEntry* pEntry = m_pLbSettings->GetCurEntry();
    if (pEntry)
    {
        SvLBoxItem* pItem = pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON);
        if (pItem && pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
            bCheckbox = true;
    }

    m_pBtnEdit->Enable( !bCheckbox );
}

ScSolverIntegerDialog::ScSolverIntegerDialog(vcl::Window * pParent)
    : ModalDialog( pParent, "IntegerDialog",
        "modules/scalc/ui/integerdialog.ui" )
{
    get(m_pFrame, "frame");
    get(m_pNfValue, "value");
}

ScSolverIntegerDialog::~ScSolverIntegerDialog()
{
    disposeOnce();
}

void ScSolverIntegerDialog::dispose()
{
    m_pFrame.clear();
    m_pNfValue.clear();
    ModalDialog::dispose();
}

void ScSolverIntegerDialog::SetOptionName( const OUString& rName )
{
    m_pFrame->set_label(rName);
}

void ScSolverIntegerDialog::SetValue( sal_Int32 nValue )
{
    m_pNfValue->SetValue( nValue );
}

sal_Int32 ScSolverIntegerDialog::GetValue() const
{
    sal_Int64 nValue = m_pNfValue->GetValue();
    if ( nValue < SAL_MIN_INT32 )
        return SAL_MIN_INT32;
    if ( nValue > SAL_MAX_INT32 )
        return SAL_MAX_INT32;
    return (sal_Int32) nValue;
}

ScSolverValueDialog::ScSolverValueDialog( vcl::Window * pParent )
    : ModalDialog( pParent, "DoubleDialog",
        "modules/scalc/ui/doubledialog.ui" )
{
    get(m_pFrame, "frame");
    get(m_pEdValue, "value");
}

ScSolverValueDialog::~ScSolverValueDialog()
{
    disposeOnce();
}

void ScSolverValueDialog::dispose()
{
    m_pFrame.clear();
    m_pEdValue.clear();
    ModalDialog::dispose();
}

void ScSolverValueDialog::SetOptionName( const OUString& rName )
{
    m_pFrame->set_label(rName);
}

void ScSolverValueDialog::SetValue( double fValue )
{
    m_pEdValue->SetText( rtl::math::doubleToUString( fValue,
            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
            ScGlobal::GetpLocaleData()->getNumDecimalSep()[0], true ) );
}

double ScSolverValueDialog::GetValue() const
{
    OUString aInput = m_pEdValue->GetText();

    const LocaleDataWrapper* pLocaleData = ScGlobal::GetpLocaleData();
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    double fValue = rtl::math::stringToDouble( aInput,
                            pLocaleData->getNumDecimalSep()[0],
                            pLocaleData->getNumThousandSep()[0],
                            &eStatus, NULL );
    return fValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
