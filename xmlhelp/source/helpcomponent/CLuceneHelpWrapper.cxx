/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2010 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implementationentry.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star::reflection;
using namespace com::sun::star::script;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;

class CLuceneHelpWrapper : public WeakImplHelper2<XServiceInfo, XInvocation>
{
public:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(const OUString&)
        throw (RuntimeException);

    virtual Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (RuntimeException);

    // XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(void) throw( RuntimeException )
    {
        return Reference< XIntrospectionAccess >();
    }
    virtual Any SAL_CALL invoke(const OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
        throw( IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual void SAL_CALL setValue(const OUString&, const Any&)
        throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException )
    {
        throw UnknownPropertyException();
    }
    virtual Any SAL_CALL getValue(const OUString&) throw( UnknownPropertyException, RuntimeException )
    {
        throw UnknownPropertyException();
    }
    virtual sal_Bool SAL_CALL hasMethod(const OUString& rName) throw( RuntimeException )
    {
        return rName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("search"))
            || rName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("createIndex"));
    }
    virtual sal_Bool SAL_CALL hasProperty(const OUString&) throw( RuntimeException )
    {
        return sal_False;
    }
};

#include <stdio.h> // FIXME: remove once the fprintf() calls below are gone

Any CLuceneHelpWrapper::invoke(const OUString& rFunctionName, const Sequence< Any >&, Sequence< sal_Int16 >&, Sequence< Any >& )
        throw( IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException )
{
    fprintf(stderr, "invoke something or other, %s\n", rtl::OUStringToOString(rFunctionName, RTL_TEXTENCODING_UTF8).getStr());
    if (rFunctionName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("search")))
        fprintf(stderr, "implement me, do search thing from helpsearch.cxx here");
    else if (rFunctionName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("createIndex")))
        fprintf(stderr, "implement me, do indexing thing for extensions with help, but without pre-created index, make need to split l10ntools HelpIndexer tool into a lib and header that we can link to here");
    else
        throw IllegalArgumentException();
    return Any();
}

namespace
{
    Reference<XInterface> create( Reference<XComponentContext> const & /*xContext*/ )
    {
        return static_cast< ::cppu::OWeakObject * >(new CLuceneHelpWrapper);
    }

    OUString getImplName()
    {
        return OUString(RTL_CONSTASCII_USTRINGPARAM("libreoffice.CLuceneWrapper"));
    }

    Sequence< OUString > getSuppServices()
    {
        OUString sHelpIndexer(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.help.HelpIndexer"));
        OUString sHelpSearch(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.help.HelpSearch"));
        Sequence< OUString > aServiceNames(2);
        aServiceNames[0] = sHelpIndexer;
        aServiceNames[1] = sHelpSearch;
        return aServiceNames;
    }
}

OUString CLuceneHelpWrapper::getImplementationName()
    throw (RuntimeException)
{
    return getImplName();
}

sal_Bool CLuceneHelpWrapper::supportsService(const OUString& rService)
    throw (RuntimeException)
{
    Sequence<OUString> names(getSupportedServiceNames());
    for (sal_Int32 i = 0; i < names.getLength(); ++i)
    {
        if (names[i] == rService)
            return true;
    }
    return false;
}

Sequence< OUString > CLuceneHelpWrapper::getSupportedServiceNames()
    throw (RuntimeException)
{
    return getSuppServices();
}

namespace
{
    static ::cppu::ImplementationEntry const entries[] = {
        { create,
          getImplName,
          getSuppServices,
          ::cppu::createSingleComponentFactory, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    };
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, entries);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
