/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertyinfo.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:25:27 $
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

#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_
#define _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#include <vector>

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= IPropertyInfoService
    //========================================================================
    class IPropertyInfoService
    {
    public:
        virtual sal_Int32               getPropertyId(const String& _rName) const = 0;
        virtual String                  getPropertyTranslation(sal_Int32 _nId) const = 0;
        virtual sal_Int32               getPropertyHelpId(sal_Int32 _nId) const = 0;
        virtual sal_Int16               getPropertyPos(sal_Int32 _nId) const = 0;
        virtual sal_uInt32              getPropertyUIFlags(sal_Int32 _nId) const = 0;
        virtual ::std::vector< String > getPropertyEnumRepresentations(sal_Int32 _nId) const = 0;
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_

