/*************************************************************************
 *
 *  $RCSfile: gridcell.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:43:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_GRIDCELL_HXX
#include "gridcell.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMPROP_HXX
#include "fmprop.hxx"
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_GRIDCOLS_HXX
#include "gridcols.hxx"
#endif
#ifndef SVX_FORM_SDBDATACOLUMN_HXX
#include "sdbdatacolumn.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_VISUALEFFECT_HPP_
#include <com/sun/star/awt/VisualEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XSTATEMENT_HPP_
#include <com/sun/star/sdbc/XStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDDB_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCOMPONENT_HPP_
#include <com/sun/star/form/XBoundComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_LINEENDFORMAT_HPP_
#include <com/sun/star/awt/LineEndFormat.hpp>
#endif

#ifndef _FMTFIELD_HXX_
#include <svtools/fmtfield.hxx>
#endif
#ifndef _NUMUNO_HXX //autogen
#include <svtools/numuno.hxx>
#endif
#ifndef _CALENDAR_HXX //autogen wg. CalendarField
#include <svtools/calendar.hxx>
#endif
#ifndef _LONGCURR_HXX //autogen
#include <vcl/longcurr.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif

#include <math.h>

#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

using namespace ::connectivity;
using namespace ::connectivity::simple;
using namespace ::svxform;
using namespace ::comphelper;
using namespace ::svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;

//==================================================================
//= helper
//==================================================================
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
                    OSL_ENSURE( sal_False, "getModelLineEndSetting: what's this?" );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "getModelLineEndSetting: caught an exception!" );
        }
        return eFormat;
    }
}

//==================================================================
//= DbGridColumn
//==================================================================
//------------------------------------------------------------------------------
CellControllerRef DbGridColumn::s_xEmptyController;

//------------------------------------------------------------------------------
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
                m_bDateTime = sal_True;

            case DataType::BIT:
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::REAL:
            case DataType::BIGINT:
            case DataType::DOUBLE:
            case DataType::NUMERIC:
            case DataType::DECIMAL:
                m_nAlign = ::com::sun::star::awt::TextAlign::RIGHT;
                m_bNumeric = sal_True;
                break;
            default:
                m_nAlign = ::com::sun::star::awt::TextAlign::LEFT;
        }
    }

    DbCellControl* pCellControl = NULL;
    if (m_rParent.IsFilterMode())
    {
        pCellControl = new DbFilterField(m_rParent.getServiceManager(),*this);
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
            case TYPE_PATTERNFIELD: pCellControl = new DbPatternField(*this); break;
            case TYPE_TEXTFIELD: pCellControl = new DbTextField(*this); break;
            case TYPE_TIMEFIELD: pCellControl = new DbTimeField(*this); break;
            case TYPE_FORMATTEDFIELD: pCellControl = new DbFormattedField(*this); break;
            default:
                DBG_ERROR("DbGridColumn::CreateControl: Unknown Column");
                return;
        }

    }
    Reference< XRowSet >  xCur;
    if (m_rParent.getDataSource())
        xCur = Reference< XRowSet > ((Reference< XInterface >)*m_rParent.getDataSource(), UNO_QUERY);
        // TODO : the cursor wrapper should use an XRowSet interface, too

    pCellControl->Init(&m_rParent.GetDataWindow(), xCur );

    // now create the control wrapper
    if (m_rParent.IsFilterMode())
        m_pCell = new FmXFilterCell(this, pCellControl);
    else
    {
        switch (nTypeId)
        {
            case TYPE_CHECKBOX: m_pCell = new FmXCheckBoxCell(this, pCellControl);  break;
            case TYPE_LISTBOX: m_pCell = new FmXListBoxCell(this, pCellControl);    break;
            default:
                m_pCell = new FmXEditCell(this, pCellControl);
        }
    }
    m_pCell->acquire();

    // only if we use have a bound field, we use a a controller for displaying the
    // window in the grid
    if (m_xField.is())
        m_xController = pCellControl->CreateController();
}

//------------------------------------------------------------------------------
void DbGridColumn::UpdateFromField(const DbGridRow* pRow, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (m_pCell && m_pCell->ISA(FmXFilterCell))
        PTR_CAST(FmXFilterCell, m_pCell)->Update();
    else if (pRow && pRow->IsValid() && m_nFieldPos >= 0 && m_pCell && pRow->HasField(m_nFieldPos))
    {
        PTR_CAST(FmXDataCell, m_pCell)->UpdateFromField( pRow->GetField( m_nFieldPos ).getColumn(), xFormatter );
    }
}

//------------------------------------------------------------------------------
sal_Bool DbGridColumn::Commit()
{
    sal_Bool bResult = sal_True;
    if (!m_bInSave && m_pCell)
    {
        m_bInSave = sal_True;
        bResult = m_pCell->Commit();

        // store the data into the model
        FmXDataCell* pDataCell = PTR_CAST(FmXDataCell, m_pCell);
        if (bResult && pDataCell)
        {
            Reference< ::com::sun::star::form::XBoundComponent >  xComp(m_xModel, UNO_QUERY);
            if (xComp.is())
                bResult = xComp->commit();
        }
        m_bInSave = sal_False;
    }
    return bResult;
}

//------------------------------------------------------------------------------
DbGridColumn::~DbGridColumn()
{
    Clear();
}

//------------------------------------------------------------------------------
void DbGridColumn::Clear()
{
    m_xController = NULL;
    m_xField = NULL;

    m_nFormatKey = 0;
    m_nFieldPos = -1;
    m_bReadOnly = sal_True;
    m_bAutoValue = sal_False;
    m_nFieldType = DataType::OTHER;

    if (m_pCell)
    {
        m_pCell->dispose();
        m_pCell->release();
        m_pCell = NULL;
    }
}

//------------------------------------------------------------------------------
sal_Int16 DbGridColumn::SetAlignment(sal_Int16 _nAlign)
{
    if (_nAlign == -1)
    {   // 'Standard'
        if (m_xField.is())
        {
            sal_Int32 nType;
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

//------------------------------------------------------------------------------
sal_Int16 DbGridColumn::SetAlignmentFromModel(sal_Int16 nStandardAlign)
{
    Any aAlign( m_xModel->getPropertyValue(FM_PROP_ALIGN));
    if (aAlign.hasValue())
    {
        sal_Int16 nTest;
        if (aAlign >>= nTest)
            nStandardAlign = nTest;
    }
    return SetAlignment(nStandardAlign);
}

//------------------------------------------------------------------------------
void DbGridColumn::setLock(sal_Bool _bLock)
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

//------------------------------------------------------------------------------
String DbGridColumn::GetCellText(const DbGridRow* pRow, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const
{
    String aText;
    if (m_pCell && m_pCell->ISA(FmXFilterCell))
        return aText;

    if (!pRow || !pRow->IsValid())
        aText  = INVALIDTEXT;
    else if (pRow->HasField(m_nFieldPos))
    {
        aText = GetCellText( pRow->GetField( m_nFieldPos ).getColumn(), xFormatter );
    }
    return aText;
}

//------------------------------------------------------------------------------
String DbGridColumn::GetCellText(const Reference< ::com::sun::star::sdb::XColumn >& xField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const
{
    String aText;
    if (xField.is())
    {
        FmXTextCell* pTextCell = PTR_CAST(FmXTextCell, m_pCell);
        if (pTextCell)
            aText = pTextCell->GetText(xField, xFormatter);
        else if (m_bObject)
            aText = OBJECTTEXT;
    }
    return aText;
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void DbGridColumn::Paint(OutputDevice& rDev,
                         const Rectangle& rRect,
                         const DbGridRow* pRow,
                         const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    FmXDataCell* pDataCell = PTR_CAST(FmXDataCell, m_pCell);
    if (pDataCell)
    {
        if (!pRow || !pRow->IsValid())
        {
            sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_CENTER;
            if (!((Window&)rDev).IsEnabled())
                nStyle |= TEXT_DRAW_DISABLE;

            rDev.DrawText(rRect, INVALIDTEXT, nStyle);
        }
        else if (m_bAutoValue && pRow->IsNew())
        {
            static String aAutoText(SVX_RES(RID_STR_AUTOFIELD));
            sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_VCENTER;
            if (!((Window&)rDev).IsEnabled())
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

            rDev.DrawText(rRect, aAutoText , nStyle);
        }
        else if (pRow->HasField(m_nFieldPos))
        {
            pDataCell->Paint(rDev, rRect, pRow->GetField( m_nFieldPos ).getColumn(), xFormatter);
        }
    }
    else if (!m_pCell)
    {
        if (!pRow || !pRow->IsValid())
        {
            sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_CENTER;
            if (!((Window&)rDev).IsEnabled())
                nStyle |= TEXT_DRAW_DISABLE;

            rDev.DrawText(rRect, INVALIDTEXT, nStyle);
        }
        else if (pRow->HasField(m_nFieldPos) && m_bObject)
        {
            sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_CENTER;
            if (!((Window&)rDev).IsEnabled())
                nStyle |= TEXT_DRAW_DISABLE;
            rDev.DrawText(rRect, OBJECTTEXT, nStyle);
        }
    }
    else if ( m_pCell->ISA( FmXFilterCell ) )
        static_cast< FmXFilterCell* >( m_pCell )->Paint( rDev, rRect );
}

//------------------------------------------------------------------------------
void DbGridColumn::ImplInitSettings(Window* pParent, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground)
{
    if (m_pCell)
        m_pCell->ImplInitSettings(pParent, bFont, bForeground, bBackground);
}

//==============================================================================
//= cell controls
//==============================================================================
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

//------------------------------------------------------------------------------
DbCellControl::DbCellControl( DbGridColumn& _rColumn, sal_Bool _bText )
    :OPropertyChangeListener(m_aMutex)
    ,m_rColumn( _rColumn )
    ,m_pWindow( NULL )
    ,m_pPainter( NULL )
    ,m_bTransparent( sal_False )
    ,m_bAlignedController( sal_True )
    ,m_bAccessingValueProperty( sal_False )
{
    Reference< XPropertySet > xColModelProps( _rColumn.getModel(), UNO_QUERY );
    if ( xColModelProps.is() )
    {
        // if our model's format key changes we want to propagate the new value to our windows
        m_pModelChangeBroadcaster = new ::comphelper::OPropertyChangeMultiplexer(this, Reference< ::com::sun::star::beans::XPropertySet > (_rColumn.getModel(), UNO_QUERY));
        m_pModelChangeBroadcaster->acquire();

        // be listener for some common properties
        implDoPropertyListening( FM_PROP_READONLY, sal_False );
        implDoPropertyListening( FM_PROP_ENABLED, sal_False );

        // add as listener for all know "value" properties
        implDoPropertyListening( FM_PROP_VALUE, sal_False );
        implDoPropertyListening( FM_PROP_STATE, sal_False );
        implDoPropertyListening( FM_PROP_TEXT, sal_False );
        implDoPropertyListening( FM_PROP_EFFECTIVE_VALUE, sal_False );
    }
}

//------------------------------------------------------------------------------
void DbCellControl::implDoPropertyListening( const ::rtl::OUString& _rPropertyName, sal_Bool _bWarnIfNotExistent )
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
        DBG_ERROR( "DbCellControl::doPropertyListening: caught an exception!" );
    }
}

//------------------------------------------------------------------------------
void DbCellControl::doPropertyListening( const ::rtl::OUString& _rPropertyName )
{
    implDoPropertyListening( _rPropertyName );
}

//------------------------------------------------------------------------------
DbCellControl::~DbCellControl()
{
    if ( m_pModelChangeBroadcaster )
    {
        m_pModelChangeBroadcaster->dispose();
        m_pModelChangeBroadcaster->release();
        m_pModelChangeBroadcaster = NULL;
        // no delete, this is done implicitly
    }

    delete m_pWindow;
    delete m_pPainter;
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void DbCellControl::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    // nothing to to here
}

//------------------------------------------------------------------------------
void DbCellControl::_propertyChanged(const PropertyChangeEvent& _rEvent) throw(RuntimeException)
{
    Reference< XPropertySet > xSourceProps( _rEvent.Source, UNO_QUERY );

    if  (   _rEvent.PropertyName.equalsAscii( FM_PROP_VALUE )
        ||  _rEvent.PropertyName.equalsAscii( FM_PROP_STATE )
        ||  _rEvent.PropertyName.equalsAscii( FM_PROP_TEXT )
        ||  _rEvent.PropertyName.equalsAscii( FM_PROP_EFFECTIVE_VALUE )
        )
    {   // it was one of the known "value" properties
        if ( !isValuePropertyLocked() )
        {
            implValuePropertyChanged( );
        }
    }
    else if ( _rEvent.PropertyName.equalsAscii( FM_PROP_READONLY ) )
    {
        implAdjustReadOnly( xSourceProps );
    }
    else if ( _rEvent.PropertyName.equalsAscii( FM_PROP_ENABLED ) )
    {
        implAdjustEnabled( xSourceProps );
    }
    else
        implAdjustGenericFieldSetting( xSourceProps );
}

//------------------------------------------------------------------------------
sal_Bool DbCellControl::Commit()
{
    // lock the listening for value property changes
    lockValueProperty();
    // commit the content of the control into the model's value property
    sal_Bool bReturn = commitControl();
    // unlock the listening for value property changes
    unlockValueProperty();
    // outta here
    return bReturn;
}

//------------------------------------------------------------------------------
void DbCellControl::ImplInitSettings(Window* pParent, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground)
{
    Window* pWindows[] = { m_pPainter,m_pWindow};
    if (bFont)
    {
        Font aFont( pParent->IsControlFont() ? pParent->GetControlFont() : pParent->GetPointFont());
        aFont.SetTransparent( isTransparent() );

        for (sal_Int32 i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
        {
            if ( pWindows[i] )
            {
                pWindows[i]->SetZoomedPointFont(aFont);
                pWindows[i]->SetZoom(pParent->GetZoom());
            }
        }
    }

    if (bFont || bForeground)
    {
        Color aTextColor( pParent->IsControlForeground() ? pParent->GetControlForeground() : pParent->GetTextColor() );

        sal_Bool bTextLineColor = pParent->IsTextLineColor();
        Color aTextLineColor( pParent->GetTextLineColor() );

        for (sal_Int32 i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
        {
            if ( pWindows[i] )
            {
                pWindows[i]->SetTextColor(aTextColor);
                if (pParent->IsControlForeground())
                    pWindows[i]->SetControlForeground(aTextColor);

                if (bTextLineColor)
                    pWindows[i]->SetTextLineColor();
                else
                    pWindows[i]->SetTextLineColor(aTextLineColor);
            }
        }
    }

    if (bBackground)
    {
        if (pParent->IsControlBackground())
        {
            Color aColor( pParent->GetControlBackground());
            for (sal_Int32 i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
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
                    m_pPainter->SetBackground(pParent->GetBackground());
                m_pPainter->SetFillColor(pParent->GetFillColor());
            }

            if (m_pWindow)
            {
                if ( isTransparent() )
                    m_pWindow->SetBackground(pParent->GetBackground());
                else
                    m_pWindow->SetFillColor(pParent->GetFillColor());
            }
        }
    }
}

//------------------------------------------------------------------------------
void DbCellControl::implAdjustReadOnly( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbCellControl::implAdjustReadOnly: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbCellControl::implAdjustReadOnly: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        WindowType eType = m_pWindow->GetType();
        if  (   ( WINDOW_EDIT == eType )
            ||  ( WINDOW_COMBOBOX == eType )
            ||  ( WINDOW_DATEFIELD == eType )
            ||  ( WINDOW_TIMEFIELD == eType )
            ||  ( WINDOW_NUMERICFIELD == eType )
            ||  ( WINDOW_PATTERNFIELD == eType )
            ||  ( WINDOW_CURRENCYFIELD == eType )
            )
        {
            sal_Bool bReadOnly = sal_True;
            _rxModel->getPropertyValue( FM_PROP_READONLY ) >>= bReadOnly;
            static_cast< Edit* >( m_pWindow )->SetReadOnly( m_rColumn.IsReadOnly() || bReadOnly );
        }
    }
}

//------------------------------------------------------------------------------
void DbCellControl::implAdjustEnabled( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbCellControl::implAdjustEnabled: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbCellControl::implAdjustEnabled: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Bool bEnable = sal_True;
        _rxModel->getPropertyValue( FM_PROP_ENABLED ) >>= bEnable;
        m_pWindow->Enable( bEnable );
    }
}

//------------------------------------------------------------------------------
void DbCellControl::Init(Window* pParent, const Reference< XRowSet >& xCursor)
{
    ImplInitSettings(pParent, sal_True ,sal_True, sal_True);
    if ( m_pWindow )
    {
        // align the control
        if ( isAlignedController() )
            AlignControl( m_rColumn.GetAlignment() );

        try
        {
            // some other common properties
            Reference< XPropertySet > xModel( m_rColumn.getModel() );
            Reference< XPropertySetInfo > xModelPSI;
            if ( xModel.is() )
                xModelPSI = xModel->getPropertySetInfo();

            // the "readonly" state
            if ( xModelPSI.is() && xModelPSI->hasPropertyByName( FM_PROP_READONLY ) )
            {
                implAdjustReadOnly( xModel );
            }

            // the "enabled" flag
            if ( xModelPSI.is() && xModelPSI->hasPropertyByName( FM_PROP_ENABLED ) )
            {
                implAdjustEnabled( xModel );
            }
        }
        catch( const Exception& e )
        {
            e;  // make compiler happy
            DBG_ERROR( "DbCellControl::Init: caught an exception!" );
        }
    }
}

//------------------------------------------------------------------------------
void DbCellControl::SetTextLineColor()
{
    if (m_pWindow)
        m_pWindow->SetTextLineColor();
    if (m_pPainter)
        m_pPainter->SetTextLineColor();
}

//------------------------------------------------------------------------------
void DbCellControl::SetTextLineColor(const Color& _rColor)
{
    if (m_pWindow)
        m_pWindow->SetTextLineColor(_rColor);
    if (m_pPainter)
        m_pPainter->SetTextLineColor(_rColor);
}

namespace
{
    static void lcl_implAlign( Window* _pWindow, WinBits _nAlignmentBit )
    {
        WinBits nStyle = _pWindow->GetStyle();
        nStyle &= ~(WB_LEFT | WB_RIGHT | WB_CENTER);
        _pWindow->SetStyle( nStyle | _nAlignmentBit );
    }
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void DbCellControl::Paint( OutputDevice& _rDev, const Rectangle& _rRect )
{
    if ( m_pPainter->GetParent() == &_rDev )
    {
        m_pPainter->SetPaintTransparent( sal_True );
        m_pPainter->SetBackground( );
        m_pPainter->SetControlBackground( _rDev.GetFillColor() );
        m_pPainter->SetControlForeground( _rDev.GetTextColor() );
        m_pPainter->SetTextColor( _rDev.GetTextColor() );
        m_pPainter->SetTextFillColor( _rDev.GetTextColor() );

        Font aFont( _rDev.GetFont() );
        aFont.SetTransparent( TRUE );
        m_pPainter->SetFont( aFont );

        m_pPainter->SetPosSizePixel( _rRect.TopLeft(), _rRect.GetSize() );
        m_pPainter->Show();
        m_pPainter->Update();
        m_pPainter->SetParentUpdateMode( sal_False );
        m_pPainter->Hide();
        m_pPainter->SetParentUpdateMode( sal_True );
    }
    else
        m_pPainter->Draw( &_rDev, _rRect.TopLeft(), _rRect.GetSize(), 0 );
}

//------------------------------------------------------------------------------
void DbCellControl::Paint( OutputDevice& _rDev, const Rectangle& _rRect, const Reference< XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter )
{
    m_pPainter->SetText( GetFormatText( _rxField, _rxFormatter ) );
    Paint( _rDev, _rRect );
}

//------------------------------------------------------------------------------
double DbCellControl::GetValue(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const
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
        sal_Bool bSuccess = sal_False;
        try
        {
            fValue = _rxField->getDouble();
            bSuccess = sal_True;
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

//------------------------------------------------------------------------------
void DbCellControl::invalidatedController()
{
    m_rColumn.GetParent().refreshController(m_rColumn.GetId(), DbGridControl::GrantCellControlAccess());
}

/*************************************************************************/
// CellModels
/*************************************************************************/

//==============================================================================
//= DbLimitedLengthField
//==============================================================================
//------------------------------------------------------------------------------
DbLimitedLengthField::DbLimitedLengthField( DbGridColumn& _rColumn )
    :DbCellControl( _rColumn )
{
    doPropertyListening( FM_PROP_MAXTEXTLEN );
}

//------------------------------------------------------------------------------
void DbLimitedLengthField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbLimitedLengthField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbLimitedLengthField::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Int16 nMaxLen = 0;
        _rxModel->getPropertyValue( FM_PROP_MAXTEXTLEN ) >>= nMaxLen;
        static_cast< Edit* >( m_pWindow )->SetMaxTextLen( nMaxLen );

        if ( m_pPainter )
            static_cast< Edit* >( m_pPainter )->SetMaxTextLen( nMaxLen );
    }
}

//==============================================================================
//= DbTextField
//==============================================================================
//------------------------------------------------------------------------------
DbTextField::DbTextField(DbGridColumn& _rColumn)
            :DbLimitedLengthField(_rColumn)
            ,m_nKeyType(::com::sun::star::util::NumberFormat::TEXT)
            ,m_pEdit( NULL )
{
}

//------------------------------------------------------------------------------
DbTextField::~DbTextField( )
{
    DELETEZ( m_pPainterImplementation );
    DELETEZ( m_pEdit );
}

//------------------------------------------------------------------------------
void DbTextField::Init(Window* pParent, const Reference< XRowSet >& xCursor)
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
    sal_Bool bIsMultiLine = sal_False;
    try
    {
        if ( xModel.is() )
        {
            OSL_VERIFY( xModel->getPropertyValue( FM_PROP_MULTILINE ) >>= bIsMultiLine );
        }
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "DbTextField::Init: caught an exception while determining the multi-line capabilities!" );
    }

    if ( bIsMultiLine )
    {
        m_pWindow = new MultiLineTextCell( pParent, nStyle );
        m_pEdit = new MultiLineEditImplementation( *static_cast< MultiLineTextCell* >( m_pWindow ) );

        m_pPainter = new MultiLineTextCell( pParent, nStyle );
        m_pPainterImplementation = new MultiLineEditImplementation( *static_cast< MultiLineTextCell* >( m_pPainter ) );
    }
    else
    {
        m_pWindow = new Edit( pParent, nStyle );
        m_pEdit = new EditImplementation( *static_cast< Edit* >( m_pWindow ) );

        m_pPainter = new Edit( pParent, nStyle );
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

    if (m_rColumn.GetParent().getNumberFormatter().is() && m_rColumn.GetKey())
        m_nKeyType  = comphelper::getNumberFormatType(m_rColumn.GetParent().getNumberFormatter()->getNumberFormatsSupplier()->getNumberFormats(), m_rColumn.GetKey());

    DbLimitedLengthField::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
CellControllerRef DbTextField::CreateController() const
{
    return new EditCellController( m_pEdit );
}

//------------------------------------------------------------------------------
void DbTextField::Paint( OutputDevice& _rDev, const Rectangle& _rRect, const Reference< XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter )
{
    if ( m_pPainterImplementation )
        m_pPainterImplementation->SetText( GetFormatText( _rxField, _rxFormatter, NULL ) );

    if ( m_pPainter->GetParent() != &_rDev )
        m_pPainter->Draw( &_rDev, _rRect.TopLeft(), _rRect.GetSize(), 0 );
    else
        DbLimitedLengthField::Paint( _rDev, _rRect, _rxField, _rxFormatter );
}

//------------------------------------------------------------------------------
String DbTextField::GetFormatText(const Reference< XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    ::rtl::OUString aString;
    if ( _rxField.is() )
        aString = getValue( _rxField, xFormatter, m_rColumn.GetParent().getNullDate(), m_rColumn.GetKey(), m_nKeyType);

    return aString;
}

//------------------------------------------------------------------------------
void DbTextField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    m_pEdit->SetText( GetFormatText( _rxField, xFormatter ) );
    m_pEdit->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}

//------------------------------------------------------------------------------
void DbTextField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbTextField::updateFromModel: invalid call!" );

    ::rtl::OUString sText;
    _rxModel->getPropertyValue( FM_PROP_TEXT ) >>= sText;

    xub_StrLen nMaxTextLen = m_pEdit->GetMaxTextLen();
    if ( EDIT_NOLIMIT != nMaxTextLen && sText.getLength() > nMaxTextLen )
    {
        sal_Int32 nDiff = sText.getLength() - nMaxTextLen;
        sText = sText.replaceAt(sText.getLength() - nDiff,nDiff,::rtl::OUString());
    }


    m_pEdit->SetText( sText );
    m_pEdit->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}

//------------------------------------------------------------------------------
sal_Bool DbTextField::commitControl()
{
    ::rtl::OUString aText( m_pEdit->GetText( getModelLineEndSetting( m_rColumn.getModel() ) ) );
    // we have to check if the length before we can decide if the value was modified
    xub_StrLen nMaxTextLen = m_pEdit->GetMaxTextLen();
    if ( EDIT_NOLIMIT != nMaxTextLen )
    {
        ::rtl::OUString sOldValue;
        m_rColumn.getModel()->getPropertyValue( FM_PROP_TEXT ) >>= sOldValue;
        // if the new value didn't change we must set the old long value again
        if ( sOldValue.getLength() > nMaxTextLen && sOldValue.compareTo(aText,nMaxTextLen) == 0 )
            aText = sOldValue;
    }
    m_rColumn.getModel()->setPropertyValue( FM_PROP_TEXT, makeAny( aText ) );
    return sal_True;
}


//==============================================================================
//= DbFormattedField
//==============================================================================
DBG_NAME(DbFormattedField);
//------------------------------------------------------------------------------
DbFormattedField::DbFormattedField(DbGridColumn& _rColumn)
    :DbLimitedLengthField(_rColumn)
    ,m_nKeyType(::com::sun::star::util::NumberFormat::UNDEFINED)
{
    DBG_CTOR(DbFormattedField,NULL);

    // if our model's format key changes we want to propagate the new value to our windows
    doPropertyListening( FM_PROP_FORMATKEY );
}

//------------------------------------------------------------------------------
DbFormattedField::~DbFormattedField()
{
    DBG_DTOR(DbFormattedField,NULL);
}

//------------------------------------------------------------------------------
void DbFormattedField::Init(Window* pParent, const Reference< XRowSet >& xCursor)
{
    sal_Int16 nAlignment = m_rColumn.SetAlignmentFromModel(-1);

    Reference< ::com::sun::star::beans::XPropertySet >  xUnoModel = m_rColumn.getModel();

    switch (nAlignment)
    {
        case ::com::sun::star::awt::TextAlign::RIGHT:
            m_pWindow  = new FormattedField(pParent, WB_RIGHT);
            m_pPainter = new FormattedField(pParent, WB_RIGHT);
            break;

        case ::com::sun::star::awt::TextAlign::CENTER:
            m_pWindow  = new FormattedField(pParent, WB_CENTER);
            m_pPainter  = new FormattedField(pParent, WB_CENTER);
            break;
        default:
            m_pWindow  = new FormattedField(pParent, WB_LEFT);
            m_pPainter  = new FormattedField(pParent, WB_LEFT);

            // Alles nur damit die Selektion bei Focuserhalt von rechts nach links geht
            AllSettings aSettings = m_pWindow->GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings);
    }

    implAdjustGenericFieldSetting( xUnoModel );

    static_cast< FormattedField* >( m_pWindow )->SetStrictFormat( sal_False );
    static_cast< FormattedField* >( m_pPainter )->SetStrictFormat( sal_False );
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
        ::cppu::extractInterface(m_xSupplier, aSupplier);
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
            m_xSupplier = getNumberFormats(getRowSetConnection(xCursorForm), sal_False);

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
        pFormatterUsed = ((FormattedField*)m_pWindow)->StandardFormatter();
        DBG_ASSERT(pFormatterUsed != NULL, "DbFormattedField::Init : no standard formatter given by the numeric field !");
    }
    // ... und einen Standard-Key
    if (nFormatKey == -1)
        nFormatKey = 0;

    m_nKeyType  = comphelper::getNumberFormatType(m_xSupplier->getNumberFormats(), nFormatKey);

    ((FormattedField*)m_pWindow)->SetFormatter(pFormatterUsed);
    ((FormattedField*)m_pPainter)->SetFormatter(pFormatterUsed);

    ((FormattedField*)m_pWindow)->SetFormatKey(nFormatKey);
    ((FormattedField*)m_pPainter)->SetFormatKey(nFormatKey);

    ((FormattedField*)m_pWindow)->TreatAsNumber(m_rColumn.IsNumeric());
    ((FormattedField*)m_pPainter)->TreatAsNumber(m_rColumn.IsNumeric());

    // Min- und Max-Werte
    if (m_rColumn.IsNumeric())
    {
        sal_Bool bClearMin = sal_True;
        if (::comphelper::hasProperty(FM_PROP_EFFECTIVE_MIN, xUnoModel))
        {
            Any aMin( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_MIN));
            if (aMin.getValueType().getTypeClass() != TypeClass_VOID)
            {
                DBG_ASSERT(aMin.getValueType().getTypeClass() == TypeClass_DOUBLE, "DbFormattedField::Init : the model has an invalid min value !");
                double dMin = ::comphelper::getDouble(aMin);
                ((FormattedField*)m_pWindow)->SetMinValue(dMin);
                ((FormattedField*)m_pPainter)->SetMinValue(dMin);
                bClearMin = sal_False;
            }
        }
        if (bClearMin)
        {
            ((FormattedField*)m_pWindow)->ClearMinValue();
            ((FormattedField*)m_pPainter)->ClearMinValue();
        }
        sal_Bool bClearMax = sal_True;
        if (::comphelper::hasProperty(FM_PROP_EFFECTIVE_MAX, xUnoModel))
        {
            Any aMin( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_MAX));
            if (aMin.getValueType().getTypeClass() != TypeClass_VOID)
            {
                DBG_ASSERT(aMin.getValueType().getTypeClass() == TypeClass_DOUBLE, "DbFormattedField::Init : the model has an invalid max value !");
                double dMin = ::comphelper::getDouble(aMin);
                ((FormattedField*)m_pWindow)->SetMaxValue(dMin);
                ((FormattedField*)m_pPainter)->SetMaxValue(dMin);
                bClearMax = sal_False;
            }
        }
        if (bClearMax)
        {
            ((FormattedField*)m_pWindow)->ClearMaxValue();
            ((FormattedField*)m_pPainter)->ClearMaxValue();
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
                    ((FormattedField*)m_pWindow)->SetDefaultValue(::comphelper::getDouble(aDefault));
                    ((FormattedField*)m_pPainter)->SetDefaultValue(::comphelper::getDouble(aDefault));
                }
                else
                {
                    String sConverted;
                    Color* pDummy;
                    pFormatterUsed->GetOutputString(::comphelper::getDouble(aDefault), 0, sConverted, &pDummy);
                    ((FormattedField*)m_pWindow)->SetDefaultText(sConverted);
                    ((FormattedField*)m_pPainter)->SetDefaultText(sConverted);
                }
                break;
            case TypeClass_STRING:
            {
                String sDefault( ::comphelper::getString(aDefault) );
                if (m_rColumn.IsNumeric())
                {
                    double dVal;
                    sal_uInt32 nTestFormat(0);
                    if (pFormatterUsed->IsNumberFormat(sDefault, nTestFormat, dVal))
                    {
                        ((FormattedField*)m_pWindow)->SetDefaultValue(dVal);
                        ((FormattedField*)m_pPainter)->SetDefaultValue(dVal);
                    }
                }
                else
                {
                    ((FormattedField*)m_pWindow)->SetDefaultText(sDefault);
                    ((FormattedField*)m_pPainter)->SetDefaultText(sDefault);
                }
            }
        }
    }
    DbLimitedLengthField::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
CellControllerRef DbFormattedField::CreateController() const
{
    return new ::svt::FormattedFieldCellController( static_cast< FormattedField* >( m_pWindow ) );
}

//------------------------------------------------------------------------------
void DbFormattedField::_propertyChanged( const PropertyChangeEvent& _rEvent ) throw( RuntimeException )
{
    if (_rEvent.PropertyName.compareTo(FM_PROP_FORMATKEY) == COMPARE_EQUAL)
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

//------------------------------------------------------------------------------
String DbFormattedField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    // defaultmaessig keine Farb-Angabe
    if (ppColor != NULL)
        *ppColor = NULL;

    // NULL-Wert -> leerer Text
    if (!_rxField.is())
        return String();

    String aText;
    if (m_rColumn.IsNumeric())
    {
        // das IsNumeric an der Column sagt nichts aus ueber die Klasse des benutzen Formates, sondern
        // ueber die des an die Column gebundenen Feldes. Wenn man also eine FormattedField-Spalte an
        // ein double-Feld bindet und als Text formatiert, liefert m_rColumn.IsNumeric() sal_True. Das heisst
        // also einfach, dass ich den Inhalt der Variant mittels getDouble abfragen kann, und dann kann
        // ich den Rest (die Formatierung) dem FormattedField ueberlassen.
        double dValue = getValue(_rxField, m_rColumn.GetParent().getNullDate(), m_nKeyType);
        if (_rxField->wasNull())
            return aText;
        ((FormattedField*)m_pPainter)->SetValue(dValue);
    }
    else
    {
        // Hier kann ich nicht mit einem double arbeiten, da das Feld mir keines liefern kann.
        // Also einfach den Text vom ::com::sun::star::util::NumberFormatter in die richtige ::com::sun::star::form::component::Form brinden lassen.
        aText = (const sal_Unicode*)_rxField->getString();
        if (_rxField->wasNull())
            return aText;
        ((FormattedField*)m_pPainter)->SetTextFormatted(aText);
    }

    aText = m_pPainter->GetText();
    if (ppColor != NULL)
        *ppColor = ((FormattedField*)m_pPainter)->GetLastOutputColor();

    return aText;
}

//------------------------------------------------------------------------------
void DbFormattedField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    FormattedField* pFormattedWindow = static_cast<FormattedField*>(m_pWindow);
    if (!_rxField.is())
    {   // NULL-Wert -> leerer Text
        m_pWindow->SetText(String());
    }
    else if (m_rColumn.IsNumeric())
    {
        // das IsNumeric an der Column sagt nichts aus ueber die Klasse des benutzen Formates, sondern
        // ueber die des an die Column gebundenen Feldes. Wenn man also eine FormattedField-Spalte an
        // ein double-Feld bindet und als Text formatiert, liefert m_rColumn.IsNumeric() sal_True. Das heisst
        // also einfach, dass ich den Inhalt der Variant mittels getDouble abfragen kann, und dann kann
        // ich den Rest (die Formatierung) dem FormattedField ueberlassen.
        double dValue = getValue(_rxField, m_rColumn.GetParent().getNullDate(), m_nKeyType);
        if (_rxField->wasNull())
            m_pWindow->SetText(String());
        else
            pFormattedWindow->SetValue(dValue);
    }
    else
    {
        // Hier kann ich nicht mit einem double arbeiten, da das Feld mir keines liefern kann.
        // Also einfach den Text vom ::com::sun::star::util::NumberFormatter in die richtige ::com::sun::star::form::component::Form brinden lassen.
        String sText( _rxField->getString());

        pFormattedWindow->SetTextFormatted( sText );
        pFormattedWindow->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
    }
}

//------------------------------------------------------------------------------
void DbFormattedField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbFormattedField::updateFromModel: invalid call!" );

    FormattedField* pFormattedWindow = static_cast< FormattedField* >( m_pWindow );

    ::rtl::OUString sText;
    Any aValue = _rxModel->getPropertyValue( FM_PROP_EFFECTIVE_VALUE );
    if ( aValue >>= sText )
    {   // our effective value is transfered as string
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

//------------------------------------------------------------------------------
sal_Bool DbFormattedField::commitControl()
{
    Any aNewVal;
    FormattedField& rField = *(FormattedField*)m_pWindow;
    DBG_ASSERT(&rField == m_pWindow, "DbFormattedField::commitControl : can't work with a window other than my own !");
    if (m_rColumn.IsNumeric())
    {
        if (rField.GetText().Len() != 0)
            aNewVal <<= rField.GetValue();
        // ein LeerString wird erst mal standardmaessig als void weitergereicht
    }
    else
        aNewVal <<= ::rtl::OUString(rField.GetTextValue());

    m_rColumn.getModel()->setPropertyValue(FM_PROP_EFFECTIVE_VALUE, aNewVal);
    return sal_True;
}

//==============================================================================
//= DbCheckBox
//==============================================================================
//------------------------------------------------------------------------------
DbCheckBox::DbCheckBox( DbGridColumn& _rColumn )
    :DbCellControl( _rColumn, sal_True )
{
    setAlignedController( sal_False );
}

namespace
{
    void setCheckBoxStyle( Window* _pWindow, USHORT nStyle )
    {
        AllSettings aSettings = _pWindow->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        aStyleSettings.SetCheckBoxStyle( nStyle );
        aSettings.SetStyleSettings( aStyleSettings );
        _pWindow->SetSettings( aSettings );
    }
}

//------------------------------------------------------------------------------
void DbCheckBox::Init(Window* pParent, const Reference< XRowSet >& xCursor)
{
    setTransparent( sal_True );

    m_pWindow  = new CheckBoxControl(pParent);
    m_pPainter = new CheckBoxControl(pParent);

    m_pWindow->SetPaintTransparent( sal_True );
    m_pPainter->SetPaintTransparent( sal_True );

    m_pPainter->SetBackground();

    try
    {
        Reference< XPropertySet > xModel( m_rColumn.getModel() );
        sal_Int16 nStyle = awt::VisualEffect::LOOK3D;
        if ( xModel.is() )
            xModel->getPropertyValue( FM_PROP_VISUALEFFECT ) >>= nStyle;

        setCheckBoxStyle( m_pWindow, nStyle == awt::VisualEffect::FLAT ? STYLE_CHECKBOX_MONO : STYLE_CHECKBOX_WIN );
        setCheckBoxStyle( m_pPainter, nStyle == awt::VisualEffect::FLAT ? STYLE_CHECKBOX_MONO : STYLE_CHECKBOX_WIN );
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "DbCheckBox::Init: caught an exception!" );
    }

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
CellControllerRef DbCheckBox::CreateController() const
{
    return new CheckBoxCellController((CheckBoxControl*)m_pWindow);
}
//------------------------------------------------------------------------------
static void lcl_setCheckBoxState(   const Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                        CheckBoxControl* _pCheckBoxControl )
{
    TriState eState = STATE_DONTKNOW;
    if (_rxField.is())
    {
        sal_Bool bValue = _rxField->getBoolean();
        if (!_rxField->wasNull())
            eState = bValue ? STATE_CHECK : STATE_NOCHECK;
    }
    _pCheckBoxControl->GetBox().SetState(eState);
}

//------------------------------------------------------------------------------
void DbCheckBox::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    lcl_setCheckBoxState( _rxField, static_cast<CheckBoxControl*>(m_pWindow) );
}

//------------------------------------------------------------------------------
void DbCheckBox::Paint(OutputDevice& rDev, const Rectangle& rRect,
                          const Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                          const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    lcl_setCheckBoxState( _rxField, static_cast<CheckBoxControl*>(m_pPainter) );
    DbCellControl::Paint( rDev, rRect, _rxField, xFormatter );
}

//------------------------------------------------------------------------------
void DbCheckBox::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbCheckBox::updateFromModel: invalid call!" );

    sal_Int16 nState = STATE_DONTKNOW;
    _rxModel->getPropertyValue( FM_PROP_STATE ) >>= nState;
    static_cast< CheckBoxControl* >( m_pWindow )->GetBox().SetState( static_cast< TriState >( nState ) );
}

//------------------------------------------------------------------------------
sal_Bool DbCheckBox::commitControl()
{
#if OSL_DEBUG_LEVEL > 0
    Any aVal = makeAny( (sal_Int16)( static_cast< CheckBoxControl* >( m_pWindow )->GetBox().GetState() ) );
#endif
    m_rColumn.getModel()->setPropertyValue( FM_PROP_STATE,
                    makeAny( (sal_Int16)( static_cast< CheckBoxControl* >( m_pWindow )->GetBox().GetState() ) ) );
    return sal_True;
}

//==============================================================================
//= DbPatternField
//==============================================================================
//------------------------------------------------------------------------------
DbPatternField::DbPatternField( DbGridColumn& _rColumn )
    :DbCellControl( _rColumn )
{
    doPropertyListening( FM_PROP_LITERALMASK );
    doPropertyListening( FM_PROP_EDITMASK );
    doPropertyListening( FM_PROP_STRICTFORMAT );
}

//------------------------------------------------------------------------------
void DbPatternField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbPatternField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbPatternField::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        ::rtl::OUString aLitMask;
        ::rtl::OUString aEditMask;
        sal_Bool bStrict = sal_False;

        _rxModel->getPropertyValue( FM_PROP_LITERALMASK ) >>= aLitMask;
        _rxModel->getPropertyValue( FM_PROP_EDITMASK ) >>= aEditMask;
        _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) >>= bStrict;

        ByteString aAsciiListMask( (const sal_Unicode*)aLitMask, RTL_TEXTENCODING_ASCII_US );

        static_cast< PatternField* >( m_pWindow )->SetMask( aAsciiListMask, aEditMask );
        static_cast< PatternField* >( m_pPainter )->SetMask( aAsciiListMask, aEditMask );
        static_cast< PatternField* >( m_pWindow )->SetStrictFormat( bStrict );
        static_cast< PatternField* >( m_pPainter )->SetStrictFormat( bStrict );
    }
}

//------------------------------------------------------------------------------
void DbPatternField::Init(Window* pParent, const Reference< XRowSet >& xCursor)
{
    m_rColumn.SetAlignmentFromModel(-1);

    m_pWindow = new PatternField( pParent, 0 );
    m_pPainter= new PatternField( pParent, 0 );

    Reference< XPropertySet >   xModel( m_rColumn.getModel() );
    implAdjustGenericFieldSetting( xModel );

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
CellControllerRef DbPatternField::CreateController() const
{
    return new SpinCellController((PatternField*)m_pWindow);
}

//------------------------------------------------------------------------------
String DbPatternField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    ::rtl::OUString aString;
    if (_rxField.is())
        aString = _rxField->getString();

    m_pPainter->SetText(aString);
    return m_pPainter->GetText();
}

//------------------------------------------------------------------------------
void DbPatternField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    Edit* pEdit = (Edit*)m_pWindow;
    pEdit->SetText(GetFormatText(_rxField, xFormatter));
    pEdit->SetSelection(Selection(SELECTION_MAX,SELECTION_MIN));
}

//------------------------------------------------------------------------------
void DbPatternField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbPatternField::updateFromModel: invalid call!" );

    ::rtl::OUString sText;
    _rxModel->getPropertyValue( FM_PROP_TEXT ) >>= sText;

    static_cast< Edit* >( m_pWindow )->SetText( sText );
    static_cast< Edit* >( m_pWindow )->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}

//------------------------------------------------------------------------------
sal_Bool DbPatternField::commitControl()
{
    String aText(m_pWindow->GetText());
    m_rColumn.getModel()->setPropertyValue(FM_PROP_TEXT, makeAny(::rtl::OUString(aText)));
    return sal_True;
}

//==============================================================================
//= DbSpinField
//==============================================================================
//------------------------------------------------------------------------------
DbSpinField::DbSpinField( DbGridColumn& _rColumn, sal_Int16 _nStandardAlign )
    :DbCellControl( _rColumn )
    ,m_nStandardAlign( _nStandardAlign )
{
}

//------------------------------------------------------------------------------
void DbSpinField::Init( Window* _pParent, const Reference< XRowSet >& _rxCursor )
{
    m_rColumn.SetAlignmentFromModel( m_nStandardAlign );

    Reference< XPropertySet > xModel( m_rColumn.getModel() );

    // determine the WinBits for the field
    WinBits nFieldStyle = 0;
    if ( ::comphelper::getBOOL( xModel->getPropertyValue( FM_PROP_SPIN ) ) )
        nFieldStyle = WB_REPEAT | WB_SPIN;
    // create the fields
    m_pWindow = createField( _pParent, nFieldStyle, xModel );
    m_pPainter = createField( _pParent, nFieldStyle, xModel );

    // adjust all other settings which depend on the property values
    implAdjustGenericFieldSetting( xModel );

    // call the base class
    DbCellControl::Init( _pParent, _rxCursor );
}

//------------------------------------------------------------------------------
CellControllerRef DbSpinField::CreateController() const
{
    return new SpinCellController( static_cast< SpinField* >( m_pWindow ) );
}

//==============================================================================
//= DbNumericField
//==============================================================================
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void DbNumericField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbNumericField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbNumericField::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Int32   nMin        = (sal_Int32)getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMIN ) );
        sal_Int32   nMax        = (sal_Int32)getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMAX ) );
        sal_Int32   nStep       = (sal_Int32)getDouble( _rxModel->getPropertyValue( FM_PROP_VALUESTEP ) );
        sal_Bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );
        sal_Int16   nScale      = getINT16( _rxModel->getPropertyValue( FM_PROP_DECIMAL_ACCURACY ) );
        sal_Bool    bThousand   = getBOOL( _rxModel->getPropertyValue( FM_PROP_SHOWTHOUSANDSEP ) );

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
            xForm = Reference< XRowSet >( ( Reference< XInterface > )*m_rColumn.GetParent().getDataSource(), UNO_QUERY );
        if ( xForm.is() )
            xSupplier = getNumberFormats( getRowSetConnection( xForm ), sal_True );
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
        String sFormatString;
        LanguageType aAppLanguage = Application::GetSettings().GetUILanguage();
        pFormatterUsed->GenerateFormat( sFormatString, 0, aAppLanguage, bThousand, sal_False, nScale );

        static_cast< DoubleNumericField* >( m_pWindow )->SetFormat( sFormatString, aAppLanguage );
        static_cast< DoubleNumericField* >( m_pPainter )->SetFormat( sFormatString, aAppLanguage );
    }
}

//------------------------------------------------------------------------------
SpinField* DbNumericField::createField( Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& _rxModel  )
{
    return new DoubleNumericField( _pParent, _nFieldStyle );
}

//------------------------------------------------------------------------------
String DbNumericField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_rxField.is())
        return String();
    else
    {
        double fValue = GetValue(_rxField, xFormatter);
        if (_rxField->wasNull())
            return String();
        else
        {
            ((DoubleNumericField*)m_pPainter)->SetValue(fValue);
            return m_pPainter->GetText();
        }
    }
}

//------------------------------------------------------------------------------
void DbNumericField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (!_rxField.is())
        m_pWindow->SetText(String());
    else
    {
        double fValue = GetValue(_rxField, xFormatter);
        if (_rxField->wasNull())
            m_pWindow->SetText(String());
        else
            ((DoubleNumericField*)m_pWindow)->SetValue(fValue);
    }
}

//------------------------------------------------------------------------------
void DbNumericField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbNumericField::updateFromModel: invalid call!" );

    double dValue = 0;
    if ( _rxModel->getPropertyValue( FM_PROP_VALUE ) >>= dValue )
        static_cast< DoubleNumericField* >( m_pWindow )->SetValue( dValue );
    else
        m_pWindow->SetText( String() );
}

//------------------------------------------------------------------------------
sal_Bool DbNumericField::commitControl()
{
    String aText( m_pWindow->GetText());
    Any aVal;

    if (aText.Len() != 0)   // nicht null
    {
        double fValue = ((DoubleNumericField*)m_pWindow)->GetValue();
        aVal <<= (double)fValue;
    }
    m_rColumn.getModel()->setPropertyValue(FM_PROP_VALUE, aVal);
    return sal_True;
}

//==============================================================================
//= DbCurrencyField
//==============================================================================
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void DbCurrencyField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbCurrencyField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbCurrencyField::1: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        m_nScale                = getINT16( _rxModel->getPropertyValue( FM_PROP_DECIMAL_ACCURACY ) );
        double  nMin            = getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMIN ) );
        double  nMax            = getDouble( _rxModel->getPropertyValue( FM_PROP_VALUEMAX ) );
        double  nStep           = getDouble( _rxModel->getPropertyValue( FM_PROP_VALUESTEP ) );
        sal_Bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );
        sal_Bool    bThousand   = getBOOL( _rxModel->getPropertyValue( FM_PROP_SHOWTHOUSANDSEP ) );
        ::rtl::OUString aStr( getString( _rxModel->getPropertyValue(FM_PROP_CURRENCYSYMBOL ) ) );

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

//------------------------------------------------------------------------------
SpinField* DbCurrencyField::createField( Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& _rxModel  )
{
    return new LongCurrencyField( _pParent, _nFieldStyle );
}

//------------------------------------------------------------------------------
double DbCurrencyField::GetCurrency(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const
{
    double fValue = GetValue(_rxField, xFormatter);
    if (m_nScale)
        fValue *= pow(10.0, double(m_nScale));
    return fValue;
}

//------------------------------------------------------------------------------
String DbCurrencyField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_rxField.is())
        return String();
    else
    {
        double fValue = GetCurrency(_rxField, xFormatter);
        if (_rxField->wasNull())
            return String();
        else
        {
            ((LongCurrencyField*)m_pPainter)->SetValue(fValue);
            return m_pPainter->GetText();
        }
    }
}

//------------------------------------------------------------------------------
void DbCurrencyField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (!_rxField.is())
        m_pWindow->SetText(String());
    else
    {
         double fValue = GetCurrency(_rxField, xFormatter);
         if (_rxField->wasNull())
             m_pWindow->SetText(String());
         else
            ((LongCurrencyField*)m_pWindow)->SetValue(fValue);
    }
}

//------------------------------------------------------------------------------
void DbCurrencyField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbCurrencyField::updateFromModel: invalid call!" );

    double dValue = 0;
    if ( _rxModel->getPropertyValue( FM_PROP_VALUE ) >>= dValue )
    {
        if ( m_nScale )
            dValue *= pow( 10.0, double( m_nScale ) );

        static_cast< LongCurrencyField* >( m_pWindow )->SetValue( dValue );
    }
    else
        m_pWindow->SetText( String() );
}

//------------------------------------------------------------------------------
sal_Bool DbCurrencyField::commitControl()
{
    String aText( m_pWindow->GetText());
    Any aVal;
    if (aText.Len() != 0)   // nicht null
    {
        double fValue = ((LongCurrencyField*)m_pWindow)->GetValue();
        if (m_nScale)
            fValue /= pow(10.0, double(m_nScale));
        aVal <<= (double)fValue;
    }
    m_rColumn.getModel()->setPropertyValue(FM_PROP_VALUE, aVal);
    return sal_True;
}

//==============================================================================
//= DbDateField
//==============================================================================
//------------------------------------------------------------------------------
DbDateField::DbDateField( DbGridColumn& _rColumn )
    :DbSpinField( _rColumn )
{
    doPropertyListening( FM_PROP_DATEFORMAT );
    doPropertyListening( FM_PROP_DATEMIN );
    doPropertyListening( FM_PROP_DATEMAX );
    doPropertyListening( FM_PROP_STRICTFORMAT );
    doPropertyListening( FM_PROP_DATE_SHOW_CENTURY );
}

//------------------------------------------------------------------------------
SpinField* DbDateField::createField( Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& _rxModel  )
{
    // check if there is a DropDown property set to TRUE
    sal_Bool bDropDown =    !hasProperty( FM_PROP_DROPDOWN, _rxModel )
                        ||  getBOOL( _rxModel->getPropertyValue( FM_PROP_DROPDOWN ) );
    if ( bDropDown )
        _nFieldStyle |= WB_DROPDOWN;

    CalendarField* pField = new CalendarField( _pParent, _nFieldStyle );

    pField->EnableToday();
    pField->EnableNone();

    return pField;
}

//------------------------------------------------------------------------------
void DbDateField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbDateField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbDateField::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Int16   nFormat     = getINT16( _rxModel->getPropertyValue( FM_PROP_DATEFORMAT ) );
        sal_Int32   nMin        = getINT32( _rxModel->getPropertyValue( FM_PROP_DATEMIN ) );
        sal_Int32   nMax        = getINT32( _rxModel->getPropertyValue( FM_PROP_DATEMAX ) );
        sal_Bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );

        Any  aCentury = _rxModel->getPropertyValue( FM_PROP_DATE_SHOW_CENTURY );
        if ( aCentury.getValueType().getTypeClass() != TypeClass_VOID )
        {
            sal_Bool bShowDateCentury = getBOOL( aCentury );

            static_cast<DateField*>( m_pWindow )->SetShowDateCentury( bShowDateCentury );
            static_cast<DateField*>( m_pPainter )->SetShowDateCentury( bShowDateCentury );
        }

        static_cast< DateField* >( m_pWindow )->SetExtDateFormat( (ExtDateFieldFormat)nFormat );
        static_cast< DateField* >( m_pWindow )->SetMin( nMin );
        static_cast< DateField* >( m_pWindow )->SetMax( nMax );
        static_cast< DateField* >( m_pWindow )->SetStrictFormat( bStrict );

        static_cast< DateField* >( m_pPainter )->SetExtDateFormat( (ExtDateFieldFormat)nFormat );
        static_cast< DateField* >( m_pPainter )->SetMin( nMin );
        static_cast< DateField* >( m_pPainter )->SetMax( nMax );
        static_cast< DateField* >( m_pPainter )->SetStrictFormat( bStrict );
    }
}

//------------------------------------------------------------------------------
String DbDateField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_rxField.is())
        return String();
    else
    {
        ::com::sun::star::util::Date aValue = _rxField->getDate();
        if (_rxField->wasNull())
            return String();
        else
        {
            static_cast<DateField*>(m_pPainter)->SetDate(::Date(aValue.Day, aValue.Month, aValue.Year));
            return m_pPainter->GetText();
        }
    }
}

//------------------------------------------------------------------------------
void DbDateField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (!_rxField.is())
        m_pWindow->SetText(String());
    else
    {
        ::com::sun::star::util::Date aValue = _rxField->getDate();
        if (_rxField->wasNull())
            m_pWindow->SetText(String());
        else
        {
            static_cast<DateField*>(m_pWindow)->SetDate(::Date(aValue.Day, aValue.Month, aValue.Year));
        }
    }
}

//------------------------------------------------------------------------------
void DbDateField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbDateField::updateFromModel: invalid call!" );

    sal_Int32 nDate = 0;
    if ( _rxModel->getPropertyValue( FM_PROP_DATE ) >>= nDate )
        static_cast< DateField* >( m_pWindow )->SetDate( ::Date( nDate ) );
    else
        static_cast< DateField* >( m_pWindow )->SetText( String() );
}

//------------------------------------------------------------------------------
sal_Bool DbDateField::commitControl()
{
    String aText( m_pWindow->GetText());
    Any aVal;
    if (aText.Len() != 0)
        aVal <<= (sal_Int32)static_cast<DateField*>(m_pWindow)->GetDate().GetDate();
    else
        aVal.clear();

    m_rColumn.getModel()->setPropertyValue(FM_PROP_DATE, aVal);
    return sal_True;
}

//==============================================================================
//= DbTimeField
//==============================================================================
//------------------------------------------------------------------------------
DbTimeField::DbTimeField( DbGridColumn& _rColumn )
    :DbSpinField( _rColumn, ::com::sun::star::awt::TextAlign::LEFT )
{
    doPropertyListening( FM_PROP_TIMEFORMAT );
    doPropertyListening( FM_PROP_TIMEMIN );
    doPropertyListening( FM_PROP_TIMEMAX );
    doPropertyListening( FM_PROP_STRICTFORMAT );
}

//------------------------------------------------------------------------------
SpinField* DbTimeField::createField( Window* _pParent, WinBits _nFieldStyle, const Reference< XPropertySet >& _rxModel )
{
    return new TimeField( _pParent, _nFieldStyle );
}

//------------------------------------------------------------------------------
void DbTimeField::implAdjustGenericFieldSetting( const Reference< XPropertySet >& _rxModel )
{
    DBG_ASSERT( m_pWindow, "DbTimeField::implAdjustGenericFieldSetting: not to be called without window!" );
    DBG_ASSERT( _rxModel.is(), "DbTimeField::implAdjustGenericFieldSetting: invalid model!" );
    if ( m_pWindow && _rxModel.is() )
    {
        sal_Int16   nFormat     = getINT16( _rxModel->getPropertyValue( FM_PROP_TIMEFORMAT ) );
        sal_Int32   nMin        = getINT32( _rxModel->getPropertyValue( FM_PROP_TIMEMIN ) );
        sal_Int32   nMax        = getINT32( _rxModel->getPropertyValue( FM_PROP_TIMEMAX ) );
        sal_Bool    bStrict     = getBOOL( _rxModel->getPropertyValue( FM_PROP_STRICTFORMAT ) );

        static_cast< TimeField* >( m_pWindow )->SetExtFormat( (ExtTimeFieldFormat)nFormat );
        static_cast< TimeField* >( m_pWindow )->SetMin( nMin );
        static_cast< TimeField* >( m_pWindow )->SetMax( nMax );
        static_cast< TimeField* >( m_pWindow )->SetStrictFormat( bStrict );

        static_cast< TimeField* >( m_pPainter )->SetExtFormat( (ExtTimeFieldFormat)nFormat );
        static_cast< TimeField* >( m_pPainter )->SetMin( nMin );
        static_cast< TimeField* >( m_pPainter )->SetMax( nMax );
        static_cast< TimeField* >( m_pPainter )->SetStrictFormat( bStrict );
    }
}

//------------------------------------------------------------------------------
String DbTimeField::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_rxField.is())
        return String();
    else
    {
        ::com::sun::star::util::Time aValue = _rxField->getTime();
        if (_rxField->wasNull())
            return String();
        else
        {
            static_cast<TimeField*>(m_pPainter)->SetTime(::Time(aValue.Hours, aValue.Minutes, aValue.Seconds, aValue.HundredthSeconds));
            return m_pPainter->GetText();
        }
    }
}

//------------------------------------------------------------------------------
void DbTimeField::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (!_rxField.is())
        m_pWindow->SetText(String());
    else
    {
        ::com::sun::star::util::Time aValue = _rxField->getTime();
        if (_rxField->wasNull())
            m_pWindow->SetText(String());
        else
        {
            static_cast<TimeField*>(m_pWindow)->SetTime(::Time(aValue.Hours, aValue.Minutes, aValue.Seconds, aValue.HundredthSeconds));
        }
    }
}

//------------------------------------------------------------------------------
void DbTimeField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbTimeField::updateFromModel: invalid call!" );

    sal_Int32 nTime = 0;
    if ( _rxModel->getPropertyValue( FM_PROP_DATE ) >>= nTime )
        static_cast< TimeField* >( m_pWindow )->SetTime( ::Time( nTime ) );
    else
        static_cast< TimeField* >( m_pWindow )->SetText( String() );
}

//------------------------------------------------------------------------------
sal_Bool DbTimeField::commitControl()
{
    String aText( m_pWindow->GetText());
    Any aVal;
    if (aText.Len() != 0)
        aVal <<= (sal_Int32)static_cast<TimeField*>(m_pWindow)->GetTime().GetTime();
    else
        aVal.clear();

    m_rColumn.getModel()->setPropertyValue(FM_PROP_TIME, aVal);
    return sal_True;
}

//==============================================================================
//= DbComboBox
//==============================================================================
//------------------------------------------------------------------------------
DbComboBox::DbComboBox(DbGridColumn& _rColumn)
           :DbCellControl(_rColumn)
           ,m_nKeyType(::com::sun::star::util::NumberFormat::UNDEFINED)
{
    setAlignedController( sal_False );

    doPropertyListening( FM_PROP_STRINGITEMLIST );
    doPropertyListening( FM_PROP_LINECOUNT );
}

//------------------------------------------------------------------------------
void DbComboBox::_propertyChanged( const PropertyChangeEvent& _rEvent ) throw( RuntimeException )
{
    if ( _rEvent.PropertyName.equalsAscii( FM_PROP_STRINGITEMLIST ) )
    {
        SetList(_rEvent.NewValue);
    }
    else
    {
        DbCellControl::_propertyChanged( _rEvent ) ;
    }
}

//------------------------------------------------------------------------------
void DbComboBox::SetList(const Any& rItems)
{
    ComboBoxControl* pField = (ComboBoxControl*)m_pWindow;
    pField->Clear();

    ::comphelper::StringSequence aTest;
    if (rItems >>= aTest)
    {
        const ::rtl::OUString* pStrings = aTest.getConstArray();
        sal_Int32 nItems = aTest.getLength();
        for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
             pField->InsertEntry(*pStrings, LISTBOX_APPEND);

        // tell the grid control that this controller is invalid and has to be re-initialized
        invalidatedController();
    }
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void DbComboBox::Init(Window* pParent, const Reference< XRowSet >& xCursor)
{
    m_rColumn.SetAlignmentFromModel(::com::sun::star::awt::TextAlign::LEFT);

    m_pWindow = new ComboBoxControl(pParent);
    m_pPainter = new Edit( pParent, WB_NOBORDER );

    // selection von rechts nach links
    AllSettings     aSettings = m_pWindow->GetSettings();
    StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
    aStyleSettings.SetSelectionOptions(
        aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
    aSettings.SetStyleSettings(aStyleSettings);
    m_pWindow->SetSettings(aSettings, sal_True);

    // some initial properties
    Reference< XPropertySet >   xModel(m_rColumn.getModel());
    SetList( xModel->getPropertyValue( FM_PROP_STRINGITEMLIST ) );
    implAdjustGenericFieldSetting( xModel );

    if (m_rColumn.GetParent().getNumberFormatter().is())
        m_nKeyType  = comphelper::getNumberFormatType(m_rColumn.GetParent().getNumberFormatter()->getNumberFormatsSupplier()->getNumberFormats(), m_rColumn.GetKey());

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
CellControllerRef DbComboBox::CreateController() const
{
    return new ComboBoxCellController((ComboBoxControl*)m_pWindow);
}

//------------------------------------------------------------------------------
String DbComboBox::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    ::rtl::OUString aString;
    if (_rxField.is())
        aString = getValue( _rxField, xFormatter, m_rColumn.GetParent().getNullDate(), m_rColumn.GetKey(), m_nKeyType );
    return aString;
}

//------------------------------------------------------------------------------
void DbComboBox::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    m_pWindow->SetText(GetFormatText(_rxField, xFormatter));
}

//------------------------------------------------------------------------------
void DbComboBox::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbComboBox::updateFromModel: invalid call!" );

    ::rtl::OUString sText;
    _rxModel->getPropertyValue( FM_PROP_TEXT ) >>= sText;

    static_cast< ComboBox* >( m_pWindow )->SetText( sText );
    static_cast< ComboBox* >( m_pWindow )->SetSelection( Selection( SELECTION_MAX, SELECTION_MIN ) );
}

//------------------------------------------------------------------------------
sal_Bool DbComboBox::commitControl()
{
    String aText( m_pWindow->GetText());
    m_rColumn.getModel()->setPropertyValue(FM_PROP_TEXT, makeAny(::rtl::OUString(aText)));
    return sal_True;
}

//------------------------------------------------------------------------------
DbListBox::DbListBox(DbGridColumn& _rColumn)
          :DbCellControl(_rColumn)
          ,m_bBound(sal_False)
{
    setAlignedController( sal_False );

    doPropertyListening( FM_PROP_STRINGITEMLIST );
    doPropertyListening( FM_PROP_LINECOUNT );
}

//------------------------------------------------------------------------------
void DbListBox::_propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent ) throw( RuntimeException )
{
    if ( _rEvent.PropertyName.equalsAscii( FM_PROP_STRINGITEMLIST ) )
    {
        SetList(_rEvent.NewValue);
    }
    else
    {
        DbCellControl::_propertyChanged( _rEvent ) ;
    }
}

//------------------------------------------------------------------------------
void DbListBox::SetList(const Any& rItems)
{
    ListBoxControl* pField = (ListBoxControl*)m_pWindow;

    pField->Clear();
    m_bBound = sal_False;

    ::comphelper::StringSequence aTest;
    if (rItems >>= aTest)
    {
        const ::rtl::OUString* pStrings = aTest.getConstArray();
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

//------------------------------------------------------------------------------
void DbListBox::Init(Window* pParent, const Reference< XRowSet >& xCursor)
{
    m_rColumn.SetAlignment(::com::sun::star::awt::TextAlign::LEFT);

    m_pWindow = new ListBoxControl(pParent);

    // some initial properties
    Reference< XPropertySet > xModel( m_rColumn.getModel() );
    SetList( xModel->getPropertyValue( FM_PROP_STRINGITEMLIST ) );
    implAdjustGenericFieldSetting( xModel );

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
CellControllerRef DbListBox::CreateController() const
{
    return new ListBoxCellController((ListBoxControl*)m_pWindow);
}

//------------------------------------------------------------------------------
String DbListBox::GetFormatText(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_rxField.is())
        return String();
    else
    {
        String aText;
        if (m_bBound)
        {
            Sequence<sal_Int16> aPosSeq = ::comphelper::findValue(m_aValueList, _rxField->getString(), sal_True);
            if (aPosSeq.getLength())
                aText = static_cast<ListBox*>(m_pWindow)->GetEntry(aPosSeq.getConstArray()[0]);
        }
        else
            aText = (const sal_Unicode*)_rxField->getString();
        return aText;
    }
}

//------------------------------------------------------------------------------
void DbListBox::UpdateFromField(const Reference< ::com::sun::star::sdb::XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    static_cast<ListBox*>(m_pWindow)->SelectEntry(GetFormatText(_rxField, xFormatter));
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
sal_Bool DbListBox::commitControl()
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
    return sal_True;
}


DBG_NAME(DbFilterField);
/*************************************************************************/
DbFilterField::DbFilterField(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,DbGridColumn& _rColumn)
              :DbCellControl(_rColumn)
              ,OSQLParserClient(_rxORB)
              ,m_bFilterList(sal_False)
              ,m_nControlClass(::com::sun::star::form::FormComponentType::TEXTFIELD)
              ,m_bFilterListFilled(sal_False)
              ,m_bBound(sal_False)
{
    DBG_CTOR(DbFilterField,NULL);

    setAlignedController( sal_False );
}

//------------------------------------------------------------------------------
DbFilterField::~DbFilterField()
{
    if (m_nControlClass == ::com::sun::star::form::FormComponentType::CHECKBOX)
        ((CheckBoxControl*)m_pWindow)->SetClickHdl( Link() );

    DBG_DTOR(DbFilterField,NULL);
}

//------------------------------------------------------------------------------
void DbFilterField::Paint(OutputDevice& rDev, const Rectangle& rRect)
{
    static sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_VCENTER | TEXT_DRAW_LEFT;
    switch (m_nControlClass)
    {
        case FormComponentType::CHECKBOX:
            DbCellControl::Paint( rDev, rRect );
            break;
        case FormComponentType::LISTBOX:
            rDev.DrawText(rRect, static_cast<ListBox*>(m_pWindow)->GetSelectEntry(), nStyle);
            break;
        default:
            rDev.DrawText(rRect, m_aText, nStyle);
    }
}

//------------------------------------------------------------------------------
void DbFilterField::SetList(const Any& rItems, sal_Bool bComboBox)
{
    ::comphelper::StringSequence aTest;
    rItems >>= aTest;
    const ::rtl::OUString* pStrings = aTest.getConstArray();
    sal_Int32 nItems = aTest.getLength();
    if (nItems)
    {
        if (bComboBox)
        {
            ComboBox* pField = (ComboBox*)m_pWindow;
            for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
                pField->InsertEntry(*pStrings, LISTBOX_APPEND);
        }
        else
        {
            ListBox* pField = (ListBox*)m_pWindow;
            for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
                pField->InsertEntry(*pStrings, LISTBOX_APPEND);

            m_rColumn.getModel()->getPropertyValue(FM_PROP_VALUE_SEQ) >>= m_aValueList;
            m_bBound = m_aValueList.getLength() > 0;
        }
    }
}

//------------------------------------------------------------------------------
void DbFilterField::CreateControl(Window* pParent, const Reference< ::com::sun::star::beans::XPropertySet >& xModel)
{
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
            m_pWindow = new CheckBoxControl(pParent);
            m_pWindow->SetPaintTransparent( sal_True );
            ((CheckBoxControl*)m_pWindow)->SetClickHdl( LINK( this, DbFilterField, OnClick ) );

            m_pPainter = new CheckBoxControl(pParent);
            m_pPainter->SetPaintTransparent( sal_True );
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
            m_pWindow->SetSettings(aSettings, sal_True);

            if (!m_bFilterList)
            {
                sal_Int16  nLines       = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_LINECOUNT));
                Any  aItems      = xModel->getPropertyValue(FM_PROP_STRINGITEMLIST);
                SetList(aItems, m_nControlClass == ::com::sun::star::form::FormComponentType::COMBOBOX);
                ((ComboBox*)m_pWindow)->SetDropDownLineCount(nLines);
            }
            else
                ((ComboBox*)m_pWindow)->SetDropDownLineCount(5);

        }   break;
        default:
        {
            m_pWindow  = new Edit(pParent, WB_LEFT);
            AllSettings     aSettings = m_pWindow->GetSettings();
            StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                           aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings, sal_True);
        }
    }
}

//------------------------------------------------------------------------------
void DbFilterField::Init(Window* pParent, const Reference< XRowSet >& xCursor)
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

    CreateControl(pParent, xModel);
    DbCellControl::Init(pParent, xCursor);

    // filter cells are never readonly
    // 31.07.2002 - 101584 - fs@openoffice.org
    if ( FormComponentType::LISTBOX != m_nControlClass )
        static_cast< Edit* >( m_pWindow )->SetReadOnly( sal_False );
}

//------------------------------------------------------------------------------
CellControllerRef DbFilterField::CreateController() const
{
    CellControllerRef xController;
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
            xController = new CheckBoxCellController((CheckBoxControl*)m_pWindow);
            break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
            xController = new ListBoxCellController((ListBoxControl*)m_pWindow);
            break;
        case ::com::sun::star::form::FormComponentType::COMBOBOX:
            xController = new ComboBoxCellController((ComboBoxControl*)m_pWindow);
            break;
        default:
            if (m_bFilterList)
                xController = new ComboBoxCellController((ComboBoxControl*)m_pWindow);
            else
                xController = new EditCellController((Edit*)m_pWindow);
    }
    return xController;
}

//------------------------------------------------------------------------------
void DbFilterField::updateFromModel( Reference< XPropertySet > _rxModel )
{
    OSL_ENSURE( _rxModel.is() && m_pWindow, "DbFilterField::updateFromModel: invalid call!" );

    OSL_ENSURE( sal_False, "DbListBox::updateFromModel: not implemented yet (how the hell did you reach this??)!" );
    // TODO: implement this.
    // remember: updateFromModel should be some kind of opposite of commitControl
}

//------------------------------------------------------------------------------
sal_Bool DbFilterField::commitControl()
{
    String aText(m_aText);
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
            return sal_True;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
            if (static_cast<ListBox*>(m_pWindow)->GetSelectEntryCount())
            {
                sal_Int16 nPos = (sal_Int16)static_cast<ListBox*>(m_pWindow)->GetSelectEntryPos();
                aText = (const sal_Unicode*)m_aValueList.getConstArray()[nPos];
            }
            else
                aText.Erase();

            if (m_aText != aText)
            {
                m_aText = aText;
                m_aCommitLink.Call(this);
            }
            return sal_True;
        default:
            aText = m_pWindow->GetText();
    }

    if (m_aText != aText)
    {
        // check the text with the SQL-Parser
        String aNewText(aText);
        aNewText.EraseTrailingChars();
        if (aNewText.Len() != 0)
        {
            ::rtl::OUString aErrorMsg;
            Reference< ::com::sun::star::util::XNumberFormatter >  xNumberFormatter(m_rColumn.GetParent().getNumberFormatter());

            ::rtl::Reference< ISQLParseNode > xParseNode = predicateTree(aErrorMsg, aNewText,xNumberFormatter, m_rColumn.GetField());
            if (xParseNode.is())
            {
                ::rtl::OUString aPreparedText;

                ::com::sun::star::lang::Locale aAppLocale = Application::GetSettings().GetUILocale();

                Reference< XRowSet > xDataSourceRowSet(
                    (Reference< XInterface >)*m_rColumn.GetParent().getDataSource(), UNO_QUERY);
                Reference< XConnection >  xConnection(getRowSetConnection(xDataSourceRowSet));

                xParseNode->parseNodeToPredicateStr(aPreparedText,
                                                    xConnection->getMetaData(),
                                                    xNumberFormatter,
                                                    m_rColumn.GetField(),aAppLocale,'.',
                                                    getParseContext());
                m_aText = aPreparedText;
            }
            else
            {
                // display the error and return sal_False
                String aTitle( SVX_RES(RID_STR_SYNTAXERROR) );

                SQLException aError;
                aError.Message = aErrorMsg;
                displayException(aError, m_pWindow->GetParent());
                    // TODO: transport the title

                return sal_False;
            }
        }
        else
            m_aText = aText;

        m_pWindow->SetText(m_aText);
        m_aCommitLink.Call(this);
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void DbFilterField::SetText(const String& rText)
{
    m_aText = rText;
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
        {
            TriState eState;
            if (rText.EqualsAscii("1"))
                eState = STATE_CHECK;
            else if (rText.EqualsAscii("0"))
                eState = STATE_NOCHECK;
            else
                eState = STATE_DONTKNOW;

            ((CheckBoxControl*)m_pWindow)->GetBox().SetState(eState);
            ((CheckBoxControl*)m_pPainter)->GetBox().SetState(eState);
        }   break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
        {
            String aText;
            Sequence<sal_Int16> aPosSeq = ::comphelper::findValue(m_aValueList, m_aText, sal_True);
            if (aPosSeq.getLength())
                static_cast<ListBox*>(m_pWindow)->SelectEntryPos(aPosSeq.getConstArray()[0], sal_True);
            else
                static_cast<ListBox*>(m_pWindow)->SelectEntryPos(-1, sal_True);
        }   break;
        default:
            m_pWindow->SetText(m_aText);
    }

    // now force a repaint on the window
    m_rColumn.GetParent().RowModified(0,m_rColumn.GetId());
}

//------------------------------------------------------------------------------
void DbFilterField::Update()
{
    // should we fill the combobox with a filter proposal?
    if (m_bFilterList && !m_bFilterListFilled)
    {
        m_bFilterListFilled = sal_True;
        Reference< ::com::sun::star::beans::XPropertySet >  xField = m_rColumn.GetField();
        if (!xField.is())
            return;

        ::rtl::OUString aName;
        xField->getPropertyValue(FM_PROP_NAME) >>= aName;

        // the columnmodel
        Reference< ::com::sun::star::container::XChild >  xModelAsChild(m_rColumn.getModel(), UNO_QUERY);
        // the grid model
        xModelAsChild = Reference< ::com::sun::star::container::XChild > (xModelAsChild->getParent(),UNO_QUERY);
        Reference< XRowSet >  xForm(xModelAsChild->getParent(), UNO_QUERY);
        if (!xForm.is())
            return;

        Reference< XConnection >  xConnection(getRowSetConnection(xForm));
        if (!xConnection.is())
            return;

        Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory >  xFactory(xConnection, UNO_QUERY);
        if (!xFactory.is())
        {
            DBG_ERROR("DbFilterField::Update : used the right place to request the ::com::sun::star::sdb::XSQLQueryComposerFactory interface ?");
            return;
        }

        Reference< ::com::sun::star::sdb::XSQLQueryComposer >  xComposer = xFactory->createQueryComposer();
        try
        {
            Reference< ::com::sun::star::beans::XPropertySet >  xFormAsSet(xForm, UNO_QUERY);
            ::rtl::OUString sStatement;
            xFormAsSet->getPropertyValue(FM_PROP_ACTIVECOMMAND) >>= sStatement;
            xComposer->setQuery(sStatement);
        }
        catch(const Exception&)
        {
            ::comphelper::disposeComponent(xComposer);
            return;
        }

        Reference< ::com::sun::star::beans::XPropertySet >  xComposerAsSet(xComposer, UNO_QUERY);
        if (!xComposerAsSet.is())
            return;

        // search the field
        Reference< ::com::sun::star::container::XNameAccess >    xFieldNames;
        Reference< ::com::sun::star::container::XNameAccess >    xTablesNames;
        Reference< ::com::sun::star::beans::XPropertySet >       xComposerFieldAsSet;

        ::cppu::extractInterface(xFieldNames, xComposerAsSet->getPropertyValue(FM_PROP_SELECTED_FIELDS));
        ::cppu::extractInterface(xTablesNames, xComposerAsSet->getPropertyValue(FM_PROP_SELECTED_TABLES));
        ::cppu::extractInterface(xComposerFieldAsSet, xFieldNames->getByName(aName));

        if (xComposerFieldAsSet.is() && ::comphelper::hasProperty(FM_PROP_TABLENAME, xComposerFieldAsSet) &&
            ::comphelper::hasProperty(FM_PROP_FIELDSOURCE, xComposerFieldAsSet))
        {
            ::rtl::OUString aFieldName;
            ::rtl::OUString aTableName;
            xComposerFieldAsSet->getPropertyValue(FM_PROP_FIELDSOURCE)  >>= aFieldName;
            xComposerFieldAsSet->getPropertyValue(FM_PROP_TABLENAME)    >>= aTableName;

            // no possibility to create a select statement
            // looking for the complete table name
            if (!xTablesNames->hasByName(aTableName))
                return;

            // this is the tablename
            Reference< ::com::sun::star::container::XNamed > xTableNameAccess;
            ::cppu::extractInterface(xTableNameAccess, xTablesNames->getByName(aTableName));
            aTableName = xTableNameAccess->getName();

            // ein Statement aufbauen und abschicken als query
            // Access to the connection
            Reference< XStatement >  xStatement;
            Reference< XResultSet >  xListCursor;
            Reference< ::com::sun::star::sdb::XColumn >  xDataField;

            try
            {
                Reference< XDatabaseMetaData >  xMeta = xConnection->getMetaData();

                String aQuote( xMeta->getIdentifierQuoteString());
                String aStatement;
                aStatement.AssignAscii("SELECT DISTINCT ");

                aStatement += String(quoteName(aQuote, aName));
                if (aFieldName.getLength() && aName != aFieldName)
                {
                    aStatement.AppendAscii(" AS ");
                    aStatement += quoteName(aQuote, aFieldName).getStr();
                }

                aStatement.AppendAscii(" FROM ");
                sal_Bool bUseCatalogInSelect = isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalogInSelect")),sal_True);
                sal_Bool bUseSchemaInSelect = isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSchemaInSelect")),sal_True);

                aStatement += quoteTableName(xMeta, aTableName,bUseCatalogInSelect,bUseSchemaInSelect).getStr();

                xStatement = xConnection->createStatement();
                Reference< ::com::sun::star::beans::XPropertySet >  xStatementProps(xStatement, UNO_QUERY);
                xStatementProps->setPropertyValue(FM_PROP_ESCAPE_PROCESSING, makeAny((sal_Bool)sal_True));

                xListCursor = xStatement->executeQuery(aStatement);

                Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(xListCursor, UNO_QUERY);
                Reference< ::com::sun::star::container::XIndexAccess >  xFields(xSupplyCols->getColumns(), UNO_QUERY);
                ::cppu::extractInterface(xDataField, xFields->getByIndex(0));
                if (!xDataField.is())
                    return;
            }
            catch(const Exception&)
            {
                ::comphelper::disposeComponent(xStatement);
                return;
            }

            sal_Int16 i = 0;
            ::std::vector< ::rtl::OUString >   aStringList;
            aStringList.reserve(16);
            ::rtl::OUString aStr;
            com::sun::star::util::Date aNullDate = m_rColumn.GetParent().getNullDate();
            sal_Int32 nFormatKey = m_rColumn.GetKey();
            Reference< ::com::sun::star::util::XNumberFormatter >  xFormatter = m_rColumn.GetParent().getNumberFormatter();
            sal_Int16 nKeyType = ::comphelper::getNumberFormatType(xFormatter->getNumberFormatsSupplier()->getNumberFormats(), nFormatKey);

            while (!xListCursor->isAfterLast() && i++ < SHRT_MAX) // max anzahl eintraege
            {
                aStr = getValue(xDataField, xFormatter, aNullDate, nFormatKey, nKeyType);
                aStringList.push_back(aStr);
                xListCursor->next();
            }

            // filling the entries for the combobox
            for (::std::vector< ::rtl::OUString >::const_iterator iter = aStringList.begin();
                 iter != aStringList.end(); ++iter)
                ((ComboBox*)m_pWindow)->InsertEntry(*iter, LISTBOX_APPEND);
        }
    }
}

//------------------------------------------------------------------
IMPL_LINK( DbFilterField, OnClick, void*, EMPTYARG )
{
    TriState eState = ((CheckBoxControl*)m_pWindow)->GetBox().GetState();
    String aText;

    switch (eState)
    {
        case STATE_CHECK:
            aText.AssignAscii("1");
            break;
        case STATE_NOCHECK:
            aText.AssignAscii("0");
            break;
        case STATE_DONTKNOW:
            aText = String();
            break;
    }

    if (m_aText != aText)
    {
        m_aText = aText;
        m_aCommitLink.Call(this);
    }
    return 1;
}

/*************************************************************************/
TYPEINIT0(FmXGridCell);


DBG_NAME(FmXGridCell);
//-----------------------------------------------------------------------------
FmXGridCell::FmXGridCell(DbGridColumn* pColumn, DbCellControl* pControl)
            :OComponentHelper(m_aMutex)
            ,m_pColumn(pColumn)
            ,m_pCellControl(pControl)
{
    DBG_CTOR(FmXGridCell,NULL);

}

//-----------------------------------------------------------------------------
FmXGridCell::~FmXGridCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(FmXGridCell,NULL);
}

//------------------------------------------------------------------
void FmXGridCell::SetTextLineColor()
{
    if (m_pCellControl)
        m_pCellControl->SetTextLineColor();
}

//------------------------------------------------------------------
void FmXGridCell::SetTextLineColor(const Color& _rColor)
{
    if (m_pCellControl)
        m_pCellControl->SetTextLineColor(_rColor);
}

// XTypeProvider
//------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL FmXGridCell::getImplementationId() throw(RuntimeException)
{
    static ::cppu::OImplementationId* pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// OComponentHelper
//-----------------------------------------------------------------------------
void FmXGridCell::disposing()
{
    OComponentHelper::disposing();
    m_pColumn = NULL;
    DELETEZ(m_pCellControl);
}

//------------------------------------------------------------------
Any SAL_CALL FmXGridCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException)
{
    Any aReturn = OComponentHelper::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XControl* >(this),
            static_cast< ::com::sun::star::form::XBoundControl* >(this)
        );

    return aReturn;
}

// ::com::sun::star::awt::XControl
//-----------------------------------------------------------------------------
Reference< XInterface >  FmXGridCell::getContext() throw( RuntimeException )
{
    return Reference< XInterface > ();
}

//-----------------------------------------------------------------------------
Reference< ::com::sun::star::awt::XControlModel >  FmXGridCell::getModel() throw( ::com::sun::star::uno::RuntimeException )
{
    return Reference< ::com::sun::star::awt::XControlModel > (m_pColumn->getModel(), UNO_QUERY);
}

// ::com::sun::star::form::XBoundControl
//------------------------------------------------------------------
sal_Bool FmXGridCell::getLock() throw( RuntimeException )
{
    return m_pColumn->isLocked();
}

//------------------------------------------------------------------
void FmXGridCell::setLock(sal_Bool _bLock) throw( RuntimeException )
{
    if (getLock() == _bLock)
        return;
    else
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        m_pColumn->setLock(_bLock);
    }
}

/*************************************************************************/
TYPEINIT1(FmXDataCell, FmXGridCell);
//------------------------------------------------------------------------------
void FmXDataCell::Paint(OutputDevice& rDev, const Rectangle& rRect,
                        const Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                        const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    m_pCellControl->Paint( rDev, rRect, _rxField, xFormatter );
}

//------------------------------------------------------------------------------
void FmXDataCell::UpdateFromColumn()
{
    Reference< ::com::sun::star::sdb::XColumn >  xField(m_pColumn->GetCurrentFieldValue());
    if (xField.is())
        m_pCellControl->UpdateFromField(xField, m_pColumn->GetParent().getNumberFormatter());
}

/*************************************************************************/
TYPEINIT1(FmXTextCell, FmXDataCell);

//------------------------------------------------------------------------------
void FmXTextCell::Paint(OutputDevice& rDev,
                        const Rectangle& rRect,
                        const Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                        const Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_VCENTER;
    if (!((Window&)rDev).IsEnabled())
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
    String aText = GetText(_rxField, xFormatter, &pColor);
    if (pColor != NULL)
    {
        Font aFont = rDev.GetFont();
        Font aOldFont = aFont;
        aFont.SetColor(*pColor);
        rDev.SetFont(aFont);
        rDev.DrawText(rRect, aText, nStyle);
        rDev.SetFont(aOldFont);
    }
    else
        rDev.DrawText(rRect, aText, nStyle);
}


/*************************************************************************/

DBG_NAME(FmXEditCell);
//------------------------------------------------------------------------------
FmXEditCell::FmXEditCell(DbGridColumn* pColumn, DbCellControl* pControl)
            :FmXTextCell(pColumn, pControl)
            ,m_aTextListeners(m_aMutex)
            ,m_pEditImplementation( NULL )
            ,m_bOwnEditImplementation( false )
{
    DBG_CTOR(FmXEditCell,NULL);

    if ( pControl->ISA( DbTextField ) )
    {
        m_pEditImplementation = PTR_CAST( DbTextField, pControl )->GetEditImplementation();
    }
    else
    {
        m_pEditImplementation = new EditImplementation( *static_cast< Edit* >( pControl->GetControl() ) );
        m_bOwnEditImplementation = true;
    }

    m_pEditImplementation->SetModifyHdl( LINK( this, FmXEditCell, OnTextChanged ) );
}

//------------------------------------------------------------------
void FmXEditCell::Paint( OutputDevice& _rDev, const Rectangle& _rRect, const Reference< XColumn >& _rxField, const Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter )
{
    m_pCellControl->Paint( _rDev, _rRect, _rxField, _rxFormatter );
}

//------------------------------------------------------------------
FmXEditCell::~FmXEditCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }


    DBG_DTOR(FmXEditCell,NULL);
}

// OComponentHelper
//-----------------------------------------------------------------------------
void FmXEditCell::disposing()
{
    ::com::sun::star::lang::EventObject aEvt(*this);
    m_aTextListeners.disposeAndClear(aEvt);

    m_pEditImplementation->SetModifyHdl( Link() );
    if ( m_bOwnEditImplementation )
        delete m_pEditImplementation;
    m_pEditImplementation = NULL;

    FmXDataCell::disposing();
}

//------------------------------------------------------------------
Any SAL_CALL FmXEditCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException)
{
    Any aReturn = FmXDataCell::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XTextComponent* >(this)
        );
    return aReturn;
}

//-------------------------------------------------------------------------
Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXEditCell::getTypes(  ) throw(RuntimeException)
{
    Sequence< ::com::sun::star::uno::Type > aTypes = OComponentHelper::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 2);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< Reference< ::com::sun::star::awt::XControl >* >(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< Reference< ::com::sun::star::awt::XTextComponent >* >(NULL));

    return aTypes;
}

// ::com::sun::star::awt::XTextComponent
//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::addTextListener(const Reference< ::com::sun::star::awt::XTextListener >& l) throw( RuntimeException )
{
    m_aTextListeners.addInterface( l );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::removeTextListener(const Reference< ::com::sun::star::awt::XTextListener >& l) throw( RuntimeException )
{
    m_aTextListeners.removeInterface( l );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::setText( const ::rtl::OUString& aText ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
    {
        m_pEditImplementation->SetText( aText );

        // In JAVA wird auch ein textChanged ausgeloest, in VCL nicht.
        // ::com::sun::star::awt::Toolkit soll JAVA-komform sein...
        OnTextChanged( NULL );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::insertText(const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
    {
        m_pEditImplementation->SetSelection( Selection( rSel.Min, rSel.Max ) );
        m_pEditImplementation->ReplaceSelected( aText );
    }
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXEditCell::getText() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aText;
    if ( m_pEditImplementation )
    {
        if ( m_pEditImplementation->GetControl().IsVisible() && m_pColumn->GetParent().getDisplaySynchron())
        {
            // if the display isn't sync with the cursor we can't ask the edit field
            LineEnd eLineEndFormat = m_pColumn ? getModelLineEndSetting( m_pColumn->getModel() ) : LINEEND_LF;
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

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXEditCell::getSelectedText( void ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aText;
    if ( m_pEditImplementation )
    {
        LineEnd eLineEndFormat = m_pColumn ? getModelLineEndSetting( m_pColumn->getModel() ) : LINEEND_LF;
        aText = m_pEditImplementation->GetSelected( eLineEndFormat );
    }
    return aText;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
        m_pEditImplementation->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}

//------------------------------------------------------------------------------
::com::sun::star::awt::Selection SAL_CALL FmXEditCell::getSelection( void ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Selection aSel;
    if ( m_pEditImplementation )
        aSel = m_pEditImplementation->GetSelection();

    return ::com::sun::star::awt::Selection(aSel.Min(), aSel.Max());
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXEditCell::isEditable( void ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return ( m_pEditImplementation && !m_pEditImplementation->IsReadOnly() && m_pEditImplementation->GetControl().IsEnabled() ) ? sal_True : sal_False;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::setEditable( sal_Bool bEditable ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
        m_pEditImplementation->SetReadOnly( !bEditable );
}

//------------------------------------------------------------------------------
sal_Int16 SAL_CALL FmXEditCell::getMaxTextLen() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pEditImplementation ? m_pEditImplementation->GetMaxTextLen() : 0;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::setMaxTextLen( sal_Int16 nLen ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pEditImplementation )
        m_pEditImplementation->SetMaxTextLen( nLen );
}

//------------------------------------------------------------------------------
IMPL_LINK( FmXEditCell, OnTextChanged, void*, EMPTYARG )
{
    if ( m_pEditImplementation )
    {
        ::cppu::OInterfaceIteratorHelper aIt( m_aTextListeners );
        ::com::sun::star::awt::TextEvent aEvt;
        aEvt.Source = *this;
        while( aIt.hasMoreElements() )
            ((::com::sun::star::awt::XTextListener *)aIt.next())->textChanged( aEvt );
    }
    return 1;
}

/*************************************************************************/
//SMART_UNO_IMPLEMENTATION(FmXCheckBoxCell, FmXDataCell);


DBG_NAME(FmXCheckBoxCell);
//------------------------------------------------------------------------------
FmXCheckBoxCell::FmXCheckBoxCell(DbGridColumn* pColumn, DbCellControl* pControl)
                :FmXDataCell(pColumn, pControl)
                ,m_aItemListeners(m_aMutex)
                ,m_pBox(&((CheckBoxControl*)pControl->GetControl())->GetBox())
{
    DBG_CTOR(FmXCheckBoxCell,NULL);

    ((CheckBoxControl*)pControl->GetControl())->SetClickHdl( LINK( this, FmXCheckBoxCell, OnClick ) );
}

//------------------------------------------------------------------
FmXCheckBoxCell::~FmXCheckBoxCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(FmXCheckBoxCell,NULL);
}

// OComponentHelper
//-----------------------------------------------------------------------------
void FmXCheckBoxCell::disposing()
{
    ::com::sun::star::lang::EventObject aEvt(*this);
    m_aItemListeners.disposeAndClear(aEvt);

    ((CheckBoxControl*)m_pCellControl->GetControl())->SetClickHdl(Link());
    m_pBox = NULL;

    FmXDataCell::disposing();
}

//------------------------------------------------------------------
Any SAL_CALL FmXCheckBoxCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException)
{
    Any aReturn = FmXDataCell::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XCheckBox* >(this)
        );
    return aReturn;
}

//-------------------------------------------------------------------------
Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXCheckBoxCell::getTypes(  ) throw(RuntimeException)
{
    Sequence< ::com::sun::star::uno::Type > aTypes = OComponentHelper::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 2);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< Reference< ::com::sun::star::awt::XControl >* >(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< Reference< ::com::sun::star::awt::XCheckBox >* >(NULL));

    return aTypes;
}

//------------------------------------------------------------------
void SAL_CALL FmXCheckBoxCell::addItemListener( const Reference< ::com::sun::star::awt::XItemListener >& l ) throw( RuntimeException )
{
    m_aItemListeners.addInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXCheckBoxCell::removeItemListener( const Reference< ::com::sun::star::awt::XItemListener >& l ) throw( RuntimeException )
{
    m_aItemListeners.removeInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXCheckBoxCell::setLabel( const ::rtl::OUString& rLabel ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
    {
        UpdateFromColumn();
        m_pBox->SetText( rLabel );
    }
}

//------------------------------------------------------------------
void SAL_CALL FmXCheckBoxCell::setState( short n ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        UpdateFromColumn();
        m_pBox->SetState( (TriState)n );
    }
}

//------------------------------------------------------------------
short SAL_CALL FmXCheckBoxCell::getState() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        UpdateFromColumn();
        return m_pBox->GetState();
    }
    return STATE_DONTKNOW;
}

//------------------------------------------------------------------
void SAL_CALL FmXCheckBoxCell::enableTriState( sal_Bool b ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->EnableTriState( b );
}

//------------------------------------------------------------------
IMPL_LINK( FmXCheckBoxCell, OnClick, void*, EMPTYARG )
{
    if (m_pBox)
    {
        // check boxes are to be committed immediately (this holds for ordinary check box controls in
        // documents, and this must hold for check boxes in grid columns, too
        // 91210 - 22.08.2001 - frank.schoenheit@sun.com
        m_pCellControl->Commit();

        // notify our listeners
        ::cppu::OInterfaceIteratorHelper aIt( m_aItemListeners );

        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = *this;
        aEvent.Highlighted = sal_False;
        aEvent.Selected = m_pBox->GetState();

        while ( aIt.hasMoreElements() )
            static_cast< awt::XItemListener* >( aIt.next() )->itemStateChanged( aEvent );
    }
    return 1;
}

/*************************************************************************/

DBG_NAME(FmXListBoxCell);
//------------------------------------------------------------------------------
FmXListBoxCell::FmXListBoxCell(DbGridColumn* pColumn, DbCellControl* pControl)
               :FmXTextCell(pColumn, pControl)
               ,m_aActionListeners(m_aMutex)
               ,m_aItemListeners(m_aMutex)
               ,m_pBox((ListBox*)pControl->GetControl())
{
    DBG_CTOR(FmXListBoxCell,NULL);

    m_pBox->SetSelectHdl( LINK( this, FmXListBoxCell, OnSelect ) );
    m_pBox->SetDoubleClickHdl( LINK( this, FmXListBoxCell, OnDoubleClick ) );
}

//------------------------------------------------------------------
FmXListBoxCell::~FmXListBoxCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(FmXListBoxCell,NULL);
}

// OComponentHelper
//-----------------------------------------------------------------------------
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

//------------------------------------------------------------------
Any SAL_CALL FmXListBoxCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException)
{
    Any aReturn = FmXTextCell::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XListBox* >(this)
        );
    return aReturn;
}

//-------------------------------------------------------------------------
Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXListBoxCell::getTypes(  ) throw(RuntimeException)
{
    Sequence< ::com::sun::star::uno::Type > aTypes = OComponentHelper::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 2);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< Reference< ::com::sun::star::awt::XControl >* >(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< Reference< ::com::sun::star::awt::XListBox >* >(NULL));

    return aTypes;
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::addItemListener(const Reference< ::com::sun::star::awt::XItemListener >& l) throw( RuntimeException )
{
    m_aItemListeners.addInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::removeItemListener(const Reference< ::com::sun::star::awt::XItemListener >& l) throw( RuntimeException )
{
    m_aItemListeners.removeInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::addActionListener(const Reference< ::com::sun::star::awt::XActionListener >& l) throw( RuntimeException )
{
    m_aActionListeners.addInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::removeActionListener(const Reference< ::com::sun::star::awt::XActionListener >& l) throw( RuntimeException )
{
    m_aActionListeners.removeInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::addItem(const ::rtl::OUString& aItem, sal_Int16 nPos) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
        m_pBox->InsertEntry( aItem, nPos );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::addItems(const ::comphelper::StringSequence& aItems, sal_Int16 nPos) throw( RuntimeException )
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

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::removeItems(sal_Int16 nPos, sal_Int16 nCount) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            m_pBox->RemoveEntry( nPos + (--n) );
    }
}

//------------------------------------------------------------------
sal_Int16 SAL_CALL FmXListBoxCell::getItemCount() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pBox ? m_pBox->GetEntryCount() : 0;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXListBoxCell::getItem(sal_Int16 nPos) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    String aItem;
    if (m_pBox)
        aItem = m_pBox->GetEntry( nPos );
    return aItem;
}
//------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL FmXListBoxCell::getItems() throw( RuntimeException )
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

//------------------------------------------------------------------
sal_Int16 SAL_CALL FmXListBoxCell::getSelectedItemPos() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
    {
        UpdateFromColumn();
        return m_pBox->GetSelectEntryPos();
    }
    return 0;
}

//------------------------------------------------------------------
Sequence< sal_Int16 > SAL_CALL FmXListBoxCell::getSelectedItemsPos() throw( RuntimeException )
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
//------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXListBoxCell::getSelectedItem() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    String aItem;
    if (m_pBox)
    {
        UpdateFromColumn();
        aItem = m_pBox->GetSelectEntry();
    }

    return aItem;
}

//------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL FmXListBoxCell::getSelectedItems() throw( RuntimeException )
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

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::selectItemPos(sal_Int16 nPos, sal_Bool bSelect) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SelectEntryPos( nPos, bSelect );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::selectItemsPos(const Sequence< sal_Int16 >& aPositions, sal_Bool bSelect) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        for ( sal_uInt16 n = (sal_uInt16)aPositions.getLength(); n; )
            m_pBox->SelectEntryPos( (sal_uInt16) aPositions.getConstArray()[--n], bSelect );
    }
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::selectItem(const ::rtl::OUString& aItem, sal_Bool bSelect) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SelectEntry( aItem, bSelect );
}

//------------------------------------------------------------------
sal_Bool SAL_CALL FmXListBoxCell::isMutipleMode() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Bool bMulti = sal_False;
    if (m_pBox)
        bMulti = m_pBox->IsMultiSelectionEnabled();
    return bMulti;
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::setMultipleMode(sal_Bool bMulti) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->EnableMultiSelection( bMulti );
}

//------------------------------------------------------------------
sal_Int16 SAL_CALL FmXListBoxCell::getDropDownLineCount() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int16 nLines = 0;
    if (m_pBox)
        nLines = m_pBox->GetDropDownLineCount();

    return nLines;
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::setDropDownLineCount(sal_Int16 nLines) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SetDropDownLineCount( nLines );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::makeVisible(sal_Int16 nEntry) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SetTopEntry( nEntry );
}

//------------------------------------------------------------------
IMPL_LINK(FmXListBoxCell, OnSelect, void*, EMPTYARG )
{
    if (m_pBox)
    {
        OnDoubleClick( NULL );

        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = *this;
        aEvent.Highlighted = sal_False;

        // Bei Mehrfachselektion 0xFFFF, sonst die ID
        aEvent.Selected = (m_pBox->GetSelectEntryCount() == 1 )
            ? m_pBox->GetSelectEntryPos() : 0xFFFF;

        ::cppu::OInterfaceIteratorHelper aIt( m_aItemListeners );

        while( aIt.hasMoreElements() )
            ((::com::sun::star::awt::XItemListener *)aIt.next())->itemStateChanged( aEvent );
    }
    return 1;
}


//------------------------------------------------------------------
IMPL_LINK( FmXListBoxCell, OnDoubleClick, void*, EMPTYARG )
{
    if (m_pBox)
    {
        ::cppu::OInterfaceIteratorHelper aIt( m_aActionListeners );

        ::com::sun::star::awt::ActionEvent aEvent;
        aEvent.Source = *this;
        aEvent.ActionCommand = m_pBox->GetSelectEntry();

        while( aIt.hasMoreElements() )
            ((::com::sun::star::awt::XActionListener *)aIt.next())->actionPerformed( aEvent );
    }
    return 1;
}


/*************************************************************************/
TYPEINIT1(FmXFilterCell, FmXGridCell);

//------------------------------------------------------------------------------
Reference< XInterface >  FmXFilterCell_CreateInstance(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
{
    return *new FmXFilterCell();
}

DBG_NAME(FmXFilterCell);
//------------------------------------------------------------------------------
FmXFilterCell::FmXFilterCell(DbGridColumn* pColumn, DbCellControl* pControl)
              :FmXGridCell(pColumn, pControl)
              ,m_aTextListeners(m_aMutex)
{
    DBG_CTOR(FmXFilterCell,NULL);

    DBG_ASSERT( m_pCellControl->ISA( DbFilterField ), "FmXFilterCell::FmXFilterCell: invalid cell control!" );
    static_cast< DbFilterField* >( m_pCellControl )->SetCommitHdl( LINK( this, FmXFilterCell, OnCommit ) );
}

//------------------------------------------------------------------
FmXFilterCell::~FmXFilterCell()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(FmXFilterCell,NULL);
}

// XUnoTunnel
//------------------------------------------------------------------------------
sal_Int64 SAL_CALL FmXFilterCell::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw(RuntimeException)
{
    sal_Int64 nReturn(0);

    if  (   (_rIdentifier.getLength() == 16)
        &&  (0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), _rIdentifier.getConstArray(), 16 ))
        )
    {
        nReturn = reinterpret_cast<sal_Int64>(this);
    }

    return nReturn;
}

//------------------------------------------------------------------------------
const Sequence<sal_Int8>& FmXFilterCell::getUnoTunnelId()
{
    static Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

//------------------------------------------------------------------------------
FmXFilterCell* FmXFilterCell::getImplementation(const Reference< ::com::sun::star::awt::XControl >& _rxObject)
{
    Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(
        _rxObject, UNO_QUERY);
    if (xTunnel.is())
        return reinterpret_cast<FmXFilterCell*>(xTunnel->getSomething(getUnoTunnelId()));
    return NULL;
}

//------------------------------------------------------------------------------
void FmXFilterCell::Paint( OutputDevice& rDev, const Rectangle& rRect )
{
    static_cast< DbFilterField* >( m_pCellControl )->Paint( rDev, rRect );
}

// OComponentHelper
//-----------------------------------------------------------------------------
void FmXFilterCell::disposing()
{
    ::com::sun::star::lang::EventObject aEvt(*this);
    m_aTextListeners.disposeAndClear(aEvt);

    ((DbFilterField*)m_pCellControl)->SetCommitHdl(Link());

    FmXGridCell::disposing();
}

//------------------------------------------------------------------
Any SAL_CALL FmXFilterCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException)
{
    Any aReturn = FmXGridCell::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XTextComponent* >(this)
        );
    return aReturn;
}

//-------------------------------------------------------------------------
Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXFilterCell::getTypes(  ) throw(RuntimeException)
{
    Sequence< ::com::sun::star::uno::Type > aTypes = OComponentHelper::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 2);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< Reference< ::com::sun::star::awt::XControl >* >(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< Reference< ::com::sun::star::awt::XTextComponent >* >(NULL));

    return aTypes;
}

// ::com::sun::star::awt::XTextComponent
//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::addTextListener(const Reference< ::com::sun::star::awt::XTextListener >& l) throw( RuntimeException )
{
    m_aTextListeners.addInterface( l );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::removeTextListener(const Reference< ::com::sun::star::awt::XTextListener >& l) throw( RuntimeException )
{
    m_aTextListeners.removeInterface( l );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::setText( const ::rtl::OUString& aText ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    ((DbFilterField*)m_pCellControl)->SetText(aText);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXFilterCell::getText() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return ((DbFilterField*)m_pCellControl)->GetText();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXFilterCell::getSelectedText( void ) throw( RuntimeException )
{
    return getText();
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
::com::sun::star::awt::Selection SAL_CALL FmXFilterCell::getSelection( void ) throw( RuntimeException )
{
    return ::com::sun::star::awt::Selection();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFilterCell::isEditable( void ) throw( RuntimeException )
{
    return sal_True;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::setEditable( sal_Bool bEditable ) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
sal_Int16 SAL_CALL FmXFilterCell::getMaxTextLen() throw( RuntimeException )
{
    return 0;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::setMaxTextLen( sal_Int16 nLen ) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
IMPL_LINK( FmXFilterCell, OnCommit, void*, EMPTYARG )
{
    ::cppu::OInterfaceIteratorHelper aIt( m_aTextListeners );
    ::com::sun::star::awt::TextEvent aEvt;
    aEvt.Source = *this;
    while( aIt.hasMoreElements() )
        ((::com::sun::star::awt::XTextListener *)aIt.next())->textChanged( aEvt );
    return 1;
}

