/*************************************************************************
 *
 *  $RCSfile: commanddefinition.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:07:37 $
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

#ifndef _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_
#define _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_

#ifndef _DBA_CORE_COMMANDBASE_HXX_
#include "commandbase.hxx"
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _DBA_REGHELPER_HXX_
#include "dba_reghelper.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XRENAME_HPP_
#include <com/sun/star/sdbcx/XRename.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _DBA_CORE_DATASETTINGS_HXX_
#include "datasettings.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBA_CONTENTHELPER_HXX
#include "ContentHelper.hxx"
#endif
#ifndef DBA_COREDATAACESS_COMPONENTDEFINITION_HXX
#include "ComponentDefinition.hxx"
#endif


//........................................................................
namespace dbaccess
{
//........................................................................

//=========================================================================
//= OCommandDefinition - a database "document" which describes a query
//=========================================================================
    class OCommandDefinition_Impl : public OComponentDefinition_Impl
                                  ,public OCommandBase
    {
    public:
    };
typedef ::cppu::ImplHelper1 <   ::com::sun::star::sdbcx::XRename
                                >   OCommandDefinition_Base;
class OCommandDefinition;
typedef ::comphelper::OPropertyArrayUsageHelper< OCommandDefinition >
                        OCommandDefinition_PROP;


class OCommandDefinition    :public OComponentDefinition
                            ,public OCommandDefinition_Base
                            ,public OCommandDefinition_PROP
{
protected:
    virtual ~OCommandDefinition();

    OCommandDefinition(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&   _xParentContainer
        ,const TContentPtr& _pImpl
        );
public:

    OCommandDefinition(
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer
            ,const ::rtl::OUString& _rElementName
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
            ,const TContentPtr& _pImpl
        );

// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    // XRename
    virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    DECLARE_PROPERTYCONTAINER_DEFAULTS( );
private:
    // helper
    void registerProperties();
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_


