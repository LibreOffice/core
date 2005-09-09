/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localfilehelper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:32:56 $
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
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#define _UNOTOOLS_LOCALFILEHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#include <rtl/ustring.hxx>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

namespace utl
{

class UNOTOOLS_DLLPUBLIC LocalFileHelper
{
public:
                    /**
                    Converts a "physical" file name into a "UCB compatible" URL ( if possible ).
                    If no UCP is available for the local file system, sal_False and an empty URL is returned.
                    Returning sal_True and an empty URL means that the URL doesn't point to a local file.
                    */
    static sal_Bool ConvertPhysicalNameToURL( const String& rName, String& rReturn );
    static sal_Bool ConvertSystemPathToURL( const String& rName, const String& rBaseURL, String& rReturn );

                    /**
                    Converts a "UCB compatible" URL into a "physical" file name.
                    If no UCP is available for the local file system, sal_False and an empty file name is returned,
                    otherwise sal_True and a valid URL, because a file name can always be converted if a UCP for the local
                    file system is present ( watch: this doesn't mean that this file really exists! )
                    */
    static sal_Bool ConvertURLToPhysicalName( const String& rName, String& rReturn );
    static sal_Bool ConvertURLToSystemPath( const String& rName, String& rReturn );

    static sal_Bool IsLocalFile( const String& rName );
    static sal_Bool IsFileContent( const String& rName );

    static          ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            GetFolderContents( const ::rtl::OUString& rFolder, sal_Bool bFolder );
};

};

#endif
