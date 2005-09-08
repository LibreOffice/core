/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventattachermgr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:30:27 $
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

#ifndef _COMPHELPER_EVENTATTACHERMGR_HXX_
#define _COMPHELPER_EVENTATTACHERMGR_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

namespace com { namespace sun { namespace star {
namespace uno {
    class Exception;
}
namespace lang {
    class XMultiServiceFactory;
}
namespace script {
    class XEventAttacherManager;
}
namespace beans {
    class XIntrospection;
}
} } }


namespace comphelper
{

COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >
createEventAttacherManager(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospection > & rIntrospection,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr )
    throw( ::com::sun::star::uno::Exception );


COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >
createEventAttacherManager(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr )
    throw( ::com::sun::star::uno::Exception );

}

#endif // _COMPHELPER_EVENTATTACHERMGR_HXX_

