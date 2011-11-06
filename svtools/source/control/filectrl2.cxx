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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

// this file contains code from filectrl.cxx which needs to be compiled with enabled exception hanling
#include <svtools/filectrl.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <vcl/unohelp.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.h>
#include <vcl/stdtext.hxx>
#include <tools/debug.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;

void FileControl::ImplBrowseFile( )
{
    try
    {
        XubString aNewText;

        const ::rtl::OUString sServiceName = ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.FilePicker" );

        Reference< XMultiServiceFactory > xMSF = vcl::unohelper::GetMultiServiceFactory();
        Reference < dialogs::XFilePicker > xFilePicker( xMSF->createInstance( sServiceName ), UNO_QUERY );
        if ( xFilePicker.is() )
        {
            // transform the system notation text into a file URL
            ::rtl::OUString sSystemNotation = GetText(), sFileURL;
            oslFileError nError = osl_getFileURLFromSystemPath( sSystemNotation.pData, &sFileURL.pData );
            if ( nError == osl_File_E_INVAL )
                sFileURL = GetText();   // #97709# Maybe URL is already a file URL...

            //#90430# Check if URL is really a file URL
            ::rtl::OUString aTmp;
            if ( osl_getSystemPathFromFileURL( sFileURL.pData, &aTmp.pData ) == osl_File_E_None )
            {
                // initially set this directory
                xFilePicker->setDisplayDirectory( sFileURL );
            }

            if ( xFilePicker.is() && xFilePicker->execute() )
            {
                Sequence < rtl::OUString > aPathSeq = xFilePicker->getFiles();

                if ( aPathSeq.getLength() )
                {
                    aNewText = aPathSeq[0];
                    INetURLObject aObj( aNewText );
                    if ( aObj.GetProtocol() == INET_PROT_FILE )
                        aNewText = aObj.PathToFileName();
                    SetText( aNewText );
                    maEdit.GetModifyHdl().Call( &maEdit );
                }
            }
        }
        else
            ShowServiceNotAvailableError( this, sServiceName, sal_True );
    }
    catch( const Exception& )
    {
        DBG_ERROR( "FileControl::ImplBrowseFile: caught an exception while executing the file picker!" );
    }
}

