/*************************************************************************
 *
 *  $RCSfile: tabletree.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-29 07:09:53 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_TABLETREE_HXX_
#include "tabletree.hxx"
#endif
#ifndef _DBAUI_TABLETREE_HRC_
#include "tabletree.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif
#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

using namespace ::dbtools;
using namespace ::comphelper;

//========================================================================
//= OTableTreeListBox
//========================================================================
OTableTreeListBox::OTableTreeListBox( Window* pParent, WinBits nWinStyle,sal_Bool _bShowFirstEntry )
    :OMarkableTreeListBox(pParent,nWinStyle)
    ,m_aTableImage(ResId(TABLE_TREE_ICON))
    ,m_aViewImage(ResId(VIEW_TREE_ICON))
    ,m_bShowFirstEntry(_bShowFirstEntry)
{
    SetDefaultExpandedEntryBmp(Image(ModuleRes(TABLEFOLDER_TREE_ICON)));
    SetDefaultCollapsedEntryBmp(Image(ModuleRes(TABLEFOLDER_TREE_ICON)));
}
//------------------------------------------------------------------------
OTableTreeListBox::OTableTreeListBox( Window* pParent, const ResId& rResId ,sal_Bool _bShowFirstEntry)
    :OMarkableTreeListBox(pParent,rResId)
    ,m_aTableImage(ModuleRes(TABLE_TREE_ICON))
    ,m_aViewImage(ModuleRes(VIEW_TREE_ICON))
    ,m_bShowFirstEntry(_bShowFirstEntry)
{
    SetDefaultExpandedEntryBmp(Image(ModuleRes(TABLEFOLDER_TREE_ICON)));
    SetDefaultCollapsedEntryBmp(Image(ModuleRes(TABLEFOLDER_TREE_ICON)));
}
//------------------------------------------------------------------------
void OTableTreeListBox::Command( const CommandEvent& rEvt )
{
    sal_Bool bHandled = sal_False;
    switch( rEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            // die Stelle, an der geklickt wurde
            Point ptWhere;
            if (rEvt.IsMouseEvent())
            {
                ptWhere = rEvt.GetMousePosPixel();
                SvLBoxEntry* ptClickedOn = GetEntry(ptWhere);
                if (ptClickedOn == NULL)
                    break;
                if ( !IsSelected(ptClickedOn) )
                {
                    SelectAll(sal_False);
                    Select(ptClickedOn, sal_True);
                    SetCurEntry(ptClickedOn);
                }
            }
            else
                ptWhere = GetEntryPos(GetCurEntry());


            SvLBoxEntry* pCurrent = GetCurEntry();
            if (!pCurrent)
                break;

            SvSortMode eSortMode = GetModel()->GetSortMode();
            PopupMenu aContextMenu(ModuleRes(MENU_TABLETREE_POPUP));
            switch( aContextMenu.Execute( this, ptWhere ) )
            {
                case MID_SORT_ASCENDING:
                    GetModel()->SetSortMode(SortAscending);
                    break;
                case MID_SORT_DECENDING:
                    GetModel()->SetSortMode(SortDescending);
                    break;
            }
            if(eSortMode != GetModel()->GetSortMode())
                GetModel()->Resort();
        }
        break;
        default:
            SvTreeListBox::Command( rEvt );
    }
}

//------------------------------------------------------------------------
Reference< XConnection > OTableTreeListBox::UpdateTableList(const ::rtl::OUString& _rConnectionURL, const Sequence< PropertyValue > _rProperties) throw(SQLException)
{
    Reference< XDatabaseMetaData > xMetaData;
    Reference< XConnection > xConnection;

    Sequence< ::rtl::OUString > sTables, sViews;
    DBG_ASSERT(m_xORB.is(), "OTableTreeListBox::UpdateTableList : please use setServiceFactory to give me a service factory !");

    String sCurrentActionError;
    String sCurrentActionDetails;
    try
    {
        if (m_xORB.is())
        {
            // get the global DriverManager
            Reference< XDriverAccess > xDriverManager;
            sCurrentActionError = String(ModuleRes(STR_COULDNOTCREATE_DRIVERMANAGER));
                // in case an error occures
            sCurrentActionError.SearchAndReplaceAscii("#servicename#", (::rtl::OUString)SERVICE_SDBC_DRIVERMANAGER);
            try
            {
                xDriverManager = Reference< XDriverAccess >(m_xORB->createInstance(SERVICE_SDBC_DRIVERMANAGER), UNO_QUERY);
                DBG_ASSERT(xDriverManager.is(), "OTableTreeListBox::UpdateTableList : could not instantiate the driver manager, or it does not provide the necessary interface!");
            }
            catch (Exception& e)
            {
                // wrap the exception into an SQLException
                SQLException aSQLWrapper(e.Message, m_xORB, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")), 0, Any());
                throw aSQLWrapper;
            }
            if (!xDriverManager.is())
                throw Exception();


            sCurrentActionError = String(ModuleRes(STR_NOREGISTEREDDRIVER));
            Reference< XDriver > xDriver = xDriverManager->getDriverByURL(_rConnectionURL);
            if (!xDriver.is())
                // will be caught and translated into an SQLContext exception
                throw Exception();

            sCurrentActionError = String(ModuleRes(STR_COULDNOTCONNECT));
            sCurrentActionDetails = String(ModuleRes(STR_COULDNOTCONNECT_PLEASECHECK));
            xConnection = xDriver->connect(_rConnectionURL, _rProperties);
                // exceptions thrown by connect will be caught and re-routed
            DBG_ASSERT(xConnection.is(), "OTableTreeListBox::UpdateTableList : got an invalid connection!");
                // if no exception was thrown, the connection should be no-NULL)
            if (!xConnection.is())
                throw Exception();
            sCurrentActionDetails = String();
            xMetaData = xConnection->getMetaData();

            // get the (very necessary) interface XDataDefinitionSupplier
            Reference< XDataDefinitionSupplier > xDefinitionAccess;
            xDefinitionAccess = Reference< XDataDefinitionSupplier >(xDriver, UNO_QUERY);
            if (!xDefinitionAccess.is())
            {
                // okay, the driver's programmer was lazy :)
                // let's do it on foot .... (ask the meta data for a result set describing the tables)
                Reference< XResultSet > xTables;

                static const ::rtl::OUString s_sTableTypeView(RTL_CONSTASCII_USTRINGPARAM("VIEW"));

                if (xMetaData.is())
                {
                    // we want all table types, the standard types are VIEW and TABLE
                    Sequence< ::rtl::OUString > sTableTypes(2);
                    sTableTypes[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE"));
                    sTableTypes[1] = s_sTableTypeView;
                    // we want all catalogues, all schemas, all tables
                    const ::rtl::OUString sAll = ::rtl::OUString::createFromAscii("%");
                    xTables = xMetaData->getTables(Any(), sAll, sAll, sTableTypes);
                }
                Reference< XRow > xCurrentRow(xTables, UNO_QUERY);
                if (xCurrentRow.is())
                {
                    StringBag aTableNames, aViewNames;
                    ::rtl::OUString sCatalog, sSchema, sName, sType, sComposedName;
                    while (xTables->next())
                    {
                        // after creation the set is positioned before the first record, per definitionem
                        sCatalog    = xCurrentRow->getString(1);
                        sSchema     = xCurrentRow->getString(2);
                        sName       = xCurrentRow->getString(3);
                        sType       = xCurrentRow->getString(4);
                        ::dbtools::composeTableName(xMetaData, sCatalog, sSchema, sName, sComposedName, sal_False);
                        if (s_sTableTypeView.equals(sType))
                            aViewNames.insert(sComposedName);
                        else
                            aTableNames.insert(sComposedName);
                    }

                    // copy the names into the sequence
                    // tables
                    sTables.realloc(aTableNames.size());
                    ::rtl::OUString* pTables = sTables.getArray();
                    ConstStringBagIterator aCopy = aTableNames.begin();
                    for (; aCopy != aTableNames.end(); ++aCopy, ++pTables)
                        *pTables = *aCopy;
                    // views
                    sViews.realloc(aViewNames.size());
                    pTables = sViews.getArray();
                    for (aCopy = aViewNames.begin(); aCopy != aViewNames.end(); ++aCopy, ++pTables)
                        *pTables = *aCopy;
                }
                disposeComponent(xTables);
            }
            else
            {
                Reference< XTablesSupplier > xTableSupp;
                Reference< XViewsSupplier > xViewSupp;
                sCurrentActionError = String(ModuleRes(STR_NOTABLEINFO));

                // get the table supplier and the tables
                xTableSupp = xDefinitionAccess->getDataDefinitionByConnection(xConnection);
                if (!xTableSupp.is())
                    throw Exception();

                Reference< XNameAccess > xTables, xViews;

                xTables = xTableSupp->getTables();

                // get the views supplier and the views
                xViewSupp = Reference< XViewsSupplier >(xTableSupp, UNO_QUERY);
                if (xViewSupp.is())
                    xViews = xViewSupp->getViews();

                if (xTables.is())
                    sTables = xTables->getElementNames();
                if (xViews.is())
                    sViews = xViews->getElementNames();
            }
        }
    }
    catch(RuntimeException&)
    {
        DBG_ERROR("OTableTreeListBox::UpdateTableList : caught an RuntimeException!");
    }
    catch(SQLException& e)
    {
        sCurrentActionError.SearchAndReplaceAscii("#connurl#", _rConnectionURL);
        // prepend a string stating what we were doing and throw again
        SQLContext aExtendedInfo = prependContextInfo(e, NULL, sCurrentActionError.GetBuffer(), sCurrentActionDetails.GetBuffer());
        throw aExtendedInfo;
    }
    catch(Exception&)
    {
        sCurrentActionError.SearchAndReplaceAscii("#connurl#", _rConnectionURL);
        // a non-SQLException exception occured ... simply throw an SQLContext
        SQLContext aExtendedInfo;
        aExtendedInfo.Message = sCurrentActionError.GetBuffer();
        throw aExtendedInfo;
    }

    UpdateTableList(xMetaData, sTables, sViews);
    return xConnection;
}

//------------------------------------------------------------------------
void OTableTreeListBox::UpdateTableList(const Reference< XDatabaseMetaData >& _rxConnMetaData, const Sequence< ::rtl::OUString >& _rTables, const Sequence< ::rtl::OUString >& _rViews)
{
    // throw away all the old stuff
    Clear();

    if (!_rTables.getLength() && !_rViews.getLength())
        // nothing to do
        return;

    try
    {
        // the root entry saying "all objects"
        SvLBoxEntry* pAllObjects = NULL;
        if(m_bShowFirstEntry)
        {
            String sRootEntryText;
            if (!_rViews.getLength())
                sRootEntryText = String(ModuleRes(STR_ALL_TABLES));
            else if (!_rTables.getLength())
                sRootEntryText = String(ModuleRes(STR_ALL_VIEWS));
            else
                sRootEntryText = String(ModuleRes(STR_ALL_TABLES_AND_VIEWS));
            pAllObjects = InsertEntry(sRootEntryText);
        }

        // get the table/view names
        const ::rtl::OUString* pTables = NULL;
        const ::rtl::OUString* pViews = NULL;
        pTables = _rTables.getConstArray();
        pViews = _rViews.getConstArray();

        ::rtl::OUString sCatalog, sSchema, sName;
        SvLBoxEntry* pCat = NULL;
        SvLBoxEntry* pSchema = NULL;
        SvLBoxEntry* pParent = pAllObjects;

        // loop through both sequences  first the vies and than the tables
        const ::rtl::OUString* pSwitchSequences = (pTables && pViews) ? pViews + _rViews.getLength() - 1 : NULL;

        sal_Int32 nOverallLen = _rTables.getLength() + _rViews.getLength();
        const ::rtl::OUString* pCurrentTable = pViews ? pViews : pTables;   // currently handled view or table name
        sal_Bool bIsView = pViews ? sal_True : sal_False;   // pCurrentTable points to a view name ?
        for (   sal_Int32 i = 0;
                i < nOverallLen;
                ++i                                                                 // inc the counter
                ,   (   pSwitchSequences == pCurrentTable                           // did we reached the last table ?
                    ?   bIsView = !((pCurrentTable = pTables) != NULL)                  // yes -> continue with the views, and set bIsView to sal_True
                    :   ++pCurrentTable != NULL                                         // no -> next table
                    )                                                                   //  (!= NULL is to make this a boolean expression, so it should work under SUNPRO5, too)
            )
        {
            pCat = pSchema = NULL;
            pParent = pAllObjects;

            // the image : table or view
            Image& aImage = bIsView ? m_aViewImage : m_aTableImage;
            // split the complete name into it's components
            qualifiedNameComponents(_rxConnMetaData, *pCurrentTable, sCatalog, sSchema, sName);

            if (sCatalog.getLength())
            {
                pCat = GetEntryPosByName(sCatalog);
                if (!pCat)
                    pCat = InsertEntry(sCatalog, pParent);
                pParent = pCat;
            }

            if (sSchema.getLength())
            {
                pSchema = GetEntryPosByName(sSchema);
                if (!pSchema)
                    pSchema = InsertEntry(sSchema, pParent);
                pParent = pSchema;
            }

            if(!GetEntryPosByName(sName,pParent)) // only insert a table once
                InsertEntry(sName, aImage, aImage, pParent);
        }
    }
    catch(RuntimeException&)
    {
        DBG_ERROR("OTableTreeListBox::UpdateTableList : caught a RuntimeException!");
    }
}
//------------------------------------------------------------------------
sal_Bool OTableTreeListBox::isWildcardChecked(SvLBoxEntry* _pEntry) const
{
    if (_pEntry)
    {
        OBoldListboxString* pTextItem = static_cast<OBoldListboxString*>(_pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING));
        if (pTextItem)
            return pTextItem->isEmphasized();
    }
    return sal_False;
}

//------------------------------------------------------------------------
void OTableTreeListBox::checkWildcard(SvLBoxEntry* _pEntry)
{
    SetCheckButtonState(_pEntry, SV_BUTTON_CHECKED);
    checkedButton_noBroadcast(_pEntry);
}

//------------------------------------------------------------------------
SvLBoxEntry* OTableTreeListBox::getAllObjectsEntry() const
{
    return m_bShowFirstEntry ? First() : NULL;
}

//------------------------------------------------------------------------
void OTableTreeListBox::checkedButton_noBroadcast(SvLBoxEntry* _pEntry)
{
    OMarkableTreeListBox::checkedButton_noBroadcast(_pEntry);

    // if an entry has children, it makes a difference if the entry is checked because alls children are checked
    // or if the user checked it explicitly.
    // So we track explicit (un)checking

    SvButtonState eState = GetCheckButtonState(_pEntry);
    DBG_ASSERT(SV_BUTTON_TRISTATE != eState, "OTableTreeListBox::CheckButtonHdl: user action which lead to TRISTATE?");
    implEmphasize(_pEntry, SV_BUTTON_CHECKED == eState);
}

//------------------------------------------------------------------------
void OTableTreeListBox::implEmphasize(SvLBoxEntry* _pEntry, sal_Bool _bChecked, sal_Bool _bUpdateRelatives)
{
    DBG_ASSERT(_pEntry, "OTableTreeListBox::implEmphasize: invalid entry (NULL)!");
    if (GetModel()->HasChilds(_pEntry))
    {
        OBoldListboxString* pTextItem = static_cast<OBoldListboxString*>(_pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING));
        if (pTextItem)
            pTextItem->emphasize(_bChecked);
    }
    if (_bUpdateRelatives)
    {
        // remove the mark for all children of the checked entry
        SvLBoxEntry* pChildLoop = FirstChild(_pEntry);
        while (pChildLoop)
        {
            if (GetModel()->HasChilds(pChildLoop))
                implEmphasize(pChildLoop, sal_False, sal_False);
            pChildLoop = NextSibling(pChildLoop);
        }
        // remove the mark for all ancestors of the entry
        if (GetModel()->HasParent(_pEntry))
            implEmphasize(GetParent(_pEntry), sal_False, sal_False);
    }
}

//------------------------------------------------------------------------
void OTableTreeListBox::InitEntry(SvLBoxEntry* _pEntry, const XubString& _rString, const Image& _rCollapsedBitmap, const Image& _rExpandedBitmap)
{
    OMarkableTreeListBox::InitEntry(_pEntry, _rString, _rCollapsedBitmap, _rExpandedBitmap);

    // replace the text item with our own one
    SvLBoxItem* pTextItem = _pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING);
    DBG_ASSERT(pTextItem, "OTableTreeListBox::InitEntry: no text item!?");
    sal_uInt16 nTextPos = _pEntry->GetPos(pTextItem);
    DBG_ASSERT(((sal_uInt16)-1) != nTextPos, "OTableTreeListBox::InitEntry: no text item pos!");

    _pEntry->ReplaceItem(new OBoldListboxString(_pEntry, 0, _rString), nTextPos);
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.9  2001/02/23 15:16:28  oj
 *  use namespace
 *
 *  Revision 1.8  2001/02/05 14:44:53  oj
 *  new member for hiding first entry
 *
 *  Revision 1.7  2001/01/30 08:29:43  fs
 *  'wildcard checking'
 *
 *  Revision 1.6  2001/01/15 10:55:43  oj
 *  wrong image for table used
 *
 *  Revision 1.5  2000/10/30 15:37:48  fs
 *  #79816# no need for a XDataDefinitionSupplier anymore - collect table/view names from the meta data
 *
 *  Revision 1.4  2000/10/26 14:55:04  oj
 *  local strings for dll
 *
 *  Revision 1.3  2000/10/13 16:02:21  fs
 *  typo in error message
 *
 *  Revision 1.2  2000/10/09 12:36:50  fs
 *  reinserted the code for connecting in UpdateTableList
 *
 *  Revision 1.1  2000/10/05 10:00:59  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 28.09.00 13:32:32  fs
 ************************************************************************/

