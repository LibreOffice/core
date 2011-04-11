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
