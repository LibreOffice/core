/*************************************************************************
 *
 *  $RCSfile: tempfile.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-30 13:49:34 $
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
#ifndef _UNOTOOLS_TEMPFILE_HXX
#define _UNOTOOLS_TEMPFILE_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

namespace utl
{

class LocalFileHelper
{
public:
                    /**
                    Converts a "physical" file name into a "UCB compatible" URL ( if possible ).
                    If no UCP is available for the local file system, sal_False and an empty URL is returned.
                    Returning sal_True and an empty URL means that the URL doesn't point to a local file.
                    */
    static sal_Bool   ConvertPhysicalNameToURL( const String& rName, String& rReturn );

                    /**
                    Converts a "UCB compatible" URL into a "physical" file name.
                    If no UCP is available for the local file system, sal_False and an empty file name is returned,
                    otherwise sal_True and a valid URL, because a file name can always be converted if a UCP for the local
                    file system is present ( watch: this doesn't mean that this file really exists! )
                    */
    static sal_Bool   ConvertURLToPhysicalName( const String& rName, String& rReturn );

};

struct TempFile_Impl;

/**
    The class TempFile gives access to temporary files in the local file system. Sometimes they are needed because a 3rd party
    code has a file name based interface, or some file access has to be done locally without transferring tons of bytes to or
    from a remote system.
    Creating a UCB content on a TempFile is only possible if a UCP for the local file system is present.
    TempFiles can always be accessed by SvFileStreams or Sot/SvStorages using the "physical" file name ( not the URL, because
    this may be a non-file URL, see below ), but if a UCB content can be created, it is also possible to take the URL and use
    the UCB helper classes for streams. For convenience use UcbStreamHelper.
    A Tempfile always has a "physical" file name ( a file name in the local computers host notation ) but it has a
    "UCB compatible" URL only if a UCP for the local file system exists. This URL may have its own URL scheme
    ( not neccessarily "file://" ! ). The TempFile class methods take this into account, but other simple conversions like
    the osl functions do not.
    So it is a potential error to convert between the filename and the URL of a TempFile object using functions or methods
    outside this class.
*/

class TempFile
{
    TempFile_Impl*  pImp;
    sal_Bool        bKillingFileEnabled;

protected:

public:
                    /**
                    Create a temporary file or directory, in the default tempfile folder or if possible in a given folder.
                    This given folder ( the "parent" parameter ( if not NULL ) ) must be a "UCB compatible" URL.
                    The temporary object is created in the local file system, even if there is no UCB that can access it.
                    If the given folder is part of the local file system, the TempFile is created in this folder.
                    */
                    TempFile( const String* pParent=NULL, sal_Bool bDirectory=sal_False );

                    /**
                    Same as above; additionally the name starts with some given characters followed by a counter ( example:
                    rLeadingChars="abc" means "abc0","abc1" and so on, depending on existing files in the folder ).
                    The extension string may be f.e. ".txt" or "", if no extension string is given, ".tmp" is used
                    */
                    TempFile( const String& rLeadingChars, const String* pExtension=NULL, const String* pParent=NULL,
                                sal_Bool bDirectory=sal_False );

                    /**
                    TempFile will be removed from disk in dtor if EnableKillingTempFile was called before.
                    Temporary directories will be removed recursively in that case.
                    */
                    ~TempFile();

                    /**
                    Returns sal_True if it has a valid file name.
                    */
    sal_Bool        IsValid() const;

                    /**
                    Returns the "UCB compatible" URL of the tempfile object.
                    If you want to have the "physical" file name, use the GetFileName() method of this object, because these
                    method uses the UCB for the conversion, but never use any external conversion functions for URLs into
                    "physical" names.
                    If no UCP is available for the local file system, an empty URL is returned. In this case you can't access
                    the file as a UCB content !
                    */
    String          GetURL() const;

                    /**
                    Returns the "physical" name of the tempfile in host notation ( should only be used for 3rd party code
                    with file name interfaces ).
                    If you want to have the URL, use the GetURL() method of this object, but never use any external
                    conversion functions for "physical" names into URLs.
                    */
    String          GetFileName() const;

                    /**
                    If enabled the file will be removed from disk when the dtor is called ( default is not enabled )
                    */
    void            EnableKillingFile( sal_Bool bEnable=sal_True )
                    { bKillingFileEnabled = bEnable; }

    sal_Bool        IsKillingFileEnabled() const
                    { return bKillingFileEnabled; }

                    /**
                    Only create a "physical" file name for a temporary file that would be valid at that moment.
                    Should only be used for 3rd party code with a file name interface that wants to create the file by itself.
                    If you want to convert file name into a URL, always use class LocalFileHelper, but never use any
                    conversion functions of osl.
                    */
    static String   CreateTempName( const String* pParent=NULL );

                    /**
                    The TempNameBaseDirectory is a subfolder in the folder that is passed as a "physical" file name in the
                    SetTempNameBaseDirectory method.
                    This subfolder will be used if a TempFile or TempName is created without a parent name or a parent name
                    that does not belong to the local file system.
                    The caller of the SetTempNameBase is responsible for deleting this folder and all temporary files in it.
                    The return value of both methods is the complete "physical" name of the tempname base folder.
                    It is not a URL because alle URLs must be "UCB compatible", so there may be no suitable URL at all.
                    */
    static String   SetTempNameBaseDirectory( const String &rBaseName );
    static String   GetTempNameBaseDirectory();
};

};

#endif
