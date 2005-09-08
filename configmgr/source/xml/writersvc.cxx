/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: writersvc.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:43:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "writersvc.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERHANDLER_HPP_
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETRUNTIMEEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

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
static inline void clear(OUString & _rs) { _rs = OUString(); }

// -----------------------------------------------------------------------------
template <class BackendInterface>
WriterService<BackendInterface>::WriterService(CreationArg _xContext)
: m_xServiceFactory(_xContext->getServiceManager(), uno::UNO_QUERY)
, m_xWriter()
{
    if (!m_xServiceFactory.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration XML Writer: Context has no service manager"));
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

            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Cannot use argument to initialize a Configuration XML Writer"
                                                            "- SAX XDocumentHandler or XOutputStream expected"));
            throw lang::IllegalArgumentException(sMessage,*this,1);
        }
    default:
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Too many arguments to initialize a Configuration Parser"));
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
        SaxHandler xNewHandler = this->createHandler();

        xDS.set( xNewHandler, uno::UNO_QUERY );
        if (!xDS.is())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration XML Writer: Cannot set output stream to sax.Writer - missing interface XActiveDataSource."));
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

        return SaxHandler::query( getServiceFactory()->createInstance(k_sSaxWriterSvc) );
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
AsciiServiceName const aLayerWriterServices[] =
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
    typedef backenduno::XLayerHandler Handler;

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

