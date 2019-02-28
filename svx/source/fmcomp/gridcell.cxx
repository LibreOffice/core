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
#include <sal/macros.h>
#include <sal/log.hxx>
#include <fmprop.hxx>
#include <svx/strings.hrc>
#include <svx/fmtools.hxx>
#include <gridcell.hxx>
#include <gridcols.hxx>
#include <sdbdatacolumn.hxx>

#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/Date.hpp>

#include <comphelper/numbers.hxx>
#include <comphelper/property.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <i18nlangtag/lang.h>

#include <rtl/math.hxx>
#include <svtools/calendar.hxx>
#include <vcl/fmtfield.hxx>
#include <svl/numuno.hxx>
#include <svl/zforlist.hxx>
#include <svtools/svmedit.hxx>
#include <svx/dialmgr.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/longcurr.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/sqlnode.hxx>

#include <math.h>

using namespace ::connectivity;
using namespace ::svxform;
using namespace ::comphelper;
using namespace ::svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using namespace ::dbtools::DBTypeConversion;
using namespace ::dbtools;

using ::com::sun::star::util::XNumberFormatter;

const char INVALIDTEXT[] = "###";
const char OBJECTTEXT[] = "<OBJECT>";


//= helper

namespace
{
    LineEnd getModelLineEndSetting( const Reference< XPropertySet >& _rxModel )
    {
        LineEnd eFormat = LINEEND_LF;

        try
        {
            sal_Int16 nLineEndFormat = awt::LineEndFormat::LINE_FEED;

            Reference< XPropertySetInfo > xPSI;
            if ( _rxModel.is() )
                xPSI = _rxModel->getPropertySetInfo();

            OSL_ENSURE( xPSI.is(), "getModelLineEndSetting: invalid column model!" );
            if ( xPSI.is() && xPSI->hasPropertyByName( FM_PROP_LINEENDFORMAT ) )
            {
                OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_LINEENDFORMAT ) >>= nLineEndFormat );

                switch ( nLineEndFormat )
                {
                case awt::LineEndFormat::CARRIAGE_RETURN:            eFormat = LINEEND_CR; break;
                case awt::LineEndFormat::LINE_FEED:                  eFormat = LINEEND_LF; break;
                case awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED:  eFormat = LINEEND_CRLF; break;
                default:
                    OSL_FAIL( "getModelLineEndSetting: what's this?" );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
            OSL_FAIL( "getModelLineEndSetting: caught an exception!" );
        }
        return eFormat;
    }
}


//= DbGridColumn


CellControllerRef DbGridColumn::s_xEmptyController;


void DbGridColumn::CreateControl(sal_Int32 _nFieldPos, const Reference< css::beans::XPropertySet >& xField, sal_Int32 nTypeId)
{
    Clear();

    m_nTypeId = static_cast<sal_Int16>(nTypeId);
    if (xField != m_xField)
    {
        // initial setting
        m_xField = xField;
        xField->getPropertyValue(FM_PROP_FORMATKEY) >>= m_nFormatKey;
        m_nFieldPos   = static_cast<sal_Int16>(_nFieldPos);
        m_bReadOnly   = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISREADONLY));
        m_bAutoValue  = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_AUTOINCREMENT));
        m_nFieldType  = static_cast<sal_Int16>(::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FIELDTYPE)));

        switch (m_nFieldType)
        {
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BIT:
            case DataType::BOOLEAN:
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::BIGINT:
            case DataType::FLOAT:
            case DataType::REAL:
            case DataType::DOUBLE:
            case DataType::NUMERIC:
            case DataType::DECIMAL:
                m_nAlign = css::awt::TextAlign::RIGHT;
                m_bNumeric = true;
                break;
            default:
                m_nAlign = css::awt::TextAlign::LEFT;
                break;
        }
    }

    std::unique_ptr<DbCellControl> pCellControl;
    if (m_rParent.IsFilterMode())
    {
        pCellControl.reset(new DbFilterField(m_rParent.getContext(),*this));
    }
    else
    {

        switch (nTypeId)
        {
            case TYPE_CHECKBOX: pCellControl.reset(new DbCheckBox(*this));   break;
            case TYPE_COMBOBOX: pCellControl.reset(new DbComboBox(*this)); break;
            case TYPE_CURRENCYFIELD: pCellControl.reset(new DbCurrencyField(*this)); break;
            case TYPE_DATEFIELD: pCellControl.reset(new DbDateField(*this)); break;
            case TYPE_LISTBOX: pCellControl.reset(new DbListBox(*this)); break;
            case TYPE_NUMERICFIELD: pCellControl.reset(new DbNumericField(*this)); break;
            case TYPE_PATTERNFIELD: pCellControl.reset(new DbPatternField( *this, m_rParent.getContext() )); break;
            case TYPE_TEXTFIELD: pCellControl.reset(new DbTextField(*this)); break;
            case TYPE_TIMEFIELD: pCellControl.reset(new DbTimeField(*this)); break;
            case TYPE_FORMATTEDFIELD: pCellControl.reset(new DbFormattedField(*this)); break;
            default:
                OSL_FAIL("DbGridColumn::CreateControl: Unknown Column");
                return;
        }

    }
    Reference< XRowSet >  xCur;
    if (m_rParent.getDataSource())
        xCur.set(Reference< XInterface >(*m_rParent.getDataSource()), UNO_QUERY);
        // TODO : the cursor wrapper should use an XRowSet interface, too

    pCellControl->Init( m_rParent.GetDataWindow(), xCur );

    // now create the control wrapper
    auto pTempCellControl = pCellControl.get();
    if (m_rParent.IsFilterMode())
        m_pCell = new FmXFilterCell(this, std::unique_ptr<DbFilterField>(static_cast<DbFilterField*>(pCellControl.release())));
    else
    {
        switch (nTypeId)
        {
            case TYPE_CHECKBOX: m_pCell = new FmXCheckBoxCell( this, std::move(pCellControl) );  break;
            case TYPE_LISTBOX: m_pCell = new FmXListBoxCell( this, std::move(pCellControl) );    break;
            case TYPE_COMBOBOX: m_pCell = new FmXComboBoxCell( this, std::move(pCellControl) );    break;
            default:
                m_pCell = new FmXEditCell( this, std::move(pCellControl) );
        }
    }
    m_pCell->init();

    impl_toggleScriptManager_nothrow( true );

    // only if we use have a bound field, we use a controller for displaying the
    // window in the grid
    if (m_xField.is())
        m_xController = pTempCellControl->CreateController();
}


void DbGridColumn::impl_toggleScriptManager_nothrow( bool _bAttach )
{
    try
    {
        Reference< container::XChild > xChild( m_xModel, UNO_QUERY_THROW );
        Reference< script::XEventAttacherManager > xManager( xChild->getParent(), UNO_QUERY_THROW );
        Reference< container::XIndexAccess > xContainer( xChild->getParent(), UNO_QUERY_THROW );

        sal_Int32 nIndexInParent( getElementPos( xContainer, m_xModel ) );

        Reference< XInterface > xCellInterface( *m_pCell, UNO_QUERY );
        if ( _bAttach )
            xManager->attach( nIndexInParent, xCellInterface, makeAny( xCellInterface ) );
        else
            xManager->detach( nIndexInParent, xCellInterface );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}

void DbGridColumn::UpdateFromField(const DbGridRow* pRow, const Reference< XNumberFormatter >& xFormatter)
{
    if (FmXFilterCell* pCell = dynamic_cast<FmXFilterCell*>(m_pCell.get()))
        pCell->Update();
    else if (pRow && pRow->IsValid() && m_nFieldPos >= 0 && m_pCell.is() && pRow->HasField(m_nFieldPos))
    {
        dynamic_cast<FmXDataCell&>(*m_pCell).UpdateFromField( pRow->GetField( m_nFieldPos ).getColumn(), xFormatter  );
    }
}

bool DbGridColumn::Commit()
{
    bool bResult = true;
    if (!m_bInSave && m_pCell.is())
    {
        m_bInSave = true;
        bResult = m_pCell->Commit();

        // store the data into the model
        FmXDataCell* pDataCell = dynamic_cast<FmXDataCell*>( m_pCell.get() );
        if (bResult && pDataCell)
        {
            Reference< css::form::XBoundComponent >  xComp(m_xModel, UNO_QUERY);
            if (xComp.is())
                bResult = xComp->commit();
        }
        m_bInSave = false;
    }
    return bResult;
}


DbGridColumn::~DbGridColumn()
{
    Clear();
}


void DbGridColumn::setModel(const css::uno::Reference< css::beans::XPropertySet >&  _xModel)
{
    if ( m_pCell.is() )
        impl_toggleScriptManager_nothrow( false );

    m_xModel = _xModel;

    if ( m_pCell.is() )
        impl_toggleScriptManager_nothrow( true );
}


void DbGridColumn::Clear()
{
    if ( m_pCell.is() )
    {
        impl_toggleScriptManager_nothrow( false );

        m_pCell->dispose();
        m_pCell.clear();
    }

    m_xController = nullptr;
    m_xField = nullptr;

    m_nFormatKey = 0;
    m_nFieldPos = -1;
    m_bReadOnly = true;
    m_bAutoValue = false;
    m_nFieldType = DataType::OTHER;
}


sal_Int16 DbGridColumn::SetAlignment(sal_Int16 _nAlign)
{
    if (_nAlign == -1)
    {   // 'Standard'
        if (m_xField.is())
        {
            sal_Int32 nType = 0;
            m_xField->getPropertyValue(FM_PROP_FIELDTYPE) >>= nType;

            switch (nType)
            {
                case DataType::NUMERIC:
                case DataType::DECIMAL:
                case DataType::DOUBLE:
                case DataType::REAL:
                case DataType::BIGINT:
                case DataType::INTEGER:
                case DataType::SMALLINT:
                case DataType::TINYINT:
                case DataType::DATE:
                case DataType::TIME:
                case DataType::TIMESTAMP:
                    _nAlign = css::awt::TextAlign::RIGHT;
                    break;
                case DataType::BIT:
                case DataType::BOOLEAN:
                    _nAlign = css::awt::TextAlign::CENTER;
                    break;
                default:
                    _nAlign = css::awt::TextAlign::LEFT;
                    break;
            }
        }
        else
            _nAlign = css::awt::TextAlign::LEFT;
    }

    m_nAlign = _nAlign;
    if (m_pCell.is() && m_pCell->isAlignedController())
        m_pCell->AlignControl(m_nAlign);

    return m_nAlign;
}


sal_Int16 DbGridColumn::SetAlignmentFromModel(sal_Int16 nStandardAlign)
{
    Any aAlign( m_xModel->getPropertyValue(FM_PROP_ALIGN));
    if (aAlign.hasValue())
    {
        sal_Int16 nTest = sal_Int16();
        if (aAlign >>= nTest)
            nStandardAlign = nTest;
    }
    return SetAlignment(nStandardAlign);
}


void DbGridColumn::setLock(bool _bLock)
{
    if (m_bLocked == _bLock)
        return;
    m_bLocked = _bLock;

    // is the column we represent active ?
    if (m_bHidden)
        return;     // no, it isn't (or at least it shouldn't be ...)

    if (m_rParent.GetCurColumnId() == m_nId)
    {
        m_rParent.DeactivateCell();
        m_rParent.ActivateCell(m_rParent.GetCurRow(), m_rParent.GetCurColumnId());
    }
}


OUString DbGridColumn::GetCellText(const DbGridRow* pRow, const Reference< XNumberFormatter >& xFormatter) const
{
    OUString aText;
    if (m_pCell.is() && dynamic_cast<const FmXFilterCell*>( m_pCell.get() ) !=  nullptr)
        return aText;

    if (!pRow || !pRow->IsValid())
        aText = INVALIDTEXT;
    else if (pRow->HasField(m_nFieldPos))
    {
        aText = GetCellText( pRow->GetField( m_nFieldPos ).getColumn(), xFormatter );
    }
    return aText;
}


OUString DbGridColumn::GetCellText(const Reference< css::sdb::XColumn >& xField, const Reference< XNumberFormatter >& xFormatter) const
{
    OUString aText;
    if (xField.is())
    {
        FmXTextCell* pTextCell = dynamic_cast<FmXTextCell*>( m_pCell.get() );
        if (pTextCell)
            aText = pTextCell->GetText(xField, xFormatter);
        else if (m_bObject)
            aText = OBJECTTEXT;
    }
    return aText;
}


Reference< css::sdb::XColumn >  DbGridColumn::GetCurrentFieldValue() const
{
    Reference< css::sdb::XColumn >  xField;
    const DbGridRowRef xRow = m_rParent.GetCurrentRow();
    if (xRow.is() && xRow->HasField(m_nFieldPos))
    {
        xField = xRow->GetField(m_nFieldPos).getColumn();
    }
    return xField;
}


void DbGridColumn::Paint(OutputDevice& rDev,
                         const tools::Rectangle& rRect,
                         const DbGridRow* pRow,
                         const Reference< XNumberFormatter >& xFormatter)
{
    bool bEnabled = ( rDev.GetOutDevType() != OUTDEV_WINDOW )
                ||  ( static_cast< vcl::Window& >( rDev ).IsEnabled() );

    FmXDataCell* pDataCell = dynamic_cast<FmXDataCell*>( m_pCell.get() );
    if (pDataCell)
    {
        if (!pRow || !pRow->IsValid())
        {
            DrawTextFlags nStyle = DrawTextFlags::Clip | DrawTextFlags::Center;
            if ( !bEnabled )
                nStyle |= DrawTextFlags::Disable;

            rDev.DrawText(rRect, OUString(INVALIDTEXT), nStyle);
        }
        else if (m_bAutoValue && pRow->IsNew())
        {
            DrawTextFlags nStyle = DrawTextFlags::Clip | DrawTextFlags::VCenter;
            if ( !bEnabled )
                nStyle |= DrawTextFlags::Disable;

            switch (GetAlignment())
            {
                case css::awt::TextAlign::RIGHT:
                    nStyle |= DrawTextFlags::Right;
                    break;
                case css::awt::TextAlign::CENTER:
                    nStyle |= DrawTextFlags::Center;
                    break;
                default:
                    nStyle |= DrawTextFlags::Left;
            }

            rDev.DrawText(rRect, SvxResId(RID_STR_AUTOFIELD), nStyle);
        }
        else if (pRow->HasField(m_nFieldPos))
        {
            pDataCell->PaintFieldToCell(rDev, rRect, pRow->GetField( m_nFieldPos ).getColumn(), xFormatter);
        }
    }
    else if (!m_pCell.is())
    {
        if (!pRow || !pRow->IsValid())
        {
            DrawTextFlags nStyle = DrawTextFlags::Clip | DrawTextFlags::Center;
            if ( !bEnabled )
                nStyle |= DrawTextFlags::Disable;

            rDev.DrawText(rRect, OUString(INVALIDTEXT), nStyle);
        }
        else if (pRow->HasField(m_nFieldPos) && m_bObject)
        {
            DrawTextFlags nStyle = DrawTextFlags::Clip | DrawTextFlags::Center;
            if ( !bEnabled )
                nStyle |= DrawTextFlags::Disable;
            rDev.DrawText(rRect, OUString(OBJECTTEXT), nStyle);
        }
    }
    else if ( dynamic_cast<const FmXFilterCell*>( m_pCell.get() ) !=  nullptr )
        static_cast< FmXFilterCell* >( m_pCell.get() )->PaintCell( rDev, rRect );
}


void DbGridColumn::ImplInitWindow( vcl::Window const & rParent, const InitWindowFacet _eInitWhat )
{
    if ( m_pCell.is() )
        m_pCell->ImplInitWindow( rParent, _eInitWhat );
}


//= cell controls


DbCellControl::DbCellControl( DbGridColumn& _rColumn )
    :OPropertyChangeListener(m_aMutex)
    ,m_bTransparent( false )
    ,m_bAlignedController( true )
    ,m_bAccessingValueProperty( false )
    ,m_rColumn( _rColumn )
    ,m_pPainter( nullptr )
    ,m_pWindow( nullptr )
{
    Reference< XPropertySet > xColModelProps( _rColumn.getModel(), UNO_QUERY );
    if ( !xColModelProps.is() )
        return;

    // if our model's format key changes we want to propagate the new value to our windows
    m_pModelChangeBroadcaster = new ::comphelper::OPropertyChangeMultiplexer(this, Reference< css::beans::XPropertySet > (_rColumn.getModel(), UNO_QUERY));

    // be listener for some common properties
    implDoPropertyListening( FM_PROP_READONLY, false );
    implDoPropertyListening( FM_PROP_ENABLED, false );

    // add as listener for all known "value" properties
    implDoPropertyListening( FM_PROP_VALUE, false );
    implDoPropertyListening( FM_PROP_STATE, false );
    implDoPropertyListening( FM_PROP_TEXT, false );
    implDoPropertyListening( FM_PROP_EFFECTIVE_VALUE, false );
    implDoPropertyListening( FM_PROP_SELECT_SEQ, false );
    implDoPropertyListening( FM_PROP_DATE, false );
    implDoPropertyListening( FM_PROP_TIME, false );

    // be listener at the bound field as well
    try
    {
        Reference< XPropertySetInfo > xPSI( xColModelProps->getPropertySetInfo(), UNO_SET_THROW );
        if ( xPSI->hasPropertyByName( FM_PROP_BOUNDFIELD ) )
        {
            Reference< XPropertySet > xField;
            xColModelProps->getPropertyValue( FM_PROP_BOUNDFIELD ) >>= xField;
            if ( xField.is() )
            {
                m_pFieldChangeBroadcaster = new ::comphelper::OPropertyChangeMultiplexer(this, xField);
                m_pFieldChangeBroadcaster->addProperty( FM_PROP_ISREADONLY );
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
        OSL_FAIL( "DbCellControl::doPropertyListening: caught an exception!" );
    }
}


void DbCellControl::implDoPropertyListening(const OUString& _rPropertyName, bool _bWarnIfNotExistent)
{
    try
    {
        Reference< XPropertySet > xColModelProps( m_rColumn.getModel(), UNO_QUERY );
        Reference< XPropertySetInfo > xPSI;
        if ( xColModelProps.is() )
            xPSI = xColModelProps->getPropertySetInfo();

        DBG_ASSERT( !_bWarnIfNotExistent || ( xPSI.is() && xPSI->hasPropertyByName( _rPropertyName ) ),
            "DbCellControl::doPropertyListening: no property set info or non-existent property!" );

        if ( xPSI.is() && xPSI->hasPropertyByName( _rPropertyName ) )
            m_pModelChangeBroadcaster->addProperty( _rPropertyName );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
        OSL_FAIL( "DbCellControl::doPropertyListening: caught an exception!" );
    }
}


void DbCellControl::doPropertyListening(const OUString& _rPropertyName)
{
    implDoPropertyListening( _rPropertyName, true );
}

static void lcl_clearBroadCaster(rtl::Reference<::comphelper::OPropertyChangeMultiplexer>& _pBroadcaster)
{
    if ( _pBroadcaster.is() )
    {
        _pBroadcaster->dispose();
        _pBroadcaster.clear();
        // no delete, this is done implicitly
    }
}

DbCellControl::~DbCellControl()
{
    lcl_clearBroadCaster(m_pModelChangeBroadcaster);
    lcl_clearBroadCaster(m_pFieldChangeBroadcaster);

    m_pWindow.disposeAndClear();
    m_pPainter.disposeAndClear();
}

void DbCellControl::implValuePropertyChanged( )
{
    OSL_ENSURE( !isValuePropertyLocked(),
        "DbCellControl::implValuePropertyChanged: not to be called with the value property locked!" );

    if ( m_pWindow )
    {
        if ( m_rColumn.getModel().is() )
            updateFromModel( m_rColumn.getModel() );
    }
}


void DbCellControl::implAdjustGenericFieldSetting( const Reference< XPropertySet >& /*_rxModel*/ )
{
    // nothing to do here
}


void DbCellControl::_propertyChanged(const PropertyChangeEvent& _rEvent)
{
    SolarMutexGuard aGuard;

    Reference< XPropertySet > xSourceProps( _rEvent.Source, UNO_QUERY );

    if  (   _rEvent.PropertyName == FM_PROP_VALUE
        ||  _rEvent.PropertyName == FM_PROP_STATE
        ||  _rEvent.PropertyName == FM_PROP_TEXT
        ||  _rEvent.PropertyName == FM_PROP_EFFECTIVE_VALUE
        ||  _rEvent.PropertyName == FM_PROP_SELECT_SEQ
        ||  _rEvent.PropertyName == FM_PROP_DATE
        ||  _rEvent.PropertyName == FM_PROP_TIME
        )
    {   // it was one of the known "value" properties
        if ( !isValuePropertyLocked() )
        {
            implValuePropertyChanged( );
        }
    }
    else if ( _rEvent.PropertyName == FM_PROP_READONLY )
    {
        implAdjustReadOnly( xSourceProps, true);
    }
    else if ( _rEvent.PropertyName == FM_PROP_ISREADONLY )
    {
        bool bReadOnly = true;
        _rEvent.NewValue >>= bReadOnly;
        m_rColumn.SetReadOnly(bReadOnly);
        implAdjustReadOnly( xSourceProps, false);
    }
    else if ( _rEvent.PropertyName == FM_PROP_ENABLED )
    {
        implAdjustEnabled( xSourceProps );
    }
    else
        implAdjustGenericFieldSetting( xSourceProps );
}


bool DbCellControl::Commit()
{
    // lock the listening for value property changes
    lockValueProperty();
    // commit the content of the control into the model's value property
    bool bReturn = false;
    try
    {
        bReturn = commitControl();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
    // unlock the listening for value property changes
    unlockValueProperty();
    // outta here
    return bReturn;
}


void DbCellControl::ImplInitWindow( vcl::Window const & rParent, const InitWindowFacet _eInitWhat )
{
    vcl::Window* pWindows[] = { m_pPainter, m_pWindow };

    if (_eInitWhat & InitWindowFacet::WritingMode)
    {
        for (vcl::Window* pWindow : pWindows)
        {
            if (pWindow)
                pWindow->EnableRTL(rParent.IsRTLEnabled());
        }
    }

    if (_eInitWhat & InitWindowFacet::Font)
    {
        for (vcl::Window* pWindow : pWindows)
        {
            if (!pWindow)
                continue;

            pWindow->SetZoom(rParent.GetZoom());

            const StyleSettings& rStyleSettings = pWindow->GetSettings().GetStyleSettings();
            vcl::Font aFont = rStyleSettings.GetFieldFont();
            aFont.SetTransparent(isTransparent());

            if (rParent.IsControlFont())
            {
                pWindow->SetControlFont(rParent.GetControlFont());
                aFont.Merge(rParent.GetControlFont());
            }
            else
                pWindow->SetControlFont();

            pWindow->SetZoomedPointFont(*pWindow, aFont); // FIXME RenderContext
        }
    }

    if ((_eInitWhat & InitWindowFacet::Font) || (_eInitWhat & InitWindowFacet::Foreground))
    {
        Color aTextColor(rParent.IsControlForeground() ? rParent.GetControlForeground() : rParent.GetTextColor());

        bool bTextLineColor = rParent.IsTextLineColor();
        Color aTextLineColor(rParent.GetTextLineColor());

        for (vcl::Window* pWindow : pWindows)
        {
            if (pWindow)
            {
                pWindow->SetTextColor(aTextColor);
                if (rParent.IsControlForeground())
                    pWindow->SetControlForeground(aTextColor);

                if (bTextLineColor)
                    pWindow->SetTextLineColor();
                else
                    pWindow->SetTextLineColor(aTextLineColor);
            }
        }
    }

    if (_eInitWhat & InitWindowFacet::Background)
    {
        if (rParent.IsControlBackground())
        {
            Color aColor(rParent.GetControlBackground());
            for (vcl::Window* pWindow : pWindows)
            {
                if (pWindow)
                {
                    if (isTransparent())
                        pWindow->SetBackground();
                    else
                    {
                        pWindow->SetBackground(aColor);
                        pWindow->SetControlBackground(aColor);
                    }
                    pWindow->SetFillColor(aColor);
                }
            }
        }
        else
        {
            if (m_pPainter)
            {
                if (isTransparent())
                    m_pPainter->SetBackground();
                else
                    m_pPainter->SetBackground(rParent.GetBackground());
                m_pPainter->SetFillColor(rParent.GetFillColor());
            }

            if (m_pWindow)
            {
                if (isTransparent())
                    m_pWindow->SetBackground(rParent.GetBackground());
                else
                    m_pWindow->SetFillColor(rParent.GetFillColor());
            }
        }
    }
}


void DbCellControl::implAdjustReadOnly( const Reference< XPropertySet >& _rxModel,bool i_bReadOnly )
{
    DBG_ASSERT( m_pWindow, "DbCellControl::implAdjustReadOnly: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbCellControl::implAdjustReadOnly: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        Edit* pEditWindow = dynamic_cast< Edit* >( m_pWindow.get() );
        if ( pEditWindow )
        {
            bool bReadOnly = m_rColumn.IsReadOnly();
            if ( !bReadOnly )
            {
                _rxModel->getPropertyValue( i_bReadOnly ? OUString(FM_PROP_READONLY) : OUString(FM_PROP_ISREADONLY)) >>= bReadOnly;
            }
            static_cast< Edit* >( m_pWindow.get() )->SetReadOnly( bReadOnly );
        }
    }
}


void DbCellControl::implAdjustEnabled( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbCellControl::implAdjustEnabled: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbCellControl::implAdjustEnabled: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        bool bEnable = true;
        _rxModel->getPropertyValue( FM_PROP_ENABLED ) >>= bEnable;
        m_pWindow->Enable( bEnable );
    }
}


void DbCellControl::Init( vcl::Window& rParent, const Reference< XRowSet >& _rxCursor )
{
    ImplInitWindow( rParent, InitWindowFacet::All );

    if ( m_pWindow )
    {
        // align the control
        if ( isAlignedController() )
            AlignControl( m_rColumn.GetAlignment() );

        try
        {
            // some other common properties
            Reference< XPropertySet > xModel( m_rColumn.getModel(), UNO_SET_THROW );
            Reference< XPropertySetInfo > xModelPSI( xModel->getPropertySetInfo(), UNO_SET_THROW );

            if ( xModelPSI->hasPropertyByName( FM_PROP_READONLY ) )
            {
                implAdjustReadOnly( xModel,true );
            }

            if ( xModelPSI->hasPropertyByName( FM_PROP_ENABLED ) )
            {
                implAdjustEnabled( xModel );
            }

            if ( xModelPSI->hasPropertyByName( FM_PROP_MOUSE_WHEEL_BEHAVIOR ) )
            {
                sal_Int16 nWheelBehavior = css::awt::MouseWheelBehavior::SCROLL_FOCUS_ONLY;
                OSL_VERIFY( xModel->getPropertyValue( FM_PROP_MOUSE_WHEEL_BEHAVIOR ) >>= nWheelBehavior );
                MouseWheelBehaviour nVclSetting = MouseWheelBehaviour::FocusOnly;
                switch ( nWheelBehavior )
                {
                case css::awt::MouseWheelBehavior::SCROLL_DISABLED:   nVclSetting = MouseWheelBehaviour::Disable; break;
                case css::awt::MouseWheelBehavior::SCROLL_FOCUS_ONLY: nVclSetting = MouseWheelBehaviour::FocusOnly; break;
                case css::awt::MouseWheelBehavior::SCROLL_ALWAYS:     nVclSetting = MouseWheelBehaviour::ALWAYS; break;
                default:
                    OSL_FAIL( "DbCellControl::Init: invalid MouseWheelBehavior!" );
                    break;
                }

                AllSettings aSettings = m_pWindow->GetSettings();
                MouseSettings aMouseSettings = aSettings.GetMouseSettings();
                aMouseSettings.SetWheelBehavior( nVclSetting );
                aSettings.SetMouseSettings( aMouseSettings );
                m_pWindow->SetSettings( aSettings, true );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }
    m_xCursor = _rxCursor;
    if ( m_rColumn.getModel().is() )
        updateFromModel( m_rColumn.getModel() );
}


void DbCellControl::SetTextLineColor()
{
    if (m_pWindow)
        m_pWindow->SetTextLineColor();
    if (m_pPainter)
        m_pPainter->SetTextLineColor();
}


void DbCellControl::SetTextLineColor(const Color& _rColor)
{
    if (m_pWindow)
        m_pWindow->SetTextLineColor(_rColor);
    if (m_pPainter)
        m_pPainter->SetTextLineColor(_rColor);
}

namespace
{
    void lcl_implAlign( vcl::Window* _pWindow, WinBits _nAlignmentBit )
    {
        WinBits nStyle = _pWindow->GetStyle();
        nStyle &= ~(WB_LEFT | WB_RIGHT | WB_CENTER);
        _pWindow->SetStyle( nStyle | _nAlignmentBit );
    }
}


void DbCellControl::AlignControl(sal_Int16 nAlignment)
{
    WinBits nAlignmentBit = 0;
    switch (nAlignment)
    {
        case css::awt::TextAlign::RIGHT:
            nAlignmentBit = WB_RIGHT;
            break;
        case css::awt::TextAlign::CENTER:
            nAlignmentBit = WB_CENTER;
            break;
        default:
            nAlignmentBit = WB_LEFT;
            break;
    }
    lcl_implAlign( m_pWindow, nAlignmentBit );
    if ( m_pPainter )
        lcl_implAlign( m_pPainter, nAlignmentBit );
}


void DbCellControl::PaintCell( OutputDevice& _rDev, const tools::Rectangle& _rRect )
{
    if ( m_pPainter->GetParent() == &_rDev )
    {
        m_pPainter->SetPaintTransparent( true );
        m_pPainter->SetBackground( );
        m_pPainter->SetControlBackground( _rDev.GetFillColor() );
        m_pPainter->SetControlForeground( _rDev.GetTextColor() );
        m_pPainter->SetTextColor( _rDev.GetTextColor() );
        m_pPainter->SetTextFillColor( _rDev.GetTextColor() );

        vcl::Font aFont( _rDev.GetFont() );
        aFont.SetTransparent( true );
        m_pPainter->SetFont( aFont );

        m_pPainter->SetPosSizePixel( _rRect.TopLeft(), _rRect.GetSize() );
        m_pPainter->Show();
        m_pPainter->Update();
        m_pPainter->SetParentUpdateMode( false );
        m_pPainter->Hide();
        m_pPainter->SetParentUpdateMode( true );
    }
    else
        m_pPainter->Draw( &_rDev, _rRect.TopLeft(), _rRect.GetSize(), DrawFlags::NONE );
}


void DbCellControl::PaintFieldToCell( OutputDevice& _rDev, const tools::Rectangle& _rRect, const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& _rxFormatter )
{
    m_pPainter->SetText( GetFormatText( _rxField, _rxFormatter ) );
    PaintCell( _rDev, _rRect );
}


double DbCellControl::GetValue(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter) const
{
    double fValue = 0;
    if (m_rColumn.IsNumeric())
    {
        try
        {
            fValue = _rxField->getDouble();
        }
        catch(const Exception&) { }
    }
    else
    {
        bool bSuccess = false;
        try
        {
            fValue = _rxField->getDouble();
            bSuccess = true;
        }
        catch(const Exception&) { }
        if (!bSuccess)
        {
            try
            {
                fValue = xFormatter->convertStringToNumber(m_rColumn.GetKey(), _rxField->getString());
            }
            catch(const Exception&) { }
        }
    }
    return fValue;
}


void DbCellControl::invalidatedController()
{
    m_rColumn.GetParent().refreshController(m_rColumn.GetId(), DbGridControl::GrantControlAccess());
}

// CellModels

DbLimitedLengthField::DbLimitedLengthField( DbGridColumn& _rColumn )
    :DbCellControl( _rColumn )
{
    doPropertyListening( FM_PROP_MAXTEXTLEN );
}


void DbLimitedLengthField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbLimitedLengthField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbLimitedLengthField::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Int16 nMaxLen = 0;
        _rxModel->getPropertyValue( FM_PROP_MAXTEXTLEN ) >>= nMaxLen;
        implSetMaxTextLen( nMaxLen );
    }
}

void DbLimitedLengthField::implSetEffectiveMaxTextLen( sal_Int32 _nMaxLen )
{
    dynamic_cast<Edit&>(*m_pWindow).SetMaxTextLen(_nMaxLen);
    if (m_pPainter)
        dynamic_cast<Edit&>(*m_pPainter).SetMaxTextLen(_nMaxLen);
}

DbTextField::DbTextField(DbGridColumn& _rColumn)
            :DbLimitedLengthField(_rColumn)
            ,m_bIsSimpleEdit( true )
{
}


DbTextField::~DbTextField( )
{
    m_pPainterImplementation.reset();
    m_pEdit.reset();
}


void DbTextField::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor)
{
    sal_Int16 nAlignment = m_rColumn.SetAlignmentFromModel(-1);

    Reference< XPropertySet > xModel( m_rColumn.getModel() );

    WinBits nStyle = WB_LEFT;
    switch (nAlignment)
    {
    case awt::TextAlign::RIGHT:
        nStyle = WB_RIGHT;
        break;

    case awt::TextAlign::CENTER:
        nStyle = WB_CENTER;
        break;
    }

    // is this a multi-line field?
    bool bIsMultiLine = false;
    try
    {
        if ( xModel.is() )
        {
            OSL_VERIFY( xModel->getPropertyValue( FM_PROP_MULTILINE ) >>= bIsMultiLine );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
        OSL_FAIL( "DbTextField::Init: caught an exception while determining the multi-line capabilities!" );
    }

    m_bIsSimpleEdit = !bIsMultiLine;
    if ( bIsMultiLine )
    {
        m_pWindow = VclPtr<MultiLineTextCell>::Create( &rParent, nStyle );
        m_pEdit.reset(new MultiLineEditImplementation( *static_cast< MultiLineTextCell* >( m_pWindow.get() ) ));

        m_pPainter = VclPtr<MultiLineTextCell>::Create( &rParent, nStyle );
        m_pPainterImplementation.reset(new MultiLineEditImplementation( *static_cast< MultiLineTextCell* >( m_pPainter.get() ) ));
    }
    else
    {
        m_pWindow = VclPtr<Edit>::Create( &rParent, nStyle );
        m_pEdit.reset(new EditImplementation( *static_cast< Edit* >( m_pWindow.get() ) ));

        m_pPainter = VclPtr<Edit>::Create( &rParent, nStyle );
        m_pPainterImplementation.reset(new EditImplementation( *static_cast< Edit* >( m_pPainter.get() ) ));
    }

    if ( WB_LEFT == nStyle )
    {
        // this is so that when getting the focus, the selection is oriented left-to-right
        AllSettings aSettings = m_pWindow->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        aStyleSettings.SetSelectionOptions(
            aStyleSettings.GetSelectionOptions() | SelectionOptions::ShowFirst);
        aSettings.SetStyleSettings(aStyleSettings);
        m_pWindow->SetSettings(aSettings);
    }

    implAdjustGenericFieldSetting( xModel );

    DbLimitedLengthField::Init( rParent, xCursor );
}


CellControllerRef DbTextField::CreateController() const
{
    return new EditCellController( m_pEdit.get() );
}


void DbTextField::PaintFieldToCell( OutputDevice& _rDev, const tools::Rectangle& _rRect, const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& _rxFormatter )
{
    if ( m_pPainterImplementation )
        m_pPainterImplementation->SetText( GetFormatText( _rxField, _rxFormatter ) );

    DbLimitedLengthField::PaintFieldToCell( _rDev, _rRect, _rxField, _rxFormatter );
}


OUString DbTextField::GetFormatText(const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter, Color** /*ppColor*/)
{
    if (!_rxField.is())
        return OUString();

    const css::uno::Reference<css::beans::XPropertySet> xPS(_rxField, UNO_QUERY);
    FormattedColumnValue fmter( xFormatter, xPS );

    try
    {
        return fmter.getFormattedValue();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
    return OUString();

}


void DbTextField::UpdateFromField(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter)
{
    m_pEdit->SetText( GetFormatText( _rxField, xFormatter ) );
    m_pEdit->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}


void DbTextField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbTextField::updateFromModel: invalid call!" );

    OUString sText;
    _rxModel->getPropertyValue( FM_PROP_TEXT ) >>= sText;

    sal_Int32 nMaxTextLen = m_pEdit->GetMaxTextLen();
    if ( EDIT_NOLIMIT != nMaxTextLen && sText.getLength() > nMaxTextLen )
    {
        sal_Int32 nDiff = sText.getLength() - nMaxTextLen;
        sText = sText.replaceAt(sText.getLength() - nDiff,nDiff, OUString());
    }


    m_pEdit->SetText( sText );
    m_pEdit->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}


bool DbTextField::commitControl()
{
    OUString aText( m_pEdit->GetText( getModelLineEndSetting( m_rColumn.getModel() ) ) );
    // we have to check if the length before we can decide if the value was modified
    sal_Int32 nMaxTextLen = m_pEdit->GetMaxTextLen();
    if ( EDIT_NOLIMIT != nMaxTextLen )
    {
        OUString sOldValue;
        m_rColumn.getModel()->getPropertyValue( FM_PROP_TEXT ) >>= sOldValue;
        // if the new value didn't change we must set the old long value again
        if ( sOldValue.getLength() > nMaxTextLen && sOldValue.compareTo(aText,nMaxTextLen) == 0 )
            aText = sOldValue;
    }
    m_rColumn.getModel()->setPropertyValue( FM_PROP_TEXT, makeAny( aText ) );
    return true;
}


void DbTextField::implSetEffectiveMaxTextLen( sal_Int32 _nMaxLen )
{
    if ( m_pEdit )
        m_pEdit->SetMaxTextLen( _nMaxLen );
    if ( m_pPainterImplementation )
        m_pPainterImplementation->SetMaxTextLen( _nMaxLen );
}

DbFormattedField::DbFormattedField(DbGridColumn& _rColumn)
    :DbLimitedLengthField(_rColumn)
{
    // if our model's format key changes we want to propagate the new value to our windows
    doPropertyListening( FM_PROP_FORMATKEY );
}


DbFormattedField::~DbFormattedField()
{
}


void DbFormattedField::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor)
{
    sal_Int16 nAlignment = m_rColumn.SetAlignmentFromModel(-1);

    Reference< css::beans::XPropertySet >  xUnoModel = m_rColumn.getModel();

    switch (nAlignment)
    {
        case css::awt::TextAlign::RIGHT:
            m_pWindow  = VclPtr<FormattedField>::Create( &rParent, WB_RIGHT );
            m_pPainter = VclPtr<FormattedField>::Create( &rParent, WB_RIGHT );
            break;

        case css::awt::TextAlign::CENTER:
            m_pWindow  = VclPtr<FormattedField>::Create( &rParent, WB_CENTER );
            m_pPainter  = VclPtr<FormattedField>::Create( &rParent, WB_CENTER );
            break;
        default:
            m_pWindow  = VclPtr<FormattedField>::Create( &rParent, WB_LEFT );
            m_pPainter  = VclPtr<FormattedField>::Create( &rParent, WB_LEFT );

            // Everything just so that the selection goes from right to left when getting focus
            AllSettings aSettings = m_pWindow->GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                aStyleSettings.GetSelectionOptions() | SelectionOptions::ShowFirst);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings);
    }

    implAdjustGenericFieldSetting( xUnoModel );

    static_cast< FormattedField* >( m_pWindow.get() )->SetStrictFormat( false );
    static_cast< FormattedField* >( m_pPainter.get() )->SetStrictFormat( false );
        // if one allows any formatting, one cannot make an entry check anyway
        // (the FormattedField does not support that anyway, only derived classes)

    // get the formatter from the uno model
    // (I could theoretically also go via the css::util::NumberFormatter, which the cursor would
    // surely give me. The problem is that I can not really rely on the fact that the two
    // formatters are the same. Clean is the whole thing if I go via the UNO model.)
    sal_Int32 nFormatKey = -1;

    // let's see if the model has one ...
    DBG_ASSERT(::comphelper::hasProperty(FM_PROP_FORMATSSUPPLIER, xUnoModel), "DbFormattedField::Init : invalid UNO model !");
    Any aSupplier( xUnoModel->getPropertyValue(FM_PROP_FORMATSSUPPLIER));
    if (aSupplier.hasValue())
    {
        m_xSupplier.set(aSupplier, css::uno::UNO_QUERY);
        if (m_xSupplier.is())
        {
            // if we take the supplier from the model, then also the key
            Any aFmtKey( xUnoModel->getPropertyValue(FM_PROP_FORMATKEY));
            if (aFmtKey.hasValue())
            {
                DBG_ASSERT(aFmtKey.getValueType().getTypeClass() == TypeClass_LONG, "DbFormattedField::Init : invalid format key property (no sal_Int32) !");
                nFormatKey = ::comphelper::getINT32(aFmtKey);
            }
            else
            {
                SAL_INFO("svx.fmcomp", "DbFormattedField::Init : my uno-model has no format-key, but a formats supplier !");
                // the OFormattedModel which we usually are working with ensures that the model has a format key
                // as soon as the form is loaded. Unfortunally this method here is called from within loaded, too.
                // So if our LoadListener is called before the LoadListener of the model, this "else case" is
                // allowed.
                // Of course our property listener for the FormatKey property will notify us if the prop is changed,
                // so this here isn't really bad ....
                nFormatKey = 0;
            }
        }
    }

    // No? Maybe the css::form::component::Form behind the cursor?
    if (!m_xSupplier.is())
    {
        Reference< XRowSet >  xCursorForm(xCursor, UNO_QUERY);
        if (xCursorForm.is())
        {   // If we take the formatter from the cursor, then also the key from the field to which we are bound
            m_xSupplier = getNumberFormats(getConnection(xCursorForm));

            if (m_rColumn.GetField().is())
                nFormatKey = ::comphelper::getINT32(m_rColumn.GetField()->getPropertyValue(FM_PROP_FORMATKEY));
        }
    }

    SvNumberFormatter* pFormatterUsed = nullptr;
    if (m_xSupplier.is())
    {
        SvNumberFormatsSupplierObj* pImplmentation = SvNumberFormatsSupplierObj::getImplementation(m_xSupplier);
        if (pImplmentation)
            pFormatterUsed = pImplmentation->GetNumberFormatter();
        else
            // Everything is invalid: the supplier is of the wrong type, then we can not
            // rely on a standard formatter to know the (possibly non-standard) key.
            nFormatKey = -1;
    }

    // a standard formatter ...
    if (pFormatterUsed == nullptr)
    {
        pFormatterUsed = static_cast<FormattedField*>(m_pWindow.get())->StandardFormatter();
        DBG_ASSERT(pFormatterUsed != nullptr, "DbFormattedField::Init : no standard formatter given by the numeric field !");
    }
    // ... and a standard key
    if (nFormatKey == -1)
        nFormatKey = 0;

    static_cast<FormattedField*>(m_pWindow.get())->SetFormatter(pFormatterUsed);
    static_cast<FormattedField*>(m_pPainter.get())->SetFormatter(pFormatterUsed);

    static_cast<FormattedField*>(m_pWindow.get())->SetFormatKey(nFormatKey);
    static_cast<FormattedField*>(m_pPainter.get())->SetFormatKey(nFormatKey);

    static_cast<FormattedField*>(m_pWindow.get())->TreatAsNumber(m_rColumn.IsNumeric());
    static_cast<FormattedField*>(m_pPainter.get())->TreatAsNumber(m_rColumn.IsNumeric());

    // min and max values
    if (m_rColumn.IsNumeric())
    {
        bool bClearMin = true;
        if (::comphelper::hasProperty(FM_PROP_EFFECTIVE_MIN, xUnoModel))
        {
            Any aMin( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_MIN));
            if (aMin.getValueType().getTypeClass() != TypeClass_VOID)
            {
                DBG_ASSERT(aMin.getValueType().getTypeClass() == TypeClass_DOUBLE, "DbFormattedField::Init : the model has an invalid min value !");
                double dMin = ::comphelper::getDouble(aMin);
                static_cast<FormattedField*>(m_pWindow.get())->SetMinValue(dMin);
                static_cast<FormattedField*>(m_pPainter.get())->SetMinValue(dMin);
                bClearMin = false;
            }
        }
        if (bClearMin)
        {
            static_cast<FormattedField*>(m_pWindow.get())->ClearMinValue();
            static_cast<FormattedField*>(m_pPainter.get())->ClearMinValue();
        }
        bool bClearMax = true;
        if (::comphelper::hasProperty(FM_PROP_EFFECTIVE_MAX, xUnoModel))
        {
            Any aMin( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_MAX));
            if (aMin.getValueType().getTypeClass() != TypeClass_VOID)
            {
                DBG_ASSERT(aMin.getValueType().getTypeClass() == TypeClass_DOUBLE, "DbFormattedField::Init : the model has an invalid max value !");
                double dMin = ::comphelper::getDouble(aMin);
                static_cast<FormattedField*>(m_pWindow.get())->SetMaxValue(dMin);
                static_cast<FormattedField*>(m_pPainter.get())->SetMaxValue(dMin);
                bClearMax = false;
            }
        }
        if (bClearMax)
        {
            static_cast<FormattedField*>(m_pWindow.get())->ClearMaxValue();
            static_cast<FormattedField*>(m_pPainter.get())->ClearMaxValue();
        }
    }

    // the default value
    Any aDefault( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_DEFAULT));
    if (aDefault.hasValue())
    {   // the thing can be a double or a string
        switch (aDefault.getValueType().getTypeClass())
        {
            case TypeClass_DOUBLE:
                if (m_rColumn.IsNumeric())
                {
                    static_cast<FormattedField*>(m_pWindow.get())->SetDefaultValue(::comphelper::getDouble(aDefault));
                    static_cast<FormattedField*>(m_pPainter.get())->SetDefaultValue(::comphelper::getDouble(aDefault));
                }
                else
                {
                    OUString sConverted;
                    Color* pDummy;
                    pFormatterUsed->GetOutputString(::comphelper::getDouble(aDefault), 0, sConverted, &pDummy);
                    static_cast<FormattedField*>(m_pWindow.get())->SetDefaultText(sConverted);
                    static_cast<FormattedField*>(m_pPainter.get())->SetDefaultText(sConverted);
                }
                break;
            case TypeClass_STRING:
            {
                OUString sDefault( ::comphelper::getString(aDefault) );
                if (m_rColumn.IsNumeric())
                {
                    double dVal;
                    sal_uInt32 nTestFormat(0);
                    if (pFormatterUsed->IsNumberFormat(sDefault, nTestFormat, dVal))
                    {
                        static_cast<FormattedField*>(m_pWindow.get())->SetDefaultValue(dVal);
                        static_cast<FormattedField*>(m_pPainter.get())->SetDefaultValue(dVal);
                    }
                }
                else
                {
                    static_cast<FormattedField*>(m_pWindow.get())->SetDefaultText(sDefault);
                    static_cast<FormattedField*>(m_pPainter.get())->SetDefaultText(sDefault);
                }
            }
            break;
            default:
                OSL_FAIL( "DbFormattedField::Init: unexpected value type!" );
                break;
        }
    }
    DbLimitedLengthField::Init( rParent, xCursor );
}


CellControllerRef DbFormattedField::CreateController() const
{
    return new ::svt::FormattedFieldCellController( static_cast< FormattedField* >( m_pWindow.get() ) );
}


void DbFormattedField::_propertyChanged( const PropertyChangeEvent& _rEvent )
{
    if (_rEvent.PropertyName == FM_PROP_FORMATKEY )
    {
        sal_Int32 nNewKey = _rEvent.NewValue.hasValue() ? ::comphelper::getINT32(_rEvent.NewValue) : 0;

        DBG_ASSERT(m_pWindow && m_pPainter, "DbFormattedField::_propertyChanged : where are my windows ?");
        if (m_pWindow)
            static_cast< FormattedField* >( m_pWindow.get() )->SetFormatKey( nNewKey );
        if (m_pPainter)
            static_cast< FormattedField* >( m_pPainter.get() )->SetFormatKey( nNewKey );
    }
    else
    {
        DbLimitedLengthField::_propertyChanged( _rEvent );
    }
}


OUString DbFormattedField::GetFormatText(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/, Color** ppColor)
{
    // no color specification by default
    if (ppColor != nullptr)
        *ppColor = nullptr;

    // NULL value -> empty text
    if (!_rxField.is())
        return OUString();

    OUString aText;
    try
    {
        if (m_rColumn.IsNumeric())
        {
            // The IsNumeric at the column says nothing about the class of the used format, but
            // about the class of the field bound to the column. So when you bind a FormattedField
            // column to a double field and format it as text, m_rColumn.IsNumeric() returns
            // sal_True. So that simply means that I can query the contents of the variant using
            // getDouble, and then I can leave the rest (the formatting) to the FormattedField.
            double dValue = getValue( _rxField, m_rColumn.GetParent().getNullDate() );
            if (_rxField->wasNull())
                return aText;
            static_cast<FormattedField*>(m_pPainter.get())->SetValue(dValue);
        }
        else
        {
            // Here I can not work with a double, since the field can not provide it to me.
            // So simply bind the text from the css::util::NumberFormatter to the correct css::form::component::Form.
            aText = _rxField->getString();
            if (_rxField->wasNull())
                return aText;
            static_cast<FormattedField*>(m_pPainter.get())->SetTextFormatted(aText);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    aText = m_pPainter->GetText();
    if (ppColor != nullptr)
        *ppColor = static_cast<FormattedField*>(m_pPainter.get())->GetLastOutputColor();

    return aText;
}


void DbFormattedField::UpdateFromField(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    try
    {
        FormattedField* pFormattedWindow = static_cast<FormattedField*>(m_pWindow.get());
        if (!_rxField.is())
        {   // NULL value -> empty text
            m_pWindow->SetText(OUString());
        }
        else if (m_rColumn.IsNumeric())
        {
            // The IsNumeric at the column says nothing about the class of the used format, but
            // about the class of the field bound to the column. So when you bind a FormattedField
            // column to a double field and format it as text, m_rColumn.IsNumeric() returns
            // sal_True. So that simply means that I can query the contents of the variant using
            // getDouble, and then I can leave the rest (the formatting) to the FormattedField.
            double dValue = getValue( _rxField, m_rColumn.GetParent().getNullDate() );
            if (_rxField->wasNull())
                m_pWindow->SetText(OUString());
            else
                pFormattedWindow->SetValue(dValue);
        }
        else
        {
            // Here I can not work with a double, since the field can not provide it to me.
            // So simply bind the text from the css::util::NumberFormatter to the correct css::form::component::Form.
            OUString sText( _rxField->getString());

            pFormattedWindow->SetTextFormatted( sText );
            pFormattedWindow->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


void DbFormattedField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbFormattedField::updateFromModel: invalid call!" );

    FormattedField* pFormattedWindow = static_cast< FormattedField* >( m_pWindow.get() );

    OUString sText;
    Any aValue = _rxModel->getPropertyValue( FM_PROP_EFFECTIVE_VALUE );
    if ( !aValue.hasValue() || (aValue >>= sText) )
    {   // our effective value is transferred as string
        pFormattedWindow->SetTextFormatted( sText );
        pFormattedWindow->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
    }
    else
    {
        double dValue = 0;
        aValue >>= dValue;
        pFormattedWindow->SetValue(dValue);
    }
}


bool DbFormattedField::commitControl()
{
    Any aNewVal;
    FormattedField& rField = *static_cast<FormattedField*>(m_pWindow.get());
    DBG_ASSERT(&rField == m_pWindow, "DbFormattedField::commitControl : can't work with a window other than my own !");
    if (m_rColumn.IsNumeric())
    {
        if (!rField.GetText().isEmpty())
            aNewVal <<= rField.GetValue();
        // an empty string is passed on as void by default, to start with
    }
    else
        aNewVal <<= rField.GetTextValue();

    m_rColumn.getModel()->setPropertyValue(FM_PROP_EFFECTIVE_VALUE, aNewVal);
    return true;
}

DbCheckBox::DbCheckBox( DbGridColumn& _rColumn )
    :DbCellControl( _rColumn )
{
    setAlignedController( false );
}

namespace
{
    void setCheckBoxStyle( vcl::Window* _pWindow, bool bMono )
    {
        AllSettings aSettings = _pWindow->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        if( bMono )
            aStyleSettings.SetOptions( aStyleSettings.GetOptions() | StyleSettingsOptions::Mono );
        else
            aStyleSettings.SetOptions( aStyleSettings.GetOptions() & (~StyleSettingsOptions::Mono) );
        aSettings.SetStyleSettings( aStyleSettings );
        _pWindow->SetSettings( aSettings );
    }
}


void DbCheckBox::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor )
{
    setTransparent( true );

    m_pWindow  = VclPtr<CheckBoxControl>::Create( &rParent );
    m_pPainter = VclPtr<CheckBoxControl>::Create( &rParent );

    m_pWindow->SetPaintTransparent( true );
    m_pPainter->SetPaintTransparent( true );

    m_pPainter->SetBackground();

    try
    {
        Reference< XPropertySet > xModel( m_rColumn.getModel(), UNO_SET_THROW );

        sal_Int16 nStyle = awt::VisualEffect::LOOK3D;
        OSL_VERIFY( xModel->getPropertyValue( FM_PROP_VISUALEFFECT ) >>= nStyle );

        setCheckBoxStyle( m_pWindow, nStyle == awt::VisualEffect::FLAT );
        setCheckBoxStyle( m_pPainter, nStyle == awt::VisualEffect::FLAT );

        bool bTristate = true;
        OSL_VERIFY( xModel->getPropertyValue( FM_PROP_TRISTATE ) >>= bTristate );
        static_cast< CheckBoxControl* >( m_pWindow.get() )->GetBox().EnableTriState( bTristate );
        static_cast< CheckBoxControl* >( m_pPainter.get() )->GetBox().EnableTriState( bTristate );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    DbCellControl::Init( rParent, xCursor );
}


CellControllerRef DbCheckBox::CreateController() const
{
    return new CheckBoxCellController(static_cast<CheckBoxControl*>(m_pWindow.get()));
}

static void lcl_setCheckBoxState(   const Reference< css::sdb::XColumn >& _rxField,
                        CheckBoxControl* _pCheckBoxControl )
{
    TriState eState = TRISTATE_INDET;
    if (_rxField.is())
    {
        try
        {
            bool bValue = _rxField->getBoolean();
            if (!_rxField->wasNull())
                eState = bValue ? TRISTATE_TRUE : TRISTATE_FALSE;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }
    _pCheckBoxControl->GetBox().SetState(eState);
}


void DbCheckBox::UpdateFromField(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    lcl_setCheckBoxState( _rxField, static_cast<CheckBoxControl*>(m_pWindow.get()) );
}


void DbCheckBox::PaintFieldToCell(OutputDevice& rDev, const tools::Rectangle& rRect,
                          const Reference< css::sdb::XColumn >& _rxField,
                          const Reference< XNumberFormatter >& xFormatter)
{
    lcl_setCheckBoxState( _rxField, static_cast<CheckBoxControl*>(m_pPainter.get()) );
    DbCellControl::PaintFieldToCell( rDev, rRect, _rxField, xFormatter );
}


void DbCheckBox::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbCheckBox::updateFromModel: invalid call!" );

    sal_Int16 nState = TRISTATE_INDET;
    _rxModel->getPropertyValue( FM_PROP_STATE ) >>= nState;
    static_cast< CheckBoxControl* >( m_pWindow.get() )->GetBox().SetState( static_cast< TriState >( nState ) );
}


bool DbCheckBox::commitControl()
{
    m_rColumn.getModel()->setPropertyValue( FM_PROP_STATE,
                    makeAny( static_cast<sal_Int16>( static_cast< CheckBoxControl* >( m_pWindow.get() )->GetBox().GetState() ) ) );
    return true;
}


OUString DbCheckBox::GetFormatText(const Reference< XColumn >& /*_rxField*/, const Reference< XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
    return OUString();
}

DbPatternField::DbPatternField( DbGridColumn& _rColumn, const Reference<XComponentContext>& _rContext )
    :DbCellControl( _rColumn )
    ,m_xContext( _rContext )
{
    doPropertyListening( FM_PROP_LITERALMASK );
    doPropertyListening( FM_PROP_EDITMASK );
    doPropertyListening( FM_PROP_STRICTFORMAT );
}


void DbPatternField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbPatternField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbPatternField::implAdjustGenericFieldSetting: invalid model!" );
    if ( !m_pWindow || !_rxModel.is() )
        return;

    OUString aLitMask;
    OUString aEditMask;
    bool bStrict = false;

    _rxModel->getPropertyValue( FM_PROP_LITERALMASK ) >>= aLitMask;
    _rxModel->getPropertyValue( FM_PROP_EDITMASK ) >>= aEditMask;
    _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) >>= bStrict;

    OString aAsciiEditMask(OUStringToOString(aEditMask, RTL_TEXTENCODING_ASCII_US));

    static_cast< PatternField* >( m_pWindow.get() )->SetMask( aAsciiEditMask, aLitMask );
    static_cast< PatternField* >( m_pPainter.get() )->SetMask( aAsciiEditMask, aLitMask );
    static_cast< PatternField* >( m_pWindow.get() )->SetStrictFormat( bStrict );
    static_cast< PatternField* >( m_pPainter.get() )->SetStrictFormat( bStrict );
}


void DbPatternField::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor)
{
    m_rColumn.SetAlignmentFromModel(-1);

    m_pWindow = VclPtr<PatternField>::Create( &rParent, 0 );
    m_pPainter= VclPtr<PatternField>::Create( &rParent, 0 );

    Reference< XPropertySet >   xModel( m_rColumn.getModel() );
    implAdjustGenericFieldSetting( xModel );

    DbCellControl::Init( rParent, xCursor );
}


CellControllerRef DbPatternField::CreateController() const
{
    return new SpinCellController( static_cast< PatternField* >( m_pWindow.get() ) );
}


OUString DbPatternField::impl_formatText( const OUString& _rText )
{
    m_pPainter->SetText( _rText );
    static_cast< PatternField* >( m_pPainter.get() )->ReformatAll();
    return m_pPainter->GetText();
}


OUString DbPatternField::GetFormatText(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
    bool bIsForPaint = _rxField != m_rColumn.GetField();
    ::std::unique_ptr< FormattedColumnValue >& rpFormatter = bIsForPaint ? m_pPaintFormatter : m_pValueFormatter;

    if (!rpFormatter)
    {
        rpFormatter = std::make_unique< FormattedColumnValue> (
            m_xContext, getCursor(), Reference< XPropertySet >( _rxField, UNO_QUERY ) );
        OSL_ENSURE(rpFormatter, "DbPatternField::Init: no value formatter!");
    }
    else
        OSL_ENSURE( rpFormatter->getColumn() == _rxField, "DbPatternField::GetFormatText: my value formatter is working for another field ...!" );
        // re-creating the value formatter here every time would be quite expensive ...

    OUString sText;
    if (rpFormatter)
        sText = rpFormatter->getFormattedValue();

    return impl_formatText( sText );
}


void DbPatternField::UpdateFromField( const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& _rxFormatter )
{
    static_cast< Edit* >( m_pWindow.get() )->SetText( GetFormatText( _rxField, _rxFormatter ) );
    static_cast< Edit* >( m_pWindow.get() )->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}


void DbPatternField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbPatternField::updateFromModel: invalid call!" );

    OUString sText;
    _rxModel->getPropertyValue( FM_PROP_TEXT ) >>= sText;

    static_cast< Edit* >( m_pWindow.get() )->SetText( impl_formatText( sText ) );
    static_cast< Edit* >( m_pWindow.get() )->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}


bool DbPatternField::commitControl()
{
    OUString aText(m_pWindow->GetText());
    m_rColumn.getModel()->setPropertyValue(FM_PROP_TEXT, makeAny(aText));
    return true;
}

DbSpinField::DbSpinField( DbGridColumn& _rColumn, sal_Int16 _nStandardAlign )
    :DbCellControl( _rColumn )
    ,m_nStandardAlign( _nStandardAlign )
{
}


void DbSpinField::Init( vcl::Window& _rParent, const Reference< XRowSet >& _rxCursor )
{
    m_rColumn.SetAlignmentFromModel( m_nStandardAlign );

    Reference< XPropertySet > xModel( m_rColumn.getModel() );

    // determine the WinBits for the field
    WinBits nFieldStyle = 0;
    if ( ::comphelper::getBOOL( xModel->getPropertyValue( FM_PROP_SPIN ) ) )
        nFieldStyle = WB_REPEAT | WB_SPIN;
    // create the fields
    m_pWindow = createField( &_rParent, nFieldStyle, xModel );
    m_pPainter = createField( &_rParent, nFieldStyle, xModel );

    // adjust all other settings which depend on the property values
    implAdjustGenericFieldSetting( xModel );

    // call the base class
    DbCellControl::Init( _rParent, _rxCursor );
}


CellControllerRef DbSpinField::CreateController() const
{
    return new SpinCellController( static_cast< SpinField* >( m_pWindow.get() ) );
}

DbNumericField::DbNumericField( DbGridColumn& _rColumn )
    :DbSpinField( _rColumn )
{
    doPropertyListening( FM_PROP_DECIMAL_ACCURACY );
    doPropertyListening( FM_PROP_VALUEMIN );
    doPropertyListening( FM_PROP_VALUEMAX );
    doPropertyListening( FM_PROP_VALUESTEP );
    doPropertyListening( FM_PROP_STRICTFORMAT );
    doPropertyListening( FM_PROP_SHOWTHOUSANDSEP );
}


void DbNumericField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbNumericField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbNumericField::implAdjustGenericFieldSetting: invalid model!" );
    if ( !m_pWindow || !_rxModel.is() )
        return;

    sal_Int32   nMin        = static_cast<sal_Int32>(getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMIN ) ));
    sal_Int32   nMax        = static_cast<sal_Int32>(getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMAX ) ));
    sal_Int32   nStep       = static_cast<sal_Int32>(getDouble( _rxModel->getPropertyValue( FM_PROP_VALUESTEP ) ));
    bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );
    sal_Int16   nScale      = getINT16( _rxModel->getPropertyValue( FM_PROP_DECIMAL_ACCURACY ) );
    bool    bThousand   = getBOOL( _rxModel->getPropertyValue( FM_PROP_SHOWTHOUSANDSEP ) );

    static_cast< DoubleNumericField* >( m_pWindow.get() )->SetMinValue(nMin);
    static_cast< DoubleNumericField* >( m_pWindow.get() )->SetMaxValue(nMax);
    static_cast< DoubleNumericField* >( m_pWindow.get() )->SetSpinSize(nStep);
    static_cast< DoubleNumericField* >( m_pWindow.get() )->SetStrictFormat(bStrict);

    static_cast< DoubleNumericField* >( m_pPainter.get() )->SetMinValue(nMin);
    static_cast< DoubleNumericField* >( m_pPainter.get() )->SetMaxValue(nMax);
    static_cast< DoubleNumericField* >( m_pPainter.get() )->SetStrictFormat(bStrict);


    // give a formatter to the field and the painter;
    // test first if I can get from the service behind a connection
    Reference< css::util::XNumberFormatsSupplier >  xSupplier;
    Reference< XRowSet > xForm;
    if ( m_rColumn.GetParent().getDataSource() )
        xForm.set( Reference< XInterface >(*m_rColumn.GetParent().getDataSource()), UNO_QUERY );
    if ( xForm.is() )
        xSupplier = getNumberFormats( getConnection( xForm ), true );
    SvNumberFormatter* pFormatterUsed = nullptr;
    if ( xSupplier.is() )
    {
        SvNumberFormatsSupplierObj* pImplmentation = SvNumberFormatsSupplierObj::getImplementation( xSupplier );
        pFormatterUsed = pImplmentation ? pImplmentation->GetNumberFormatter() : nullptr;
    }
    if ( nullptr == pFormatterUsed )
    {   // the cursor didn't lead to success -> standard
        pFormatterUsed = static_cast< DoubleNumericField* >( m_pWindow.get() )->StandardFormatter();
        DBG_ASSERT( pFormatterUsed != nullptr, "DbNumericField::implAdjustGenericFieldSetting: no standard formatter given by the numeric field !" );
    }
    static_cast< DoubleNumericField* >( m_pWindow.get() )->SetFormatter( pFormatterUsed );
    static_cast< DoubleNumericField* >( m_pPainter.get() )->SetFormatter( pFormatterUsed );

    // and then generate a format which has the desired length after the decimal point, etc.
    LanguageType aAppLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType();
    OUString sFormatString = pFormatterUsed->GenerateFormat(0, aAppLanguage, bThousand, false, nScale);

    static_cast< DoubleNumericField* >( m_pWindow.get() )->SetFormat( sFormatString, aAppLanguage );
    static_cast< DoubleNumericField* >( m_pPainter.get() )->SetFormat( sFormatString, aAppLanguage );
}


VclPtr<SpinField> DbNumericField::createField( vcl::Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& /*_rxModel*/  )
{
    return VclPtr<DoubleNumericField>::Create( _pParent, _nFieldStyle );
}

namespace
{

    OUString lcl_setFormattedNumeric_nothrow( DoubleNumericField& _rField, const DbCellControl& _rControl,
        const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& _rxFormatter )
    {
        OUString sValue;
        if ( _rxField.is() )
        {
            try
            {
                double fValue = _rControl.GetValue( _rxField, _rxFormatter );
                if ( !_rxField->wasNull() )
                {
                    _rField.SetValue( fValue );
                    sValue = _rField.GetText();
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
        }
        return sValue;
    }
}


OUString DbNumericField::GetFormatText(const Reference< css::sdb::XColumn >& _rxField, const Reference< css::util::XNumberFormatter >& _rxFormatter, Color** /*ppColor*/)
{
    return lcl_setFormattedNumeric_nothrow(dynamic_cast<DoubleNumericField&>(*m_pPainter), *this, _rxField, _rxFormatter);
}


void DbNumericField::UpdateFromField(const Reference< css::sdb::XColumn >& _rxField, const Reference< css::util::XNumberFormatter >& _rxFormatter)
{
    lcl_setFormattedNumeric_nothrow(dynamic_cast<DoubleNumericField&>(*m_pWindow), *this, _rxField, _rxFormatter);
}


void DbNumericField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbNumericField::updateFromModel: invalid call!" );

    double dValue = 0;
    if ( _rxModel->getPropertyValue( FM_PROP_VALUE ) >>= dValue )
        static_cast< DoubleNumericField* >( m_pWindow.get() )->SetValue( dValue );
    else
        m_pWindow->SetText( OUString() );
}


bool DbNumericField::commitControl()
{
    OUString aText( m_pWindow->GetText());
    Any aVal;

    if (!aText.isEmpty())   // not empty
    {
        double fValue = static_cast<DoubleNumericField*>(m_pWindow.get())->GetValue();
        aVal <<= fValue;
    }
    m_rColumn.getModel()->setPropertyValue(FM_PROP_VALUE, aVal);
    return true;
}

DbCurrencyField::DbCurrencyField(DbGridColumn& _rColumn)
    :DbSpinField( _rColumn )
    ,m_nScale( 0 )
{
    doPropertyListening( FM_PROP_DECIMAL_ACCURACY );
    doPropertyListening( FM_PROP_VALUEMIN );
    doPropertyListening( FM_PROP_VALUEMAX );
    doPropertyListening( FM_PROP_VALUESTEP );
    doPropertyListening( FM_PROP_STRICTFORMAT );
    doPropertyListening( FM_PROP_SHOWTHOUSANDSEP );
    doPropertyListening( FM_PROP_CURRENCYSYMBOL );
}


void DbCurrencyField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbCurrencyField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbCurrencyField::implAdjustGenericFieldSetting: invalid model!" );
    if ( !m_pWindow || !_rxModel.is() )
        return;

    m_nScale                = getINT16( _rxModel->getPropertyValue( FM_PROP_DECIMAL_ACCURACY ) );
    double  nMin            = getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMIN ) );
    double  nMax            = getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMAX ) );
    double  nStep           = getDouble( _rxModel->getPropertyValue( FM_PROP_VALUESTEP ) );
    bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );
    bool    bThousand   = getBOOL( _rxModel->getPropertyValue( FM_PROP_SHOWTHOUSANDSEP ) );
    OUString aStr( getString( _rxModel->getPropertyValue(FM_PROP_CURRENCYSYMBOL ) ) );

    //fdo#42747 the min/max/first/last of vcl NumericFormatters needs to be
    //multiplied by the no of decimal places. See also
    //VclBuilder::mungeAdjustment
    int nMul = rtl_math_pow10Exp(1, m_nScale);
    nMin *= nMul;
    nMax *= nMul;

    static_cast< LongCurrencyField* >( m_pWindow.get() )->SetUseThousandSep( bThousand );
    static_cast< LongCurrencyField* >( m_pWindow.get() )->SetDecimalDigits( m_nScale );
    static_cast< LongCurrencyField* >( m_pWindow.get() )->SetCurrencySymbol( aStr );
    static_cast< LongCurrencyField* >( m_pWindow.get() )->SetFirst( nMin );
    static_cast< LongCurrencyField* >( m_pWindow.get() )->SetLast( nMax );
    static_cast< LongCurrencyField* >( m_pWindow.get() )->SetMin( nMin );
    static_cast< LongCurrencyField* >( m_pWindow.get() )->SetMax( nMax );
    static_cast< LongCurrencyField* >( m_pWindow.get() )->SetSpinSize( nStep );
    static_cast< LongCurrencyField* >( m_pWindow.get() )->SetStrictFormat( bStrict );

    static_cast< LongCurrencyField* >( m_pPainter.get() )->SetUseThousandSep( bThousand );
    static_cast< LongCurrencyField* >( m_pPainter.get() )->SetDecimalDigits( m_nScale );
    static_cast< LongCurrencyField* >( m_pPainter.get() )->SetCurrencySymbol( aStr );
    static_cast< LongCurrencyField* >( m_pPainter.get() )->SetFirst( nMin );
    static_cast< LongCurrencyField* >( m_pPainter.get() )->SetLast( nMax );
    static_cast< LongCurrencyField* >( m_pPainter.get() )->SetMin( nMin );
    static_cast< LongCurrencyField* >( m_pPainter.get() )->SetMax( nMax );
    static_cast< LongCurrencyField* >( m_pPainter.get() )->SetStrictFormat( bStrict );
}


VclPtr<SpinField> DbCurrencyField::createField( vcl::Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& /*_rxModel*/  )
{
    return VclPtr<LongCurrencyField>::Create( _pParent, _nFieldStyle );
}


double DbCurrencyField::GetCurrency(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter) const
{
    double fValue = GetValue(_rxField, xFormatter);
    if (m_nScale)
    {
        // SAL_INFO("svx",("double = %.64f ",fValue);
        fValue = ::rtl::math::pow10Exp(fValue, m_nScale);
        fValue = ::rtl::math::round(fValue);
    }
    return fValue;
}

namespace
{

    OUString lcl_setFormattedCurrency_nothrow( LongCurrencyField& _rField, const DbCurrencyField& _rControl,
        const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& _rxFormatter )
    {
        OUString sValue;
        if ( _rxField.is() )
        {
            try
            {
                double fValue = _rControl.GetCurrency( _rxField, _rxFormatter );
                if ( !_rxField->wasNull() )
                {
                    _rField.SetValue( fValue );
                    sValue = _rField.GetText();
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
        }
        return sValue;
    }
}


OUString DbCurrencyField::GetFormatText(const Reference< css::sdb::XColumn >& _rxField, const Reference< css::util::XNumberFormatter >& _rxFormatter, Color** /*ppColor*/)
{
    return lcl_setFormattedCurrency_nothrow( dynamic_cast< LongCurrencyField& >( *m_pPainter ), *this, _rxField, _rxFormatter );
}


void DbCurrencyField::UpdateFromField(const Reference< css::sdb::XColumn >& _rxField, const Reference< css::util::XNumberFormatter >& _rxFormatter)
{
    lcl_setFormattedCurrency_nothrow( dynamic_cast< LongCurrencyField& >( *m_pWindow ), *this, _rxField, _rxFormatter );
}


void DbCurrencyField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbCurrencyField::updateFromModel: invalid call!" );

    double dValue = 0;
    if ( _rxModel->getPropertyValue( FM_PROP_VALUE ) >>= dValue )
    {
        if ( m_nScale )
        {
            dValue = ::rtl::math::pow10Exp( dValue, m_nScale );
            dValue = ::rtl::math::round(dValue);
        }

        static_cast< LongCurrencyField* >( m_pWindow.get() )->SetValue( dValue );
    }
    else
        m_pWindow->SetText( OUString() );
}


bool DbCurrencyField::commitControl()
{
    OUString aText(m_pWindow->GetText());
    Any aVal;
    if (!aText.isEmpty())   // not empty
    {
        double fValue = static_cast<LongCurrencyField*>(m_pWindow.get())->GetValue();
        if (m_nScale)
        {
            fValue /= ::rtl::math::pow10Exp(1.0, m_nScale);
        }
        aVal <<= fValue;
    }
    m_rColumn.getModel()->setPropertyValue(FM_PROP_VALUE, aVal);
    return true;
}

DbDateField::DbDateField( DbGridColumn& _rColumn )
    :DbSpinField( _rColumn )
{
    doPropertyListening( FM_PROP_DATEFORMAT );
    doPropertyListening( FM_PROP_DATEMIN );
    doPropertyListening( FM_PROP_DATEMAX );
    doPropertyListening( FM_PROP_STRICTFORMAT );
    doPropertyListening( FM_PROP_DATE_SHOW_CENTURY );
}


VclPtr<SpinField> DbDateField::createField( vcl::Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& _rxModel  )
{
    // check if there is a DropDown property set to TRUE
    bool bDropDown =    !hasProperty( FM_PROP_DROPDOWN, _rxModel )
                        ||  getBOOL( _rxModel->getPropertyValue( FM_PROP_DROPDOWN ) );
    if ( bDropDown )
        _nFieldStyle |= WB_DROPDOWN;

    VclPtr<CalendarField> pField = VclPtr<CalendarField>::Create( _pParent, _nFieldStyle );

    pField->EnableToday();
    pField->EnableNone();

    return pField;
}


void DbDateField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbDateField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbDateField::implAdjustGenericFieldSetting: invalid model!" );
    if ( !m_pWindow || !_rxModel.is() )
        return;

    sal_Int16   nFormat     = getINT16( _rxModel->getPropertyValue( FM_PROP_DATEFORMAT ) );
    util::Date  aMin;
    OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_DATEMIN ) >>= aMin );
    util::Date  aMax;
    OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_DATEMAX ) >>= aMax );
    bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );

    Any  aCentury = _rxModel->getPropertyValue( FM_PROP_DATE_SHOW_CENTURY );
    if ( aCentury.getValueType().getTypeClass() != TypeClass_VOID )
    {
        bool bShowDateCentury = getBOOL( aCentury );

        static_cast<DateField*>( m_pWindow.get() )->SetShowDateCentury( bShowDateCentury );
        static_cast<DateField*>( m_pPainter.get() )->SetShowDateCentury( bShowDateCentury );
    }

    static_cast< DateField* >( m_pWindow.get() )->SetExtDateFormat( static_cast<ExtDateFieldFormat>(nFormat) );
    static_cast< DateField* >( m_pWindow.get() )->SetMin( aMin );
    static_cast< DateField* >( m_pWindow.get() )->SetMax( aMax );
    static_cast< DateField* >( m_pWindow.get() )->SetStrictFormat( bStrict );
    static_cast< DateField* >( m_pWindow.get() )->EnableEmptyFieldValue( true );

    static_cast< DateField* >( m_pPainter.get() )->SetExtDateFormat( static_cast<ExtDateFieldFormat>(nFormat) );
    static_cast< DateField* >( m_pPainter.get() )->SetMin( aMin );
    static_cast< DateField* >( m_pPainter.get() )->SetMax( aMax );
    static_cast< DateField* >( m_pPainter.get() )->SetStrictFormat( bStrict );
    static_cast< DateField* >( m_pPainter.get() )->EnableEmptyFieldValue( true );
}

namespace
{

    OUString lcl_setFormattedDate_nothrow( DateField& _rField, const Reference< XColumn >& _rxField )
    {
        OUString sDate;
        if ( _rxField.is() )
        {
            try
            {
                css::util::Date aValue = _rxField->getDate();
                if ( _rxField->wasNull() )
                    _rField.SetText( sDate );
                else
                {
                    _rField.SetDate( ::Date( aValue.Day, aValue.Month, aValue.Year ) );
                    sDate = _rField.GetText();
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
        }
        return sDate;
    }
}

OUString DbDateField::GetFormatText(const Reference< css::sdb::XColumn >& _rxField, const Reference< css::util::XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
     return lcl_setFormattedDate_nothrow(dynamic_cast<DateField&>(*m_pPainter.get()), _rxField);
}


void DbDateField::UpdateFromField(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    lcl_setFormattedDate_nothrow(dynamic_cast<DateField&>(*m_pWindow.get()), _rxField);
}


void DbDateField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbDateField::updateFromModel: invalid call!" );

    util::Date aDate;
    if ( _rxModel->getPropertyValue( FM_PROP_DATE ) >>= aDate )
        static_cast< DateField* >( m_pWindow.get() )->SetDate( ::Date( aDate ) );
    else
        static_cast< DateField* >( m_pWindow.get() )->SetText( OUString() );
}


bool DbDateField::commitControl()
{
    OUString aText(m_pWindow->GetText());
    Any aVal;
    if (!aText.isEmpty())
        aVal <<= static_cast<DateField*>(m_pWindow.get())->GetDate().GetUNODate();
    else
        aVal.clear();

    m_rColumn.getModel()->setPropertyValue(FM_PROP_DATE, aVal);
    return true;
}

DbTimeField::DbTimeField( DbGridColumn& _rColumn )
    :DbSpinField( _rColumn, css::awt::TextAlign::LEFT )
{
    doPropertyListening( FM_PROP_TIMEFORMAT );
    doPropertyListening( FM_PROP_TIMEMIN );
    doPropertyListening( FM_PROP_TIMEMAX );
    doPropertyListening( FM_PROP_STRICTFORMAT );
}


VclPtr<SpinField> DbTimeField::createField( vcl::Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& /*_rxModel*/ )
{
    return VclPtr<TimeField>::Create( _pParent, _nFieldStyle );
}


void DbTimeField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbTimeField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbTimeField::implAdjustGenericFieldSetting: invalid model!" );
    if ( !m_pWindow || !_rxModel.is() )
        return;

    sal_Int16   nFormat     = getINT16( _rxModel->getPropertyValue( FM_PROP_TIMEFORMAT ) );
    util::Time  aMin;
    OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_TIMEMIN ) >>= aMin );
    util::Time  aMax;
    OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_TIMEMAX ) >>= aMax );
    bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );

    static_cast< TimeField* >( m_pWindow.get() )->SetExtFormat( static_cast<ExtTimeFieldFormat>(nFormat) );
    static_cast< TimeField* >( m_pWindow.get() )->SetMin( aMin );
    static_cast< TimeField* >( m_pWindow.get() )->SetMax( aMax );
    static_cast< TimeField* >( m_pWindow.get() )->SetStrictFormat( bStrict );
    static_cast< TimeField* >( m_pWindow.get() )->EnableEmptyFieldValue( true );

    static_cast< TimeField* >( m_pPainter.get() )->SetExtFormat( static_cast<ExtTimeFieldFormat>(nFormat) );
    static_cast< TimeField* >( m_pPainter.get() )->SetMin( aMin );
    static_cast< TimeField* >( m_pPainter.get() )->SetMax( aMax );
    static_cast< TimeField* >( m_pPainter.get() )->SetStrictFormat( bStrict );
    static_cast< TimeField* >( m_pPainter.get() )->EnableEmptyFieldValue( true );
}

namespace
{

    OUString lcl_setFormattedTime_nothrow( TimeField& _rField, const Reference< XColumn >& _rxField )
    {
        OUString sTime;
        if ( _rxField.is() )
        {
            try
            {
                css::util::Time aValue = _rxField->getTime();
                if ( _rxField->wasNull() )
                    _rField.SetText( sTime );
                else
                {
                    _rField.SetTime( ::tools::Time( aValue ) );
                    sTime = _rField.GetText();
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
        }
        return sTime;
    }
}

OUString DbTimeField::GetFormatText(const Reference< css::sdb::XColumn >& _rxField, const Reference< css::util::XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
    return lcl_setFormattedTime_nothrow( *static_cast< TimeField* >( m_pPainter.get() ), _rxField );
}


void DbTimeField::UpdateFromField(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    lcl_setFormattedTime_nothrow( *static_cast< TimeField* >( m_pWindow.get() ), _rxField );
}


void DbTimeField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbTimeField::updateFromModel: invalid call!" );

    util::Time aTime;
    if ( _rxModel->getPropertyValue( FM_PROP_TIME ) >>= aTime )
        static_cast< TimeField* >( m_pWindow.get() )->SetTime( ::tools::Time( aTime ) );
    else
        static_cast< TimeField* >( m_pWindow.get() )->SetText( OUString() );
}


bool DbTimeField::commitControl()
{
    OUString aText(m_pWindow->GetText());
    Any aVal;
    if (!aText.isEmpty())
        aVal <<= static_cast<TimeField*>(m_pWindow.get())->GetTime().GetUNOTime();
    else
        aVal.clear();

    m_rColumn.getModel()->setPropertyValue(FM_PROP_TIME, aVal);
    return true;
}

DbComboBox::DbComboBox(DbGridColumn& _rColumn)
           :DbCellControl(_rColumn)
{
    setAlignedController( false );

    doPropertyListening( FM_PROP_STRINGITEMLIST );
    doPropertyListening( FM_PROP_LINECOUNT );
}


void DbComboBox::_propertyChanged( const PropertyChangeEvent& _rEvent )
{
    if ( _rEvent.PropertyName == FM_PROP_STRINGITEMLIST )
    {
        SetList(_rEvent.NewValue);
    }
    else
    {
        DbCellControl::_propertyChanged( _rEvent ) ;
    }
}


void DbComboBox::SetList(const Any& rItems)
{
    ComboBoxControl* pField = static_cast<ComboBoxControl*>(m_pWindow.get());
    pField->Clear();

    css::uno::Sequence<OUString> aTest;
    if (rItems >>= aTest)
    {
        const OUString* pStrings = aTest.getConstArray();
        sal_Int32 nItems = aTest.getLength();
        for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
             pField->InsertEntry(*pStrings);

        // tell the grid control that this controller is invalid and has to be re-initialized
        invalidatedController();
    }
}


void DbComboBox::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbComboBox::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbComboBox::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Int16  nLines = getINT16( _rxModel->getPropertyValue( FM_PROP_LINECOUNT ) );
        static_cast< ComboBoxControl* >( m_pWindow.get() )->SetDropDownLineCount( nLines );
    }
}


void DbComboBox::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor )
{
    m_rColumn.SetAlignmentFromModel(css::awt::TextAlign::LEFT);

    m_pWindow = VclPtr<ComboBoxControl>::Create( &rParent );

    // selection from right to left
    AllSettings     aSettings = m_pWindow->GetSettings();
    StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
    aStyleSettings.SetSelectionOptions(
        aStyleSettings.GetSelectionOptions() | SelectionOptions::ShowFirst);
    aSettings.SetStyleSettings(aStyleSettings);
    m_pWindow->SetSettings(aSettings, true);

    // some initial properties
    Reference< XPropertySet >   xModel(m_rColumn.getModel());
    SetList( xModel->getPropertyValue( FM_PROP_STRINGITEMLIST ) );
    implAdjustGenericFieldSetting( xModel );

    DbCellControl::Init( rParent, xCursor );
}


CellControllerRef DbComboBox::CreateController() const
{
    return new ComboBoxCellController(static_cast<ComboBoxControl*>(m_pWindow.get()));
}


OUString DbComboBox::GetFormatText(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter, Color** /*ppColor*/)
{
    const css::uno::Reference<css::beans::XPropertySet> xPS(_rxField, UNO_QUERY);
    ::dbtools::FormattedColumnValue fmter( xFormatter, xPS );

    return fmter.getFormattedValue();
}


void DbComboBox::UpdateFromField(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter)
{
    m_pWindow->SetText(GetFormatText(_rxField, xFormatter));
}


void DbComboBox::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbComboBox::updateFromModel: invalid call!" );

    OUString sText;
    _rxModel->getPropertyValue( FM_PROP_TEXT ) >>= sText;

    static_cast< ComboBox* >( m_pWindow.get() )->SetText( sText );
    static_cast< ComboBox* >( m_pWindow.get() )->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}


bool DbComboBox::commitControl()
{
    OUString aText( m_pWindow->GetText());
    m_rColumn.getModel()->setPropertyValue(FM_PROP_TEXT, makeAny(aText));
    return true;
}


DbListBox::DbListBox(DbGridColumn& _rColumn)
          :DbCellControl(_rColumn)
          ,m_bBound(false)
{
    setAlignedController( false );

    doPropertyListening( FM_PROP_STRINGITEMLIST );
    doPropertyListening( FM_PROP_LINECOUNT );
}


void DbListBox::_propertyChanged( const css::beans::PropertyChangeEvent& _rEvent )
{
    if ( _rEvent.PropertyName == FM_PROP_STRINGITEMLIST )
    {
        SetList(_rEvent.NewValue);
    }
    else
    {
        DbCellControl::_propertyChanged( _rEvent ) ;
    }
}


void DbListBox::SetList(const Any& rItems)
{
    ListBoxControl* pField = static_cast<ListBoxControl*>(m_pWindow.get());

    pField->Clear();
    m_bBound = false;

    css::uno::Sequence<OUString> aTest;
    if (rItems >>= aTest)
    {
        const OUString* pStrings = aTest.getConstArray();
        sal_Int32 nItems = aTest.getLength();
        if (nItems)
        {
            for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
                 pField->InsertEntry(*pStrings);

            m_rColumn.getModel()->getPropertyValue(FM_PROP_VALUE_SEQ) >>= m_aValueList;
            m_bBound = m_aValueList.getLength() > 0;

            // tell the grid control that this controller is invalid and has to be re-initialized
            invalidatedController();
        }
    }
}


void DbListBox::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor)
{
    m_rColumn.SetAlignment(css::awt::TextAlign::LEFT);

    m_pWindow = VclPtr<ListBoxControl>::Create( &rParent );

    // some initial properties
    Reference< XPropertySet > xModel( m_rColumn.getModel() );
    SetList( xModel->getPropertyValue( FM_PROP_STRINGITEMLIST ) );
    implAdjustGenericFieldSetting( xModel );

    DbCellControl::Init( rParent, xCursor );
}


void DbListBox::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbListBox::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbListBox::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Int16  nLines   = getINT16( _rxModel->getPropertyValue( FM_PROP_LINECOUNT ) );
        static_cast< ListBoxControl* >( m_pWindow.get() )->SetDropDownLineCount( nLines );
    }
}


CellControllerRef DbListBox::CreateController() const
{
    return new ListBoxCellController(static_cast<ListBoxControl*>(m_pWindow.get()));
}


OUString DbListBox::GetFormatText(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
    OUString sText;
    if ( _rxField.is() )
    {
        try
        {
            sText = _rxField->getString();
            if ( m_bBound )
            {
                sal_Int32 nPos = ::comphelper::findValue( m_aValueList, sText );
                if ( nPos != -1 )
                    sText = static_cast<ListBox*>(m_pWindow.get())->GetEntry(nPos);
                else
                    sText.clear();
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }
    return sText;
}


void DbListBox::UpdateFromField(const Reference< css::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter)
{
    OUString sFormattedText( GetFormatText( _rxField, xFormatter ) );
    if (!sFormattedText.isEmpty())
        static_cast< ListBox* >( m_pWindow.get() )->SelectEntry( sFormattedText );
    else
        static_cast< ListBox* >( m_pWindow.get() )->SetNoSelection();
}


void DbListBox::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbListBox::updateFromModel: invalid call!" );

    Sequence< sal_Int16 > aSelection;
    _rxModel->getPropertyValue( FM_PROP_SELECT_SEQ ) >>= aSelection;

    sal_Int16 nSelection = -1;
    if ( aSelection.getLength() > 0 )
        nSelection = aSelection[ 0 ];

    ListBox* pListBox = static_cast< ListBox* >( m_pWindow.get() );

    if ( ( nSelection >= 0 ) && ( nSelection < pListBox->GetEntryCount() ) )
        pListBox->SelectEntryPos( nSelection );
    else
        pListBox->SetNoSelection( );
}


bool DbListBox::commitControl()
{
    Any aVal;
    Sequence<sal_Int16> aSelectSeq;
    if (static_cast<ListBox*>(m_pWindow.get())->GetSelectedEntryCount())
    {
        aSelectSeq.realloc(1);
        *aSelectSeq.getArray() = static_cast<sal_Int16>(static_cast<ListBox*>(m_pWindow.get())->GetSelectedEntryPos());
    }
    aVal <<= aSelectSeq;
    m_rColumn.getModel()->setPropertyValue(FM_PROP_SELECT_SEQ, aVal);
    return true;
}

DbFilterField::DbFilterField(const Reference< XComponentContext >& rxContext,DbGridColumn& _rColumn)
              :DbCellControl(_rColumn)
              ,OSQLParserClient(rxContext)
              ,m_nControlClass(css::form::FormComponentType::TEXTFIELD)
              ,m_bFilterList(false)
              ,m_bFilterListFilled(false)
{

    setAlignedController( false );
}


DbFilterField::~DbFilterField()
{
    if (m_nControlClass == css::form::FormComponentType::CHECKBOX)
        static_cast<CheckBoxControl*>(m_pWindow.get())->SetClickHdl( Link<VclPtr<CheckBox>,void>() );

}


void DbFilterField::PaintCell(OutputDevice& rDev, const tools::Rectangle& rRect)
{
    static const DrawTextFlags nStyle = DrawTextFlags::Clip | DrawTextFlags::VCenter | DrawTextFlags::Left;
    switch (m_nControlClass)
    {
        case FormComponentType::CHECKBOX:
            DbCellControl::PaintCell( rDev, rRect );
            break;
        case FormComponentType::LISTBOX:
            rDev.DrawText(rRect, static_cast<ListBox*>(m_pWindow.get())->GetSelectedEntry(), nStyle);
            break;
        default:
            rDev.DrawText(rRect, m_aText, nStyle);
    }
}


void DbFilterField::SetList(const Any& rItems, bool bComboBox)
{
    css::uno::Sequence<OUString> aTest;
    rItems >>= aTest;
    const OUString* pStrings = aTest.getConstArray();
    sal_Int32 nItems = aTest.getLength();
    if (nItems)
    {
        if (bComboBox)
        {
            ComboBox* pField = static_cast<ComboBox*>(m_pWindow.get());
            for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
                pField->InsertEntry(*pStrings);
        }
        else
        {
            ListBox* pField = static_cast<ListBox*>(m_pWindow.get());
            for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
                pField->InsertEntry(*pStrings);

            m_rColumn.getModel()->getPropertyValue(FM_PROP_VALUE_SEQ) >>= m_aValueList;
        }
    }
}


void DbFilterField::CreateControl(vcl::Window* pParent, const Reference< css::beans::XPropertySet >& xModel)
{
    switch (m_nControlClass)
    {
        case css::form::FormComponentType::CHECKBOX:
            m_pWindow = VclPtr<CheckBoxControl>::Create(pParent);
            m_pWindow->SetPaintTransparent( true );
            static_cast<CheckBoxControl*>(m_pWindow.get())->SetClickHdl( LINK( this, DbFilterField, OnClick ) );

            m_pPainter = VclPtr<CheckBoxControl>::Create(pParent);
            m_pPainter->SetPaintTransparent( true );
            m_pPainter->SetBackground();
            break;
        case css::form::FormComponentType::LISTBOX:
        {
            m_pWindow = VclPtr<ListBoxControl>::Create(pParent);
            sal_Int16  nLines       = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_LINECOUNT));
            Any  aItems      = xModel->getPropertyValue(FM_PROP_STRINGITEMLIST);
            SetList(aItems, m_nControlClass == css::form::FormComponentType::COMBOBOX);
            static_cast<ListBox*>(m_pWindow.get())->SetDropDownLineCount(nLines);
        }   break;
        case css::form::FormComponentType::COMBOBOX:
        {
            m_pWindow = VclPtr<ComboBoxControl>::Create(pParent);

            AllSettings     aSettings = m_pWindow->GetSettings();
            StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                           aStyleSettings.GetSelectionOptions() | SelectionOptions::ShowFirst);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings, true);

            if (!m_bFilterList)
            {
                sal_Int16  nLines       = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_LINECOUNT));
                Any  aItems      = xModel->getPropertyValue(FM_PROP_STRINGITEMLIST);
                SetList(aItems, m_nControlClass == css::form::FormComponentType::COMBOBOX);
                static_cast<ComboBox*>(m_pWindow.get())->SetDropDownLineCount(nLines);
            }
            else
                static_cast<ComboBox*>(m_pWindow.get())->SetDropDownLineCount(5);

        }   break;
        default:
        {
            m_pWindow  = VclPtr<Edit>::Create(pParent, WB_LEFT);
            AllSettings     aSettings = m_pWindow->GetSettings();
            StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                           aStyleSettings.GetSelectionOptions() | SelectionOptions::ShowFirst);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings, true);
        }
    }
}


void DbFilterField::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor )
{
    Reference< css::beans::XPropertySet >  xModel(m_rColumn.getModel());
    m_rColumn.SetAlignment(css::awt::TextAlign::LEFT);

    if (xModel.is())
    {
        m_bFilterList = ::comphelper::hasProperty(FM_PROP_FILTERPROPOSAL, xModel) && ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_FILTERPROPOSAL));
        if (m_bFilterList)
            m_nControlClass = css::form::FormComponentType::COMBOBOX;
        else
        {
            sal_Int16 nClassId = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_CLASSID));
            switch (nClassId)
            {
                case FormComponentType::CHECKBOX:
                case FormComponentType::LISTBOX:
                case FormComponentType::COMBOBOX:
                    m_nControlClass = nClassId;
                    break;
                default:
                    if (m_bFilterList)
                        m_nControlClass = FormComponentType::COMBOBOX;
                    else
                        m_nControlClass = FormComponentType::TEXTFIELD;
            }
        }
    }

    CreateControl( &rParent, xModel );
    DbCellControl::Init( rParent, xCursor );

    // filter cells are never readonly
    Edit* pAsEdit = dynamic_cast< Edit* >( m_pWindow.get() );
    if ( pAsEdit )
        pAsEdit->SetReadOnly( false );
}


CellControllerRef DbFilterField::CreateController() const
{
    CellControllerRef xController;
    switch (m_nControlClass)
    {
        case css::form::FormComponentType::CHECKBOX:
            xController = new CheckBoxCellController(static_cast<CheckBoxControl*>(m_pWindow.get()));
            break;
        case css::form::FormComponentType::LISTBOX:
            xController = new ListBoxCellController(static_cast<ListBoxControl*>(m_pWindow.get()));
            break;
        case css::form::FormComponentType::COMBOBOX:
            xController = new ComboBoxCellController(static_cast<ComboBoxControl*>(m_pWindow.get()));
            break;
        default:
            if (m_bFilterList)
                xController = new ComboBoxCellController(static_cast<ComboBoxControl*>(m_pWindow.get()));
            else
                xController = new EditCellController(static_cast<Edit*>(m_pWindow.get()));
    }
    return xController;
}


void DbFilterField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbFilterField::updateFromModel: invalid call!" );

    OSL_FAIL( "DbListBox::updateFromModel: not implemented yet (how the hell did you reach this?)!" );
    // TODO: implement this.
    // remember: updateFromModel should be some kind of opposite of commitControl
}


bool DbFilterField::commitControl()
{
    OUString aText(m_aText);
    switch (m_nControlClass)
    {
        case css::form::FormComponentType::CHECKBOX:
            return true;
        case css::form::FormComponentType::LISTBOX:
            aText.clear();
            if (static_cast<ListBox*>(m_pWindow.get())->GetSelectedEntryCount())
            {
                sal_Int16 nPos = static_cast<sal_Int16>(static_cast<ListBox*>(m_pWindow.get())->GetSelectedEntryPos());
                if ( ( nPos >= 0 ) && ( nPos < m_aValueList.getLength() ) )
                    aText = m_aValueList.getConstArray()[nPos];
            }

            if (m_aText != aText)
            {
                m_aText = aText;
                m_aCommitLink.Call(*this);
            }
            return true;
        default:
            aText = m_pWindow->GetText();
    }

    if (m_aText != aText)
    {
        // check the text with the SQL-Parser
        OUString aNewText(comphelper::string::stripEnd(aText, ' '));
        if (!aNewText.isEmpty())
        {
            OUString aErrorMsg;
            Reference< XNumberFormatter >  xNumberFormatter(m_rColumn.GetParent().getNumberFormatter());

            std::unique_ptr< OSQLParseNode > pParseNode = predicateTree(aErrorMsg, aNewText,xNumberFormatter, m_rColumn.GetField());
            if (pParseNode != nullptr)
            {
                OUString aPreparedText;

                css::lang::Locale aAppLocale = Application::GetSettings().GetUILanguageTag().getLocale();

                Reference< XRowSet > xDataSourceRowSet(
                    Reference< XInterface >(*m_rColumn.GetParent().getDataSource()), UNO_QUERY);
                Reference< XConnection >  xConnection(getConnection(xDataSourceRowSet));

                pParseNode->parseNodeToPredicateStr(aPreparedText,
                                                    xConnection,
                                                    xNumberFormatter,
                                                    m_rColumn.GetField(),
                                                    OUString(),
                                                    aAppLocale,
                                                    '.',
                                                    getParseContext());
                m_aText = aPreparedText;
            }
            else
            {

                SQLException aError;
                aError.Message = aErrorMsg;
                displayException(aError, m_pWindow->GetParent());
                    // TODO: transport the title

                return false;
            }
        }
        else
            m_aText = aText;

        m_pWindow->SetText(m_aText);
        m_aCommitLink.Call(*this);
    }
    return true;
}


void DbFilterField::SetText(const OUString& rText)
{
    m_aText = rText;
    switch (m_nControlClass)
    {
        case css::form::FormComponentType::CHECKBOX:
        {
            TriState eState;
            if (rText == "1")
                eState = TRISTATE_TRUE;
            else if (rText == "0")
                eState = TRISTATE_FALSE;
            else
                eState = TRISTATE_INDET;

            static_cast<CheckBoxControl*>(m_pWindow.get())->GetBox().SetState(eState);
            static_cast<CheckBoxControl*>(m_pPainter.get())->GetBox().SetState(eState);
        }   break;
        case css::form::FormComponentType::LISTBOX:
        {
            sal_Int32 nPos = ::comphelper::findValue(m_aValueList, m_aText);
            if (nPos != -1)
                static_cast<ListBox*>(m_pWindow.get())->SelectEntryPos(nPos);
            else
                static_cast<ListBox*>(m_pWindow.get())->SetNoSelection();
        }   break;
        default:
            m_pWindow->SetText(m_aText);
    }

    // now force a repaint on the window
    m_rColumn.GetParent().RowModified(0);
}


void DbFilterField::Update()
{
    // should we fill the combobox with a filter proposal?
    if (!m_bFilterList || m_bFilterListFilled)
        return;

    m_bFilterListFilled = true;
    Reference< css::beans::XPropertySet >  xField = m_rColumn.GetField();
    if (!xField.is())
        return;

    OUString aName;
    xField->getPropertyValue(FM_PROP_NAME) >>= aName;

    // the columnmodel
    Reference< css::container::XChild >  xModelAsChild(m_rColumn.getModel(), UNO_QUERY);
    // the grid model
    xModelAsChild.set(xModelAsChild->getParent(),UNO_QUERY);
    Reference< XRowSet >  xForm(xModelAsChild->getParent(), UNO_QUERY);
    if (!xForm.is())
        return;

    Reference<XPropertySet> xFormProp(xForm,UNO_QUERY);
    Reference< XTablesSupplier > xSupTab;
    xFormProp->getPropertyValue("SingleSelectQueryComposer") >>= xSupTab;

    Reference< XConnection >  xConnection(getConnection(xForm));
    if (!xSupTab.is())
        return;

    // search the field
    Reference< XColumnsSupplier > xSupCol(xSupTab,UNO_QUERY);
    Reference< css::container::XNameAccess >    xFieldNames = xSupCol->getColumns();
    if (!xFieldNames->hasByName(aName))
        return;

    Reference< css::container::XNameAccess >    xTablesNames = xSupTab->getTables();
    Reference< css::beans::XPropertySet >       xComposerFieldAsSet(xFieldNames->getByName(aName),UNO_QUERY);

    if (!xComposerFieldAsSet.is() ||
        !::comphelper::hasProperty(FM_PROP_TABLENAME, xComposerFieldAsSet) ||
        !::comphelper::hasProperty(FM_PROP_FIELDSOURCE, xComposerFieldAsSet))
        return;

    OUString aFieldName;
    OUString aTableName;
    xComposerFieldAsSet->getPropertyValue(FM_PROP_FIELDSOURCE)  >>= aFieldName;
    xComposerFieldAsSet->getPropertyValue(FM_PROP_TABLENAME)    >>= aTableName;

    // no possibility to create a select statement
    // looking for the complete table name
    if (!xTablesNames->hasByName(aTableName))
        return;

    // build a statement and send as query;
    // Access to the connection
    Reference< XStatement >  xStatement;
    Reference< XResultSet >  xListCursor;
    Reference< css::sdb::XColumn >  xDataField;

    try
    {
        Reference< XDatabaseMetaData >  xMeta = xConnection->getMetaData();

        OUString aQuote(xMeta->getIdentifierQuoteString());
        OUStringBuffer aStatement("SELECT DISTINCT ");
        aStatement.append(quoteName(aQuote, aName));
        if (!aFieldName.isEmpty() && aName != aFieldName)
        {
            aStatement.append(" AS ");
            aStatement.append(quoteName(aQuote, aFieldName));
        }

        aStatement.append(" FROM ");

        Reference< XPropertySet > xTableNameAccess(xTablesNames->getByName(aTableName), UNO_QUERY_THROW);
        aStatement.append(composeTableNameForSelect(xConnection, xTableNameAccess));

        xStatement = xConnection->createStatement();
        Reference< css::beans::XPropertySet >  xStatementProps(xStatement, UNO_QUERY);
        xStatementProps->setPropertyValue(FM_PROP_ESCAPE_PROCESSING, makeAny(true));

        xListCursor = xStatement->executeQuery(aStatement.makeStringAndClear());

        Reference< css::sdbcx::XColumnsSupplier >  xSupplyCols(xListCursor, UNO_QUERY);
        Reference< css::container::XIndexAccess >  xFields(xSupplyCols->getColumns(), UNO_QUERY);
        xDataField.set(xFields->getByIndex(0), css::uno::UNO_QUERY);
        if (!xDataField.is())
            return;
    }
    catch(const Exception&)
    {
        ::comphelper::disposeComponent(xStatement);
        return;
    }

    sal_Int16 i = 0;
    ::std::vector< OUString >   aStringList;
    aStringList.reserve(16);
    OUString aStr;
    css::util::Date aNullDate = m_rColumn.GetParent().getNullDate();
    sal_Int32 nFormatKey = m_rColumn.GetKey();
    Reference< XNumberFormatter >  xFormatter = m_rColumn.GetParent().getNumberFormatter();
    sal_Int16 nKeyType = ::comphelper::getNumberFormatType(xFormatter->getNumberFormatsSupplier()->getNumberFormats(), nFormatKey);

    while (!xListCursor->isAfterLast() && i++ < SHRT_MAX) // max number of entries
    {
        aStr = getFormattedValue(xDataField, xFormatter, aNullDate, nFormatKey, nKeyType);
        aStringList.push_back(aStr);
        (void)xListCursor->next();
    }

    // filling the entries for the combobox
    for (const auto& rString : aStringList)
        static_cast<ComboBox*>(m_pWindow.get())->InsertEntry(rString);
}


OUString DbFilterField::GetFormatText(const Reference< XColumn >& /*_rxField*/, const Reference< XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
    return OUString();
}


void DbFilterField::UpdateFromField(const Reference< XColumn >& /*_rxField*/, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    OSL_FAIL( "DbFilterField::UpdateFromField: cannot update a filter control from a field!" );
}


IMPL_LINK_NOARG(DbFilterField, OnClick, VclPtr<CheckBox>, void)
{
    TriState eState = static_cast<CheckBoxControl*>(m_pWindow.get())->GetBox().GetState();
    OUStringBuffer aTextBuf;

    Reference< XRowSet > xDataSourceRowSet(
                    Reference< XInterface >(*m_rColumn.GetParent().getDataSource()), UNO_QUERY);
    Reference< XConnection >  xConnection(getConnection(xDataSourceRowSet));
    const sal_Int32 nBooleanComparisonMode = ::dbtools::DatabaseMetaData( xConnection ).getBooleanComparisonMode();

    switch (eState)
    {
        case TRISTATE_TRUE:
            ::dbtools::getBooleanComparisonPredicate("", true, nBooleanComparisonMode, aTextBuf);
            break;
        case TRISTATE_FALSE:
            ::dbtools::getBooleanComparisonPredicate("", false, nBooleanComparisonMode, aTextBuf);
            break;
        case TRISTATE_INDET:
            break;
    }

    const OUString aText(aTextBuf.makeStringAndClear());

    if (m_aText != aText)
    {
        m_aText = aText;
        m_aCommitLink.Call(*this);
    }
}


FmXGridCell::FmXGridCell( DbGridColumn* pColumn, std::unique_ptr<DbCellControl> _pControl )
            :OComponentHelper(m_aMutex)
            ,m_pColumn(pColumn)
            ,m_pCellControl( std::move(_pControl) )
            ,m_aWindowListeners( m_aMutex )
            ,m_aFocusListeners( m_aMutex )
            ,m_aKeyListeners( m_aMutex )
            ,m_aMouseListeners( m_aMutex )
            ,m_aMouseMotionListeners( m_aMutex )
{
}


void FmXGridCell::init()
{
    vcl::Window* pEventWindow( getEventWindow() );
    if ( pEventWindow )
        pEventWindow->AddEventListener( LINK( this, FmXGridCell, OnWindowEvent ) );
}


vcl::Window* FmXGridCell::getEventWindow() const
{
    if ( m_pCellControl )
        return &m_pCellControl->GetWindow();
    return nullptr;
}


FmXGridCell::~FmXGridCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

}


void FmXGridCell::SetTextLineColor()
{
    if (m_pCellControl)
        m_pCellControl->SetTextLineColor();
}


void FmXGridCell::SetTextLineColor(const Color& _rColor)
{
    if (m_pCellControl)
        m_pCellControl->SetTextLineColor(_rColor);
}

// XTypeProvider

Sequence< Type > SAL_CALL FmXGridCell::getTypes( )
{
    Sequence< uno::Type > aTypes = ::comphelper::concatSequences(
        ::cppu::OComponentHelper::getTypes(),
        FmXGridCell_Base::getTypes()
    );
    if ( m_pCellControl )
        aTypes = ::comphelper::concatSequences(
            aTypes,
            FmXGridCell_WindowBase::getTypes()
        );
    return aTypes;
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXGridCell )

// OComponentHelper

void FmXGridCell::disposing()
{
    lang::EventObject aEvent( *this );
    m_aWindowListeners.disposeAndClear( aEvent );
    m_aFocusListeners.disposeAndClear( aEvent );
    m_aKeyListeners.disposeAndClear( aEvent );
    m_aMouseListeners.disposeAndClear( aEvent );
    m_aMouseMotionListeners.disposeAndClear( aEvent );

    OComponentHelper::disposing();
    m_pColumn = nullptr;
    m_pCellControl.reset();
}


Any SAL_CALL FmXGridCell::queryAggregation( const css::uno::Type& _rType )
{
    Any aReturn = OComponentHelper::queryAggregation( _rType );

    if ( !aReturn.hasValue() )
        aReturn = FmXGridCell_Base::queryInterface( _rType );

    if ( !aReturn.hasValue() && ( m_pCellControl != nullptr ) )
        aReturn = FmXGridCell_WindowBase::queryInterface( _rType );

    return aReturn;
}

// css::awt::XControl

Reference< XInterface >  FmXGridCell::getContext()
{
    return Reference< XInterface > ();
}


Reference< css::awt::XControlModel >  FmXGridCell::getModel()
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< css::awt::XControlModel > (m_pColumn->getModel(), UNO_QUERY);
}

// css::form::XBoundControl

sal_Bool FmXGridCell::getLock()
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return m_pColumn->isLocked();
}


void FmXGridCell::setLock(sal_Bool _bLock)
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    if (getLock() == _bLock)
        return;
    else
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        m_pColumn->setLock(_bLock);
    }
}


void SAL_CALL FmXGridCell::setPosSize( ::sal_Int32, ::sal_Int32, ::sal_Int32, ::sal_Int32, ::sal_Int16 )
{
    OSL_FAIL( "FmXGridCell::setPosSize: not implemented" );
    // not allowed to tamper with this for a grid cell
}


awt::Rectangle SAL_CALL FmXGridCell::getPosSize(  )
{
    OSL_FAIL( "FmXGridCell::getPosSize: not implemented" );
    return awt::Rectangle();
}


void SAL_CALL FmXGridCell::setVisible( sal_Bool )
{
    OSL_FAIL( "FmXGridCell::setVisible: not implemented" );
    // not allowed to tamper with this for a grid cell
}


void SAL_CALL FmXGridCell::setEnable( sal_Bool )
{
    OSL_FAIL( "FmXGridCell::setEnable: not implemented" );
    // not allowed to tamper with this for a grid cell
}


void SAL_CALL FmXGridCell::setFocus(  )
{
    OSL_FAIL( "FmXGridCell::setFocus: not implemented" );
    // not allowed to tamper with this for a grid cell
}


void SAL_CALL FmXGridCell::addWindowListener( const Reference< awt::XWindowListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aWindowListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeWindowListener( const Reference< awt::XWindowListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aWindowListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addFocusListener( const Reference< awt::XFocusListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aFocusListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeFocusListener( const Reference< awt::XFocusListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aFocusListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addKeyListener( const Reference< awt::XKeyListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aKeyListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeKeyListener( const Reference< awt::XKeyListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aKeyListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addMouseListener( const Reference< awt::XMouseListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aMouseListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeMouseListener( const Reference< awt::XMouseListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aMouseListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addMouseMotionListener( const Reference< awt::XMouseMotionListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aMouseMotionListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeMouseMotionListener( const Reference< awt::XMouseMotionListener >& _rxListener )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aMouseMotionListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addPaintListener( const Reference< awt::XPaintListener >& )
{
    OSL_FAIL( "FmXGridCell::addPaintListener: not implemented" );
}


void SAL_CALL FmXGridCell::removePaintListener( const Reference< awt::XPaintListener >& )
{
    OSL_FAIL( "FmXGridCell::removePaintListener: not implemented" );
}


IMPL_LINK( FmXGridCell, OnWindowEvent, VclWindowEvent&, _rEvent, void )
{
    ENSURE_OR_THROW( _rEvent.GetWindow(), "illegal window" );
    onWindowEvent( _rEvent.GetId(), *_rEvent.GetWindow(), _rEvent.GetData() );
}


void FmXGridCell::onFocusGained( const awt::FocusEvent& _rEvent )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aFocusListeners.notifyEach( &awt::XFocusListener::focusGained, _rEvent );
}


void FmXGridCell::onFocusLost( const awt::FocusEvent& _rEvent )
{
    checkDisposed(OComponentHelper::rBHelper.bDisposed);
    m_aFocusListeners.notifyEach( &awt::XFocusListener::focusLost, _rEvent );
}


void FmXGridCell::onWindowEvent( const VclEventId _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{
    switch ( _nEventId )
    {
    case VclEventId::ControlGetFocus:
    case VclEventId::WindowGetFocus:
    case VclEventId::ControlLoseFocus:
    case VclEventId::WindowLoseFocus:
    {
        if  (   (   _rWindow.IsCompoundControl()
                &&  (   _nEventId == VclEventId::ControlGetFocus
                    ||  _nEventId == VclEventId::ControlLoseFocus
                    )
                )
            ||  (   !_rWindow.IsCompoundControl()
                &&  (   _nEventId == VclEventId::WindowGetFocus
                    ||  _nEventId == VclEventId::WindowLoseFocus
                    )
                )
            )
        {
            if ( !m_aFocusListeners.getLength() )
                break;

            bool bFocusGained = ( _nEventId == VclEventId::ControlGetFocus ) || ( _nEventId == VclEventId::WindowGetFocus );

            awt::FocusEvent aEvent;
            aEvent.Source = *this;
            aEvent.FocusFlags = static_cast<sal_Int16>(_rWindow.GetGetFocusFlags());
            aEvent.Temporary = false;

            if ( bFocusGained )
                onFocusGained( aEvent );
            else
                onFocusLost( aEvent );
        }
    }
    break;
    case VclEventId::WindowMouseButtonDown:
    case VclEventId::WindowMouseButtonUp:
    {
        if ( !m_aMouseListeners.getLength() )
            break;

        const bool bButtonDown = ( _nEventId == VclEventId::WindowMouseButtonDown );

        awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *static_cast< const ::MouseEvent* >( _pEventData ), *this ) );
        m_aMouseListeners.notifyEach( bButtonDown ? &awt::XMouseListener::mousePressed : &awt::XMouseListener::mouseReleased, aEvent );
    }
    break;
    case VclEventId::WindowMouseMove:
    {
        const MouseEvent& rMouseEvent = *static_cast< const ::MouseEvent* >( _pEventData );
        if ( rMouseEvent.IsEnterWindow() || rMouseEvent.IsLeaveWindow() )
        {
            if ( m_aMouseListeners.getLength() != 0 )
            {
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( rMouseEvent, *this ) );
                m_aMouseListeners.notifyEach( rMouseEvent.IsEnterWindow() ? &awt::XMouseListener::mouseEntered: &awt::XMouseListener::mouseExited, aEvent );
            }
        }
        else if ( !rMouseEvent.IsEnterWindow() && !rMouseEvent.IsLeaveWindow() )
        {
            if ( m_aMouseMotionListeners.getLength() != 0 )
            {
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( rMouseEvent, *this ) );
                aEvent.ClickCount = 0;
                const bool bSimpleMove = bool( rMouseEvent.GetMode() & MouseEventModifiers::SIMPLEMOVE );
                m_aMouseMotionListeners.notifyEach( bSimpleMove ? &awt::XMouseMotionListener::mouseMoved: &awt::XMouseMotionListener::mouseDragged, aEvent );
            }
        }
    }
    break;
    case VclEventId::WindowKeyInput:
    case VclEventId::WindowKeyUp:
    {
        if ( !m_aKeyListeners.getLength() )
            break;

        const bool bKeyPressed = ( _nEventId == VclEventId::WindowKeyInput );
        awt::KeyEvent aEvent( VCLUnoHelper::createKeyEvent( *static_cast< const ::KeyEvent* >( _pEventData ), *this ) );
        m_aKeyListeners.notifyEach( bKeyPressed ? &awt::XKeyListener::keyPressed: &awt::XKeyListener::keyReleased, aEvent );
    }
    break;
    default: break;
    }
}


void FmXDataCell::PaintFieldToCell(OutputDevice& rDev, const tools::Rectangle& rRect,
                        const Reference< css::sdb::XColumn >& _rxField,
                        const Reference< XNumberFormatter >& xFormatter)
{
    m_pCellControl->PaintFieldToCell( rDev, rRect, _rxField, xFormatter );
}


void FmXDataCell::UpdateFromColumn()
{
    Reference< css::sdb::XColumn >  xField(m_pColumn->GetCurrentFieldValue());
    if (xField.is())
        m_pCellControl->UpdateFromField(xField, m_pColumn->GetParent().getNumberFormatter());
}


FmXTextCell::FmXTextCell( DbGridColumn* pColumn, std::unique_ptr<DbCellControl> pControl )
    :FmXDataCell( pColumn, std::move(pControl) )
    ,m_bFastPaint( true )
{
}


void FmXTextCell::PaintFieldToCell(OutputDevice& rDev,
                        const tools::Rectangle& rRect,
                        const Reference< css::sdb::XColumn >& _rxField,
                        const Reference< XNumberFormatter >& xFormatter)
{
    if ( !m_bFastPaint )
    {
        FmXDataCell::PaintFieldToCell( rDev, rRect, _rxField, xFormatter );
        return;
    }

    DrawTextFlags nStyle = DrawTextFlags::Clip | DrawTextFlags::VCenter;
    if ( ( rDev.GetOutDevType() == OUTDEV_WINDOW ) && !static_cast< vcl::Window& >( rDev ).IsEnabled() )
        nStyle |= DrawTextFlags::Disable;

    switch (m_pColumn->GetAlignment())
    {
        case css::awt::TextAlign::RIGHT:
            nStyle |= DrawTextFlags::Right;
            break;
        case css::awt::TextAlign::CENTER:
            nStyle |= DrawTextFlags::Center;
            break;
        default:
            nStyle |= DrawTextFlags::Left;
    }

    try
    {
        Color* pColor = nullptr;
        OUString aText = GetText(_rxField, xFormatter, &pColor);
        if (pColor != nullptr)
        {
            Color aOldTextColor( rDev.GetTextColor() );
            rDev.SetTextColor( *pColor );
            rDev.DrawText(rRect, aText, nStyle);
            rDev.SetTextColor( aOldTextColor );
        }
        else
            rDev.DrawText(rRect, aText, nStyle);
    }
    catch (const Exception& e)
    {
        SAL_WARN("svx.fmcomp", "PaintFieldToCell: caught " << e);
    }
}

FmXEditCell::FmXEditCell( DbGridColumn* pColumn, std::unique_ptr<DbCellControl> pControl )
            :FmXTextCell( pColumn, std::move(pControl) )
            ,m_aTextListeners(m_aMutex)
            ,m_aChangeListeners( m_aMutex )
            ,m_pEditImplementation( nullptr )
            ,m_bOwnEditImplementation( false )
{

    DbTextField* pTextField = dynamic_cast<DbTextField*>( m_pCellControl.get()  );
    if ( pTextField )
    {

        m_pEditImplementation = pTextField->GetEditImplementation();
        if ( !pTextField->IsSimpleEdit() )
            m_bFastPaint = false;
    }
    else
    {
        m_pEditImplementation = new EditImplementation( static_cast< Edit& >( m_pCellControl->GetWindow() ) );
        m_bOwnEditImplementation = true;
    }
}


FmXEditCell::~FmXEditCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }


}

// OComponentHelper

void FmXEditCell::disposing()
{
    css::lang::EventObject aEvt(*this);
    m_aTextListeners.disposeAndClear(aEvt);
    m_aChangeListeners.disposeAndClear(aEvt);

    m_pEditImplementation->SetModifyHdl( Link<Edit&,void>() );
    if ( m_bOwnEditImplementation )
        delete m_pEditImplementation;
    m_pEditImplementation = nullptr;

    FmXDataCell::disposing();
}


Any SAL_CALL FmXEditCell::queryAggregation( const css::uno::Type& _rType )
{
    Any aReturn = FmXTextCell::queryAggregation( _rType );

    if ( !aReturn.hasValue() )
        aReturn = FmXEditCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< css::uno::Type > SAL_CALL FmXEditCell::getTypes(  )
{
    return ::comphelper::concatSequences(
        FmXTextCell::getTypes(),
        FmXEditCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXEditCell )

// css::awt::XTextComponent

void SAL_CALL FmXEditCell::addTextListener(const Reference< css::awt::XTextListener >& l)
{
    m_aTextListeners.addInterface( l );
}


void SAL_CALL FmXEditCell::removeTextListener(const Reference< css::awt::XTextListener >& l)
{
    m_aTextListeners.removeInterface( l );
}


void SAL_CALL FmXEditCell::setText( const OUString& aText )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
    {
        m_pEditImplementation->SetText( aText );

        // In Java, a textChanged is fired as well; not in VCL.
        // css::awt::Toolkit must be Java-compliant...
        onTextChanged();
    }
}


void SAL_CALL FmXEditCell::insertText(const css::awt::Selection& rSel, const OUString& aText)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
    {
        m_pEditImplementation->SetSelection( Selection( rSel.Min, rSel.Max ) );
        m_pEditImplementation->ReplaceSelected( aText );
    }
}


OUString SAL_CALL FmXEditCell::getText()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aText;
    if ( m_pEditImplementation )
    {
        if ( m_pEditImplementation->GetControl().IsVisible() && m_pColumn->GetParent().getDisplaySynchron())
        {
            // if the display isn't sync with the cursor we can't ask the edit field
            LineEnd eLineEndFormat = getModelLineEndSetting( m_pColumn->getModel() );
            aText = m_pEditImplementation->GetText( eLineEndFormat );
        }
        else
        {
            Reference< css::sdb::XColumn >  xField(m_pColumn->GetCurrentFieldValue());
            if (xField.is())
                aText = GetText(xField, m_pColumn->GetParent().getNumberFormatter());
        }
    }
    return aText;
}


OUString SAL_CALL FmXEditCell::getSelectedText()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aText;
    if ( m_pEditImplementation )
    {
        LineEnd eLineEndFormat = m_pColumn ? getModelLineEndSetting( m_pColumn->getModel() ) : LINEEND_LF;
        aText = m_pEditImplementation->GetSelected( eLineEndFormat );
    }
    return aText;
}


void SAL_CALL FmXEditCell::setSelection( const css::awt::Selection& aSelection )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
        m_pEditImplementation->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}


css::awt::Selection SAL_CALL FmXEditCell::getSelection()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Selection aSel;
    if ( m_pEditImplementation )
        aSel = m_pEditImplementation->GetSelection();

    return css::awt::Selection(aSel.Min(), aSel.Max());
}


sal_Bool SAL_CALL FmXEditCell::isEditable()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pEditImplementation && !m_pEditImplementation->IsReadOnly() && m_pEditImplementation->GetControl().IsEnabled();
}


void SAL_CALL FmXEditCell::setEditable( sal_Bool bEditable )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
        m_pEditImplementation->SetReadOnly( !bEditable );
}


sal_Int16 SAL_CALL FmXEditCell::getMaxTextLen()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pEditImplementation ? m_pEditImplementation->GetMaxTextLen() : 0;
}


void SAL_CALL FmXEditCell::setMaxTextLen( sal_Int16 nLen )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
        m_pEditImplementation->SetMaxTextLen( nLen );
}


void SAL_CALL FmXEditCell::addChangeListener( const Reference< form::XChangeListener >& Listener )
{
    m_aChangeListeners.addInterface( Listener );
}


void SAL_CALL FmXEditCell::removeChangeListener( const Reference< form::XChangeListener >& Listener )
{
    m_aChangeListeners.removeInterface( Listener );
}


void FmXEditCell::onTextChanged()
{
    css::awt::TextEvent aEvent;
    aEvent.Source = *this;
    m_aTextListeners.notifyEach( &awt::XTextListener::textChanged, aEvent );
}


void FmXEditCell::onFocusGained( const awt::FocusEvent& _rEvent )
{
    FmXTextCell::onFocusGained( _rEvent );
    m_sValueOnEnter = getText();
}


void FmXEditCell::onFocusLost( const awt::FocusEvent& _rEvent )
{
    FmXTextCell::onFocusLost( _rEvent );

    if ( getText() != m_sValueOnEnter )
    {
        lang::EventObject aEvent( *this );
        m_aChangeListeners.notifyEach( &XChangeListener::changed, aEvent );
    }
}


void FmXEditCell::onWindowEvent( const VclEventId _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{
    switch ( _nEventId )
    {
    case VclEventId::EditModify:
    {
        if ( m_pEditImplementation && m_aTextListeners.getLength() )
            onTextChanged();
        return;
    }
    default: break;
    }

    FmXTextCell::onWindowEvent( _nEventId, _rWindow, _pEventData );
}

FmXCheckBoxCell::FmXCheckBoxCell( DbGridColumn* pColumn, std::unique_ptr<DbCellControl> pControl )
                :FmXDataCell( pColumn, std::move(pControl) )
                ,m_aItemListeners(m_aMutex)
                ,m_aActionListeners( m_aMutex )
                ,m_pBox( & static_cast< CheckBoxControl& >( m_pCellControl->GetWindow() ).GetBox() )
{
}


FmXCheckBoxCell::~FmXCheckBoxCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

}

// OComponentHelper

void FmXCheckBoxCell::disposing()
{
    css::lang::EventObject aEvt(*this);
    m_aItemListeners.disposeAndClear(aEvt);
    m_aActionListeners.disposeAndClear(aEvt);

    static_cast< CheckBoxControl& >( m_pCellControl->GetWindow() ).SetClickHdl(Link<VclPtr<CheckBox>,void>());
    m_pBox = nullptr;

    FmXDataCell::disposing();
}


Any SAL_CALL FmXCheckBoxCell::queryAggregation( const css::uno::Type& _rType )
{
    Any aReturn = FmXDataCell::queryAggregation( _rType );

    if ( !aReturn.hasValue() )
        aReturn = FmXCheckBoxCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< css::uno::Type > SAL_CALL FmXCheckBoxCell::getTypes(  )
{
    return ::comphelper::concatSequences(
        FmXDataCell::getTypes(),
        FmXCheckBoxCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXCheckBoxCell )


void SAL_CALL FmXCheckBoxCell::addItemListener( const Reference< css::awt::XItemListener >& l )
{
    m_aItemListeners.addInterface( l );
}


void SAL_CALL FmXCheckBoxCell::removeItemListener( const Reference< css::awt::XItemListener >& l )
{
    m_aItemListeners.removeInterface( l );
}


void SAL_CALL FmXCheckBoxCell::setState( sal_Int16 n )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        UpdateFromColumn();
        m_pBox->SetState( static_cast<TriState>(n) );
    }
}


sal_Int16 SAL_CALL FmXCheckBoxCell::getState()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        UpdateFromColumn();
        return static_cast<sal_Int16>(m_pBox->GetState());
    }
    return TRISTATE_INDET;
}


void SAL_CALL FmXCheckBoxCell::enableTriState( sal_Bool b )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->EnableTriState( b );
}


void SAL_CALL FmXCheckBoxCell::addActionListener( const Reference< awt::XActionListener >& Listener )
{
    m_aActionListeners.addInterface( Listener );
}


void SAL_CALL FmXCheckBoxCell::removeActionListener( const Reference< awt::XActionListener >& Listener )
{
    m_aActionListeners.removeInterface( Listener );
}


void SAL_CALL FmXCheckBoxCell::setLabel( const OUString& Label )
{
    SolarMutexGuard aGuard;
    if ( m_pColumn )
    {
        DbGridControl& rGrid( m_pColumn->GetParent() );
        rGrid.SetColumnTitle( rGrid.GetColumnId( m_pColumn->GetFieldPos() ), Label );
    }
}


void SAL_CALL FmXCheckBoxCell::setActionCommand( const OUString& Command )
{
    m_aActionCommand = Command;
}


vcl::Window* FmXCheckBoxCell::getEventWindow() const
{
    return m_pBox;
}


void FmXCheckBoxCell::onWindowEvent( const VclEventId _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{
    switch ( _nEventId )
    {
    case VclEventId::CheckboxToggle:
    {
        // check boxes are to be committed immediately (this holds for ordinary check box controls in
        // documents, and this must hold for check boxes in grid columns, too
        // 91210 - 22.08.2001 - frank.schoenheit@sun.com
        m_pCellControl->Commit();

        Reference< XWindow > xKeepAlive( this );
        if ( m_aItemListeners.getLength() && m_pBox )
        {
            awt::ItemEvent aEvent;
            aEvent.Source = *this;
            aEvent.Highlighted = 0;
            aEvent.Selected = m_pBox->GetState();
            m_aItemListeners.notifyEach( &awt::XItemListener::itemStateChanged, aEvent );
        }
        if ( m_aActionListeners.getLength() )
        {
            awt::ActionEvent aEvent;
            aEvent.Source = *this;
            aEvent.ActionCommand = m_aActionCommand;
            m_aActionListeners.notifyEach( &awt::XActionListener::actionPerformed, aEvent );
        }
    }
    break;

    default:
        FmXDataCell::onWindowEvent( _nEventId, _rWindow, _pEventData );
        break;
    }
}

FmXListBoxCell::FmXListBoxCell(DbGridColumn* pColumn, std::unique_ptr<DbCellControl> pControl)
               :FmXTextCell( pColumn, std::move(pControl) )
               ,m_aItemListeners(m_aMutex)
               ,m_aActionListeners(m_aMutex)
               ,m_pBox( &static_cast< ListBox& >( m_pCellControl->GetWindow() ) )
{

    m_pBox->SetDoubleClickHdl( LINK( this, FmXListBoxCell, OnDoubleClick ) );
}


FmXListBoxCell::~FmXListBoxCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

}

// OComponentHelper

void FmXListBoxCell::disposing()
{
    css::lang::EventObject aEvt(*this);
    m_aItemListeners.disposeAndClear(aEvt);
    m_aActionListeners.disposeAndClear(aEvt);

    m_pBox->SetSelectHdl( Link<ListBox&,void>() );
    m_pBox->SetDoubleClickHdl( Link<ListBox&,void>() );
    m_pBox = nullptr;

    FmXTextCell::disposing();
}


Any SAL_CALL FmXListBoxCell::queryAggregation( const css::uno::Type& _rType )
{
    Any aReturn = FmXTextCell::queryAggregation(_rType);

    if ( !aReturn.hasValue() )
        aReturn = FmXListBoxCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< css::uno::Type > SAL_CALL FmXListBoxCell::getTypes(  )
{
    return ::comphelper::concatSequences(
        FmXTextCell::getTypes(),
        FmXListBoxCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXListBoxCell )


void SAL_CALL FmXListBoxCell::addItemListener(const Reference< css::awt::XItemListener >& l)
{
    m_aItemListeners.addInterface( l );
}


void SAL_CALL FmXListBoxCell::removeItemListener(const Reference< css::awt::XItemListener >& l)
{
    m_aItemListeners.removeInterface( l );
}


void SAL_CALL FmXListBoxCell::addActionListener(const Reference< css::awt::XActionListener >& l)
{
    m_aActionListeners.addInterface( l );
}


void SAL_CALL FmXListBoxCell::removeActionListener(const Reference< css::awt::XActionListener >& l)
{
    m_aActionListeners.removeInterface( l );
}


void SAL_CALL FmXListBoxCell::addItem(const OUString& aItem, sal_Int16 nPos)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
        m_pBox->InsertEntry( aItem, nPos );
}


void SAL_CALL FmXListBoxCell::addItems(const css::uno::Sequence<OUString>& aItems, sal_Int16 nPos)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
    {
        sal_uInt16 nP = nPos;
        for ( sal_Int32 n = 0; n < aItems.getLength(); n++ )
        {
            m_pBox->InsertEntry( aItems.getConstArray()[n], nP );
            if ( nPos != -1 )    // Not if 0xFFFF, because LIST_APPEND
                nP++;
        }
    }
}


void SAL_CALL FmXListBoxCell::removeItems(sal_Int16 nPos, sal_Int16 nCount)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            m_pBox->RemoveEntry( nPos + (--n) );
    }
}


sal_Int16 SAL_CALL FmXListBoxCell::getItemCount()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pBox ? m_pBox->GetEntryCount() : 0;
}


OUString SAL_CALL FmXListBoxCell::getItem(sal_Int16 nPos)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pBox ? m_pBox->GetEntry(nPos) : OUString();
}

css::uno::Sequence<OUString> SAL_CALL FmXListBoxCell::getItems()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    css::uno::Sequence<OUString> aSeq;
    if (m_pBox)
    {
        const sal_Int32 nEntries = m_pBox ->GetEntryCount();
        aSeq = css::uno::Sequence<OUString>( nEntries );
        for ( sal_Int32 n = nEntries; n; )
        {
            --n;
            aSeq.getArray()[n] = m_pBox ->GetEntry( n );
        }
    }
    return aSeq;
}


sal_Int16 SAL_CALL FmXListBoxCell::getSelectedItemPos()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
    {
        UpdateFromColumn();
        sal_Int32 nPos = m_pBox->GetSelectedEntryPos();
        if (nPos > SHRT_MAX || nPos < SHRT_MIN)
            throw std::out_of_range("awt::XListBox::getSelectedItemPos can only return a short");
        return nPos;
    }
    return 0;
}


Sequence< sal_Int16 > SAL_CALL FmXListBoxCell::getSelectedItemsPos()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Sequence<sal_Int16> aSeq;

    if (m_pBox)
    {
        UpdateFromColumn();
        const sal_Int32 nSelEntries = m_pBox->GetSelectedEntryCount();
        aSeq = Sequence<sal_Int16>( nSelEntries );
        for ( sal_Int32 n = 0; n < nSelEntries; ++n )
            aSeq.getArray()[n] = m_pBox->GetSelectedEntryPos( n );
    }
    return aSeq;
}

OUString SAL_CALL FmXListBoxCell::getSelectedItem()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aItem;

    if (m_pBox)
    {
        UpdateFromColumn();
        aItem = m_pBox->GetSelectedEntry();
    }

    return aItem;
}


css::uno::Sequence<OUString> SAL_CALL FmXListBoxCell::getSelectedItems()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    css::uno::Sequence<OUString> aSeq;

    if (m_pBox)
    {
        UpdateFromColumn();
        const sal_Int32 nSelEntries = m_pBox->GetSelectedEntryCount();
        aSeq = css::uno::Sequence<OUString>( nSelEntries );
        for ( sal_Int32 n = 0; n < nSelEntries; ++n )
            aSeq.getArray()[n] = m_pBox->GetSelectedEntry( n );
    }
    return aSeq;
}


void SAL_CALL FmXListBoxCell::selectItemPos(sal_Int16 nPos, sal_Bool bSelect)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SelectEntryPos( nPos, bSelect );
}


void SAL_CALL FmXListBoxCell::selectItemsPos(const Sequence< sal_Int16 >& aPositions, sal_Bool bSelect)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        for ( sal_uInt16 n = static_cast<sal_uInt16>(aPositions.getLength()); n; )
            m_pBox->SelectEntryPos( static_cast<sal_uInt16>(aPositions.getConstArray()[--n]), bSelect );
    }
}


void SAL_CALL FmXListBoxCell::selectItem(const OUString& aItem, sal_Bool bSelect)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SelectEntry( aItem, bSelect );
}


sal_Bool SAL_CALL FmXListBoxCell::isMutipleMode()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bMulti = false;
    if (m_pBox)
        bMulti = m_pBox->IsMultiSelectionEnabled();
    return bMulti;
}


void SAL_CALL FmXListBoxCell::setMultipleMode(sal_Bool bMulti)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->EnableMultiSelection( bMulti );
}


sal_Int16 SAL_CALL FmXListBoxCell::getDropDownLineCount()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int16 nLines = 0;
    if (m_pBox)
        nLines = m_pBox->GetDropDownLineCount();

    return nLines;
}


void SAL_CALL FmXListBoxCell::setDropDownLineCount(sal_Int16 nLines)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SetDropDownLineCount( nLines );
}


void SAL_CALL FmXListBoxCell::makeVisible(sal_Int16 nEntry)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SetTopEntry( nEntry );
}


void FmXListBoxCell::onWindowEvent( const VclEventId _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{
    if  (   ( &_rWindow == m_pBox )
        &&  ( _nEventId == VclEventId::ListboxSelect )
        )
    {
        OnDoubleClick( *m_pBox );

        css::awt::ItemEvent aEvent;
        aEvent.Source = *this;
        aEvent.Highlighted = 0;

        // with multiple selection 0xFFFF, otherwise the ID
        aEvent.Selected = (m_pBox->GetSelectedEntryCount() == 1 )
            ? m_pBox->GetSelectedEntryPos() : 0xFFFF;

        m_aItemListeners.notifyEach( &awt::XItemListener::itemStateChanged, aEvent );
        return;
    }

    FmXTextCell::onWindowEvent( _nEventId, _rWindow, _pEventData );
}


IMPL_LINK_NOARG(FmXListBoxCell, OnDoubleClick, ListBox&, void)
{
    if (m_pBox)
    {
        ::comphelper::OInterfaceIteratorHelper2 aIt( m_aActionListeners );

        css::awt::ActionEvent aEvent;
        aEvent.Source = *this;
        aEvent.ActionCommand = m_pBox->GetSelectedEntry();

        while( aIt.hasMoreElements() )
            static_cast< css::awt::XActionListener *>(aIt.next())->actionPerformed( aEvent );
    }
}

FmXComboBoxCell::FmXComboBoxCell( DbGridColumn* pColumn, std::unique_ptr<DbCellControl> pControl )
    :FmXTextCell( pColumn, std::move(pControl) )
    ,m_aItemListeners( m_aMutex )
    ,m_aActionListeners( m_aMutex )
    ,m_pComboBox( &static_cast< ComboBox& >( m_pCellControl->GetWindow() ) )
{
}


FmXComboBoxCell::~FmXComboBoxCell()
{
    if ( !OComponentHelper::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }

}


void FmXComboBoxCell::disposing()
{
    css::lang::EventObject aEvt(*this);
    m_aItemListeners.disposeAndClear(aEvt);
    m_aActionListeners.disposeAndClear(aEvt);

    FmXTextCell::disposing();
}


Any SAL_CALL FmXComboBoxCell::queryAggregation( const css::uno::Type& _rType )
{
    Any aReturn = FmXTextCell::queryAggregation(_rType);

    if ( !aReturn.hasValue() )
        aReturn = FmXComboBoxCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< Type > SAL_CALL FmXComboBoxCell::getTypes(  )
{
    return ::comphelper::concatSequences(
        FmXTextCell::getTypes(),
        FmXComboBoxCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXComboBoxCell )


void SAL_CALL FmXComboBoxCell::addItemListener(const Reference< awt::XItemListener >& l)
{
    m_aItemListeners.addInterface( l );
}


void SAL_CALL FmXComboBoxCell::removeItemListener(const Reference< awt::XItemListener >& l)
{
    m_aItemListeners.removeInterface( l );
}


void SAL_CALL FmXComboBoxCell::addActionListener(const Reference< awt::XActionListener >& l)
{
    m_aActionListeners.addInterface( l );
}


void SAL_CALL FmXComboBoxCell::removeActionListener(const Reference< awt::XActionListener >& l)
{
    m_aActionListeners.removeInterface( l );
}


void SAL_CALL FmXComboBoxCell::addItem( const OUString& Item, sal_Int16 Pos )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pComboBox )
        m_pComboBox->InsertEntry( Item, Pos );
}


void SAL_CALL FmXComboBoxCell::addItems( const Sequence< OUString >& Items, sal_Int16 Pos )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pComboBox )
    {
        sal_uInt16 nP = Pos;
        for ( sal_Int32 n = 0; n < Items.getLength(); n++ )
        {
            m_pComboBox->InsertEntry( Items.getConstArray()[n], nP );
            if ( Pos != -1 )
                nP++;
        }
    }
}


void SAL_CALL FmXComboBoxCell::removeItems( sal_Int16 Pos, sal_Int16 Count )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pComboBox )
    {
        for ( sal_uInt16 n = Count; n; )
            m_pComboBox->RemoveEntryAt( Pos + (--n) );
    }
}


sal_Int16 SAL_CALL FmXComboBoxCell::getItemCount()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pComboBox ? m_pComboBox->GetEntryCount() : 0;
}


OUString SAL_CALL FmXComboBoxCell::getItem( sal_Int16 Pos )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pComboBox ? m_pComboBox->GetEntry(Pos) : OUString();
}

Sequence< OUString > SAL_CALL FmXComboBoxCell::getItems()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Sequence< OUString > aItems;
    if ( m_pComboBox )
    {
        const sal_Int32 nEntries = m_pComboBox->GetEntryCount();
        aItems.realloc( nEntries );
        OUString* pItem = aItems.getArray();
        for ( sal_Int32 n=0; n<nEntries; ++n, ++pItem )
            *pItem = m_pComboBox->GetEntry( n );
    }
    return aItems;
}


sal_Int16 SAL_CALL FmXComboBoxCell::getDropDownLineCount()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int16 nLines = 0;
    if ( m_pComboBox )
        nLines = m_pComboBox->GetDropDownLineCount();

    return nLines;
}


void SAL_CALL FmXComboBoxCell::setDropDownLineCount(sal_Int16 nLines)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pComboBox )
        m_pComboBox->SetDropDownLineCount( nLines );
}


void FmXComboBoxCell::onWindowEvent( const VclEventId _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{

    switch ( _nEventId )
    {
    case VclEventId::ComboboxSelect:
    {
        awt::ItemEvent aEvent;
        aEvent.Source = *this;
        aEvent.Highlighted = 0;

        // with multiple selection 0xFFFF, otherwise the ID
        aEvent.Selected =   ( m_pComboBox->GetSelectedEntryCount() == 1 )
                        ?   m_pComboBox->GetSelectedEntryPos()
                        :   0xFFFF;
        m_aItemListeners.notifyEach( &awt::XItemListener::itemStateChanged, aEvent );
    }
    break;

    default:
        FmXTextCell::onWindowEvent( _nEventId, _rWindow, _pEventData );
        break;
    }
}


FmXFilterCell::FmXFilterCell(DbGridColumn* pColumn, std::unique_ptr<DbFilterField> pControl )
              :FmXGridCell( pColumn, std::move(pControl) )
              ,m_aTextListeners(m_aMutex)
{
    static_cast<DbFilterField*>(m_pCellControl.get())->SetCommitHdl( LINK( this, FmXFilterCell, OnCommit ) );
}


FmXFilterCell::~FmXFilterCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

}

// XUnoTunnel

sal_Int64 SAL_CALL FmXFilterCell::getSomething( const Sequence< sal_Int8 >& _rIdentifier )
{
    sal_Int64 nReturn(0);

    if  (   (_rIdentifier.getLength() == 16)
        &&  (0 == memcmp( getUnoTunnelId().getConstArray(), _rIdentifier.getConstArray(), 16 ))
        )
    {
        nReturn = reinterpret_cast<sal_Int64>(this);
    }

    return nReturn;
}

namespace
{
    class theFmXFilterCellUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theFmXFilterCellUnoTunnelId > {};
}

const Sequence<sal_Int8>& FmXFilterCell::getUnoTunnelId()
{
    return theFmXFilterCellUnoTunnelId::get().getSeq();
}


void FmXFilterCell::PaintCell( OutputDevice& rDev, const tools::Rectangle& rRect )
{
    static_cast< DbFilterField* >( m_pCellControl.get() )->PaintCell( rDev, rRect );
}

// OComponentHelper

void FmXFilterCell::disposing()
{
    css::lang::EventObject aEvt(*this);
    m_aTextListeners.disposeAndClear(aEvt);

    static_cast<DbFilterField*>(m_pCellControl.get())->SetCommitHdl(Link<DbFilterField&,void>());

    FmXGridCell::disposing();
}


Any SAL_CALL FmXFilterCell::queryAggregation( const css::uno::Type& _rType )
{
    Any aReturn = FmXGridCell::queryAggregation(_rType);

    if ( !aReturn.hasValue() )
        aReturn = FmXFilterCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< css::uno::Type > SAL_CALL FmXFilterCell::getTypes(  )
{
    return ::comphelper::concatSequences(
        FmXGridCell::getTypes(),
        FmXFilterCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXFilterCell )

// css::awt::XTextComponent

void SAL_CALL FmXFilterCell::addTextListener(const Reference< css::awt::XTextListener >& l)
{
    m_aTextListeners.addInterface( l );
}


void SAL_CALL FmXFilterCell::removeTextListener(const Reference< css::awt::XTextListener >& l)
{
    m_aTextListeners.removeInterface( l );
}


void SAL_CALL FmXFilterCell::setText( const OUString& aText )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    static_cast<DbFilterField*>(m_pCellControl.get())->SetText(aText);
}


void SAL_CALL FmXFilterCell::insertText( const css::awt::Selection& /*rSel*/, const OUString& /*aText*/ )
{
}


OUString SAL_CALL FmXFilterCell::getText()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return static_cast<DbFilterField*>(m_pCellControl.get())->GetText();
}


OUString SAL_CALL FmXFilterCell::getSelectedText()
{
    return getText();
}


void SAL_CALL FmXFilterCell::setSelection( const css::awt::Selection& /*aSelection*/ )
{
}


css::awt::Selection SAL_CALL FmXFilterCell::getSelection()
{
    return css::awt::Selection();
}


sal_Bool SAL_CALL FmXFilterCell::isEditable()
{
    return true;
}


void SAL_CALL FmXFilterCell::setEditable( sal_Bool /*bEditable*/ )
{
}


sal_Int16 SAL_CALL FmXFilterCell::getMaxTextLen()
{
    return 0;
}


void SAL_CALL FmXFilterCell::setMaxTextLen( sal_Int16 /*nLen*/ )
{
}


IMPL_LINK_NOARG(FmXFilterCell, OnCommit, DbFilterField&, void)
{
    ::comphelper::OInterfaceIteratorHelper2 aIt( m_aTextListeners );
    css::awt::TextEvent aEvt;
    aEvt.Source = *this;
    while( aIt.hasMoreElements() )
        static_cast< css::awt::XTextListener *>(aIt.next())->textChanged( aEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
