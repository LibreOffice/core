/*************************************************************************
 *
 *  $RCSfile: gridcell.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:56:25 $
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
#ifndef _SVX_FMSTL_HXX
#include "fmstl.hxx"
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_GRIDCOLS_HXX
#include "gridcols.hxx"
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
#ifndef _SVX_DBERRBOX_HXX
#include "dbmsgbox.hxx"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _FM_IMPLEMENTATION_IDS_HXX_
#include "fmimplids.hxx"
#endif

#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif

#include <math.h>


using namespace connectivity;
// An irgendeiner Stelle dieser include-Orgie hier gehen die defines fuer WB_LEFT und WB_RIGHT verloren, und ich habe einfach
// nicht herausgefunden, wo. Also eben ein Hack.
#define WB_LEFT                 ((WinBits)0x00004000)
#define WB_CENTER               ((WinBits)0x00008000)
#define WB_RIGHT                ((WinBits)0x00010000)

//------------------------------------------------------------------------------
DbCellControllerRef DbGridColumn::s_xEmptyController;

//------------------------------------------------------------------------------
void DbGridColumn::CreateControl(sal_Int32 _nFieldPos, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xField, sal_Int32 nTypeId)
{
    Clear();

    m_nTypeId = nTypeId;
    if (xField != m_xField)
    {
        // Grundeinstellung
        m_xField = xField;
        xField->getPropertyValue(FM_PROP_FORMATKEY) >>= m_nFormatKey;
        m_nFieldPos   = (sal_Int16)_nFieldPos;
        m_bReadOnly   = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISREADONLY));
        m_bAutoValue  = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_AUTOINCREMENT));
        m_nFieldType  = ::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FIELDTYPE));

        switch (m_nFieldType)
        {
            case ::com::sun::star::sdbc::DataType::DATE:
            case ::com::sun::star::sdbc::DataType::TIME:
            case ::com::sun::star::sdbc::DataType::TIMESTAMP:
                m_bDateTime = sal_True;

            case ::com::sun::star::sdbc::DataType::BIT:
            case ::com::sun::star::sdbc::DataType::TINYINT:
            case ::com::sun::star::sdbc::DataType::SMALLINT:
            case ::com::sun::star::sdbc::DataType::INTEGER:
            case ::com::sun::star::sdbc::DataType::REAL:
            case ::com::sun::star::sdbc::DataType::BIGINT:
            case ::com::sun::star::sdbc::DataType::DOUBLE:
            case ::com::sun::star::sdbc::DataType::NUMERIC:
            case ::com::sun::star::sdbc::DataType::DECIMAL:
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
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xCur;
    if (m_rParent.getDataSource())
        xCur = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > (*m_rParent.getDataSource(), ::com::sun::star::uno::UNO_QUERY);
        // TODO : the cursor wrapper should use an ::com::sun::star::sdbc::XRowSet interface, too

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
void DbGridColumn::UpdateFromField(const DbGridRow* pRow, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (m_pCell && m_pCell->ISA(FmXFilterCell))
        PTR_CAST(FmXFilterCell, m_pCell)->Update();
    else if (pRow && pRow->IsValid() && m_nFieldPos >= 0 && m_pCell && pRow->HasField(m_nFieldPos))
    {
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField = pRow->GetField(m_nFieldPos);
        PTR_CAST(FmXDataCell, m_pCell)->UpdateFromField(xField, xFormatter);
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
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >  xComp(m_xModel, ::com::sun::star::uno::UNO_QUERY);
            if (xComp.is())
                bResult = xComp->commit();
        }
    }
    m_bInSave = sal_False;
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
                case ::com::sun::star::sdbc::DataType::NUMERIC:
                case ::com::sun::star::sdbc::DataType::DECIMAL:
                case ::com::sun::star::sdbc::DataType::DOUBLE:
                case ::com::sun::star::sdbc::DataType::REAL:
                case ::com::sun::star::sdbc::DataType::BIGINT:
                case ::com::sun::star::sdbc::DataType::INTEGER:
                case ::com::sun::star::sdbc::DataType::SMALLINT:
                case ::com::sun::star::sdbc::DataType::TINYINT:
                case ::com::sun::star::sdbc::DataType::DATE:
                case ::com::sun::star::sdbc::DataType::TIME:
                case ::com::sun::star::sdbc::DataType::TIMESTAMP:
                    _nAlign = ::com::sun::star::awt::TextAlign::RIGHT;
                    break;
                case ::com::sun::star::sdbc::DataType::BIT:
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
    if (m_pCell && m_pCell->IsAlignedController())
        m_pCell->AlignControl(m_nAlign);

    return m_nAlign;
}

//------------------------------------------------------------------------------
sal_Int16 DbGridColumn::SetAlignmentFromModel(sal_Int16 nStandardAlign)
{
    ::com::sun::star::uno::Any aAlign( m_xModel->getPropertyValue(FM_PROP_ALIGN));
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
XubString DbGridColumn::GetCellText(const DbGridRow* pRow, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const
{
    XubString aText;
    if (m_pCell && m_pCell->ISA(FmXFilterCell))
        return aText;

    if (!pRow || !pRow->IsValid())
        aText  = INVALIDTEXT;
    else if (pRow->HasField(m_nFieldPos))
    {
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField = pRow->GetField(m_nFieldPos);
        aText = GetCellText(xField, xFormatter);
    }
    return aText;
}

//------------------------------------------------------------------------------
XubString DbGridColumn::GetCellText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const
{
    XubString aText;
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
::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >  DbGridColumn::GetCurrentFieldValue() const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >  xField;
    const DbGridRowRef xRow = m_rParent.GetCurrentRow();
    if (xRow.Is() && xRow->HasField(m_nFieldPos))
    {
        xField = xRow->GetField(m_nFieldPos);
    }
    return xField;
}

//------------------------------------------------------------------------------
void DbGridColumn::Paint(OutputDevice& rDev,
                         const Rectangle& rRect,
                         const DbGridRow* pRow,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
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
            static XubString aAutoText(SVX_RES(RID_STR_AUTOFIELD));
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
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField = pRow->GetField(m_nFieldPos);
            pDataCell->Paint(rDev, rRect, xField, xFormatter);
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
    else if (m_pCell->ISA(FmXFilterCell))
        ((FmXFilterCell*)m_pCell)->Paint(rDev, rRect);
}

//------------------------------------------------------------------------------
void DbGridColumn::ImplInitSettings(Window* pParent, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground)
{
    if (m_pCell)
        m_pCell->ImplInitSettings(pParent, bFont, bForeground, bBackground);
}

//------------------------------------------------------------------------------
DbCellControl::~DbCellControl()
{
    delete m_pWindow;
    delete m_pPainter;
}

//------------------------------------------------------------------------------
void DbCellControl::ImplInitSettings(Window* pParent, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground)
{
    if (bFont)
    {
        Font aFont( pParent->IsControlFont() ? pParent->GetControlFont() : pParent->GetPointFont());
        aFont.SetTransparent(m_bTransparent);
        if (m_pPainter)
            m_pPainter->SetZoomedPointFont(aFont);
        if (m_pWindow)
            m_pWindow->SetZoomedPointFont(aFont);
    }

    if (bFont || bForeground)
    {
        Color aTextColor( pParent->IsControlForeground() ? pParent->GetControlForeground() : pParent->GetTextColor() );
        if (m_pPainter)
        {
            m_pPainter->SetTextColor(aTextColor);
            if (pParent->IsControlForeground())
                m_pPainter->SetControlForeground(aTextColor);
        }
        if (m_pWindow)
        {
            m_pWindow->SetTextColor(aTextColor);
            if (pParent->IsControlForeground())
                m_pWindow->SetControlForeground(aTextColor);
        }
    }

    if (bBackground)
    {
        if (pParent->IsControlBackground())
        {
            Color aColor( pParent->GetControlBackground());
            if (m_pPainter)
            {
                if (m_bTransparent)
                    m_pPainter->SetBackground();
                else
                {
                    m_pPainter->SetBackground(aColor);
                    m_pPainter->SetControlBackground(aColor);
                }
                m_pPainter->SetFillColor(aColor);
            }

            if (m_pWindow)
            {
                if (m_bTransparent)
                    m_pWindow->SetBackground();
                else
                {
                    m_pWindow->SetBackground(aColor);
                    m_pWindow->SetControlBackground(aColor);
                }
                m_pWindow->SetFillColor(aColor);
            }
        }
        else
        {
            if (m_pPainter)
            {
                if (m_bTransparent)
                    m_pPainter->SetBackground();
                else
                    m_pPainter->SetBackground(pParent->GetBackground());
                m_pPainter->SetFillColor(pParent->GetFillColor());
            }

            if (m_pWindow)
            {
                if (m_bTransparent)
                    m_pWindow->SetBackground(pParent->GetBackground());
                else
                    m_pWindow->SetFillColor(pParent->GetFillColor());
            }
        }
    }
}

//------------------------------------------------------------------------------
void DbCellControl::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    ImplInitSettings(pParent, sal_True ,sal_True, sal_True);
    if (IsAlignedController() && m_pWindow)
        AlignControl(m_rColumn.GetAlignment());
}

//------------------------------------------------------------------------------
void DbCellControl::AlignControl(sal_Int16 nAlignment)
{
    WinBits nStyle = m_pWindow->GetStyle();
    nStyle &= ~(WB_LEFT | WB_RIGHT | WB_CENTER);
    switch (nAlignment)
    {
        case ::com::sun::star::awt::TextAlign::RIGHT:
            nStyle |= WB_RIGHT;
            break;
        case ::com::sun::star::awt::TextAlign::CENTER:
            nStyle |= WB_CENTER;
            break;
        default:
            nStyle |= WB_LEFT;
            break;
    }
    m_pWindow->SetStyle(nStyle);
}

//------------------------------------------------------------------------------
void DbCellControl::Paint(OutputDevice& rDev, const Rectangle& rRect)
{
    m_pPainter->SetPosSizePixel(rRect.TopLeft(), rRect.GetSize());
    m_pPainter->Show();
    m_pPainter->Update();
    m_pPainter->SetParentUpdateMode( sal_False );
    m_pPainter->Hide();
    m_pPainter->SetParentUpdateMode( sal_True );
}

//------------------------------------------------------------------------------
double DbCellControl::GetValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const
{
    double fValue = 0;
    if (m_rColumn.IsNumeric())
    {
        fValue = _xVariant->getDouble();
    }
    else
    {
        sal_Bool bSuccess = sal_False;
        try
        {
            fValue = _xVariant->getDouble();
            bSuccess = sal_True;
        }
        catch(...) { }
        if (!bSuccess)
        {
            try
            {
                fValue = xFormatter->convertStringToNumber(m_rColumn.GetKey(), _xVariant->getString());
            }
            catch(...) { }
        }
    }
    return fValue;
}

/*************************************************************************/
// CellModels
/*************************************************************************/
//------------------------------------------------------------------------------
DbTextField::DbTextField(DbGridColumn& _rColumn)
            :DbCellControl(_rColumn)
            ,m_nKeyType(::com::sun::star::util::NumberFormat::TEXT)
{
}

//------------------------------------------------------------------------------
void DbTextField::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    sal_Int16 nAlignment = m_rColumn.SetAlignmentFromModel(-1);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
    sal_Bool   bReadOnly = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_READONLY));
    sal_Bool   bEnable   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_ENABLED));
    sal_Int16 nMaxLen = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_MAXTEXTLEN));
    switch (nAlignment)
    {
        case ::com::sun::star::awt::TextAlign::RIGHT:
            m_pWindow  = new Edit(pParent, WB_RIGHT);
            break;
        case ::com::sun::star::awt::TextAlign::CENTER:
            m_pWindow  = new Edit(pParent, WB_CENTER);
            break;
        default:
            m_pWindow  = new Edit(pParent, WB_LEFT);

            // Alles nur damit die Selektion bei Focuserhalt von rechts nach links geht
            AllSettings aSettings = m_pWindow->GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings);
    }

    ((Edit*)m_pWindow)->SetReadOnly(m_rColumn.IsReadOnly() || bReadOnly);
    ((Edit*)m_pWindow)->SetMaxTextLen(nMaxLen);
    m_pWindow->Enable(bEnable);

    if (m_rColumn.GetParent().getNumberFormatter().is() && m_rColumn.GetKey())
        m_nKeyType  = comphelper::getNumberFormatType(m_rColumn.GetParent().getNumberFormatter()->getNumberFormatsSupplier()->getNumberFormats(), m_rColumn.GetKey());

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbTextField::CreateController() const
{
    return new DbEditCellController((Edit*)m_pWindow);
}

//------------------------------------------------------------------------------
XubString DbTextField::GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    ::rtl::OUString aString;
    if (_xVariant.is())
        aString = ::dbtools::DBTypeConversion::getValue(_xVariant,
                                             xFormatter,
                                             m_rColumn.GetParent().getNullDate(),
                                             m_rColumn.GetKey(),
                                             m_nKeyType);
    return aString;
}

//------------------------------------------------------------------------------
void DbTextField::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    Edit* pEdit = (Edit*)m_pWindow;
    pEdit->SetText(GetFormatText(_xVariant, xFormatter));
    pEdit->SetSelection(Selection(SELECTION_MAX,SELECTION_MIN));
}

//------------------------------------------------------------------------------
sal_Bool DbTextField::Commit()
{
    ::rtl::OUString aText( m_pWindow->GetText());
    m_rColumn.getModel()->setPropertyValue(FM_PROP_TEXT, ::com::sun::star::uno::makeAny(aText));
    return sal_True;
}


DBG_NAME(DbFormattedField);
//------------------------------------------------------------------------------
DbFormattedField::DbFormattedField(DbGridColumn& _rColumn)
    :DbCellControl(_rColumn)
    ,OPropertyChangeListener(m_aDummy)
    ,m_nKeyType(::com::sun::star::util::NumberFormat::UNDEFINED)
{
    DBG_CTOR(DbFormattedField,NULL);

    // if our model's format key changes we want to propagate the new value to our windows
    m_pFormatListener = new ::comphelper::OPropertyChangeMultiplexer(this, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (_rColumn.getModel(), ::com::sun::star::uno::UNO_QUERY));
    m_pFormatListener->acquire();
    m_pFormatListener->addProperty(FM_PROP_FORMATKEY);
}

//------------------------------------------------------------------------------
DbFormattedField::~DbFormattedField()
{
    m_pFormatListener->dispose();
    m_pFormatListener->release();
    // no delete, this is done implicitly

    DBG_DTOR(DbFormattedField,NULL);
}

//------------------------------------------------------------------------------
void DbFormattedField::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    sal_Int16 nAlignment = m_rColumn.SetAlignmentFromModel(-1);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xUnoModel = m_rColumn.getModel();
    sal_Bool    bReadOnly   = ::comphelper::getBOOL(xUnoModel->getPropertyValue(FM_PROP_READONLY));
    sal_Bool    bEnable     = ::comphelper::getBOOL(xUnoModel->getPropertyValue(FM_PROP_ENABLED));
    sal_Int16   nMaxLen     = ::comphelper::getINT16(xUnoModel->getPropertyValue(FM_PROP_MAXTEXTLEN));

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

    ((Edit*)m_pWindow)->SetReadOnly(m_rColumn.IsReadOnly() || bReadOnly);
    ((Edit*)m_pWindow)->SetMaxTextLen(nMaxLen);
    ((Edit*)m_pPainter)->SetMaxTextLen(nMaxLen);
    m_pWindow->Enable(bEnable);

    ((FormattedField*)m_pWindow)->SetStrictFormat(sal_False);
    ((FormattedField*)m_pPainter)->SetStrictFormat(sal_False);
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
    ::com::sun::star::uno::Any aSupplier( xUnoModel->getPropertyValue(FM_PROP_FORMATSSUPPLIER));
    if (aSupplier.hasValue())
    {
        ::cppu::extractInterface(m_xSupplier, aSupplier);
        if (m_xSupplier.is())
        {
            // wenn wir den Supplier vom Model nehmen, dann auch den Key
            ::com::sun::star::uno::Any aFmtKey( xUnoModel->getPropertyValue(FM_PROP_FORMATKEY));
            if (aFmtKey.hasValue())
            {
                DBG_ASSERT(aFmtKey.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_LONG, "DbFormattedField::Init : invalid format key property (no sal_Int32) !");
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
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xCursorForm(xCursor, ::com::sun::star::uno::UNO_QUERY);
        if (xCursorForm.is())
        {   // wenn wir vom Cursor den Formatter nehmen, dann auch den Key vom Feld, an das wir gebunden sind
            m_xSupplier = ::dbtools::getNumberFormats(::dbtools::getConnection(xCursorForm), sal_False);

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
            ::com::sun::star::uno::Any aMin( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_MIN));
            if (aMin.getValueType().getTypeClass() != ::com::sun::star::uno::TypeClass_VOID)
            {
                DBG_ASSERT(aMin.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_DOUBLE, "DbFormattedField::Init : the model has an invalid min value !");
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
            ::com::sun::star::uno::Any aMin( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_MAX));
            if (aMin.getValueType().getTypeClass() != ::com::sun::star::uno::TypeClass_VOID)
            {
                DBG_ASSERT(aMin.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_DOUBLE, "DbFormattedField::Init : the model has an invalid max value !");
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
    ::com::sun::star::uno::Any aDefault( xUnoModel->getPropertyValue(FM_PROP_EFFECTIVE_DEFAULT));
    if (aDefault.hasValue())
    {   // das Ding kann ein double oder ein XubString sein
        switch (aDefault.getValueType().getTypeClass())
        {
            case ::com::sun::star::uno::TypeClass_DOUBLE:
                if (m_rColumn.IsNumeric())
                {
                    ((FormattedField*)m_pWindow)->SetDefaultValue(::comphelper::getDouble(aDefault));
                    ((FormattedField*)m_pPainter)->SetDefaultValue(::comphelper::getDouble(aDefault));
                }
                else
                {
                    XubString sConverted;
                    Color* pDummy;
                    pFormatterUsed->GetOutputString(::comphelper::getDouble(aDefault), 0, sConverted, &pDummy);
                    ((FormattedField*)m_pWindow)->SetDefaultText(sConverted);
                    ((FormattedField*)m_pPainter)->SetDefaultText(sConverted);
                }
                break;
            case ::com::sun::star::uno::TypeClass_STRING:
            {
                XubString sDefault( UniString( ::comphelper::getString(aDefault) ) );
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
    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbFormattedField::CreateController() const
{
    return new DbEditCellController((Edit*)m_pWindow);
}

//------------------------------------------------------------------------------
void DbFormattedField::_propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException )
{
    if (evt.PropertyName.compareTo(FM_PROP_FORMATKEY) == COMPARE_EQUAL)
    {
        sal_Int32 nNewKey = evt.NewValue.hasValue() ? ::comphelper::getINT32(evt.NewValue) : 0;
        m_nKeyType = comphelper::getNumberFormatType(m_xSupplier->getNumberFormats(), nNewKey);

        DBG_ASSERT(m_pWindow && m_pPainter, "DbFormattedField::_propertyChanged : where are my windows ?");
        if (m_pWindow)
            ((FormattedField*)m_pWindow)->SetFormatKey(nNewKey);
        if (m_pPainter)
            ((FormattedField*)m_pPainter)->SetFormatKey(nNewKey);
    }
}

//------------------------------------------------------------------------------
XubString DbFormattedField::GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    // defaultmaessig keine Farb-Angabe
    if (ppColor != NULL)
        *ppColor = NULL;

    // NULL-Wert -> leerer Text
    if (!_xVariant.is())
        return String();

    String aText;
    if (m_rColumn.IsNumeric())
    {
        // das IsNumeric an der Column sagt nichts aus ueber die Klasse des benutzen Formates, sondern
        // ueber die des an die Column gebundenen Feldes. Wenn man also eine FormattedField-Spalte an
        // ein double-Feld bindet und als Text formatiert, liefert m_rColumn.IsNumeric() sal_True. Das heisst
        // also einfach, dass ich den Inhalt der Variant mittels getDouble abfragen kann, und dann kann
        // ich den Rest (die Formatierung) dem FormattedField ueberlassen.
        double dValue = ::dbtools::DBTypeConversion::getValue(_xVariant, m_rColumn.GetParent().getNullDate(), m_nKeyType);
        if (_xVariant->wasNull())
            return aText;
        ((FormattedField*)m_pPainter)->SetValue(dValue);
    }
    else
    {
        // Hier kann ich nicht mit einem double arbeiten, da das Feld mir keines liefern kann.
        // Also einfach den Text vom ::com::sun::star::util::NumberFormatter in die richtige ::com::sun::star::form::component::Form brinden lassen.
        aText = (const sal_Unicode*)_xVariant->getString();
        if (_xVariant->wasNull())
            return aText;
        ((FormattedField*)m_pPainter)->SetTextFormatted(aText);
    }

    aText = m_pPainter->GetText();
    if (ppColor != NULL)
        *ppColor = ((FormattedField*)m_pPainter)->GetLastOutputColor();

    return aText;
}

//------------------------------------------------------------------------------
void DbFormattedField::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (!_xVariant.is())
    {   // NULL-Wert -> leerer Text
        m_pWindow->SetText(XubString());
    }
    else if (m_rColumn.IsNumeric())
    {
        // das IsNumeric an der Column sagt nichts aus ueber die Klasse des benutzen Formates, sondern
        // ueber die des an die Column gebundenen Feldes. Wenn man also eine FormattedField-Spalte an
        // ein double-Feld bindet und als Text formatiert, liefert m_rColumn.IsNumeric() sal_True. Das heisst
        // also einfach, dass ich den Inhalt der Variant mittels getDouble abfragen kann, und dann kann
        // ich den Rest (die Formatierung) dem FormattedField ueberlassen.
        double dValue = ::dbtools::DBTypeConversion::getValue(_xVariant, m_rColumn.GetParent().getNullDate(), m_nKeyType);
        if (_xVariant->wasNull())
            m_pWindow->SetText(XubString());
        else
            ((FormattedField*)m_pWindow)->SetValue(dValue);
    }
    else
    {
        // Hier kann ich nicht mit einem double arbeiten, da das Feld mir keines liefern kann.
        // Also einfach den Text vom ::com::sun::star::util::NumberFormatter in die richtige ::com::sun::star::form::component::Form brinden lassen.
        XubString sText( _xVariant->getString());
        ((FormattedField*)m_pWindow)->SetTextFormatted(sText);
    }
}

//------------------------------------------------------------------------------
sal_Bool DbFormattedField::Commit()
{
    ::com::sun::star::uno::Any aNewVal;
    FormattedField& rField = *(FormattedField*)m_pWindow;
    DBG_ASSERT(&rField == m_pWindow, "DbFormattedField::Commit : can't work with a window other than my own !");
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

//------------------------------------------------------------------------------
void DbCheckBox::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    m_bTransparent = sal_True;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
    sal_Bool bEnable  = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_ENABLED));

    m_pWindow  = new DbCheckBoxCtrl(pParent);
    m_pPainter = new DbCheckBoxCtrl(pParent);
    m_pWindow->Enable(bEnable);
    m_pWindow->SetPaintTransparent( sal_True );

    m_pPainter->SetPaintTransparent( sal_True );
    m_pPainter->SetBackground();

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbCheckBox::CreateController() const
{
    return new DbCheckBoxCellController((DbCheckBoxCtrl*)m_pWindow);
}

//------------------------------------------------------------------------------
void DbCheckBox::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    TriState eState = STATE_DONTKNOW;
    if (_xVariant.is())
    {
        if (_xVariant->getBoolean())
            eState = STATE_CHECK;
        else if (!_xVariant->wasNull())
            eState = STATE_NOCHECK;
    }
    ((DbCheckBoxCtrl*)m_pWindow)->GetBox().SetState(eState);
}

//------------------------------------------------------------------------------
void DbCheckBox::Paint(OutputDevice& rDev, const Rectangle& rRect,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    TriState eState = STATE_DONTKNOW;
    if (_xVariant.is())
    {
        if (_xVariant->getBoolean())
            eState = STATE_CHECK;
        else if (!_xVariant->wasNull())
            eState = STATE_NOCHECK;
    }
    ((DbCheckBoxCtrl*)m_pPainter)->GetBox().SetState(eState);
    DbCellControl::Paint(rDev, rRect);
}

//------------------------------------------------------------------------------
sal_Bool DbCheckBox::Commit()
{
    ::com::sun::star::uno::Any aVal;
    aVal <<= (sal_Int16) (((DbCheckBoxCtrl*)m_pWindow)->GetBox().GetState());
    m_rColumn.getModel()->setPropertyValue(FM_PROP_STATE, aVal);
    return sal_True;
}

//------------------------------------------------------------------------------
void DbPatternField::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
    sal_Bool bReadOnly = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_READONLY));
    sal_Bool bEnable   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_ENABLED));

    m_rColumn.SetAlignmentFromModel(-1);

    PatternField* pField = new PatternField(pParent, 0);
    m_pWindow = pField;
    m_pPainter= new PatternField(pParent, 0);

    ::rtl::OUString aLitMask;
    xModel->getPropertyValue(FM_PROP_LITERALMASK) >>= aLitMask;
    ::rtl::OUString aEditMask;
    xModel->getPropertyValue(FM_PROP_EDITMASK) >>= aEditMask;
    sal_Bool    bStrict   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_STRICTFORMAT));

    ByteString aAsciiListMask((const sal_Unicode*)aLitMask, RTL_TEXTENCODING_ASCII_US);
    pField->SetMask(aAsciiListMask, aEditMask);
    ((PatternField*)m_pPainter)->SetMask(aAsciiListMask, aEditMask);
    pField->SetStrictFormat(bStrict);
    ((PatternField*)m_pPainter)->SetStrictFormat(bStrict);

    pField->SetReadOnly(m_rColumn.IsReadOnly() || bReadOnly);
    pField->Enable(bEnable);

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbPatternField::CreateController() const
{
    return new DbSpinCellController((PatternField*)m_pWindow);
}

//------------------------------------------------------------------------------
XubString DbPatternField::GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    ::rtl::OUString aString;
    if (_xVariant.is())
        aString = _xVariant->getString();

    m_pPainter->SetText(aString);
    return m_pPainter->GetText();
}

//------------------------------------------------------------------------------
void DbPatternField::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    Edit* pEdit = (Edit*)m_pWindow;
    pEdit->SetText(GetFormatText(_xVariant, xFormatter));
    pEdit->SetSelection(Selection(SELECTION_MAX,SELECTION_MIN));
}

//------------------------------------------------------------------------------
sal_Bool DbPatternField::Commit()
{
    XubString aText( m_pWindow->GetText());
    m_rColumn.getModel()->setPropertyValue(FM_PROP_TEXT, ::com::sun::star::uno::makeAny(::rtl::OUString(aText)));
    return sal_True;
}

//------------------------------------------------------------------------------
void DbNumericField::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
    sal_Bool bReadOnly = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_READONLY));
    sal_Bool bEnable   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_ENABLED));

    m_rColumn.SetAlignmentFromModel(::com::sun::star::awt::TextAlign::RIGHT);

    DoubleNumericField* pField;
    if (::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_SPIN)))
    {
        pField  = new DoubleNumericField(pParent, WB_REPEAT | WB_SPIN);
        m_pPainter= new DoubleNumericField(pParent, WB_REPEAT | WB_SPIN);
    }
    else
    {
        m_pPainter= new DoubleNumericField(pParent, 0);
        pField  = new DoubleNumericField(pParent, 0);
    }
    m_pWindow = pField;

    pField->EnableEmptyField(sal_True);
    ((DoubleNumericField*)m_pPainter)->EnableEmptyField(sal_True);

    sal_Int16   nScale      = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_DECIMAL_ACCURACY));
    sal_Int32   nMin        = (sal_Int32)::comphelper::getDouble(xModel->getPropertyValue(FM_PROP_VALUEMIN));
    sal_Int32   nMax        = (sal_Int32)::comphelper::getDouble(xModel->getPropertyValue(FM_PROP_VALUEMAX));
    sal_Int32   nStep       = (sal_Int32)::comphelper::getDouble(xModel->getPropertyValue(FM_PROP_VALUESTEP));
    sal_Bool    bStrict     = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_STRICTFORMAT));
    sal_Bool    bThousand   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_SHOWTHOUSANDSEP));

    pField->SetMinValue(nMin);
    pField->SetMaxValue(nMax);
    pField->SetSpinSize(nStep);
    pField->SetStrictFormat(bStrict);
    pField->SetReadOnly(m_rColumn.IsReadOnly() || bReadOnly);
    pField->Enable(bEnable);

    ((DoubleNumericField*)m_pPainter)->SetMinValue(nMin);
    ((DoubleNumericField*)m_pPainter)->SetMaxValue(nMax);
    ((DoubleNumericField*)m_pPainter)->SetStrictFormat(bStrict);

    // dem Field und dem Painter einen Formatter spendieren
    // zuerst testen, ob ich von dem Service hinter einer Connection bekommen kann
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xForm(xCursor, ::com::sun::star::uno::UNO_QUERY);
    if (xForm.is())
        xSupplier = ::dbtools::getNumberFormats(::dbtools::getConnection(xForm), sal_True);
    SvNumberFormatter* pFormatterUsed = NULL;
    if (xSupplier.is())
    {
        SvNumberFormatsSupplierObj* pImplmentation = SvNumberFormatsSupplierObj::getImplementation(xSupplier);
        pFormatterUsed = pImplmentation ? pImplmentation->GetNumberFormatter() : NULL;
    }
    if (pFormatterUsed == NULL)
    {   // der Cursor fuehrte nicht zum Erfolg -> Standard
        pFormatterUsed = pField->StandardFormatter();
        DBG_ASSERT(pFormatterUsed != NULL, "DbNumericField::Init : no standard formatter given by the numeric field !");
    }
    pField->SetFormatter(pFormatterUsed);
    ((DoubleNumericField*)m_pPainter)->SetFormatter(pFormatterUsed);

    // und dann ein Format generieren, dass die gewuenschten Nachkommastellen usw. hat
    XubString sFormatString;
    LanguageType aAppLanguage = Application::GetAppInternational().GetLanguage();
    pFormatterUsed->GenerateFormat(sFormatString, 0, aAppLanguage, bThousand, sal_False, nScale);

    pField->SetFormat(sFormatString, aAppLanguage);
    ((DoubleNumericField*)m_pPainter)->SetFormat(sFormatString, aAppLanguage);

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbNumericField::CreateController() const
{
    return new DbSpinCellController((SpinField*)m_pWindow);
}

//------------------------------------------------------------------------------
XubString DbNumericField::GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_xVariant.is())
        return XubString();
    else
    {
        double fValue = GetValue(_xVariant, xFormatter);
        if (_xVariant->wasNull())
            return XubString();
        else
        {
            ((DoubleNumericField*)m_pPainter)->SetValue(fValue);
            return m_pPainter->GetText();
        }
    }
}

//------------------------------------------------------------------------------
void DbNumericField::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (!_xVariant.is())
        m_pWindow->SetText(XubString());
    else
    {
        double fValue = GetValue(_xVariant, xFormatter);
        if (_xVariant->wasNull())
            m_pWindow->SetText(XubString());
        else
            ((DoubleNumericField*)m_pWindow)->SetValue(fValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool DbNumericField::Commit()
{
    XubString aText( m_pWindow->GetText());
    ::com::sun::star::uno::Any aVal;

    if (aText.Len() != 0)   // nicht null
    {
        double fValue = ((DoubleNumericField*)m_pWindow)->GetValue();
        aVal <<= (double)fValue;
    }
    m_rColumn.getModel()->setPropertyValue(FM_PROP_VALUE, aVal);
    return sal_True;
}

//------------------------------------------------------------------------------
void DbCurrencyField::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
    sal_Bool bReadOnly = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_READONLY));
    sal_Bool bEnable   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_ENABLED));

    m_rColumn.SetAlignmentFromModel(::com::sun::star::awt::TextAlign::RIGHT);

    LongCurrencyField* pField;
    if (::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_SPIN)))
    {
        pField  = new LongCurrencyField(pParent, WB_REPEAT | WB_SPIN);
        m_pPainter= new LongCurrencyField(pParent, WB_REPEAT | WB_SPIN);
    }
    else
    {
        m_pPainter= new LongCurrencyField(pParent, 0);
        pField  = new LongCurrencyField(pParent, 0);
    }
    m_pWindow = pField;

    m_nScale                = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_DECIMAL_ACCURACY));
    double  nMin            = ::comphelper::getDouble(xModel->getPropertyValue(FM_PROP_VALUEMIN));
    double  nMax            = ::comphelper::getDouble(xModel->getPropertyValue(FM_PROP_VALUEMAX));
    double  nStep           = ::comphelper::getDouble(xModel->getPropertyValue(FM_PROP_VALUESTEP));
    sal_Bool    bStrict     = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_STRICTFORMAT));
    sal_Bool    bThousand   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_SHOWTHOUSANDSEP));
    ::rtl::OUString aStr( ::comphelper::getString(xModel->getPropertyValue(FM_PROP_CURRENCYSYMBOL)));

    International  aInternational( pField->GetInternational() );
    aInternational.EnableNumThousandSep(bThousand);
    aInternational.SetNumDigits(m_nScale);
    aInternational.SetCurrSymbol(aStr);
    pField->SetInternational(aInternational);
    pField->SetDecimalDigits(m_nScale);

    pField->SetFirst(nMin);
    pField->SetLast(nMax);
    pField->SetMin(nMin);
    pField->SetMax(nMax);
    pField->SetSpinSize(nStep);
    pField->SetStrictFormat(bStrict);
    pField->SetReadOnly(m_rColumn.IsReadOnly() || bReadOnly);
    pField->Enable(bEnable);

    ((LongCurrencyField*)m_pPainter)->SetInternational(aInternational);
    ((LongCurrencyField*)m_pPainter)->SetFirst(nMin);
    ((LongCurrencyField*)m_pPainter)->SetLast(nMax);
    ((LongCurrencyField*)m_pPainter)->SetMin(nMin);
    ((LongCurrencyField*)m_pPainter)->SetMax(nMax);
    ((LongCurrencyField*)m_pPainter)->SetStrictFormat(bStrict);
    ((LongCurrencyField*)m_pPainter)->SetDecimalDigits(m_nScale);

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbCurrencyField::CreateController() const
{
    return new DbSpinCellController((SpinField*)m_pWindow);
}

//------------------------------------------------------------------------------
double DbCurrencyField::GetCurrency(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const
{
    double fValue = GetValue(_xVariant, xFormatter);
    if (m_nScale)
        fValue *= pow(10.0, double(m_nScale));
    return fValue;
}

//------------------------------------------------------------------------------
XubString DbCurrencyField::GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_xVariant.is())
        return XubString();
    else
    {
        double fValue = GetCurrency(_xVariant, xFormatter);
        if (_xVariant->wasNull())
            return XubString();
        else
        {
            ((LongCurrencyField*)m_pPainter)->SetValue(fValue);
            return m_pPainter->GetText();
        }
    }
}

//------------------------------------------------------------------------------
void DbCurrencyField::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (!_xVariant.is())
        m_pWindow->SetText(XubString());
    else
    {
         double fValue = GetCurrency(_xVariant, xFormatter);
         if (_xVariant->wasNull())
             m_pWindow->SetText(XubString());
         else
            ((LongCurrencyField*)m_pWindow)->SetValue(fValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool DbCurrencyField::Commit()
{
    XubString aText( m_pWindow->GetText());
    ::com::sun::star::uno::Any aVal;
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

//------------------------------------------------------------------------------
void DbDateField::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    m_rColumn.SetAlignmentFromModel(::com::sun::star::awt::TextAlign::RIGHT);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());

    WinBits aFieldBits = 0;
    sal_Bool    bSpin       = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_SPIN));
    sal_Bool    bDropDown   = !::comphelper::hasProperty(FM_PROP_DROPDOWN, xModel) || ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_DROPDOWN));
    if (bSpin)
        aFieldBits |= WB_REPEAT | WB_SPIN;
    if (bDropDown)
        aFieldBits |= WB_DROPDOWN;

    CalendarField* pField = new CalendarField(pParent, aFieldBits);
    pField->EnableToday();
    pField->EnableNone();
    m_pPainter = new CalendarField(pParent, aFieldBits);

    m_pWindow = pField;

    sal_Int16   nFormat     = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_DATEFORMAT));
    sal_Int32   nMin        = ::comphelper::getINT32(xModel->getPropertyValue(FM_PROP_DATEMIN));
    sal_Int32   nMax        = ::comphelper::getINT32(xModel->getPropertyValue(FM_PROP_DATEMAX));
    sal_Bool    bStrict     = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_STRICTFORMAT));
    sal_Bool    bReadOnly   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_READONLY));
    sal_Bool    bEnable     = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_ENABLED));

    ::com::sun::star::uno::Any  aCentury = xModel->getPropertyValue(FM_PROP_DATE_SHOW_CENTURY);
    if (aCentury.getValueType().getTypeClass() != ::com::sun::star::uno::TypeClass_VOID)
    {
        International  aInternational( pField->GetInternational() );
        aInternational.SetDateCentury(::comphelper::getBOOL(aCentury));
        pField->SetInternational(aInternational);

        static_cast<DateField*>(m_pPainter)->SetInternational(aInternational);
    }

    pField->SetExtFormat( (ExtDateFieldFormat)nFormat);
    pField->SetMin(nMin);
    pField->SetMax(nMax);
    pField->SetStrictFormat(bStrict);
    pField->SetReadOnly(m_rColumn.IsReadOnly() || bReadOnly);
    pField->Enable(bEnable);

    static_cast<DateField*>(m_pPainter)->SetStrictFormat(bStrict);
    static_cast<DateField*>(m_pPainter)->SetMin(nMin);
    static_cast<DateField*>(m_pPainter)->SetMax(nMax);

    static_cast<DateField*>(m_pPainter)->SetExtFormat( (ExtDateFieldFormat)nFormat);

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbDateField::CreateController() const
{
    return new DbSpinCellController((SpinField*)m_pWindow);
}

//------------------------------------------------------------------------------
XubString DbDateField::GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_xVariant.is())
        return XubString();
    else
    {
        ::com::sun::star::util::Date aValue = _xVariant->getDate();
        if (_xVariant->wasNull())
            return XubString();
        else
        {
            static_cast<DateField*>(m_pPainter)->SetDate(Date(aValue.Day, aValue.Month, aValue.Year));
            return m_pPainter->GetText();
        }
    }
}

//------------------------------------------------------------------------------
void DbDateField::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (!_xVariant.is())
        m_pWindow->SetText(XubString());
    else
    {
        ::com::sun::star::util::Date aValue = _xVariant->getDate();
        if (_xVariant->wasNull())
            m_pWindow->SetText(XubString());
        else
        {
            static_cast<DateField*>(m_pWindow)->SetDate(Date(aValue.Day, aValue.Month, aValue.Year));
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool DbDateField::Commit()
{
    XubString aText( m_pWindow->GetText());
    ::com::sun::star::uno::Any aVal;
    if (aText.Len() != 0)
        aVal <<= (sal_Int32)static_cast<DateField*>(m_pWindow)->GetDate().GetDate();
    else
        aVal.clear();

    m_rColumn.getModel()->setPropertyValue(FM_PROP_DATE, aVal);
    return sal_True;
}

//------------------------------------------------------------------------------
void DbTimeField::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
    sal_Bool bReadOnly = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_READONLY));
    sal_Bool bEnable   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_ENABLED));

    m_rColumn.SetAlignmentFromModel(::com::sun::star::awt::TextAlign::LEFT);

    TimeField* pField;
    if (::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_SPIN)))
    {
        pField  = new TimeField(pParent, WB_REPEAT | WB_SPIN);
        m_pPainter= new TimeField(pParent, WB_REPEAT | WB_SPIN);
    }
    else
    {
        m_pPainter= new TimeField(pParent, 0);
        pField  = new TimeField(pParent, 0);
    }
    m_pWindow = pField;

    sal_Int16  nFormat      = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_TIMEFORMAT));
    sal_Int32   nMin        = ::comphelper::getINT32(xModel->getPropertyValue(FM_PROP_TIMEMIN));
    sal_Int32   nMax        = ::comphelper::getINT32(xModel->getPropertyValue(FM_PROP_TIMEMAX));
    sal_Bool    bStrict     = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_STRICTFORMAT));

    pField->SetExtFormat( (ExtTimeFieldFormat)nFormat);
    pField->SetMin(nMin);
    pField->SetMax(nMax);
    pField->SetStrictFormat(bStrict);
    pField->SetReadOnly(m_rColumn.IsReadOnly() || bReadOnly);
    pField->Enable(bEnable);

    static_cast<TimeField*>(m_pPainter)->SetExtFormat( (ExtTimeFieldFormat)nFormat);
    static_cast<TimeField*>(m_pPainter)->SetMin(nMin);
    static_cast<TimeField*>(m_pPainter)->SetMax(nMax);
    static_cast<TimeField*>(m_pPainter)->SetStrictFormat(bStrict);

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbTimeField::CreateController() const
{
    return new DbSpinCellController((SpinField*)m_pWindow);
}

//------------------------------------------------------------------------------
XubString DbTimeField::GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_xVariant.is())
        return XubString();
    else
    {
        ::com::sun::star::util::Time aValue = _xVariant->getTime();
        if (_xVariant->wasNull())
            return XubString();
        else
        {
            static_cast<TimeField*>(m_pPainter)->SetTime(Time(aValue.Hours, aValue.Minutes, aValue.Seconds, aValue.HundredthSeconds));
            return m_pPainter->GetText();
        }
    }
}

//------------------------------------------------------------------------------
void DbTimeField::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    if (!_xVariant.is())
        m_pWindow->SetText(XubString());
    else
    {
        ::com::sun::star::util::Time aValue = _xVariant->getTime();
        if (_xVariant->wasNull())
            m_pWindow->SetText(XubString());
        else
        {
            static_cast<TimeField*>(m_pWindow)->SetTime(Time(aValue.Hours, aValue.Minutes, aValue.Seconds, aValue.HundredthSeconds));
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool DbTimeField::Commit()
{
    XubString aText( m_pWindow->GetText());
    ::com::sun::star::uno::Any aVal;
    if (aText.Len() != 0)
        aVal <<= (sal_Int32)static_cast<TimeField*>(m_pWindow)->GetTime().GetTime();
    else
        aVal.clear();

    m_rColumn.getModel()->setPropertyValue(FM_PROP_TIME, aVal);
    return sal_True;
}

//------------------------------------------------------------------------------
DbComboBox::DbComboBox(DbGridColumn& _rColumn)
           :DbCellControl(_rColumn)
           ,OPropertyChangeListener(m_aDummy)
           ,m_nKeyType(::com::sun::star::util::NumberFormat::UNDEFINED)
{
    m_bAlignedController = sal_False;
}

//------------------------------------------------------------------------------
void DbComboBox::_propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& rEvt) throw( ::com::sun::star::uno::RuntimeException )
{
    SetList(rEvt.NewValue);
}

//------------------------------------------------------------------------------
void DbComboBox::SetList(const ::com::sun::star::uno::Any& rItems)
{
    DbComboBoxCtrl* pField = (DbComboBoxCtrl*)m_pWindow;
    pField->Clear();

    ::comphelper::StringSequence aTest;
    if (rItems >>= aTest)
    {
        const ::rtl::OUString* pStrings = aTest.getConstArray();
        sal_Int32 nItems = aTest.getLength();
        for (sal_Int32 i = 0; i < nItems; ++i, ++pStrings )
             pField->InsertEntry(*pStrings, LISTBOX_APPEND);
    }
}

//------------------------------------------------------------------------------
void DbComboBox::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
    sal_Bool bReadOnly = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_READONLY));
    sal_Bool bEnable   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_ENABLED));

    m_rColumn.SetAlignmentFromModel(::com::sun::star::awt::TextAlign::LEFT);

    DbComboBoxCtrl* pField;
    pField  = new DbComboBoxCtrl(pParent);
    m_pWindow = pField;

    // selection von rechts nach links
    AllSettings     aSettings = m_pWindow->GetSettings();
    StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
    aStyleSettings.SetSelectionOptions(
        aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
    aSettings.SetStyleSettings(aStyleSettings);
    m_pWindow->SetSettings(aSettings, sal_True);

    sal_Int16  nLines       = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_LINECOUNT));
    ::com::sun::star::uno::Any  aItems      = xModel->getPropertyValue(FM_PROP_STRINGITEMLIST);

    if (m_rColumn.GetParent().getNumberFormatter().is())
        m_nKeyType  = comphelper::getNumberFormatType(m_rColumn.GetParent().getNumberFormatter()->getNumberFormatsSupplier()->getNumberFormats(), m_rColumn.GetKey());

    SetList(aItems);

    // Am Model horchen, um Aenderungen der Stringliste mitzubekommen
    ::comphelper::OPropertyChangeMultiplexer* pMultiplexer = new ::comphelper::OPropertyChangeMultiplexer(this, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (xModel, ::com::sun::star::uno::UNO_QUERY));
    pMultiplexer->addProperty(FM_PROP_STRINGITEMLIST);

    pField->SetDropDownLineCount(nLines);
    pField->SetReadOnly(m_rColumn.IsReadOnly() || bReadOnly);
    pField->Enable(bEnable);

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbComboBox::CreateController() const
{
    return new DbComboBoxCellController((DbComboBoxCtrl*)m_pWindow);
}

//------------------------------------------------------------------------------
XubString DbComboBox::GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    ::rtl::OUString aString;
    if (_xVariant.is())
        aString = ::dbtools::DBTypeConversion::getValue(_xVariant,
                                             xFormatter,
                                             m_rColumn.GetParent().getNullDate(),
                                             m_rColumn.GetKey(),
                                             m_nKeyType);
    return aString;
}

//------------------------------------------------------------------------------
void DbComboBox::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    m_pWindow->SetText(GetFormatText(_xVariant, xFormatter));
}

//------------------------------------------------------------------------------
sal_Bool DbComboBox::Commit()
{
    XubString aText( m_pWindow->GetText());
    m_rColumn.getModel()->setPropertyValue(FM_PROP_TEXT, ::com::sun::star::uno::makeAny(::rtl::OUString(aText)));
    return sal_True;
}

//------------------------------------------------------------------------------
DbListBox::DbListBox(DbGridColumn& _rColumn)
          :DbCellControl(_rColumn)
          ,OPropertyChangeListener(m_aDummy)
          ,m_bBound(sal_False)
{
    m_bAlignedController = sal_False;
}

//------------------------------------------------------------------------------
void DbListBox::_propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& rEvt) throw( ::com::sun::star::uno::RuntimeException )
{
    SetList(rEvt.NewValue);
}

//------------------------------------------------------------------------------
void DbListBox::SetList(const ::com::sun::star::uno::Any& rItems)
{
    DbListBoxCtrl* pField = (DbListBoxCtrl*)m_pWindow;

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
        }
    }
}

//------------------------------------------------------------------------------
void DbListBox::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
    sal_Bool bEnable   = ::comphelper::getBOOL(xModel->getPropertyValue(FM_PROP_ENABLED));
    m_rColumn.SetAlignment(::com::sun::star::awt::TextAlign::LEFT);

    DbListBoxCtrl* pField;
    pField      = new DbListBoxCtrl(pParent);
    m_pWindow   = pField;

    sal_Int16  nLines   = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_LINECOUNT));
    ::com::sun::star::uno::Any  aItems  = xModel->getPropertyValue(FM_PROP_STRINGITEMLIST);

    SetList(aItems);

    // Am Model horchen, um Aenderungen der Stringliste mitzubekommen
    ::comphelper::OPropertyChangeMultiplexer* pMultiplexer = new ::comphelper::OPropertyChangeMultiplexer(this, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (xModel, ::com::sun::star::uno::UNO_QUERY));
    pMultiplexer->addProperty(FM_PROP_STRINGITEMLIST);

    pField->SetDropDownLineCount(nLines);
    pField->Enable(bEnable);

    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbListBox::CreateController() const
{
    return new DbListBoxCellController((DbListBoxCtrl*)m_pWindow);
}

//------------------------------------------------------------------------------
XubString DbListBox::GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor)
{
    if (!_xVariant.is())
        return XubString();
    else
    {
        XubString aText;
        if (m_bBound)
        {
            ::com::sun::star::uno::Sequence<sal_Int16> aPosSeq = findValue(m_aValueList, _xVariant->getString(), sal_True);
            if (aPosSeq.getLength())
                aText = static_cast<ListBox*>(m_pWindow)->GetEntry(aPosSeq.getConstArray()[0]);
        }
        else
            aText = (const sal_Unicode*)_xVariant->getString();
        return aText;
    }
}

//------------------------------------------------------------------------------
void DbListBox::UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    static_cast<ListBox*>(m_pWindow)->SelectEntry(GetFormatText(_xVariant, xFormatter));
}

//------------------------------------------------------------------------------
sal_Bool DbListBox::Commit()
{
    ::com::sun::star::uno::Any aVal;
    ::com::sun::star::uno::Sequence<sal_Int16> aSelectSeq;
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
DbFilterField::DbFilterField(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,DbGridColumn& _rColumn)
              :DbCellControl(_rColumn)
              ,m_bFilterList(sal_False)
              ,m_nControlClass(::com::sun::star::form::FormComponentType::TEXTFIELD)
              ,m_bFilterListFilled(sal_False)
              ,m_bBound(sal_False)
              ,m_aParser(_rxORB)
{
    DBG_CTOR(DbFilterField,NULL);

    m_bAlignedController = sal_False;
}

//------------------------------------------------------------------------------
DbFilterField::~DbFilterField()
{
    if (m_nControlClass == ::com::sun::star::form::FormComponentType::CHECKBOX)
        ((DbCheckBoxCtrl*)m_pWindow)->SetClickHdl( Link() );

    DBG_DTOR(DbFilterField,NULL);
}

//------------------------------------------------------------------------------
void DbFilterField::Paint(OutputDevice& rDev, const Rectangle& rRect)
{
    static sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_VCENTER | TEXT_DRAW_LEFT;
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
            DbCellControl::Paint(rDev, rRect);
            break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
            rDev.DrawText(rRect, static_cast<ListBox*>(m_pWindow)->GetSelectEntry(), nStyle);
            break;
        default:
            rDev.DrawText(rRect, m_aText, nStyle);
    }
}

//------------------------------------------------------------------------------
void DbFilterField::SetList(const ::com::sun::star::uno::Any& rItems, sal_Bool bComboBox)
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
void DbFilterField::CreateControl(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xModel)
{
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
            m_pWindow = new DbCheckBoxCtrl(pParent);
            m_pWindow->SetPaintTransparent( sal_True );
            ((DbCheckBoxCtrl*)m_pWindow)->SetClickHdl( LINK( this, DbFilterField, OnClick ) );

            m_pPainter = new DbCheckBoxCtrl(pParent);
            m_pPainter->SetPaintTransparent( sal_True );
            m_pPainter->SetBackground();
            break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
        {
            m_pWindow = new DbListBoxCtrl(pParent);
            sal_Int16  nLines       = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_LINECOUNT));
            ::com::sun::star::uno::Any  aItems      = xModel->getPropertyValue(FM_PROP_STRINGITEMLIST);
            SetList(aItems, m_nControlClass == ::com::sun::star::form::FormComponentType::COMBOBOX);
            static_cast<ListBox*>(m_pWindow)->SetDropDownLineCount(nLines);
        }   break;
        case ::com::sun::star::form::FormComponentType::COMBOBOX:
        {
            m_pWindow = new DbComboBoxCtrl(pParent);

            AllSettings     aSettings = m_pWindow->GetSettings();
            StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetSelectionOptions(
                           aStyleSettings.GetSelectionOptions() | SELECTION_OPTION_SHOWFIRST);
            aSettings.SetStyleSettings(aStyleSettings);
            m_pWindow->SetSettings(aSettings, sal_True);

            if (!m_bFilterList)
            {
                sal_Int16  nLines       = ::comphelper::getINT16(xModel->getPropertyValue(FM_PROP_LINECOUNT));
                ::com::sun::star::uno::Any  aItems      = xModel->getPropertyValue(FM_PROP_STRINGITEMLIST);
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
void DbFilterField::Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModel(m_rColumn.getModel());
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
                case ::com::sun::star::form::FormComponentType::CHECKBOX:
                case ::com::sun::star::form::FormComponentType::LISTBOX:
                case ::com::sun::star::form::FormComponentType::COMBOBOX:
                    m_nControlClass = nClassId;
                    break;
                default:
                    if (m_bFilterList)
                        m_nControlClass = ::com::sun::star::form::FormComponentType::COMBOBOX;
                    else
                        m_nControlClass = ::com::sun::star::form::FormComponentType::TEXTFIELD;
            }
        }
    }

    CreateControl(pParent, xModel);
    DbCellControl::Init(pParent, xCursor);
}

//------------------------------------------------------------------------------
DbCellControllerRef DbFilterField::CreateController() const
{
    DbCellControllerRef xController;
    switch (m_nControlClass)
    {
        case ::com::sun::star::form::FormComponentType::CHECKBOX:
            xController = new DbCheckBoxCellController((DbCheckBoxCtrl*)m_pWindow);
            break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
            xController = new DbListBoxCellController((DbListBoxCtrl*)m_pWindow);
            break;
        case ::com::sun::star::form::FormComponentType::COMBOBOX:
            xController = new DbComboBoxCellController((DbComboBoxCtrl*)m_pWindow);
            break;
        default:
            if (m_bFilterList)
                xController = new DbComboBoxCellController((DbComboBoxCtrl*)m_pWindow);
            else
                xController = new DbEditCellController((Edit*)m_pWindow);
    }
    return xController;
}

//------------------------------------------------------------------------------
sal_Bool DbFilterField::Commit()
{
    XubString aText(m_aText);
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
        XubString aNewText(aText);
        aNewText.EraseTrailingChars();
        if (aNewText.Len() != 0)
        {
            ::rtl::OUString aErrorMsg;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >  xNumberFormatter(m_rColumn.GetParent().getNumberFormatter());

            OSQLParseNode* pParseNode = m_aParser.predicateTree(aErrorMsg, aNewText,xNumberFormatter, m_rColumn.GetField());
            if (pParseNode)
            {
                ::rtl::OUString aPreparedText;

                XubString sLanguage, sCountry;
                ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);
                ::com::sun::star::lang::Locale aAppLocale(sLanguage, sCountry, ::rtl::OUString());

                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > xDataSourceRowSet(
                    *m_rColumn.GetParent().getDataSource(), ::com::sun::star::uno::UNO_QUERY);
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >  xConnection(
                    ::dbtools::getConnection(xDataSourceRowSet));

                pParseNode->parseNodeToPredicateStr(aPreparedText,
                                                    xConnection->getMetaData(),
                                                    xNumberFormatter,
                                                    m_rColumn.GetField(),aAppLocale,'.');
                delete pParseNode;
                m_aText = aPreparedText;
            }
            else
            {
                // display the error and return sal_False
                ::vos::OGuard aGuard(Application::GetSolarMutex());
                XubString aTitle( SVX_RES(RID_STR_SYNTAXERROR) );
                SvxDBMsgBox aDlg(m_pWindow->GetParent(), aTitle, aErrorMsg, WB_OK | WB_DEF_OK,
                                 SvxDBMsgBox::Info);
                aDlg.Execute();
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
void DbFilterField::SetText(const XubString& rText)
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

            ((DbCheckBoxCtrl*)m_pWindow)->GetBox().SetState(eState);
            ((DbCheckBoxCtrl*)m_pPainter)->GetBox().SetState(eState);
        }   break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
        {
            XubString aText;
            ::com::sun::star::uno::Sequence<sal_Int16> aPosSeq = findValue(m_aValueList, m_aText, sal_True);
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
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xField = m_rColumn.GetField();
        if (!xField.is())
            return;

        ::rtl::OUString aName;
        xField->getPropertyValue(FM_PROP_NAME) >>= aName;

        // the columnmodel
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xModelAsChild(m_rColumn.getModel(), ::com::sun::star::uno::UNO_QUERY);
        // the grid model
        xModelAsChild = ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild > (xModelAsChild->getParent(),::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xForm(xModelAsChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
        if (!xForm.is())
            return;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >  xConnection = ::dbtools::getConnection(xForm);
        if (!xConnection.is())
            return;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory >  xFactory(xConnection, ::com::sun::star::uno::UNO_QUERY);
        if (!xFactory.is())
        {
            DBG_ERROR("DbFilterField::Update : used the right place to request the ::com::sun::star::sdb::XSQLQueryComposerFactory interface ?");
            return;
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >  xComposer = xFactory->createQueryComposer();
        try
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xFormAsSet(xForm, ::com::sun::star::uno::UNO_QUERY);
            ::rtl::OUString sStatement;
            xFormAsSet->getPropertyValue(FM_PROP_ACTIVECOMMAND) >>= sStatement;
            xComposer->setQuery(sStatement);
        }
        catch(...)
        {
            ::comphelper::disposeComponent(xComposer);
            return;
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xComposerAsSet(xComposer, ::com::sun::star::uno::UNO_QUERY);
        if (!xComposerAsSet.is())
            return;

        // search the field
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xFieldNames;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xTablesNames;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       xComposerFieldAsSet;

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
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > xTableNameAccess;
            ::cppu::extractInterface(xTableNameAccess, xTablesNames->getByName(aTableName));
            aTableName = xTableNameAccess->getName();

            // ein Statement aufbauen und abschicken als query
            // Access to the connection
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement >  xStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >  xListCursor;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >  xDataField;

            try
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >  xMeta = xConnection->getMetaData();

                String aQuote( xMeta->getIdentifierQuoteString());
                String aStatement;
                aStatement.AssignAscii("SELECT DISTINCT ");

                aStatement += quoteName(aQuote, aName);
                if (aFieldName.len() && aName != aFieldName)
                {
                    aStatement.AppendAscii(" AS ");
                    aStatement += quoteName(aQuote, aFieldName);
                }

                aStatement.AppendAscii(" FROM ");
                aStatement += quoteTableName(xMeta, aTableName);

                xStatement = xConnection->createStatement();
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xStatementProps(xStatement, ::com::sun::star::uno::UNO_QUERY);
                xStatementProps->setPropertyValue(FM_PROP_ESCAPE_PROCESSING, ::com::sun::star::uno::makeAny((sal_Bool)sal_True));

                xListCursor = xStatement->executeQuery(aStatement);

                ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(xListCursor, ::com::sun::star::uno::UNO_QUERY);
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xFields(xSupplyCols->getColumns(), ::com::sun::star::uno::UNO_QUERY);
                ::cppu::extractInterface(xDataField, xFields->getByIndex(0));
                if (!xDataField.is())
                    return;
            }
            catch(...)
            {
                ::comphelper::disposeComponent(xStatement);
                return;
            }

            sal_Int16 i = 0;
            vector< ::rtl::OUString >   aStringList;
            aStringList.reserve(16);
            ::rtl::OUString aStr;
            com::sun::star::util::Date aNullDate = m_rColumn.GetParent().getNullDate();
            sal_Int32 nFormatKey = m_rColumn.GetKey();
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >  xFormatter = m_rColumn.GetParent().getNumberFormatter();
            sal_Int16 nKeyType = ::comphelper::getNumberFormatType(xFormatter->getNumberFormatsSupplier()->getNumberFormats(), nFormatKey);

            while (!xListCursor->isAfterLast() && i++ < SHRT_MAX) // max anzahl eintraege
            {
                aStr = ::dbtools::DBTypeConversion::getValue(xDataField,
                                                   xFormatter,
                                                   aNullDate,
                                                   nFormatKey,
                                                   nKeyType);
                aStringList.push_back(aStr);
                xListCursor->next();
            }

            // filling the entries for the combobox
            for (vector< ::rtl::OUString >::const_iterator iter = aStringList.begin();
                 iter != aStringList.end(); ++iter)
                ((ComboBox*)m_pWindow)->InsertEntry(*iter, LISTBOX_APPEND);
        }
    }
}

//------------------------------------------------------------------
IMPL_LINK( DbFilterField, OnClick, void*, EMPTYARG )
{
    TriState eState = ((DbCheckBoxCtrl*)m_pWindow)->GetBox().GetState();
    XubString aText;

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

// XTypeProvider
//------------------------------------------------------------------
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL FmXGridCell::getImplementationId() throw(::com::sun::star::uno::RuntimeException)
{
    return form::OImplementationIds::getImplementationId(getTypes());
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
::com::sun::star::uno::Any SAL_CALL FmXGridCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = OComponentHelper::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XControl* >(this),
            static_cast< ::com::sun::star::form::XBoundControl* >(this)
        );

    return aReturn;
}

// ::com::sun::star::awt::XControl
//-----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  FmXGridCell::getContext() throw( ::com::sun::star::uno::RuntimeException )
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ();
}

//-----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  FmXGridCell::getModel()
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > (m_pColumn->getModel(), ::com::sun::star::uno::UNO_QUERY);
}

// ::com::sun::star::form::XBoundControl
//------------------------------------------------------------------
sal_Bool FmXGridCell::getLock() throw( ::com::sun::star::uno::RuntimeException )
{
    return m_pColumn->isLocked();
}

//------------------------------------------------------------------
void FmXGridCell::setLock(sal_Bool _bLock) throw( ::com::sun::star::uno::RuntimeException )
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
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
{
    m_pCellControl->Paint(rDev,
                          rRect,
                          _xVariant,
                          xFormatter);
}

//------------------------------------------------------------------------------
void FmXDataCell::UpdateFromColumn()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >  xField(m_pColumn->GetCurrentFieldValue());
    if (xField.is())
        m_pCellControl->UpdateFromField(xField, m_pColumn->GetParent().getNumberFormatter());
}

/*************************************************************************/
TYPEINIT1(FmXTextCell, FmXDataCell);

//------------------------------------------------------------------------------
void FmXTextCell::Paint(OutputDevice& rDev,
                        const Rectangle& rRect,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
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
    XubString aText = GetText(_xVariant, xFormatter, &pColor);
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
SMART_UNO_IMPLEMENTATION(FmXEditCell, FmXTextCell);


DBG_NAME(FmXEditCell);
//------------------------------------------------------------------------------
FmXEditCell::FmXEditCell(DbGridColumn* pColumn, DbCellControl* pControl)
            :FmXTextCell(pColumn, pControl)
            ,m_aTextListeners(m_aMutex)
            ,m_pEdit((Edit*)pControl->GetControl())
{
    DBG_CTOR(FmXEditCell,NULL);

    // all Textfields must be drived from an Edit-Class, as we use a
    // direct cast on Edit
    m_pEdit->SetModifyHdl( LINK( this, FmXEditCell, OnTextChanged ) );
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

    m_pEdit->SetModifyHdl( Link() );
    m_pEdit = NULL;

    FmXDataCell::disposing();
}

//------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL FmXEditCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = FmXDataCell::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XTextComponent* >(this)
        );
    return aReturn;
}

//-------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXEditCell::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypes = OComponentHelper::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 2);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >* >(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >* >(NULL));

    return aTypes;
}

// ::com::sun::star::awt::XTextComponent
//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::addTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aTextListeners.addInterface( l );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::removeTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aTextListeners.removeInterface( l );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::setText( const ::rtl::OUString& aText ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pEdit)
    {
        m_pEdit->SetText(aText);

        // In JAVA wird auch ein textChanged ausgeloest, in VCL nicht.
        // ::com::sun::star::awt::Toolkit soll JAVA-komform sein...
        OnTextChanged( NULL );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::insertText(const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pEdit)
    {
        m_pEdit->SetSelection( Selection(rSel.Min, rSel.Max) );
        m_pEdit->ReplaceSelected( aText );
    }
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXEditCell::getText() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aText;
    if (m_pEdit)
    {
        if (m_pEdit->IsVisible() && m_pColumn->GetParent().getDisplaySynchron())
            // if the display isn't sync with the cursor we can't ask the edit field
            aText = m_pEdit->GetText();
        else
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >  xField(m_pColumn->GetCurrentFieldValue());
            if (xField.is())
                aText = GetText(xField, m_pColumn->GetParent().getNumberFormatter());
        }
    }
    return aText;
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXEditCell::getSelectedText( void ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aText;
    if (m_pEdit)
        aText = m_pEdit->GetSelected();
    return aText;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pEdit)
        m_pEdit->SetSelection( Selection(aSelection.Min, aSelection.Max) );
}

//------------------------------------------------------------------------------
::com::sun::star::awt::Selection SAL_CALL FmXEditCell::getSelection( void ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Selection aSel;
    if (m_pEdit)
        aSel = m_pEdit->GetSelection();

    return ::com::sun::star::awt::Selection(aSel.Min(), aSel.Max());
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXEditCell::isEditable( void ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return ( m_pEdit && !m_pEdit->IsReadOnly() && m_pEdit->IsEnabled() ) ? sal_True : sal_False;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::setEditable( sal_Bool bEditable ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pEdit)
        m_pEdit->SetReadOnly( !bEditable );
}

//------------------------------------------------------------------------------
sal_Int16 SAL_CALL FmXEditCell::getMaxTextLen() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pEdit ? m_pEdit->GetMaxTextLen() : 0;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXEditCell::setMaxTextLen( sal_Int16 nLen ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pEdit)
        m_pEdit->SetMaxTextLen( nLen );
}

//------------------------------------------------------------------------------
IMPL_LINK( FmXEditCell, OnTextChanged, void*, EMPTYARG )
{
    if (m_pEdit)
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
SMART_UNO_IMPLEMENTATION(FmXCheckBoxCell, FmXDataCell);


DBG_NAME(FmXCheckBoxCell);
//------------------------------------------------------------------------------
FmXCheckBoxCell::FmXCheckBoxCell(DbGridColumn* pColumn, DbCellControl* pControl)
                :FmXDataCell(pColumn, pControl)
                ,m_aItemListeners(m_aMutex)
                ,m_pBox(&((DbCheckBoxCtrl*)pControl->GetControl())->GetBox())
{
    DBG_CTOR(FmXCheckBoxCell,NULL);

    ((DbCheckBoxCtrl*)pControl->GetControl())->SetClickHdl( LINK( this, FmXCheckBoxCell, OnClick ) );
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

    ((DbCheckBoxCtrl*)m_pCellControl->GetControl())->SetClickHdl(Link());
    m_pBox = NULL;

    FmXDataCell::disposing();
}

//------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL FmXCheckBoxCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = FmXDataCell::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XCheckBox* >(this)
        );
    return aReturn;
}

//-------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXCheckBoxCell::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypes = OComponentHelper::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 2);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >* >(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox >* >(NULL));

    return aTypes;
}

//------------------------------------------------------------------
void SAL_CALL FmXCheckBoxCell::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aItemListeners.addInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXCheckBoxCell::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aItemListeners.removeInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXCheckBoxCell::setLabel( const ::rtl::OUString& rLabel ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
    {
        UpdateFromColumn();
        m_pBox->SetText( rLabel );
    }
}

//------------------------------------------------------------------
void SAL_CALL FmXCheckBoxCell::setState( short n ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        UpdateFromColumn();
        m_pBox->SetState( (TriState)n );
    }
}

//------------------------------------------------------------------
short SAL_CALL FmXCheckBoxCell::getState() throw( ::com::sun::star::uno::RuntimeException )
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
void SAL_CALL FmXCheckBoxCell::enableTriState( sal_Bool b ) throw( ::com::sun::star::uno::RuntimeException )
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
        ::cppu::OInterfaceIteratorHelper aIt( m_aItemListeners );

        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = *this;
        aEvent.Highlighted = sal_False;
        aEvent.Selected = m_pBox->GetState();

        while( aIt.hasMoreElements() )
            ((::com::sun::star::awt::XItemListener *)aIt.next())->itemStateChanged( aEvent );
    }
    return 1;
}

/*************************************************************************/
SMART_UNO_IMPLEMENTATION(FmXListBoxCell, FmXTextCell);


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
::com::sun::star::uno::Any SAL_CALL FmXListBoxCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = FmXTextCell::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XListBox* >(this)
        );
    return aReturn;
}

//-------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXListBoxCell::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypes = OComponentHelper::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 2);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >* >(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox >* >(NULL));

    return aTypes;
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::addItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aItemListeners.addInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::removeItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aItemListeners.removeInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::addActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aActionListeners.addInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::removeActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aActionListeners.removeInterface( l );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::addItem(const ::rtl::OUString& aItem, sal_Int16 nPos) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
        m_pBox->InsertEntry( aItem, nPos );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::addItems(const ::comphelper::StringSequence& aItems, sal_Int16 nPos) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_pBox)
    {
        sal_uInt16 nP = nPos;
        for ( sal_uInt16 n = 0; n < aItems.getLength(); n++ )
        {
            m_pBox->InsertEntry( aItems.getConstArray()[n], nP );
            if ( nPos < 0xFFFF )    // Nicht wenn 0xFFFF, weil LIST_APPEND
                nP++;
        }
    }
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::removeItems(sal_Int16 nPos, sal_Int16 nCount) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            m_pBox->RemoveEntry( nPos + (--n) );
    }
}

//------------------------------------------------------------------
sal_Int16 SAL_CALL FmXListBoxCell::getItemCount() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pBox ? m_pBox->GetEntryCount() : 0;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXListBoxCell::getItem(sal_Int16 nPos) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    XubString aItem;
    if (m_pBox)
        aItem = m_pBox->GetEntry( nPos );
    return aItem;
}
//------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL FmXListBoxCell::getItems() throw( ::com::sun::star::uno::RuntimeException )
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
sal_Int16 SAL_CALL FmXListBoxCell::getSelectedItemPos() throw( ::com::sun::star::uno::RuntimeException )
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
::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL FmXListBoxCell::getSelectedItemsPos() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    ::com::sun::star::uno::Sequence<sal_Int16> aSeq;

    if (m_pBox)
    {
        UpdateFromColumn();
        sal_uInt16 nSelEntries = m_pBox->GetSelectEntryCount();
        aSeq = ::com::sun::star::uno::Sequence<sal_Int16>( nSelEntries );
        for ( sal_uInt16 n = 0; n < nSelEntries; n++ )
            aSeq.getArray()[n] = m_pBox->GetSelectEntryPos( n );
    }
    return aSeq;
}
//------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXListBoxCell::getSelectedItem() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    XubString aItem;
    if (m_pBox)
    {
        UpdateFromColumn();
        aItem = m_pBox->GetSelectEntry();
    }

    return aItem;
}

//------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL FmXListBoxCell::getSelectedItems() throw( ::com::sun::star::uno::RuntimeException )
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
void SAL_CALL FmXListBoxCell::selectItemPos(sal_Int16 nPos, sal_Bool bSelect) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SelectEntryPos( nPos, bSelect );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::selectItemsPos(const ::com::sun::star::uno::Sequence< sal_Int16 >& aPositions, sal_Bool bSelect) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
    {
        for ( sal_uInt16 n = (sal_uInt16)aPositions.getLength(); n; )
            m_pBox->SelectEntryPos( (sal_uInt16) aPositions.getConstArray()[--n], bSelect );
    }
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::selectItem(const ::rtl::OUString& aItem, sal_Bool bSelect) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SelectEntry( aItem, bSelect );
}

//------------------------------------------------------------------
sal_Bool SAL_CALL FmXListBoxCell::isMutipleMode() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Bool bMulti = sal_False;
    if (m_pBox)
        bMulti = m_pBox->IsMultiSelectionEnabled();
    return bMulti;
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::setMultipleMode(sal_Bool bMulti) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->EnableMultiSelection( bMulti );
}

//------------------------------------------------------------------
sal_Int16 SAL_CALL FmXListBoxCell::getDropDownLineCount() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int16 nLines = 0;
    if (m_pBox)
        nLines = m_pBox->GetDropDownLineCount();

    return nLines;
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::setDropDownLineCount(sal_Int16 nLines) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_pBox)
        m_pBox->SetDropDownLineCount( nLines );
}

//------------------------------------------------------------------
void SAL_CALL FmXListBoxCell::makeVisible(sal_Int16 nEntry) throw( ::com::sun::star::uno::RuntimeException )
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
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  FmXFilterCell_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
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

    ((DbFilterField*)m_pCellControl)->SetCommitHdl(LINK( this, FmXFilterCell, OnCommit ) );
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
sal_Int64 SAL_CALL FmXFilterCell::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rIdentifier ) throw(::com::sun::star::uno::RuntimeException)
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
const ::com::sun::star::uno::Sequence<sal_Int8>& FmXFilterCell::getUnoTunnelId()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

//------------------------------------------------------------------------------
FmXFilterCell* FmXFilterCell::getImplementation(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxObject)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(
        _rxObject, ::com::sun::star::uno::UNO_QUERY);
    if (xTunnel.is())
        return reinterpret_cast<FmXFilterCell*>(xTunnel->getSomething(getUnoTunnelId()));
    return NULL;
}

//------------------------------------------------------------------------------
void FmXFilterCell::Paint(OutputDevice& rDev,
                          const Rectangle& rRect)
{
    m_pCellControl->Paint(rDev, rRect);
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
::com::sun::star::uno::Any SAL_CALL FmXFilterCell::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = FmXGridCell::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< ::com::sun::star::awt::XTextComponent* >(this)
        );
    return aReturn;
}

//-------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXFilterCell::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypes = OComponentHelper::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 2);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >* >(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >* >(NULL));

    return aTypes;
}

// ::com::sun::star::awt::XTextComponent
//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::addTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aTextListeners.addInterface( l );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::removeTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aTextListeners.removeInterface( l );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::setText( const ::rtl::OUString& aText ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    ((DbFilterField*)m_pCellControl)->SetText(aText);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw( ::com::sun::star::uno::RuntimeException )
{
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXFilterCell::getText() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return ((DbFilterField*)m_pCellControl)->GetText();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXFilterCell::getSelectedText( void ) throw( ::com::sun::star::uno::RuntimeException )
{
    return getText();
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw( ::com::sun::star::uno::RuntimeException )
{
}

//------------------------------------------------------------------------------
::com::sun::star::awt::Selection SAL_CALL FmXFilterCell::getSelection( void ) throw( ::com::sun::star::uno::RuntimeException )
{
    return ::com::sun::star::awt::Selection();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFilterCell::isEditable( void ) throw( ::com::sun::star::uno::RuntimeException )
{
    return sal_True;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::setEditable( sal_Bool bEditable ) throw( ::com::sun::star::uno::RuntimeException )
{
}

//------------------------------------------------------------------------------
sal_Int16 SAL_CALL FmXFilterCell::getMaxTextLen() throw( ::com::sun::star::uno::RuntimeException )
{
    return 0;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFilterCell::setMaxTextLen( sal_Int16 nLen ) throw( ::com::sun::star::uno::RuntimeException )
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

