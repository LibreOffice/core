/*************************************************************************
 *
 *  $RCSfile: DAVResourceAccess.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-29 09:00:12 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DAVRESOURCEACCESS_HXX_
#define _DAVRESOURCEACCESS_HXX_

#include <vector>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_LOCK_HPP_
#include <com/sun/star/ucb/Lock.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif

#ifndef _DAVAUTHLISTENER_HXX_
#include "DAVAuthListener.hxx"
#endif
#ifndef _DAVEXCEPTION_HXX_
#include "DAVException.hxx"
#endif
#ifndef _DAVSESSION_HXX_
#include "DAVSession.hxx"
#endif
#ifndef _DAVRESOURCE_HXX_
#include "DAVResource.hxx"
#endif
#ifndef _DAVTYPES_HXX_
#include "DAVTypes.hxx"
#endif
#ifndef _NEONURI_HXX_
#include "NeonUri.hxx"
#endif

namespace webdav_ucp
{

class DAVSessionFactory;

class DAVResourceAccess
{
    osl::Mutex    m_aMutex;
    rtl::OUString m_aURL;
    rtl::OUString m_aPath;
    rtl::Reference< DAVSession > m_xSession;
    rtl::Reference< DAVSessionFactory > m_xSessionFactory;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    std::vector< NeonUri > m_aRedirectURIs;

public:
    DAVResourceAccess() : m_xSessionFactory( 0 ) {}
    DAVResourceAccess( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory > & rSMgr,
                       rtl::Reference<
                        DAVSessionFactory > const & rSessionFactory,
                       const rtl::OUString & rURL )
        throw( DAVException );

    void setURL( const rtl::OUString & rNewURL )
        throw( DAVException );

    const rtl::OUString & getURL() const { return m_aURL; }

    rtl::Reference< DAVSessionFactory > getSessionFactory() const
    { return m_xSessionFactory; }

    // DAV methods
    //

    void
    OPTIONS(  DAVCapabilities & rCapabilities,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    // allprop & named
    void
    PROPFIND( const Depth nDepth,
              const std::vector< rtl::OUString > & rPropertyNames,
              std::vector< DAVResource > & rResources,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    // propnames
    void
    PROPFIND( const Depth nDepth,
              std::vector< DAVResourceInfo > & rResInfo,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    void
    PROPPATCH( const std::vector< ProppatchValue > & rValues,
               const com::sun::star::uno::Reference<
                   com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( DAVException );

    void
    HEAD( const std::vector< rtl::OUString > & rHeaderNames, // empty == 'all'
          std::vector< DAVResource > & rResources,
          const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( DAVException );

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET(      const com::sun::star::uno::Reference<
                   com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    void
    GET(      com::sun::star::uno::Reference<
                com::sun::star::io::XOutputStream > & rStream,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    void
    PUT(      const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & rStream,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    POST(     const rtl::OUString & rContentType,
                const rtl::OUString & rReferer,
              const com::sun::star::uno::Reference<
                com::sun::star::io::XInputStream > & rInputStream,
              const com::sun::star::uno::Reference<
                com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    void
    POST(     const rtl::OUString & rContentType,
              const rtl::OUString & rReferer,
              const com::sun::star::uno::Reference<
                com::sun::star::io::XInputStream > & rInputStream,
              com::sun::star::uno::Reference<
                com::sun::star::io::XOutputStream > & rOutputStream,
              const com::sun::star::uno::Reference<
                com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    void
    MKCOL(    const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    void
    COPY(     const ::rtl::OUString & rSourcePath,
                 const ::rtl::OUString & rDestinationURI,
              sal_Bool bOverwrite,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    void
    MOVE(     const ::rtl::OUString & rSourcePath,
              const ::rtl::OUString & rDestinationURI,
              sal_Bool bOverwrite,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    void
    DESTROY(  const com::sun::star::uno::Reference<
                   com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    void
    LOCK (    const com::sun::star::ucb::Lock & rLock,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

    void
    UNLOCK (  const com::sun::star::ucb::Lock & rLock,
              const com::sun::star::uno::Reference<
                   com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

private:
    sal_Bool detectRedirectCycle( const rtl::OUString& rRedirectURL );
    sal_Bool handleException( DAVException & e );
    void initialize()
        throw ( DAVException );
};

}; // namespace webdav_ucp

#endif // _DAVRESOURCEACCESS_HXX_
