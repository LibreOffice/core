/*************************************************************************
 *
 *  $RCSfile: swunohelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:07:22 $
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
#ifndef _SWUNOHELPER_HXX
#define _SWUNOHELPER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace com { namespace sun { namespace star {
    namespace uno {
        class Any;
    }
}}};

class String;
class SvStrings;

namespace SWUnoHelper {

// calls over the compherl the getEnumAsInt32 function and handle the
// exceptions.
sal_Int32 GetEnumAsInt32( const com::sun::star::uno::Any& rVal );


// methods for UCB actions:
    // delete the file under this URL
BOOL UCB_DeleteFile( const String& rURL );

    // copy/move the file to a new location
BOOL UCB_CopyFile( const String& rURL, const String& rNewURL,
                    BOOL bCopyIsMove = FALSE );

    // is the URL on the current system case sentive?
BOOL UCB_IsCaseSensitiveFileName( const String& rURL );

    // is the URL readonly?
BOOL UCB_IsReadOnlyFileName( const String& rURL );

    // get a list of files from the folder of the URL
    // options: pExtension = 0 -> all, else this specific extension
    //          pDateTime != 0 -> returns also the modified date/time of
    //                       the files in a SvPtrarr -->
    //                       !! objects must be deleted from the caller!!
BOOL UCB_GetFileListOfFolder( const String& rURL, SvStrings& rList,
                                const String* pExtension = 0,
                                SvPtrarr* pDateTimeList = 0 );

    // is the URL a existing directory?
BOOL UCB_IsDirectory( const String& rURL );
}

#endif
