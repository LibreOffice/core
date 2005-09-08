/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commandbase.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:35:07 $
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

#ifndef _DBA_CORE_COMMANDBASE_HXX_
#define _DBA_CORE_COMMANDBASE_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OCommandBase - a base class (in fact just a container for some members)
//=                 for classes implementing the sdb.CommandDefinition service
//==========================================================================
class OCommandBase
{
public: // need public access
// <properties>
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                                                m_aLayoutInformation;
    ::rtl::OUString                             m_sCommand;
    sal_Bool                                    m_bEscapeProcessing;        // no BitField ! so it can be used with a OPropertyStateContainer
    ::rtl::OUString                             m_sUpdateTableName;
    ::rtl::OUString                             m_sUpdateSchemaName;
    ::rtl::OUString                             m_sUpdateCatalogName;
// </properties>

protected:
    OCommandBase() : m_bEscapeProcessing(sal_True) { }

};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_CORE_COMMANDBASE_HXX_


