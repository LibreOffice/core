/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLMANAGER_HXX
#define INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLMANAGER_HXX

#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XManager.hpp>

namespace avmedia { namespace ogl {

class OGLManager : public ::cppu::WeakImplHelper2 < com::sun::star::media::XManager,
                                                    com::sun::star::lang::XServiceInfo >
{
public:

    explicit OGLManager( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rMgr );
    virtual ~OGLManager();

    // XManager
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > SAL_CALL createPlayer( const OUString& rURL ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
private:

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xMgr;
};

}
}

#endif // INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
