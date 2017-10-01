/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_MACROMGR_HXX
#define INCLUDED_SC_INC_MACROMGR_HXX

#include <com/sun/star/container/XContainerListener.hpp>

#include <rtl/ustring.hxx>
#include "scdllapi.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

class ScDocument;
class ScFormulaCell;
class ScUserMacroDepTracker;

class ScMacroManager
{
public:
    explicit ScMacroManager(ScDocument* pDoc);
    ~ScMacroManager();

    SC_DLLPUBLIC void InitUserFuncData();
    SC_DLLPUBLIC void SetUserFuncVolatile( const OUString& sName, bool isVolatile );
    SC_DLLPUBLIC bool GetUserFuncVolatile( const OUString& sName );

    void AddDependentCell(const OUString& aModuleName, ScFormulaCell* pCell);
    void RemoveDependentCell(const ScFormulaCell* pCell);
    void BroadcastModuleUpdate(const OUString& aModuleName);

private:
    typedef std::unordered_map< OUString, bool, OUStringHash > NameBoolMap;
    NameBoolMap mhFuncToVolatile;
    css::uno::Reference< css::container::XContainerListener > mxContainerListener;

    ::std::unique_ptr<ScUserMacroDepTracker> mpDepTracker;
    ScDocument* mpDoc;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
