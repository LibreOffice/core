/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swunohelper.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:12:51 $
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
#ifndef _SWUNOHELPER_HXX
#define _SWUNOHELPER_HXX

#include <tools/solar.h>
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

namespace com { namespace sun { namespace star {
    namespace uno {
        class Any;
    }
}}}

class String;
class SvStrings;
class SvPtrarr;

namespace SWUnoHelper {

// calls over the compherl the getEnumAsInt32 function and handle the
// exceptions.
sal_Int32 GetEnumAsInt32( const com::sun::star::uno::Any& rVal );


// methods for UCB actions:
    // delete the file under this URL
SW_DLLPUBLIC BOOL UCB_DeleteFile( const String& rURL );

    // copy/move the file to a new location
BOOL UCB_CopyFile( const String& rURL, const String& rNewURL,
                    BOOL bCopyIsMove = FALSE );

    // is the URL on the current system case sentive?
SW_DLLPUBLIC BOOL UCB_IsCaseSensitiveFileName( const String& rURL );

    // is the URL readonly?
SW_DLLPUBLIC BOOL UCB_IsReadOnlyFileName( const String& rURL );

    // get a list of files from the folder of the URL
    // options: pExtension = 0 -> all, else this specific extension
    //          pDateTime != 0 -> returns also the modified date/time of
    //                       the files in a SvPtrarr -->
    //                       !! objects must be deleted from the caller!!
BOOL UCB_GetFileListOfFolder( const String& rURL, SvStrings& rList,
                                const String* pExtension = 0,
                                SvPtrarr* pDateTimeList = 0 );

    // is the URL an existing file?
BOOL UCB_IsFile( const String& rURL );

    // is the URL a existing directory?
BOOL UCB_IsDirectory( const String& rURL );
}

#endif
