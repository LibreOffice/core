/*************************************************************************
 *
 *  $RCSfile: FormsCollection.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:05 $
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

#ifndef _FRM_INTERFACE_CONTAINER_HXX_
#include "InterfaceContainer.hxx"
#endif

#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif

#ifndef _UTL_UNO3_HXX_
#include <unotools/uno3.hxx>
#endif
#ifndef _FRM_IDS_HXX_
#include "ids.hxx"
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
// oder auﬂen einen Context uebergeben bekommen
//==================================================================
typedef ::cppu::OComponentHelper FormsCollectionComponentBase;
    // else MSVC kills itself on some statements
class OFormsCollection
        :public FormsCollectionComponentBase
        ,public OInterfaceContainer
        ,public starcontainer::XChild
        ,public starlang::XServiceInfo

{
    ::osl::Mutex            m_aMutex;
    OImplementationIdsRef   m_aHoldIdHelper;
    ::utl::InterfaceRef     m_xParent;          // Parent

public:
    OFormsCollection(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    virtual ~OFormsCollection();

public:
    DECLARE_UNO3_AGG_DEFAULTS(OFormsCollection, FormsCollectionComponentBase);

    virtual staruno::Any SAL_CALL queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException);

// XTypeProvider
    virtual staruno::Sequence< staruno::Type > SAL_CALL getTypes(  ) throw(staruno::RuntimeException);
    virtual staruno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(staruno::RuntimeException);

// XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw(staruno::RuntimeException);

// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(staruno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(staruno::RuntimeException);
    virtual StringSequence SAL_CALL getSupportedServiceNames(  ) throw(staruno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// starcontainer::XChild
    virtual ::utl::InterfaceRef SAL_CALL getParent() throw(staruno::RuntimeException);
    virtual void SAL_CALL setParent(const ::utl::InterfaceRef& Parent) throw(starlang::NoSupportException, staruno::RuntimeException);
};

//.........................................................................
}   // namespace frm
//.........................................................................

