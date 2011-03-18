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

#ifndef _SVXMSBAS_HXX
#define _SVXMSBAS_HXX

#include <tools/solar.h>
#include "filter/msfilter/msfilterdllapi.h"

#include <sot/storage.hxx>
#include <map>
#include <boost/unordered_map.hpp>

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

typedef boost::unordered_map< sal_Int32, String >  ObjIdToName;

typedef std::map< String, ObjIdToName >  ControlAttributeInfo;

class VBA_Impl;

class MSFILTER_DLLPUBLIC SvxImportMSVBasic
{
public:
    SvxImportMSVBasic( SfxObjectShell &rDocS, SotStorage &rRoot,
                        sal_Bool bImportCode = sal_True, sal_Bool bCopyStorage = sal_True )
        :   xRoot(&rRoot), rDocSh(rDocS),
            bImport(bImportCode), bCopy(bCopyStorage)
        {}
    // returns the status of import:
    // 0 - nothing has done
    // bit 0 = 1 -> any code is imported to the SO-Basic
    // bit 1 = 1 -> the VBA - storage is copy to the ObjectShell storage
    int Import( const String& rStorageName, const String &rSubStorageName,
                sal_Bool bAsComment=sal_True, sal_Bool bStripped=sal_True );
    int Import( const String& rStorageName, const String &rSubStorageName,
                const std::vector< String >& codeNames,
                sal_Bool bAsComment=sal_True, sal_Bool bStripped=sal_True );

    // only for the export - copy or delete the saved VBA-macro-storage
    // form the ObjectShell
    // - returns a warning code if a modified basic exist, in all other
    //   cases return ERRCODE_NONE.
    sal_uLong SaveOrDelMSVBAStorage( sal_Bool bSaveInto, const String& rStorageName );

    // check if the MS-VBA-Storage exist in the RootStorage of the DocShell.
    // If it exist, then return the WarningId for loosing the information.
    static sal_uLong GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS );
    const ControlAttributeInfo& ControlNameForObjectId(){ return m_ModuleNameToObjIdHash; }

    static String GetMSBasicStorageName();
        rtl::OUString GetVBAProjectName() { return msProjectName; }
private:
    SotStorageRef xRoot;
    SfxObjectShell &rDocSh;
    sal_Bool bImport;
    sal_Bool bCopy;
    ControlAttributeInfo m_ModuleNameToObjIdHash;
    MSFILTER_DLLPRIVATE void extractAttribute( const String& rAttribute, const String& rModName );

    MSFILTER_DLLPRIVATE sal_Bool ImportCode_Impl( const String& rStorageName,
                          const String &rSubStorageName,
                          const std::vector< String >& codeNames,
                          sal_Bool bAsComment, sal_Bool bStripped);
    MSFILTER_DLLPRIVATE bool ImportForms_Impl(const String& rStorageName,
                                              const String &rSubStorageName, sal_Bool bVBAMode);
    MSFILTER_DLLPRIVATE sal_Bool CopyStorage_Impl( const String& rStorageName,
                           const String &rSubStorageName);
        rtl::OUString msProjectName;
    MSFILTER_DLLPRIVATE sal_Bool ImportCode_Impl( VBA_Impl&, const std::vector< String >&, sal_Bool, sal_Bool );
    MSFILTER_DLLPRIVATE bool ImportForms_Impl( VBA_Impl&, const String&, const String&, sal_Bool);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
