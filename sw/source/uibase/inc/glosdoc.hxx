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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_GLOSDOC_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_GLOSDOC_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/text/XAutoTextGroup.hpp>

class SwTextBlocks;
class SwDocShell;

#ifndef SW_DECL_SWDOCSHELL_DEFINED
#define SW_DECL_SWDOCSHELL_DEFINED
#include <tools/ref.hxx>
typedef tools::SvRef<SwDocShell> SwDocShellRef;
#endif
#include <cppuhelper/weakref.hxx>

#include <vector>
#include "swdllapi.h"

typedef ::std::vector< css::uno::WeakReference< css::text::XAutoTextGroup > > UnoAutoTextGroups;
typedef ::std::vector< css::uno::WeakReference< css::text::XAutoTextEntry > > UnoAutoTextEntries;

#define GLOS_DELIM (sal_Unicode)'*'

class SW_DLLPUBLIC SwGlossaries
{
    UnoAutoTextGroups       m_aGlossaryGroups;
    UnoAutoTextEntries      m_aGlossaryEntries;

    OUString                m_aPath;
    std::vector<OUString>   m_aInvalidPaths;
    std::vector<OUString>   m_PathArr;
    std::vector<OUString>   m_GlosArr;
    bool                m_bError;

    SAL_DLLPRIVATE SwTextBlocks* GetGlosDoc(const OUString &rName, bool bCreate = true) const;
    SAL_DLLPRIVATE std::vector<OUString> & GetNameList();

    // implementation in unoatxt.cxx
    SAL_DLLPRIVATE void RemoveFileFromList( const OUString& rGroup );
    SAL_DLLPRIVATE void InvalidateUNOOjects();

public:
    SwGlossaries();
    ~SwGlossaries();

    /** returns the cached AutoTextGroup (if any) for the given group name

        @precond
            If <arg>_bCreate</arg> is <TRUE/>, the SolarMutex must be locked when calling into this method.

        @param _rGroupName
            the name of the glossaries group
        @param _bCreate
            if <TRUE/>, the group is created if it does not yet exist
    */
    css::uno::Reference< css::text::XAutoTextGroup >
                            GetAutoTextGroup(
                                const OUString& _rGroupName,
                                bool _bCreate = false
                            );

    /** returns the cached AutoTextEntry (if any) for the given group/with the given name

        @precond
            If <arg>_bCreate</arg> is <TRUE/>, the SolarMutex must be locked when calling into this method.

        @param _rGroupAccessName
            the name to access the group
        @param _rGroupName
            the name of the glossaries group, as to be passed to the entry
        @param _rEntryName
            the name of the auto text entry
        @param _bCreate
            if <TRUE/>, the entry is created if it does not yet exist
    */
    css::uno::Reference< css::text::XAutoTextEntry >
                            GetAutoTextEntry(
                                const OUString& _rCompleteGroupName,
                                const OUString& _rGroupName,
                                const OUString& _rEntryName,
                                bool _bCreate = false
                            );

    size_t          GetGroupCnt();
    OUString        GetGroupName(size_t);
    OUString        GetGroupTitle( const OUString& rGroupName );

    bool            FindGroupName(OUString& rGroup);

    SwTextBlocks*   GetGroupDoc(const OUString &rName,
                                bool bCreate = false);
    static OUString GetDefName();
    static OUString GetExtension();

    OUString        GetCompleteGroupName( const OUString& GroupName );

    bool            NewGroupDoc(OUString &rGroupName, const OUString& rTitle);
    bool            RenameGroupDoc(const OUString& sOldGroup, OUString& sNewGroup, const OUString& rNewTitle);
    bool            DelGroupDoc(const OUString &);
    SwDocShellRef   EditGroupDoc(const OUString &rGrpName, const OUString& rShortName, bool bShow = true );
    void            UpdateGlosPath(bool bFull);
    void            ShowError();
    bool            IsGlosPathErr() { return m_bError; }
    std::vector<OUString> const& GetPathArray() const { return m_PathArr; }
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_GLOSDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
