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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "cppuhelper/implbase4.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "rtl/bootstrap.hxx"
#include "sal/types.h"
#include "sal/config.h"
#include "boost/scoped_array.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/task/XJob.hpp"
#include "com/sun/star/configuration/backend/XLayer.hpp"
#include "com/sun/star/configuration/backend/XLayerHandler.hpp"
#include "com/sun/star/configuration/backend/MalformedDataException.hpp"
#include "com/sun/star/configuration/backend/TemplateIdentifier.hpp"
#include "jvmfwk/framework.h"
#include "jvmfwk.hxx"
#include <stack>
#include <stdio.h>

#include "osl/thread.hxx"
using ::rtl::OUString;

#define OUSTR(x) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( x ))

#define SERVICE_NAME "com.sun.star.migration.Java"
#define IMPL_NAME "com.sun.star.comp.desktop.migration.Java"

#define ENABLE_JAVA     1
#define USER_CLASS_PATH 2

namespace css = com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::configuration::backend;

namespace migration
{

class CJavaInfo
{
    CJavaInfo(const CJavaInfo&);
    CJavaInfo& operator = (const CJavaInfo&);
public:
    JavaInfo* pData;
    CJavaInfo();
    ~CJavaInfo();
    operator JavaInfo* () const;
};

CJavaInfo::CJavaInfo(): pData(NULL)
{
}

CJavaInfo::~CJavaInfo()
{
    jfw_freeJavaInfo(pData);
}

CJavaInfo::operator JavaInfo*() const
{
    return pData;
}


class JavaMigration : public ::cppu::WeakImplHelper4<
    css::lang::XServiceInfo,
    css::lang::XInitialization,
    css::task::XJob,
    css::configuration::backend::XLayerHandler>
{
public:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName )
        throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException);

    //XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw(css::uno::Exception, css::uno::RuntimeException);

    //XJob
    virtual css::uno::Any SAL_CALL execute(
        const css::uno::Sequence<css::beans::NamedValue >& Arguments )
        throw (css::lang::IllegalArgumentException, css::uno::Exception,
               css::uno::RuntimeException);

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
            const css::uno::Type& aType,
            sal_Bool bClear )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  setPropertyValue(
            const css::uno::Any& aValue )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL setPropertyValueForLocale(
            const css::uno::Any& aValue,
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
            const css::uno::Type& aType )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  addPropertyWithValue(
            const rtl::OUString& aName,
            sal_Int16 aAttributes,
            const css::uno::Any& aValue )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );



    //----------------
    ~JavaMigration();

private:
    OUString m_sUserDir;
    css::uno::Reference< ::css::configuration::backend::XLayer> m_xLayer;

    void migrateJavarc();
    typedef ::std::pair< ::rtl::OUString,   sal_Int16>  TElementType;
    typedef ::std::stack< TElementType > TElementStack;
    TElementStack m_aStack;

};

JavaMigration::~JavaMigration()
{
    OSL_ASSERT(m_aStack.empty());
}

OUString jvmfwk_getImplementationName()
{
    return OUSTR(IMPL_NAME);
}

css::uno::Sequence< OUString > jvmfwk_getSupportedServiceNames()
{
    OUString str_name = OUSTR(SERVICE_NAME);
    return css::uno::Sequence< OUString >( &str_name, 1 );
}

// XServiceInfo
OUString SAL_CALL JavaMigration::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return jvmfwk_getImplementationName();
}

sal_Bool SAL_CALL JavaMigration::supportsService( const OUString & rServiceName )
        throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > const & rSNL = getSupportedServiceNames();
    OUString const * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (rServiceName.equals( pArray[ nPos ] ))
            return true;
    }
    return false;

}

css::uno::Sequence< OUString > SAL_CALL JavaMigration::getSupportedServiceNames()
        throw (css::uno::RuntimeException)
{
    return jvmfwk_getSupportedServiceNames();
}

//XInitialization ----------------------------------------------------------------------
void SAL_CALL JavaMigration::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw(css::uno::Exception, css::uno::RuntimeException)
{
    const css::uno::Any* pIter = aArguments.getConstArray();
    const css::uno::Any* pEnd = pIter + aArguments.getLength();
    css::uno::Sequence<css::beans::NamedValue> aOldConfigValues;
    css::beans::NamedValue aValue;
    for(;pIter != pEnd;++pIter)
    {
        *pIter >>= aValue;
        if (aValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("OldConfiguration")))
        {
            sal_Bool bSuccess = aValue.Value >>= aOldConfigValues;
            OSL_ENSURE(bSuccess == sal_True, "[Service implementation " IMPL_NAME
                       "] XInitialization::initialize: Argument OldConfiguration has wrong type.");
            if (bSuccess)
            {
                const css::beans::NamedValue* pIter2 = aOldConfigValues.getConstArray();
                const css::beans::NamedValue* pEnd2 = pIter2 + aOldConfigValues.getLength();
                for(;pIter2 != pEnd2;++pIter2)
                {
                    if ( pIter2->Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("org.openoffice.Office.Java")) )
                    {
                        pIter2->Value >>= m_xLayer;
                        break;
                    }
                }
            }
        }
        else if (aValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("UserData")))
        {
            if ( !(aValue.Value >>= m_sUserDir) )
            {
                OSL_ENSURE(
                    false,
                    "[Service implementation " IMPL_NAME
                    "] XInitialization::initialize: Argument UserData has wrong type.");
            }
        }
    }

}

//XJob
css::uno::Any SAL_CALL JavaMigration::execute(
        const css::uno::Sequence<css::beans::NamedValue >& )
        throw (css::lang::IllegalArgumentException, css::uno::Exception,
               css::uno::RuntimeException)
{
    migrateJavarc();
    if (m_xLayer.is())
        m_xLayer->readData(this);

    return css::uno::Any();
}

void JavaMigration::migrateJavarc()
{
    if (m_sUserDir.getLength() == 0)
        return;

    OUString sValue;
    rtl::Bootstrap javaini(m_sUserDir + OUSTR("/user/config/"SAL_CONFIGFILE("java")));
    sal_Bool bSuccess = javaini.getFrom(OUSTR("Home"), sValue);
    OSL_ENSURE(bSuccess, "[Service implementation " IMPL_NAME
                       "] XJob::execute: Could not get Home entry from java.ini/javarc.");
    if (bSuccess == sal_True && sValue.getLength() > 0)
    {
        //get the directory
        CJavaInfo aInfo;
        javaFrameworkError err = jfw_getJavaInfoByPath(sValue.pData, &aInfo.pData);

        if (err == JFW_E_NONE)
        {
            if (jfw_setSelectedJRE(aInfo) != JFW_E_NONE)
            {
                OSL_FAIL("[Service implementation " IMPL_NAME
                           "] XJob::execute: jfw_setSelectedJRE failed.");
                fprintf(stderr, "\nCannot migrate Java. An error occurred.\n");
            }
        }
        else if (err == JFW_E_FAILED_VERSION)
        {
            fprintf(stderr, "\nCannot migrate Java settings because the version of the Java  "
                    "is not supported anymore.\n");
        }
    }
}


// XLayerHandler
void SAL_CALL JavaMigration::startLayer()
    throw(css::lang::WrappedTargetException)
{
}
// -----------------------------------------------------------------------------

void SAL_CALL JavaMigration::endLayer()
    throw(
        MalformedDataException,
        WrappedTargetException )
{
}
// -----------------------------------------------------------------------------

void SAL_CALL JavaMigration::overrideNode(
        const ::rtl::OUString&,
        sal_Int16,
        sal_Bool)
    throw(
        MalformedDataException,
        WrappedTargetException )

{

}
// -----------------------------------------------------------------------------

void SAL_CALL JavaMigration::addOrReplaceNode(
        const ::rtl::OUString&,
        sal_Int16)
    throw(
        MalformedDataException,
        WrappedTargetException )
{

}
void SAL_CALL  JavaMigration::endNode()
    throw(
        MalformedDataException,
        WrappedTargetException )
{
}
// -----------------------------------------------------------------------------

void SAL_CALL  JavaMigration::dropNode(
        const ::rtl::OUString& )
    throw(
        MalformedDataException,
        WrappedTargetException )
{
}
// -----------------------------------------------------------------------------

void SAL_CALL  JavaMigration::overrideProperty(
        const ::rtl::OUString& aName,
        sal_Int16,
        const Type&,
        sal_Bool )
    throw(
        MalformedDataException,
        WrappedTargetException )
{
    if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Enable")))
        m_aStack.push(TElementStack::value_type(aName,ENABLE_JAVA));
    else if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("UserClassPath")))
        m_aStack.push(TElementStack::value_type(aName, USER_CLASS_PATH));
}
// -----------------------------------------------------------------------------

void SAL_CALL  JavaMigration::setPropertyValue(
        const Any& aValue )
    throw(
        MalformedDataException,
        WrappedTargetException )
{
    if ( !m_aStack.empty())
    {
        switch (m_aStack.top().second)
        {
        case ENABLE_JAVA:
        {
            sal_Bool val = sal_Bool();
            if ((aValue >>= val) == sal_False)
                throw MalformedDataException(
                    OUSTR("[Service implementation " IMPL_NAME
                       "] XLayerHandler::setPropertyValue received wrong type for Enable property"), 0, Any());
            if (jfw_setEnabled(val) != JFW_E_NONE)
                throw WrappedTargetException(
                    OUSTR("[Service implementation " IMPL_NAME
                       "] XLayerHandler::setPropertyValue: jfw_setEnabled failed."), 0, Any());

            break;
        }
        case USER_CLASS_PATH:
         {
             OUString cp;
             if ((aValue >>= cp) == sal_False)
                 throw MalformedDataException(
                     OUSTR("[Service implementation " IMPL_NAME
                           "] XLayerHandler::setPropertyValue received wrong type for UserClassPath property"), 0, Any());

             if (jfw_setUserClassPath(cp.pData) != JFW_E_NONE)
                 throw WrappedTargetException(
                     OUSTR("[Service implementation " IMPL_NAME
                       "] XLayerHandler::setPropertyValue: jfw_setUserClassPath failed."), 0, Any());
             break;
         }
        default:
            OSL_ASSERT(0);
        }
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL JavaMigration::setPropertyValueForLocale(
        const Any&,
        const ::rtl::OUString& )
    throw(
        MalformedDataException,
        WrappedTargetException )
{
}
// -----------------------------------------------------------------------------

void SAL_CALL  JavaMigration::endProperty()
    throw(
        MalformedDataException,
        WrappedTargetException )
{
            if (!m_aStack.empty())
                m_aStack.pop();
}
// -----------------------------------------------------------------------------

void SAL_CALL  JavaMigration::addProperty(
        const rtl::OUString&,
        sal_Int16,
        const Type& )
    throw(
        MalformedDataException,
        WrappedTargetException )
{
}
// -----------------------------------------------------------------------------

void SAL_CALL  JavaMigration::addPropertyWithValue(
        const rtl::OUString&,
        sal_Int16,
        const Any& )
    throw(
        MalformedDataException,
        WrappedTargetException )
{
}

void SAL_CALL JavaMigration::addOrReplaceNodeFromTemplate(
        const rtl::OUString&,
        const TemplateIdentifier&,
        sal_Int16 )
    throw(
        MalformedDataException,
        WrappedTargetException )
{
}

// -----------------------------------------------------------------------------
//ToDo enable java, user class path

} //end namespace jfw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
