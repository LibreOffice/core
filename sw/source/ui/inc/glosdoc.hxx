/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
#ifndef _GLOSDOC_HXX
#define _GLOSDOC_HXX

#include <tools/string.hxx>
#include <svl/svarray.hxx>
#include <com/sun/star/text/XAutoTextGroup.hpp>

class SwTextBlocks;
class SvStrings;
class SwDocShell;

#ifndef SW_DECL_SWDOCSHELL_DEFINED
#define SW_DECL_SWDOCSHELL_DEFINED
#include <tools/ref.hxx>
SV_DECL_REF( SwDocShell )
#endif
#include <cppuhelper/weakref.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif
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
    String                  m_sOldErrPath;
    String                  m_sErrPath;
    SvStrings               *m_pPathArr;
    SvStrings               *m_pGlosArr;
    sal_Bool                    m_bError;

    SW_DLLPRIVATE SwTextBlocks* GetGlosDoc(const String &rName, sal_Bool bCreate = sal_True) const;
    SW_DLLPRIVATE SvStrings     *GetNameList();

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
                                const ::rtl::OUString& _rGroupName,
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
                                const ::rtl::OUString& _rGroupName,
                                const ::rtl::OUString& _rEntryName,
                                bool _bCreate = false
                            );

    sal_uInt16                  GetGroupCnt();
    String                  GetGroupName(sal_uInt16 );
    String                  GetGroupTitle( const String& rGroupName );

    sal_Bool            FindGroupName(String & rGroup);

    SwTextBlocks*   GetGroupDoc(const String &rName,
                                sal_Bool bCreate = sal_False) const;
    SwTextBlocks*   GetDefGroupDoc() const {return GetGroupDoc(GetDefName());}
    void            PutGroupDoc(SwTextBlocks *pBlock);
    static String   GetDefName();
    static String   GetExtension();

    String          GetCompleteGroupName( const ::rtl::OUString& GroupName );

    sal_Bool            NewGroupDoc(String &rGroupName, const String& rTitle);
    sal_Bool            RenameGroupDoc(const String& sOldGroup, String& sNewGroup, const String& rNewTitle);
    sal_Bool            DelGroupDoc(const String &);
    SwDocShellRef   EditGroupDoc(const String &rGrpName, const String& rShortName, sal_Bool bShow = sal_True );
    void            SaveGroupDoc(const String &rGrpName, const String& rLongName );
    void            UpdateGlosPath(sal_Bool bFull);
    void            ShowError();
    inline sal_uLong    IsGlosPathErr() { return m_bError; }
    const SvStrings*    GetPathArray() const {return m_pPathArr;}
};


#endif // _GLOSDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
