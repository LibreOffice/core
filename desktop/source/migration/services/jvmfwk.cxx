/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/types.h>
#include <sal/config.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>
#include <com/sun/star/configuration/backend/TemplateIdentifier.hpp>
#include <jvmfwk/framework.hxx>
#include "jvmfwk.hxx"
#include <memory>
#include <stack>
#include <stdio.h>

#include <osl/diagnose.h>

constexpr OUString SERVICE_NAME = u"com.sun.star.migration.Java"_ustr;
#define IMPL_NAME "com.sun.star.comp.desktop.migration.Java"

#define ENABLE_JAVA     1
#define USER_CLASS_PATH 2

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::configuration::backend;

namespace migration
{

namespace {

class JavaMigration : public ::cppu::WeakImplHelper<
    css::lang::XServiceInfo,
    css::lang::XInitialization,
    css::task::XJob,
    css::configuration::backend::XLayerHandler>
{
public:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    //XJob
    virtual css::uno::Any SAL_CALL execute(
        const css::uno::Sequence<css::beans::NamedValue >& Arguments ) override;

        // XLayerHandler
    virtual void SAL_CALL startLayer() override;

    virtual void SAL_CALL endLayer() override;

    virtual void SAL_CALL overrideNode(
            const OUString& aName,
            sal_Int16 aAttributes,
            sal_Bool bClear) override;

    virtual void SAL_CALL addOrReplaceNode(
            const OUString& aName,
            sal_Int16 aAttributes) override;

    virtual void SAL_CALL  addOrReplaceNodeFromTemplate(
            const OUString& aName,
            const css::configuration::backend::TemplateIdentifier& aTemplate,
            sal_Int16 aAttributes ) override;

    virtual void SAL_CALL  endNode() override;

    virtual void SAL_CALL  dropNode(
            const OUString& aName ) override;

    virtual void SAL_CALL  overrideProperty(
            const OUString& aName,
            sal_Int16 aAttributes,
            const css::uno::Type& aType,
            sal_Bool bClear ) override;

    virtual void SAL_CALL  setPropertyValue(
            const css::uno::Any& aValue ) override;

    virtual void SAL_CALL setPropertyValueForLocale(
            const css::uno::Any& aValue,
            const OUString& aLocale ) override;

    virtual void SAL_CALL  endProperty() override;

    virtual void SAL_CALL  addProperty(
            const OUString& aName,
            sal_Int16 aAttributes,
            const css::uno::Type& aType ) override;

    virtual void SAL_CALL  addPropertyWithValue(
            const OUString& aName,
            sal_Int16 aAttributes,
            const css::uno::Any& aValue ) override;


    virtual ~JavaMigration() override;

private:
    OUString m_sUserDir;
    css::uno::Reference< css::configuration::backend::XLayer> m_xLayer;

    void migrateJavarc();
    typedef std::pair< OUString,   sal_Int16>  TElementType;
    typedef std::stack< TElementType > TElementStack;
    TElementStack m_aStack;

};

}

JavaMigration::~JavaMigration()
{
    OSL_ASSERT(m_aStack.empty());
}

OUString jvmfwk_getImplementationName()
{
    return u"" IMPL_NAME ""_ustr;
}

css::uno::Sequence< OUString > jvmfwk_getSupportedServiceNames()
{
    return { SERVICE_NAME };
}

// XServiceInfo
OUString SAL_CALL JavaMigration::getImplementationName()
{
    return jvmfwk_getImplementationName();
}

sal_Bool JavaMigration::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL JavaMigration::getSupportedServiceNames()
{
    return jvmfwk_getSupportedServiceNames();
}

//XInitialization ----------------------------------------------------------------------
void SAL_CALL JavaMigration::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    const css::uno::Any* pIter = aArguments.getConstArray();
    const css::uno::Any* pEnd = pIter + aArguments.getLength();
    css::uno::Sequence<css::beans::NamedValue> aOldConfigValues;
    css::beans::NamedValue aValue;
    for(;pIter != pEnd;++pIter)
    {
        *pIter >>= aValue;
        if ( aValue.Name == "OldConfiguration" )
        {
            bool bSuccess = aValue.Value >>= aOldConfigValues;
            OSL_ENSURE(bSuccess, "[Service implementation " IMPL_NAME
                       "] XInitialization::initialize: Argument OldConfiguration has wrong type.");
            if (bSuccess)
            {
                const css::beans::NamedValue* pIter2 = aOldConfigValues.getConstArray();
                const css::beans::NamedValue* pEnd2 = pIter2 + aOldConfigValues.getLength();
                for(;pIter2 != pEnd2;++pIter2)
                {
                    if ( pIter2->Name == "org.openoffice.Office.Java" )
                    {
                        pIter2->Value >>= m_xLayer;
                        break;
                    }
                }
            }
        }
        else if ( aValue.Name == "UserData" )
        {
            if ( !(aValue.Value >>= m_sUserDir) )
            {
                OSL_FAIL(
                    "[Service implementation " IMPL_NAME
                    "] XInitialization::initialize: Argument UserData has wrong type.");
            }
        }
    }

}

//XJob
css::uno::Any SAL_CALL JavaMigration::execute(
        const css::uno::Sequence<css::beans::NamedValue >& )
{
    migrateJavarc();
    if (m_xLayer.is())
        m_xLayer->readData(this);

    return css::uno::Any();
}

void JavaMigration::migrateJavarc()
{
    if (m_sUserDir.isEmpty())
        return;

    OUString sValue;
    rtl::Bootstrap javaini(m_sUserDir + "/user/config/" SAL_CONFIGFILE("java"));
    bool bSuccess = javaini.getFrom(u"Home"_ustr, sValue);
    OSL_ENSURE(bSuccess, "[Service implementation " IMPL_NAME
                       "] XJob::execute: Could not get Home entry from java.ini/javarc.");
    if (!bSuccess || sValue.isEmpty())
        return;

    //get the directory
    std::unique_ptr<JavaInfo> aInfo;
    javaFrameworkError err = jfw_getJavaInfoByPath(sValue, &aInfo);

    if (err == JFW_E_NONE)
    {
        if (jfw_setSelectedJRE(aInfo.get()) != JFW_E_NONE)
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


// XLayerHandler
void SAL_CALL JavaMigration::startLayer()
{
}


void SAL_CALL JavaMigration::endLayer()
{
}


void SAL_CALL JavaMigration::overrideNode(
        const OUString&,
        sal_Int16,
        sal_Bool)

{

}


void SAL_CALL JavaMigration::addOrReplaceNode(
        const OUString&,
        sal_Int16)
{

}
void SAL_CALL  JavaMigration::endNode()
{
}


void SAL_CALL  JavaMigration::dropNode(
        const OUString& )
{
}


void SAL_CALL  JavaMigration::overrideProperty(
        const OUString& aName,
        sal_Int16,
        const Type&,
        sal_Bool )
{
    if ( aName == "Enable" )
        m_aStack.push(TElementStack::value_type(aName,ENABLE_JAVA));
    else if ( aName == "UserClassPath" )
        m_aStack.push(TElementStack::value_type(aName, USER_CLASS_PATH));
}


void SAL_CALL  JavaMigration::setPropertyValue(
        const Any& aValue )
{
    if ( m_aStack.empty())
        return;

    switch (m_aStack.top().second)
    {
    case ENABLE_JAVA:
    {
        bool val;
        if (!(aValue >>= val))
            throw MalformedDataException(
                   u"[Service implementation " IMPL_NAME
                   "] XLayerHandler::setPropertyValue received wrong type for Enable property"_ustr, nullptr, Any());
        if (jfw_setEnabled(val) != JFW_E_NONE)
            throw WrappedTargetException(
                   u"[Service implementation " IMPL_NAME
                   "] XLayerHandler::setPropertyValue: jfw_setEnabled failed."_ustr, nullptr, Any());

        break;
    }
    case USER_CLASS_PATH:
     {
         OUString cp;
         if (!(aValue >>= cp))
             throw MalformedDataException(
                       u"[Service implementation " IMPL_NAME
                       "] XLayerHandler::setPropertyValue received wrong type for UserClassPath property"_ustr, nullptr, Any());

         if (jfw_setUserClassPath(cp) != JFW_E_NONE)
             throw WrappedTargetException(
                   u"[Service implementation " IMPL_NAME
                   "] XLayerHandler::setPropertyValue: jfw_setUserClassPath failed."_ustr, nullptr, Any());
         break;
     }
    default:
        OSL_ASSERT(false);
    }
}


void SAL_CALL JavaMigration::setPropertyValueForLocale(
        const Any&,
        const OUString& )
{
}


void SAL_CALL  JavaMigration::endProperty()
{
            if (!m_aStack.empty())
                m_aStack.pop();
}


void SAL_CALL  JavaMigration::addProperty(
        const OUString&,
        sal_Int16,
        const Type& )
{
}


void SAL_CALL  JavaMigration::addPropertyWithValue(
        const OUString&,
        sal_Int16,
        const Any& )
{
}

void SAL_CALL JavaMigration::addOrReplaceNodeFromTemplate(
        const OUString&,
        const TemplateIdentifier&,
        sal_Int16 )
{
}


//ToDo enable java, user class path

} //end namespace jfw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
