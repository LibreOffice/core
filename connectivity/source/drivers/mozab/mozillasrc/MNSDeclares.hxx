/*************************************************************************
 *
 *  $RCSfile: MNSDeclares.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 18:31:49 $
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
 *  Contributor(s): Darren Kenny
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_MAB_NS_DECLARES_HXX_
#define _CONNECTIVITY_MAB_NS_DECLARES_HXX_ 1

#include <sal/types.h>

#define PROXIED_FUNCTION
#define ACCESSED_IN_PROXY
#define ACCESS_PROXIED_VARIABLES
#define PROXY_RELATED

const   sal_Int32 RowStates_Normal = 0;
const   sal_Int32 RowStates_Inserted = 1;
const   sal_Int32 RowStates_Updated = 2;
const   sal_Int32 RowStates_Deleted  = 4;
const   sal_Int32 RowStates_Error  = 32;

ACCESSED_IN_PROXY  extern sal_Bool isProfileLocked();

class nsIAbDirectory;
ACCESSED_IN_PROXY sal_Int32 getDirectoryType(const nsIAbDirectory*  directory);


#endif // _CONNECTIVITY_MAB_NS_DECLARES_HXX_ 1

