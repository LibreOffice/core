/*************************************************************************
 *
 *  $RCSfile: wizardservices.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-05 14:53:13 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_DBW_WIZARDSERVICES_HXX_
#define _EXTENSIONS_DBW_WIZARDSERVICES_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//.........................................................................
namespace dbp
{
//.........................................................................

    //=====================================================================
    //= OGroupBoxSI
    //=====================================================================
    /// service info for the group box wizard
    struct OGroupBoxSI
    {
    public:
        ::rtl::OUString         getImplementationName() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                getServiceNames() const;
    };

    //=====================================================================
    //= OListComboSI
    //=====================================================================
    /// service info for the list/combo box wizard
    struct OListComboSI
    {
    public:
        ::rtl::OUString         getImplementationName() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                getServiceNames() const;
    };

    //=====================================================================
    //= OGridSI
    //=====================================================================
    /// service info for the grid wizard
    struct OGridSI
    {
    public:
        ::rtl::OUString         getImplementationName() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                getServiceNames() const;
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif _EXTENSIONS_DBW_WIZARDSERVICES_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2001/02/23 15:19:08  fs
 *  some changes / centralizations - added the list-/combobox wizard
 *
 *  Revision 1.1  2001/02/21 09:25:07  fs
 *  initial checkin - form control auto pilots
 *
 *
 *  Revision 1.0 14.02.01 10:27:36  fs
 ************************************************************************/

