/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tempfile.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:19:07 $
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
#ifndef _TOOLS_TEMPFILE_HXX
#define _TOOLS_TEMPFILE_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

struct TempFile_Impl;
class TOOLS_DLLPUBLIC TempFile
{
    TempFile_Impl*  pImp;
    sal_Bool        bKillingFileEnabled;

public:
                    // Create a temporary file or directory in a given folder or the default tempfile folder
                    TempFile( const String* pParent=NULL, sal_Bool bDirectory=sal_False );

                    // Create a temporary file or directory in a given folder or the default tempfile folder; its name starts
                    // with some given characters followed by a counter ( example: rLeadingChars="abc" means "abc0","abc1"
                    // and so on, depending on existing files in that folder ).
                    // The extension string may be f.e. ".txt" or "", if no extension string is given, ".tmp" is used
                    TempFile( const String& rLeadingChars, const String* pExtension=NULL, const String* pParent=NULL, sal_Bool bDirectory=sal_False );

                    // TempFile will be removed from disk in dtor if EnableKillingTempFile was called before.
                    // TempDirs will be removed recursively in that case.
                    ~TempFile();

    sal_Bool        IsValid() const;

                    // Returns the real name of the tempfile in file URL scheme.
    String          GetName() const;

                    // If enabled the file will be removed from disk when the dtor is called ( default is not enabled )
    void            EnableKillingFile( sal_Bool bEnable=sal_True )
                    { bKillingFileEnabled = bEnable; }

    sal_Bool        IsKillingFileEnabled() const
                    { return bKillingFileEnabled; }

                    // Only create a name for a temporary file that would be valid at that moment.
    static String   CreateTempName( const String* pParent=NULL );

                    // The TempNameBase is a folder in the default ( system ) tempfile folder.
                    // This subfolder will be used if a TempFile or TempName is created without a parent name.
                    // The caller of the SetTempNameBase is responsible for deleting this folder and all temporary files in it.
                    // The argument must be a simple name, not a complete URL.
                    // The return value of both methods is the complete URL of the tempname base folder.
    static String   SetTempNameBaseDirectory( const String &rBaseName );
    static String   GetTempNameBaseDirectory();
};

#endif
