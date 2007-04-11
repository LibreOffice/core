/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unohelp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:15:18 $
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

#ifndef _VCL_UNOHELP_HXX
#define _VCL_UNOHELP_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

namespace com {
namespace sun {
namespace star {
namespace i18n {
    class XBreakIterator;
    class XCharacterClassification;
    class XCollator;
}
namespace lang {
    class XMultiServiceFactory;
}
}}}

namespace com {
namespace sun {
namespace star {
namespace accessibility {
    struct AccessibleEventObject;
}
}}}

namespace vcl
{
namespace unohelper
{

::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > VCL_DLLPUBLIC GetMultiServiceFactory();
::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > VCL_DLLPUBLIC CreateBreakIterator();
::com::sun::star::uno::Reference < ::com::sun::star::i18n::XCharacterClassification> VCL_DLLPUBLIC CreateCharacterClassification();
::com::sun::star::uno::Reference < ::com::sun::star::i18n::XCollator > VCL_DLLPUBLIC CreateCollator();
::rtl::OUString VCL_DLLPUBLIC CreateLibraryName( const sal_Char* pModName, sal_Bool bSUPD );
void VCL_DLLPUBLIC NotifyAccessibleStateEventGlobally( const ::com::sun::star::accessibility::AccessibleEventObject& rEventObject );
}}  // namespace vcl::unohelper

#endif  // _VCL_UNOHELP_HXX

