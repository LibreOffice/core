/*************************************************************************
 *
 *  $RCSfile: svxmsbas.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _BASMGR_HXX
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBMOD_HXX
#include <basic/sbmod.hxx>
#endif
#ifndef _SVXERR_HXX
#include <svxerr.hxx>
#endif

#ifndef _SVXMSBAS_HXX
#include <svxmsbas.hxx>
#endif
#ifndef _MSVBASIC_HXX
#include <msvbasic.hxx>
#endif

int SvxImportMSVBasic::Import( const String& rStorageName,
                                const String &rSubStorageName,
                                BOOL bAsComment, BOOL bStripped )
{
    int nRet = 0;
    if( bImport && ImportCode_Impl( rStorageName, rSubStorageName,
                                    bAsComment, bStripped ))
        nRet |= 1;

    if( bCopy && CopyStorage_Impl( rStorageName, rSubStorageName ))
        nRet |= 2;

    return nRet;
}

BOOL SvxImportMSVBasic::CopyStorage_Impl( const String& rStorageName,
                                         const String& rSubStorageName)
{
    BOOL bValidStg = FALSE;
    {
        SvStorageRef xVBAStg( xRoot->OpenStorage( rStorageName,
                                    STREAM_READWRITE | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYALL ));
        if( xVBAStg.Is() && !xVBAStg->GetError() )
        {
            SvStorageRef xVBASubStg( xVBAStg->OpenStorage( rSubStorageName,
                                     STREAM_READWRITE | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYALL ));
            if( xVBASubStg.Is() && !xVBASubStg->GetError() )
            {
                // then we will copy these storages into the SfxObjectShell-storage
                bValidStg = TRUE;
            }
        }
    }

    if( bValidStg )
    {
        SvStorageRef xDstRoot( rDocSh.GetStorage() );
        String aDstStgName( GetMSBasicStorageName() );
        bValidStg = xRoot->CopyTo( rStorageName, xDstRoot, aDstStgName );
    }

    return bValidStg;
}

BOOL SvxImportMSVBasic::ImportCode_Impl( const String& rStorageName,
                                        const String &rSubStorageName,
                                        BOOL bAsComment, BOOL bStripped )
{
    BOOL bRet = FALSE;
    VBA_Impl aVBA( *xRoot, bAsComment );
    if( aVBA.Open(rStorageName,rSubStorageName) )
    {
        SFX_APP()->EnterBasicCall();
//this method is removed in the Unicode-Version
//      rDocSh.GetSbxObject();

        BasicManager *pBasicMan = rDocSh.GetBasicManager();
        DBG_ASSERT( pBasicMan, "Wo ist der BasicManager?" );

        StarBASIC *pBasic;
        if( pBasicMan && (0 != (pBasic = pBasicMan->GetStdLib()) ) )
        {
            for( UINT16 i=0; i<aVBA.GetNoStreams();i++)
            {
                StringArray aDecompressed = aVBA.Decompress(i);
                ByteString sByteBasic(aVBA.GetStreamName(i),
                    RTL_TEXTENCODING_ASCII_US,
                        (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_UNDERLINE|
                        RTL_UNICODETOTEXT_FLAGS_INVALID_UNDERLINE |
                        RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 |
                        RTL_UNICODETOTEXT_FLAGS_NOCOMPOSITE)
                );

                //const String &sBasicModule = aVBA.GetStreamName( i);
                const String sBasicModule(sByteBasic,
                    RTL_TEXTENCODING_ASCII_US);
                for(INT32 j=0;j<aDecompressed.GetSize();j++)
                    {
                    String sModule(sBasicModule);
                    if (j>0)
                    {
                        sModule.AppendAscii("_Part");
                        sModule += String::CreateFromInt32(j+1);
                    }

                    if( bStripped )
                    {
                        xub_StrLen nBegin, nEnd;
                        static const sal_Char sAttribute[] = "Attribute";

                        while( STRING_NOTFOUND != (nBegin =
                                aDecompressed.Get(j)->SearchAscii(sAttribute)))
                        {
                            nEnd = aDecompressed.Get(j)->Search('\n',nBegin);
                            nBegin = aDecompressed.Get(j)->SearchBackward('\n',nBegin);
                            if( STRING_NOTFOUND == nBegin )
                                nBegin=0;
                            aDecompressed.Get(j)->Erase(nBegin,nEnd-nBegin+1);
                        }
                    }

                    if( aDecompressed.Get(j)->Len() )
                    {
                        if (bAsComment)
                        {
                            String sTemp( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "Sub " )));
                            String sMunge(sModule);
                            //Streams can have spaces in them, but modulenames
                            //cannot !
                            sMunge.SearchAndReplaceAll(' ','_');

                            sTemp += sMunge;
                            sTemp.AppendAscii("\n");
                            aDecompressed.Get(j)->Insert(sTemp,0);
                            aDecompressed.Get(j)->InsertAscii("\nEnd Sub");
                        }

                        SbModule *pModule = pBasic->MakeModule( sModule,
                            *aDecompressed.Get(j));
                        if( pModule )
                        {
                            pModule->Compile();
                            bRet = TRUE;
                        }
                    }
                }
            }
            pBasic->SetModified( FALSE );
        }
        SFX_APP()->LeaveBasicCall();
    }
    return bRet;
}

ULONG SvxImportMSVBasic::SaveOrDelMSVBAStorage( BOOL bSaveInto,
                                                const String& rStorageName )
{
    ULONG nRet = ERRCODE_NONE;
    SvStorageRef xSrcRoot( rDocSh.GetStorage() );
    String aDstStgName( GetMSBasicStorageName() );
    SvStorageRef xVBAStg( xSrcRoot->OpenStorage( aDstStgName,
                                STREAM_READWRITE | STREAM_NOCREATE |
                                STREAM_SHARE_DENYALL ));
    if( xVBAStg.Is() && !xVBAStg->GetError() )
    {
        xVBAStg = 0;
        if( bSaveInto )
        {
            BasicManager *pBasicMan = rDocSh.GetBasicManager();
            if( pBasicMan && pBasicMan->IsBasicModified() )
            {
                nRet = ERRCODE_SVX_MODIFIED_VBASIC_STORAGE;

// don't reset the modified flag
//              StarBASIC *pBasic = pBasicMan->GetStdLib();
//              if( pBasic )
//                  pBasic->SetModified( FALSE );
            }
// JP 02.05.00: Bug 75492 - save always the BasicStorage
//          else
                xSrcRoot->CopyTo( aDstStgName, xRoot, rStorageName);
        }
// we never stored this storage in our own format, so it may not be deleted
//      else
//          rDocSh.SvPersist::Remove( aDstStgName );
    }
    return nRet;
}

    // check if the MS-VBA-Storage exist in the RootStorage of the DocShell.
    // If it exist, then return the WarningId for loosing the information.
ULONG SvxImportMSVBasic::GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocSh)
{
    SvStorageRef xSrcRoot( rDocSh.GetStorage() );
    SvStorageRef xVBAStg( xSrcRoot->OpenStorage( GetMSBasicStorageName(),
                    STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL ));
    return ( xVBAStg.Is() && !xVBAStg->GetError() )
                    ? ERRCODE_SVX_VBASIC_STORAGE_EXIST
                    : ERRCODE_NONE;
}

String SvxImportMSVBasic::GetMSBasicStorageName()
{
char __READONLY_DATA sSO_VBAStgName[] = "_MS_VBA_Macros";
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(sSO_VBAStgName));
}

