/*************************************************************************
 *
 *  $RCSfile: NeonSession.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2001-02-16 08:14:51 $
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

#include "DAVSession.hxx"
#include "NeonTypes.hxx"
#include <osl/mutex.hxx>
#include <vector>

namespace webdav_ucp
{

// -------------------------------------------------------------------
// NeonSession
// A DAVSession implementation using the neon/expat library
// -------------------------------------------------------------------
class NeonSession : public DAVSession
{
    private:
        osl::Mutex              mMutex;
        rtl::OString            mHostName;
        rtl::OString            mProxyName;
        sal_Int32               mPort;
        sal_Int32               mProxyPort;
        HttpSession *           mHttpSession;

        // Authentication stuff
        DAVAuthListener *       mListener;
        com::sun::star::uno::Reference<
         com::sun::star::ucb::XCommandEnvironment > mEnv;

        DAVSessionFactory *     m_pSessionFactory;

        // Note: Uncomment the following if locking support is required
        // NeonLockSession *    mNeonLockSession;

        static sal_Bool         sSockInited;

    public:
        NeonSession( DAVSessionFactory* psessionFactory,
                     const rtl::OUString& inUri,
                     const ProxyConfig& rProxyCfg );
        virtual ~NeonSession( );


        // DAVSession methods
        virtual sal_Bool CanUse( const ::rtl::OUString & inUri );

        virtual void setServerAuthListener(DAVAuthListener * inDAVAuthListener);
        virtual void setProxyAuthListener(DAVAuthListener * inDAVAuthListener);

        virtual void OPTIONS( const ::rtl::OUString &  inUri,
                              DAVCapabilities & outCapabilities,
                              const com::sun::star::uno::Reference<
                                com::sun::star::ucb::XCommandEnvironment >& inEnv )
                                 throw( DAVException );

        virtual void PROPFIND( const ::rtl::OUString &              inUri,
                                  const Depth                           inDepth,
                                  const std::vector< ::rtl::OUString >& inPropNames,
                                  std::vector< DAVResource > &          ioResources,
                               const com::sun::star::uno::Reference<
                                com::sun::star::ucb::XCommandEnvironment >& inEnv )
                                 throw( DAVException );

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
                        GET( const ::rtl::OUString & inUri,
                             const com::sun::star::uno::Reference<
                              com::sun::star::ucb::XCommandEnvironment >& inEnv )
                                throw( DAVException );

        virtual void GET( const ::rtl::OUString &               inUri,
                         com::sun::star::uno::Reference<
                          com::sun::star::io::XOutputStream > & ioOutputStream,
                         const com::sun::star::uno::Reference<
                         com::sun::star::ucb::XCommandEnvironment >& inEnv )
                                throw( DAVException );


        virtual void PUT( const ::rtl::OUString &               inUri,
                          com::sun::star::uno::Reference<
                         com::sun::star::io::XInputStream > &   inInputStream,
                        const com::sun::star::uno::Reference<
                         com::sun::star::ucb::XCommandEnvironment >& inEnv )
                                throw( DAVException );

        virtual void MKCOL( const ::rtl::OUString & inUri,
                            const com::sun::star::uno::Reference<
                             com::sun::star::ucb::XCommandEnvironment >& inEnv )
                              throw( DAVException );

        virtual void COPY( const ::rtl::OUString &  inSource,
                              const ::rtl::OUString &   inDestination,
                           const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment >& inEnv,
                              sal_Bool                  inOverWrite )
                                throw( DAVException );

        virtual void MOVE( const ::rtl::OUString &  inSource,
                              const ::rtl::OUString &   inDestination,
                           const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment >& inEnv,
                              sal_Bool                  inOverWrite )
                                throw( DAVException );

        virtual void DESTROY( const ::rtl::OUString & inUri,
                              const com::sun::star::uno::Reference<
                               com::sun::star::ucb::XCommandEnvironment >& inEnv )
                                throw( DAVException );

        // Note: Uncomment the following if locking support is required
        /*
        virtual void LOCK (const Lock & inLock,
                           const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment >& inEnv )
                             throw ( DAVException );

        virtual void UNLOCK (const Lock & inLock,
                             const com::sun::star::uno::Reference<
                              com::sun::star::ucb::XCommandEnvironment >& inEnv )
                               throw ( DAVException );
        */

    private:
        // Initialise "Neon sockets"
        void            Init( void );

        // Create a Neon session for server at supplied host & port
        HttpSession *   CreateSession( const ::rtl::OString & inHostName,
                                       int inPort,
                                        const ::rtl::OString & inProxyName,
                                       int inProxyPort );

        // A simple Neon http_block_reader for use with a http GET method
        // in conjunction with an XInputStream
        static void     GETReader( void *       inUserData,
                                   const char * inBuf,
                                   size_t       inLen );

        // A simple Neon http_block_reader for use with a http GET method
        // in conjunction with an XOutputStream
        static void     GETWriter( void *       inUserData,
                                   const char * inBuf,
                                   size_t       inLen );

        // Note: Uncomment the following if locking support is required
        // void         Lockit( const Lock & inLock, bool inLockit );

        static int      NeonAuth( void *        inUserData,
                                  const char *  inRealm,
                                  const char *  inHostName,
                                  char **       inUserName,
                                  char **       inPassWord );

        // Redirection callbacks.
        static int RedirectConfirm( void *       userdata,
                                     const char * src,
                                    const char * dest );
        static void RedirectNotify( void *       userdata,
                                     const char * src,
                                    const char * dest );
};

}; // namespace_ucp
#endif // _NEONSESSION_HXX_
