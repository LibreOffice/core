/*************************************************************************
 *
 *  $RCSfile: localfilehelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-02 10:22:45 $
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
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#define _UNOTOOLS_LOCALFILEHELPER_HXX

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

};

#endif
