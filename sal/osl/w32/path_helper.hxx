/*************************************************************************
 *
 *  $RCSfile: path_helper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:20:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *    GNU Lesser General Public License Version 2.1
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

#pragma warning (disable : 4800)

#ifndef _PATH_HELPER_HXX_
#define _PATH_HELPER_HXX_

#ifndef _PATH_HELPER_H_
#include "path_helper.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace osl
{

/*******************************************************************
 osl_systemPathEnsureSeparator
 Adds a trailing path separator to the given system path if not
 already there and if the path is not the root path or a logical
 drive alone
 ******************************************************************/

inline void systemPathEnsureSeparator(/*inout*/ rtl::OUString& Path)
{
    osl_systemPathEnsureSeparator(&Path.pData);
}

/*******************************************************************
 osl_systemPathRemoveSeparator
 Removes the last separator from the given system path if any and
 if the path is not the root path '\'
 ******************************************************************/

inline void systemPathRemoveSeparator(/*inout*/ rtl::OUString& Path)
{
    osl_systemPathRemoveSeparator(&Path.pData);
}

/*******************************************************************
 osl_systemPathIsLogicalDrivePattern
 ******************************************************************/

inline bool systemPathIsLogicalDrivePattern(/*in*/ const rtl::OUString& path)
{
    return osl_systemPathIsLogicalDrivePattern(path.pData);
}

} // end namespace osl

#endif
