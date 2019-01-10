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

#include <macromgr.hxx>
#include <document.hxx>

#include <basic/basmgr.hxx>
#include <cppuhelper/implbase.hxx>
#include <sfx2/objsh.hxx>
#include <formulacell.hxx>
#include <vector>
#include <com/sun/star/container/XContainer.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::std::vector;
using ::std::pair;

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
            pair<ModuleCellMap::iterator, bool> r = maCells.emplace(
                rModuleName, vector<ScFormulaCell*>());

            if (!r.second)
                // insertion failed.
                return;

            itr = r.first;
        }
        itr->second.push_back(pCell);
    }

    void removeCell(const ScFormulaCell* pCell)
    {
        for (auto& rEntry : maCells)
        {
            rEntry.second.erase(std::remove(rEntry.second.begin(), rEntry.second.end(), pCell), rEntry.second.end() );
        }
    }

    void getCellsByModule(const OUString& rModuleName, vector<ScFormulaCell*>& rCells)
    {
        ModuleCellMap::iterator itr = maCells.find(rModuleName);
        if (itr == maCells.end())
            return;

        vector<ScFormulaCell*>& rCellList = itr->second;

        // Remove duplicates.
        std::sort(rCellList.begin(), rCellList.end());
        auto last = std::unique(rCellList.begin(), rCellList.end());
        rCellList.erase(last, rCellList.end());

        // exception safe copy
        vector<ScFormulaCell*> temp(rCellList);
        rCells.swap(temp);
    }

private:
    typedef std::unordered_map<OUString, vector<ScFormulaCell*>> ModuleCellMap;
    ModuleCellMap maCells;
};

ScMacroManager::ScMacroManager(ScDocument* pDoc) :
    mpDepTracker(new ScUserMacroDepTracker),
    mpDoc(pDoc)
{
}

ScMacroManager::~ScMacroManager()
{
}

typedef ::cppu::WeakImplHelper< css::container::XContainerListener > ContainerListenerHelper;

class VBAProjectListener : public ContainerListenerHelper
{
    ScMacroManager* mpMacroMgr;
public:
    explicit VBAProjectListener( ScMacroManager* pMacroMgr ) : mpMacroMgr( pMacroMgr ) {}
    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& /*Source*/ ) override {}

    // XContainerListener
    virtual void SAL_CALL elementInserted( const container::ContainerEvent& /*Event*/ ) override {}
    virtual void SAL_CALL elementReplaced( const container::ContainerEvent& Event ) override
    {
        OUString sModuleName;
        Event.Accessor >>= sModuleName;
        mpMacroMgr->InitUserFuncData();
        mpMacroMgr->BroadcastModuleUpdate(sModuleName);
    }
    virtual void SAL_CALL elementRemoved( const container::ContainerEvent& /*Event*/ ) override {}

};

void ScMacroManager::InitUserFuncData()
{
    // Clear unordered_map
    mhFuncToVolatile.clear();
    OUString sProjectName("Standard");

    Reference< container::XContainer > xModuleContainer;
    SfxObjectShell* pShell = mpDoc->GetDocumentShell();
    if (!pShell)
        return;
    if (!pShell->GetBasicManager()->GetName().isEmpty())
    {
        sProjectName = pShell->GetBasicManager()->GetName();
    }
    try
    {
        Reference< script::XLibraryContainer > xLibraries( pShell->GetBasicContainer(), uno::UNO_QUERY_THROW );
        xModuleContainer.set( xLibraries->getByName( sProjectName ), uno::UNO_QUERY_THROW );

        // remove old listener ( if there was one )
        if ( mxContainerListener.is() )
            xModuleContainer->removeContainerListener( mxContainerListener );
        // Create listener
        mxContainerListener = new VBAProjectListener( this );
        xModuleContainer->addContainerListener( mxContainerListener );
    }
    catch (const uno::Exception&)
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

void ScMacroManager::RemoveDependentCell(const ScFormulaCell* pCell)
{
    mpDepTracker->removeCell(pCell);
}

void ScMacroManager::BroadcastModuleUpdate(const OUString& aModuleName)
{
    vector<ScFormulaCell*> aCells;
    mpDepTracker->getCellsByModule(aModuleName, aCells);
    for (ScFormulaCell* pCell : aCells)
    {
        mpDoc->PutInFormulaTree(pCell); // for F9 recalc

        // for recalc on cell value change.  If the cell is not volatile, the
        // cell stops listening right away after it gets re-interpreted.
        mpDoc->StartListeningArea(BCA_LISTEN_ALWAYS, false, pCell);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
