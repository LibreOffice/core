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

#ifndef SC_TABLINK_HXX
#define SC_TABLINK_HXX

#include "scdllapi.h"
#include "refreshtimer.hxx"
#include <sfx2/lnkbase.hxx>

#include <sfx2/objsh.hxx>

class ScDocShell;
struct TableLink_Impl;

class ScTableLink : public ::sfx2::SvBaseLink, public ScRefreshTimer
{
private:
    TableLink_Impl* pImpl;
    rtl::OUString aFileName;
    rtl::OUString aFilterName;
    rtl::OUString aOptions;
    bool bInCreate:1;
    bool bInEdit:1;
    bool bAddUndo:1;
    bool bDoPaint:1;

public:
    TYPEINFO();
    ScTableLink( ScDocShell* pDocSh, const String& rFile,
                    const String& rFilter, const String& rOpt, sal_uLong nRefresh );
    ScTableLink( SfxObjectShell* pShell, const String& rFile,
                    const String& rFilter, const String& rOpt, sal_uLong nRefresh );
    virtual ~ScTableLink();
    virtual void Closed();
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const String& rMimeType, const ::com::sun::star::uno::Any & rValue );

    virtual void    Edit( Window*, const Link& rEndEditHdl );

    sal_Bool    Refresh(const String& rNewFile, const String& rNewFilter,
                    const String* pNewOptions /* = NULL */, sal_uLong nNewRefresh );
    void    SetInCreate(bool bSet)      { bInCreate = bSet; }
    void    SetAddUndo(bool bSet)       { bAddUndo = bSet; }
    void    SetPaint(bool bSet)         { bDoPaint = bSet; }

    const rtl::OUString& GetFileName() const   { return aFileName; }
    const rtl::OUString& GetFilterName() const { return aFilterName; }
    const rtl::OUString& GetOptions() const    { return aOptions; }

    sal_Bool    IsUsed() const;

    DECL_LINK( RefreshHdl, ScTableLink* );
    DECL_LINK( TableEndEditHdl, ::sfx2::SvBaseLink* );
};

class ScDocument;
class SfxMedium;

class SC_DLLPUBLIC ScDocumentLoader
{
private:
    ScDocShell*         pDocShell;
    SfxObjectShellRef   aRef;
    SfxMedium*          pMedium;

    static bool         GetFilterName( const String& rFileName,
                                       String& rFilter, String& rOptions,
                                       bool bWithContent, bool bWithInteraction );

public:
                        ScDocumentLoader( const String& rFileName,
                                          String& rFilterName, String& rOptions,
                                          sal_uInt32 nRekCnt = 0, bool bWithInteraction = false );
                        ~ScDocumentLoader();
    ScDocument*         GetDocument();
    ScDocShell*         GetDocShell()       { return pDocShell; }
    bool                IsError() const;
    rtl::OUString       GetTitle() const;

    void                ReleaseDocRef();    // without calling DoClose

    static rtl::OUString GetOptions( SfxMedium& rMedium );

    /** Returns the filter name and options from a file name.
        @param bWithContent
            true = Tries to detect the filter by looking at the file contents.
            false = Detects filter by file name extension only (should be used in filter code only).
        @return sal_True if a filter could be found, sal_False otherwise. */

    static bool         GetFilterName( const ::rtl::OUString& rFileName,
                                       ::rtl::OUString& rFilter, ::rtl::OUString& rOptions,
                                       bool bWithContent, bool bWithInteraction );

    static void         RemoveAppPrefix( rtl::OUString& rFilterName );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
