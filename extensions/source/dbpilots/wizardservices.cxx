/*************************************************************************
 *
 *  $RCSfile: wizardservices.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-21 09:24:59 $
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

#ifndef _EXTENSIONS_DBP_WIZARDSERVICES_HXX_
#include "wizardservices.hxx"
#endif
#ifndef _EXTENSIONS_DBP_UNOAUTOPILOT_HXX_
#include "unoautopilot.hxx"
#endif
#ifndef _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_
#include "groupboxwiz.hxx"
#endif

// the registration methods
extern "C" void SAL_CALL createRegistryInfo_OGroupBoxWizard()
{
    static ::dbp::OMultiInstanceAutoRegistration<
        ::dbp::OUnoAutoPilot< ::dbp::OGroupBoxWizard, ::dbp::OGroupBoxSI >
    > aAutoRegistration;
}

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;

    //=====================================================================
    //= OGroupBoxSI
    //=====================================================================
    //---------------------------------------------------------------------
    ::rtl::OUString OGroupBoxSI::getImplementationName() const
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbp.OGroupBoxWizard");
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > OGroupBoxSI::getServiceNames() const
    {
        Sequence< ::rtl::OUString > aReturn(1);
        aReturn[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.GroupBoxAutoPilot");
        return aReturn;
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 14.02.01 10:24:18  fs
 ************************************************************************/

