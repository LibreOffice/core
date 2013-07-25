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
#ifndef _GLOSDOC_HXX
#define _GLOSDOC_HXX

#include <tools/string.hxx>
#include <com/sun/star/text/XAutoTextGroup.hpp>

class SwTextBlocks;
class SwDocShell;

#ifndef SW_DECL_SWDOCSHELL_DEFINED
#define SW_DECL_SWDOCSHELL_DEFINED
#include <tools/ref.hxx>
SV_DECL_REF( SwDocShell )
#endif
#include <cppuhelper/weakref.hxx>

#include <vector>
#include "swdllapi.h"

typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XAutoTextGroup > AutoTextGroupRef;
typedef ::std::vector< AutoTextGroupRef > UnoAutoTextGroups;

typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XAutoTextEntry > AutoTextEntryRef;
typedef ::std::vector< AutoTextEntryRef > UnoAutoTextEntries;

#define GLOS_DELIM (sal_Unicode)'*'

// CLASS -----------------------------------------------------------------
class SW_DLLPUBLIC SwGlossaries
{
    UnoAutoTextGroups       m_aGlossaryGroups;
    UnoAutoTextEntries      m_aGlossaryEntries;

    String                  m_aPath;
    std::vector<rtl::OUString> m_aInvalidPaths;
    std::vector<String>     m_PathArr;
    std::vector<String>     m_GlosArr;
    sal_Bool                m_bError;

    SW_DLLPRIVATE SwTextBlocks* GetGlosDoc(const String &rName, sal_Bool bCreate = sal_True) const;
    SW_DLLPRIVATE std::vector<String> & GetNameList();

    // implementation in unoatxt.cxx
    SW_DLLPRIVATE void RemoveFileFromList( const String& rGroup );
    SW_DLLPRIVATE void InvalidateUNOOjects();

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
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XAutoTextGroup >
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
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XAutoTextEntry >
                            GetAutoTextEntry(
                                const String& _rCompleteGroupName,
                                const OUString& _rGroupName,
                                const OUString& _rEntryName,
                                bool _bCreate = false
                            );

    sal_uInt16                  GetGroupCnt();
    String                  GetGroupName(sal_uInt16 );
    String                  GetGroupTitle( const String& rGroupName );

    sal_Bool            FindGroupName(OUString& rGroup);

    SwTextBlocks*   GetGroupDoc(const String &rName,
                                sal_Bool bCreate = sal_False);
    SwTextBlocks*   GetDefGroupDoc() { return GetGroupDoc(GetDefName()); }
    void            PutGroupDoc(SwTextBlocks *pBlock);
    static String   GetDefName();
    static String   GetExtension();

    String          GetCompleteGroupName( const OUString& GroupName );

    sal_Bool            NewGroupDoc(OUString &rGroupName, const OUString& rTitle);
    sal_Bool            RenameGroupDoc(const OUString& sOldGroup, OUString& sNewGroup, const OUString& rNewTitle);
    sal_Bool            DelGroupDoc(const String &);
    SwDocShellRef   EditGroupDoc(const String &rGrpName, const String& rShortName, sal_Bool bShow = sal_True );
    void            SaveGroupDoc(const String &rGrpName, const String& rLongName );
    void            UpdateGlosPath(sal_Bool bFull);
    void            ShowError();
    inline sal_uLong            IsGlosPathErr() { return m_bError; }
    std::vector<String> const& GetPathArray() const { return m_PathArr; }
};


#endif // _GLOSDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
