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


// this file contains code from filectrl.cxx which needs to be compiled with enabled exception hanling
#include <svtools/filectrl.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.h>
#include <vcl/stdtext.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;

void FileControl::ImplBrowseFile( )
{
    try
    {
        XubString aNewText;

        const ::rtl::OUString sServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ));

        Reference< XMultiServiceFactory > xMSF = comphelper::getProcessServiceFactory();
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
        OSL_FAIL( "FileControl::ImplBrowseFile: caught an exception while executing the file picker!" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
