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


#ifndef _FILEDLG_HXX
#define _FILEDLG_HXX

#include <tools/string.hxx>
#include <tools/errcode.hxx>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif
#include "sddllapi.h"

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
    const std::auto_ptr< SdFileDialog_Imp > mpImpl;

    // forbidden and not implemented
    SdOpenSoundFileDialog (const SdOpenSoundFileDialog &);
    SdOpenSoundFileDialog & operator= (const SdOpenSoundFileDialog &);

public:
                            SdOpenSoundFileDialog();
                            ~SdOpenSoundFileDialog();

    ErrCode                  Execute();
    String                   GetPath() const;
    void                     SetPath( const String& rPath );

    String                   ReqDisplayDirectory() const;
};

#endif // _FILEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
