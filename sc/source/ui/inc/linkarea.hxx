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

    DECL_LINK( FileHdl, ComboBox* );
    DECL_LINK( BrowseHdl, PushButton* );
    DECL_LINK( RangeHdl, MultiListBox* );
    DECL_LINK( ReloadHdl, CheckBox* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

    void                UpdateSourceRanges();
    void                UpdateEnable();
    void                LoadDocument( const String& rFile, const String& rFilter,
                                        const String& rOptions );

public:
            ScLinkedAreaDlg( Window* pParent );
            ~ScLinkedAreaDlg();

    void            InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        sal_uLong nRefresh );

    virtual short   Execute();          // overwritten to set dialog parent

    String          GetURL();
    String          GetFilter();        // may be empty
    String          GetOptions();       // filter options
    String          GetSource();        // separated by ";"
    sal_uLong           GetRefresh();       // 0 if disabled
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
