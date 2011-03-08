/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CFG_CFGIMPORT_HXX
#define CFG_CFGIMPORT_HXX

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <osl/diagnose.h>
#include "apitools.hxx"
#include <comphelper/stl_types.hxx>
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>

#include <memory>
#include <stack>

namespace dbacfg
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

typedef ::cppu::WeakImplHelper4 <       ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::lang::XInitialization
                                    ,   ::com::sun::star::task::XJob
                                    ,   ::com::sun::star::configuration::backend::XLayerHandler
                                >   OCfgImport_COMPBASE;
// -------------
// - OCfgImport -
// -------------
class OCfgImport : public OCfgImport_COMPBASE
{
private:
    typedef ::std::pair< ::rtl::OUString,   sal_Int16>  TElementType;
    typedef ::std::stack< TElementType >                TElementStack;
    typedef ::std::vector< PropertyValue >              TDataSourceSettings;

    Reference< XMultiServiceFactory >                               m_xORB;
    Reference< XMultiServiceFactory >                               m_xOldORB;
    Reference< ::com::sun::star::configuration::backend::XLayer>    m_xLayer;
    Reference<XPropertySet>                                         m_xCurrentDS;
    Reference<XModel>                                               m_xModel;
    Reference<XPropertySet>                                         m_xCurrentObject; /// can either be a query or a table
    Reference<XPropertySet>                                         m_xCurrentColumn;
    ::std::map< sal_Int16 ,Sequence< ::rtl::OUString> >         m_aProperties;
    ::std::map< sal_Int16 ,Sequence< Any> >                      m_aValues;
    ::rtl::OUString                                                 m_sCurrentDataSourceName;
    ::rtl::OUString                                                 m_sBookmarkName;
    ::rtl::OUString                                                 m_sDocumentLocation;

    TElementStack                                                   m_aStack;
    TDataSourceSettings                                             m_aDataSourceSettings;
    sal_Bool                                                        m_bPropertyMayBeVoid;

    /** convert the old configuration settings into new database documents.
    */
    void convert();
    void createDataSource(const ::rtl::OUString& _sName);
    void createObject(sal_Bool _bQuery ,const ::rtl::OUString& _sName);
    void setProperties(sal_Int16 _eType);

protected:
    virtual ~OCfgImport()  throw();
public:

    OCfgImport( const Reference< XMultiServiceFactory >& _rxMSF );


    // XServiceInfo
    DECLARE_SERVICE_INFO_STATIC( );

    inline Reference< XMultiServiceFactory > getORB() const { return m_xORB; }

    // lang::XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);
    // task::XJob
    virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XLayerHandler
    virtual void SAL_CALL startLayer()
        throw(::com::sun::star::lang::WrappedTargetException);

    virtual void SAL_CALL endLayer()
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL overrideNode(
            const rtl::OUString& aName,
            sal_Int16 aAttributes,
            sal_Bool bClear)
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL addOrReplaceNode(
            const rtl::OUString& aName,
            sal_Int16 aAttributes)
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  addOrReplaceNodeFromTemplate(
            const rtl::OUString& aName,
            const ::com::sun::star::configuration::backend::TemplateIdentifier& aTemplate,
            sal_Int16 aAttributes )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  endNode()
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  dropNode(
            const rtl::OUString& aName )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  overrideProperty(
            const rtl::OUString& aName,
            sal_Int16 aAttributes,
            const Type& aType,
            sal_Bool bClear )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  setPropertyValue(
            const Any& aValue )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL setPropertyValueForLocale(
            const Any& aValue,
            const rtl::OUString& aLocale )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  endProperty()
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  addProperty(
            const rtl::OUString& aName,
            sal_Int16 aAttributes,
            const Type& aType )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  addPropertyWithValue(
            const rtl::OUString& aName,
            sal_Int16 aAttributes,
            const Any& aValue )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );
};
// -----------------------------------------------------------------------------
} // dbacfg
// -----------------------------------------------------------------------------
#endif // CFG_CFGIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
