/*************************************************************************
 *
 *  $RCSfile: NeonUri.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:55:20 $
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
#ifndef _NEONURI_HXX_
#define _NEONURI_HXX_

#include <uri.h>
#include <rtl/ustring.hxx>

namespace webdav_ucp
{

#define DEFAULT_HTTP_PORT       80

// -------------------------------------------------------------------
// NeonUri
// A URI implementation for use with the neon/expat library
// -------------------------------------------------------------------
class NeonUri
{
    private:
        ::rtl::OUString mURI;
        ::rtl::OUString mScheme;
        ::rtl::OUString mHostName;
        sal_Int32       mPort;
        ::rtl::OUString mPath;
        static uri      sUriDefaults;

        void calculateURI ();

    public:
        NeonUri( const ::rtl::OUString & inUri );
        ~NeonUri( );

        const ::rtl::OUString & GetHostName( void ) const
                                            { return mHostName; };
        const ::rtl::OUString & GetURI( void ) const
                                            { return mURI; };
        const ::rtl::OUString & GetScheme( void ) const
                                            { return mScheme; };
        const ::rtl::OUString & GetHost( void ) const
                                            { return mHostName; };
        sal_Int32       GetPort( void )     const
                                            { return mPort; };
        const ::rtl::OUString &     GetPath( void ) const
                                            { return mPath; };

        ::rtl::OUString GetPathBaseName ( void ) const;
        ::rtl::OUString GetPathDirName ( void ) const;


        void SetScheme (const ::rtl::OUString& scheme)
            { mScheme = scheme; calculateURI (); };

        void AppendPath (const ::rtl::OUString& path)
            { mPath += path; calculateURI (); };
};

}; // namespace webdav_ucp
#endif // _NEONURI_HXX_
