/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSProfileManager.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:48:48 $
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

#ifndef __MNSProfileManager_h___
#define __MNSProfileManager_h___

#include "mozilla_profilemanager.h"

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

