/*************************************************************************
 *
 *  $RCSfile: NeonSession.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-25 11:39:46 $
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

#ifndef _NEONSESSION_HXX_
#define _NEONSESSION_HXX_

#include <vector>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _DAVSESSION_HXX_
#include "DAVSession.hxx"
#endif
#ifndef _NEONTYPES_HXX_
#include "NeonTypes.hxx"
#endif

namespace webdav_ucp
{

// -------------------------------------------------------------------
// NeonSession
// A DAVSession implementation using the neon/expat library
// -------------------------------------------------------------------

class NeonSession : public DAVSession
{
    private:
        osl::Mutex        m_aMutex;
        rtl::OUString     m_aScheme;
        rtl::OUString     m_aHostName;
        rtl::OUString     m_aProxyName;
        sal_Int32         m_nPort;
        sal_Int32         m_nProxyPort;
        HttpSession *     m_pHttpSession;
        void *            m_pRequestData;

        // @@@ This should really be per-request data. A NeonSession
        // instance can handle multiple requests at a time!!! But Neon
        // currently has no better interface!
        DAVRequestEnvironment m_aEnv;

        // Note: Uncomment the following if locking support is required
        // NeonLockSession *      mNeonLockSession;

        static sal_Bool   m_bSockInited;

    protected:
        virtual ~NeonSession();

    public:
        NeonSession( rtl::Reference< DAVSessionFactory > const & rSessionFactory,
                     const rtl::OUString& inUri,
                     const ucbhelper::InternetProxyServer & rProxyCfg )
            throw ( DAVException );

        // DAVSession methods
        virtual sal_Bool CanUse( const ::rtl::OUString & inUri );

        virtual sal_Bool UsesProxy();

        const DAVRequestEnvironment & getRequestEnvironment() const
        { return m_aEnv; }

        virtual void
        OPTIONS( const ::rtl::OUString &  inPath,
                 DAVCapabilities & outCapabilities,
                 const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        // allprop & named
        virtual void
        PROPFIND( const ::rtl::OUString & inPath,
                  const Depth inDepth,
                  const std::vector< ::rtl::OUString > & inPropNames,
                  std::vector< DAVResource > & ioResources,
                  const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        // propnames
        virtual void
        PROPFIND( const ::rtl::OUString & inPath,
                  const Depth inDepth,
                  std::vector< DAVResourceInfo >& ioResInfo,
                  const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual void
        PROPPATCH( const ::rtl::OUString & inPath,
                   const std::vector< ProppatchValue > & inValues,
                   const DAVRequestEnvironment & rEnv )
        throw( DAVException );

        virtual void
        HEAD( const ::rtl::OUString &  inPath,
              const std::vector< ::rtl::OUString > & inHeaderNames,
              DAVResource & ioResource,
              const DAVRequestEnvironment & rEnv )
            throw( DAVException );

        virtual com::sun::star::uno::Reference<
            com::sun::star::io::XInputStream >
        GET( const ::rtl::OUString & inPath,
             const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual void
        GET( void* userData,
             const ::rtl::OUString & inPath,
             const DAVRequestEnvironment & rEnv )
            throw( DAVException );

        virtual void
        GET( const ::rtl::OUString & inPath,
             com::sun::star::uno::Reference<
                com::sun::star::io::XOutputStream > &  ioOutputStream,
             const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual com::sun::star::uno::Reference<
            com::sun::star::io::XInputStream >
        GET( const ::rtl::OUString & inPath,
             const std::vector< ::rtl::OUString > & inHeaderNames,
             DAVResource & ioResource,
             const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual void
        GET( void* userData,
             const ::rtl::OUString & inPath,
             const std::vector< ::rtl::OUString > & inHeaderNames,
             DAVResource & ioResource,
             const DAVRequestEnvironment & rEnv )
            throw( DAVException );

        virtual void
        GET( const ::rtl::OUString & inPath,
             com::sun::star::uno::Reference<
                com::sun::star::io::XOutputStream > & ioOutputStream,
             const std::vector< ::rtl::OUString > & inHeaderNames,
             DAVResource & ioResource,
             const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual void
        PUT( const ::rtl::OUString & inPath,
             const com::sun::star::uno::Reference<
                com::sun::star::io::XInputStream > & inInputStream,
                const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual com::sun::star::uno::Reference<
            com::sun::star::io::XInputStream >
        POST( const rtl::OUString & inPath,
              const rtl::OUString & rContentType,
              const rtl::OUString & rReferer,
              const com::sun::star::uno::Reference<
                com::sun::star::io::XInputStream > & inInputStream,
              const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual void
        POST( const rtl::OUString & inPath,
              const rtl::OUString & rContentType,
              const rtl::OUString & rReferer,
              const com::sun::star::uno::Reference<
                com::sun::star::io::XInputStream > & inInputStream,
              com::sun::star::uno::Reference<
                com::sun::star::io::XOutputStream > & oOutputStream,
              const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual void
        MKCOL( const ::rtl::OUString & inPath,
               const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual void
        COPY( const ::rtl::OUString & inSourceURL,
              const ::rtl::OUString & inDestinationURL,
              const DAVRequestEnvironment & rEnv,
              sal_Bool inOverWrite )
            throw ( DAVException );

        virtual void
        MOVE( const ::rtl::OUString & inSourceURL,
              const ::rtl::OUString & inDestinationURL,
              const DAVRequestEnvironment & rEnv,
              sal_Bool inOverWrite )
            throw ( DAVException );

        virtual void DESTROY( const ::rtl::OUString & inPath,
                              const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        // Note: Uncomment the following if locking support is required
        /*
        virtual void LOCK (const Lock & inLock,
                           const DAVRequestEnvironment & rEnv )
            throw ( DAVException );

        virtual void UNLOCK (const Lock & inLock,
                             const DAVRequestEnvironment & rEnv )
            throw ( DAVException );
        */

        // helpers
        const rtl::OUString & getHostName() const { return m_aHostName; }

        const void * getRequestData() const { return m_pRequestData; }

    private:
        // Initialise "Neon sockets"
        void Init( void )
            throw ( DAVException );

        void HandleError( int nError )
            throw ( DAVException );

        // Note: Uncomment the following if locking support is required
        // void         Lockit( const Lock & inLock, bool inLockit )
        //  throw ( DAVException );

        // low level GET implementation, used by public GET implementations
        static int GET( ne_session * sess,
                        const char * uri,
                        ne_block_reader reader,
                        ne_header_handler handler,
                        void * userdata );

        // Buffer-based PUT implementation. Neon only has file descriptor-
        // based API.
        static int PUT( ne_session * sess,
                        const char * uri,
                        const char * buffer,
                        size_t size );

        // Buffer-based POST implementation. Neon only has file descriptor-
        // based API.
        int POST( ne_session * sess,
                  const char * uri,
                  const char * buffer,
                  ne_block_reader reader,
                  void * userdata,
                  const rtl::OUString & rContentType,
                  const rtl::OUString & rReferer );

        // Helper: XInputStream -> Sequence< sal_Int8 >
        static bool getDataFromInputStream(
                            const com::sun::star::uno::Reference<
                                com::sun::star::io::XInputStream > & xStream,
                            com::sun::star::uno::Sequence< sal_Int8 > & rData );
};

}; // namespace_ucp

#endif // _NEONSESSION_HXX_
