/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: writersvc.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_configmgr.hxx"

#include "writersvc.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace io    = ::com::sun::star::io;
        namespace sax   = ::com::sun::star::xml::sax;
        namespace backenduno = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

template <class BackendInterface>
struct WriterServiceTraits;
// -----------------------------------------------------------------------------
static inline void clear(rtl::OUString & _rs) { _rs = rtl::OUString(); }

// -----------------------------------------------------------------------------
template <class BackendInterface>
WriterService<BackendInterface>::WriterService(uno::Reference< uno::XComponentContext > const & _xContext)
: m_xServiceFactory(_xContext->getServiceManager(), uno::UNO_QUERY)
, m_xWriter()
{
    if (!m_xServiceFactory.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration XML Writer: Context has no service manager"));
        throw uno::RuntimeException(sMessage,NULL);
    }
}
// -----------------------------------------------------------------------------

// XInitialization
template <class BackendInterface>
void SAL_CALL
    WriterService<BackendInterface>::initialize( const uno::Sequence< uno::Any >& aArguments )
        throw (uno::Exception, uno::RuntimeException)
{
    switch(aArguments.getLength())
    {
    case 0:
        {
           break;
        }

    case 1:
        {
            if (aArguments[0] >>= m_xWriter)
                break;

            uno::Reference< io::XOutputStream > xStream;

            if (aArguments[0] >>= xStream)
            {
                this->setOutputStream(xStream);
                break;
            }

            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Cannot use argument to initialize a Configuration XML Writer"
                                                            "- SAX XDocumentHandler or XOutputStream expected"));
            throw lang::IllegalArgumentException(sMessage,*this,1);
        }
    default:
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Too many arguments to initialize a Configuration Parser"));
            throw lang::IllegalArgumentException(sMessage,*this,0);
        }
    }
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
inline
ServiceInfoHelper WriterService<BackendInterface>::getServiceInfo()
{
    return WriterServiceTraits<BackendInterface>::getServiceInfo();
}
// -----------------------------------------------------------------------------

// XServiceInfo
template <class BackendInterface>
::rtl::OUString SAL_CALL
    WriterService<BackendInterface>::getImplementationName(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getImplementationName( );
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
sal_Bool SAL_CALL
    WriterService<BackendInterface>::supportsService( const ::rtl::OUString& ServiceName )
        throw (uno::RuntimeException)
{
    return getServiceInfo().supportsService( ServiceName );
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
uno::Sequence< ::rtl::OUString > SAL_CALL
    WriterService<BackendInterface>::getSupportedServiceNames(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getSupportedServiceNames( );
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
void SAL_CALL
    WriterService<BackendInterface>::setOutputStream( const uno::Reference< io::XOutputStream >& aStream )
        throw (uno::RuntimeException)
{
    uno::Reference< io::XActiveDataSource > xDS( m_xWriter, uno::UNO_QUERY );

    if (xDS.is())
    {
        xDS->setOutputStream(aStream);
    }
    else
    {
        uno::Reference< sax::XDocumentHandler > xNewHandler = this->createHandler();

        xDS.set( xNewHandler, uno::UNO_QUERY );
        if (!xDS.is())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration XML Writer: Cannot set output stream to sax.Writer - missing interface XActiveDataSource."));
            throw uno::RuntimeException(sMessage,*this);
        }
        xDS->setOutputStream(aStream);

        m_xWriter = xNewHandler;
    }
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
uno::Reference< io::XOutputStream > SAL_CALL
    WriterService<BackendInterface>::getOutputStream(  )
        throw (uno::RuntimeException)
{
    uno::Reference< io::XActiveDataSource > xDS( m_xWriter, uno::UNO_QUERY );

    return xDS.is()? xDS->getOutputStream() : uno::Reference< io::XOutputStream >();
}
// -----------------------------------------------------------------------------

template <class BackendInterface>
uno::Reference< sax::XDocumentHandler > WriterService<BackendInterface>::getWriteHandler()
    throw (uno::RuntimeException)
{
    if (!m_xWriter.is())
        m_xWriter = this->createHandler();

    return m_xWriter;
}

// -----------------------------------------------------------------------------

template <class BackendInterface>
uno::Reference< sax::XDocumentHandler > WriterService<BackendInterface>::createHandler() const
    throw (uno::RuntimeException)
{
    try
    {
        static rtl::OUString const k_sSaxWriterSvc( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") );

        return uno::Reference< sax::XDocumentHandler >::query( getServiceFactory()->createInstance(k_sSaxWriterSvc) );
    }
    catch (uno::RuntimeException& ) { throw; }
    catch (uno::Exception& e)
    {
        lang::XInitialization * const pThis = const_cast<WriterService *>(this);
        throw lang::WrappedTargetRuntimeException(e.Message, pThis, uno::makeAny(e));
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
sal_Char const * const aLayerWriterServices[] =
{
    "com.sun.star.configuration.backend.xml.LayerWriter",
    0
};
extern // needed by SunCC 5.2, if used from template
const ServiceImplementationInfo aLayerWriterSI =
{
    "com.sun.star.comp.configuration.backend.xml.LayerWriter",
    aLayerWriterServices,
    0
};
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
template <>
struct WriterServiceTraits< backenduno::XLayerHandler >
{
    static ServiceImplementationInfo const * getServiceInfo()
    { return & aLayerWriterSI; }
};
// -----------------------------------------------------------------------------

const ServiceRegistrationInfo* getLayerWriterServiceInfo()
{ return getRegistrationInfo(& aLayerWriterSI); }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// instantiate here !
template class WriterService< backenduno::XLayerHandler >;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

