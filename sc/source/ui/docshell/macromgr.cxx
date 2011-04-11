/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include "macromgr.hxx"
#include "document.hxx"

#include "basic/basmgr.hxx"
#include "cppuhelper/implbase1.hxx"
#include "sfx2/objsh.hxx"
#include "cell.hxx"
#include <com/sun/star/container/XContainer.hpp>

#include <list>

using namespace ::com::sun::star;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::rtl::OUString;
using ::rtl::OUStringHash;
using ::boost::unordered_map;
using ::std::list;
using ::std::for_each;
using ::std::pair;

// ============================================================================

/**
 * A simple container to keep track of cells that depend on basic modules
 * changes.  We don't check for duplicates at insertion time; instead, we
 * remove duplicates at query time.
 */
class ScUserMacroDepTracker
{
public:
    void addCell(const OUString& rModuleName, ScFormulaCell* pCell)
    {
        ModuleCellMap::iterator itr = maCells.find(rModuleName);
        if (itr == maCells.end())
        {
            pair<ModuleCellMap::iterator, bool> r = maCells.insert(
                ModuleCellMap::value_type(rModuleName, list<ScFormulaCell*>()));

            if (!r.second)
                // insertion failed.
                return;

            itr = r.first;
        }
        itr->second.push_back(pCell);
    }

    void removeCell(ScFormulaCell* pCell)
    {
        ModuleCellMap::iterator itr = maCells.begin(), itrEnd = maCells.end();
        for (; itr != itrEnd; ++itr)
            itr->second.remove(pCell);
    }

    void getCellsByModule(const OUString& rModuleName, list<ScFormulaCell*>& rCells)
    {
        ModuleCellMap::iterator itr = maCells.find(rModuleName);
        if (itr == maCells.end())
            return;

        list<ScFormulaCell*>& rCellList = itr->second;

        // Remove duplicates.
        rCellList.sort();
        rCellList.unique();
        // exception safe copy
        list<ScFormulaCell*> temp(rCellList);
        rCells.swap(temp);
    }

private:
    typedef boost::unordered_map<OUString, list<ScFormulaCell*>, OUStringHash> ModuleCellMap;
    ModuleCellMap maCells;
};


// ============================================================================

ScMacroManager::ScMacroManager(ScDocument* pDoc) :
    mpDepTracker(new ScUserMacroDepTracker),
    mpDoc(pDoc)
{
}

ScMacroManager::~ScMacroManager()
{
}

typedef ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener > ContainerListenerHelper;

class VBAProjectListener : public ContainerListenerHelper
{
    ScMacroManager* mpMacroMgr;
public:
    VBAProjectListener( ScMacroManager* pMacroMgr ) : mpMacroMgr( pMacroMgr ) {}
    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& /*Source*/ ) throw(RuntimeException) {}

    // XContainerListener
    virtual void SAL_CALL elementInserted( const container::ContainerEvent& /*Event*/ ) throw(RuntimeException){}
    virtual void SAL_CALL elementReplaced( const container::ContainerEvent& Event ) throw(RuntimeException)
    {
        rtl::OUString sModuleName;
        Event.Accessor >>= sModuleName;
        OSL_TRACE("VBAProjectListener::elementReplaced(%s)", rtl::OUStringToOString( sModuleName, RTL_TEXTENCODING_UTF8 ).getStr() );
        mpMacroMgr->InitUserFuncData();
        mpMacroMgr->BroadcastModuleUpdate(sModuleName);
    }
    virtual void SAL_CALL elementRemoved( const container::ContainerEvent& /*Event*/ ) throw(RuntimeException){}

};

void ScMacroManager::InitUserFuncData()
{
    // Clear boost::unordered_map
    mhFuncToVolatile.clear();
    String sProjectName( RTL_CONSTASCII_USTRINGPARAM("Standard") );

    Reference< container::XContainer > xModuleContainer;
    SfxObjectShell* pShell = mpDoc->GetDocumentShell();
    if ( pShell && pShell->GetBasicManager()->GetName().Len() > 0 )
        sProjectName = pShell->GetBasicManager()->GetName();
    try
    {
        Reference< script::XLibraryContainer > xLibraries( pShell->GetBasicContainer(), uno::UNO_QUERY_THROW );
        xModuleContainer.set( xLibraries->getByName( sProjectName ), uno::UNO_QUERY_THROW );

        if ( xModuleContainer.is() )
        {
            // remove old listener ( if there was one )
            if ( mxContainerListener.is() )
                xModuleContainer->removeContainerListener( mxContainerListener );
            // Create listener
            mxContainerListener = new VBAProjectListener( this );
            xModuleContainer->addContainerListener( mxContainerListener );
        }
    }
    catch( uno::Exception& )
    {
    }
}

void ScMacroManager::SetUserFuncVolatile( const OUString& sName, bool isVolatile )
{
    mhFuncToVolatile[ sName ] = isVolatile;
}

bool ScMacroManager::GetUserFuncVolatile( const OUString& sName )
{
    NameBoolMap::iterator it = mhFuncToVolatile.find( sName );
    if ( it == mhFuncToVolatile.end() )
        return false;
    return it->second;
}

void ScMacroManager::AddDependentCell(const OUString& aModuleName, ScFormulaCell* pCell)
{
    mpDepTracker->addCell(aModuleName, pCell);
}

void ScMacroManager::RemoveDependentCell(ScFormulaCell* pCell)
{
    mpDepTracker->removeCell(pCell);
}

void ScMacroManager::BroadcastModuleUpdate(const OUString& aModuleName)
{
    list<ScFormulaCell*> aCells;
    mpDepTracker->getCellsByModule(aModuleName, aCells);
    list<ScFormulaCell*>::iterator itr = aCells.begin(), itrEnd = aCells.end();
    for (; itr != itrEnd; ++itr)
    {
        ScFormulaCell* pCell = *itr;
        mpDoc->PutInFormulaTree(pCell); // for F9 recalc

        // for recalc on cell value change.  If the cell is not volatile, the
        // cell stops listening right away after it gets re-interpreted.
        mpDoc->StartListeningArea(BCA_LISTEN_ALWAYS, pCell);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
