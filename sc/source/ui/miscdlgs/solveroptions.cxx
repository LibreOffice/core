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
#include <solveroptions.hxx>
#include <global.hxx>
#include <miscuno.hxx>
#include <solverutil.hxx>

#include <rtl/math.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <osl/diagnose.h>

#include <algorithm>

#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace com::sun::star;

namespace {

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

}

ScSolverOptionsDialog::ScSolverOptionsDialog(weld::Window* pParent,
                        const uno::Sequence<OUString>& rImplNames,
                        const uno::Sequence<OUString>& rDescriptions,
                        const OUString& rEngine,
                        const uno::Sequence<beans::PropertyValue>& rProperties )
    : GenericDialogController(pParent, "modules/scalc/ui/solveroptionsdialog.ui", "SolverOptionsDialog")
    , maImplNames(rImplNames)
    , maEngine(rEngine)
    , maProperties(rProperties)
    , m_xLbEngine(m_xBuilder->weld_combo_box("engine"))
    , m_xLbSettings(m_xBuilder->weld_tree_view("settings"))
    , m_xBtnEdit(m_xBuilder->weld_button("edit"))
{
    m_xLbSettings->set_size_request(m_xLbSettings->get_approximate_digit_width() * 32,
                                    m_xLbSettings->get_height_rows(6));

    m_xLbSettings->enable_toggle_buttons(weld::ColumnToggleType::Check);

    m_xLbEngine->connect_changed( LINK( this, ScSolverOptionsDialog, EngineSelectHdl ) );

    m_xBtnEdit->connect_clicked( LINK( this, ScSolverOptionsDialog, ButtonHdl ) );

    m_xLbSettings->connect_changed( LINK( this, ScSolverOptionsDialog, SettingsSelHdl ) );
    m_xLbSettings->connect_row_activated( LINK( this, ScSolverOptionsDialog, SettingsDoubleClickHdl ) );

    sal_Int32 nSelect = -1;
    sal_Int32 nImplCount = maImplNames.getLength();
    for (sal_Int32 nImpl=0; nImpl<nImplCount; ++nImpl)
    {
        OUString aImplName( maImplNames[nImpl] );
        OUString aDescription( rDescriptions[nImpl] );   // user-visible descriptions in list box
        m_xLbEngine->append_text(aDescription);
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
        m_xLbEngine->set_active(nSelect);

    if ( !maProperties.hasElements() )
        ReadFromComponent();            // fill maProperties from component (using maEngine)
    FillListBox();                      // using maProperties
}

ScSolverOptionsDialog::~ScSolverOptionsDialog()
{
    if (m_xIntDialog)
        m_xIntDialog->response(RET_CANCEL);
    assert(!m_xIntDialog);
    if (m_xValDialog)
        m_xValDialog->response(RET_CANCEL);
    assert(!m_xValDialog);
}

const uno::Sequence<beans::PropertyValue>& ScSolverOptionsDialog::GetProperties()
{
    // update maProperties from list box content
    // order of entries in list box and maProperties is the same
    sal_Int32 nEntryCount = maProperties.getLength();
    if (nEntryCount == m_xLbSettings->n_children())
    {
        for (sal_Int32 nEntryPos=0; nEntryPos<nEntryCount; ++nEntryPos)
        {
            uno::Any& rValue = maProperties[nEntryPos].Value;
            if (ScSolverOptionsString* pStringItem = reinterpret_cast<ScSolverOptionsString*>(m_xLbSettings->get_id(nEntryPos).toInt64()))
            {
                if (pStringItem->IsDouble())
                    rValue <<= pStringItem->GetDoubleValue();
                else
                    rValue <<= pStringItem->GetIntValue();
            }
            else
                rValue <<= m_xLbSettings->get_toggle(nEntryPos) == TRISTATE_TRUE;
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
    std::transform(aDescriptions.begin(), aDescriptions.end(), aNewSeq.begin(),
        [this](const ScSolverOptionsEntry& rDescr) -> beans::PropertyValue { return maProperties[ rDescr.nPosition ]; });
    maProperties = aNewSeq;

    // fill the list box

    m_xLbSettings->freeze();
    m_xLbSettings->clear();

    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
    {
        OUString aVisName = aDescriptions[nPos].aDescription;

        uno::Any aValue = maProperties[nPos].Value;
        uno::TypeClass eClass = aValue.getValueTypeClass();

        m_xLbSettings->append();

        if ( eClass == uno::TypeClass_BOOLEAN )
        {
            // check box entry
            m_xLbSettings->set_toggle(nPos, ScUnoHelpFunctions::GetBoolFromAny(aValue) ? TRISTATE_TRUE : TRISTATE_FALSE);
            m_xLbSettings->set_text(nPos, aVisName, 0);
        }
        else
        {
            // value entry
            m_xLbSettings->set_text(nPos, aVisName, 0);
            m_aOptions.emplace_back(new ScSolverOptionsString(aVisName));
            if (eClass == uno::TypeClass_DOUBLE)
            {
                double fDoubleValue = 0.0;
                if (aValue >>= fDoubleValue)
                    m_aOptions.back()->SetDoubleValue(fDoubleValue);

                OUString sTxt = aVisName + ": ";
                sTxt += rtl::math::doubleToUString(fDoubleValue,
                    rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                    ScGlobal::getLocaleDataPtr()->getNumDecimalSep()[0], true );

                m_xLbSettings->set_text(nPos, sTxt, 0);
            }
            else
            {
                sal_Int32 nIntValue = 0;
                if (aValue >>= nIntValue)
                    m_aOptions.back()->SetIntValue(nIntValue);

                OUString sTxt = aVisName + ": " + OUString::number(nIntValue);

                m_xLbSettings->set_text(nPos, sTxt, 0);
            }
            m_xLbSettings->set_id(nPos, OUString::number(reinterpret_cast<sal_Int64>(m_aOptions.back().get())));
        }
    }

    m_xLbSettings->thaw();
}

void ScSolverOptionsDialog::ReadFromComponent()
{
    maProperties = ScSolverUtil::GetDefaults( maEngine );
}

void ScSolverOptionsDialog::EditOption()
{
    int nEntry = m_xLbSettings->get_selected_index();
    if (nEntry == -1)
        return;
    ScSolverOptionsString* pStringItem = reinterpret_cast<ScSolverOptionsString*>(m_xLbSettings->get_id(nEntry).toInt64());
    if (!pStringItem)
        return;

    if (pStringItem->IsDouble())
    {
        m_xValDialog = std::make_shared<ScSolverValueDialog>(m_xDialog.get());
        m_xValDialog->SetOptionName(pStringItem->GetText());
        if (maProperties[nEntry].Name == "DECR")
            m_xValDialog->SetMax(1.0);
        else if (maProperties[nEntry].Name == "DEFactorMax")
            m_xValDialog->SetMax(1.2);
        else if (maProperties[nEntry].Name == "DEFactorMin")
            m_xValDialog->SetMax(1.2);
        else if (maProperties[nEntry].Name == "PSCL")
            m_xValDialog->SetMax(0.005);
        m_xValDialog->SetValue(pStringItem->GetDoubleValue());
        weld::DialogController::runAsync(m_xValDialog, [nEntry, pStringItem, this](sal_Int32 nResult){
            if (nResult == RET_OK)
            {
                pStringItem->SetDoubleValue(m_xValDialog->GetValue());

                OUString sTxt(pStringItem->GetText() + ": ");
                sTxt += rtl::math::doubleToUString(pStringItem->GetDoubleValue(),
                    rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                    ScGlobal::getLocaleDataPtr()->getNumDecimalSep()[0], true );

                m_xLbSettings->set_text(nEntry, sTxt, 0);
            }
            m_xValDialog.reset();
        });
    }
    else
    {
        m_xIntDialog = std::make_shared<ScSolverIntegerDialog>(m_xDialog.get());
        m_xIntDialog->SetOptionName( pStringItem->GetText() );
        if (maProperties[nEntry].Name == "EpsilonLevel")
            m_xIntDialog->SetMax(3);
        else if (maProperties[nEntry].Name == "Algorithm")
            m_xIntDialog->SetMax(1);
        m_xIntDialog->SetValue( pStringItem->GetIntValue() );
        weld::DialogController::runAsync(m_xIntDialog, [nEntry, pStringItem, this](sal_Int32 nResult){
            if (nResult == RET_OK)
            {
                pStringItem->SetIntValue(m_xIntDialog->GetValue());

                OUString sTxt(pStringItem->GetText() + ": ");
                sTxt += OUString::number(pStringItem->GetIntValue());

                m_xLbSettings->set_text(nEntry, sTxt, 0);
            }
            m_xIntDialog.reset();
        });
    }
}

IMPL_LINK( ScSolverOptionsDialog, ButtonHdl, weld::Button&, rBtn, void )
{
    if (&rBtn == m_xBtnEdit.get())
        EditOption();
}

IMPL_LINK_NOARG(ScSolverOptionsDialog, SettingsDoubleClickHdl, weld::TreeView&, bool)
{
    EditOption();
    return true;
}

IMPL_LINK_NOARG(ScSolverOptionsDialog, EngineSelectHdl, weld::ComboBox&, void)
{
    const sal_Int32 nSelectPos = m_xLbEngine->get_active();
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
}

IMPL_LINK_NOARG(ScSolverOptionsDialog, SettingsSelHdl, weld::TreeView&, void)
{
    bool bCheckbox = false;

    int nEntry = m_xLbSettings->get_selected_index();
    if (nEntry != -1)
    {
        ScSolverOptionsString* pStringItem = reinterpret_cast<ScSolverOptionsString*>(m_xLbSettings->get_id(nEntry).toInt64());
        if (!pStringItem)
            bCheckbox = true;
    }

    m_xBtnEdit->set_sensitive(!bCheckbox);
}

ScSolverIntegerDialog::ScSolverIntegerDialog(weld::Window * pParent)
    : GenericDialogController(pParent, "modules/scalc/ui/integerdialog.ui", "IntegerDialog")
    , m_xFrame(m_xBuilder->weld_frame("frame"))
    , m_xNfValue(m_xBuilder->weld_spin_button("value"))
{
}

ScSolverIntegerDialog::~ScSolverIntegerDialog()
{
}

void ScSolverIntegerDialog::SetOptionName( const OUString& rName )
{
    m_xFrame->set_label(rName);
}

void ScSolverIntegerDialog::SetValue( sal_Int32 nValue )
{
    m_xNfValue->set_value( nValue );
}

void ScSolverIntegerDialog::SetMax( sal_Int32 nMax )
{
    m_xNfValue->set_range(0, nMax);
}

sal_Int32 ScSolverIntegerDialog::GetValue() const
{
    return m_xNfValue->get_value();
}

ScSolverValueDialog::ScSolverValueDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/scalc/ui/doubledialog.ui", "DoubleDialog")
    , m_xFrame(m_xBuilder->weld_frame("frame"))
    , m_xEdValue(m_xBuilder->weld_entry("value"))
{
    ::rtl::math::setNan(&m_fMaxValue);
}

ScSolverValueDialog::~ScSolverValueDialog()
{
}

void ScSolverValueDialog::SetOptionName( const OUString& rName )
{
    m_xFrame->set_label(rName);
}

void ScSolverValueDialog::SetValue( double fValue )
{
    m_xEdValue->set_text( rtl::math::doubleToUString( fValue,
            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
            ScGlobal::getLocaleDataPtr()->getNumDecimalSep()[0], true ) );
}

void ScSolverValueDialog::SetMax(double fMax)
{
    m_fMaxValue = fMax;
}

double ScSolverValueDialog::GetValue() const
{
    OUString aInput = m_xEdValue->get_text();

    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nParseEnd = 0;
    double fValue = ScGlobal::getLocaleDataPtr()->stringToDouble( aInput, true, &eStatus, &nParseEnd);
    /* TODO: shouldn't there be some error checking? */
    if (!std::isnan(m_fMaxValue) && fValue > m_fMaxValue)
        fValue = m_fMaxValue;
    return fValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
