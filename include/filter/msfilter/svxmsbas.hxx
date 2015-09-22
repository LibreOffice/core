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

#ifndef INCLUDED_FILTER_MSFILTER_SVXMSBAS_HXX
#define INCLUDED_FILTER_MSFILTER_SVXMSBAS_HXX

#include <map>
#include <unordered_map>

#include <filter/msfilter/msfilterdllapi.h>
#include <rtl/ustring.hxx>
#include <sot/storage.hxx>
#include <tools/solar.h>
#include <tools/ref.hxx>

class SfxObjectShell;

/* Construct with the root storage of the MS document, with bImportCode
 * set the visual basic code will be imported into the stardocument when Import
 * is called, with bCopyStorage set, the visual basic storage tree will be
 * copied completely into staroffice, so that any future export to a msoffice
 * format will retain the vba code, allowing a lossless roundtrip from
 * msoffice to staroffice and back.
 *
 * Setting bAsComment to true in Import will import the visual basic as a
 * starbasic comment. Which is currently necessary, as vb is not valid sb.
 *
 * Setting bStripped will remove the "Attribute" lines from the vb, msoffice
 * does this itself when it shows the vb code in the vbeditor, so this is
 * probably what the user expects to see when viewing the code
 */



class MSFILTER_DLLPUBLIC SvxImportMSVBasic
{
public:
    SvxImportMSVBasic( SfxObjectShell &rDocS, SotStorage &rRoot )
        :   xRoot(&rRoot), rDocSh(rDocS)
        {}
    // only for the export - copy or delete the saved VBA-macro-storage
    // form the ObjectShell
    // - returns a warning code if a modified basic exist, in all other
    //   cases return ERRCODE_NONE.
    ErrCode SaveOrDelMSVBAStorage( bool bSaveInto, const OUString& rStorageName );

    // check if the MS-VBA-Storage exist in the RootStorage of the DocShell.
    // If it exist, then return the WarningId for losing the information.
    static ErrCode GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS );

    static OUString GetMSBasicStorageName();
private:
    tools::SvRef<SotStorage> xRoot;
    SfxObjectShell &rDocSh;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
