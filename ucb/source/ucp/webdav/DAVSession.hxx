/*************************************************************************
 *
 *  $RCSfile: DAVSession.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-25 11:39:28 $
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

#ifndef _DAVSESSION_HXX_
#define _DAVSESSION_HXX_

#include <memory>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _DAVEXCEPTION_HXX_
#include "DAVException.hxx"
#endif
#ifndef _DAVPROPERTIES_HXX_
#include "DAVProperties.hxx"
#endif
#ifndef _DAVRESOURCE_HXX_
#include "DAVResource.hxx"
#endif
#ifndef _DAVSESSIONFACTORY_HXX_
#include "DAVSessionFactory.hxx"
#endif
#ifndef _DAVTYPES_HXX_
#include "DAVTypes.hxx"
#endif
#ifndef _DAVREQUESTENVIRONMENT_HXX_
#include "DAVRequestEnvironment.hxx"
#endif



namespace webdav_ucp
{

class DAVAuthListener;

class DAVSession
{
public:
    inline void acquire() SAL_THROW(())
    {
        osl_incrementInterlockedCount( &m_nRefCount );
    }

    void release() SAL_THROW(())
    {
        if ( osl_decrementInterlockedCount( &m_nRefCount ) == 0 )
        {
            m_xFactory->releaseElement( this );
            delete this;
        }
    }

    virtual sal_Bool CanUse( const ::rtl::OUString & inPath ) = 0;

    virtual sal_Bool UsesProxy() = 0;

    // DAV methods
    //

    virtual void OPTIONS( const ::rtl::OUString &  inPath,
                          DAVCapabilities & outCapabilities,
                          const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    // allprop & named
    virtual void PROPFIND( const ::rtl::OUString &                inPath,
                           const Depth                            inDepth,
                           const std::vector< ::rtl::OUString > & inPropertyNames,
                           std::vector< DAVResource > &           ioResources,
                           const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    // propnames
    virtual void PROPFIND( const ::rtl::OUString &          inPath,
                           const Depth                      inDepth,
                           std::vector< DAVResourceInfo > & ioResInfo,
                           const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void PROPPATCH( const ::rtl::OUString &               inPath,
                             const std::vector< ProppatchValue > & inValues,
                            const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void HEAD( const ::rtl::OUString &  inPath,
                       const std::vector< ::rtl::OUString > & inHeaderNames,
                       DAVResource & ioResource,
                       const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
                    GET( const ::rtl::OUString & inPath,
                         const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void GET( void* userData,
                      const ::rtl::OUString & inPath,
                      const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void    GET( const ::rtl::OUString & inPath,
        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& o,
        const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
                    GET( const ::rtl::OUString & inPath,
                         const std::vector< ::rtl::OUString > & inHeaderNames,
                         DAVResource & ioResource,
                         const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void GET( void* userData,
                      const ::rtl::OUString & inPath,
                      const std::vector< ::rtl::OUString > & inHeaderNames,
                      DAVResource & ioResource,
                      const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void    GET( const ::rtl::OUString & inPath,
        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& o,
        const std::vector< ::rtl::OUString > & inHeaderNames,
        DAVResource & ioResource,
        const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void    PUT( const ::rtl::OUString & inPath,
        const com::sun::star::uno::Reference<
            com::sun::star::io::XInputStream >& s,
        const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
                    POST( const rtl::OUString & inPath,
                          const rtl::OUString & rContentType,
                          const rtl::OUString & rReferer,
                          const com::sun::star::uno::Reference<
                           com::sun::star::io::XInputStream > & inInputStream,
                          const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    virtual void POST( const rtl::OUString & inPath,
                       const rtl::OUString & rContentType,
                       const rtl::OUString & rReferer,
                       const com::sun::star::uno::Reference<
                        com::sun::star::io::XInputStream > & inInputStream,
                       com::sun::star::uno::Reference<
                        com::sun::star::io::XOutputStream > & oOutputStream,
                       const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    virtual void    MKCOL( const ::rtl::OUString & inPath,
                           const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void    COPY( const ::rtl::OUString & inSource,
                          const ::rtl::OUString & inDestination,
                          const DAVRequestEnvironment & rEnv,
                          sal_Bool inOverwrite = false )
        throw( DAVException ) = 0;

    virtual void    MOVE( const ::rtl::OUString & inSource,
                          const ::rtl::OUString & inDestination,
                          const DAVRequestEnvironment & rEnv,
                          sal_Bool inOverwrite = false )
        throw( DAVException ) = 0;

    virtual void    DESTROY( const ::rtl::OUString & inPath,
                             const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    // Note: Uncomment the following if locking support is required
    /*
    virtual void LOCK ( const Lock & inLock,
                        const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void UNLOCK ( const Lock & inLock,
                          const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;
    */
protected:
    rtl::Reference< DAVSessionFactory > m_xFactory;

    DAVSession( rtl::Reference< DAVSessionFactory > const & rFactory )
    : m_xFactory( rFactory ), m_nRefCount( 0 ) {}

    virtual ~DAVSession() {}

private:
    DAVSessionFactory::Map::iterator m_aContainerIt;
    oslInterlockedCount m_nRefCount;

    friend class DAVSessionFactory;
#if defined WNT
    friend struct std::auto_ptr< DAVSession >;
    // work around compiler bug...
#else // WNT
    friend class std::auto_ptr< DAVSession >;
#endif // WNT
};

}; // namespace webdav_ucp

#endif // _DAVSESSION_HXX_

