/*************************************************************************
 *
 *  $RCSfile: queryfilter.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-04 13:57:29 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef DBAUI_QUERYFILTER_HXX
#include "queryfilter.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNSEARCH_HPP_
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include <connectivity/sqliterator.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBAUI_QUERYFILTER_HRC
#include "queryfilter.hrc"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif


using namespace dbaui;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;

static const char   aAND[] = "AND";
static const char   aOR[]  = "OR";

//------------------------------------------------------------------------------
void Replace_OS_PlaceHolder(String& aString)
{
    while (aString.SearchAndReplace( '*', '%' ) != STRING_NOTFOUND);
    while (aString.SearchAndReplace( '?', '_' ) != STRING_NOTFOUND);
}

//------------------------------------------------------------------------------
void Replace_SQL_PlaceHolder(String& aString)
{
    while (aString.SearchAndReplace( '%', '*' ) != STRING_NOTFOUND);
    while (aString.SearchAndReplace( '_', '?' ) != STRING_NOTFOUND);
}

DBG_NAME(DlgFilterCrit);
//------------------------------------------------------------------------------
DlgFilterCrit::DlgFilterCrit(Window * pParent,
                             const Reference< XConnection>& _rxConnection,
                             const Reference< XSQLQueryComposer>& _rxQueryComposer,
                             const Reference< XNameAccess>& _rxCols,
                             const String& rFieldName)
            : ModalDialog( pParent, ModuleRes( DLG_FILTERCRIT ) )
            ,aLB_WHEREFIELD1    ( this, ResId( LB_WHEREFIELD1 ) )
            ,aLB_WHERECOMP1     ( this, ResId( LB_WHERECOMP1 ) )
            ,aET_WHEREVALUE1    ( this, ResId( ET_WHEREVALUE1 ) )
            ,aLB_WHERECOND2     ( this, ResId( LB_WHERECOND2 ) )
            ,aLB_WHEREFIELD2    ( this, ResId( LB_WHEREFIELD2 ) )
            ,aLB_WHERECOMP2     ( this, ResId( LB_WHERECOMP2 ) )
            ,aET_WHEREVALUE2    ( this, ResId( ET_WHEREVALUE2 ) )
            ,aLB_WHERECOND3     ( this, ResId( LB_WHERECOND3 ) )
            ,aLB_WHEREFIELD3    ( this, ResId( LB_WHEREFIELD3 ) )
            ,aLB_WHERECOMP3     ( this, ResId( LB_WHERECOMP3 ) )
            ,aET_WHEREVALUE3    ( this, ResId( ET_WHEREVALUE3 ) )
            ,aFT_WHEREFIELD     ( this, ResId( FT_WHEREFIELD ) )
            ,aFT_WHERECOMP      ( this, ResId( FT_WHERECOMP ) )
            ,aFT_WHEREVALUE     ( this, ResId( FT_WHEREVALUE ) )
            ,aFT_WHEREOPER      ( this, ResId( FT_WHEREOPER ) )
            ,aGB_FIELDS         ( this, ResId( GB_FIELDS ) )
            ,aBT_OK             ( this, ResId( BT_OK ) )
            ,aBT_CANCEL         ( this, ResId( BT_CANCEL ) )
            ,aBT_HELP           ( this, ResId( BT_HELP ) )
            ,aSTR_NOENTRY       ( ResId( STR_NOENTRY ) )
            ,aSTR_COMPARE_OPERATORS( ResId( STR_COMPARE_OPERATORS ) )
            ,m_xQueryComposer(_rxQueryComposer)
            ,m_xColumns(_rxCols)
            ,m_xConnection(_rxConnection)
{
    DBG_CTOR(DlgFilterCrit,NULL);
    // Den String fuer noEntry in die ListBoxen der Feldnamen schreiben
    aLB_WHEREFIELD1.InsertEntry( aSTR_NOENTRY );
    aLB_WHEREFIELD2.InsertEntry( aSTR_NOENTRY );
    aLB_WHEREFIELD3.InsertEntry( aSTR_NOENTRY );

    try
    {
        // ... sowie auch die restlichen Felder
        Sequence< ::rtl::OUString> aNames = m_xColumns->getElementNames();
        const ::rtl::OUString* pBegin = aNames.getConstArray();
        const ::rtl::OUString* pEnd   = pBegin + aNames.getLength();
        Reference<XPropertySet> xColumn;
        for(;pBegin != pEnd;++pBegin)
        {
            if (m_xColumns->hasByName(*pBegin))
            {
                ::cppu::extractInterface(xColumn,m_xColumns->getByName(*pBegin));
                OSL_ENSURE(xColumn.is(),"DlgFilterCrit::DlgFilterCrit: Column is null!");
            }
            else
                OSL_ENSURE(sal_False, "DlgFilterCrit::DlgFilterCrit: invalid column name!");
            sal_Int32 nDataType(0);
            xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
            sal_Int32 eColumnSearch = dbtools::getSearchColumnFlag(m_xConnection,nDataType);
            // TODO
            // !pColumn->IsFunction()
            if(eColumnSearch != ColumnSearch::NONE)
            {
                aLB_WHEREFIELD1.InsertEntry( *pBegin );
                aLB_WHEREFIELD2.InsertEntry( *pBegin );
                aLB_WHEREFIELD3.InsertEntry( *pBegin );
            }
        }
        // initialize the listboxes with noEntry
        aLB_WHEREFIELD1.SelectEntryPos(0);
        aLB_WHEREFIELD2.SelectEntryPos(0);
        aLB_WHEREFIELD3.SelectEntryPos(0);

        // Jetzt die Felder mit den Kriterien des SQL-Strings fuellen
        ::cppu::extractInterface(xColumn,m_xColumns->getByName(rFieldName));
        m_xQueryComposer->appendFilterByColumn(xColumn);

        // insert the criteria into the dialog
        Sequence<Sequence<PropertyValue > > aValues = m_xQueryComposer->getStructuredFilter();
        const Sequence<PropertyValue >* pOrBegin = aValues.getConstArray();
        const Sequence<PropertyValue >* pOrEnd   = pOrBegin + aValues.getLength();
        sal_Bool bOr = sal_True;
        for(sal_uInt16 i=0;pOrBegin != pOrEnd; ++pOrBegin)
        {
            bOr = sal_True;
            const PropertyValue* pAndBegin  = pOrBegin->getConstArray();
            const PropertyValue* pAndEnd    = pAndBegin + pOrBegin->getLength();
            for(;pAndBegin != pAndEnd; ++pAndBegin)
            {
                SetLine( i++,*pAndBegin,bOr);
                bOr = sal_False;
            }
        }
    }
    catch(Exception&)
    {
        FreeResource();
        throw;
    }

    EnableLines();

    aLB_WHEREFIELD1.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectHdl));
    aLB_WHEREFIELD2.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectHdl));
    aLB_WHEREFIELD3.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectHdl));

    aLB_WHERECOMP1.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectCompHdl));
    aLB_WHERECOMP2.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectCompHdl));
    aLB_WHERECOMP3.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectCompHdl));

    FreeResource();
}
//------------------------------------------------------------------------------
DlgFilterCrit::~DlgFilterCrit()
{
    DBG_DTOR(DlgFilterCrit,NULL);
}

#define LbText(x)       ((x).GetSelectEntry())
#define LbPos(x)        ((x).GetSelectEntryPos())

//------------------------------------------------------------------------------
OSQLPredicateType DlgFilterCrit::GetOSQLPredicateType(sal_uInt16 nPos,sal_uInt16 nCount) const
{
    OSQLPredicateType ePreType;

    if(nCount == 10)
    {
        switch(nPos)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            ePreType = (OSQLPredicateType)(nPos + SQL_PRED_EQUAL);
            break;
        case 6:
            ePreType = SQL_PRED_LIKE;
            break;
        case 7:
            ePreType = SQL_PRED_NOTLIKE;
            break;
        case 8:
            ePreType = SQL_PRED_ISNULL;
            break;
        case 9:
            ePreType = SQL_PRED_ISNOTNULL;
            break;
        }
    }
    else if(nCount == 8)
    {
        switch(nPos)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            ePreType = (OSQLPredicateType)(nPos + SQL_PRED_EQUAL);
            break;
        case 6:
            ePreType = SQL_PRED_ISNULL;
            break;
        case 7:
            ePreType = SQL_PRED_ISNOTNULL;
            break;
        }
    }
    else
    {
        switch(nPos)
        {
        case 0:
            ePreType = SQL_PRED_LIKE;
            break;
        case 1:
            ePreType = SQL_PRED_NOTLIKE;
            break;
        }
    }
    return ePreType;
}
//------------------------------------------------------------------------------
sal_uInt16 DlgFilterCrit::GetSelectionPos(OSQLPredicateType eType,const ListBox& rListBox) const
{
    sal_uInt16 nPos;
    switch(eType)
    {
        case SQL_PRED_EQUAL:
            nPos = 0;
            break;
        case SQL_PRED_NOTEQUAL:
            nPos = 1;
            break;
        case SQL_PRED_LESS:
            nPos = 2;
            break;
        case SQL_PRED_LESSOREQUAL:
            nPos = 3;
            break;
        case SQL_PRED_GREATER:
            nPos = 4;
            break;
        case SQL_PRED_GREATEROREQUAL:
            nPos = 5;
            break;
        case SQL_PRED_NOTLIKE:
            nPos = rListBox.GetEntryCount() > 2 ? rListBox.GetEntryCount()-3 : 0;
            break;
        case SQL_PRED_LIKE:
            nPos = rListBox.GetEntryCount() > 2 ? rListBox.GetEntryCount()-4 : 1;
            break;
        case SQL_PRED_ISNULL:
            nPos = rListBox.GetEntryCount()-2;
            break;
        case SQL_PRED_ISNOTNULL:
            nPos = rListBox.GetEntryCount()-1;
            break;
    }
    return nPos;
}
// -----------------------------------------------------------------------------
::rtl::OUString DlgFilterCrit::getCondition(const ListBox& _rField,const ListBox& _rComp,const Edit& _rValue) const
{
    ::rtl::OUString aFilter(_rField.GetSelectEntry());
    // first quote the field name
    Reference<XDatabaseMetaData> xMetaData = m_xConnection.is() ? m_xConnection->getMetaData() : Reference<XDatabaseMetaData>();
    ::rtl::OUString aQuote  = xMetaData.is() ? xMetaData->getIdentifierQuoteString() : ::rtl::OUString();
    aFilter = ::dbtools::quoteName(aQuote,aFilter);

    aFilter += ::rtl::OUString::createFromAscii(" ");
    sal_Bool bNeedText = sal_True;
    switch(GetOSQLPredicateType(_rComp.GetSelectEntryPos(),_rComp.GetEntryCount()))
    {
        case SQL_PRED_EQUAL:
            aFilter += ::rtl::OUString::createFromAscii("=");
            break;
        case SQL_PRED_NOTEQUAL:
            aFilter += ::rtl::OUString::createFromAscii("<>");
            break;
        case SQL_PRED_LESS:
            aFilter += ::rtl::OUString::createFromAscii("<");
            break;
        case SQL_PRED_LESSOREQUAL:
            aFilter += ::rtl::OUString::createFromAscii("<=");
            break;
        case SQL_PRED_GREATER:
            aFilter += ::rtl::OUString::createFromAscii(">");
            break;
        case SQL_PRED_GREATEROREQUAL:
            aFilter += ::rtl::OUString::createFromAscii(">=");
            break;
        case SQL_PRED_NOTLIKE:
            aFilter += ::rtl::OUString::createFromAscii("NOT LIKE");
            break;
        case SQL_PRED_LIKE:
            aFilter += ::rtl::OUString::createFromAscii("LIKE");
            break;
        case SQL_PRED_ISNULL:
            aFilter += ::rtl::OUString::createFromAscii("IS NULL");
            bNeedText = sal_False;
            break;
        case SQL_PRED_ISNOTNULL:
            aFilter += ::rtl::OUString::createFromAscii("IS NOT NULL");
            bNeedText = sal_False;
            break;
    }
    if(bNeedText)
    {
        aFilter += ::rtl::OUString::createFromAscii(" ");
        String aTemp = _rValue.GetText();
        ::Replace_OS_PlaceHolder(aTemp);
        aFilter += aTemp.GetBuffer();
    }
    return aFilter;
}
//------------------------------------------------------------------------------
void DlgFilterCrit::GetFilterList() const
{
    PropertyValue aItem;
    ::rtl::OUString aFilter;

    if( LbPos(aLB_WHEREFIELD1) != 0 )
        aFilter = getCondition(aLB_WHEREFIELD1,aLB_WHERECOMP1,aET_WHEREVALUE1);

    if( LbPos(aLB_WHEREFIELD2) != 0 )
    {
        if(aFilter.getLength())
        {
            if(aLB_WHERECOND2.GetSelectEntryPos())
                aFilter += ::rtl::OUString::createFromAscii(" AND ");
            else
                aFilter += ::rtl::OUString::createFromAscii(" OR ");
        }
        aFilter += getCondition(aLB_WHEREFIELD2,aLB_WHERECOMP2,aET_WHEREVALUE2);
    }

    if( LbPos(aLB_WHEREFIELD3) != 0 )
    {
        if(aFilter.getLength())
        {
            if(aLB_WHERECOND3.GetSelectEntryPos())
                aFilter += ::rtl::OUString::createFromAscii(" AND ");
            else
                aFilter += ::rtl::OUString::createFromAscii(" OR ");
        }
        aFilter += getCondition(aLB_WHEREFIELD3,aLB_WHERECOMP3,aET_WHEREVALUE3);
    }
    // now set the filter in the querycomposer
//  ::rtl::OUString aOldFilter = m_xQueryComposer->getFilter();
//  if(aOldFilter.getLength())
//  {
//      aOldFilter += ::rtl::OUString::createFromAscii(" AND ");
//      aOldFilter += aFilter;
//      aFilter = aOldFilter;
//  }
    m_xQueryComposer->setFilter(aFilter);
}

//------------------------------------------------------------------------------
//void DlgFilterCrit::SetFilterList( const FilterPredicateItemList* pItemList )
//{
//  for (sal_uInt16 i=0; i<pItemList->Count(); ++i)
//  {
//      FilterPredicateItem* pCritItem = pItemList->GetObject(i);
//      SetLine(i, pCritItem);
//  }
//
//  // die nicht gesetzten auf 'kein' '=' ''
//  sal_uInt16 nItemsSet = pItemList ? min((sal_Int32)pItemList->Count(), 3L) : 0;
//  if (nItemsSet<3)
//  {
//      aLB_WHEREFIELD3.SelectEntryPos(0);
//      aLB_WHERECOMP3.SelectEntryPos(0);
//      aET_WHEREVALUE3.SetText(String());
//  }
//  if (nItemsSet<2)
//  {
//      aLB_WHEREFIELD2.SelectEntryPos(0);
//      aLB_WHERECOMP2.SelectEntryPos(0);
//      aET_WHEREVALUE2.SetText(String());
//  }
//  if (nItemsSet<1)
//  {
//      aLB_WHEREFIELD1.SelectEntryPos(0);
//      aLB_WHERECOMP1.SelectEntryPos(0);
//      aET_WHEREVALUE1.SetText(String());
//  }
//  EnableLines();
//}

//------------------------------------------------------------------------------
void DlgFilterCrit::SetLine( sal_uInt16 nIdx,const PropertyValue& _rItem,sal_Bool _bOr  )
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    ::rtl::OUString aCondition;
    _rItem.Value >>= aCondition;
    String aStr = aCondition.getStr();
    ::Replace_SQL_PlaceHolder(aStr);
    aStr.EraseTrailingChars();

    // remove the predicate from the condition
    switch(_rItem.Handle)
    {
        case SQL_PRED_EQUAL:
            //  aStr.Erase(0,1);
            break;
        case SQL_PRED_NOTEQUAL:
            aStr.Erase(0,2);
            break;
        case SQL_PRED_LESS:
            aStr.Erase(0,1);
            break;
        case SQL_PRED_LESSOREQUAL:
            aStr.Erase(0,2);
            break;
        case SQL_PRED_GREATER:
            aStr.Erase(0,1);
            break;
        case SQL_PRED_GREATEROREQUAL:
            aStr.Erase(0,2);
            break;
        case SQL_PRED_NOTLIKE:
            aStr.Erase(0,8);
            break;
        case SQL_PRED_LIKE:
            aStr.Erase(0,4);
            break;
        case SQL_PRED_ISNULL:
            aStr.Erase(0,7);
            break;
        case SQL_PRED_ISNOTNULL:
            aStr.Erase(0,11);
            break;
    }
    aStr.EraseLeadingChars();

    // to make sure that we only set first three
    switch( nIdx )
    {
        case 0:
        {
            SelectField( aLB_WHEREFIELD1, _rItem.Name );
            ListSelectHdl(&aLB_WHEREFIELD1);
            aLB_WHERECOMP1.SelectEntryPos( GetSelectionPos((OSQLPredicateType)_rItem.Handle , aLB_WHERECOMP1));
            aET_WHEREVALUE1.SetText( aStr );
        }
        break;

        case 1:
        {
            aLB_WHERECOND2.SelectEntryPos( _bOr ? 1 : 0 );
            SelectField( aLB_WHEREFIELD2, _rItem.Name );
            ListSelectHdl(&aLB_WHEREFIELD2);
            aLB_WHERECOMP2.SelectEntryPos( GetSelectionPos((OSQLPredicateType)_rItem.Handle , aLB_WHERECOMP2));
            aET_WHEREVALUE2.SetText( aStr );
        }
        break;

        case 2:
        {
            aLB_WHERECOND3.SelectEntryPos( _bOr ? 1 : 0 );
            SelectField( aLB_WHEREFIELD3, _rItem.Name );
            ListSelectHdl(&aLB_WHEREFIELD3);
            aLB_WHERECOMP3.SelectEntryPos( GetSelectionPos((OSQLPredicateType)_rItem.Handle , aLB_WHERECOMP3));
            aET_WHEREVALUE3.SetText( aStr );
        }
        break;
    }
}

//------------------------------------------------------------------------------
void DlgFilterCrit::SelectField( ListBox& rBox, const String& rField )
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    sal_uInt16 nCnt = rBox.GetEntryCount();
    //  sal_Bool bCase = m_rIterator.TablesAreSensitive();

    for( sal_uInt16 i=0 ; i<nCnt ; i++ )
    {
        //  if(bCase ? rBox.GetEntry(i) == rField : rBox.GetEntry(i).EqualsIgnoreCaseAscii(rField))
        if(rBox.GetEntry(i) == rField)
        {
            rBox.SelectEntryPos(i);
            return;
        }
    }

    rBox.SelectEntryPos(0);
}

//------------------------------------------------------------------------------
void DlgFilterCrit::EnableLines()
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    // Enablen/Disablen ganzer Zeilen
    if( LbPos(aLB_WHEREFIELD1) == 0 )
    {
        aLB_WHEREFIELD2.Disable();
        aLB_WHERECOND2.Disable();
        aLB_WHERECOMP2.Disable();
        aET_WHEREVALUE2.Disable();

        aLB_WHEREFIELD3.Disable();
        aLB_WHERECOND3.Disable();
        aLB_WHERECOMP3.Disable();
        aET_WHEREVALUE3.Disable();
    }
    else
    {
        aLB_WHEREFIELD2.Enable();
        aLB_WHERECOND2.Enable();
        aLB_WHERECOMP2.Enable();
        aET_WHEREVALUE2.Enable();

        aLB_WHEREFIELD3.Enable();
        aLB_WHERECOND3.Enable();
        aLB_WHERECOMP3.Enable();
        aET_WHEREVALUE3.Enable();
    }

    if( LbPos(aLB_WHEREFIELD2) == 0 )
    {
        aLB_WHEREFIELD3.Disable();
        aLB_WHERECOND3.Disable();
        aLB_WHERECOMP3.Disable();
        aET_WHEREVALUE3.Disable();
    }
    else
    {
        aLB_WHEREFIELD3.Enable();
        aLB_WHERECOND3.Enable();
        aLB_WHERECOMP3.Enable();
        aET_WHEREVALUE3.Enable();
    }

    // Vergleichsfeld gleich NOENTRY
    if( LbPos(aLB_WHEREFIELD1) == 0 )
    {
        aLB_WHERECOMP1.Disable();
        aET_WHEREVALUE1.Disable();
    }
    else
    {
        aLB_WHEREFIELD1.Enable();
        aLB_WHERECOMP1.Enable();
        aET_WHEREVALUE1.Enable();
    }

    if( LbPos(aLB_WHEREFIELD2) == 0 )
    {
        aLB_WHERECOND2.Disable();
        aLB_WHERECOMP2.Disable();
        aET_WHEREVALUE2.Disable();
    }
    else
    {
        aLB_WHERECOND2.Enable();
        aLB_WHEREFIELD2.Enable();
        aLB_WHERECOMP2.Enable();
        aET_WHEREVALUE2.Enable();
    }

    if( LbPos(aLB_WHEREFIELD3) == 0 )
    {
        aLB_WHERECOND3.Disable();
        aLB_WHERECOMP3.Disable();
        aET_WHEREVALUE3.Disable();
    }
    else
    {
        aLB_WHERECOND3.Enable();
        aLB_WHERECOND3.Enable();
        aLB_WHEREFIELD3.Enable();
        aLB_WHERECOMP3.Enable();
        aET_WHEREVALUE3.Enable();
    }

    // Vergleichsoperator gleich ISNULL oder ISNOTNULL
    if(aLB_WHERECOMP1.GetEntryCount() > 2 &&
        ((LbPos(aLB_WHERECOMP1) == aLB_WHERECOMP1.GetEntryCount()-1) ||
         (LbPos(aLB_WHERECOMP1) == aLB_WHERECOMP1.GetEntryCount()-2)) )
        aET_WHEREVALUE1.Disable();

    if(aLB_WHERECOMP2.GetEntryCount() > 2 &&
        ((LbPos(aLB_WHERECOMP2) == aLB_WHERECOMP2.GetEntryCount()-1) ||
         (LbPos(aLB_WHERECOMP2) == aLB_WHERECOMP2.GetEntryCount()-2)) )
        aET_WHEREVALUE2.Disable();

    if(aLB_WHERECOMP3.GetEntryCount() > 2 &&
        ((LbPos(aLB_WHERECOMP3) == aLB_WHERECOMP3.GetEntryCount()-1) ||
         (LbPos(aLB_WHERECOMP3) == aLB_WHERECOMP3.GetEntryCount()-2)) )
        aET_WHEREVALUE3.Disable();

}

//------------------------------------------------------------------------------
IMPL_LINK( DlgFilterCrit, ListSelectHdl, ListBox *, pListBox )
{
    String aName;
    ListBox* pComp;
    if(pListBox == &aLB_WHEREFIELD1)
    {
        aName = LbText(aLB_WHEREFIELD1);
        pComp = &aLB_WHERECOMP1;
    }
    else if(pListBox == &aLB_WHEREFIELD2)
    {
        aName = LbText(aLB_WHEREFIELD2);
        pComp = &aLB_WHERECOMP2;
    }
    else
    {
        aName = LbText(aLB_WHEREFIELD3);
        pComp = &aLB_WHERECOMP3;
    }

    pComp->Clear();

    Reference<XPropertySet> xColumn;
    if (m_xColumns->hasByName(aName))
        ::cppu::extractInterface(xColumn,m_xColumns->getByName(aName));
    if(xColumn.is())
    {
        sal_Int32 nDataType;
        xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
        sal_Int32 eColumnSearch = dbtools::getSearchColumnFlag(m_xConnection,nDataType);

        if(eColumnSearch  == ColumnSearch::FULL)
        {
            for(xub_StrLen i=0;i<aSTR_COMPARE_OPERATORS.GetTokenCount();i++)
                pComp->InsertEntry(aSTR_COMPARE_OPERATORS.GetToken(i));
        }
        else if(eColumnSearch == ColumnSearch::CHAR)
        {
            for(xub_StrLen i=6;i<10;i++)
                pComp->InsertEntry(aSTR_COMPARE_OPERATORS.GetToken(i));
        }
        else if(eColumnSearch == ColumnSearch::BASIC)
        {
            for(xub_StrLen i=0;i<6;i++)
                pComp->InsertEntry(aSTR_COMPARE_OPERATORS.GetToken(i));
            for(i=8;i<aSTR_COMPARE_OPERATORS.GetTokenCount();i++)
                pComp->InsertEntry(aSTR_COMPARE_OPERATORS.GetToken(i));
        }
        else
        {
            DBG_ASSERT(0,"DlgFilterCrit::ListSelectHdl: Diese Column dürfte garnicht vorhanden sein!");
        }
    }
    pComp->SelectEntryPos(0);

    EnableLines();
    return 0;
}



//------------------------------------------------------------------------------
IMPL_LINK_INLINE_START( DlgFilterCrit, ListSelectCompHdl, ListBox *, pListBox )
{
    EnableLines();
    return 0;
}
IMPL_LINK_INLINE_END( DlgFilterCrit, ListSelectCompHdl, ListBox *, pListBox )
//------------------------------------------------------------------------------
String DlgFilterCrit::BuildWherePart()
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    GetFilterList();

    return m_xQueryComposer->getFilter();
}


