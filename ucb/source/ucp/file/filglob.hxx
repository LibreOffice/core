/*************************************************************************
 *
 *  $RCSfile: filglob.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: abi $ $Date: 2001-06-29 15:00:12 $
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
#ifndef _FILGLOB_HXX_
#define _FILGLOB_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif


namespace fileaccess {


    struct equalOUString
    {
        bool operator()( const rtl::OUString& rKey1, const rtl::OUString& rKey2 ) const
        {
            return !!( rKey1 == rKey2 );
        }
    };


    struct hashOUString
    {
        size_t operator()( const rtl::OUString& rName ) const
        {
            return rName.hashCode();
        }
    };


    /******************************************************************************/
    /*                                                                            */
    /*                         Helper functions                                   */
    /*                                                                            */
    /******************************************************************************/


    // Returns true if dstUnqPath is a child from srcUnqPath or both are equal

    extern sal_Bool isChild( const rtl::OUString& srcUnqPath,
                                      const rtl::OUString& dstUnqPath );


    // Changes the prefix in name
    extern rtl::OUString newName( const rtl::OUString& aNewPrefix,
                                           const rtl::OUString& aOldPrefix,
                                           const rtl::OUString& old_Name );

    // returns the last part of the given url as title
    extern rtl::OUString getTitle( const rtl::OUString& aPath );

    // returns the url without last part as parentname
    extern rtl::OUString getParentName( const rtl::OUString& aFileName );


    extern osl::FileBase::RC osl_File_copy( const rtl::OUString& strPath,
                                            const rtl::OUString& strDestPath,
                                            sal_Bool test = false );

    extern osl::FileBase::RC osl_File_move( const rtl::OUString& strPath,
                                            const rtl::OUString& strDestPath,
                                            sal_Bool test = false );

    extern oslFileError getResolvedURL( rtl_uString* ustrPath,
                                        rtl_uString** pustrResolvedURL);


    // Removes ellipses like .. and . from a file path
    // Needs rework; This seems to be the most time consuming function in
    // the whole file content provider

    extern sal_Bool SAL_CALL makeAbsolutePath( const rtl::OUString& aRelPath,
                                               rtl::OUString& aAbsPath );


    // This function implements the global exception handler of the file_ucp;
    // It never returns;

    extern void throw_handler( sal_Int32 errorCode,
                               sal_Int32 minorCode );

} // end namespace fileaccess

#endif
