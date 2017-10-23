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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FILEDLG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FILEDLG_HXX

#include <vcl/errcode.hxx>

#include <memory>

#include <sddllapi.h>

class SdFileDialog_Imp;

/******************************************************************************/

/**
   The class SdOpenSoundFileDialog wraps the FileDialogHelper, displaying the
   FILEOPEN_PLAY dialog template and performing the 'preview' functionality
   (playing the selected sound file). The interface is a downstripped version
   of the aforementioned class, with similar semantics.
 */
class SD_DLLPUBLIC SdOpenSoundFileDialog
{
    const std::unique_ptr< SdFileDialog_Imp > mpImpl;

    SdOpenSoundFileDialog (const SdOpenSoundFileDialog &) = delete;
    SdOpenSoundFileDialog & operator= (const SdOpenSoundFileDialog &) = delete;

public:
    SdOpenSoundFileDialog(const vcl::Window* pParent);
    ~SdOpenSoundFileDialog();

    ErrCode Execute();
    OUString GetPath() const;
    void SetPath( const OUString& rPath );
    // WIP, please don't remove, dear Clang plugins
    bool IsInsertAsLinkSelected();
};

#endif // INCLUDED_SD_SOURCE_UI_INC_FILEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
