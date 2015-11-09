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
#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVSESSIONFACTORY_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVSESSIONFACTORY_HXX

#ifdef min
#undef min // GNU libstdc++ <memory> includes <limit> which defines methods called min...
#endif
#include <config_lgpl.h>
#include <map>
#include <memory>
#include <osl/mutex.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <ucbhelper/proxydecider.hxx>
#include "DAVException.hxx"

using namespace com::sun::star;

namespace com { namespace sun { namespace star { namespace beans {
    struct NamedValue;
} } } }

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace webdav_ucp
{

class DAVSession;

class DAVSessionFactory : public salhelper::SimpleReferenceObject
{
public:
    virtual ~DAVSessionFactory();

    rtl::Reference< DAVSession >
        createDAVSession( const OUString & inUri,
                          const ::uno::Sequence< css::beans::NamedValue >& rFlags,
                          const ::uno::Reference< ::uno::XComponentContext >& rxContext )
            throw( DAVException );

    ::uno::Reference< ::uno::XComponentContext > getComponentContext() {  return m_xContext; }
private:
    typedef std::map< OUString, DAVSession * > Map;

    Map m_aMap;
    osl::Mutex m_aMutex;
    std::unique_ptr< ucbhelper::InternetProxyDecider > m_xProxyDecider;

    ::uno::Reference< ::uno::XComponentContext > m_xContext;

    void releaseElement( DAVSession * pElement );

    friend class DAVSession;
};

} // namespace webdav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVSESSIONFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
