/*************************************************************************
 *
 *  $RCSfile: queryorder.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:34:01 $
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
#ifndef DBAUI_QUERYORDER_HRC
#include "queryorder.hrc"
#endif
#ifndef DBAUI_QUERYORDER_HXX
#include "queryorder.hxx"
#endif

#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
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
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif


using namespace dbaui;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;

DBG_NAME(DlgOrderCrit);
//------------------------------------------------------------------------------
DlgOrderCrit::DlgOrderCrit( Window * pParent,
                            const Reference< XConnection>& _rxConnection,
                            const Reference< XSQLQueryComposer>& _rxQueryComposer,
                            const Reference< XNameAccess>& _rxCols)
             :ModalDialog( pParent, ModuleRes(DLG_ORDERCRIT) )
            ,aLB_ORDERFIELD1(   this, ResId( LB_ORDERFIELD1 ) )
            ,aLB_ORDERVALUE1(   this, ResId( LB_ORDERVALUE1 ) )
            ,aLB_ORDERFIELD2(   this, ResId( LB_ORDERFIELD2 ) )
            ,aLB_ORDERVALUE2(   this, ResId( LB_ORDERVALUE2 ) )
            ,aLB_ORDERFIELD3(   this, ResId( LB_ORDERFIELD3 ) )
            ,aLB_ORDERVALUE3(   this, ResId( LB_ORDERVALUE3 ) )
            ,aFT_ORDERFIELD(    this, ResId( FT_ORDERFIELD ) )
            ,aFT_ORDERVALUE(    this, ResId( FT_ORDERVALUE ) )
            ,aFT_ORDERAFTER1(   this, ResId( FT_ORDERAFTER1 ) )
            ,aFT_ORDERAFTER2(   this, ResId( FT_ORDERAFTER2 ) )
            ,aFT_ORDEROPER(     this, ResId( FT_ORDEROPER ) )
            ,aFT_ORDERDIR(      this, ResId( FT_ORDERDIR ) )
            ,aBT_OK(            this, ResId( BT_OK ) )
            ,aBT_CANCEL(        this, ResId( BT_CANCEL ) )
            ,aBT_HELP(          this, ResId( BT_HELP ) )
            ,aGB_ORDER(         this, ResId( GB_ORDER ) )
            ,aSTR_NOENTRY(      ResId( STR_NOENTRY ) )
            ,m_xQueryComposer(_rxQueryComposer)
            ,m_xColumns(_rxCols)
            ,m_xConnection(_rxConnection)
{
    DBG_CTOR(DlgOrderCrit,NULL);

    arrLbFields[0] = &aLB_ORDERFIELD1;
    arrLbFields[1] = &aLB_ORDERFIELD2;
    arrLbFields[2] = &aLB_ORDERFIELD3;

    arrLbValues[0] = &aLB_ORDERVALUE1;
    arrLbValues[1] = &aLB_ORDERVALUE2;
    arrLbValues[2] = &aLB_ORDERVALUE3;

    xub_StrLen j;
    for(j=0 ; j < DOG_ROWS ; j++ )
    {
        arrLbFields[j]->InsertEntry( aSTR_NOENTRY );
    }

    for( j=0 ; j < DOG_ROWS ; j++ )
    {
        arrLbFields[j]->SelectEntryPos(0);
        arrLbValues[j]->SelectEntryPos(0);
    }
    // ... sowie auch die restlichen Felder
    Sequence< ::rtl::OUString> aNames = m_xColumns->getElementNames();
    const ::rtl::OUString* pBegin = aNames.getConstArray();
    const ::rtl::OUString* pEnd   = pBegin + aNames.getLength();
    Reference<XPropertySet> xColumn;
    for(;pBegin != pEnd;++pBegin)
    {
        ::cppu::extractInterface(xColumn,m_xColumns->getByName(*pBegin));
        OSL_ENSURE(xColumn.is(),"Column is null!");
        sal_Int32 nDataType;
        xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
        sal_Int32 eColumnSearch = dbtools::getSearchColumnFlag(m_xConnection,nDataType);
        if(eColumnSearch != ColumnSearch::NONE)
        {
            for( j=0 ; j < DOG_ROWS ; j++ )
            {
                arrLbFields[j]->InsertEntry(*pBegin);
            }
        }
    }

    SetOrderList(m_xQueryComposer->getOrder());
    EnableLines();

    aLB_ORDERFIELD1.SetSelectHdl(LINK(this,DlgOrderCrit,FieldListSelectHdl));
    aLB_ORDERFIELD2.SetSelectHdl(LINK(this,DlgOrderCrit,FieldListSelectHdl));

    FreeResource();

}

//------------------------------------------------------------------------------

#define LbText(x)       ((x).GetSelectEntry())
#define LbPos(x)        ((x).GetSelectEntryPos())

//------------------------------------------------------------------------------
DlgOrderCrit::~DlgOrderCrit()
{
    DBG_DTOR(DlgOrderCrit,NULL);
}

//------------------------------------------------------------------------------
IMPL_LINK_INLINE_START( DlgOrderCrit, FieldListSelectHdl, ListBox *, pListBox )
{
    DBG_CHKTHIS(DlgOrderCrit,NULL);
    EnableLines();
    return 0;
}
IMPL_LINK_INLINE_END( DlgOrderCrit, FieldListSelectHdl, ListBox *, pListBox )

//------------------------------------------------------------------------------
void DlgOrderCrit::EnableLines()
{
    DBG_CHKTHIS(DlgOrderCrit,NULL);
    if( LbPos(aLB_ORDERFIELD1) == 0 )
    {
        String aOrderList(GetOrderList());
        if(aOrderList.GetTokenCount(','))
            SetOrderList(aOrderList);
        else
        {
            aLB_ORDERFIELD2.Disable();
            aLB_ORDERVALUE2.Disable();

            aLB_ORDERFIELD3.Disable();
            aLB_ORDERVALUE3.Disable();
        }
    }
    else
    {
        aLB_ORDERFIELD2.Enable();
        aLB_ORDERVALUE2.Enable();

        aLB_ORDERFIELD3.Enable();
        aLB_ORDERVALUE3.Enable();
    }

    if( LbPos(aLB_ORDERFIELD2) == 0 )
    {
        String aOrderList(GetOrderList());
        if(aOrderList.GetTokenCount(','))
            SetOrderList(aOrderList);
        else
        {
            aLB_ORDERFIELD3.Disable();
            aLB_ORDERVALUE3.Disable();
        }
    }
    else
    {
        aLB_ORDERFIELD3.Enable();
        aLB_ORDERVALUE3.Enable();
    }
}

//------------------------------------------------------------------------------
void DlgOrderCrit::SetOrderList( const String& _rOrderList )
{
    DBG_CHKTHIS(DlgOrderCrit,NULL);
    xub_StrLen nLen = _rOrderList.GetTokenCount(',');
    xub_StrLen i;
    for(i=0;i<nLen && i<DOG_ROWS;++i)
    {
        String aOrder = _rOrderList.GetToken(i,',');
        aOrder.EraseTrailingChars();
        arrLbFields[i]->SelectEntry( aOrder.GetToken(0,' ') );
        xub_StrLen nAsc = (aOrder.GetTokenCount(' ') == 2) ? (aOrder.GetToken(1,' ').EqualsAscii("ASC") ? 1 : 0) : 1;
        arrLbValues[i]->SelectEntryPos( nAsc );
    }

    // die nicht gesetzten auf 'kein' 'aufsteigend'
    xub_StrLen nItemsSet = min(nLen, xub_StrLen(DOG_ROWS));
    for (i=0 ; i<DOG_ROWS-nItemsSet; ++i)
    {
        arrLbFields[2-i]->SelectEntryPos( 0 );
        arrLbValues[2-i]->SelectEntryPos( 0 );
    }

}

//------------------------------------------------------------------------------
String DlgOrderCrit::GetOrderList( )
{
    DBG_CHKTHIS(DlgOrderCrit,NULL);
    String aOrder;
    for( sal_uInt16 i=0 ; i<DOG_ROWS; i++ )
    {
        if(arrLbFields[i]->GetSelectEntryPos() != 0)
        {
            if(aOrder.Len())
                aOrder.AppendAscii(",");

            aOrder += arrLbFields[i]->GetSelectEntry();
            if(arrLbValues[i]->GetSelectEntryPos())
                aOrder.AppendAscii(" DESC ");
            else
                aOrder.AppendAscii(" ASC ");
        }
    }
    return aOrder;
}

//------------------------------------------------------------------------------
void DlgOrderCrit::BuildOrderPart()
{
    DBG_CHKTHIS(DlgOrderCrit,NULL);
    m_xQueryComposer->setOrder(GetOrderList());
}
// -----------------------------------------------------------------------------





