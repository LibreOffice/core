/*************************************************************************
 *
 *  $RCSfile: commandcontainer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-18 16:15:16 $
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

#ifndef _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_
#define _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_

#ifndef _DBA_CORE_DEFINITIONCONTAINER_HXX_
#include "definitioncontainer.hxx"
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OCommandContainer - collection of command descriptions
//==========================================================================

class OCommandContainer :public ODefinitionContainer
{
public:
    /** constructs the container.<BR>
        @param      _rParent                the parent object which is used for ref counting
        @param      _rMutex                 the parent's mutex object for access safety
    */
    OCommandContainer(
        ::cppu::OWeakObject& _rParent,
        ::osl::Mutex& _rMutex
        );
    virtual ~OCommandContainer();

protected:
// ODefinitionContainer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createObject(
        const ::rtl::OUString& _rName,
        const OConfigurationNode& _rObjectNode
    );
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_


