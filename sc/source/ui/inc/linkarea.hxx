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

#ifndef SC_LINKAREA_HXX
#define SC_LINKAREA_HXX

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/inettbc.hxx>

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class ScDocShell;

//------------------------------------------------------------------------

class ScLinkedAreaDlg : public ModalDialog
{
private:
    FixedLine           aFlLocation;
    SvtURLBox           aCbUrl;
    PushButton          aBtnBrowse;
    FixedInfo           aTxtHint;
    FixedText           aFtRanges;
    MultiListBox        aLbRanges;
    CheckBox            aBtnReload;
    NumericField        aNfDelay;
    FixedText           aFtSeconds;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    ScDocShell*             pSourceShell;
    sfx2::DocumentInserter* pDocInserter;

    SfxObjectShellRef   aSourceRef;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject > xSourceObject;

    DECL_LINK(FileHdl, void *);
    DECL_LINK(BrowseHdl, void *);
    DECL_LINK(RangeHdl, void *);
    DECL_LINK(ReloadHdl, void *);
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

    void                UpdateSourceRanges();
    void                UpdateEnable();
    void                LoadDocument( const OUString& rFile, const String& rFilter,
                                        const String& rOptions );

public:
            ScLinkedAreaDlg( Window* pParent );
            ~ScLinkedAreaDlg();

    void            InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        sal_uLong nRefresh );

    virtual short   Execute();          // overwritten to set dialog parent

    OUString       GetURL();
    OUString       GetFilter();        // may be empty
    OUString       GetOptions();       // filter options
    OUString       GetSource();        // separated by ";"
    sal_uLong           GetRefresh();       // 0 if disabled
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
