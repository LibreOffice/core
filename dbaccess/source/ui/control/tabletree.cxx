/*************************************************************************
 *
 *  $RCSfile: tabletree.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:37:10 $
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
#ifndef _DBU_CONTROL_HRC_
#include "dbu_control.hrc"
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
OTableTreeListBox::OTableTreeListBox( Window* pParent, const Reference< XMultiServiceFactory >& _rxORB, sal_Bool _bHiContrast,WinBits nWinStyle,sal_Bool _bVirtualRoot )
    :OMarkableTreeListBox(pParent,_rxORB,nWinStyle)
    ,m_bVirtualRoot(_bVirtualRoot)
{
    notifyHiContrastChanged();
}
//------------------------------------------------------------------------
OTableTreeListBox::OTableTreeListBox( Window* pParent, const Reference< XMultiServiceFactory >& _rxORB, const ResId& rResId ,sal_Bool _bHiContrast,sal_Bool _bVirtualRoot)
    :OMarkableTreeListBox(pParent,_rxORB,rResId)
    ,m_bVirtualRoot(_bVirtualRoot)
{
    notifyHiContrastChanged();
}
// -----------------------------------------------------------------------------
void OTableTreeListBox::notifyHiContrastChanged()
{
    sal_Bool bHiContrast = GetBackground().GetColor().IsDark();
    m_aTableImage = Image(ModuleRes(bHiContrast ? TABLE_TREE_ICON_SCH : TABLE_TREE_ICON));
    m_aViewImage = Image(ModuleRes(bHiContrast ? VIEW_TREE_ICON_SCH : VIEW_TREE_ICON));

    SetDefaultExpandedEntryBmp(Image(ModuleRes(bHiContrast ? TABLEFOLDER_TREE_ICON_SCH : TABLEFOLDER_TREE_ICON)));
    SetDefaultCollapsedEntryBmp(Image(ModuleRes(bHiContrast ? TABLEFOLDER_TREE_ICON_SCH : TABLEFOLDER_TREE_ICON)));

    SvLBoxEntry* pEntryLoop = First();
    while (pEntryLoop)
    {
        sal_Int32 nType = reinterpret_cast<sal_Int32>(pEntryLoop->GetUserData());
        if ( nType )
        {
            USHORT nCount = pEntryLoop->ItemCount();
            for (USHORT i=0;i<nCount;++i)
            {
                SvLBoxItem* pItem = pEntryLoop->GetItem(i);
                if ( pItem && pItem->IsA() == SV_ITEM_ID_LBOXCONTEXTBMP)
                {
                    Image aImage;
                    switch( nType )
                    {
                        case TABLE_TYPE:    aImage = m_aTableImage; break;
                        case VIEW_TYPE:     aImage = m_aViewImage;  break;
                        default:            aImage = Image(ModuleRes(bHiContrast ? TABLEFOLDER_TREE_ICON_SCH : TABLEFOLDER_TREE_ICON));
                    }
                    static_cast<SvLBoxContextBmp*>(pItem)->SetBitmap1(pEntryLoop,aImage);
                    static_cast<SvLBoxContextBmp*>(pItem)->SetBitmap2(pEntryLoop,aImage);
                    break;
                }
            }
        }
        pEntryLoop = Next(pEntryLoop);
    }
}
//------------------------------------------------------------------------
Reference< XConnection > OTableTreeListBox::UpdateTableList( const ::rtl::OUString& _rConnectionURL,
    const Sequence< PropertyValue > _rProperties, Reference< XDriver >& _rxCreator ) throw(SQLException)
{
    Reference< XDatabaseMetaData > xMetaData;
    Reference< XConnection > xConnection;
    _rxCreator.clear();

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
            sCurrentActionError.SearchAndReplaceAscii("#servicename#", (::rtl::OUString)SERVICE_SDBC_CONNECTIONPOOL);
            try
            {
                xDriverManager = Reference< XDriverAccess >(m_xORB->createInstance(SERVICE_SDBC_CONNECTIONPOOL), UNO_QUERY);
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
            _rxCreator = xDriverManager->getDriverByURL(_rConnectionURL);
            if (!_rxCreator.is())
                // will be caught and translated into an SQLContext exception
                throw Exception();

            sCurrentActionError = String(ModuleRes(STR_COULDNOTCONNECT));
            sCurrentActionDetails = String(ModuleRes(STR_COULDNOTCONNECT_PLEASECHECK));
            xConnection = _rxCreator->connect(_rConnectionURL, _rProperties);
                // exceptions thrown by connect will be caught and re-routed
            DBG_ASSERT(xConnection.is(), "OTableTreeListBox::UpdateTableList : got an invalid connection!");
                // if no exception was thrown, the connection should be no-NULL)
            if (!xConnection.is())
                throw Exception();
            sCurrentActionDetails = String();
            xMetaData = xConnection->getMetaData();

            // get the (very necessary) interface XDataDefinitionSupplier
            Reference< XDataDefinitionSupplier > xDefinitionAccess(_rxCreator, UNO_QUERY);
            if (!xDefinitionAccess.is())
            {
                // okay, the driver's programmer was lazy :)
                // let's do it on foot .... (ask the meta data for a result set describing the tables)
                Reference< XResultSet > xTables;

                static const ::rtl::OUString s_sTableTypeView(RTL_CONSTASCII_USTRINGPARAM("VIEW"));
                static const ::rtl::OUString s_sTableTypeTable(RTL_CONSTASCII_USTRINGPARAM("TABLE"));

                if (xMetaData.is())
                {
                    // we want all catalogues, all schemas, all tables
                    Sequence< ::rtl::OUString > sTableTypes(3);
                    static const ::rtl::OUString sWildcard = ::rtl::OUString::createFromAscii("%");
                    sTableTypes[0] = s_sTableTypeView;
                    sTableTypes[1] = s_sTableTypeTable;
                    sTableTypes[2] = sWildcard; // just to be sure to include anything else ....

                    xTables = xMetaData->getTables(Any(), sWildcard, sWildcard, sTableTypes);
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
                        ::dbtools::composeTableName(xMetaData, sCatalog, sSchema, sName, sComposedName, sal_False,::dbtools::eInDataManipulation);
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
                xViewSupp.set(xTableSupp,UNO_QUERY);
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

    try
    {
        // the root entry saying "all objects"
        SvLBoxEntry* pAllObjects = NULL;
        if (haveVirtualRoot())
        {
            String sRootEntryText;
            if (!_rViews.getLength())
                sRootEntryText  =String(ModuleRes(STR_ALL_TABLES));
            else if (!_rTables.getLength())
                sRootEntryText  =String(ModuleRes(STR_ALL_VIEWS));
            else
                sRootEntryText  =String(ModuleRes(STR_ALL_TABLES_AND_VIEWS));
            pAllObjects = InsertEntry(sRootEntryText,NULL,FALSE,LIST_APPEND,reinterpret_cast<void*>(FOLDER_TYPE));
        }

        if (!_rTables.getLength() && !_rViews.getLength())
            // nothing to do (besides inserting the root entry)
            return;

        // get the table/view names
        const ::rtl::OUString* pTables = NULL;
        const ::rtl::OUString* pViews = NULL;
        if(_rTables.getLength())
            pTables = _rTables.getConstArray();
        if(_rViews.getLength())
            pViews = _rViews.getConstArray();

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
            // add the entry
            implAddEntry(
                _rxConnMetaData,
                *pCurrentTable,
                bIsView ? m_aViewImage : m_aTableImage,
                pAllObjects,
                bIsView ? VIEW_TYPE : TABLE_TYPE,
                sal_False
            );
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
    return haveVirtualRoot() ? First() : NULL;
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
void OTableTreeListBox::implEmphasize(SvLBoxEntry* _pEntry, sal_Bool _bChecked, sal_Bool _bUpdateDescendants, sal_Bool _bUpdateAncestors)
{
    DBG_ASSERT(_pEntry, "OTableTreeListBox::implEmphasize: invalid entry (NULL)!");

    // special emphasizing handling for the "all objects" entry
    // 89709 - 16.07.2001 - frank.schoenheit@sun.com
    sal_Bool bAllObjectsEntryAffected = haveVirtualRoot() && (getAllObjectsEntry() == _pEntry);
    if  (   GetModel()->HasChilds(_pEntry)              // the entry has children
        ||  bAllObjectsEntryAffected                    // or it is the "all objects" entry
        )
    {
        OBoldListboxString* pTextItem = static_cast<OBoldListboxString*>(_pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING));
        if (pTextItem)
            pTextItem->emphasize(_bChecked);

        if (bAllObjectsEntryAffected)
            InvalidateEntry(_pEntry);
    }

    if (_bUpdateDescendants)
    {
        // remove the mark for all children of the checked entry
        SvLBoxEntry* pChildLoop = FirstChild(_pEntry);
        while (pChildLoop)
        {
            if (GetModel()->HasChilds(pChildLoop))
                implEmphasize(pChildLoop, sal_False, sal_True, sal_False);
            pChildLoop = NextSibling(pChildLoop);
        }
    }

    if (_bUpdateAncestors)
    {
        // remove the mark for all ancestors of the entry
        if (GetModel()->HasParent(_pEntry))
            implEmphasize(GetParent(_pEntry), sal_False, sal_False, sal_True);
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

//------------------------------------------------------------------------
SvLBoxEntry* OTableTreeListBox::implAddEntry(
        const Reference< XDatabaseMetaData >& _rxConnMetaData,
        const ::rtl::OUString& _rTableName,
        const Image& _rImage,
        SvLBoxEntry* _pParentEntry,
        sal_Int32 _nType,
        sal_Bool _bCheckName
    )
{
    // split the complete name into it's components
    ::rtl::OUString sCatalog, sSchema, sName;
    qualifiedNameComponents(_rxConnMetaData, _rTableName, sCatalog, sSchema, sName,::dbtools::eInDataManipulation);

    SvLBoxEntry* pCat = NULL;
    SvLBoxEntry* pSchema = NULL;
    if (sCatalog.getLength())
    {
        pCat = GetEntryPosByName(sCatalog, _pParentEntry);
        if (!pCat)
            pCat = InsertEntry(sCatalog, _pParentEntry,FALSE,LIST_APPEND,reinterpret_cast<void*>(FOLDER_TYPE));
        _pParentEntry = pCat;
    }

    if (sSchema.getLength())
    {
        pSchema = GetEntryPosByName(sSchema, _pParentEntry);
        if (!pSchema)
            pSchema = InsertEntry(sSchema, _pParentEntry,FALSE,LIST_APPEND,reinterpret_cast<void*>(FOLDER_TYPE));
        _pParentEntry = pSchema;
    }

    SvLBoxEntry* pRet = NULL;
    if ( !_bCheckName || !GetEntryPosByName(sName, _pParentEntry))
        pRet = InsertEntry(sName, _rImage, _rImage, _pParentEntry,FALSE,LIST_APPEND,reinterpret_cast<void*>(_nType));
    return pRet;
}

//------------------------------------------------------------------------
SvLBoxEntry* OTableTreeListBox::addedTable( const Reference< XConnection >& _rxConn, const ::rtl::OUString& _rName, const Any& _rObject )
{
    try
    {
        // get the connection meta data
        Reference< XDatabaseMetaData > xMeta;
        if (_rxConn.is()) xMeta = _rxConn->getMetaData();
        if (!xMeta.is())
        {
            DBG_ERROR( "OTableTreeListBox::addedTable: invalid connection!" );
            return NULL;
        }

        // add the entry
        return implAddEntry( xMeta, _rName, m_aTableImage, getAllObjectsEntry(),TABLE_TYPE );
            // TODO: the image
    }
    catch( const Exception& )
    {
        DBG_ERROR( "OTableTreeListBox::addedTable: caught an exception!" );
    }
    return NULL;
}

//------------------------------------------------------------------------
SvLBoxEntry* OTableTreeListBox::getEntryByQualifiedName( const Reference< XConnection >& _rxConn, const ::rtl::OUString& _rName )
{
    try
    {
        // get the connection meta data
        Reference< XDatabaseMetaData > xMeta;
        if (_rxConn.is()) xMeta = _rxConn->getMetaData();
        if (!xMeta.is())
        {
            DBG_ERROR( "OTableTreeListBox::removedTable: invalid connection!" );
            return NULL;
        }

        // split the complete name into it's components
        ::rtl::OUString sCatalog, sSchema, sName;
        qualifiedNameComponents(xMeta, _rName, sCatalog, sSchema, sName,::dbtools::eInDataManipulation);

        SvLBoxEntry* pParent = getAllObjectsEntry();
        SvLBoxEntry* pCat = NULL;
        SvLBoxEntry* pSchema = NULL;
        if ( sCatalog.getLength() )
        {
            pCat = GetEntryPosByName(sCatalog, pParent);
            if ( pCat )
                pParent = pCat;
        }

        if ( sSchema.getLength() )
        {
            pSchema = GetEntryPosByName(sSchema, pParent);
            if ( pSchema )
                pParent = pSchema;
        }

        return GetEntryPosByName(sName, pParent);
    }
    catch( const Exception& )
    {
        DBG_ERROR( "OTableTreeListBox::removedTable: caught an exception!" );
    }
    return NULL;
}
//------------------------------------------------------------------------
void OTableTreeListBox::removedTable( const Reference< XConnection >& _rxConn, const ::rtl::OUString& _rName )
{
    try
    {
        SvLBoxEntry* pEntry = getEntryByQualifiedName(_rxConn,_rName);
        if ( pEntry )
            GetModel()->Remove(pEntry);
    }
    catch( const Exception& )
    {
        DBG_ERROR( "OTableTreeListBox::removedTable: caught an exception!" );
    }
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

