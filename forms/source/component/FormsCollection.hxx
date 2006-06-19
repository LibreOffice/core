/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormsCollection.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 12:50:09 $
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

#ifndef _FRM_INTERFACE_CONTAINER_HXX_
#include "InterfaceContainer.hxx"
#endif
#ifndef _FRM_IDS_HXX_
#include "ids.hxx"
#endif

#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OFormsCollection
// Implementiert den UNO-Container fuer Formulare
// enthaelt alle zugeordneten Forms
// dieses Container kann selbst den Context fuer Formulare darstellen
// oder aussen einen Context uebergeben bekommen
//==================================================================
typedef ::cppu::OComponentHelper FormsCollectionComponentBase;
typedef ::cppu::ImplHelper2<    ::com::sun::star::container::XChild
                                ,::com::sun::star::lang::XServiceInfo > OFormsCollection_BASE;

    // else MSVC kills itself on some statements
class OFormsCollection
        :public FormsCollectionComponentBase
        ,public OInterfaceContainer
        ,public OFormsCollection_BASE
{
    ::osl::Mutex                m_aMutex;
    OImplementationIdsRef       m_aHoldIdHelper;
    ::comphelper::InterfaceRef  m_xParent;          // Parent

public:
    OFormsCollection(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OFormsCollection();

public:
    DECLARE_UNO3_AGG_DEFAULTS(OFormsCollection, FormsCollectionComponentBase);

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual StringSequence SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // ::com::sun::star::container::XChild
    virtual ::comphelper::InterfaceRef SAL_CALL getParent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent(const ::comphelper::InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    // prevent method hiding
    using OInterfaceContainer::disposing;
};

//.........................................................................
}   // namespace frm
//.........................................................................

