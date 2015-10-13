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
#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_FPDIALOGBASE_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_FPDIALOGBASE_HXX

#include <vcl/dialog.hxx>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include "svl/inettype.hxx"
#include "asyncfilepicker.hxx"
#include "OfficeControlAccess.hxx"
#include "fpsmartcontent.hxx"

#include <set>

class SvTabListBox;
class SvtFileView;
class SvtFileDialogFilter_Impl;


#define SFXWB_INSERT            ( 0x04000000L | WB_OPEN )
#define SFXWB_PASSWORD          WB_PASSWORD
#define SFXWB_READONLY          WB_READONLY
#define SFXWB_PATHDIALOG        WB_PATH
#define SFXWB_CLASSPATH         ( 0x08000000L | SFXWB_PATHDIALOG )
#define SFXWB_MULTISELECTION    0x20000000L     // activate Multiselection
#define SFXWB_NOREMOTE          0x40000000L

#define SFX_EXTRA_AUTOEXTENSION     0x00000001L
#define SFX_EXTRA_FILTEROPTIONS     0x00000002L
#define SFX_EXTRA_SHOWVERSIONS      0x00000004L
#define SFX_EXTRA_INSERTASLINK      0x00000008L
#define SFX_EXTRA_SHOWPREVIEW       0x00000010L
#define SFX_EXTRA_TEMPLATES         0x00000020L
#define SFX_EXTRA_PLAYBUTTON        0x00000040L
#define SFX_EXTRA_SELECTION         0x00000080L
#define SFX_EXTRA_IMAGE_TEMPLATE    0x00000100L

#define FILEDIALOG_FILTER_ALL   "*.*"

// SvtFileDialog_Base

class SvtFileDialog_Base : public ModalDialog, public ::svt::IFilePickerController
{
public:
    SvtFileDialog_Base( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription )
    : ModalDialog( pParent, rID, rUIXMLDescription )
    {
    }

    virtual SvtFileView* GetView() = 0;

    virtual void SetHasFilename( bool bHasFilename ) = 0;
    virtual void SetBlackList( const css::uno::Sequence< OUString >& rBlackList ) = 0;
    virtual const css::uno::Sequence< OUString >& GetBlackList() const = 0;
    virtual void SetStandardDir( const OUString& rStdDir ) = 0;
    virtual const OUString& GetStandardDir() const = 0;
    virtual void SetPath( const OUString& rNewURL ) = 0;
    virtual const OUString& GetPath() = 0;
    virtual std::vector<OUString> GetPathList() const = 0;
    virtual bool ContentIsFolder( const OUString& rURL ) = 0;

    virtual OUString getCurrentFileText() const = 0;
    virtual void setCurrentFileText( const OUString& rText, bool bSelectAll = false ) = 0;

    virtual void AddFilter( const OUString& rFilter, const OUString& rType ) = 0;
    virtual void AddFilterGroup( const OUString& _rFilter,
                                const css::uno::Sequence< css::beans::StringPair >& rFilters ) = 0;
    virtual OUString GetCurFilter() const = 0;
    virtual void SetCurFilter( const OUString& rFilter ) = 0;
    virtual void FilterSelect() = 0;

    virtual void SetFileCallback( ::svt::IFilePickerListener *pNotifier ) = 0;
    virtual void onAsyncOperationStarted() = 0;
    virtual void onAsyncOperationFinished() = 0;
    virtual void UpdateControls( const OUString& rURL ) = 0;

    virtual void EnableAutocompletion( bool _bEnable = true ) = 0;

    virtual sal_Int32 getTargetColorDepth() = 0;
    virtual sal_Int32 getAvailableWidth() = 0;
    virtual sal_Int32 getAvailableHeight() = 0;

    virtual void setImage( sal_Int16 aImageFormat, const css::uno::Any& rImage ) = 0;

    virtual bool getShowState() = 0;
};

#define FILE_SELECTION_CHANGED  1
#define DIRECTORY_CHANGED       2
#define CTRL_STATE_CHANGED      4
#define DIALOG_SIZE_CHANGED     5


#endif // INCLUDED_FPICKER_SOURCE_OFFICE_IODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
