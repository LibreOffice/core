/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSDeclares.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:27:40 $
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

#ifndef _CONNECTIVITY_MAB_NS_DECLARES_HXX_
#define _CONNECTIVITY_MAB_NS_DECLARES_HXX_

#include <sal/types.h>


const   sal_Int32 RowStates_Normal = 0;
const   sal_Int32 RowStates_Inserted = 1;
const   sal_Int32 RowStates_Updated = 2;
const   sal_Int32 RowStates_Deleted  = 4;
const   sal_Int32 RowStates_Error  = 32;

namespace connectivity{
    namespace mozab{
        class OConnection;
    }
}
sal_Bool isProfileLocked(connectivity::mozab::OConnection* _pCon);

class nsIAbDirectory;
sal_Int32 getDirectoryType(const nsIAbDirectory*  directory);


#endif // _CONNECTIVITY_MAB_NS_DECLARES_HXX_ 1

