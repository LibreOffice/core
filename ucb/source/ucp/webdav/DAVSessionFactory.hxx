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
#ifndef _DAVSESSIONFACTORY_HXX_
#define _DAVSESSIONFACTORY_HXX_

#ifdef min
#undef min // GNU libstdc++ <memory> includes <limit> which defines methods called min...
#endif
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
    ~DAVSessionFactory() SAL_THROW(());

    rtl::Reference< DAVSession >
        createDAVSession( const ::rtl::OUString & inUri,
                          const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rFlags,
                          const ::com::sun::star::uno::Reference<
                               ::com::sun::star::lang::XMultiServiceFactory >&
                                rxSMgr )
            throw( DAVException );

    ::uno::Reference< ::lang::XMultiServiceFactory > getServiceFactory() {  return m_xMSF; }
private:
    typedef std::map< rtl::OUString, DAVSession * > Map;

    Map m_aMap;
    osl::Mutex m_aMutex;
    std::auto_ptr< ucbhelper::InternetProxyDecider > m_xProxyDecider;

    ::uno::Reference< ::lang::XMultiServiceFactory > m_xMSF;

    void releaseElement( DAVSession * pElement ) SAL_THROW(());

    friend class DAVSession;
};

} // namespace webdav_ucp

#endif // _DAVSESSIONFACTORY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
