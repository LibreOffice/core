/*************************************************************************
 *
 *  $RCSfile: MNSProfileManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 12:29:09 $
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

#ifndef __MNSProfileManager_h___
#define __MNSProfileManager_h___

#include "nsCOMPtr.h"
#include "nsISupports.h"
#include "nsString.h"
#include "nsIRegistry.h"
#include "nsXPIDLString.h"
#include "nsVoidArray.h"
#include "nsIFile.h"
#include "nsILocalFile.h"

#include <sal/types.h>
#include <osl/diagnose.h>
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _COM_SUN_STAR_MOZILLA_MOZILLPRODUCTTYPE_HPP_
#include <com/sun/star/mozilla/MozillaProductType.hpp>
#endif
#include <com/sun/star/uno/RuntimeException.hpp>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include <rtl/ustring.hxx>
#include <vector>
#include <map>

#ifdef XP_WIN
#include <windows.h>
#endif

#ifdef XP_OS2
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#include <os2.h>
#endif

using namespace com::sun::star::mozilla;
class nsProfile;
namespace connectivity
{
    namespace mozab
    {
        class ProfileManager
        {
        public:

            virtual ~ProfileManager();
            ProfileManager();

            ::sal_Int32 SAL_CALL bootupProfile( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException) ;
            ::sal_Int32 SAL_CALL shutdownProfile(  ) throw (::com::sun::star::uno::RuntimeException) ;
            ::com::sun::star::mozilla::MozillaProductType SAL_CALL getCurrentProduct(  ) throw (::com::sun::star::uno::RuntimeException) ;
            ::rtl::OUString SAL_CALL getCurrentProfile(  ) throw (::com::sun::star::uno::RuntimeException) ;
            ::rtl::OUString SAL_CALL setCurrentProfile( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException) ;

        protected:
            ::com::sun::star::mozilla::MozillaProductType m_CurrentProduct;
            nsProfile *aProfile;
        };

    }
}

#endif // __MNSProfileManager_h___

