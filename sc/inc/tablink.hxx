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

#ifndef INCLUDED_SC_INC_TABLINK_HXX
#define INCLUDED_SC_INC_TABLINK_HXX

#include "scdllapi.h"
#include "refreshtimer.hxx"
#include <sfx2/lnkbase.hxx>

#include <sfx2/objsh.hxx>
#include <memory>

class ScDocShell;
struct TableLink_Impl;

class ScTableLink : public ::sfx2::SvBaseLink, public ScRefreshTimer
{
private:
    std::unique_ptr<TableLink_Impl> pImpl;
    OUString aFileName;
    OUString aFilterName;
    OUString aOptions;
    bool bInCreate:1;
    bool bInEdit:1;
    bool bAddUndo:1;

public:
    ScTableLink( ScDocShell* pDocSh, const OUString& rFile,
                    const OUString& rFilter, const OUString& rOpt, sal_uLong nRefresh );
    ScTableLink( SfxObjectShell* pShell, const OUString& rFile,
                    const OUString& rFilter, const OUString& rOpt, sal_uLong nRefresh );
    virtual ~ScTableLink();
    virtual void Closed() override;
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue ) override;

    virtual void    Edit( vcl::Window*, const Link<SvBaseLink&,void>& rEndEditHdl ) override;

    bool    Refresh(const OUString& rNewFile, const OUString& rNewFilter,
                    const OUString* pNewOptions /* = NULL */, sal_uLong nNewRefresh );
    void    SetInCreate(bool bSet)      { bInCreate = bSet; }
    void    SetAddUndo(bool bSet)       { bAddUndo = bSet; }

    const OUString& GetFileName() const   { return aFileName; }
    const OUString& GetFilterName() const { return aFilterName; }
    const OUString& GetOptions() const    { return aOptions; }

    bool    IsUsed() const;

    DECL_LINK_TYPED( RefreshHdl, Timer*, void );
    DECL_LINK_TYPED( TableEndEditHdl, ::sfx2::SvBaseLink&, void );
};

class ScDocument;
class SfxMedium;
class SfxFilter;

class SC_DLLPUBLIC ScDocumentLoader
{
private:
    ScDocShell*         pDocShell;
    SfxObjectShellRef   aRef;
    SfxMedium*          pMedium;

public:
                        ScDocumentLoader( const OUString& rFileName,
                                          OUString& rFilterName, OUString& rOptions,
                                          sal_uInt32 nRekCnt = 0, bool bWithInteraction = false );
                        ~ScDocumentLoader();
    ScDocument*         GetDocument();
    ScDocShell*         GetDocShell()       { return pDocShell; }
    bool                IsError() const;
    OUString       GetTitle() const;

    void                ReleaseDocRef();    // without calling DoClose

    /** Create SfxMedium for stream read with SfxFilter and filter options set
        at the medium's SfxItemSet.
     */
    static SfxMedium*   CreateMedium( const OUString& rFileName, std::shared_ptr<const SfxFilter> const & pFilter, const OUString& rOptions );

    static OUString     GetOptions( SfxMedium& rMedium );

    /** Returns the filter name and options from a file name.
        @param bWithContent
            true = Tries to detect the filter by looking at the file contents.
            false = Detects filter by file name extension only (should be used in filter code only).
        @return sal_True if a filter could be found, sal_False otherwise. */

    static bool         GetFilterName( const OUString& rFileName,
                                       OUString& rFilter, OUString& rOptions,
                                       bool bWithContent, bool bWithInteraction );

    static void         RemoveAppPrefix( OUString& rFilterName );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
