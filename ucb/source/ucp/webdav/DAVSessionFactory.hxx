/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DAVSessionFactory.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 10:03:31 $
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
#ifndef _DAVSESSIONFACTORY_HXX_
#define _DAVSESSIONFACTORY_HXX_

#ifdef min
#undef min // GNU libstdc++ <memory> includes <limit> which defines methods called min...
#endif
#include <map>
#include <memory>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _UCBHELPER_PROXYDECIDER_HXX
#include <ucbhelper/proxydecider.hxx>
#endif

#ifndef _DAVEXCEPTION_HXX_
#include "DAVException.hxx"
#endif

using namespace com::sun::star;

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
