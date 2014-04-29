/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVXMSBAS_HXX
#define _SVXMSBAS_HXX

#include <tools/solar.h>
#include "filter/msfilter/msfilterdllapi.h"

#include <sot/storage.hxx>

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
    // If it exists, then return the WarningId for losing the information.
    static sal_uLong GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS );

    static String GetMSBasicStorageName();
private:
    SotStorageRef xRoot;
    SfxObjectShell &rDocSh;
    sal_Bool bImport;
    sal_Bool bCopy;

    MSFILTER_DLLPRIVATE sal_Bool ImportCode_Impl( const String& rStorageName,
                          const String &rSubStorageName,
                          const std::vector< String >& codeNames,
                          sal_Bool bAsComment, sal_Bool bStripped);
    MSFILTER_DLLPRIVATE bool ImportForms_Impl(const String& rStorageName,
        const String &rSubStorageName);
    MSFILTER_DLLPRIVATE sal_Bool CopyStorage_Impl( const String& rStorageName,
                           const String &rSubStorageName);
};

#endif
