/*************************************************************************
 *
 *  $RCSfile: FormattedFieldWrapper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:01:16 $
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

#ifndef _FRM_FORMATTED_FIELD_WRAPPER_HXX_
#define _FRM_FORMATTED_FIELD_WRAPPER_HXX_

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

//.........................................................................
namespace frm
{

class OEditModel;
//==================================================================
//= OFormattedFieldWrapper
//==================================================================
typedef ::cppu::WeakAggImplHelper3  <   ::com::sun::star::io::XPersistObject
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::util::XCloneable
                                    >   OFormattedFieldWrapper_Base;

class OFormattedFieldWrapper : public OFormattedFieldWrapper_Base
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xServiceFactory;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation>      m_xAggregate;

    OEditModel*     m_pEditPart;
        // if we act as formatted this is used to write the EditModel part
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>     m_xFormattedPart;
        // if we act as formatted, this is the PersistObject interface of our aggregate, used
        // to read and write the FormattedModel part

    OFormattedFieldWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory, sal_Bool _bActAsFormatted);
        // if _bActAsFormatted is sal_False, the state is undetermined until somebody calls ::read or does
        // anything which requires a living aggregate
    OFormattedFieldWrapper( const OFormattedFieldWrapper* _pCloneSource );
    virtual ~OFormattedFieldWrapper();

    friend InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    friend InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

public:
    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OFormattedFieldWrapper, OWeakAggObject);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

protected:
    /// ensure we're in a defined state, which means a FormattedModel _OR_ an EditModel
    void ensureAggregate();
};

//.........................................................................
}
//.........................................................................

#endif // _FRM_FORMATTED_FIELD_WRAPPER_HXX_

