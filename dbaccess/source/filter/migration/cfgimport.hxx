/*************************************************************************
 *
 *  $RCSfile: cfgimport.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 15:18:07 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

#ifndef CFG_CFGIMPORT_HXX
#define CFG_CFGIMPORT_HXX

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERHANDLER_HPP_
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
#endif

#include <memory>
#include <stack>

namespace dbacfg
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

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
    Reference<XPropertySet>                                         m_xCurrentObject; /// can either be a query or a table
    Reference<XPropertySet>                                         m_xCurrentColumn;
    Sequence< ::rtl::OUString>                                      m_aProperties;
    Sequence< Any>                                                  m_aValues;
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
    void setProperties();

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
