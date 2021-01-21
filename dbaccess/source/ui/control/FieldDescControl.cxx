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

#include <core_resource.hxx>
#include <FieldDescControl.hxx>
#include <FieldControls.hxx>
#include <tools/diagnose_ex.h>
#include <TableDesignHelpBar.hxx>
#include <vcl/svapp.hxx>
#include <FieldDescriptions.hxx>
#include <svl/zforlist.hxx>
#include <svl/numuno.hxx>
#include <vcl/transfer.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatPreviewer.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <QEnumTypes.hxx>
#include <helpids.h>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/types.hxx>
#include <UITools.hxx>
#include <strings.hrc>
#include <osl/diagnose.h>

using namespace dbaui;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::util;

namespace
{
    template< typename T1, typename T2> void lcl_HideAndDeleteControl(short& _nPos,std::unique_ptr<T1>& _pControl, std::unique_ptr<T2>& _pControlText)
    {
        if ( _pControl )
        {
            --_nPos;
            _pControl->hide();
            _pControlText->hide();
            _pControl.reset();
            _pControlText.reset();
        }
    }
}

OFieldDescControl::OFieldDescControl(weld::Container* pPage, OTableDesignHelpBar* pHelpBar)
    : m_xBuilder(Application::CreateBuilder(pPage, "dbaccess/ui/fielddescpage.ui"))
    , m_xContainer(m_xBuilder->weld_container("FieldDescPage"))
    , m_pHelp( pHelpBar )
    , m_pLastFocusWindow(nullptr)
    , m_pActFocusWindow(nullptr)
    , m_pPreviousType()
    , m_nPos(-1)
    , aYes(DBA_RES(STR_VALUE_YES))
    , aNo(DBA_RES(STR_VALUE_NO))
    , m_nEditWidth(50)
    , pActFieldDescr(nullptr)
{
    if (m_pHelp)
        m_pHelp->connect_focus_out(LINK(this, OFieldDescControl, HelpFocusOut));
}

OFieldDescControl::~OFieldDescControl()
{
    dispose();
}

void OFieldDescControl::dispose()
{
    // Destroy children
    DeactivateAggregate( tpDefault );
    DeactivateAggregate( tpRequired );
    DeactivateAggregate( tpTextLen );
    DeactivateAggregate( tpNumType );
    DeactivateAggregate( tpScale );
    DeactivateAggregate( tpLength );
    DeactivateAggregate( tpFormat );
    DeactivateAggregate( tpAutoIncrement );
    DeactivateAggregate( tpBoolDefault );
    DeactivateAggregate( tpColumnName );
    DeactivateAggregate( tpType );
    DeactivateAggregate( tpAutoIncrementValue );
    m_pHelp = nullptr;
    m_pLastFocusWindow = nullptr;
    m_pActFocusWindow = nullptr;
    m_xDefaultText.reset();
    m_xRequiredText.reset();
    m_xAutoIncrementText.reset();
    m_xTextLenText.reset();
    m_xNumTypeText.reset();
    m_xLengthText.reset();
    m_xScaleText.reset();
    m_xFormatText.reset();
    m_xBoolDefaultText.reset();
    m_xColumnNameText.reset();
    m_xTypeText.reset();
    m_xAutoIncrementValueText.reset();
    m_xRequired.reset();
    m_xNumType.reset();
    m_xAutoIncrement.reset();
    m_xDefault.reset();
    m_xTextLen.reset();
    m_xLength.reset();
    m_xScale.reset();
    m_xFormatSample.reset();
    m_xBoolDefault.reset();
    m_xColumnName.reset();
    m_xType.reset();
    m_xAutoIncrementValue.reset();
    m_xFormat.reset();
    m_xContainer.reset();
    m_xBuilder.reset();
}

OUString OFieldDescControl::BoolStringPersistent(std::u16string_view rUIString) const
{
    if (rUIString == aNo)
        return OUString('0');
    if (rUIString == aYes)
        return OUString('1');
    return OUString();
}

OUString OFieldDescControl::BoolStringUI(const OUString& rPersistentString) const
{
    // Older versions may store a language dependent string as a default
    if (rPersistentString == aYes || rPersistentString == aNo)
        return rPersistentString;

    if (rPersistentString == "0")
        return aNo;
    if (rPersistentString == "1")
        return aYes;

    return DBA_RES(STR_VALUE_NONE);
}

void OFieldDescControl::Init()
{
    Reference< css::util::XNumberFormatter > xFormatter = GetFormatter();
    ::dbaui::setEvalDateFormatForFormatter(xFormatter);
}

void OFieldDescControl::SetReadOnly( bool bReadOnly )
{
    // Enable/disable Controls
    OWidgetBase* ppAggregates[]     = {   m_xRequired.get(), m_xNumType.get()
                                        , m_xAutoIncrement.get(), m_xDefault.get()
                                        , m_xTextLen.get(), m_xLength.get()
                                        , m_xScale.get(), m_xColumnName.get()
                                        , m_xType.get(), m_xAutoIncrementValue.get()
    };
    weld::Widget* ppAggregatesText[] = {  m_xRequiredText.get(), m_xNumTypeText.get()
                                        , m_xAutoIncrementText.get(), m_xDefaultText.get()
                                        , m_xTextLenText.get(), m_xLengthText.get()
                                        , m_xScaleText.get(), m_xColumnNameText.get()
                                        , m_xTypeText.get(), m_xAutoIncrementValueText.get()
    };

    OSL_ENSURE(SAL_N_ELEMENTS(ppAggregates) == SAL_N_ELEMENTS(ppAggregatesText),"Lists are not identical!");

    for (size_t i=0; i<SAL_N_ELEMENTS(ppAggregates); ++i)
    {
        if ( ppAggregatesText[i] )
            ppAggregatesText[i]->set_sensitive( !bReadOnly );
        if ( ppAggregates[i] )
            ppAggregates[i]->set_sensitive( !bReadOnly );
    }

    if (m_xFormat)
    {
        assert(m_xFormatText);
        m_xFormat->set_sensitive(!bReadOnly);
        m_xFormatText->set_sensitive(!bReadOnly);
    }
}

void OFieldDescControl::SetControlText( sal_uInt16 nControlId, const OUString& rText )
{
    // Set the Controls' texts
    switch( nControlId )
    {
        case FIELD_PROPERTY_BOOL_DEFAULT:
            if (m_xBoolDefault)
            {
                OUString sOld = m_xBoolDefault->get_active_text();
                m_xBoolDefault->set_active_text(rText);
                if (sOld != rText)
                    ChangeHdl(m_xBoolDefault->GetComboBox());
            }
            break;
        case FIELD_PROPERTY_DEFAULT:
            if (m_xDefault)
            {
                m_xDefault->set_text(rText);
                UpdateFormatSample(pActFieldDescr);
            }
            break;

        case FIELD_PROPERTY_REQUIRED:
            if (m_xRequired)
                m_xRequired->set_active_text(rText);
            break;

        case FIELD_PROPERTY_TEXTLEN:
            if (m_xTextLen)
                m_xTextLen->set_text(rText);
            break;

        case FIELD_PROPERTY_NUMTYPE:
            if (m_xNumType)
                m_xNumType->set_active_text(rText);
            break;

        case FIELD_PROPERTY_AUTOINC:
            if (m_xAutoIncrement)
            {
                OUString sOld = m_xAutoIncrement->get_active_text();
                m_xAutoIncrement->set_active_text(rText);
                if (sOld != rText)
                    ChangeHdl(m_xAutoIncrement->GetComboBox());
            }
            break;

        case FIELD_PROPERTY_LENGTH:
            if (m_xLength)
                m_xLength->set_text(rText);
            break;

        case FIELD_PROPERTY_SCALE:
            if (m_xScale)
                m_xScale->set_text(rText);
            break;

        case FIELD_PROPERTY_FORMAT:
            if (pActFieldDescr)
                UpdateFormatSample(pActFieldDescr);
            break;
        case FIELD_PROPERTY_COLUMNNAME:
            if (m_xColumnName)
                m_xColumnName->set_text(rText);
            break;
        case FIELD_PROPERTY_TYPE:
            if (m_xType)
                m_xType->set_active_text(rText);
            break;
        case FIELD_PROPERTY_AUTOINCREMENT:
            if (m_xAutoIncrementValue)
                m_xAutoIncrementValue->set_text(rText);
            break;
    }
}

IMPL_LINK_NOARG(OFieldDescControl, FormatClickHdl, weld::Button&, void)
{
    // Create temporary Column, which is used for data exchange with Dialog
    if( !pActFieldDescr )
        return;

    sal_Int32 nOldFormatKey(pActFieldDescr->GetFormatKey());
    SvxCellHorJustify rOldJustify = pActFieldDescr->GetHorJustify();
    Reference< XNumberFormatsSupplier >  xSupplier = GetFormatter()->getNumberFormatsSupplier();
    SvNumberFormatsSupplierObj* pSupplierImpl = comphelper::getUnoTunnelImplementation<SvNumberFormatsSupplierObj>( xSupplier );
    if (!pSupplierImpl)
        return;

    SvNumberFormatter* pFormatter = pSupplierImpl->GetNumberFormatter();
    if(!::dbaui::callColumnFormatDialog(m_xContainer.get(),pFormatter,pActFieldDescr->GetType(),nOldFormatKey,rOldJustify,true))
        return;

    bool bModified = false;
    if(nOldFormatKey != pActFieldDescr->GetFormatKey())
    {
        pActFieldDescr->SetFormatKey( nOldFormatKey );
        bModified = true;
    }
    if(rOldJustify != pActFieldDescr->GetHorJustify())
    {
        pActFieldDescr->SetHorJustify( rOldJustify );
        bModified = true;
    }

    if(bModified)
    {
        SetModified(true);
        UpdateFormatSample(pActFieldDescr);
    }
}

void OFieldDescControl::SetModified(bool /*bModified*/)
{
}

IMPL_LINK(OFieldDescControl, ChangeHdl, weld::ComboBox&, rListBox, void)
{
    if (!pActFieldDescr)
        return;

    if (rListBox.get_value_changed_from_saved())
        SetModified(true);

    // Special treatment for Bool fields
    if (m_xRequired && &rListBox == m_xRequired->GetWidget() && m_xBoolDefault)
    {
        // If m_xRequired = sal_True then the sal_Bool field must NOT contain <<none>>
        OUString sDef = BoolStringUI(::comphelper::getString(pActFieldDescr->GetControlDefault()));

        if (m_xRequired->get_active() == 0) // Yes
        {
            m_xBoolDefault->remove_text(DBA_RES(STR_VALUE_NONE));
            if (sDef != aYes && sDef != aNo)
                m_xBoolDefault->set_active(1);  // No as a default
            else
                m_xBoolDefault->set_active_text(sDef);
        }
        else if (m_xBoolDefault->get_count() < 3)
        {
            m_xBoolDefault->append_text(DBA_RES(STR_VALUE_NONE));
            m_xBoolDefault->set_active_text(sDef);
        }
    }

    // A special treatment only for AutoIncrement
    if (m_xAutoIncrement && &rListBox == m_xAutoIncrement->GetWidget())
    {
        if (rListBox.get_active() == 1)
        { // no
            DeactivateAggregate( tpAutoIncrementValue );
            if(pActFieldDescr->IsPrimaryKey())
                DeactivateAggregate( tpRequired );
            else if( pActFieldDescr->getTypeInfo()->bNullable )
            {
                ActivateAggregate( tpRequired );
                if (m_xRequired)
                {
                    if( pActFieldDescr->IsNullable() )
                        m_xRequired->set_active(1); // no
                    else
                        m_xRequired->set_active(0); // yes
                }
            }
            ActivateAggregate( tpDefault );
        }
        else
        {
            DeactivateAggregate( tpRequired );
            DeactivateAggregate( tpDefault );
            ActivateAggregate( tpAutoIncrementValue );
        }
    }

    if (m_xType && &rListBox == m_xType->GetWidget())
    {
        TOTypeInfoSP pTypeInfo = getTypeInfo(m_xType->get_active());
        pActFieldDescr->FillFromTypeInfo(pTypeInfo,true,false); // SetType(pTypeInfo);

        DisplayData(pActFieldDescr);
        CellModified(-1, m_xType->GetPos());
    }
}

void OFieldDescControl::ActivateAggregate( EControlType eType )
{
    // Create Controls
    switch( eType )
    {
    case tpDefault:
        if (m_xDefault)
            return;
        m_nPos++;
        m_xDefaultText = m_xBuilder->weld_label("DefaultValueText");
        m_xDefaultText->show();
        m_xDefault = std::make_unique<OPropEditCtrl>(
                m_xBuilder->weld_entry("DefaultValue"), STR_HELP_DEFAULT_VALUE, FIELD_PROPERTY_DEFAULT);
        InitializeControl(m_xDefault->GetWidget(),HID_TAB_ENT_DEFAULT);
        m_xDefault->show();
        break;
    case tpAutoIncrementValue:
        if (m_xAutoIncrementValue || !isAutoIncrementValueEnabled())
            return;
        m_nPos++;
        m_xAutoIncrementValueText = m_xBuilder->weld_label("AutoIncrementValueText");
        m_xAutoIncrementValueText->show();
        m_xAutoIncrementValue = std::make_unique<OPropEditCtrl>(
                m_xBuilder->weld_spin_button("AutoIncrementValue"), STR_HELP_AUTOINCREMENT_VALUE,
                FIELD_PROPERTY_AUTOINCREMENT);
        m_xAutoIncrementValue->set_text( getAutoIncrementValue() );
        InitializeControl(m_xAutoIncrementValue->GetWidget(),HID_TAB_AUTOINCREMENTVALUE);
        m_xAutoIncrementValue->show();
        break;

    case tpRequired:
    {
        if (m_xRequired)
            return;
        Reference< XDatabaseMetaData> xMetaData = getMetaData();

        if(xMetaData.is() && xMetaData->supportsNonNullableColumns())
        {
            m_nPos++;
            m_xRequiredText = m_xBuilder->weld_label("RequiredText");
            m_xRequiredText->show();
            m_xRequired = std::make_unique<OPropListBoxCtrl>(
                    m_xBuilder->weld_combo_box("Required"), STR_HELP_AUTOINCREMENT_VALUE,
                    FIELD_PROPERTY_AUTOINCREMENT);
            m_xRequired->append_text(aYes);
            m_xRequired->append_text(aNo);
            m_xRequired->set_active(1);

            InitializeControl(m_xRequired.get(),HID_TAB_ENT_REQUIRED, true);
            m_xRequired->show();
        }
    }
    break;
    case tpAutoIncrement:
    {
        if (m_xAutoIncrement)
            return;
        m_nPos++;
        m_xAutoIncrementText = m_xBuilder->weld_label("AutoIncrementText");
        m_xAutoIncrementText->show();
        m_xAutoIncrement = std::make_unique<OPropListBoxCtrl>(
                m_xBuilder->weld_combo_box("AutoIncrement"), STR_HELP_AUTOINCREMENT,
                FIELD_PROPERTY_AUTOINC);
        m_xAutoIncrement->append_text(aYes);
        m_xAutoIncrement->append_text(aNo);
        m_xAutoIncrement->set_active(0);
        InitializeControl(m_xAutoIncrement.get(),HID_TAB_ENT_AUTOINCREMENT, true);
        m_xAutoIncrement->show();
    }
    break;
    case tpTextLen:
        if (m_xTextLen)
            return;
        m_nPos++;
        m_xTextLenText = m_xBuilder->weld_label("TextLengthText");
        m_xTextLenText->show();
        m_xTextLen = CreateNumericControl("TextLength", STR_HELP_TEXT_LENGTH, FIELD_PROPERTY_TEXTLEN,HID_TAB_ENT_TEXT_LEN);
        break;

    case tpType:
        if (m_xType)
            return;
        m_nPos++;
        m_xTypeText = m_xBuilder->weld_label("TypeText");
        m_xTypeText->show();
        m_xType = std::make_unique<OPropListBoxCtrl>(
                m_xBuilder->weld_combo_box("Type"), STR_HELP_AUTOINCREMENT, FIELD_PROPERTY_TYPE);
        {
            const OTypeInfoMap* pTypeInfo = getTypeInfo();
            for (auto const& elem : *pTypeInfo)
                m_xType->append_text(elem.second->aUIName);
        }
        m_xType->set_active(0);
        InitializeControl(m_xType.get(),HID_TAB_ENT_TYPE, true);
        m_xType->show();
        break;
    case tpColumnName:
        if (m_xColumnName)
            return;
        m_nPos++;
        {
            sal_Int32 nMax(0);
            OUString aTmpString;
            try
            {
                Reference< XDatabaseMetaData> xMetaData = getMetaData();
                if ( xMetaData.is() )
                {
                    nMax =  xMetaData->getMaxColumnNameLength();
                    aTmpString = xMetaData->getExtraNameCharacters();
                }
            }
            catch (const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
            m_xColumnNameText = m_xBuilder->weld_label("ColumnNameText");
            m_xColumnNameText->show();
            m_xColumnName = std::make_unique<OPropColumnEditCtrl>(
                    m_xBuilder->weld_entry("ColumnName"), aTmpString,
                    STR_HELP_DEFAULT_VALUE, FIELD_PROPERTY_COLUMNNAME);
            m_xColumnName->set_max_length(nMax);
            m_xColumnName->setCheck( isSQL92CheckEnabled(getConnection()) );
        }

        InitializeControl(m_xColumnName->GetWidget(),HID_TAB_ENT_COLUMNNAME);
        m_xColumnName->show();
        break;
    case tpNumType:
        if (m_xNumType)
            return;
        m_nPos++;
        m_xNumTypeText = m_xBuilder->weld_label("NumTypeText");
        m_xNumTypeText->show();
        m_xNumType = std::make_unique<OPropListBoxCtrl>(
                m_xBuilder->weld_combo_box("NumType"), STR_HELP_NUMERIC_TYPE, FIELD_PROPERTY_NUMTYPE);
        m_xNumType->append_text("Byte");
        m_xNumType->append_text("SmallInt");
        m_xNumType->append_text("Integer");
        m_xNumType->append_text("Single");
        m_xNumType->append_text("Double");
        m_xNumType->set_active(2);
        InitializeControl(m_xNumType.get(),HID_TAB_ENT_NUMTYP, true);
        m_xNumType->show();
        break;

    case tpLength:
        if (m_xLength)
            return;
        m_nPos++;
        m_xLengthText = m_xBuilder->weld_label("LengthText");
        m_xLengthText->show();
        m_xLength = CreateNumericControl("Length", STR_HELP_LENGTH, FIELD_PROPERTY_LENGTH,HID_TAB_ENT_LEN);
        break;

    case tpScale:
        if (m_xScale)
            return;
        m_nPos++;
        m_xScaleText = m_xBuilder->weld_label("ScaleText");
        m_xScaleText->show();
        m_xScale = CreateNumericControl("Scale", STR_HELP_SCALE, FIELD_PROPERTY_SCALE,HID_TAB_ENT_SCALE);
        break;

    case tpFormat:
        if (!m_xFormat)
        {
            m_nPos++;
            m_xFormatText = m_xBuilder->weld_label("FormatTextText");
            m_xFormatText->show();

            m_xFormatSample = std::make_unique<OPropEditCtrl>(
                    m_xBuilder->weld_entry("FormatText"), STR_HELP_FORMAT_CODE, -1);
            m_xFormatSample->set_editable(false);
            m_xFormatSample->set_sensitive(false);
            InitializeControl(m_xFormatSample->GetWidget(),HID_TAB_ENT_FORMAT_SAMPLE);
            m_xFormatSample->show();

            m_xFormat = m_xBuilder->weld_button("FormatButton");
            m_xFormat->connect_clicked( LINK( this, OFieldDescControl, FormatClickHdl ) );
            InitializeControl(m_xFormat.get(),HID_TAB_ENT_FORMAT);
            m_xFormat->show();
        }

        UpdateFormatSample(pActFieldDescr);
        break;
    case tpBoolDefault:
        if (m_xBoolDefault)
            return;

        m_nPos++;
        m_xBoolDefaultText = m_xBuilder->weld_label("BoolDefaultText");
        m_xBoolDefaultText->show();
        m_xBoolDefault = std::make_unique<OPropListBoxCtrl>(
                m_xBuilder->weld_combo_box("BoolDefault"), STR_HELP_BOOL_DEFAULT,
                FIELD_PROPERTY_BOOL_DEFAULT);
        m_xBoolDefault->append_text(DBA_RES(STR_VALUE_NONE));
        m_xBoolDefault->append_text(aYes);
        m_xBoolDefault->append_text(aNo);
        InitializeControl(m_xBoolDefault->GetWidget(),HID_TAB_ENT_BOOL_DEFAULT);
        m_xBoolDefault->show();
        break;
    }
}

void OFieldDescControl::InitializeControl(OPropListBoxCtrl* _pControl,const OString& _sHelpId,bool _bAddChangeHandler)
{
    if ( _bAddChangeHandler )
        _pControl->GetComboBox().connect_changed(LINK(this,OFieldDescControl,ChangeHdl));

    InitializeControl(_pControl->GetWidget(), _sHelpId);
}

void OFieldDescControl::InitializeControl(weld::Widget* pControl,const OString& _sHelpId)
{
    pControl->set_help_id(_sHelpId);
    pControl->connect_focus_in(LINK(this, OFieldDescControl, OnControlFocusGot));
    pControl->connect_focus_out(LINK(this, OFieldDescControl, OnControlFocusLost));

    if (dynamic_cast<weld::Entry*>(pControl))
    {
        int nWidthRequest = Application::GetDefaultDevice()->LogicToPixel(Size(m_nEditWidth, 0), MapMode(MapUnit::MapAppFont)).Width();
        pControl->set_size_request(nWidthRequest, -1);
    }
}

std::unique_ptr<OPropNumericEditCtrl> OFieldDescControl::CreateNumericControl(const OString& rId, const char* pHelpId, short _nProperty, const OString& _sHelpId)
{
    auto xControl = std::make_unique<OPropNumericEditCtrl>(
            m_xBuilder->weld_spin_button(rId), pHelpId, _nProperty);
    xControl->set_digits(0);
    xControl->set_range(0, 0x7FFFFFFF);   // Should be changed outside, if needed
    xControl->show();

    InitializeControl(xControl->GetWidget(),_sHelpId);

    return xControl;
}

void OFieldDescControl::DeactivateAggregate( EControlType eType )
{
    m_pLastFocusWindow = nullptr;
    // Destroy Controls
    switch( eType )
    {
    case tpDefault:
        lcl_HideAndDeleteControl(m_nPos,m_xDefault,m_xDefaultText);
        break;

    case tpAutoIncrementValue:
        lcl_HideAndDeleteControl(m_nPos,m_xAutoIncrementValue,m_xAutoIncrementValueText);
        break;

    case tpColumnName:
        lcl_HideAndDeleteControl(m_nPos,m_xColumnName,m_xColumnNameText);
        break;

    case tpType:
        lcl_HideAndDeleteControl(m_nPos,m_xType,m_xTypeText);
        break;

    case tpAutoIncrement:
        lcl_HideAndDeleteControl(m_nPos,m_xAutoIncrement,m_xAutoIncrementText);
        break;

    case tpRequired:
        lcl_HideAndDeleteControl(m_nPos,m_xRequired,m_xRequiredText);
        break;

    case tpTextLen:
        lcl_HideAndDeleteControl(m_nPos,m_xTextLen,m_xTextLenText);
        break;

    case tpNumType:
        lcl_HideAndDeleteControl(m_nPos,m_xNumType,m_xNumTypeText);
        break;

    case tpLength:
        lcl_HideAndDeleteControl(m_nPos,m_xLength,m_xLengthText);
        break;

    case tpScale:
        lcl_HideAndDeleteControl(m_nPos,m_xScale,m_xScaleText);
        break;

    case tpFormat:
        // TODO: we have to check if we have to increment m_nPos again
        lcl_HideAndDeleteControl(m_nPos,m_xFormat,m_xFormatText);
        if (m_xFormatSample)
        {
            m_xFormatSample->hide();
            m_xFormatSample.reset();
        }
        break;
    case tpBoolDefault:
        lcl_HideAndDeleteControl(m_nPos,m_xBoolDefault,m_xBoolDefaultText);
        break;
    }
}

void OFieldDescControl::DisplayData(OFieldDescription* pFieldDescr )
{
    pActFieldDescr = pFieldDescr;
    if(!pFieldDescr)
    {
        if (m_pHelp)
            m_pHelp->SetHelpText( OUString() );
        DeactivateAggregate( tpDefault );
        DeactivateAggregate( tpRequired );
        DeactivateAggregate( tpTextLen );
        DeactivateAggregate( tpNumType );
        DeactivateAggregate( tpScale );
        DeactivateAggregate( tpLength );
        DeactivateAggregate( tpFormat );
        DeactivateAggregate( tpAutoIncrement );
        DeactivateAggregate( tpBoolDefault );
        DeactivateAggregate( tpColumnName );
        DeactivateAggregate( tpType );
        DeactivateAggregate( tpAutoIncrementValue );
        m_pPreviousType = TOTypeInfoSP();
        // Reset the saved focus' pointer
        m_pLastFocusWindow = nullptr;
        return;
    }

    TOTypeInfoSP pFieldType(pFieldDescr->getTypeInfo());

    ActivateAggregate( tpColumnName );
    ActivateAggregate( tpType );

    OSL_ENSURE(pFieldType,"We need a type information here!");
    // If the type has changed, substitute Controls
    if( m_pPreviousType != pFieldType )
    {
        // Reset the saved focus' pointer
        m_pLastFocusWindow = nullptr;

        // Controls, which must NOT be displayed again
        DeactivateAggregate( tpNumType );

        // determine which controls we should show and which not

        // 1. the required control
        if ( pFieldType->bNullable )
            ActivateAggregate( tpRequired );
        else
            DeactivateAggregate( tpRequired );

        // 2. the autoincrement
        if ( pFieldType->bAutoIncrement )
        {
            DeactivateAggregate( tpRequired );
            DeactivateAggregate( tpDefault );
            ActivateAggregate( tpAutoIncrement );
            ActivateAggregate( tpAutoIncrementValue );
        }
        else
        {
            DeactivateAggregate( tpAutoIncrement );
            DeactivateAggregate( tpAutoIncrementValue );
            if(pFieldType->bNullable)
                ActivateAggregate( tpRequired );
            else
                DeactivateAggregate( tpRequired );
            ActivateAggregate( tpDefault );
        }
        // 3. the scale and precision
        if (pFieldType->nPrecision)
        {
            ActivateAggregate( tpLength );
            m_xLength->set_max(std::max<sal_Int32>(pFieldType->nPrecision,pFieldDescr->GetPrecision()));
            m_xLength->set_editable(!pFieldType->aCreateParams.isEmpty());
        }
        else
            DeactivateAggregate( tpLength );

        if (pFieldType->nMaximumScale)
        {
            ActivateAggregate( tpScale );
            m_xScale->set_range(pFieldType->nMinimumScale,
                                std::max<sal_Int32>(pFieldType->nMaximumScale,pFieldDescr->GetScale()));
            m_xScale->set_editable(!pFieldType->aCreateParams.isEmpty() && pFieldType->aCreateParams != "PRECISION");
        }
        else
            DeactivateAggregate( tpScale );

        // and now look for type specific things
        switch( pFieldType->nType )
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                DeactivateAggregate( tpLength );
                DeactivateAggregate( tpBoolDefault );

                ActivateAggregate( tpDefault );
                ActivateAggregate( tpFormat );
                if (pFieldType->nPrecision)
                {
                    ActivateAggregate( tpTextLen );
                    m_xTextLen->set_max(std::max<sal_Int32>(pFieldType->nPrecision,pFieldDescr->GetPrecision()));
                    m_xTextLen->set_editable(!pFieldType->aCreateParams.isEmpty());
                }
                else
                    DeactivateAggregate( tpTextLen );
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
                DeactivateAggregate( tpLength ); // we don't need a length for date types
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpBoolDefault );

                ActivateAggregate( tpDefault );
                ActivateAggregate( tpFormat );
                break;
            case DataType::BIT:
                if ( !pFieldType->aCreateParams.isEmpty() )
                {
                    DeactivateAggregate( tpFormat );
                    DeactivateAggregate( tpTextLen );
                    DeactivateAggregate( tpBoolDefault );
                    break;
                }
                [[fallthrough]];
            case DataType::BOOLEAN:
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpFormat );
                DeactivateAggregate( tpDefault );

                ActivateAggregate( tpBoolDefault );
                break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::BIGINT:
            case DataType::FLOAT:
            case DataType::DOUBLE:
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::REAL:
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpBoolDefault );

                ActivateAggregate( tpFormat );
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
                DeactivateAggregate( tpDefault );
                DeactivateAggregate( tpRequired );
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpBoolDefault );

                ActivateAggregate( tpFormat );
                break;
            case DataType::LONGVARBINARY:
            case DataType::SQLNULL:
            case DataType::OBJECT:
            case DataType::DISTINCT:
            case DataType::STRUCT:
            case DataType::ARRAY:
            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::REF:
            case DataType::OTHER:
                DeactivateAggregate( tpFormat );
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpBoolDefault );

                break;
            default:
                OSL_FAIL("Unknown type");
        }
        m_pPreviousType = pFieldType;
    }

    if (pFieldDescr->IsPrimaryKey())
    {
        DeactivateAggregate(tpRequired);
    }
    else if (!m_xAutoIncrement && pFieldType)
    {
        if (pFieldType->bNullable)
            ActivateAggregate(tpRequired);
        else
            DeactivateAggregate(tpRequired);
    }
    // Initialize Controls
    if (m_xAutoIncrement)
    {
        if ( pFieldDescr->IsAutoIncrement() )
        {
            m_xAutoIncrement->set_active(0); // yes
            ActivateAggregate( tpAutoIncrementValue );
            if (m_xAutoIncrementValue)
                m_xAutoIncrementValue->set_text(pFieldDescr->GetAutoIncrementValue());
            DeactivateAggregate( tpRequired );
            DeactivateAggregate( tpDefault );
        }
        else
        {
            // disable autoincrement value because it should only be visible when autoincrement is to true
            DeactivateAggregate( tpAutoIncrementValue );
            m_xAutoIncrement->set_active(1);        // no
            ActivateAggregate( tpDefault );
            // Affects pRequired
            if(!pFieldDescr->IsPrimaryKey())
                ActivateAggregate( tpRequired );
        }
    }

    if (m_xDefault)
    {
        m_xDefault->set_text(getControlDefault(pFieldDescr));
        m_xDefault->save_value();
    }

    if (m_xBoolDefault)
    {
        // If m_xRequired = sal_True then the sal_Bool field must NOT contain <<none>>
        OUString sValue;
        pFieldDescr->GetControlDefault() >>= sValue;
        OUString sDef = BoolStringUI(sValue);

        // Make sure that <<none>> is only present if the field can be NULL
        if ( ( pFieldType && !pFieldType->bNullable ) || !pFieldDescr->IsNullable() )
        {
            pFieldDescr->SetIsNullable(ColumnValue::NO_NULLS); // The type says so

            m_xBoolDefault->remove_text(DBA_RES(STR_VALUE_NONE));
            if ( sDef != aYes && sDef != aNo )
                m_xBoolDefault->set_active(1);  // No as a default
            else
                m_xBoolDefault->set_active_text(sDef);

            pFieldDescr->SetControlDefault(makeAny(BoolStringPersistent(m_xBoolDefault->get_active_text())));
        }
        else if (m_xBoolDefault->get_count() < 3)
        {
            m_xBoolDefault->append_text(DBA_RES(STR_VALUE_NONE));
            m_xBoolDefault->set_active_text(sDef);
        }
        else
            m_xBoolDefault->set_active_text(sDef);
    }

    if (m_xRequired)
    {
        if( pFieldDescr->IsNullable() )
            m_xRequired->set_active(1); // no
        else
            m_xRequired->set_active(0); // yes
    }

    if (m_xTextLen)
    {
        m_xTextLen->set_text(OUString::number(pFieldDescr->GetPrecision()));
        m_xTextLen->save_value();
    }

    if( m_xNumType )
    {
        OSL_FAIL("OFieldDescControl::DisplayData: invalid num type!");
    }

    if (m_xLength)
        m_xLength->set_text(OUString::number(pFieldDescr->GetPrecision()));

    if (m_xScale)
        m_xScale->set_text(OUString::number(pFieldDescr->GetScale()));

    if (m_xFormat)
        UpdateFormatSample(pFieldDescr);

    if (m_xColumnName)
        m_xColumnName->set_text(pFieldDescr->GetName());

    if (m_xType)
    {
        sal_Int32 nPos = pFieldType ? m_xType->find_text(pFieldDescr->getTypeInfo()->aUIName) : -1;
        if (nPos == -1)
        {
            const OTypeInfoMap* pMap = getTypeInfo();
            OTypeInfoMap::const_iterator aIter = pMap->find(pFieldType ? pFieldDescr->getTypeInfo()->nType : pFieldDescr->GetType());
            if(aIter == pMap->end() && !pMap->empty())
            {
                aIter = pMap->begin();
                if(pFieldDescr->GetPrecision() > aIter->second->nPrecision)
                    pFieldDescr->SetPrecision(aIter->second->nPrecision);
                if(pFieldDescr->GetScale() > aIter->second->nMaximumScale)
                    pFieldDescr->SetScale(0);
                if(!aIter->second->bNullable && pFieldDescr->IsNullable())
                    pFieldDescr->SetIsNullable(ColumnValue::NO_NULLS);
                if(!aIter->second->bAutoIncrement && pFieldDescr->IsAutoIncrement())
                    pFieldDescr->SetAutoIncrement(false);
            }
            if ( aIter != pMap->end() )
            {
                pFieldDescr->SetType(aIter->second);
            }
        }
        m_xType->set_active_text(pFieldDescr->getTypeInfo()->aUIName);
    }

    // Enable/disable Controls
    bool bRead(IsReadOnly());

    SetReadOnly( bRead );
}

IMPL_LINK(OFieldDescControl, OnControlFocusGot, weld::Widget&, rControl, void )
{
    OUString strHelpText;

    if (m_xTextLen && &rControl == m_xTextLen->GetWidget())
    {
        m_xTextLen->save_value();
        strHelpText = m_xTextLen->GetHelp();
    }
    else if (m_xLength && &rControl == m_xLength->GetWidget())
    {
        m_xLength->save_value();
        strHelpText = m_xLength->GetHelp();
    }
    else if (m_xScale && &rControl == m_xScale->GetWidget())
    {
        m_xScale->save_value();
        strHelpText = m_xScale->GetHelp();
    }
    else if (m_xColumnName && &rControl == m_xColumnName->GetWidget())
    {
        m_xColumnName->save_value();
        strHelpText = m_xColumnName->GetHelp();
    }
    else if (m_xDefault && &rControl == m_xDefault->GetWidget())
    {
        m_xDefault->save_value();
        strHelpText = m_xDefault->GetHelp();
    }
    else if (m_xFormatSample && &rControl == m_xFormatSample->GetWidget())
    {
        m_xFormatSample->save_value();
        strHelpText = m_xFormatSample->GetHelp();
    }
    else if (m_xAutoIncrementValue && &rControl == m_xAutoIncrementValue->GetWidget())
    {
        m_xAutoIncrementValue->save_value();
        strHelpText = m_xAutoIncrementValue->GetHelp();
    }
    else if (m_xRequired && &rControl == m_xRequired->GetWidget())
    {
        m_xRequired->save_value();
        strHelpText = m_xRequired->GetHelp();
    }
    else if (m_xNumType && &rControl == m_xNumType->GetWidget())
    {
        m_xNumType->save_value();
        strHelpText = m_xNumType->GetHelp();
    }
    else if (m_xAutoIncrement && &rControl == m_xAutoIncrement->GetWidget())
    {
        m_xAutoIncrement->save_value();
        strHelpText = m_xAutoIncrement->GetHelp();
    }
    else if (m_xBoolDefault && &rControl == m_xBoolDefault->GetWidget())
    {
        m_xBoolDefault->save_value();
        strHelpText = m_xBoolDefault->GetHelp();
    }
    else if (m_xType && &rControl == m_xType->GetWidget())
    {
        m_xType->save_value();
        strHelpText = m_xType->GetHelp();
    }
    else if (m_xFormat && &rControl == m_xFormat.get())
        strHelpText = DBA_RES(STR_HELP_FORMAT_BUTTON);

    if (!strHelpText.isEmpty() && m_pHelp)
        m_pHelp->SetHelpText(strHelpText);

    m_pActFocusWindow = &rControl;

    m_aControlFocusIn.Call(rControl);
}

IMPL_LINK(OFieldDescControl, OnControlFocusLost, weld::Widget&, rControl, void )
{
    if (m_xLength && &rControl == m_xLength->GetWidget() && m_xLength->get_value_changed_from_saved())
        CellModified(-1, m_xLength->GetPos());
    else if (m_xTextLen && &rControl == m_xTextLen->GetWidget() && m_xTextLen->get_value_changed_from_saved())
        CellModified(-1, m_xTextLen->GetPos());
    else if (m_xScale && &rControl == m_xScale->GetWidget() && m_xScale->get_value_changed_from_saved())
        CellModified(-1, m_xScale->GetPos());
    else if (m_xColumnName && &rControl == m_xColumnName->GetWidget() && m_xColumnName->get_value_changed_from_saved())
        CellModified(-1, m_xColumnName->GetPos());
    else if (m_xDefault && &rControl == m_xDefault->GetWidget() && m_xDefault->get_value_changed_from_saved())
        CellModified(-1, m_xDefault->GetPos());
    else if (m_xFormatSample && &rControl == m_xFormatSample->GetWidget() && m_xFormatSample->get_value_changed_from_saved())
        CellModified(-1, m_xFormatSample->GetPos());
    else if (m_xAutoIncrementValue && &rControl == m_xAutoIncrementValue->GetWidget() && m_xAutoIncrementValue->get_value_changed_from_saved())
        CellModified(-1, m_xAutoIncrementValue->GetPos());
    else if (m_xRequired && &rControl == m_xRequired->GetWidget() && m_xRequired->get_value_changed_from_saved())
        CellModified(-1, m_xRequired->GetPos());
    else if (m_xNumType && &rControl == m_xNumType->GetWidget() && m_xNumType->get_value_changed_from_saved())
        CellModified(-1, m_xNumType->GetPos());
    else if (m_xAutoIncrement && &rControl == m_xAutoIncrement->GetWidget() && m_xAutoIncrement->get_value_changed_from_saved())
        CellModified(-1, m_xAutoIncrement->GetPos());
    else if (m_xBoolDefault && &rControl == m_xBoolDefault->GetWidget() && m_xBoolDefault->get_value_changed_from_saved())
        CellModified(-1, m_xBoolDefault->GetPos());
    else if (m_xType && &rControl == m_xType->GetWidget() && m_xType->get_value_changed_from_saved())
        CellModified(-1, m_xType->GetPos());
    else if (m_xDefault && &rControl == m_xDefault->GetWidget())
        UpdateFormatSample(pActFieldDescr);

    implFocusLost(&rControl);
}

void OFieldDescControl::SaveData( OFieldDescription* pFieldDescr )
{
    if( !pFieldDescr )
        return;

    // Read out Controls
    OUString sDefault;
    if (m_xDefault)
    {
        // tdf#138409 take the control default in the UI Locale format, e.g. 12,34 and return a string
        // suitable as the database default, e.g. 12.34
        sDefault = CanonicalizeToControlDefault(pFieldDescr, m_xDefault->get_text());
    }
    else if (m_xBoolDefault)
    {
        sDefault = BoolStringPersistent(m_xBoolDefault->get_active_text());
    }

    if ( !sDefault.isEmpty() )
        pFieldDescr->SetControlDefault(makeAny(sDefault));
    else
        pFieldDescr->SetControlDefault(Any());

    if((m_xRequired && m_xRequired->get_active() == 0) || pFieldDescr->IsPrimaryKey() || (m_xBoolDefault && m_xBoolDefault->get_count() == 2))  // yes
        pFieldDescr->SetIsNullable( ColumnValue::NO_NULLS );
    else
        pFieldDescr->SetIsNullable( ColumnValue::NULLABLE );

    if (m_xAutoIncrement)
        pFieldDescr->SetAutoIncrement(m_xAutoIncrement->get_active() == 0);

    if( m_xTextLen )
        pFieldDescr->SetPrecision( static_cast<sal_Int32>(m_xTextLen->get_value()) );
    else if (m_xLength)
        pFieldDescr->SetPrecision( static_cast<sal_Int32>(m_xLength->get_value()) );
    if (m_xScale)
        pFieldDescr->SetScale( static_cast<sal_Int32>(m_xScale->get_value()) );

    if (m_xColumnName)
        pFieldDescr->SetName(m_xColumnName->get_text());

    if (m_xAutoIncrementValue && isAutoIncrementValueEnabled())
        pFieldDescr->SetAutoIncrementValue(m_xAutoIncrementValue->get_text());
}

void OFieldDescControl::UpdateFormatSample(OFieldDescription const * pFieldDescr)
{
    if (pFieldDescr && m_xFormatSample)
        m_xFormatSample->set_text(getControlDefault(pFieldDescr,false));
}

void OFieldDescControl::GrabFocus()
{
    m_xContainer->grab_focus();

    // Set the Focus to the Control that has been active last
    if (m_pLastFocusWindow)
    {
        m_pLastFocusWindow->grab_focus();
        m_pLastFocusWindow = nullptr;
    }
}

void OFieldDescControl::implFocusLost(weld::Widget* _pWhich)
{
    // Remember the active Control
    if (!m_pLastFocusWindow)
        m_pLastFocusWindow = _pWhich;

    // Reset HelpText
    if (m_pHelp && !m_pHelp->HasFocus())
        m_pHelp->SetHelpText( OUString() );
}

IMPL_LINK_NOARG(OFieldDescControl, HelpFocusOut, weld::Widget&, void)
{
    m_pHelp->SetHelpText(OUString());
}

bool OFieldDescControl::IsFocusInEditableWidget() const
{
    if (m_xDefault && m_pActFocusWindow == m_xDefault->GetWidget())
        return true;
    if (m_xFormatSample && m_pActFocusWindow == m_xFormatSample->GetWidget())
        return true;
    if (m_xTextLen && m_pActFocusWindow == m_xTextLen->GetWidget())
        return true;
    if (m_xLength && m_pActFocusWindow == m_xLength->GetWidget())
        return true;
    if (m_xScale && m_pActFocusWindow == m_xScale->GetWidget())
        return true;
    if (m_xColumnName && m_pActFocusWindow == m_xColumnName->GetWidget())
        return true;
    if (m_xAutoIncrementValue && m_pActFocusWindow == m_xAutoIncrementValue->GetWidget())
        return true;
    return false;
}

bool OFieldDescControl::HasChildPathFocus() const
{
    return m_xContainer && m_xContainer->has_child_focus();
}

bool OFieldDescControl::isCopyAllowed()
{
    int nStartPos, nEndPos;
    bool bAllowed = (m_pActFocusWindow != nullptr) && IsFocusInEditableWidget() &&
                        dynamic_cast<weld::Entry&>(*m_pActFocusWindow).get_selection_bounds(nStartPos, nEndPos);
    return bAllowed;
}

bool OFieldDescControl::isCutAllowed()
{
    int nStartPos, nEndPos;
    bool bAllowed = (m_pActFocusWindow != nullptr) && IsFocusInEditableWidget() &&
                        dynamic_cast<weld::Entry&>(*m_pActFocusWindow).get_selection_bounds(nStartPos, nEndPos);
    return bAllowed;
}

bool OFieldDescControl::isPasteAllowed()
{
    bool bAllowed = (m_pActFocusWindow != nullptr) && IsFocusInEditableWidget();
    if ( bAllowed )
    {
        TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromClipboard(GetSystemClipboard()));
        bAllowed = aTransferData.HasFormat(SotClipboardFormatId::STRING);
    }
    return bAllowed;
}

void OFieldDescControl::cut()
{
    if (isCutAllowed())
        dynamic_cast<weld::Entry&>(*m_pActFocusWindow).cut_clipboard();
}

void OFieldDescControl::copy()
{
    if (isCopyAllowed()) // this only checks if the focus window is valid
        dynamic_cast<weld::Entry&>(*m_pActFocusWindow).copy_clipboard();
}

void OFieldDescControl::paste()
{
    if (m_pActFocusWindow) // this only checks if the focus window is valid
        dynamic_cast<weld::Entry&>(*m_pActFocusWindow).paste_clipboard();
}

bool OFieldDescControl::isTextFormat(const OFieldDescription* _pFieldDescr, sal_uInt32& _nFormatKey) const
{
    _nFormatKey = _pFieldDescr->GetFormatKey();
    bool bTextFormat = true;

    try
    {
        if (!_nFormatKey)
        {
            Reference< css::util::XNumberFormatTypes> xNumberTypes(GetFormatter()->getNumberFormatsSupplier()->getNumberFormats(),UNO_QUERY);
            OSL_ENSURE(xNumberTypes.is(),"XNumberFormatTypes is null!");

            _nFormatKey = ::dbtools::getDefaultNumberFormat( _pFieldDescr->GetType(),
                _pFieldDescr->GetScale(),
                _pFieldDescr->IsCurrency(),
                xNumberTypes,
                GetLocale());
        }
        sal_Int32 nNumberFormat = ::comphelper::getNumberFormatType(GetFormatter(),_nFormatKey);
        bTextFormat = (nNumberFormat == css::util::NumberFormat::TEXT);
    }
    catch(const Exception&)
    {

    }

    return bTextFormat;
}

OUString OFieldDescControl::getControlDefault( const OFieldDescription* _pFieldDescr, bool _bCheck) const
{
    OUString sDefault;
    bool bCheck = !_bCheck || _pFieldDescr->GetControlDefault().hasValue();
    if ( bCheck )
    {
        try
        {
            double nValue = 0.0;
            sal_uInt32 nFormatKey;
            bool bTextFormat = isTextFormat(_pFieldDescr,nFormatKey);
            if ( _pFieldDescr->GetControlDefault() >>= sDefault )
            {
                if ( !bTextFormat )
                {
                    if ( !sDefault.isEmpty() )
                    {
                        try
                        {
                            nValue = GetFormatter()->convertStringToNumber(nFormatKey,sDefault);
                        }
                        catch(const Exception&)
                        {
                            return OUString(); // return empty string for format example
                        }
                    }
                }
            }
            else
                _pFieldDescr->GetControlDefault() >>= nValue;

            Reference< css::util::XNumberFormatter> xNumberFormatter = GetFormatter();
            Reference<XPropertySet> xFormSet = xNumberFormatter->getNumberFormatsSupplier()->getNumberFormats()->getByKey(nFormatKey);
            OSL_ENSURE(xFormSet.is(),"XPropertySet is null!");
            OUString sFormat;
            xFormSet->getPropertyValue("FormatString") >>= sFormat;

            if ( !bTextFormat )
            {
                Locale aLocale;
                ::comphelper::getNumberFormatProperty(xNumberFormatter,nFormatKey,"Locale") >>= aLocale;

                sal_Int32 nNumberFormat = ::comphelper::getNumberFormatType(xNumberFormatter,nFormatKey);
                if(     (nNumberFormat & css::util::NumberFormat::DATE)    == css::util::NumberFormat::DATE
                    || (nNumberFormat & css::util::NumberFormat::DATETIME) == css::util::NumberFormat::DATETIME )
                {
                    nValue = DBTypeConversion::toNullDate(DBTypeConversion::getNULLDate(xNumberFormatter->getNumberFormatsSupplier()),nValue);
                }

                Reference< css::util::XNumberFormatPreviewer> xPreviewer(xNumberFormatter,UNO_QUERY);
                OSL_ENSURE(xPreviewer.is(),"XNumberFormatPreviewer is null!");
                sDefault = xPreviewer->convertNumberToPreviewString(sFormat,nValue,aLocale,true);
            }
            else if ( !(_bCheck && sDefault.isEmpty()) )
                sDefault = xNumberFormatter->formatString(nFormatKey, sDefault.isEmpty() ? sFormat : sDefault);
        }
        catch(const Exception&)
        {

        }
    }

    return sDefault;
}

// tdf#138409 intended to be effectively the reverse of getControlDefault to
// turn a user's possibly 12,34 format into 12.34 format for numerical types
OUString OFieldDescControl::CanonicalizeToControlDefault(const OFieldDescription* pFieldDescr, const OUString& rDefault) const
{
    if (rDefault.isEmpty())
        return rDefault;

    bool bIsNumericalType = false;
    switch (pFieldDescr->GetType())
    {
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
        case DataType::BIGINT:
        case DataType::FLOAT:
        case DataType::REAL:
        case DataType::DOUBLE:
        case DataType::NUMERIC:
        case DataType::DECIMAL:
            bIsNumericalType = true;
            break;
    }

    if (!bIsNumericalType)
        return rDefault;

    try
    {
        sal_uInt32 nFormatKey;
        bool bTextFormat = isTextFormat(pFieldDescr, nFormatKey);
        if (bTextFormat)
            return rDefault;
        double nValue = GetFormatter()->convertStringToNumber(nFormatKey, rDefault);
        return OUString::number(nValue);
    }
    catch(const Exception&)
    {
    }

    return rDefault;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
