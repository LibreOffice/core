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


#include <sal/macros.h>
#include "fmprop.hrc"
#include "svx/fmresids.hrc"
#include "svx/fmtools.hxx"
#include "gridcell.hxx"
#include "gridcols.hxx"
#include "sdbdatacolumn.hxx"

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
#include <connectivity/formattedcolumnvalue.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <i18nlangtag/lang.h>

#include <rtl/math.hxx>
#include <svtools/calendar.hxx>
#include <svtools/fmtfield.hxx>
#include <svl/numuno.hxx>
#include <svtools/svmedit.hxx>
#include <svx/dialmgr.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/longcurr.hxx>
#include <vcl/settings.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>

#include <math.h>
#include <stdio.h>

using namespace ::connectivity;
using namespace ::connectivity::simple;
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
namespace MouseWheelBehavior = ::com::sun::star::awt::MouseWheelBehavior;

const char INVALIDTEXT[] = "###";
const char OBJECTTEXT[] = "<OBJECT>";


//= helper

namespace
{
    static LineEnd getModelLineEndSetting( const Reference< XPropertySet >& _rxModel )
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
            OSL_FAIL( "getModelLineEndSetting: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }
        return eFormat;
    }
}


//= DbGridColumn


CellControllerRef DbGridColumn::s_xEmptyController;


void DbGridColumn::CreateControl(sal_Int32 _nFieldPos, const Reference< ::com::sun::star::beans::XPropertySet >& xField, sal_Int32 nTypeId)
{
    Clear();

    m_nTypeId = (sal_Int16)nTypeId;
    if (xField != m_xField)
    {
        // Grundeinstellung
        m_xField = xField;
        xField->getPropertyValue(FM_PROP_FORMATKEY) >>= m_nFormatKey;
        m_nFieldPos   = (sal_Int16)_nFieldPos;
        m_bReadOnly   = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISREADONLY));
        m_bAutoValue  = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_AUTOINCREMENT));
        m_nFieldType  = (sal_Int16)::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FIELDTYPE));

        switch (m_nFieldType)
        {
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
                m_bDateTime = true;
                //fall-through
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
                m_nAlign = ::com::sun::star::awt::TextAlign::RIGHT;
                m_bNumeric = true;
                break;
            default:
                m_nAlign = ::com::sun::star::awt::TextAlign::LEFT;
                break;
        }
    }

    DbCellControl* pCellControl = NULL;
    if (m_rParent.IsFilterMode())
    {
        pCellControl = new DbFilterField(m_rParent.getContext(),*this);
    }
    else
    {

        switch (nTypeId)
        {
            case TYPE_CHECKBOX: pCellControl = new DbCheckBox(*this);   break;
            case TYPE_COMBOBOX: pCellControl = new DbComboBox(*this); break;
            case TYPE_CURRENCYFIELD: pCellControl = new DbCurrencyField(*this); break;
            case TYPE_DATEFIELD: pCellControl = new DbDateField(*this); break;
            case TYPE_LISTBOX: pCellControl = new DbListBox(*this); break;
            case TYPE_NUMERICFIELD: pCellControl = new DbNumericField(*this); break;
            case TYPE_PATTERNFIELD: pCellControl = new DbPatternField( *this, m_rParent.getContext() ); break;
            case TYPE_TEXTFIELD: pCellControl = new DbTextField(*this); break;
            case TYPE_TIMEFIELD: pCellControl = new DbTimeField(*this); break;
            case TYPE_FORMATTEDFIELD: pCellControl = new DbFormattedField(*this); break;
            default:
                OSL_FAIL("DbGridColumn::CreateControl: Unknown Column");
                return;
        }

    }
    Reference< XRowSet >  xCur;
    if (m_rParent.getDataSource())
        xCur = Reference< XRowSet > (Reference< XInterface >(*m_rParent.getDataSource()), UNO_QUERY);
        // TODO : the cursor wrapper should use an XRowSet interface, too

    pCellControl->Init( m_rParent.GetDataWindow(), xCur );

    // now create the control wrapper
    if (m_rParent.IsFilterMode())
        m_pCell = new FmXFilterCell(this, pCellControl);
    else
    {
        switch (nTypeId)
        {
            case TYPE_CHECKBOX: m_pCell = new FmXCheckBoxCell( this, *pCellControl );  break;
            case TYPE_LISTBOX: m_pCell = new FmXListBoxCell( this, *pCellControl );    break;
            case TYPE_COMBOBOX: m_pCell = new FmXComboBoxCell( this, *pCellControl );    break;
            default:
                m_pCell = new FmXEditCell( this, *pCellControl );
        }
    }
    m_pCell->acquire();
    m_pCell->init();

    impl_toggleScriptManager_nothrow( true );

    // only if we use have a bound field, we use a controller for displaying the
    // window in the grid
    if (m_xField.is())
        m_xController = pCellControl->CreateController();
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
        DBG_UNHANDLED_EXCEPTION();
    }
}


void DbGridColumn::UpdateFromField(const DbGridRow* pRow, const Reference< XNumberFormatter >& xFormatter)
{
    if (m_pCell && m_pCell->ISA(FmXFilterCell))
        PTR_CAST(FmXFilterCell, m_pCell)->Update();
    else if (pRow && pRow->IsValid() && m_nFieldPos >= 0 && m_pCell && pRow->HasField(m_nFieldPos))
    {
        PTR_CAST(FmXDataCell, m_pCell)->UpdateFromField( pRow->GetField( m_nFieldPos ).getColumn(), xFormatter );
    }
}


bool DbGridColumn::Commit()
{
    bool bResult = true;
    if (!m_bInSave && m_pCell)
    {
        m_bInSave = true;
        bResult = m_pCell->Commit();

        // store the data into the model
        FmXDataCell* pDataCell = PTR_CAST(FmXDataCell, m_pCell);
        if (bResult && pDataCell)
        {
            Reference< ::com::sun::star::form::XBoundComponent >  xComp(m_xModel, UNO_QUERY);
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


void DbGridColumn::setModel(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  _xModel)
{
    if ( m_pCell )
        impl_toggleScriptManager_nothrow( false );

    m_xModel = _xModel;

    if ( m_pCell )
        impl_toggleScriptManager_nothrow( true );
}


void DbGridColumn::Clear()
{
    if ( m_pCell )
    {
        impl_toggleScriptManager_nothrow( false );

        m_pCell->dispose();
        m_pCell->release();
        m_pCell = NULL;
    }

    m_xController = NULL;
    m_xField = NULL;

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
                    _nAlign = ::com::sun::star::awt::TextAlign::RIGHT;
                    break;
                case DataType::BIT:
                case DataType::BOOLEAN:
                    _nAlign = ::com::sun::star::awt::TextAlign::CENTER;
                    break;
                default:
                    _nAlign = ::com::sun::star::awt::TextAlign::LEFT;
                    break;
            }
        }
        else
            _nAlign = ::com::sun::star::awt::TextAlign::LEFT;
    }

    m_nAlign = _nAlign;
    if (m_pCell && m_pCell->isAlignedController())
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
    if (m_pCell && m_pCell->ISA(FmXFilterCell))
        return aText;

    if (!pRow || !pRow->IsValid())
        aText = OUString(INVALIDTEXT);
    else if (pRow->HasField(m_nFieldPos))
    {
        aText = GetCellText( pRow->GetField( m_nFieldPos ).getColumn(), xFormatter );
    }
    return aText;
}


OUString DbGridColumn::GetCellText(const Reference< ::com::sun::star::sdb::XColumn >& xField, const Reference< XNumberFormatter >& xFormatter) const
{
    OUString aText;
    if (xField.is())
    {
        FmXTextCell* pTextCell = PTR_CAST(FmXTextCell, m_pCell);
        if (pTextCell)
            aText = pTextCell->GetText(xField, xFormatter);
        else if (m_bObject)
            aText = OUString(OBJECTTEXT);
    }
    return aText;
}


Reference< ::com::sun::star::sdb::XColumn >  DbGridColumn::GetCurrentFieldValue() const
{
    Reference< ::com::sun::star::sdb::XColumn >  xField;
    const DbGridRowRef xRow = m_rParent.GetCurrentRow();
    if (xRow.Is() && xRow->HasField(m_nFieldPos))
    {
        xField = xRow->GetField(m_nFieldPos).getColumn();
    }
    return xField;
}


void DbGridColumn::Paint(OutputDevice& rDev,
                         const Rectangle& rRect,
                         const DbGridRow* pRow,
                         const Reference< XNumberFormatter >& xFormatter)
{
    bool bEnabled = ( rDev.GetOutDevType() != OUTDEV_WINDOW )
                ||  ( static_cast< vcl::Window& >( rDev ).IsEnabled() );

    FmXDataCell* pDataCell = PTR_CAST(FmXDataCell, m_pCell);
    if (pDataCell)
    {
        if (!pRow || !pRow->IsValid())
        {
            sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_CENTER;
            if ( !bEnabled )
                nStyle |= TEXT_DRAW_DISABLE;

            rDev.DrawText(rRect, OUString(INVALIDTEXT), nStyle);
        }
        else if (m_bAutoValue && pRow->IsNew())
        {
            sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_VCENTER;
            if ( !bEnabled )
                nStyle |= TEXT_DRAW_DISABLE;

            switch (GetAlignment())
            {
                case ::com::sun::star::awt::TextAlign::RIGHT:
                    nStyle |= TEXT_DRAW_RIGHT;
                    break;
                case ::com::sun::star::awt::TextAlign::CENTER:
                    nStyle |= TEXT_DRAW_CENTER;
                    break;
                default:
                    nStyle |= TEXT_DRAW_LEFT;
            }

            rDev.DrawText(rRect, SVX_RESSTR(RID_STR_AUTOFIELD), nStyle);
        }
        else if (pRow->HasField(m_nFieldPos))
        {
            pDataCell->PaintFieldToCell(rDev, rRect, pRow->GetField( m_nFieldPos ).getColumn(), xFormatter);
        }
    }
    else if (!m_pCell)
    {
        if (!pRow || !pRow->IsValid())
        {
            sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_CENTER;
            if ( !bEnabled )
                nStyle |= TEXT_DRAW_DISABLE;

            rDev.DrawText(rRect, OUString(INVALIDTEXT), nStyle);
        }
        else if (pRow->HasField(m_nFieldPos) && m_bObject)
        {
            sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_CENTER;
            if ( !bEnabled )
                nStyle |= TEXT_DRAW_DISABLE;
            rDev.DrawText(rRect, OUString(OBJECTTEXT), nStyle);
        }
    }
    else if ( m_pCell->ISA( FmXFilterCell ) )
        static_cast< FmXFilterCell* >( m_pCell )->PaintCell( rDev, rRect );
}


void DbGridColumn::ImplInitWindow( vcl::Window& rParent, const InitWindowFacet _eInitWhat )
{
    if ( m_pCell )
        m_pCell->ImplInitWindow( rParent, _eInitWhat );
}


//= cell controls

TYPEINIT0( DbCellControl )
TYPEINIT1( DbLimitedLengthField, DbCellControl )
TYPEINIT1( DbTextField, DbLimitedLengthField )
TYPEINIT1( DbFormattedField, DbLimitedLengthField )
TYPEINIT1( DbCheckBox, DbCellControl )
TYPEINIT1( DbComboBox, DbCellControl )
TYPEINIT1( DbListBox, DbCellControl )
TYPEINIT1( DbPatternField, DbCellControl )
TYPEINIT1( DbSpinField, DbCellControl )
TYPEINIT1( DbDateField, DbSpinField )
TYPEINIT1( DbTimeField, DbSpinField )
TYPEINIT1( DbCurrencyField, DbSpinField )
TYPEINIT1( DbNumericField, DbSpinField )
TYPEINIT1( DbFilterField, DbCellControl )


DbCellControl::DbCellControl( DbGridColumn& _rColumn, bool /*_bText*/ )
    :OPropertyChangeListener(m_aMutex)
    ,m_pModelChangeBroadcaster(NULL)
    ,m_pFieldChangeBroadcaster(NULL)
    ,m_bTransparent( false )
    ,m_bAlignedController( true )
    ,m_bAccessingValueProperty( false )
    ,m_rColumn( _rColumn )
    ,m_pPainter( NULL )
    ,m_pWindow( NULL )
{
    Reference< XPropertySet > xColModelProps( _rColumn.getModel(), UNO_QUERY );
    if ( xColModelProps.is() )
    {
        // if our model's format key changes we want to propagate the new value to our windows
        m_pModelChangeBroadcaster = new ::comphelper::OPropertyChangeMultiplexer(this, Reference< ::com::sun::star::beans::XPropertySet > (_rColumn.getModel(), UNO_QUERY));
        m_pModelChangeBroadcaster->acquire();

        // be listener for some common properties
        implDoPropertyListening( FM_PROP_READONLY, false );
        implDoPropertyListening( FM_PROP_ENABLED, false );

        // add as listener for all know "value" properties
        implDoPropertyListening( FM_PROP_VALUE, false );
        implDoPropertyListening( FM_PROP_STATE, false );
        implDoPropertyListening( FM_PROP_TEXT, false );
        implDoPropertyListening( FM_PROP_EFFECTIVE_VALUE, false );

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
                    m_pFieldChangeBroadcaster->acquire();
                    m_pFieldChangeBroadcaster->addProperty( FM_PROP_ISREADONLY );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "DbCellControl::doPropertyListening: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }
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
        (void)_bWarnIfNotExistent;

        if ( xPSI.is() && xPSI->hasPropertyByName( _rPropertyName ) )
            m_pModelChangeBroadcaster->addProperty( _rPropertyName );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "DbCellControl::doPropertyListening: caught an exception!" );
        DBG_UNHANDLED_EXCEPTION();
    }
}


void DbCellControl::doPropertyListening(const OUString& _rPropertyName)
{
    implDoPropertyListening( _rPropertyName );
}

static void lcl_clearBroadCaster(::comphelper::OPropertyChangeMultiplexer*& _pBroadcaster)
{
    if ( _pBroadcaster )
    {
        _pBroadcaster->dispose();
        _pBroadcaster->release();
        _pBroadcaster = NULL;
        // no delete, this is done implicitly
    }
}

DbCellControl::~DbCellControl()
{
    lcl_clearBroadCaster(m_pModelChangeBroadcaster);
    lcl_clearBroadCaster(m_pFieldChangeBroadcaster);

    delete m_pWindow;
    delete m_pPainter;
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


void DbCellControl::_propertyChanged(const PropertyChangeEvent& _rEvent) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XPropertySet > xSourceProps( _rEvent.Source, UNO_QUERY );

    if  (   _rEvent.PropertyName.equals( FM_PROP_VALUE )
        ||  _rEvent.PropertyName.equals( FM_PROP_STATE )
        ||  _rEvent.PropertyName.equals( FM_PROP_TEXT )
        ||  _rEvent.PropertyName.equals( FM_PROP_EFFECTIVE_VALUE )
        )
    {   // it was one of the known "value" properties
        if ( !isValuePropertyLocked() )
        {
            implValuePropertyChanged( );
        }
    }
    else if ( _rEvent.PropertyName.equals( FM_PROP_READONLY ) )
    {
        implAdjustReadOnly( xSourceProps, true);
    }
    else if ( _rEvent.PropertyName.equals( FM_PROP_ISREADONLY ) )
    {
        bool bReadOnly = true;
        _rEvent.NewValue >>= bReadOnly;
        m_rColumn.SetReadOnly(bReadOnly);
        implAdjustReadOnly( xSourceProps, false);
    }
    else if ( _rEvent.PropertyName.equals( FM_PROP_ENABLED ) )
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
        DBG_UNHANDLED_EXCEPTION();
    }
    // unlock the listening for value property changes
    unlockValueProperty();
    // outta here
    return bReturn;
}


void DbCellControl::ImplInitWindow( vcl::Window& rParent, const InitWindowFacet _eInitWhat )
{
    vcl::Window* pWindows[] = { m_pPainter, m_pWindow };

    if ( ( _eInitWhat & InitWritingMode ) != 0 )
    {
        for ( size_t i=0; i < sizeof( pWindows ) / sizeof( pWindows[0] ); ++i )
        {
            if ( pWindows[i] )
                pWindows[i]->EnableRTL( rParent.IsRTLEnabled() );
        }
    }

    if ( ( _eInitWhat & InitFontFacet ) != 0 )
    {
        for (size_t i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
        {
            if ( !pWindows[i] )
                continue;

            pWindows[i]->SetZoom( rParent.GetZoom() );

            const StyleSettings& rStyleSettings = pWindows[i]->GetSettings().GetStyleSettings();
            vcl::Font aFont = rStyleSettings.GetFieldFont();
            aFont.SetTransparent( isTransparent() );

            if ( rParent.IsControlFont() )
            {
                pWindows[i]->SetControlFont( rParent.GetControlFont() );
                aFont.Merge( rParent.GetControlFont() );
            }
            else
                pWindows[i]->SetControlFont();

            pWindows[i]->SetZoomedPointFont( aFont );
        }
    }

    if  (   ( ( _eInitWhat & InitFontFacet ) != 0 )
        ||  ( ( _eInitWhat & InitForeground ) != 0 )
        )
    {
        Color aTextColor( rParent.IsControlForeground() ? rParent.GetControlForeground() : rParent.GetTextColor() );

        bool bTextLineColor = rParent.IsTextLineColor();
        Color aTextLineColor( rParent.GetTextLineColor() );

        for (size_t i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
        {
            if ( pWindows[i] )
            {
                pWindows[i]->SetTextColor(aTextColor);
                if (rParent.IsControlForeground())
                    pWindows[i]->SetControlForeground(aTextColor);

                if (bTextLineColor)
                    pWindows[i]->SetTextLineColor();
                else
                    pWindows[i]->SetTextLineColor(aTextLineColor);
            }
        }
    }

    if ( ( _eInitWhat & InitBackground ) != 0 )
    {
        if (rParent.IsControlBackground())
        {
            Color aColor( rParent.GetControlBackground());
            for (size_t i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
            {
                if ( pWindows[i] )
                {
                    if ( isTransparent() )
                        pWindows[i]->SetBackground();
                    else
                    {
                        pWindows[i]->SetBackground(aColor);
                        pWindows[i]->SetControlBackground(aColor);
                    }
                    pWindows[i]->SetFillColor(aColor);
                }
            }
        }
        else
        {
            if (m_pPainter)
            {
                if ( isTransparent() )
                    m_pPainter->SetBackground();
                else
                    m_pPainter->SetBackground(rParent.GetBackground());
                m_pPainter->SetFillColor(rParent.GetFillColor());
            }

            if (m_pWindow)
            {
                if ( isTransparent() )
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
        Edit* pEditWindow = dynamic_cast< Edit* >( m_pWindow );
        if ( pEditWindow )
        {
            bool bReadOnly = m_rColumn.IsReadOnly();
            if ( !bReadOnly )
            {
                _rxModel->getPropertyValue( i_bReadOnly ? OUString(FM_PROP_READONLY) : OUString(FM_PROP_ISREADONLY)) >>= bReadOnly;
            }
            static_cast< Edit* >( m_pWindow )->SetReadOnly( bReadOnly );
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
    ImplInitWindow( rParent, InitAll );

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
                sal_Int16 nWheelBehavior = MouseWheelBehavior::SCROLL_FOCUS_ONLY;
                OSL_VERIFY( xModel->getPropertyValue( FM_PROP_MOUSE_WHEEL_BEHAVIOR ) >>= nWheelBehavior );
                sal_uInt16 nVclSetting = MOUSE_WHEEL_FOCUS_ONLY;
                switch ( nWheelBehavior )
                {
                case MouseWheelBehavior::SCROLL_DISABLED:   nVclSetting = MOUSE_WHEEL_DISABLE; break;
                case MouseWheelBehavior::SCROLL_FOCUS_ONLY: nVclSetting = MOUSE_WHEEL_FOCUS_ONLY; break;
                case MouseWheelBehavior::SCROLL_ALWAYS:     nVclSetting = MOUSE_WHEEL_ALWAYS; break;
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    m_xCursor = _rxCursor;
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
    static void lcl_implAlign( vcl::Window* _pWindow, WinBits _nAlignmentBit )
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
        case ::com::sun::star::awt::TextAlign::RIGHT:
            nAlignmentBit = WB_RIGHT;
            break;
        case ::com::sun::star::awt::TextAlign::CENTER:
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


void DbCellControl::PaintCell( OutputDevice& _rDev, const Rectangle& _rRect )
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
        m_pPainter->Draw( &_rDev, _rRect.TopLeft(), _rRect.GetSize(), 0 );
}


void DbCellControl::PaintFieldToCell( OutputDevice& _rDev, const Rectangle& _rRect, const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& _rxFormatter )
{
    m_pPainter->SetText( GetFormatText( _rxField, _rxFormatter ) );
    PaintCell( _rDev, _rRect );
}


double DbCellControl::GetValue(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter) const
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
            ,m_pEdit( NULL )
            ,m_pPainterImplementation( NULL )
            ,m_bIsSimpleEdit( true )
{
}


DbTextField::~DbTextField( )
{
    DELETEZ( m_pPainterImplementation );
    DELETEZ( m_pEdit );
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
        OSL_FAIL( "DbTextField::Init: caught an exception while determining the multi-line capabilities!" );
        DBG_UNHANDLED_EXCEPTION();
    }

    m_bIsSimpleEdit = !bIsMultiLine;
    if ( bIsMultiLine )
    {
        m_pWindow = new MultiLineTextCell( &rParent, nStyle );
        m_pEdit = new MultiLineEditImplementation( *static_cast< MultiLineTextCell* >( m_pWindow ) );

        m_pPainter = new MultiLineTextCell( &rParent, nStyle );
        m_pPainterImplementation = new MultiLineEditImplementation( *static_cast< MultiLineTextCell* >( m_pPainter ) );
    }
    else
    {
        m_pWindow = new Edit( &rParent, nStyle );
        m_pEdit = new EditImplementation( *static_cast< Edit* >( m_pWindow ) );

        m_pPainter = new Edit( &rParent, nStyle );
        m_pPainterImplementation = new EditImplementation( *static_cast< Edit* >( m_pPainter ) );
    }

    if ( WB_LEFT == nStyle )
    {
        // this is so that when getting the focus, the selection is oriented left-to-right
        AllSettings aSettings = m_pWindow->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        aStyleSettings.SetSelectionOptions(
            aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
        aSettings.SetStyleSettings(aStyleSettings);
        m_pWindow->SetSettings(aSettings);
    }

    implAdjustGenericFieldSetting( xModel );

    DbLimitedLengthField::Init( rParent, xCursor );
}


CellControllerRef DbTextField::CreateController() const
{
    return new EditCellController( m_pEdit );
}


void DbTextField::PaintFieldToCell( OutputDevice& _rDev, const Rectangle& _rRect, const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& _rxFormatter )
{
    if ( m_pPainterImplementation )
        m_pPainterImplementation->SetText( GetFormatText( _rxField, _rxFormatter, NULL ) );

    DbLimitedLengthField::PaintFieldToCell( _rDev, _rRect, _rxField, _rxFormatter );
}


OUString DbTextField::GetFormatText(const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter, Color** /*ppColor*/)
{
    const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xPS(_rxField, UNO_QUERY);
    FormattedColumnValue fmter( xFormatter, xPS );

    return fmter.getFormattedValue();
}


void DbTextField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter)
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
    ,m_nKeyType(::com::sun::star::util::NumberFormat::UNDEFINED)
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

    Reference< ::com::sun::star::beans::XPropertySet >  xUnoModel = m_rColumn.getModel();

    switch (nAlignment)
    {
        case ::com::sun::star::awt::TextAlign::RIGHT:
            m_pWindow  = new FormattedField( &rParent, WB_RIGHT );
            m_pPainter = new FormattedField( &rParent, WB_RIGHT );
            break;

        case ::com::sun::star::awt::TextAlign::CENTER:
            m_pWindow  = new FormattedField( &rParent, WB_CENTER );
            m_pPainter  = new FormattedField( &rParent, WB_CENTER );
            break;
        default:
            m_pWindow  = new FormattedField( &rParent, WB_LEFT );
            m_pPainter  = new FormattedField( &rParent, WB_LEFT );

            // Alles nur damit die Selektion bei Focuserhalt von rechts nach links geht
            AllSettings aSettings = m_pWindow->GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings);
    }

    implAdjustGenericFieldSetting( xUnoModel );

    static_cast< FormattedField* >( m_pWindow )->SetStrictFormat( false );
    static_cast< FormattedField* >( m_pPainter )->SetStrictFormat( false );
        // wenn man _irgendeine_ Formatierung zulaesst, kann man da sowieso keine Eingabe-Ueberpruefung
        // machen (das FormattedField unterstuetzt das sowieso nicht, nur abgeleitete Klassen)

    // von dem Uno-Model den Formatter besorgen
    // (Ich koennte theoretisch auch ueber den ::com::sun::star::util::NumberFormatter gehen, den mir der Cursor bestimmt
    // liefern wuerde. Das Problem dabei ist, dass ich mich eigentlich nicht darauf verlassen
    // kann, dass die beiden Formatter die selben sind, sauber ist das Ganze, wenn ich ueber das
    // UNO-Model gehe.)
    sal_Int32 nFormatKey = -1;

    // mal sehen, ob das Model einen hat ...
    DBG_ASSERT(::comphelper::hasProperty(FM_PROP_FORMATSSUPPLIER, xUnoModel), "DbFormattedField::Init : invalid UNO model !");
    Any aSupplier( xUnoModel->getPropertyValue(FM_PROP_FORMATSSUPPLIER));
    if (aSupplier.hasValue())
    {
        m_xSupplier.set(aSupplier, css::uno::UNO_QUERY);
        if (m_xSupplier.is())
        {
            // wenn wir den Supplier vom Model nehmen, dann auch den Key
            Any aFmtKey( xUnoModel->getPropertyValue(FM_PROP_FORMATKEY));
            if (aFmtKey.hasValue())
            {
                DBG_ASSERT(aFmtKey.getValueType().getTypeClass() == TypeClass_LONG, "DbFormattedField::Init : invalid format key property (no sal_Int32) !");
                nFormatKey = ::comphelper::getINT32(aFmtKey);
            }
            else
            {
                DBG_WARNING("DbFormattedField::Init : my uno-model has no format-key, but a formats supplier !");
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

    // nein ? vielleicht die ::com::sun::star::form::component::Form hinter dem Cursor ?
    if (!m_xSupplier.is())
    {
        Reference< XRowSet >  xCursorForm(xCursor, UNO_QUERY);
        if (xCursorForm.is())
        {   // wenn wir vom Cursor den Formatter nehmen, dann auch den Key vom Feld, an das wir gebunden sind
            m_xSupplier = getNumberFormats(getConnection(xCursorForm), false);

            if (m_rColumn.GetField().is())
                nFormatKey = ::comphelper::getINT32(m_rColumn.GetField()->getPropertyValue(FM_PROP_FORMATKEY));
        }
    }

    SvNumberFormatter* pFormatterUsed = NULL;
    if (m_xSupplier.is())
    {
        SvNumberFormatsSupplierObj* pImplmentation = SvNumberFormatsSupplierObj::getImplementation(m_xSupplier);
        if (pImplmentation)
            pFormatterUsed = pImplmentation->GetNumberFormatter();
        else
            // alles hingfaellig : der Supplier ist vom falschen Typ, dann koennen wir uns auch nicht darauf verlassen, dass
            // ein Standard-Formatter den (eventuell nicht-Standard-)Key kennt.
            nFormatKey = -1;
    }

    // einen Standard-Formatter ...
    if (pFormatterUsed == NULL)
    {
        pFormatterUsed = static_cast<FormattedField*>(m_pWindow)->StandardFormatter();
        DBG_ASSERT(pFormatterUsed != NULL, "DbFormattedField::Init : no standard formatter given by the numeric field !");
    }
    // ... und einen Standard-Key
    if (nFormatKey == -1)
        nFormatKey = 0;

    m_nKeyType  = comphelper::getNumberFormatType(m_xSupplier->getNumberFormats(), nFormatKey);

    static_cast<FormattedField*>(m_pWindow)->SetFormatter(pFormatterUsed);
    static_cast<FormattedField*>(m_pPainter)->SetFormatter(pFormatterUsed);

    static_cast<FormattedField*>(m_pWindow)->SetFormatKey(nFormatKey);
    static_cast<FormattedField*>(m_pPainter)->SetFormatKey(nFormatKey);

    static_cast<FormattedField*>(m_pWindow)->TreatAsNumber(m_rColumn.IsNumeric());
    static_cast<FormattedField*>(m_pPainter)->TreatAsNumber(m_rColumn.IsNumeric());

    // Min- und Max-Werte
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
                static_cast<FormattedField*>(m_pWindow)->SetMinValue(dMin);
                static_cast<FormattedField*>(m_pPainter)->SetMinValue(dMin);
                bClearMin = false;
            }
        }
        if (bClearMin)
        {
            static_cast<FormattedField*>(m_pWindow)->ClearMinValue();
            static_cast<FormattedField*>(m_pPainter)->ClearMinValue();
        }
        bool bClearMax = true;
        if (::comphelper::hasProperty(FM_PROP_EFFECTIVE_MAX, xUnoModel))
        {
            Any aMin( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_MAX));
            if (aMin.getValueType().getTypeClass() != TypeClass_VOID)
            {
                DBG_ASSERT(aMin.getValueType().getTypeClass() == TypeClass_DOUBLE, "DbFormattedField::Init : the model has an invalid max value !");
                double dMin = ::comphelper::getDouble(aMin);
                static_cast<FormattedField*>(m_pWindow)->SetMaxValue(dMin);
                static_cast<FormattedField*>(m_pPainter)->SetMaxValue(dMin);
                bClearMax = false;
            }
        }
        if (bClearMax)
        {
            static_cast<FormattedField*>(m_pWindow)->ClearMaxValue();
            static_cast<FormattedField*>(m_pPainter)->ClearMaxValue();
        }
    }

    // den Default-Wert
    Any aDefault( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_DEFAULT));
    if (aDefault.hasValue())
    {   // das Ding kann ein double oder ein String sein
        switch (aDefault.getValueType().getTypeClass())
        {
            case TypeClass_DOUBLE:
                if (m_rColumn.IsNumeric())
                {
                    static_cast<FormattedField*>(m_pWindow)->SetDefaultValue(::comphelper::getDouble(aDefault));
                    static_cast<FormattedField*>(m_pPainter)->SetDefaultValue(::comphelper::getDouble(aDefault));
                }
                else
                {
                    OUString sConverted;
                    Color* pDummy;
                    pFormatterUsed->GetOutputString(::comphelper::getDouble(aDefault), 0, sConverted, &pDummy);
                    static_cast<FormattedField*>(m_pWindow)->SetDefaultText(sConverted);
                    static_cast<FormattedField*>(m_pPainter)->SetDefaultText(sConverted);
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
                        static_cast<FormattedField*>(m_pWindow)->SetDefaultValue(dVal);
                        static_cast<FormattedField*>(m_pPainter)->SetDefaultValue(dVal);
                    }
                }
                else
                {
                    static_cast<FormattedField*>(m_pWindow)->SetDefaultText(sDefault);
                    static_cast<FormattedField*>(m_pPainter)->SetDefaultText(sDefault);
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
    return new ::svt::FormattedFieldCellController( static_cast< FormattedField* >( m_pWindow ) );
}


void DbFormattedField::_propertyChanged( const PropertyChangeEvent& _rEvent ) throw( RuntimeException )
{
    if (_rEvent.PropertyName == FM_PROP_FORMATKEY )
    {
        sal_Int32 nNewKey = _rEvent.NewValue.hasValue() ? ::comphelper::getINT32(_rEvent.NewValue) : 0;
        m_nKeyType = comphelper::getNumberFormatType(m_xSupplier->getNumberFormats(), nNewKey);

        DBG_ASSERT(m_pWindow && m_pPainter, "DbFormattedField::_propertyChanged : where are my windows ?");
        if (m_pWindow)
            static_cast< FormattedField* >( m_pWindow )->SetFormatKey( nNewKey );
        if (m_pPainter)
            static_cast< FormattedField* >( m_pPainter )->SetFormatKey( nNewKey );
    }
    else
    {
        DbLimitedLengthField::_propertyChanged( _rEvent );
    }
}


OUString DbFormattedField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/, Color** ppColor)
{
    // defaultmaessig keine Farb-Angabe
    if (ppColor != NULL)
        *ppColor = NULL;

    // NULL-Wert -> leerer Text
    if (!_rxField.is())
        return OUString();

    OUString aText;
    try
    {
        if (m_rColumn.IsNumeric())
        {
            // das IsNumeric an der Column sagt nichts aus ueber die Klasse des benutzen Formates, sondern
            // ueber die des an die Column gebundenen Feldes. Wenn man also eine FormattedField-Spalte an
            // ein double-Feld bindet und als Text formatiert, liefert m_rColumn.IsNumeric() sal_True. Das heisst
            // also einfach, dass ich den Inhalt der Variant mittels getDouble abfragen kann, und dann kann
            // ich den Rest (die Formatierung) dem FormattedField ueberlassen.
            double dValue = getValue( _rxField, m_rColumn.GetParent().getNullDate() );
            if (_rxField->wasNull())
                return aText;
            static_cast<FormattedField*>(m_pPainter)->SetValue(dValue);
        }
        else
        {
            // Hier kann ich nicht mit einem double arbeiten, da das Feld mir keines liefern kann.
            // Also einfach den Text vom ::com::sun::star::util::NumberFormatter in die richtige ::com::sun::star::form::component::Form brinden lassen.
            aText = _rxField->getString();
            if (_rxField->wasNull())
                return aText;
            static_cast<FormattedField*>(m_pPainter)->SetTextFormatted(aText);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    aText = m_pPainter->GetText();
    if (ppColor != NULL)
        *ppColor = static_cast<FormattedField*>(m_pPainter)->GetLastOutputColor();

    return aText;
}


void DbFormattedField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    try
    {
        FormattedField* pFormattedWindow = static_cast<FormattedField*>(m_pWindow);
        if (!_rxField.is())
        {   // NULL-Wert -> leerer Text
            m_pWindow->SetText(OUString());
        }
        else if (m_rColumn.IsNumeric())
        {
            // das IsNumeric an der Column sagt nichts aus ueber die Klasse des benutzen Formates, sondern
            // ueber die des an die Column gebundenen Feldes. Wenn man also eine FormattedField-Spalte an
            // ein double-Feld bindet und als Text formatiert, liefert m_rColumn.IsNumeric() sal_True. Das heisst
            // also einfach, dass ich den Inhalt der Variant mittels getDouble abfragen kann, und dann kann
            // ich den Rest (die Formatierung) dem FormattedField ueberlassen.
            double dValue = getValue( _rxField, m_rColumn.GetParent().getNullDate() );
            if (_rxField->wasNull())
                m_pWindow->SetText(OUString());
            else
                pFormattedWindow->SetValue(dValue);
        }
        else
        {
            // Hier kann ich nicht mit einem double arbeiten, da das Feld mir keines liefern kann.
            // Also einfach den Text vom ::com::sun::star::util::NumberFormatter in die richtige ::com::sun::star::form::component::Form brinden lassen.
            OUString sText( _rxField->getString());

            pFormattedWindow->SetTextFormatted( sText );
            pFormattedWindow->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}


void DbFormattedField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbFormattedField::updateFromModel: invalid call!" );

    FormattedField* pFormattedWindow = static_cast< FormattedField* >( m_pWindow );

    OUString sText;
    Any aValue = _rxModel->getPropertyValue( FM_PROP_EFFECTIVE_VALUE );
    if ( aValue >>= sText )
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
    FormattedField& rField = *static_cast<FormattedField*>(m_pWindow);
    DBG_ASSERT(&rField == m_pWindow, "DbFormattedField::commitControl : can't work with a window other than my own !");
    if (m_rColumn.IsNumeric())
    {
        if (!rField.GetText().isEmpty())
            aNewVal <<= rField.GetValue();
        // ein LeerString wird erst mal standardmaessig als void weitergereicht
    }
    else
        aNewVal <<= OUString(rField.GetTextValue());

    m_rColumn.getModel()->setPropertyValue(FM_PROP_EFFECTIVE_VALUE, aNewVal);
    return true;
}

DbCheckBox::DbCheckBox( DbGridColumn& _rColumn )
    :DbCellControl( _rColumn, true )
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
            aStyleSettings.SetOptions( aStyleSettings.GetOptions() | STYLE_OPTION_MONO );
        else
            aStyleSettings.SetOptions( aStyleSettings.GetOptions() & (~STYLE_OPTION_MONO) );
        aSettings.SetStyleSettings( aStyleSettings );
        _pWindow->SetSettings( aSettings );
    }
}


void DbCheckBox::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor )
{
    setTransparent( true );

    m_pWindow  = new CheckBoxControl( &rParent );
    m_pPainter = new CheckBoxControl( &rParent );

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
        static_cast< CheckBoxControl* >( m_pWindow )->GetBox().EnableTriState( bTristate );
        static_cast< CheckBoxControl* >( m_pPainter )->GetBox().EnableTriState( bTristate );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    DbCellControl::Init( rParent, xCursor );
}


CellControllerRef DbCheckBox::CreateController() const
{
    return new CheckBoxCellController(static_cast<CheckBoxControl*>(m_pWindow));
}

static void lcl_setCheckBoxState(   const Reference< ::com::sun::star::sdb::XColumn >& _rxField,
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    _pCheckBoxControl->GetBox().SetState(eState);
}


void DbCheckBox::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    lcl_setCheckBoxState( _rxField, static_cast<CheckBoxControl*>(m_pWindow) );
}


void DbCheckBox::PaintFieldToCell(OutputDevice& rDev, const Rectangle& rRect,
                          const Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                          const Reference< XNumberFormatter >& xFormatter)
{
    lcl_setCheckBoxState( _rxField, static_cast<CheckBoxControl*>(m_pPainter) );
    DbCellControl::PaintFieldToCell( rDev, rRect, _rxField, xFormatter );
}


void DbCheckBox::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbCheckBox::updateFromModel: invalid call!" );

    sal_Int16 nState = TRISTATE_INDET;
    _rxModel->getPropertyValue( FM_PROP_STATE ) >>= nState;
    static_cast< CheckBoxControl* >( m_pWindow )->GetBox().SetState( static_cast< TriState >( nState ) );
}


bool DbCheckBox::commitControl()
{
    m_rColumn.getModel()->setPropertyValue( FM_PROP_STATE,
                    makeAny( (sal_Int16)( static_cast< CheckBoxControl* >( m_pWindow )->GetBox().GetState() ) ) );
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
    if ( m_pWindow && _rxModel.is() )
    {
        OUString aLitMask;
        OUString aEditMask;
        bool bStrict = false;

        _rxModel->getPropertyValue( FM_PROP_LITERALMASK ) >>= aLitMask;
        _rxModel->getPropertyValue( FM_PROP_EDITMASK ) >>= aEditMask;
        _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) >>= bStrict;

        OString aAsciiEditMask(OUStringToOString(aEditMask, RTL_TEXTENCODING_ASCII_US));

        static_cast< PatternField* >( m_pWindow )->SetMask( aAsciiEditMask, aLitMask );
        static_cast< PatternField* >( m_pPainter )->SetMask( aAsciiEditMask, aLitMask );
        static_cast< PatternField* >( m_pWindow )->SetStrictFormat( bStrict );
        static_cast< PatternField* >( m_pPainter )->SetStrictFormat( bStrict );
    }
}


void DbPatternField::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor)
{
    m_rColumn.SetAlignmentFromModel(-1);

    m_pWindow = new PatternField( &rParent, 0 );
    m_pPainter= new PatternField( &rParent, 0 );

    Reference< XPropertySet >   xModel( m_rColumn.getModel() );
    implAdjustGenericFieldSetting( xModel );

    DbCellControl::Init( rParent, xCursor );
}


CellControllerRef DbPatternField::CreateController() const
{
    return new SpinCellController( static_cast< PatternField* >( m_pWindow ) );
}


OUString DbPatternField::impl_formatText( const OUString& _rText )
{
    m_pPainter->SetText( _rText );
    static_cast< PatternField* >( m_pPainter )->ReformatAll();
    return m_pPainter->GetText();
}


OUString DbPatternField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
    bool bIsForPaint = _rxField != m_rColumn.GetField();
    ::std::unique_ptr< FormattedColumnValue >& rpFormatter = bIsForPaint ? m_pPaintFormatter : m_pValueFormatter;

    if ( !rpFormatter.get() )
    {
        DBToolsObjectFactory aFactory;
        rpFormatter = aFactory.createFormattedColumnValue(
            m_xContext, getCursor(), Reference< XPropertySet >( _rxField, UNO_QUERY ) );
        OSL_ENSURE( rpFormatter.get(), "DbPatternField::Init: no value formatter!" );
    }
    else
        OSL_ENSURE( rpFormatter->getColumn() == _rxField, "DbPatternField::GetFormatText: my value formatter is working for another field ...!" );
        // re-creating the value formatter here every time would be quite expensive ...

    OUString sText;
    if ( rpFormatter.get() )
        sText = rpFormatter->getFormattedValue();

    return impl_formatText( sText );
}


void DbPatternField::UpdateFromField( const Reference< XColumn >& _rxField, const Reference< XNumberFormatter >& _rxFormatter )
{
    static_cast< Edit* >( m_pWindow )->SetText( GetFormatText( _rxField, _rxFormatter ) );
    static_cast< Edit* >( m_pWindow )->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}


void DbPatternField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbPatternField::updateFromModel: invalid call!" );

    OUString sText;
    _rxModel->getPropertyValue( FM_PROP_TEXT ) >>= sText;

    static_cast< Edit* >( m_pWindow )->SetText( impl_formatText( sText ) );
    static_cast< Edit* >( m_pWindow )->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
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
    return new SpinCellController( static_cast< SpinField* >( m_pWindow ) );
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
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Int32   nMin        = (sal_Int32)getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMIN ) );
        sal_Int32   nMax        = (sal_Int32)getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMAX ) );
        sal_Int32   nStep       = (sal_Int32)getDouble( _rxModel->getPropertyValue( FM_PROP_VALUESTEP ) );
        bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );
        sal_Int16   nScale      = getINT16( _rxModel->getPropertyValue( FM_PROP_DECIMAL_ACCURACY ) );
        bool    bThousand   = getBOOL( _rxModel->getPropertyValue( FM_PROP_SHOWTHOUSANDSEP ) );

        static_cast< DoubleNumericField* >( m_pWindow )->SetMinValue(nMin);
        static_cast< DoubleNumericField* >( m_pWindow )->SetMaxValue(nMax);
        static_cast< DoubleNumericField* >( m_pWindow )->SetSpinSize(nStep);
        static_cast< DoubleNumericField* >( m_pWindow )->SetStrictFormat(bStrict);

        static_cast< DoubleNumericField* >( m_pPainter )->SetMinValue(nMin);
        static_cast< DoubleNumericField* >( m_pPainter )->SetMaxValue(nMax);
        static_cast< DoubleNumericField* >( m_pPainter )->SetStrictFormat(bStrict);


        // dem Field und dem Painter einen Formatter spendieren
        // zuerst testen, ob ich von dem Service hinter einer Connection bekommen kann
        Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier;
        Reference< XRowSet > xForm;
        if ( m_rColumn.GetParent().getDataSource() )
            xForm = Reference< XRowSet >( Reference< XInterface >(*m_rColumn.GetParent().getDataSource()), UNO_QUERY );
        if ( xForm.is() )
            xSupplier = getNumberFormats( getConnection( xForm ), true );
        SvNumberFormatter* pFormatterUsed = NULL;
        if ( xSupplier.is() )
        {
            SvNumberFormatsSupplierObj* pImplmentation = SvNumberFormatsSupplierObj::getImplementation( xSupplier );
            pFormatterUsed = pImplmentation ? pImplmentation->GetNumberFormatter() : NULL;
        }
        if ( NULL == pFormatterUsed )
        {   // der Cursor fuehrte nicht zum Erfolg -> Standard
            pFormatterUsed = static_cast< DoubleNumericField* >( m_pWindow )->StandardFormatter();
            DBG_ASSERT( pFormatterUsed != NULL, "DbNumericField::implAdjustGenericFieldSetting: no standard formatter given by the numeric field !" );
        }
        static_cast< DoubleNumericField* >( m_pWindow )->SetFormatter( pFormatterUsed );
        static_cast< DoubleNumericField* >( m_pPainter )->SetFormatter( pFormatterUsed );

        // und dann ein Format generieren, dass die gewuenschten Nachkommastellen usw. hat
        LanguageType aAppLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType();
        OUString sFormatString = pFormatterUsed->GenerateFormat(0, aAppLanguage, bThousand, false, nScale);

        static_cast< DoubleNumericField* >( m_pWindow )->SetFormat( sFormatString, aAppLanguage );
        static_cast< DoubleNumericField* >( m_pPainter )->SetFormat( sFormatString, aAppLanguage );
    }
}


SpinField* DbNumericField::createField( vcl::Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& /*_rxModel*/  )
{
    return new DoubleNumericField( _pParent, _nFieldStyle );
}

namespace
{

    static OUString lcl_setFormattedNumeric_nothrow( DoubleNumericField& _rField, const DbCellControl& _rControl,
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
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return sValue;
    }
}


OUString DbNumericField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter, Color** /*ppColor*/)
{
    return lcl_setFormattedNumeric_nothrow(dynamic_cast<DoubleNumericField&>(*m_pPainter), *this, _rxField, _rxFormatter);
}


void DbNumericField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter)
{
    lcl_setFormattedNumeric_nothrow(dynamic_cast<DoubleNumericField&>(*m_pWindow), *this, _rxField, _rxFormatter);
}


void DbNumericField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbNumericField::updateFromModel: invalid call!" );

    double dValue = 0;
    if ( _rxModel->getPropertyValue( FM_PROP_VALUE ) >>= dValue )
        static_cast< DoubleNumericField* >( m_pWindow )->SetValue( dValue );
    else
        m_pWindow->SetText( OUString() );
}


bool DbNumericField::commitControl()
{
    OUString aText( m_pWindow->GetText());
    Any aVal;

    if (!aText.isEmpty())   // not empty
    {
        double fValue = static_cast<DoubleNumericField*>(m_pWindow)->GetValue();
        aVal <<= (double)fValue;
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
    if ( m_pWindow && _rxModel.is() )
    {
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

        static_cast< LongCurrencyField* >( m_pWindow )->SetUseThousandSep( bThousand );
        static_cast< LongCurrencyField* >( m_pWindow )->SetDecimalDigits( m_nScale );
        static_cast< LongCurrencyField* >( m_pWindow )->SetCurrencySymbol( aStr );
        static_cast< LongCurrencyField* >( m_pWindow )->SetFirst( nMin );
        static_cast< LongCurrencyField* >( m_pWindow )->SetLast( nMax );
        static_cast< LongCurrencyField* >( m_pWindow )->SetMin( nMin );
        static_cast< LongCurrencyField* >( m_pWindow )->SetMax( nMax );
        static_cast< LongCurrencyField* >( m_pWindow )->SetSpinSize( nStep );
        static_cast< LongCurrencyField* >( m_pWindow )->SetStrictFormat( bStrict );

        static_cast< LongCurrencyField* >( m_pPainter )->SetUseThousandSep( bThousand );
        static_cast< LongCurrencyField* >( m_pPainter )->SetDecimalDigits( m_nScale );
        static_cast< LongCurrencyField* >( m_pPainter )->SetCurrencySymbol( aStr );
        static_cast< LongCurrencyField* >( m_pPainter )->SetFirst( nMin );
        static_cast< LongCurrencyField* >( m_pPainter )->SetLast( nMax );
        static_cast< LongCurrencyField* >( m_pPainter )->SetMin( nMin );
        static_cast< LongCurrencyField* >( m_pPainter )->SetMax( nMax );
        static_cast< LongCurrencyField* >( m_pPainter )->SetStrictFormat( bStrict );
    }
}


SpinField* DbCurrencyField::createField( vcl::Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& /*_rxModel*/  )
{
    return new LongCurrencyField( _pParent, _nFieldStyle );
}


double DbCurrencyField::GetCurrency(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter) const
{
    double fValue = GetValue(_rxField, xFormatter);
    if (m_nScale)
    {
        // OSL_TRACE("double = %.64f ",fValue);
        fValue = ::rtl::math::pow10Exp(fValue, m_nScale);
        fValue = ::rtl::math::round(fValue, 0);
    }
    return fValue;
}

namespace
{

    static OUString lcl_setFormattedCurrency_nothrow( LongCurrencyField& _rField, const DbCurrencyField& _rControl,
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
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return sValue;
    }
}


OUString DbCurrencyField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter, Color** /*ppColor*/)
{
    return lcl_setFormattedCurrency_nothrow( dynamic_cast< LongCurrencyField& >( *m_pPainter ), *this, _rxField, _rxFormatter );
}


void DbCurrencyField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter)
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
            dValue = ::rtl::math::round(dValue, 0);
        }

        static_cast< LongCurrencyField* >( m_pWindow )->SetValue( dValue );
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
        double fValue = static_cast<LongCurrencyField*>(m_pWindow)->GetValue();
        if (m_nScale)
        {
            fValue /= ::rtl::math::pow10Exp(1.0, m_nScale);
        }
        aVal <<= (double)fValue;
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


SpinField* DbDateField::createField( vcl::Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& _rxModel  )
{
    // check if there is a DropDown property set to TRUE
    bool bDropDown =    !hasProperty( FM_PROP_DROPDOWN, _rxModel )
                        ||  getBOOL( _rxModel->getPropertyValue( FM_PROP_DROPDOWN ) );
    if ( bDropDown )
        _nFieldStyle |= WB_DROPDOWN;

    CalendarField* pField = new CalendarField( _pParent, _nFieldStyle );

    pField->EnableToday();
    pField->EnableNone();

    return pField;
}


void DbDateField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbDateField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbDateField::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
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

            static_cast<DateField*>( m_pWindow )->SetShowDateCentury( bShowDateCentury );
            static_cast<DateField*>( m_pPainter )->SetShowDateCentury( bShowDateCentury );
        }

        static_cast< DateField* >( m_pWindow )->SetExtDateFormat( (ExtDateFieldFormat)nFormat );
        static_cast< DateField* >( m_pWindow )->SetMin( aMin );
        static_cast< DateField* >( m_pWindow )->SetMax( aMax );
        static_cast< DateField* >( m_pWindow )->SetStrictFormat( bStrict );
        static_cast< DateField* >( m_pWindow )->EnableEmptyFieldValue( true );

        static_cast< DateField* >( m_pPainter )->SetExtDateFormat( (ExtDateFieldFormat)nFormat );
        static_cast< DateField* >( m_pPainter )->SetMin( aMin );
        static_cast< DateField* >( m_pPainter )->SetMax( aMax );
        static_cast< DateField* >( m_pPainter )->SetStrictFormat( bStrict );
        static_cast< DateField* >( m_pPainter )->EnableEmptyFieldValue( true );
    }
}

namespace
{

    static OUString lcl_setFormattedDate_nothrow( DateField& _rField, const Reference< XColumn >& _rxField )
    {
        OUString sDate;
        if ( _rxField.is() )
        {
            try
            {
                ::com::sun::star::util::Date aValue = _rxField->getDate();
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
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return sDate;
    }
}

OUString DbDateField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
     return lcl_setFormattedDate_nothrow(dynamic_cast<DateField&>(*m_pPainter), _rxField);
}


void DbDateField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    lcl_setFormattedDate_nothrow(dynamic_cast<DateField&>(*m_pWindow), _rxField);
}


void DbDateField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbDateField::updateFromModel: invalid call!" );

    util::Date aDate;
    if ( _rxModel->getPropertyValue( FM_PROP_DATE ) >>= aDate )
        static_cast< DateField* >( m_pWindow )->SetDate( ::Date( aDate ) );
    else
        static_cast< DateField* >( m_pWindow )->SetText( OUString() );
}


bool DbDateField::commitControl()
{
    OUString aText(m_pWindow->GetText());
    Any aVal;
    if (!aText.isEmpty())
        aVal <<= static_cast<DateField*>(m_pWindow)->GetDate().GetUNODate();
    else
        aVal.clear();

    m_rColumn.getModel()->setPropertyValue(FM_PROP_DATE, aVal);
    return true;
}

DbTimeField::DbTimeField( DbGridColumn& _rColumn )
    :DbSpinField( _rColumn, ::com::sun::star::awt::TextAlign::LEFT )
{
    doPropertyListening( FM_PROP_TIMEFORMAT );
    doPropertyListening( FM_PROP_TIMEMIN );
    doPropertyListening( FM_PROP_TIMEMAX );
    doPropertyListening( FM_PROP_STRICTFORMAT );
}


SpinField* DbTimeField::createField( vcl::Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& /*_rxModel*/ )
{
    return new TimeField( _pParent, _nFieldStyle );
}


void DbTimeField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbTimeField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbTimeField::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Int16   nFormat     = getINT16( _rxModel->getPropertyValue( FM_PROP_TIMEFORMAT ) );
        util::Time  aMin;
        OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_TIMEMIN ) >>= aMin );
        util::Time  aMax;
        OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_TIMEMAX ) >>= aMax );
        bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );

        static_cast< TimeField* >( m_pWindow )->SetExtFormat( (ExtTimeFieldFormat)nFormat );
        static_cast< TimeField* >( m_pWindow )->SetMin( aMin );
        static_cast< TimeField* >( m_pWindow )->SetMax( aMax );
        static_cast< TimeField* >( m_pWindow )->SetStrictFormat( bStrict );
        static_cast< TimeField* >( m_pWindow )->EnableEmptyFieldValue( true );

        static_cast< TimeField* >( m_pPainter )->SetExtFormat( (ExtTimeFieldFormat)nFormat );
        static_cast< TimeField* >( m_pPainter )->SetMin( aMin );
        static_cast< TimeField* >( m_pPainter )->SetMax( aMax );
        static_cast< TimeField* >( m_pPainter )->SetStrictFormat( bStrict );
        static_cast< TimeField* >( m_pPainter )->EnableEmptyFieldValue( true );
    }
}

namespace
{

    static OUString lcl_setFormattedTime_nothrow( TimeField& _rField, const Reference< XColumn >& _rxField )
    {
        OUString sTime;
        if ( _rxField.is() )
        {
            try
            {
                ::com::sun::star::util::Time aValue = _rxField->getTime();
                if ( _rxField->wasNull() )
                    _rField.SetText( sTime );
                else
                {
                    _rField.SetTime( ::tools::Time( aValue.Hours, aValue.Minutes, aValue.Seconds, aValue.NanoSeconds ) );
                    sTime = _rField.GetText();
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return sTime;
    }
}

OUString DbTimeField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
    return lcl_setFormattedTime_nothrow( *static_cast< TimeField* >( m_pPainter ), _rxField );
}


void DbTimeField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    lcl_setFormattedTime_nothrow( *static_cast< TimeField* >( m_pWindow ), _rxField );
}


void DbTimeField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbTimeField::updateFromModel: invalid call!" );

    util::Time aTime;
    if ( _rxModel->getPropertyValue( FM_PROP_DATE ) >>= aTime )
        static_cast< TimeField* >( m_pWindow )->SetTime( ::tools::Time( aTime ) );
    else
        static_cast< TimeField* >( m_pWindow )->SetText( OUString() );
}


bool DbTimeField::commitControl()
{
    OUString aText(m_pWindow->GetText());
    Any aVal;
    if (!aText.isEmpty())
        aVal <<= static_cast<TimeField*>(m_pWindow)->GetTime().GetUNOTime();
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


void DbComboBox::_propertyChanged( const PropertyChangeEvent& _rEvent ) throw( RuntimeException )
{
    if ( _rEvent.PropertyName.equals( FM_PROP_STRINGITEMLIST ) )
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
    ComboBoxControl* pField = static_cast<ComboBoxControl*>(m_pWindow);
    pField->Clear();

    ::comphelper::StringSequence aTest;
    if (rItems >>= aTest)
    {
        const OUString* pStrings = aTest.getConstArray();
        sal_Int32 nItems = aTest.getLength();
        for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
             pField->InsertEntry(*pStrings, LISTBOX_APPEND);

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
        static_cast< ComboBoxControl* >( m_pWindow )->SetDropDownLineCount( nLines );
    }
}


void DbComboBox::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor )
{
    m_rColumn.SetAlignmentFromModel(::com::sun::star::awt::TextAlign::LEFT);

    m_pWindow = new ComboBoxControl( &rParent );

    // selection von rechts nach links
    AllSettings     aSettings = m_pWindow->GetSettings();
    StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
    aStyleSettings.SetSelectionOptions(
        aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
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
    return new ComboBoxCellController(static_cast<ComboBoxControl*>(m_pWindow));
}


OUString DbComboBox::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter, Color** /*ppColor*/)
{
    const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xPS(_rxField, UNO_QUERY);
    ::dbtools::FormattedColumnValue fmter( xFormatter, xPS );

    return fmter.getFormattedValue();
}


void DbComboBox::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter)
{
    m_pWindow->SetText(GetFormatText(_rxField, xFormatter));
}


void DbComboBox::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbComboBox::updateFromModel: invalid call!" );

    OUString sText;
    _rxModel->getPropertyValue( FM_PROP_TEXT ) >>= sText;

    static_cast< ComboBox* >( m_pWindow )->SetText( sText );
    static_cast< ComboBox* >( m_pWindow )->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
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


void DbListBox::_propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent ) throw( RuntimeException )
{
    if ( _rEvent.PropertyName.equals( FM_PROP_STRINGITEMLIST ) )
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
    ListBoxControl* pField = static_cast<ListBoxControl*>(m_pWindow);

    pField->Clear();
    m_bBound = false;

    ::comphelper::StringSequence aTest;
    if (rItems >>= aTest)
    {
        const OUString* pStrings = aTest.getConstArray();
        sal_Int32 nItems = aTest.getLength();
        if (nItems)
        {
            for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
                 pField->InsertEntry(*pStrings, LISTBOX_APPEND);

            m_rColumn.getModel()->getPropertyValue(FM_PROP_VALUE_SEQ) >>= m_aValueList;
            m_bBound = m_aValueList.getLength() > 0;

            // tell the grid control that this controller is invalid and has to be re-initialized
            invalidatedController();
        }
    }
}


void DbListBox::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor)
{
    m_rColumn.SetAlignment(::com::sun::star::awt::TextAlign::LEFT);

    m_pWindow = new ListBoxControl( &rParent );

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
        static_cast< ListBoxControl* >( m_pWindow )->SetDropDownLineCount( nLines );
    }
}


CellControllerRef DbListBox::CreateController() const
{
    return new ListBoxCellController(static_cast<ListBoxControl*>(m_pWindow));
}


OUString DbListBox::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
    OUString sText;
    if ( _rxField.is() )
    {
        try
        {
            sText = _rxField->getString();
            if ( m_bBound )
            {
                Sequence< sal_Int16 > aPosSeq = ::comphelper::findValue( m_aValueList, sText, true );
                if ( aPosSeq.getLength() )
                    sText = static_cast<ListBox*>(m_pWindow)->GetEntry(aPosSeq.getConstArray()[0]);
                else
                    sText = "";
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return sText;
}


void DbListBox::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< XNumberFormatter >& xFormatter)
{
    OUString sFormattedText( GetFormatText( _rxField, xFormatter ) );
    if (!sFormattedText.isEmpty())
        static_cast< ListBox* >( m_pWindow )->SelectEntry( sFormattedText );
    else
        static_cast< ListBox* >( m_pWindow )->SetNoSelection();
}


void DbListBox::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbListBox::updateFromModel: invalid call!" );

    Sequence< sal_Int16 > aSelection;
    _rxModel->getPropertyValue( FM_PROP_SELECT_SEQ );

    sal_Int16 nSelection = -1;
    if ( aSelection.getLength() > 0 )
        nSelection = aSelection[ 0 ];

    ListBox* pListBox = static_cast< ListBox* >( m_pWindow );

    if ( ( nSelection >= 0 ) && ( nSelection < pListBox->GetEntryCount() ) )
        pListBox->SelectEntryPos( nSelection );
    else
        pListBox->SetNoSelection( );
}


bool DbListBox::commitControl()
{
    Any aVal;
    Sequence<sal_Int16> aSelectSeq;
    if (static_cast<ListBox*>(m_pWindow)->GetSelectEntryCount())
    {
        aSelectSeq.realloc(1);
        *(sal_Int16 *)aSelectSeq.getArray() = (sal_Int16)static_cast<ListBox*>(m_pWindow)->GetSelectEntryPos();
    }
    aVal <<= aSelectSeq;
    m_rColumn.getModel()->setPropertyValue(FM_PROP_SELECT_SEQ, aVal);
    return true;
}

DbFilterField::DbFilterField(const Reference< XComponentContext >& rxContext,DbGridColumn& _rColumn)
              :DbCellControl(_rColumn)
              ,OSQLParserClient(rxContext)
              ,m_nControlClass(::com::sun::star::form::FormComponentType::TEXTFIELD)
              ,m_bFilterList(false)
              ,m_bFilterListFilled(false)
              ,m_bBound(false)
{

    setAlignedController( false );
}


DbFilterField::~DbFilterField()
{
    if (m_nControlClass == ::com::sun::star::form::FormComponentType::CHECKBOX)
        static_cast<CheckBoxControl*>(m_pWindow)->SetClickHdl( Link() );

}


void DbFilterField::PaintCell(OutputDevice& rDev, const Rectangle& rRect)
{
    static sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_VCENTER | TEXT_DRAW_LEFT;
    switch (m_nControlClass)
    {
        case FormComponentType::CHECKBOX:
            DbCellControl::PaintCell( rDev, rRect );
            break;
        case FormComponentType::LISTBOX:
            rDev.DrawText(rRect, static_cast<ListBox*>(m_pWindow)->GetSelectEntry(), nStyle);
            break;
        default:
            rDev.DrawText(rRect, m_aText, nStyle);
    }
}


void DbFilterField::SetList(const Any& rItems, bool bComboBox)
{
    ::comphelper::StringSequence aTest;
    rItems >>= aTest;
    const OUString* pStrings = aTest.getConstArray();
    sal_Int32 nItems = aTest.getLength();
    if (nItems)
    {
        if (bComboBox)
        {
            ComboBox* pField = static_cast<ComboBox*>(m_pWindow);
            for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
                pField->InsertEntry(*pStrings, LISTBOX_APPEND);
        }
        else
        {
            ListBox* pField = static_cast<ListBox*>(m_pWindow);
            for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
                pField->InsertEntry(*pStrings, LISTBOX_APPEND);

            m_rColumn.getModel()->getPropertyValue(FM_PROP_VALUE_SEQ) >>= m_aValueList;
            m_bBound = m_aValueList.getLength() > 0;
        }
    }
}


void DbFilterField::CreateControl(vcl::Window* pParent, const Reference< ::com::sun::star::beans::XPropertySet >& xModel)
{
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
            m_pWindow = new CheckBoxControl(pParent);
            m_pWindow->SetPaintTransparent( true );
            static_cast<CheckBoxControl*>(m_pWindow)->SetClickHdl( LINK( this, DbFilterField, OnClick ) );

            m_pPainter = new CheckBoxControl(pParent);
            m_pPainter->SetPaintTransparent( true );
            m_pPainter->SetBackground();
            break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
        {
            m_pWindow = new ListBoxControl(pParent);
            sal_Int16  nLines       = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_LINECOUNT));
            Any  aItems      = xModel->getPropertyValue(FM_PROP_STRINGITEMLIST);
            SetList(aItems, m_nControlClass == ::com::sun::star::form::FormComponentType::COMBOBOX);
            static_cast<ListBox*>(m_pWindow)->SetDropDownLineCount(nLines);
        }   break;
        case ::com::sun::star::form::FormComponentType::COMBOBOX:
        {
            m_pWindow = new ComboBoxControl(pParent);

            AllSettings     aSettings = m_pWindow->GetSettings();
            StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                           aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings, true);

            if (!m_bFilterList)
            {
                sal_Int16  nLines       = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_LINECOUNT));
                Any  aItems      = xModel->getPropertyValue(FM_PROP_STRINGITEMLIST);
                SetList(aItems, m_nControlClass == ::com::sun::star::form::FormComponentType::COMBOBOX);
                static_cast<ComboBox*>(m_pWindow)->SetDropDownLineCount(nLines);
            }
            else
                static_cast<ComboBox*>(m_pWindow)->SetDropDownLineCount(5);

        }   break;
        default:
        {
            m_pWindow  = new Edit(pParent, WB_LEFT);
            AllSettings     aSettings = m_pWindow->GetSettings();
            StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                           aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings, true);
        }
    }
}


void DbFilterField::Init( vcl::Window& rParent, const Reference< XRowSet >& xCursor )
{
    Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
    m_rColumn.SetAlignment(::com::sun::star::awt::TextAlign::LEFT);

    if (xModel.is())
    {
        m_bFilterList = ::comphelper::hasProperty(FM_PROP_FILTERPROPOSAL, xModel) && ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_FILTERPROPOSAL));
        if (m_bFilterList)
            m_nControlClass = ::com::sun::star::form::FormComponentType::COMBOBOX;
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
    Edit* pAsEdit = dynamic_cast< Edit* >( m_pWindow );
    if ( pAsEdit )
        pAsEdit->SetReadOnly( false );
}


CellControllerRef DbFilterField::CreateController() const
{
    CellControllerRef xController;
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
            xController = new CheckBoxCellController(static_cast<CheckBoxControl*>(m_pWindow));
            break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
            xController = new ListBoxCellController(static_cast<ListBoxControl*>(m_pWindow));
            break;
        case ::com::sun::star::form::FormComponentType::COMBOBOX:
            xController = new ComboBoxCellController(static_cast<ComboBoxControl*>(m_pWindow));
            break;
        default:
            if (m_bFilterList)
                xController = new ComboBoxCellController(static_cast<ComboBoxControl*>(m_pWindow));
            else
                xController = new EditCellController(static_cast<Edit*>(m_pWindow));
    }
    return xController;
}


void DbFilterField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbFilterField::updateFromModel: invalid call!" );
    (void)_rxModel;

    OSL_FAIL( "DbListBox::updateFromModel: not implemented yet (how the hell did you reach this?)!" );
    // TODO: implement this.
    // remember: updateFromModel should be some kind of opposite of commitControl
}


bool DbFilterField::commitControl()
{
    OUString aText(m_aText);
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
            return true;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
            aText.clear();
            if (static_cast<ListBox*>(m_pWindow)->GetSelectEntryCount())
            {
                sal_Int16 nPos = (sal_Int16)static_cast<ListBox*>(m_pWindow)->GetSelectEntryPos();
                if ( ( nPos >= 0 ) && ( nPos < m_aValueList.getLength() ) )
                    aText = m_aValueList.getConstArray()[nPos];
            }

            if (m_aText != aText)
            {
                m_aText = aText;
                m_aCommitLink.Call(this);
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

            ::rtl::Reference< ISQLParseNode > xParseNode = predicateTree(aErrorMsg, aNewText,xNumberFormatter, m_rColumn.GetField());
            if (xParseNode.is())
            {
                OUString aPreparedText;

                ::com::sun::star::lang::Locale aAppLocale = Application::GetSettings().GetUILanguageTag().getLocale();

                Reference< XRowSet > xDataSourceRowSet(
                    Reference< XInterface >(*m_rColumn.GetParent().getDataSource()), UNO_QUERY);
                Reference< XConnection >  xConnection(getConnection(xDataSourceRowSet));

                xParseNode->parseNodeToPredicateStr(aPreparedText,
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
        m_aCommitLink.Call(this);
    }
    return true;
}


void DbFilterField::SetText(const OUString& rText)
{
    m_aText = rText;
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
        {
            TriState eState;
            if (rText == "1")
                eState = TRISTATE_TRUE;
            else if (rText == "0")
                eState = TRISTATE_FALSE;
            else
                eState = TRISTATE_INDET;

            static_cast<CheckBoxControl*>(m_pWindow)->GetBox().SetState(eState);
            static_cast<CheckBoxControl*>(m_pPainter)->GetBox().SetState(eState);
        }   break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
        {
            Sequence<sal_Int16> aPosSeq = ::comphelper::findValue(m_aValueList, m_aText, true);
            if (aPosSeq.getLength())
                static_cast<ListBox*>(m_pWindow)->SelectEntryPos(aPosSeq.getConstArray()[0], true);
            else
                static_cast<ListBox*>(m_pWindow)->SetNoSelection();
        }   break;
        default:
            m_pWindow->SetText(m_aText);
    }

    // now force a repaint on the window
    m_rColumn.GetParent().RowModified(0,m_rColumn.GetId());
}


void DbFilterField::Update()
{
    // should we fill the combobox with a filter proposal?
    if (m_bFilterList && !m_bFilterListFilled)
    {
        m_bFilterListFilled = true;
        Reference< ::com::sun::star::beans::XPropertySet >  xField = m_rColumn.GetField();
        if (!xField.is())
            return;

        OUString aName;
        xField->getPropertyValue(FM_PROP_NAME) >>= aName;

        // the columnmodel
        Reference< ::com::sun::star::container::XChild >  xModelAsChild(m_rColumn.getModel(), UNO_QUERY);
        // the grid model
        xModelAsChild = Reference< ::com::sun::star::container::XChild > (xModelAsChild->getParent(),UNO_QUERY);
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
        Reference< ::com::sun::star::container::XNameAccess >    xFieldNames = xSupCol->getColumns();
        if (!xFieldNames->hasByName(aName))
            return;

        Reference< ::com::sun::star::container::XNameAccess >    xTablesNames = xSupTab->getTables();
        Reference< ::com::sun::star::beans::XPropertySet >       xComposerFieldAsSet(xFieldNames->getByName(aName),UNO_QUERY);

        if (xComposerFieldAsSet.is() && ::comphelper::hasProperty(FM_PROP_TABLENAME, xComposerFieldAsSet) &&
            ::comphelper::hasProperty(FM_PROP_FIELDSOURCE, xComposerFieldAsSet))
        {
            OUString aFieldName;
            OUString aTableName;
            xComposerFieldAsSet->getPropertyValue(FM_PROP_FIELDSOURCE)  >>= aFieldName;
            xComposerFieldAsSet->getPropertyValue(FM_PROP_TABLENAME)    >>= aTableName;

            // no possibility to create a select statement
            // looking for the complete table name
            if (!xTablesNames->hasByName(aTableName))
                return;

            // ein Statement aufbauen und abschicken als query
            // Access to the connection
            Reference< XStatement >  xStatement;
            Reference< XResultSet >  xListCursor;
            Reference< ::com::sun::star::sdb::XColumn >  xDataField;

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
                Reference< ::com::sun::star::beans::XPropertySet >  xStatementProps(xStatement, UNO_QUERY);
                xStatementProps->setPropertyValue(FM_PROP_ESCAPE_PROCESSING, makeAny(true));

                xListCursor = xStatement->executeQuery(aStatement.makeStringAndClear());

                Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(xListCursor, UNO_QUERY);
                Reference< ::com::sun::star::container::XIndexAccess >  xFields(xSupplyCols->getColumns(), UNO_QUERY);
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
            com::sun::star::util::Date aNullDate = m_rColumn.GetParent().getNullDate();
            sal_Int32 nFormatKey = m_rColumn.GetKey();
            Reference< XNumberFormatter >  xFormatter = m_rColumn.GetParent().getNumberFormatter();
            sal_Int16 nKeyType = ::comphelper::getNumberFormatType(xFormatter->getNumberFormatsSupplier()->getNumberFormats(), nFormatKey);

            while (!xListCursor->isAfterLast() && i++ < SHRT_MAX) // max anzahl eintraege
            {
                aStr = getFormattedValue(xDataField, xFormatter, aNullDate, nFormatKey, nKeyType);
                aStringList.push_back(aStr);
                (void)xListCursor->next();
            }

            // filling the entries for the combobox
            for (::std::vector< OUString >::const_iterator iter = aStringList.begin();
                 iter != aStringList.end(); ++iter)
                static_cast<ComboBox*>(m_pWindow)->InsertEntry(*iter, LISTBOX_APPEND);
        }
    }
}


OUString DbFilterField::GetFormatText(const Reference< XColumn >& /*_rxField*/, const Reference< XNumberFormatter >& /*xFormatter*/, Color** /*ppColor*/)
{
    return OUString();
}


void DbFilterField::UpdateFromField(const Reference< XColumn >& /*_rxField*/, const Reference< XNumberFormatter >& /*xFormatter*/)
{
    OSL_FAIL( "DbFilterField::UpdateFromField: cannot update a filter control from a field!" );
}


IMPL_LINK_NOARG(DbFilterField, OnClick)
{
    TriState eState = static_cast<CheckBoxControl*>(m_pWindow)->GetBox().GetState();
    OUString aText;

    switch (eState)
    {
        case TRISTATE_TRUE:
            aText = "1";
            break;
        case TRISTATE_FALSE:
            aText = "0";
            break;
        case TRISTATE_INDET:
            break;
    }

    if (m_aText != aText)
    {
        m_aText = aText;
        m_aCommitLink.Call(this);
    }
    return 1;
}

TYPEINIT0(FmXGridCell);



FmXGridCell::FmXGridCell( DbGridColumn* pColumn, DbCellControl* _pControl )
            :OComponentHelper(m_aMutex)
            ,m_pColumn(pColumn)
            ,m_pCellControl( _pControl )
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
    return NULL;
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

Sequence< Type > SAL_CALL FmXGridCell::getTypes( ) throw (RuntimeException, std::exception)
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
    m_pColumn = NULL;
    DELETEZ(m_pCellControl);
}


Any SAL_CALL FmXGridCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException, std::exception)
{
    Any aReturn = OComponentHelper::queryAggregation( _rType );

    if ( !aReturn.hasValue() )
        aReturn = FmXGridCell_Base::queryInterface( _rType );

    if ( !aReturn.hasValue() && ( m_pCellControl != NULL ) )
        aReturn = FmXGridCell_WindowBase::queryInterface( _rType );

    return aReturn;
}

// ::com::sun::star::awt::XControl

Reference< XInterface >  FmXGridCell::getContext() throw( RuntimeException, std::exception )
{
    return Reference< XInterface > ();
}


Reference< ::com::sun::star::awt::XControlModel >  FmXGridCell::getModel() throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    return Reference< ::com::sun::star::awt::XControlModel > (m_pColumn->getModel(), UNO_QUERY);
}

// ::com::sun::star::form::XBoundControl

sal_Bool FmXGridCell::getLock() throw( RuntimeException, std::exception )
{
    return m_pColumn->isLocked();
}


void FmXGridCell::setLock(sal_Bool _bLock) throw( RuntimeException, std::exception )
{
    if (getLock() == _bLock)
        return;
    else
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        m_pColumn->setLock(_bLock);
    }
}


void SAL_CALL FmXGridCell::setPosSize( ::sal_Int32 _XX, ::sal_Int32 _Y, ::sal_Int32 _Width, ::sal_Int32 _Height, ::sal_Int16 _Flags ) throw (RuntimeException, std::exception)
{
    OSL_FAIL( "FmXGridCell::setPosSize: not implemented" );
    (void)_XX;
    (void)_Y;
    (void)_Width;
    (void)_Height;
    (void)_Flags;
    // not allowed to tamper with this for a grid cell
}


awt::Rectangle SAL_CALL FmXGridCell::getPosSize(  ) throw (RuntimeException, std::exception)
{
    OSL_FAIL( "FmXGridCell::getPosSize: not implemented" );
    return awt::Rectangle();
}


void SAL_CALL FmXGridCell::setVisible( sal_Bool _Visible ) throw (RuntimeException, std::exception)
{
    OSL_FAIL( "FmXGridCell::setVisible: not implemented" );
    (void)_Visible;
    // not allowed to tamper with this for a grid cell
}


void SAL_CALL FmXGridCell::setEnable( sal_Bool _Enable ) throw (RuntimeException, std::exception)
{
    OSL_FAIL( "FmXGridCell::setEnable: not implemented" );
    (void)_Enable;
    // not allowed to tamper with this for a grid cell
}


void SAL_CALL FmXGridCell::setFocus(  ) throw (RuntimeException, std::exception)
{
    OSL_FAIL( "FmXGridCell::setFocus: not implemented" );
    // not allowed to tamper with this for a grid cell
}


void SAL_CALL FmXGridCell::addWindowListener( const Reference< awt::XWindowListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aWindowListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeWindowListener( const Reference< awt::XWindowListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aWindowListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addFocusListener( const Reference< awt::XFocusListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aFocusListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeFocusListener( const Reference< awt::XFocusListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aFocusListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addKeyListener( const Reference< awt::XKeyListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aKeyListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeKeyListener( const Reference< awt::XKeyListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aKeyListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addMouseListener( const Reference< awt::XMouseListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aMouseListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeMouseListener( const Reference< awt::XMouseListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aMouseListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addMouseMotionListener( const Reference< awt::XMouseMotionListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aMouseMotionListeners.addInterface( _rxListener );
}


void SAL_CALL FmXGridCell::removeMouseMotionListener( const Reference< awt::XMouseMotionListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    m_aMouseMotionListeners.removeInterface( _rxListener );
}


void SAL_CALL FmXGridCell::addPaintListener( const Reference< awt::XPaintListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    OSL_FAIL( "FmXGridCell::addPaintListener: not implemented" );
    (void)_rxListener;
}


void SAL_CALL FmXGridCell::removePaintListener( const Reference< awt::XPaintListener >& _rxListener ) throw (RuntimeException, std::exception)
{
    OSL_FAIL( "FmXGridCell::removePaintListener: not implemented" );
    (void)_rxListener;
}


IMPL_LINK( FmXGridCell, OnWindowEvent, VclWindowEvent*, _pEvent )
{
    ENSURE_OR_THROW( _pEvent, "illegal event pointer" );
    ENSURE_OR_THROW( _pEvent->GetWindow(), "illegal window" );
    onWindowEvent( _pEvent->GetId(), *_pEvent->GetWindow(), _pEvent->GetData() );
    return 1L;
}


void FmXGridCell::onFocusGained( const awt::FocusEvent& _rEvent )
{
    m_aFocusListeners.notifyEach( &awt::XFocusListener::focusGained, _rEvent );
}


void FmXGridCell::onFocusLost( const awt::FocusEvent& _rEvent )
{
    m_aFocusListeners.notifyEach( &awt::XFocusListener::focusLost, _rEvent );
}


void FmXGridCell::onWindowEvent( const sal_uIntPtr _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{
    switch ( _nEventId )
    {
    case VCLEVENT_CONTROL_GETFOCUS:
    case VCLEVENT_WINDOW_GETFOCUS:
    case VCLEVENT_CONTROL_LOSEFOCUS:
    case VCLEVENT_WINDOW_LOSEFOCUS:
    {
        if  (   (   _rWindow.IsCompoundControl()
                &&  (   _nEventId == VCLEVENT_CONTROL_GETFOCUS
                    ||  _nEventId == VCLEVENT_CONTROL_LOSEFOCUS
                    )
                )
            ||  (   !_rWindow.IsCompoundControl()
                &&  (   _nEventId == VCLEVENT_WINDOW_GETFOCUS
                    ||  _nEventId == VCLEVENT_WINDOW_LOSEFOCUS
                    )
                )
            )
        {
            if ( !m_aFocusListeners.getLength() )
                break;

            bool bFocusGained = ( _nEventId == VCLEVENT_CONTROL_GETFOCUS ) || ( _nEventId == VCLEVENT_WINDOW_GETFOCUS );

            awt::FocusEvent aEvent;
            aEvent.Source = *this;
            aEvent.FocusFlags = _rWindow.GetGetFocusFlags();
            aEvent.Temporary = sal_False;

            if ( bFocusGained )
                onFocusGained( aEvent );
            else
                onFocusLost( aEvent );
        }
    }
    break;
    case VCLEVENT_WINDOW_MOUSEBUTTONDOWN:
    case VCLEVENT_WINDOW_MOUSEBUTTONUP:
    {
        if ( !m_aMouseListeners.getLength() )
            break;

        const bool bButtonDown = ( _nEventId == VCLEVENT_WINDOW_MOUSEBUTTONDOWN );

        awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *static_cast< const ::MouseEvent* >( _pEventData ), *this ) );
        m_aMouseListeners.notifyEach( bButtonDown ? &awt::XMouseListener::mousePressed : &awt::XMouseListener::mouseReleased, aEvent );
    }
    break;
    case VCLEVENT_WINDOW_MOUSEMOVE:
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
    case VCLEVENT_WINDOW_KEYINPUT:
    case VCLEVENT_WINDOW_KEYUP:
    {
        if ( !m_aKeyListeners.getLength() )
            break;

        const bool bKeyPressed = ( _nEventId == VCLEVENT_WINDOW_KEYINPUT );
        awt::KeyEvent aEvent( VCLUnoHelper::createKeyEvent( *static_cast< const ::KeyEvent* >( _pEventData ), *this ) );
        m_aKeyListeners.notifyEach( bKeyPressed ? &awt::XKeyListener::keyPressed: &awt::XKeyListener::keyReleased, aEvent );
    }
    break;
    }
}

TYPEINIT1(FmXDataCell, FmXGridCell);

void FmXDataCell::PaintFieldToCell(OutputDevice& rDev, const Rectangle& rRect,
                        const Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                        const Reference< XNumberFormatter >& xFormatter)
{
    m_pCellControl->PaintFieldToCell( rDev, rRect, _rxField, xFormatter );
}


void FmXDataCell::UpdateFromColumn()
{
    Reference< ::com::sun::star::sdb::XColumn >  xField(m_pColumn->GetCurrentFieldValue());
    if (xField.is())
        m_pCellControl->UpdateFromField(xField, m_pColumn->GetParent().getNumberFormatter());
}

TYPEINIT1(FmXTextCell, FmXDataCell);

FmXTextCell::FmXTextCell( DbGridColumn* pColumn, DbCellControl& _rControl )
    :FmXDataCell( pColumn, _rControl )
    ,m_bFastPaint( true )
{
}


void FmXTextCell::PaintFieldToCell(OutputDevice& rDev,
                        const Rectangle& rRect,
                        const Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                        const Reference< XNumberFormatter >& xFormatter)
{
    if ( !m_bFastPaint )
    {
        FmXDataCell::PaintFieldToCell( rDev, rRect, _rxField, xFormatter );
        return;
    }

    sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_VCENTER;
    if ( ( rDev.GetOutDevType() == OUTDEV_WINDOW ) && !static_cast< vcl::Window& >( rDev ).IsEnabled() )
        nStyle |= TEXT_DRAW_DISABLE;

    switch (m_pColumn->GetAlignment())
    {
        case ::com::sun::star::awt::TextAlign::RIGHT:
            nStyle |= TEXT_DRAW_RIGHT;
            break;
        case ::com::sun::star::awt::TextAlign::CENTER:
            nStyle |= TEXT_DRAW_CENTER;
            break;
        default:
            nStyle |= TEXT_DRAW_LEFT;
    }

    Color* pColor = NULL;
    OUString aText = GetText(_rxField, xFormatter, &pColor);
    if (pColor != NULL)
    {
        Color aOldTextColor( rDev.GetTextColor() );
        rDev.SetTextColor( *pColor );
        rDev.DrawText(rRect, aText, nStyle);
        rDev.SetTextColor( aOldTextColor );
    }
    else
        rDev.DrawText(rRect, aText, nStyle);
}

FmXEditCell::FmXEditCell( DbGridColumn* pColumn, DbCellControl& _rControl )
            :FmXTextCell( pColumn, _rControl )
            ,m_aTextListeners(m_aMutex)
            ,m_aChangeListeners( m_aMutex )
            ,m_pEditImplementation( NULL )
            ,m_bOwnEditImplementation( false )
{

    DbTextField* pTextField = PTR_CAST( DbTextField, &_rControl );
    if ( pTextField )
    {

        m_pEditImplementation = pTextField->GetEditImplementation();
        if ( !pTextField->IsSimpleEdit() )
            m_bFastPaint = false;
    }
    else
    {
        m_pEditImplementation = new EditImplementation( static_cast< Edit& >( _rControl.GetWindow() ) );
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
    ::com::sun::star::lang::EventObject aEvt(*this);
    m_aTextListeners.disposeAndClear(aEvt);
    m_aChangeListeners.disposeAndClear(aEvt);

    m_pEditImplementation->SetModifyHdl( Link() );
    if ( m_bOwnEditImplementation )
        delete m_pEditImplementation;
    m_pEditImplementation = NULL;

    FmXDataCell::disposing();
}


Any SAL_CALL FmXEditCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException, std::exception)
{
    Any aReturn = FmXTextCell::queryAggregation( _rType );

    if ( !aReturn.hasValue() )
        aReturn = FmXEditCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXEditCell::getTypes(  ) throw(RuntimeException, std::exception)
{
    return ::comphelper::concatSequences(
        FmXTextCell::getTypes(),
        FmXEditCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXEditCell )

// ::com::sun::star::awt::XTextComponent

void SAL_CALL FmXEditCell::addTextListener(const Reference< ::com::sun::star::awt::XTextListener >& l) throw( RuntimeException, std::exception )
{
    m_aTextListeners.addInterface( l );
}


void SAL_CALL FmXEditCell::removeTextListener(const Reference< ::com::sun::star::awt::XTextListener >& l) throw( RuntimeException, std::exception )
{
    m_aTextListeners.removeInterface( l );
}


void SAL_CALL FmXEditCell::setText( const OUString& aText ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
    {
        m_pEditImplementation->SetText( aText );

        // In JAVA wird auch ein textChanged ausgeloest, in VCL nicht.
        // ::com::sun::star::awt::Toolkit soll JAVA-komform sein...
        onTextChanged();
    }
}


void SAL_CALL FmXEditCell::insertText(const ::com::sun::star::awt::Selection& rSel, const OUString& aText) throw(RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
    {
        m_pEditImplementation->SetSelection( Selection( rSel.Min, rSel.Max ) );
        m_pEditImplementation->ReplaceSelected( aText );
    }
}


OUString SAL_CALL FmXEditCell::getText() throw( RuntimeException, std::exception )
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
            Reference< ::com::sun::star::sdb::XColumn >  xField(m_pColumn->GetCurrentFieldValue());
            if (xField.is())
                aText = GetText(xField, m_pColumn->GetParent().getNumberFormatter());
        }
    }
    return aText;
}


OUString SAL_CALL FmXEditCell::getSelectedText( void ) throw( RuntimeException, std::exception )
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


void SAL_CALL FmXEditCell::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
        m_pEditImplementation->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}


::com::sun::star::awt::Selection SAL_CALL FmXEditCell::getSelection( void ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Selection aSel;
    if ( m_pEditImplementation )
        aSel = m_pEditImplementation->GetSelection();

    return ::com::sun::star::awt::Selection(aSel.Min(), aSel.Max());
}


sal_Bool SAL_CALL FmXEditCell::isEditable( void ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return ( m_pEditImplementation && !m_pEditImplementation->IsReadOnly() && m_pEditImplementation->GetControl().IsEnabled() ) ? sal_True : sal_False;
}


void SAL_CALL FmXEditCell::setEditable( sal_Bool bEditable ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
        m_pEditImplementation->SetReadOnly( !bEditable );
}


sal_Int16 SAL_CALL FmXEditCell::getMaxTextLen() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pEditImplementation ? m_pEditImplementation->GetMaxTextLen() : 0;
}


void SAL_CALL FmXEditCell::setMaxTextLen( sal_Int16 nLen ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
        m_pEditImplementation->SetMaxTextLen( nLen );
}


void SAL_CALL FmXEditCell::addChangeListener( const Reference< form::XChangeListener >& _Listener ) throw (RuntimeException, std::exception)
{
    m_aChangeListeners.addInterface( _Listener );
}


void SAL_CALL FmXEditCell::removeChangeListener( const Reference< form::XChangeListener >& _Listener ) throw (RuntimeException, std::exception)
{
    m_aChangeListeners.removeInterface( _Listener );
}


void FmXEditCell::onTextChanged()
{
    ::com::sun::star::awt::TextEvent aEvent;
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


void FmXEditCell::onWindowEvent( const sal_uIntPtr _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{
    switch ( _nEventId )
    {
    case VCLEVENT_EDIT_MODIFY:
    {
        if ( m_pEditImplementation && m_aTextListeners.getLength() )
            onTextChanged();
        return;
    }
    }

    FmXTextCell::onWindowEvent( _nEventId, _rWindow, _pEventData );
}

FmXCheckBoxCell::FmXCheckBoxCell( DbGridColumn* pColumn, DbCellControl& _rControl )
                :FmXDataCell( pColumn, _rControl )
                ,m_aItemListeners(m_aMutex)
                ,m_aActionListeners( m_aMutex )
                ,m_pBox( & static_cast< CheckBoxControl& >( _rControl.GetWindow() ).GetBox() )
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
    ::com::sun::star::lang::EventObject aEvt(*this);
    m_aItemListeners.disposeAndClear(aEvt);
    m_aActionListeners.disposeAndClear(aEvt);

    static_cast< CheckBoxControl& >( m_pCellControl->GetWindow() ).SetClickHdl(Link());
    m_pBox = NULL;

    FmXDataCell::disposing();
}


Any SAL_CALL FmXCheckBoxCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException, std::exception)
{
    Any aReturn = FmXDataCell::queryAggregation( _rType );

    if ( !aReturn.hasValue() )
        aReturn = FmXCheckBoxCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXCheckBoxCell::getTypes(  ) throw(RuntimeException, std::exception)
{
    return ::comphelper::concatSequences(
        FmXDataCell::getTypes(),
        FmXCheckBoxCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXCheckBoxCell )


void SAL_CALL FmXCheckBoxCell::addItemListener( const Reference< ::com::sun::star::awt::XItemListener >& l ) throw( RuntimeException, std::exception )
{
    m_aItemListeners.addInterface( l );
}


void SAL_CALL FmXCheckBoxCell::removeItemListener( const Reference< ::com::sun::star::awt::XItemListener >& l ) throw( RuntimeException, std::exception )
{
    m_aItemListeners.removeInterface( l );
}


void SAL_CALL FmXCheckBoxCell::setState( short n ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        UpdateFromColumn();
        m_pBox->SetState( (TriState)n );
    }
}


short SAL_CALL FmXCheckBoxCell::getState() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        UpdateFromColumn();
        return (short)m_pBox->GetState();
    }
    return TRISTATE_INDET;
}


void SAL_CALL FmXCheckBoxCell::enableTriState( sal_Bool b ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->EnableTriState( b );
}


void SAL_CALL FmXCheckBoxCell::addActionListener( const Reference< awt::XActionListener >& _Listener ) throw (RuntimeException, std::exception)
{
    m_aActionListeners.addInterface( _Listener );
}


void SAL_CALL FmXCheckBoxCell::removeActionListener( const Reference< awt::XActionListener >& _Listener ) throw (RuntimeException, std::exception)
{
    m_aActionListeners.removeInterface( _Listener );
}


void SAL_CALL FmXCheckBoxCell::setLabel( const OUString& _Label ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pColumn )
    {
        DbGridControl& rGrid( m_pColumn->GetParent() );
        rGrid.SetColumnTitle( rGrid.GetColumnId( m_pColumn->GetFieldPos() ), _Label );
    }
}


void SAL_CALL FmXCheckBoxCell::setActionCommand( const OUString& _Command ) throw (RuntimeException, std::exception)
{
    m_aActionCommand = _Command;
}


vcl::Window* FmXCheckBoxCell::getEventWindow() const
{
    return m_pBox;
}


void FmXCheckBoxCell::onWindowEvent( const sal_uIntPtr _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{
    switch ( _nEventId )
    {
    case VCLEVENT_CHECKBOX_TOGGLE:
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
            aEvent.Highlighted = sal_False;
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

FmXListBoxCell::FmXListBoxCell(DbGridColumn* pColumn, DbCellControl& _rControl)
               :FmXTextCell( pColumn, _rControl )
               ,m_aItemListeners(m_aMutex)
               ,m_aActionListeners(m_aMutex)
               ,m_pBox( &static_cast< ListBox& >( _rControl.GetWindow() ) )
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
    ::com::sun::star::lang::EventObject aEvt(*this);
    m_aItemListeners.disposeAndClear(aEvt);
    m_aActionListeners.disposeAndClear(aEvt);

    m_pBox->SetSelectHdl( Link() );
    m_pBox->SetDoubleClickHdl( Link() );
    m_pBox = NULL;

    FmXTextCell::disposing();
}


Any SAL_CALL FmXListBoxCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException, std::exception)
{
    Any aReturn = FmXTextCell::queryAggregation(_rType);

    if ( !aReturn.hasValue() )
        aReturn = FmXListBoxCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXListBoxCell::getTypes(  ) throw(RuntimeException, std::exception)
{
    return ::comphelper::concatSequences(
        FmXTextCell::getTypes(),
        FmXListBoxCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXListBoxCell )


void SAL_CALL FmXListBoxCell::addItemListener(const Reference< ::com::sun::star::awt::XItemListener >& l) throw( RuntimeException, std::exception )
{
    m_aItemListeners.addInterface( l );
}


void SAL_CALL FmXListBoxCell::removeItemListener(const Reference< ::com::sun::star::awt::XItemListener >& l) throw( RuntimeException, std::exception )
{
    m_aItemListeners.removeInterface( l );
}


void SAL_CALL FmXListBoxCell::addActionListener(const Reference< ::com::sun::star::awt::XActionListener >& l) throw( RuntimeException, std::exception )
{
    m_aActionListeners.addInterface( l );
}


void SAL_CALL FmXListBoxCell::removeActionListener(const Reference< ::com::sun::star::awt::XActionListener >& l) throw( RuntimeException, std::exception )
{
    m_aActionListeners.removeInterface( l );
}


void SAL_CALL FmXListBoxCell::addItem(const OUString& aItem, sal_Int16 nPos) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
        m_pBox->InsertEntry( aItem, nPos );
}


void SAL_CALL FmXListBoxCell::addItems(const ::comphelper::StringSequence& aItems, sal_Int16 nPos) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
    {
        sal_uInt16 nP = nPos;
        for ( sal_uInt16 n = 0; n < aItems.getLength(); n++ )
        {
            m_pBox->InsertEntry( aItems.getConstArray()[n], nP );
            if ( nPos != -1 )    // Nicht wenn 0xFFFF, weil LIST_APPEND
                nP++;
        }
    }
}


void SAL_CALL FmXListBoxCell::removeItems(sal_Int16 nPos, sal_Int16 nCount) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            m_pBox->RemoveEntry( nPos + (--n) );
    }
}


sal_Int16 SAL_CALL FmXListBoxCell::getItemCount() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pBox ? m_pBox->GetEntryCount() : 0;
}


OUString SAL_CALL FmXListBoxCell::getItem(sal_Int16 nPos) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pBox ? OUString(m_pBox->GetEntry(nPos)) : OUString();
}

::comphelper::StringSequence SAL_CALL FmXListBoxCell::getItems() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::comphelper::StringSequence aSeq;
    if (m_pBox)
    {
        sal_uInt16 nEntries = m_pBox ->GetEntryCount();
        aSeq = ::comphelper::StringSequence( nEntries );
        for ( sal_uInt16 n = nEntries; n; )
        {
            --n;
            aSeq.getArray()[n] = m_pBox ->GetEntry( n );
        }
    }
    return aSeq;
}


sal_Int16 SAL_CALL FmXListBoxCell::getSelectedItemPos() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
    {
        UpdateFromColumn();
        sal_Int32 nPos = m_pBox->GetSelectEntryPos();
        if (nPos > SHRT_MAX || nPos < SHRT_MIN)
            throw std::out_of_range("awt::XListBox::getSelectedItemPos can only return a short");
        return nPos;
    }
    return 0;
}


Sequence< sal_Int16 > SAL_CALL FmXListBoxCell::getSelectedItemsPos() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Sequence<sal_Int16> aSeq;

    if (m_pBox)
    {
        UpdateFromColumn();
        sal_uInt16 nSelEntries = m_pBox->GetSelectEntryCount();
        aSeq = Sequence<sal_Int16>( nSelEntries );
        for ( sal_uInt16 n = 0; n < nSelEntries; n++ )
            aSeq.getArray()[n] = m_pBox->GetSelectEntryPos( n );
    }
    return aSeq;
}

OUString SAL_CALL FmXListBoxCell::getSelectedItem() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aItem;

    if (m_pBox)
    {
        UpdateFromColumn();
        aItem = m_pBox->GetSelectEntry();
    }

    return aItem;
}


::comphelper::StringSequence SAL_CALL FmXListBoxCell::getSelectedItems() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::comphelper::StringSequence aSeq;

    if (m_pBox)
    {
        UpdateFromColumn();
        sal_uInt16 nSelEntries = m_pBox->GetSelectEntryCount();
        aSeq = ::comphelper::StringSequence( nSelEntries );
        for ( sal_uInt16 n = 0; n < nSelEntries; n++ )
            aSeq.getArray()[n] = m_pBox->GetSelectEntry( n );
    }
    return aSeq;
}


void SAL_CALL FmXListBoxCell::selectItemPos(sal_Int16 nPos, sal_Bool bSelect) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SelectEntryPos( nPos, bSelect );
}


void SAL_CALL FmXListBoxCell::selectItemsPos(const Sequence< sal_Int16 >& aPositions, sal_Bool bSelect) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        for ( sal_uInt16 n = (sal_uInt16)aPositions.getLength(); n; )
            m_pBox->SelectEntryPos( (sal_uInt16) aPositions.getConstArray()[--n], bSelect );
    }
}


void SAL_CALL FmXListBoxCell::selectItem(const OUString& aItem, sal_Bool bSelect) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SelectEntry( aItem, bSelect );
}


sal_Bool SAL_CALL FmXListBoxCell::isMutipleMode() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bMulti = false;
    if (m_pBox)
        bMulti = m_pBox->IsMultiSelectionEnabled();
    return bMulti;
}


void SAL_CALL FmXListBoxCell::setMultipleMode(sal_Bool bMulti) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->EnableMultiSelection( bMulti );
}


sal_Int16 SAL_CALL FmXListBoxCell::getDropDownLineCount() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int16 nLines = 0;
    if (m_pBox)
        nLines = m_pBox->GetDropDownLineCount();

    return nLines;
}


void SAL_CALL FmXListBoxCell::setDropDownLineCount(sal_Int16 nLines) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SetDropDownLineCount( nLines );
}


void SAL_CALL FmXListBoxCell::makeVisible(sal_Int16 nEntry) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SetTopEntry( nEntry );
}


void FmXListBoxCell::onWindowEvent( const sal_uIntPtr _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{
    if  (   ( &_rWindow == m_pBox )
        &&  ( _nEventId == VCLEVENT_LISTBOX_SELECT )
        )
    {
        OnDoubleClick( NULL );

        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = *this;
        aEvent.Highlighted = sal_False;

        // Bei Mehrfachselektion 0xFFFF, sonst die ID
        aEvent.Selected = (m_pBox->GetSelectEntryCount() == 1 )
            ? m_pBox->GetSelectEntryPos() : 0xFFFF;

        m_aItemListeners.notifyEach( &awt::XItemListener::itemStateChanged, aEvent );
        return;
    }

    FmXTextCell::onWindowEvent( _nEventId, _rWindow, _pEventData );
}



IMPL_LINK_NOARG(FmXListBoxCell, OnDoubleClick)
{
    if (m_pBox)
    {
        ::cppu::OInterfaceIteratorHelper aIt( m_aActionListeners );

        ::com::sun::star::awt::ActionEvent aEvent;
        aEvent.Source = *this;
        aEvent.ActionCommand = m_pBox->GetSelectEntry();

        while( aIt.hasMoreElements() )
            static_cast< ::com::sun::star::awt::XActionListener *>(aIt.next())->actionPerformed( aEvent );
    }
    return 1;
}

FmXComboBoxCell::FmXComboBoxCell( DbGridColumn* pColumn, DbCellControl& _rControl )
    :FmXTextCell( pColumn, _rControl )
    ,m_aItemListeners( m_aMutex )
    ,m_aActionListeners( m_aMutex )
    ,m_pComboBox( &static_cast< ComboBox& >( _rControl.GetWindow() ) )
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
    ::com::sun::star::lang::EventObject aEvt(*this);
    m_aItemListeners.disposeAndClear(aEvt);
    m_aActionListeners.disposeAndClear(aEvt);

    FmXTextCell::disposing();
}


Any SAL_CALL FmXComboBoxCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException, std::exception)
{
    Any aReturn = FmXTextCell::queryAggregation(_rType);

    if ( !aReturn.hasValue() )
        aReturn = FmXComboBoxCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< Type > SAL_CALL FmXComboBoxCell::getTypes(  ) throw(RuntimeException, std::exception)
{
    return ::comphelper::concatSequences(
        FmXTextCell::getTypes(),
        FmXComboBoxCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXComboBoxCell )


void SAL_CALL FmXComboBoxCell::addItemListener(const Reference< awt::XItemListener >& l) throw( RuntimeException, std::exception )
{
    m_aItemListeners.addInterface( l );
}


void SAL_CALL FmXComboBoxCell::removeItemListener(const Reference< awt::XItemListener >& l) throw( RuntimeException, std::exception )
{
    m_aItemListeners.removeInterface( l );
}


void SAL_CALL FmXComboBoxCell::addActionListener(const Reference< awt::XActionListener >& l) throw( RuntimeException, std::exception )
{
    m_aActionListeners.addInterface( l );
}


void SAL_CALL FmXComboBoxCell::removeActionListener(const Reference< awt::XActionListener >& l) throw( RuntimeException, std::exception )
{
    m_aActionListeners.removeInterface( l );
}


void SAL_CALL FmXComboBoxCell::addItem( const OUString& _Item, sal_Int16 _Pos ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pComboBox )
        m_pComboBox->InsertEntry( _Item, _Pos );
}


void SAL_CALL FmXComboBoxCell::addItems( const Sequence< OUString >& _Items, sal_Int16 _Pos ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pComboBox )
    {
        sal_uInt16 nP = _Pos;
        for ( sal_uInt16 n = 0; n < _Items.getLength(); n++ )
        {
            m_pComboBox->InsertEntry( _Items.getConstArray()[n], nP );
            if ( _Pos != -1 )
                nP++;
        }
    }
}


void SAL_CALL FmXComboBoxCell::removeItems( sal_Int16 _Pos, sal_Int16 _Count ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pComboBox )
    {
        for ( sal_uInt16 n = _Count; n; )
            m_pComboBox->RemoveEntryAt( _Pos + (--n) );
    }
}


sal_Int16 SAL_CALL FmXComboBoxCell::getItemCount() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pComboBox ? m_pComboBox->GetEntryCount() : 0;
}


OUString SAL_CALL FmXComboBoxCell::getItem( sal_Int16 _Pos ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pComboBox ? OUString(m_pComboBox->GetEntry(_Pos)) : OUString();
}

Sequence< OUString > SAL_CALL FmXComboBoxCell::getItems() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Sequence< OUString > aItems;
    if ( m_pComboBox )
    {
        sal_uInt16 nEntries = m_pComboBox->GetEntryCount();
        aItems.realloc( nEntries );
        OUString* pItem = aItems.getArray();
        for ( sal_uInt16 n=0; n<nEntries; ++n, ++pItem )
            *pItem = m_pComboBox->GetEntry( n );
    }
    return aItems;
}


sal_Int16 SAL_CALL FmXComboBoxCell::getDropDownLineCount() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int16 nLines = 0;
    if ( m_pComboBox )
        nLines = m_pComboBox->GetDropDownLineCount();

    return nLines;
}


void SAL_CALL FmXComboBoxCell::setDropDownLineCount(sal_Int16 nLines) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pComboBox )
        m_pComboBox->SetDropDownLineCount( nLines );
}


void FmXComboBoxCell::onWindowEvent( const sal_uIntPtr _nEventId, const vcl::Window& _rWindow, const void* _pEventData )
{

    switch ( _nEventId )
    {
    case VCLEVENT_COMBOBOX_SELECT:
    {
        awt::ItemEvent aEvent;
        aEvent.Source = *this;
        aEvent.Highlighted = sal_False;

        // Bei Mehrfachselektion 0xFFFF, sonst die ID
        aEvent.Selected =   ( m_pComboBox->GetSelectEntryCount() == 1 )
                        ?   m_pComboBox->GetSelectEntryPos()
                        :   0xFFFF;
        m_aItemListeners.notifyEach( &awt::XItemListener::itemStateChanged, aEvent );
    }
    break;

    default:
        FmXTextCell::onWindowEvent( _nEventId, _rWindow, _pEventData );
        break;
    }
}

TYPEINIT1(FmXFilterCell, FmXGridCell);

FmXFilterCell::FmXFilterCell(DbGridColumn* pColumn, DbCellControl* pControl )
              :FmXGridCell( pColumn, pControl )
              ,m_aTextListeners(m_aMutex)
{

    DBG_ASSERT( m_pCellControl->ISA( DbFilterField ), "FmXFilterCell::FmXFilterCell: invalid cell control!" );
    static_cast< DbFilterField* >( m_pCellControl )->SetCommitHdl( LINK( this, FmXFilterCell, OnCommit ) );
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

sal_Int64 SAL_CALL FmXFilterCell::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw(RuntimeException, std::exception)
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


void FmXFilterCell::PaintCell( OutputDevice& rDev, const Rectangle& rRect )
{
    static_cast< DbFilterField* >( m_pCellControl )->PaintCell( rDev, rRect );
}

// OComponentHelper

void FmXFilterCell::disposing()
{
    ::com::sun::star::lang::EventObject aEvt(*this);
    m_aTextListeners.disposeAndClear(aEvt);

    static_cast<DbFilterField*>(m_pCellControl)->SetCommitHdl(Link());

    FmXGridCell::disposing();
}


Any SAL_CALL FmXFilterCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException, std::exception)
{
    Any aReturn = FmXGridCell::queryAggregation(_rType);

    if ( !aReturn.hasValue() )
        aReturn = FmXFilterCell_Base::queryInterface( _rType );

    return aReturn;
}


Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXFilterCell::getTypes(  ) throw(RuntimeException, std::exception)
{
    return ::comphelper::concatSequences(
        FmXGridCell::getTypes(),
        FmXFilterCell_Base::getTypes()
    );
}


IMPLEMENT_GET_IMPLEMENTATION_ID( FmXFilterCell )

// ::com::sun::star::awt::XTextComponent

void SAL_CALL FmXFilterCell::addTextListener(const Reference< ::com::sun::star::awt::XTextListener >& l) throw( RuntimeException, std::exception )
{
    m_aTextListeners.addInterface( l );
}


void SAL_CALL FmXFilterCell::removeTextListener(const Reference< ::com::sun::star::awt::XTextListener >& l) throw( RuntimeException, std::exception )
{
    m_aTextListeners.removeInterface( l );
}


void SAL_CALL FmXFilterCell::setText( const OUString& aText ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    static_cast<DbFilterField*>(m_pCellControl)->SetText(aText);
}


void SAL_CALL FmXFilterCell::insertText( const ::com::sun::star::awt::Selection& /*rSel*/, const OUString& /*aText*/ ) throw( RuntimeException, std::exception )
{
}


OUString SAL_CALL FmXFilterCell::getText() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return static_cast<DbFilterField*>(m_pCellControl)->GetText();
}


OUString SAL_CALL FmXFilterCell::getSelectedText( void ) throw( RuntimeException, std::exception )
{
    return getText();
}


void SAL_CALL FmXFilterCell::setSelection( const ::com::sun::star::awt::Selection& /*aSelection*/ ) throw( RuntimeException, std::exception )
{
}


::com::sun::star::awt::Selection SAL_CALL FmXFilterCell::getSelection( void ) throw( RuntimeException, std::exception )
{
    return ::com::sun::star::awt::Selection();
}


sal_Bool SAL_CALL FmXFilterCell::isEditable( void ) throw( RuntimeException, std::exception )
{
    return sal_True;
}


void SAL_CALL FmXFilterCell::setEditable( sal_Bool /*bEditable*/ ) throw( RuntimeException, std::exception )
{
}


sal_Int16 SAL_CALL FmXFilterCell::getMaxTextLen() throw( RuntimeException, std::exception )
{
    return 0;
}


void SAL_CALL FmXFilterCell::setMaxTextLen( sal_Int16 /*nLen*/ ) throw( RuntimeException, std::exception )
{
}


IMPL_LINK_NOARG(FmXFilterCell, OnCommit)
{
    ::cppu::OInterfaceIteratorHelper aIt( m_aTextListeners );
    ::com::sun::star::awt::TextEvent aEvt;
    aEvt.Source = *this;
    while( aIt.hasMoreElements() )
        static_cast< ::com::sun::star::awt::XTextListener *>(aIt.next())->textChanged( aEvt );
    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
