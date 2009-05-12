/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unohelp.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _VCL_UNOHELP_HXX
#define _VCL_UNOHELP_HXX

#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

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

