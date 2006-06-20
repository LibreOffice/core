/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabletree.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:00:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif
#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif
#include <algorithm>

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
OTableTreeListBox::OTableTreeListBox( Window* pParent, const Reference< XMultiServiceFactory >& _rxORB, WinBits nWinStyle,sal_Bool _bVirtualRoot )
    :OMarkableTreeListBox(pParent,_rxORB,nWinStyle)
    ,m_bVirtualRoot(_bVirtualRoot)
{
    notifyHiContrastChanged();
}
//------------------------------------------------------------------------
OTableTreeListBox::OTableTreeListBox( Window* pParent, const Reference< XMultiServiceFactory >& _rxORB, const ResId& rResId ,sal_Bool _bVirtualRoot)
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
        sal_Int32 nType = reinterpret_cast<sal_IntPtr>(pEntryLoop->GetUserData());
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
                    static_cast<SvLBoxContextBmp*>(pItem)->SetBitmap1(aImage);
                    static_cast<SvLBoxContextBmp*>(pItem)->SetBitmap2(aImage);
                    break;
                }
            }
        }
        pEntryLoop = Next(pEntryLoop);
    }
}
//------------------------------------------------------------------------
void OTableTreeListBox::UpdateTableList( const Reference< XConnection >& _xConnection ) throw(SQLException)
{
    Reference< XDatabaseMetaData > xMetaData;

    Sequence< ::rtl::OUString > sTables, sViews;
    DBG_ASSERT(m_xORB.is(), "OTableTreeListBox::UpdateTableList : please use setServiceFactory to give me a service factory !");

    String sCurrentActionError;
    String sCurrentActionDetails;
    try
    {
        if (m_xORB.is())
        {

            sCurrentActionDetails = String();
            xMetaData = _xConnection->getMetaData();

            Reference< XTablesSupplier > xTableSupp(_xConnection,UNO_QUERY);
            sCurrentActionError = String(ModuleRes(STR_NOTABLEINFO));

            Reference< XNameAccess > xTables,xViews;

            xTables = xTableSupp->getTables();
            Reference< XViewsSupplier > xViewSupp(xTableSupp,UNO_QUERY);
            if ( xViewSupp.is() )
            {
                xViews = xViewSupp->getViews();
                if (xViews.is())
                    sViews = xViews->getElementNames();
            }

            if (xTables.is())
                sTables = xTables->getElementNames();
        }
    }
    catch(RuntimeException&)
    {
        DBG_ERROR("OTableTreeListBox::UpdateTableList : caught an RuntimeException!");
    }
    catch(Exception&)
    {
        // a non-SQLException exception occured ... simply throw an SQLContext
        SQLContext aExtendedInfo;
        throw aExtendedInfo;
    }

    UpdateTableList(xMetaData, sTables,sViews);
}
// -----------------------------------------------------------------------------
namespace
{
    struct OViewSetter : public ::std::unary_function< OTableTreeListBox::TNames::value_type, bool>
    {
        const Sequence< ::rtl::OUString> m_aViews;
        ::comphelper::TStringMixEqualFunctor m_aEqualFunctor;

        OViewSetter(const Sequence< ::rtl::OUString>& _rViews,sal_Bool _bCase) : m_aViews(_rViews),m_aEqualFunctor(_bCase){}
        OTableTreeListBox::TNames::value_type operator() (const ::rtl::OUString& lhs)
        {
            OTableTreeListBox::TNames::value_type aRet;
            aRet.first = lhs;
            const ::rtl::OUString* pIter = m_aViews.getConstArray();
            const ::rtl::OUString* pEnd = m_aViews.getConstArray() + m_aViews.getLength();
            aRet.second = (::std::find_if(pIter,pEnd,::std::bind2nd(m_aEqualFunctor,lhs)) != pEnd);

            return aRet;
        }
    };

}
// -----------------------------------------------------------------------------
void OTableTreeListBox::UpdateTableList(
                const Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData,
                const Sequence< ::rtl::OUString>& _rTables,
                const Sequence< ::rtl::OUString>& _rViews
            )
{
    TNames aTables;
    aTables.resize(_rTables.getLength());
    const ::rtl::OUString* pIter = _rTables.getConstArray();
    const ::rtl::OUString* pEnd = _rTables.getConstArray() + _rTables.getLength();
    try
    {
        ::std::transform(pIter,pEnd,aTables.begin(),OViewSetter(_rViews,_rxConnMetaData.is() ? _rxConnMetaData->storesMixedCaseQuotedIdentifiers() : sal_False));
    }
    catch(Exception&)
    {
    }
    UpdateTableList(_rxConnMetaData,aTables);
}
//------------------------------------------------------------------------
void OTableTreeListBox::UpdateTableList(const Reference< XDatabaseMetaData >& _rxConnMetaData, const TNames& _rTables)
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
            TNames::const_iterator aViews = ::std::find_if(_rTables.begin(),_rTables.end(),
            ::std::compose1(::std::bind2nd(::std::equal_to<sal_Bool>(),sal_False),::std::select2nd<TNames::value_type>()));
            TNames::const_iterator aTables = ::std::find_if(_rTables.begin(),_rTables.end(),
            ::std::compose1(::std::bind2nd(::std::equal_to<sal_Bool>(),sal_True),::std::select2nd<TNames::value_type>()));

            if ( aViews == _rTables.end() )
                sRootEntryText  = String(ModuleRes(STR_ALL_TABLES));
            else if ( aTables == _rTables.end() )
                sRootEntryText  = String(ModuleRes(STR_ALL_VIEWS));
            else
                sRootEntryText  = String(ModuleRes(STR_ALL_TABLES_AND_VIEWS));
            pAllObjects = InsertEntry(sRootEntryText,NULL,FALSE,LIST_APPEND,reinterpret_cast<void*>(FOLDER_TYPE));
        }

        if ( _rTables.empty() )
            // nothing to do (besides inserting the root entry)
            return;

        // get the table/view names
        TNames::const_iterator aIter = _rTables.begin();
        TNames::const_iterator aEnd = _rTables.end();

        for ( ; aIter != aEnd; ++aIter )
        {
            // add the entry
            implAddEntry(
                _rxConnMetaData,
                aIter->first,
                aIter->second ? m_aViewImage : m_aTableImage,
                pAllObjects,
                aIter->second ? VIEW_TYPE : TABLE_TYPE,
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

        sal_Int32 nType = TABLE_TYPE;
        Reference<XPropertySet> xProp(_rObject,UNO_QUERY);
        if ( xProp.is() )
        {
            ::rtl::OUString sValue;
            xProp->getPropertyValue(PROPERTY_TYPE) >>= sValue;
            if ( sValue.equalsAscii("VIEW") )
                nType = VIEW_TYPE;
        }
        // add the entry
        return implAddEntry( xMeta, _rName, nType == TABLE_TYPE ? m_aTableImage : m_aViewImage, getAllObjectsEntry(),nType );
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

