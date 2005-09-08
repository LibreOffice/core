/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svxmsbas.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:02:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVXMSBAS_HXX
#define _SVXMSBAS_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

class SVX_DLLPUBLIC SvxImportMSVBasic
{
public:
    SvxImportMSVBasic( SfxObjectShell &rDocS, SotStorage &rRoot,
                        BOOL bImportCode = TRUE, BOOL bCopyStorage = TRUE )
        :   xRoot(&rRoot), rDocSh(rDocS),
            bImport(bImportCode), bCopy(bCopyStorage)
        {}
    // returns the status of import:
    // 0 - nothing has done
    // bit 0 = 1 -> any code is imported to the SO-Basic
    // bit 1 = 1 -> the VBA - storage is copy to the ObjectShell storage
    int Import( const String& rStorageName, const String &rSubStorageName,
                BOOL bAsComment=TRUE, BOOL bStripped=TRUE );

    // only for the export - copy or delete the saved VBA-macro-storage
    // form the ObjectShell
    // - returns a warning code if a modified basic exist, in all other
    //   cases return ERRCODE_NONE.
    ULONG SaveOrDelMSVBAStorage( BOOL bSaveInto, const String& rStorageName );

    // check if the MS-VBA-Storage exist in the RootStorage of the DocShell.
    // If it exist, then return the WarningId for loosing the information.
    static ULONG GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS );

    static String GetMSBasicStorageName();
private:
    SotStorageRef xRoot;
    SfxObjectShell &rDocSh;
    BOOL bImport;
    BOOL bCopy;

    SVX_DLLPRIVATE BOOL ImportCode_Impl( const String& rStorageName,
                          const String &rSubStorageName,
                          BOOL bAsComment, BOOL bStripped);
    SVX_DLLPRIVATE bool ImportForms_Impl(const String& rStorageName,
        const String &rSubStorageName);
    SVX_DLLPRIVATE BOOL CopyStorage_Impl( const String& rStorageName,
                           const String &rSubStorageName);
};

#endif
