/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_MACROMGR_HXX
#define SC_MACROMGR_HXX

#include <com/sun/star/container/XContainerListener.hpp>

#include "rtl/ustring.hxx"
#include "scdllapi.h"

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <memory>

class ScDocument;
class ScFormulaCell;
class ScUserMacroDepTracker;

class ScMacroManager
{
public:
    explicit ScMacroManager(ScDocument* pDoc);
    ~ScMacroManager();

    SC_DLLPUBLIC void InitUserFuncData();
    SC_DLLPUBLIC void SetUserFuncVolatile( const rtl::OUString& sName, bool isVolatile );
    SC_DLLPUBLIC bool GetUserFuncVolatile( const rtl::OUString& sName );

    void AddDependentCell(const ::rtl::OUString& aModuleName, ScFormulaCell* pCell);
    void RemoveDependentCell(ScFormulaCell* pCell);
    void BroadcastModuleUpdate(const ::rtl::OUString& aModuleName);

private:
    typedef boost::unordered_map< ::rtl::OUString, bool, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > NameBoolMap;
    NameBoolMap mhFuncToVolatile;
    com::sun::star::uno::Reference< com::sun::star::container::XContainerListener > mxContainerListener;

    ::std::auto_ptr<ScUserMacroDepTracker> mpDepTracker;
    ScDocument* mpDoc;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
