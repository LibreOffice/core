/*************************************************************************
 *
 *  $RCSfile: VView.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:29 $
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
#ifndef _CONNECTIVITY_SDBCX_VIEW_HXX_
#include "connectivity/sdbcx/VView.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(OView,"com.sun.star.sdbcx.VView","com.sun.star.sdbcx.View");
// -------------------------------------------------------------------------
OView::OView(sal_Bool _bCase,
            const ::rtl::OUString& _Name,
            sal_Int32 _CheckOption,
            const ::rtl::OUString& _Command,
            const ::rtl::OUString& _SchemaName,
            const ::rtl::OUString& _CatalogName) : ODescriptor(OViewHelper::rBHelper,_bCase)
            ,m_CatalogName(_CatalogName)
            ,m_SchemaName(_SchemaName)
            ,m_Command(_Command)
            ,m_CheckOption(_CheckOption)

{
    m_Name = _Name;
}
// -------------------------------------------------------------------------
OView::OView(sal_Bool _bCase): ODescriptor(OViewHelper::rBHelper,_bCase,sal_True)
{
}
// -------------------------------------------------------------------------
void OView::construct()
{
    ODescriptor::construct();

        sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(PROPERTY_CATALOGNAME,      PROPERTY_ID_CATALOGNAME,nAttrib,&m_CatalogName, ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_SCHEMANAME,       PROPERTY_ID_SCHEMANAME, nAttrib,&m_SchemaName,  ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_COMMAND,          PROPERTY_ID_COMMAND,    nAttrib,&m_Command,     ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_CHECKOPTION,      PROPERTY_ID_CHECKOPTION,nAttrib,&m_CheckOption, ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
}
// -------------------------------------------------------------------------
void OView::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OView::getTypes(  ) throw(RuntimeException)
{
        Sequence< Type > aTypes(2);
        aTypes.getArray()[0] = ::getCppuType(static_cast< Reference< ::com::sun::star::container::XNamed> *> (NULL));
        aTypes.getArray()[1] = ::getCppuType(static_cast< Reference< XServiceInfo> *> (NULL));

    return ::utl::concatSequences(ODescriptor::getTypes(),aTypes);
}
// -------------------------------------------------------------------------
Any SAL_CALL OView::queryInterface( const Type & rType ) throw(RuntimeException)
{
        Any aRet = ::cppu::queryInterface(rType,static_cast< ::com::sun::star::container::XNamed*> (this),
                                                                                                         static_cast< XServiceInfo*> (this));
    if(aRet.hasValue())
        return aRet;
    return ODescriptor::queryInterface( rType);;
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OView::createArrayHelper( ) const
{
        Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);

}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OView::getInfoHelper()
{
    return *const_cast<OView*>(this)->getArrayHelper();
}

