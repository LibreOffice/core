/*************************************************************************
 *
 *  $RCSfile: fmgridcl.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 08:58:26 $
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
#pragma hdrstop

#ifndef _SVX_FMGRIDCL_HXX
#include "fmgridcl.hxx"
#endif
#ifndef _SVX_GRIDCELL_HXX
#include "gridcell.hxx"
#endif
#ifndef _SVX_FMURL_HXX
#include "fmurl.hxx"
#endif

#ifndef _SVX_FMPROP_HXX
#include "fmprop.hxx"
#endif

#ifndef _FMTFIELD_HXX_
#include <svtools/fmtfield.hxx>
#endif

#include <math.h>
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_DBEXCH_HRC
#include "dbexch.hrc"
#endif

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDELETEROWS_HPP_
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDDB_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDDB_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWCHANGEACTION_HPP_
#include <com/sun/star/sdb/RowChangeAction.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XCONFIRMDELETELISTENER_HPP_
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDCOLUMNFACTORY_HPP_
#include <com/sun/star/form/XGridColumnFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif

#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif

#ifndef _DTRANS_HXX //autogen
#include <so3/dtrans.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif

#ifndef _LONGCURR_HXX
#include <vcl/longcurr.hxx>
#endif

#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _SVX_GRIDCOLS_HXX
#include "gridcols.hxx"
#endif

#ifndef _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#endif

#ifndef _NUMUNO_HXX //autogen
#include <svtools/numuno.hxx>
#endif

#ifndef _SVX_FMGRIDIF_HXX
#include "fmgridif.hxx"
#endif

#ifndef _SVX_SHOWCOLS_HXX
#include "showcols.hxx"
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifdef ENABLEUNICODE
#define XUB2ANY(c)      makeAny(::rtl::OUString(c))
#endif
#ifndef _TRACE_HXX_
#include "trace.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::container;
using namespace ::cppu;
using namespace ::svxform;

static sal_uInt32 nFormat = 0;

//==============================================================================
//------------------------------------------------------------------------------
::rtl::OUString FieldServiceFromId(sal_Int32 nID)
{
    switch (nID)
    {
        case SID_FM_EDIT            : return FM_COL_TEXTFIELD;
        case SID_FM_COMBOBOX        : return FM_COL_COMBOBOX;
        case SID_FM_LISTBOX         : return FM_COL_LISTBOX;
        case SID_FM_CHECKBOX        : return FM_COL_CHECKBOX;
        case SID_FM_DATEFIELD       : return FM_COL_DATEFIELD;
        case SID_FM_TIMEFIELD       : return FM_COL_TIMEFIELD;
        case SID_FM_NUMERICFIELD    : return FM_COL_NUMERICFIELD;
        case SID_FM_CURRENCYFIELD   : return FM_COL_CURRENCYFIELD;
        case SID_FM_PATTERNFIELD    : return FM_COL_PATTERNFIELD;
        case SID_FM_FORMATTEDFIELD  : return FM_COL_FORMATTEDFIELD;
    }
    return ::rtl::OUString();
}

//==============================================================================
//------------------------------------------------------------------------------
const sal_Int16 nChangeTypeOffset = 1000;
void SetMenuItem(const ImageList& rList, sal_uInt16 nID, Menu* pMenu, Menu& rNewMenu, sal_Bool bDesignMode = sal_True, sal_Int16 nOffset = nChangeTypeOffset)
{
    pMenu->SetItemImage(nID, rList.GetImage(nID));
    pMenu->EnableItem(nID, bDesignMode);
    rNewMenu.InsertItem(nID + nOffset, pMenu->GetItemText(nID));
    rNewMenu.SetItemImage(nID + nOffset, rList.GetImage(nID));
    rNewMenu.SetHelpId(nID + nOffset, pMenu->GetHelpId(nID));
    rNewMenu.EnableItem(nID + nOffset, bDesignMode);
}

//------------------------------------------------------------------------------
FmGridHeader::FmGridHeader( BrowseBox* pParent, WinBits nWinBits)
                      :DbBrowseHeader(pParent, nWinBits)
{
    EnableDrop(sal_True);
}

//------------------------------------------------------------------------------
sal_uInt16 FmGridHeader::GetModelColumnPos(sal_uInt16 nId) const
{
    return static_cast<FmGridControl*>(GetParent())->GetModelColumnPos(nId);
}

//------------------------------------------------------------------------------
void FmGridHeader::Select()
{
    DbBrowseHeader::Select();
    if (static_cast<FmGridControl*>(GetParent())->IsDesignMode())
    {
        sal_uInt16 nPos = GetModelColumnPos(GetCurItemId());

        Reference< ::com::sun::star::container::XIndexContainer >  xColumns(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
        Reference< ::com::sun::star::beans::XPropertySet> xColumn;
        ::cppu::extractInterface(xColumn, xColumns->getByIndex(nPos));
        Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(xColumns, UNO_QUERY);
        if (xSelSupplier.is())
            xSelSupplier->select(makeAny(xColumn));
    }
}

//------------------------------------------------------------------------------
void FmGridHeader::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );
    if ( nItemId )
    {
        if ( rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON) )
        {
            Rectangle aItemRect = GetItemRect( nItemId );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();

            sal_uInt16 nPos = GetModelColumnPos(nItemId);
            Reference< ::com::sun::star::container::XIndexContainer >  xColumns(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
            try
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xColumn;
                ::cppu::extractInterface(xColumn, xColumns->getByIndex(nPos));
                ::rtl::OUString aHelpText;
                xColumn->getPropertyValue(FM_PROP_HELPTEXT) >>= aHelpText;
                if ( aHelpText.len() )
                {
                    if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aHelpText );
                    else
                        Help::ShowQuickHelp( this, aItemRect, aHelpText );
                    return;
                }
            }
            catch(...)
            {
                return;
            }
        }
    }
    DbBrowseHeader::RequestHelp( rHEvt );
}

//------------------------------------------------------------------------------
sal_Bool FmGridHeader::QueryDrop(DropEvent& rEvt)
{
    if (!nFormat)
        nFormat = Exchange::RegisterFormatName(String::CreateFromAscii(SBA_FIELDEXCHANGE_FORMAT));

    if (!static_cast<FmGridControl*>(GetParent())->IsDesignMode())
        return sal_False;

    SvDataObjectRef xDataObj = SvDataObject::PasteDragServer( rEvt );
    return xDataObj.Is() && xDataObj->GetTypeList().Get(nFormat) != NULL;
}

//------------------------------------------------------------------------------
sal_Bool FmGridHeader::Drop( const DropEvent& rEvt )
{
    if (!nFormat)
         nFormat = Exchange::RegisterFormatName(String::CreateFromAscii(SBA_FIELDEXCHANGE_FORMAT));

    if (!static_cast<FmGridControl*>(GetParent())->IsDesignMode())
        return sal_False;

    SvDataObjectRef xDataObj = SvDataObject::PasteDragServer( rEvt );
    SvData aData(nFormat);
    xDataObj->GetData( &aData );
    XubString aDataExchStr;
    if (!aData.GetData(aDataExchStr))
        return sal_False;

    // SBA_FIELDEXCHANGE_FORMAT
    // "Datenbankname";"Tabellen/QueryName";1/0(fuer Tabelle/Abfrage);"Feldname"
    String aDatabaseName    = aDataExchStr.GetToken(0,sal_Unicode(11));
    String aObjectName      = aDataExchStr.GetToken(1,sal_Unicode(11));
    sal_uInt16 nObjectType  = aDataExchStr.GetToken(2,sal_Unicode(11)).ToInt32();
    String aFieldName       = aDataExchStr.GetToken(3,sal_Unicode(11));

    if (!aFieldName.Len() || !aObjectName.Len() || !aDatabaseName.Len())
        return sal_False;

    // database, table/query and field

    Reference< XConnection >            xConnection;
    //  Reference< XDataSource >            xDataSource;
    Reference< XPreparedStatement >     xStatement;


    ::rtl::OUString sDatabaseName = aDatabaseName;
    try
    {
        //  xDataSource = dbtools::getDataSource(sDatabaseName,static_cast<FmGridControl*>(GetParent())->getServiceManager());
        xConnection = dbtools::getConnection(sDatabaseName,::rtl::OUString(),::rtl::OUString(),static_cast<FmGridControl*>(GetParent())->getServiceManager());
    }
    catch(::com::sun::star::container::NoSuchElementException&)
    {   // allowed, means aDatabaseName isn't a valid favorite name ....
    }
    catch(SQLException&)
    {   // allowed ... the env may throw an exception if it didn't find the object
    }
    if (!xConnection.is())
    {
        DBG_ERROR("FmGridHeader::Drop : could not retrieve the database access object !");
        return sal_False;
    }
    try
    {

#if DBG_UTIL
        Reference< ::com::sun::star::lang::XServiceInfo >  xServiceInfo(xConnection, UNO_QUERY);
        DBG_ASSERT(xServiceInfo.is() && xServiceInfo->supportsService(SRV_SDB_CONNECTION), "FmGridHeader::Drop : invalid connection (no database access connection !)");
#endif

        // Festellen des Feldes
        Reference< ::com::sun::star::container::XNameAccess >   xFields;
        Reference< ::com::sun::star::beans::XPropertySet >      xField;
        switch (nObjectType)
        {
            case 0: // old : DataSelectionType_TABLE:
            {
                Reference< ::com::sun::star::sdbcx::XTablesSupplier > xSupplyTables(xConnection, UNO_QUERY);
                Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyColumns;
                xSupplyTables->getTables()->getByName(aObjectName) >>= xSupplyColumns;
                xFields = xSupplyColumns->getColumns();
            }
            break;
            case 1: // old : DataSelectionType_QUERY:
            {
                Reference< ::com::sun::star::sdb::XQueriesSupplier > xSupplyQueries(xConnection, UNO_QUERY);
                Reference< ::com::sun::star::sdbcx::XColumnsSupplier > xSupplyColumns;
                xSupplyQueries->getQueries()->getByName(aObjectName) >>= xSupplyColumns;
                xFields  = xSupplyColumns->getColumns();
            }
            break;
            default:
            {
                xStatement = xConnection->prepareStatement(aObjectName);
                // not interested in any results

                Reference< ::com::sun::star::beans::XPropertySet > xStatProps(xStatement,UNO_QUERY);
                xStatProps->setPropertyValue(rtl::OUString::createFromAscii("MaxRows"), makeAny(sal_Int32(0)));

                Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(xStatement->executeQuery(), UNO_QUERY);
                if (xSupplyCols.is())
                    xFields = xSupplyCols->getColumns();
            }
        }

        if (xFields.is() && xFields->hasByName(aFieldName))
            xFields->getByName(aFieldName) >>= xField;

        Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = ::dbtools::getNumberFormats(xConnection, sal_True);

        if (!xSupplier.is() || !xField.is())
        {
            ::comphelper::disposeComponent(xStatement);
            return sal_False;
        }

        Reference< ::com::sun::star::util::XNumberFormats >  xNumberFormats(xSupplier->getNumberFormats());
        if (!xNumberFormats.is())
        {
            ::comphelper::disposeComponent(xStatement);
            return sal_False;
        }

        // Vom Feld werden nun zwei Informationen benoetigt:
        // a.) Name des Feldes fuer Label und ControlSource
        // b.) FormatKey, um festzustellen, welches Feld erzeugt werden soll
        sal_Int32 nDataType;
        xField->getPropertyValue(FM_PROP_FIELDTYPE) >>= nDataType;
        // diese Datentypen koennen im Gridcontrol nicht verarbeitet werden
        switch (nDataType)
        {
            case DataType::LONGVARBINARY:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::OTHER:
                ::comphelper::disposeComponent(xStatement);
                return sal_False;
        }

        sal_Int32 nFormatKey;
        xField->getPropertyValue(FM_PROP_FORMATKEY) >>= nFormatKey;

        // Erstellen der Column
        Reference< ::com::sun::star::container::XIndexContainer >  xCols(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
        Reference< ::com::sun::star::form::XGridColumnFactory >  xFactory(xCols, UNO_QUERY);

        Point aPos  = OutputToScreenPixel(rEvt.GetPosPixel());
        sal_uInt16 nColId = GetItemId(rEvt.GetPosPixel());
        // EinfuegePosition, immer vor der aktuellen Spalte
        sal_uInt16 nPos = GetModelColumnPos(nColId);
        Reference< ::com::sun::star::beans::XPropertySet >  xCol, xSecondCol;

        // erzeugen der Column in abhaengigkeit vom type, default textfeld
        SvULongs aPossibleTypes;
        switch (nDataType)
        {
            case DataType::BIT:
                aPossibleTypes.Insert(SID_FM_CHECKBOX, aPossibleTypes.Count());
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
                aPossibleTypes.Insert(SID_FM_NUMERICFIELD, aPossibleTypes.Count());
                aPossibleTypes.Insert(SID_FM_FORMATTEDFIELD, aPossibleTypes.Count());
                break;
            case DataType::REAL:
            case DataType::DOUBLE:
            case DataType::NUMERIC:
            case DataType::DECIMAL:
                aPossibleTypes.Insert(SID_FM_FORMATTEDFIELD, aPossibleTypes.Count());
                aPossibleTypes.Insert(SID_FM_NUMERICFIELD, aPossibleTypes.Count());
                break;
            case DataType::TIMESTAMP:
                aPossibleTypes.Insert(SID_FM_TWOFIELDS_DATE_N_TIME, aPossibleTypes.Count());
                aPossibleTypes.Insert(SID_FM_DATEFIELD, aPossibleTypes.Count());
                aPossibleTypes.Insert(SID_FM_TIMEFIELD, aPossibleTypes.Count());
                aPossibleTypes.Insert(SID_FM_FORMATTEDFIELD, aPossibleTypes.Count());
                break;
            case DataType::DATE:
                aPossibleTypes.Insert(SID_FM_DATEFIELD, aPossibleTypes.Count());
                aPossibleTypes.Insert(SID_FM_FORMATTEDFIELD, aPossibleTypes.Count());
                break;
            case DataType::TIME:
                aPossibleTypes.Insert(SID_FM_TIMEFIELD, aPossibleTypes.Count());
                aPossibleTypes.Insert(SID_FM_FORMATTEDFIELD, aPossibleTypes.Count());
                break;
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
            default:
                aPossibleTypes.Insert(SID_FM_EDIT, aPossibleTypes.Count());
                aPossibleTypes.Insert(SID_FM_FORMATTEDFIELD, aPossibleTypes.Count());
                break;
        }
        // if it's a currency field, a a "currency field" option
        try
        {
            if  (   ::comphelper::hasProperty(FM_PROP_ISCURRENCY, xField)
                &&  ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISCURRENCY)))
                aPossibleTypes.Insert(SID_FM_CURRENCYFIELD, 0);
        }
        catch(Exception&)
        {
            DBG_ERROR("FmGridHeader::Drop Exception occured!");
        }

        sal_Int32 nPreferedType = -1;
        sal_Bool bDateNTimeCol = sal_False;
        if (aPossibleTypes.Count() != 0)
        {
            nPreferedType = aPossibleTypes[0];
            if ((rEvt.GetAction() == DROP_LINK) && (aPossibleTypes.Count() > 1))
            {
                ImageList aImageList( SVX_RES(RID_SVXIMGLIST_FMEXPL) );

                PopupMenu aInsertMenu(SVX_RES(RID_SVXMNU_COLS));
                PopupMenu aTypeMenu;
                PopupMenu* pMenu = aInsertMenu.GetPopupMenu(SID_FM_INSERTCOL);
                for (sal_Int32 i=0; i<aPossibleTypes.Count(); ++i)
                    SetMenuItem(aImageList, aPossibleTypes[i], pMenu, aTypeMenu, sal_True, 0);
                nPreferedType = aTypeMenu.Execute(this, rEvt.GetPosPixel());
            }

            bDateNTimeCol = nPreferedType == SID_FM_TWOFIELDS_DATE_N_TIME;
            sal_uInt16 nColCount = bDateNTimeCol ? 2 : 1;
            ::rtl::OUString sFieldService;
            while (nColCount--)
            {
                if (bDateNTimeCol)
                    nPreferedType = nColCount ? SID_FM_DATEFIELD : SID_FM_TIMEFIELD;

                sFieldService = FieldServiceFromId(nPreferedType);
                Reference< ::com::sun::star::beans::XPropertySet >  xThisRoundCol;
                if (sFieldService.len())
                {
                    xThisRoundCol = xFactory->createColumn(sFieldService);
                    if (xThisRoundCol.is() && ::comphelper::hasProperty(FM_PROP_STRICTFORMAT, xThisRoundCol))
                        xThisRoundCol->setPropertyValue(FM_PROP_STRICTFORMAT, makeAny(sal_Bool(sal_False)));
                }
                if (nColCount)
                    xSecondCol = xThisRoundCol;
                else
                    xCol = xThisRoundCol;
            }
        }

        if (!xCol.is() || (bDateNTimeCol && !xSecondCol.is()))
        {
            ::comphelper::disposeComponent(xCol);   // in case only teh creation of the second column failed
            ::comphelper::disposeComponent(xStatement);
            return sal_False;
        }

        if (bDateNTimeCol)
        {
            XubString sPostfix(SVX_RES(RID_STR_DATETIME_LABELPOSTFIX));
            xCol->setPropertyValue(FM_PROP_LABEL, XUB2ANY(aFieldName + ::rtl::OUString(sPostfix.GetToken(1, ';'))));
            xSecondCol->setPropertyValue(FM_PROP_LABEL, XUB2ANY(aFieldName + ::rtl::OUString(sPostfix.GetToken(0, ';'))));
        }
        else
            xCol->setPropertyValue(FM_PROP_LABEL, XUB2ANY(aFieldName));

        if (nPreferedType == SID_FM_NUMERICFIELD)
        {
            // set properties for numerix field
            {
                Any aScaleVal(::comphelper::getNumberFormatDecimals(xNumberFormats, nFormatKey));
                xCol->setPropertyValue(FM_PROP_DECIMAL_ACCURACY,aScaleVal);
            }

            // set the max and min value for this field
            sal_Int32 nMinValue = 0, nMaxValue = 1000000000;
            switch (nDataType)
            {
                case DataType::TINYINT  : nMinValue = 0;            nMaxValue = 255; break;
                case DataType::SMALLINT : nMinValue = -32768;       nMaxValue = 32767; break;
                case DataType::INTEGER  : nMinValue = 0x80000000;   nMaxValue = 0x7FFFFFFF; break;
                    // um die doubles/singles kuemmere ich mich nicht, da es ein wenig sinnlos ist
                    // double and singles are ignored
            }
            xCol->setPropertyValue(FM_PROP_VALUEMIN,makeAny((double)nMinValue));
            xCol->setPropertyValue(FM_PROP_VALUEMAX,makeAny((double)nMaxValue));

            // format checking for numeric fields is default sal_True
            xCol->setPropertyValue(FM_PROP_STRICTFORMAT, bool2any(sal_True));
        }

        xCol->setPropertyValue(FM_PROP_CONTROLSOURCE, XUB2ANY(aFieldName));
        if (bDateNTimeCol)
            xSecondCol->setPropertyValue(FM_PROP_CONTROLSOURCE, XUB2ANY(aFieldName));

        if (bDateNTimeCol)
        {
            XubString sRealName,sPurePostfix;
            XubString sPostfix(SVX_RES(RID_STR_DATETIME_LABELPOSTFIX));

            for (xub_StrLen i=0; i<2; ++i)
            {
                sPurePostfix = sPostfix.GetToken(i, ';');
                sPurePostfix.EraseLeadingChars(' ');
                sPurePostfix.EraseLeadingChars('(');
                sPurePostfix.EraseTrailingChars(')');
                sRealName = aFieldName;
                sRealName += '_';
                sRealName += sPurePostfix;
                if (i)
                    xSecondCol->setPropertyValue(FM_PROP_NAME, XUB2ANY(sRealName));
                else
                    xCol->setPropertyValue(FM_PROP_NAME, XUB2ANY(sRealName));
            }
        }
        else
            xCol->setPropertyValue(FM_PROP_NAME, XUB2ANY(aFieldName));

        // jetzt einfuegen
        Any aElement;
        aElement <<= xCol;
        xCols->insertByIndex(nPos, aElement);

        if (bDateNTimeCol)
        {
            aElement <<= xSecondCol;
            xCols->insertByIndex(nPos == (sal_uInt16)-1 ? nPos : ++nPos, aElement);
        }

        // ist die ::com::sun::star::form::component::Form an die Datenbankangebunden?
        Reference< ::com::sun::star::form::XFormComponent >  xFormCp(xCols, UNO_QUERY);
        Reference< ::com::sun::star::beans::XPropertySet >  xForm(xFormCp->getParent(), UNO_QUERY);
        if (xForm.is())
        {
            if (!::comphelper::getString(xForm->getPropertyValue(FM_PROP_DATASOURCE)).getLength())
                xForm->setPropertyValue(FM_PROP_DATASOURCE, XUB2ANY(aDatabaseName));

            if (!::comphelper::getString(xForm->getPropertyValue(FM_PROP_COMMAND)).getLength())
            {
                xForm->setPropertyValue(FM_PROP_COMMAND, XUB2ANY(aObjectName));
                Any aCommandType;
                switch (nObjectType)
                {
                    case 0: // old : DataSelectionType_TABLE
                        aCommandType <<= (sal_Int32)::com::sun::star::sdb::CommandType::TABLE;
                        break;
                    case 1: // old : DataSelectionType_QUERY
                        aCommandType <<= (sal_Int32)::com::sun::star::sdb::CommandType::QUERY;
                        break;
                    default:
                        aCommandType <<= (sal_Int32)::com::sun::star::sdb::CommandType::COMMAND;
                        xForm->setPropertyValue(FM_PROP_ESCAPE_PROCESSING, bool2any((sal_Bool)(2 == nObjectType)));
                            // 2 -> old: DataSelectionType_SQL
                        break;
                }
                xForm->setPropertyValue(FM_PROP_COMMANDTYPE, aCommandType);
            }
        }
    }
    catch(...)
    {
        DBG_ERROR("FmGridHeader::Drop : catched an exception while creatin' the column !");
        ::comphelper::disposeComponent(xStatement);
        return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void FmGridHeader::PreExecuteColumnContextMenu(sal_uInt16 nColId, PopupMenu& rMenu)
{
    sal_Bool bDesignMode = static_cast<FmGridControl*>(GetParent())->IsDesignMode();

    Reference< ::com::sun::star::container::XIndexContainer >  xCols(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
    // Aufbau des Insert Menues
    // mark the column if nColId != HEADERBAR_ITEM_NOTFOUND
    if(nColId > 0)
    {
        sal_uInt16 nPos2 = GetModelColumnPos(nColId);

        Reference< ::com::sun::star::container::XIndexContainer >  xColumns(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
        Reference< ::com::sun::star::beans::XPropertySet> xColumn;
        ::cppu::extractInterface(xColumn, xColumns->getByIndex(nPos2));
        Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(xColumns, UNO_QUERY);
        if (xSelSupplier.is())
            xSelSupplier->select(makeAny(xColumn));
    }

    // EinfuegePosition, immer vor der aktuellen Spalte
    sal_uInt16 nPos = GetModelColumnPos(nColId);
    sal_Bool bMarked = nColId && static_cast<FmGridControl*>(GetParent())->isColumnMarked(nColId);

    ImageList aImageList( SVX_RES(RID_SVXIMGLIST_FMEXPL) );
    PopupMenu* pControlMenu = new PopupMenu;

    PopupMenu* pMenu = rMenu.GetPopupMenu(SID_FM_INSERTCOL);
    if (pMenu)
    {
        SetMenuItem(aImageList, SID_FM_EDIT, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_CHECKBOX, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_COMBOBOX, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_LISTBOX, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_DATEFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_TIMEFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_NUMERICFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_CURRENCYFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_PATTERNFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_FORMATTEDFIELD, pMenu, *pControlMenu, bDesignMode);
    }

    if (pMenu && xCols.is() && nColId)
    {
        Reference< ::com::sun::star::beans::XPropertySet > xSet;
        ::cppu::extractInterface(xSet, xCols->getByIndex(nPos));
        sal_Int16 nClassId;
        xSet->getPropertyValue(FM_PROP_CLASSID) >>= nClassId;

        Reference< ::com::sun::star::io::XPersistObject >  xServiceQuestion(xSet, UNO_QUERY);
        sal_Int32 nColType = xServiceQuestion.is() ? getColumnTypeByModelName(xServiceQuestion->getServiceName()) : 0;
        if (nColType == TYPE_TEXTFIELD)
        {   // edit fields and formatted fields have the same service name, thus getColumnTypeByModelName returns TYPE_TEXTFIELD
            // in both cases. And as columns don't have an ::com::sun::star::lang::XServiceInfo interface, we have to distinguish both
            // types via the existence of special properties
            Reference< ::com::sun::star::beans::XPropertySet >  xProps(xSet, UNO_QUERY);
            if (xProps.is())
            {
                Reference< ::com::sun::star::beans::XPropertySetInfo >  xPropsInfo = xProps->getPropertySetInfo();
                if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(FM_PROP_FORMATSSUPPLIER))
                    nColType = TYPE_FORMATTEDFIELD;
            }
        }

        pControlMenu->EnableItem(SID_FM_EDIT + nChangeTypeOffset, bDesignMode && (nColType != TYPE_TEXTFIELD));
        pControlMenu->EnableItem(SID_FM_COMBOBOX + nChangeTypeOffset, bDesignMode && (nColType != TYPE_COMBOBOX));
        pControlMenu->EnableItem(SID_FM_LISTBOX + nChangeTypeOffset, bDesignMode && (nColType != TYPE_LISTBOX));
        pControlMenu->EnableItem(SID_FM_CHECKBOX + nChangeTypeOffset, bDesignMode && (nColType != TYPE_CHECKBOX));
        pControlMenu->EnableItem(SID_FM_DATEFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_DATEFIELD));
        pControlMenu->EnableItem(SID_FM_NUMERICFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_NUMERICFIELD));
        pControlMenu->EnableItem(SID_FM_TIMEFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_TIMEFIELD));
        pControlMenu->EnableItem(SID_FM_CURRENCYFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_CURRENCYFIELD));
        pControlMenu->EnableItem(SID_FM_PATTERNFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_PATTERNFIELD));
        pControlMenu->EnableItem(SID_FM_FORMATTEDFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_FORMATTEDFIELD));
        rMenu.SetPopupMenu(SID_FM_CHANGECOL, pControlMenu);
    }

    rMenu.EnableItem(SID_FM_INSERTCOL, bDesignMode && xCols.is());
    rMenu.EnableItem(SID_FM_DELETECOL, bDesignMode && bMarked && xCols.is());
    rMenu.EnableItem(SID_FM_CHANGECOL, bDesignMode && bMarked && xCols.is());
    rMenu.EnableItem(SID_FM_SHOW_PROPERTY_BROWSER, bDesignMode && bMarked && xCols.is());

    PopupMenu* pShowColsMenu = rMenu.GetPopupMenu(SID_FM_SHOWCOLS);
    sal_uInt16 nHiddenCols = 0;
    if (pShowColsMenu)
    {
        if (xCols.is())
        {
            // check for hidden cols
            Reference< ::com::sun::star::beans::XPropertySet >  xCurCol;
            Any aHidden,aName;
            for (sal_uInt16 i=0; i<xCols->getCount(); ++i)
            {
                ::cppu::extractInterface(xCurCol, xCols->getByIndex(i));
                DBG_ASSERT(xCurCol.is(), "FmGridHeader::PreExecuteColumnContextMenu : the Peer has invalid columns !");
                aHidden = xCurCol->getPropertyValue(FM_PROP_HIDDEN);
                DBG_ASSERT(aHidden.getValueType().getTypeClass() == TypeClass_BOOLEAN,
                    "FmGridHeader::PreExecuteColumnContextMenu : the property 'hidden' should be boolean !");
                if (::comphelper::getBOOL(aHidden))
                {
                    // put the column name into the 'show col' menu
                    if (nHiddenCols < 16)
                    {   // (only the first 16 items to keep the menu rather small)
                        aName = xCurCol->getPropertyValue(FM_PROP_LABEL);
                        pShowColsMenu->InsertItem(nHiddenCols + 1, ::comphelper::getString(aName), 0, nHiddenCols);
                            // the ID is arbitrary, but should be unique within the whole menu
                    }
                    ++nHiddenCols;
                }
            }
        }
        pShowColsMenu->EnableItem(SID_FM_SHOWCOLS_MORE, xCols.is() && (nHiddenCols > 16));
        pShowColsMenu->EnableItem(SID_FM_SHOWALLCOLS, xCols.is() && (nHiddenCols > 0));
    }

    // allow the 'hide column' item ?
    sal_Bool bAllowHide = bMarked;                                          // a column is marked
    bAllowHide = bAllowHide || (!bDesignMode && (nPos != (sal_uInt16)-1));  // OR we are in alive mode and have hit a column
    bAllowHide = bAllowHide && xCols.is();                              // AND we have a column container
    bAllowHide = bAllowHide && (xCols->getCount()-nHiddenCols > 1);     // AND there are at least two visible columns
    rMenu.EnableItem(SID_FM_HIDECOL,  bAllowHide);

    sal_Bool bChecked = sal_False;
    if (bMarked)
    {
        SfxPoolItem* pItem = NULL;

        SfxViewFrame* pCurrentFrame = SfxViewFrame::Current();
        SfxItemState eState = SFX_ITEM_UNKNOWN;
        // ask the bindings of the current view frame (which should be the one we're residing in) for the state
        if (pCurrentFrame)
            eState = pCurrentFrame->GetBindings().QueryState(SID_FM_CTL_PROPERTIES, pItem);
        else
            DBG_ERROR("FmGridHeader::PreExecuteColumnContextMenu : no current view frame -> no bindings !");

        if (eState >= SFX_ITEM_AVAILABLE)
        {
            if (pItem)
            {
                bChecked = pItem->ISA(SfxBoolItem) && ((SfxBoolItem*)pItem)->GetValue();
                rMenu.CheckItem(SID_FM_SHOW_PROPERTY_BROWSER,bChecked);
            }
        }
        delete pItem;
    }
}

//------------------------------------------------------------------------------
void FmGridHeader::PostExecuteColumnContextMenu(sal_uInt16 nColId, const PopupMenu& rMenu, sal_uInt16 nExecutionResult)
{
    Reference< ::com::sun::star::container::XIndexContainer >  xCols(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
    sal_uInt16 nPos = GetModelColumnPos(nColId);

    // remove and delet the menu we inserted in PreExecuteColumnContextMenu
    PopupMenu* pControlMenu = rMenu.GetPopupMenu(SID_FM_CHANGECOL);
    delete pControlMenu;

    ::rtl::OUString aFieldType;
    sal_Bool    bReplace = sal_False;
    switch (nExecutionResult)
    {
        case SID_FM_DELETECOL:
        {
            Reference< XInterface >  xCol;
            ::cppu::extractInterface(xCol, xCols->getByIndex(nPos));
            xCols->removeByIndex(nPos);
            ::comphelper::disposeComponent(xCol);
        }   break;
        case SID_FM_SHOW_PROPERTY_BROWSER:
        {
            Reference< XInterface >  xCol;
            ::cppu::extractInterface(xCol, xCols->getByIndex(nPos));
            FmInterfaceItem aIFaceItem(SID_FM_SHOW_PROPERTY_BROWSER, xCol);
            SfxBoolItem aShowItem(SID_FM_SHOW_PROPERTIES, !rMenu.IsItemChecked(SID_FM_SHOW_PROPERTY_BROWSER));

            // execute the slot, use the dispatcher of the current view frame (which should be the one we're residing in)
            SfxViewFrame* pCurrentFrame = SfxViewFrame::Current();
            if (pCurrentFrame)
                pCurrentFrame->GetBindings().GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SFX_CALLMODE_ASYNCHRON,
                                          &aIFaceItem, &aShowItem, 0L );
            else
                DBG_ERROR("FmGridHeader::PostExecuteColumnContextMenu : no current view frame -> no bindings !");
        }   break;
        case SID_FM_EDIT + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_EDIT:
            aFieldType = FM_COL_TEXTFIELD;
            break;
        case SID_FM_COMBOBOX + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_COMBOBOX:
            aFieldType = FM_COL_COMBOBOX;
            break;
        case SID_FM_LISTBOX + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_LISTBOX:
            aFieldType = FM_COL_LISTBOX;
            break;
        case SID_FM_CHECKBOX + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_CHECKBOX:
            aFieldType = FM_COL_CHECKBOX;
            break;
        case SID_FM_DATEFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_DATEFIELD:
            aFieldType = FM_COL_DATEFIELD;
            break;
        case SID_FM_TIMEFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_TIMEFIELD:
            aFieldType = FM_COL_TIMEFIELD;
            break;
        case SID_FM_NUMERICFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_NUMERICFIELD:
            aFieldType = FM_COL_NUMERICFIELD;
            break;
        case SID_FM_CURRENCYFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_CURRENCYFIELD:
            aFieldType = FM_COL_CURRENCYFIELD;
            break;
        case SID_FM_PATTERNFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_PATTERNFIELD:
            aFieldType = FM_COL_PATTERNFIELD;
            break;
        case SID_FM_FORMATTEDFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_FORMATTEDFIELD:
            aFieldType = FM_COL_FORMATTEDFIELD;
            break;
        case SID_FM_HIDECOL:
        {
            Reference< ::com::sun::star::beans::XPropertySet >  xCurCol;
            ::cppu::extractInterface(xCurCol, xCols->getByIndex(nPos));
            xCurCol->setPropertyValue(FM_PROP_HIDDEN, makeAny((sal_Bool)sal_True));
        }
        break;
        case SID_FM_SHOWCOLS_MORE:
        {
            FmShowColsDialog dlg(NULL);
            dlg.SetColumns(xCols);
            dlg.Execute();
        }
        break;
        case SID_FM_SHOWALLCOLS:
        {
            // just iterate through all the cols ...
            Reference< ::com::sun::star::beans::XPropertySet >  xCurCol;
            for (sal_uInt16 i=0; i<xCols->getCount(); ++i)
            {
                ::cppu::extractInterface(xCurCol, xCols->getByIndex(i));
                xCurCol->setPropertyValue(FM_PROP_HIDDEN, makeAny((sal_Bool)sal_False));
            }
            // TODO : there must be a more clever way to do this ....
            // with the above the view is updated after every single model update ...
        }
        break;
        default:
            if (nExecutionResult>0 && nExecutionResult<=16)
            {   // it was a "show column/<colname>" command (there are at most 16 such items)
                // search the nExecutionResult'th hidden col
                Reference< ::com::sun::star::beans::XPropertySet >  xCurCol;
                for (sal_uInt16 i=0; i<xCols->getCount() && nExecutionResult; ++i)
                {
                    ::cppu::extractInterface(xCurCol, xCols->getByIndex(i));
                    Any aHidden = xCurCol->getPropertyValue(FM_PROP_HIDDEN);
                    if (::comphelper::getBOOL(aHidden))
                        if (!--nExecutionResult)
                        {
                            xCurCol->setPropertyValue(FM_PROP_HIDDEN, makeAny((sal_Bool)sal_False));
                            break;
                        }
                }
            }
            break;
    }

    if (aFieldType.len())
    {
        Reference< ::com::sun::star::form::XGridColumnFactory >  xFactory(xCols, UNO_QUERY);
        Reference< ::com::sun::star::beans::XPropertySet >  xCol = xFactory->createColumn(aFieldType);
        if (xCol.is())
        {
            Any aNew;
            aNew <<= xCol;
            if (bReplace)
            {
                // ein paar Properties hinueberretten
                Reference< ::com::sun::star::beans::XPropertySet >  xReplaced;
                ::cppu::extractInterface(xReplaced, xCols->getByIndex(nPos));

                // the application locale
                XubString sLanguage, sCountry;
                ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);
                ::com::sun::star::lang::Locale aAppLocale(sLanguage, sCountry, ::rtl::OUString());

                ::dbtools::TransferFormComponentProperties(xReplaced, xCol, aAppLocale);

                xCols->replaceByIndex(nPos, aNew);
                ::comphelper::disposeComponent(xReplaced);
            }
            else
            {
                // Standardlabel setzen
                ::rtl::OUString sLabelBase(SVX_RES(RID_STR_COLUMN));
                // disambiguate the name
                Reference< XNameAccess > xColNames(xCols, UNO_QUERY);
                ::rtl::OUString sLabel;
                for (sal_Int32 i=1; i<65535; ++i)
                {
                    sLabel = sLabelBase;
                    sLabel += ::rtl::OUString::valueOf((sal_Int32)i);
                    if (!xColNames->hasByName(sLabel))
                        break;
                }
                // no fallback in case the name is not unique (which is rather improbable) ....
                xCol->setPropertyValue(FM_PROP_LABEL, makeAny(sLabel));
                xCol->setPropertyValue(FM_PROP_NAME, makeAny(sLabel));
                xCols->insertByIndex(nPos, aNew);
            }

        }
    }
}

//------------------------------------------------------------------------------
void FmGridHeader::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            if (!rEvt.IsMouseEvent())
                return;

            sal_uInt16 nColId = GetItemId(rEvt.GetMousePosPixel());
            PopupMenu aContextMenu(SVX_RES(RID_SVXMNU_COLS));

            PreExecuteColumnContextMenu(nColId, aContextMenu);
            aContextMenu.RemoveDisabledEntries(sal_True, sal_True);
            PostExecuteColumnContextMenu(nColId, aContextMenu, aContextMenu.Execute(this, rEvt.GetMousePosPixel()));
        }
        break;
        default:
            DbBrowseHeader::Command(rEvt);
    }
}

//------------------------------------------------------------------------------
FmGridControl::FmGridControl(
                Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxFactory,
                Window* pParent,
                FmXGridPeer* _pPeer,
                WinBits nBits)
        :DbGridControl(_rxFactory, pParent, nBits)
        ,m_bInColumnMove(sal_False)
        ,m_nMarkedColumnId(BROWSER_INVALIDID)
        ,m_pPeer(_pPeer)
{
}

// ::com::sun::star::beans::XPropertyChangeListener
//------------------------------------------------------------------------------
void FmGridControl::propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
{
    if (evt.PropertyName == FM_PROP_ROWCOUNT)
    {
        // if we're not in the main thread call AdjustRows asynchronously
        implAdjustInSolarThread(sal_True);
        return;
    }

    const DbGridRowRef& xRow = GetCurrentRow();
    // waehrend Positionierung wird kein abgleich  der Properties vorgenommen
    Reference<XPropertySet> xSet(evt.Source,UNO_QUERY);
    if (xRow.Is() && (::cppu::any2bool(xSet->getPropertyValue(FM_PROP_ISNEW))|| CompareBookmark(getDataSource()->getBookmark(), xRow->GetBookmark())))
    {
        if (evt.PropertyName == FM_PROP_ISMODIFIED)
        {
            // modified or clean ?
            GridRowStatus eStatus = ::comphelper::getBOOL(evt.NewValue) ? GRS_MODIFIED : GRS_CLEAN;
            if (eStatus != xRow->GetStatus())
            {
                xRow->SetStatus(eStatus);
                vos::OGuard aGuard( Application::GetSolarMutex() );
                RowModified(GetCurrentPos());
            }
        }
    }
}

//------------------------------------------------------------------------------
void FmGridControl::SetDesignMode(sal_Bool bMode)
{
    sal_Bool bOldMode = IsDesignMode();
    DbGridControl::SetDesignMode(bMode);
    if (bOldMode != bMode)
        if (!bMode)
        {
            // selection aufheben
            markColumn(USHRT_MAX);
        }
        else
        {
            Reference< ::com::sun::star::container::XIndexContainer >  xColumns(GetPeer()->getColumns());
            Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(xColumns, UNO_QUERY);
            if (xSelSupplier.is())
            {
                Any aSelection = xSelSupplier->getSelection();
                Reference< ::com::sun::star::beans::XPropertySet >  xColumn;
                if (aSelection.getValueType().getTypeClass() == TypeClass_INTERFACE)
                    ::cppu::extractInterface(xColumn, aSelection);
                Reference< XInterface >  xCurrent;
                for (sal_uInt16 i=0; i<xColumns->getCount(); ++i)
                {
                    ::cppu::extractInterface(xCurrent, xColumns->getByIndex(i));
                    if (xCurrent == xColumn)
                    {
                        markColumn(GetColumnIdFromModelPos(i));
                        break;
                    }
                }
            }
        }
}

//------------------------------------------------------------------------------
void FmGridControl::DeleteSelectedRows()
{
    if (!m_pSeekCursor)
        return;

    // how many rows are selected?
    sal_Int32 nSelectedRows = GetSelectRowCount();
    // is the insert row selected
    if (GetEmptyRow().Is() && IsRowSelected(GetRowCount() - 1))
        nSelectedRows -= 1;

    // nothing to do
    if (nSelectedRows <= 0)
        return;

    // try to confirm the delete
    Reference< ::com::sun::star::frame::XDispatchProvider >  xDispatcher = (::com::sun::star::frame::XDispatchProvider*)GetPeer();
    if (xDispatcher.is())
    {
        ::com::sun::star::util::URL aUrl;
        aUrl.Complete = FMURL_CONFIRM_DELETION;
        Reference< ::com::sun::star::frame::XDispatch >  xDispatch = xDispatcher->queryDispatch(aUrl, rtl::OUString(), 0);
        Reference< ::com::sun::star::form::XConfirmDeleteListener >  xConfirm(xDispatch, UNO_QUERY);
        if (xConfirm.is())
        {
            ::com::sun::star::sdb::RowChangeEvent aEvent;
            aEvent.Source = (Reference< XInterface > )(*getDataSource());
            aEvent.Rows = nSelectedRows;
            aEvent.Action = ::com::sun::star::sdb::RowChangeAction::DELETE;
            if (!xConfirm->confirmDelete(aEvent))
                return;
        }
    }

    Reference< ::com::sun::star::sdbcx::XDeleteRows >  xDeleteThem((Reference< XInterface >)*getDataSource(), UNO_QUERY);

    // colect the bookmarks of the selected rows
    Sequence < Any> aBookmarks = getSelectionBookmarks();

    // determine the next row to position after deletion
    Any aBookmark;
    sal_Bool bNewPos = sal_False;
    // if the current row isn't selected we take the row as row after deletion
    if (!IsRowSelected(GetCurrentPos()) && !IsCurrentAppending())
    {
        aBookmark = GetCurrentRow()->GetBookmark();
        bNewPos   = sal_True;
    }
    else
    {
        // we look for the first row after the selected block for selection
        long nIdx = LastSelectedRow() + 1;
        if (nIdx < GetRowCount() - 1)
        {
            // there is a next row to position on
            if (SeekCursor(nIdx))
            {
                GetSeekRow()->SetState(m_pSeekCursor, sal_True);

                bNewPos = sal_True;
                // if it's not the row for inserting we keep the bookmark
                if (!IsEmptyRow(nIdx))
                    aBookmark = m_pSeekCursor->getBookmark();
            }
        }
        else
        {
            // we look for the first row before the selected block for selection after deletion
            nIdx = FirstSelectedRow() - 1;
            if (nIdx >= 0 && SeekCursor(nIdx))
            {
                GetSeekRow()->SetState(m_pSeekCursor, sal_True);

                bNewPos = sal_True;
                aBookmark = m_pSeekCursor->getBookmark();
            }
        }
    }

    // Sind alle Zeilen Selectiert
    // Zweite bedingung falls keine einguegeZeile existiert
    sal_Bool bAllSelected = GetTotalCount() == nSelectedRows || GetRowCount() == nSelectedRows;

    BeginCursorAction();

    // now delete the row
    Sequence <sal_Int32> aDeletedRows;
    try
    {
        aDeletedRows = xDeleteThem->deleteRows(aBookmarks);
    }
    catch(SQLException&)
    {
    }

    // how many rows are deleted?
    sal_Int32 nDeletedRows = 0;
    const sal_Int32* pSuccess = aDeletedRows.getConstArray();
    for (sal_Int32 i = 0; i < aDeletedRows.getLength(); i++)
    {
        if (pSuccess[i])
            nDeletedRows++;
    }

    // sind Zeilen geloescht worden?
    if (nDeletedRows)
    {
        SetUpdateMode(sal_False);
        SetNoSelection();
        try
        {
            // did we delete all the rows than try to move to the next possible row
            if (nDeletedRows == aDeletedRows.getLength())
            {
                // there exists a new position to move on
                if (bNewPos)
                {
                    if (aBookmark.hasValue())
                        getDataSource()->moveToBookmark(aBookmark);
                    // no valid bookmark so move to the insert row
                    else
                    {
                        Reference< XResultSetUpdate >  xUpdateCursor((Reference< XInterface >)*getDataSource(), UNO_QUERY);
                        xUpdateCursor->moveToInsertRow();
                    }
                }
                else
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xSet((Reference< XInterface >)*m_pDataCursor, UNO_QUERY);
                    sal_Int32 nRecordCount;
                    xSet->getPropertyValue(FM_PROP_ROWCOUNT) >>= nRecordCount;
                    // there are no rows left and we have an insert row
                    if (!nRecordCount && GetEmptyRow().Is())
                    {
                        Reference< XResultSetUpdate >  xUpdateCursor((Reference< XInterface >)*getDataSource(), UNO_QUERY);
                        xUpdateCursor->moveToInsertRow();
                    }
                    else if (nRecordCount)
                        // move to the first row
                        getDataSource()->first();
                }
            }
            // not all the rows where deleted, so move to the first row which remained in the resultset
            else
            {
                for (sal_Int32 i = 0; i < aDeletedRows.getLength(); i++)
                {
                    if (!pSuccess[i])
                    {
                        getDataSource()->moveToBookmark(aBookmarks.getConstArray()[i]);
                        break;
                    }
                }
            }
        }
        catch(...)
        {
            try
            {
                // positioning went wrong so try to move to the first row
                getDataSource()->first();
            }
            catch(...)
            {
            }
        }

        // An den DatenCursor anpassen
        AdjustDataSource(sal_True);

        // es konnten nicht alle Zeilen geloescht werden
        // da nie nicht geloeschten wieder selektieren
        if (nDeletedRows < nSelectedRows)
        {
            // waren alle selektiert
            if (bAllSelected)
            {
                SelectAll();
                if (IsEmptyRow(GetRowCount() - 1))  // einfuegeZeile nicht
                    SelectRow(GetRowCount() - 1, sal_False);
            }
            else
            {
                // select the remaining rows
                for (sal_Int32 i = 0; i < aDeletedRows.getLength(); i++)
                {
                    try
                    {
                        if (!pSuccess[i])
                        {
                            m_pSeekCursor->moveToBookmark(m_pDataCursor->getBookmark());
                            SetSeekPos(m_pSeekCursor->getRow() - 1);
                            SelectRow(GetSeekPos());
                        }
                    }
                    catch(...)
                    {
                        // keep the seekpos in all cases
                        SetSeekPos(m_pSeekCursor->getRow() - 1);
                    }
                }
            }
        }

        EndCursorAction();
        SetUpdateMode(sal_True);
    }
    else // Zeile konnte nicht geloescht werden
    {
        EndCursorAction();
        try
        {
            // currentrow is the insert row?
            if (!IsCurrentAppending())
                getDataSource()->refreshRow();
        }
        catch(...)
        {
        }
    }

    // if there is no selection anymore we can start editing
    if (!GetSelectRowCount())
        ActivateCell();
}


// XCurrentRecordListener
//------------------------------------------------------------------------------
void FmGridControl::positioned(const ::com::sun::star::lang::EventObject& rEvent)
{
    TRACE_RANGE("FmGridControl::positioned");
    // position on the data source (force it to be done in the main thread)
    implAdjustInSolarThread(sal_False);
}

//------------------------------------------------------------------------------
sal_Bool FmGridControl::commit()
{
    // Commit nur ausfuehren, wenn nicht bereits ein Update vom ::com::sun::star::form::component::GridControl ausgefuehrt
    // wird
    if (!IsUpdating())
    {
        if (Controller().Is() && Controller()->IsModified())
        {
            if (!SaveModified())
                return sal_False;
        }
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void FmGridControl::inserted(const ::com::sun::star::lang::EventObject& rEvent)
{
    const DbGridRowRef& xRow = GetCurrentRow();
    if (!xRow.Is())
        return;

    // Zeile ist eingefuegt worden, dann den status und mode zuruecksetzen
    xRow->SetState(m_pDataCursor, sal_False);
    xRow->SetNew(sal_False);

}

// XCancelUpdateRecordListener
//------------------------------------------------------------------------------
void FmGridControl::restored(const ::com::sun::star::lang::EventObject& rEvent)
{
    if (!GetCurrentRow().Is())
        return;

    sal_Bool bAppending = GetCurrentRow()->IsNew();
    sal_Bool bDirty     = GetCurrentRow()->IsModified();
    if (bAppending && (DbBrowseBox::IsModified() || bDirty))
    {
        if (Controller().Is())
            Controller()->ClearModified();

        // jetzt die Zeile herausnehmen
        RowRemoved(GetRowCount() - 1, 1, sal_True);
        GetNavigationBar().InvalidateAll();
    }

    positioned(rEvent);
}

//------------------------------------------------------------------------------
BrowserHeader* FmGridControl::imp_CreateHeaderBar(BrowseBox* pParent)
{
    return new FmGridHeader(this);
}

//------------------------------------------------------------------------------
void FmGridControl::markColumn(sal_uInt16 nId)
{
    if (GetHeaderBar() && m_nMarkedColumnId != nId)
    {
        // deselektieren
        if (m_nMarkedColumnId != BROWSER_INVALIDID)
        {
            HeaderBarItemBits aBits = GetHeaderBar()->GetItemBits(m_nMarkedColumnId) & ~HIB_FLAT;
            GetHeaderBar()->SetItemBits(m_nMarkedColumnId, aBits);
        }


        if (nId != BROWSER_INVALIDID)
        {
            HeaderBarItemBits aBits = GetHeaderBar()->GetItemBits(nId) | HIB_FLAT;
            GetHeaderBar()->SetItemBits(nId, aBits);
        }
        m_nMarkedColumnId = nId;
    }
}

//------------------------------------------------------------------------------
sal_Bool FmGridControl::isColumnMarked(sal_uInt16 nId) const
{
    return m_nMarkedColumnId == nId;
}

//------------------------------------------------------------------------------
void FmGridControl::ColumnResized(sal_uInt16 nId)
{
    DbGridControl::ColumnResized(nId);

    // Wert ans model uebergeben
    DbGridColumn* pCol = DbGridControl::GetColumns().GetObject(GetModelColumnPos(nId));
    Reference< ::com::sun::star::beans::XPropertySet >  xColModel(pCol->getModel());
    if (xColModel.is())
    {
        Any aWidth;
        sal_Int32 nColumnWidth = GetColumnWidth(nId);
        nColumnWidth = CalcReverseZoom(nColumnWidth);
        // Umrechnen in 10THMM
        aWidth <<= (sal_Int32)PixelToLogic(Point(nColumnWidth,0),MAP_10TH_MM).X();
        xColModel->setPropertyValue(FM_PROP_WIDTH, aWidth);
    }
}

//------------------------------------------------------------------------------
void FmGridControl::CellModified()
{
    DbGridControl::CellModified();
    GetPeer()->CellModified();
}

//------------------------------------------------------------------------------
void FmGridControl::BeginCursorAction()
{
    DbGridControl::BeginCursorAction();
    m_pPeer->stopCursorListening();
}

//------------------------------------------------------------------------------
void FmGridControl::EndCursorAction()
{
    m_pPeer->startCursorListening();
    DbGridControl::EndCursorAction();
}

//------------------------------------------------------------------------------
void FmGridControl::ColumnMoved(sal_uInt16 nId)
{
    m_bInColumnMove = sal_True;

    DbGridControl::ColumnMoved(nId);
    Reference< ::com::sun::star::container::XIndexContainer >  xColumns(GetPeer()->getColumns());

    if (xColumns.is())
    {
        // suchen der Spalte und verschieben im Model
        // ColumnPos holen
        DbGridColumn* pCol = DbGridControl::GetColumns().GetObject(GetModelColumnPos(nId));
        Reference< ::com::sun::star::beans::XPropertySet >  xCol;

        // Einfuegen muﬂ sich an den Column Positionen orientieren
        sal_uInt32 i;
        Reference< XInterface > xCurrent;
        for (i = 0; !xCol.is() && i < xColumns->getCount(); i++)
        {
            ::cppu::extractInterface(xCurrent, xColumns->getByIndex(i));
            if (xCurrent == pCol->getModel())
            {
                xCol = pCol->getModel();
                break;
            }
        }

        DBG_ASSERT(i < xColumns->getCount(), "Falscher ::com::sun::star::sdbcx::Index");
        xColumns->removeByIndex(i);
        Any aElement;
        aElement <<= xCol;
        xColumns->insertByIndex(GetModelColumnPos(nId), aElement);
        pCol->setModel(xCol);
    }

    m_bInColumnMove = sal_False;
}

//------------------------------------------------------------------------------
void FmGridControl::InitColumnsByModels(const Reference< ::com::sun::star::container::XIndexContainer >& xColumns)
{
    // Spalten wieder neu setzen
    // wenn es nur eine HandleColumn gibt, dann nicht
    if (GetModelColCount())
    {
        RemoveColumns();
        InsertHandleColumn();
    }

    if (!xColumns.is())
        return;

    SetUpdateMode(sal_False);

    // Einfuegen muﬂ sich an den Column Positionen orientieren
    sal_uInt32 i;
    XubString aName;
    Any aWidth;
    for (i = 0; i < xColumns->getCount(); ++i)
    {
        Reference< ::com::sun::star::beans::XPropertySet > xCol;
        ::cppu::extractInterface(xCol, xColumns->getByIndex(i));

        Reference< XPropertySetInfo > xPropsInfo = xCol->getPropertySetInfo();
        sal_Bool bHas = xPropsInfo->hasPropertyByName(FM_PROP_LABEL);

        aName  = (const sal_Unicode*)::comphelper::getString(xCol->getPropertyValue(FM_PROP_LABEL));

        aWidth = xCol->getPropertyValue(FM_PROP_WIDTH);
        sal_Int32 nWidth = 0;
        if (aWidth >>= nWidth)
            nWidth = LogicToPixel(Point(nWidth,0),MAP_10TH_MM).X();

        AppendColumn(aName, nWidth);
        DbGridColumn* pCol = DbGridControl::GetColumns().GetObject(i);
        pCol->setModel(xCol);
    }

    // und jetzt noch die hidden columns rausnehmen
    // (wir haben das nicht gleich in der oberen Schleife gemacht, da wir dann Probleme mit den
    // IDs der Spalten bekommen haetten : AppendColumn vergibt die automatisch, die Spalte _nach_
    // einer versteckten braucht aber eine um eine erhoehte ID ....
    Any aHidden;
    for (i = 0; i < xColumns->getCount(); ++i)
    {
        Reference< ::com::sun::star::beans::XPropertySet > xCol;
        ::cppu::extractInterface(xCol, xColumns->getByIndex(i));
        aHidden = xCol->getPropertyValue(FM_PROP_HIDDEN);
        if (::comphelper::getBOOL(aHidden))
            HideColumn(GetColumnIdFromModelPos(i));
    }

    SetUpdateMode(sal_True);
}

//------------------------------------------------------------------------------
void FmGridControl::InitColumnsByFields(const Reference< ::com::sun::star::container::XIndexAccess >& xFields)
{
    if (!xFields.is())
        return;

    // Spalten initialisieren
    Reference< ::com::sun::star::container::XIndexContainer >  xColumns(GetPeer()->getColumns());
    Reference< ::com::sun::star::container::XNameAccess >  xFieldsAsNames(xFields, UNO_QUERY);
    sal_Int32 nFieldCount = xFields->getCount();

    // Einfuegen muﬂ sich an den Column Positionen orientieren
    ::rtl::OUString aFieldName;
    for (sal_Int32 i = 0; i < xColumns->getCount(); i++)
    {
        DbGridColumn*   pCol = GetColumns().GetObject(i);
        Reference< ::com::sun::star::beans::XPropertySet > xCol;
        ::cppu::extractInterface(xCol, xColumns->getByIndex(i));
        DbCellControl*  pCellControl  = NULL;

        // suchen des Feldes, das zur Controlsource gehoert
        xCol->getPropertyValue(FM_PROP_CONTROLSOURCE) >>= aFieldName;
        Reference< ::com::sun::star::beans::XPropertySet >  xField;

        if (aFieldName.len() && xFieldsAsNames->hasByName(aFieldName))
        {
            ::cppu::extractInterface(xField, xFieldsAsNames->getByName(aFieldName));
        }

        // feststellen der Feldposition
        sal_Int32 nFieldPos = -1;
        if (xField.is())
        {
            Reference< ::com::sun::star::beans::XPropertySet > xCheck;
            for (sal_Int32 i = 0; i < nFieldCount; i++)
            {
                ::cppu::extractInterface(xCheck, xFields->getByIndex(i));
                if (xField == xCheck)
                {
                    nFieldPos = i;
                    break;
                }
            }
        }

        if (xField.is() && nFieldPos >= 0)
        {
            // Datenfelder mit folgenden Datentypen kˆnnen nicht verwendet werden
            sal_Int32 nDataType;
            xField->getPropertyValue(FM_PROP_FIELDTYPE) >>= nDataType;
            sal_Bool bIllegalType(sal_False);
            switch (nDataType)
            {
                case DataType::LONGVARBINARY:
                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::OTHER:
                    bIllegalType = sal_True;
            }

            if (bIllegalType)
            {
                pCol->SetObject(nFieldPos);
                continue;
            }
            else
            {
                // Feststellen ob ReadOnly
                sal_Bool bReadOnly = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISREADONLY));
                pCol->SetReadOnly(bReadOnly);
            }
        }

        // anhand des ServiceNamens wird das Control bestimmt
        ::rtl::OUString sPropColumnServiceName = ::rtl::OUString::createFromAscii("ColumnServiceName");
        if (!::comphelper::hasProperty(sPropColumnServiceName, xCol))
            return;

        pCol->setModel(xCol);

        sal_Int32 nTypeId = getColumnTypeByModelName(::comphelper::getString(xCol->getPropertyValue(sPropColumnServiceName)));
        pCol->CreateControl(nFieldPos, xField, nTypeId);
    }
}

//------------------------------------------------------------------------------
void FmGridControl::HideColumn(sal_uInt16 nId)
{
    DbGridControl::HideColumn(nId);

    sal_uInt16 nPos = GetModelColumnPos(nId);
    if (nPos == (sal_uInt16)-1)
        return;

    DbGridColumn* pColumn = GetColumns().GetObject(nPos);
    if (pColumn->IsHidden())
        GetPeer()->columnHidden(pColumn);

    if (nId == m_nMarkedColumnId)
        m_nMarkedColumnId = (sal_uInt16)-1;
}

//------------------------------------------------------------------------------
void FmGridControl::ShowColumn(sal_uInt16 nId)
{
    DbGridControl::ShowColumn(nId);

    sal_uInt16 nPos = GetModelColumnPos(nId);
    if (nPos == (sal_uInt16)-1)
        return;

    DbGridColumn* pColumn = GetColumns().GetObject(nPos);
    if (!pColumn->IsHidden())
        GetPeer()->columnVisible(pColumn);

    // if the column which is shown here is selected ...
    Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(GetPeer()->getColumns(), UNO_QUERY);
    if (xSelSupplier.is())
    {
        Reference< ::com::sun::star::beans::XPropertySet >  xColumn;
        ::cppu::extractInterface(xColumn, xSelSupplier->getSelection());
        if (xColumn.get() == pColumn->getModel().get())
            // ... -> mark it
            markColumn(nId);
    }
}

//------------------------------------------------------------------------------
Sequence< Any> FmGridControl::getSelectionBookmarks()
{
    sal_Int32 nSelectedRows = GetSelectRowCount(), i = 0;
    Sequence< Any> aBookmarks(nSelectedRows);
    Any* pBookmarks = (Any*)aBookmarks.getArray();

    // lock our update so no paint-triggered seeks interfere ...
    SetUpdateMode(sal_False);
    // (I'm not sure if the problem isn't deeper : The szenario : a large table displayed by a grid with a
    // thread-safe cursor (dBase). On loading the sdb-cursor started a counting thread. While this counting progress
    // was running, I tried do delete 3 records from within the grid. Deletion caused a SeekCursor, which did a
    // m_pSeekCursor->moveRelative and a m_pSeekCursor->getPosition.
    // Unfortunally the first call caused a propertyChanged(RECORDCOUNT) which resulted in a repaint of the
    // navigation bar and the grid. The latter itself will result in SeekRow calls. So after (successfully) returning
    // from the moveRelative the getPosition returns an invalid value. And so the SeekCursor fails.
    // In the consequence ALL parts of code where two calls to the seek cursor are done, while the second call _relys_ on
    // the first one, should be secured against recursion, with a broad-minded interpretion of "recursion" : if any of these
    // code parts is executed, no other should be accessible. But this sounds very difficult to achieve ....
    // )

    // The next problem caused by the same behaviuor (SeekCursor causes a propertyChanged) : when adjusting rows we implicitly
    // change our selection. So a "FirstSelected(); SeekCursor(); NextSelected();" may produce unpredictable results.
    // That's why we _first_ collect the indicies of the selected rows and _then_ their bookmarks.
    long nIdx = FirstSelectedRow();
    while (nIdx >= 0)
    {
        // (we misuse the bookmarks array for this ...)
        pBookmarks[i++] <<= (sal_Int32)nIdx;
        nIdx = NextSelectedRow();
    }
    DBG_ASSERT(i == nSelectedRows, "FmGridControl::DeleteSelectedRows : could not collect the row indicies !");

    for (i=0; i<nSelectedRows; ++i)
    {
        nIdx = ::comphelper::getINT32(pBookmarks[i]);
        if (IsEmptyRow(nIdx))
        {
            // leerzeile nicht loeschen
            aBookmarks.realloc(--nSelectedRows);
            SelectRow(nIdx,sal_False);          // selection aufheben fuer leerzeile
            break;
        }

        // Zunaechst den DatenCursor auf den selektierten Satz pos.
        if (SeekCursor(nIdx))
        {
            GetSeekRow()->SetState(m_pSeekCursor, sal_True);

            pBookmarks[i] = m_pSeekCursor->getBookmark();
        }
#if DBG_UTIL
        else
            DBG_ERROR("FmGridControl::DeleteSelectedRows : a bookmark could not be determined !");
#endif
    }
    SetUpdateMode(sal_True);

    // if one of the SeekCursor-calls failed ....
    aBookmarks.realloc(i);

    // (the alternative : while collecting the bookmarks lock our propertyChanged, this should resolve both our problems.
    // but this would be incompatible as we need a locking flag, then ...)

    return aBookmarks;
}

