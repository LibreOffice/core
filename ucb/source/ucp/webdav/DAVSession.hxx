/*************************************************************************
 *
 *  $RCSfile: DAVSession.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-11-13 15:20:30 $
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

#include "DAVAuthListener.hxx"
#include "DAVException.hxx"
#include "DAVProperties.hxx"
#include "DAVResource.hxx"
#include "DAVSessionFactory.hxx"
#include "DAVTypes.hxx"
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/XReference.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace webdav_ucp
{

class DAVSession : public ::cppu::OWeakObject
{
public:
    virtual sal_Bool CanUse( const ::rtl::OUString & inUri ) = 0;

    // Authentication methods
    //
    virtual void setServerAuthListener(DAVAuthListener * inDAVAuthListener) = 0;
    virtual void setProxyAuthListener(DAVAuthListener * inDAVAuthListener ) = 0;


    // DAV methods
    //

    virtual void OPTIONS( const ::rtl::OUString &  inUri,
                          DAVCapabilities & outCapabilities,
                          const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment >& inEnv ) = 0;

    virtual void PROPFIND( const ::rtl::OUString &          inUri,
                    const Depth                             inDepth,
                    const std::vector< ::rtl::OUString > &  inPropertyNames,
                    std::vector< DAVResource > &            inResources,
                    const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XCommandEnvironment >& inEnv ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
                    GET( const ::rtl::OUString & inUri,
                         const com::sun::star::uno::Reference<
                          com::sun::star::ucb::XCommandEnvironment >& inEnv ) = 0;

    virtual void    GET( const ::rtl::OUString & inUri,
        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& o,
        const com::sun::star::uno::Reference<
         com::sun::star::ucb::XCommandEnvironment >& inEnv ) = 0;

    virtual void    PUT( const ::rtl::OUString & inUri,
        com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& s,
        const com::sun::star::uno::Reference<
         com::sun::star::ucb::XCommandEnvironment >& inEnv ) = 0;

    virtual void    MKCOL( const ::rtl::OUString & inUri,
                           const com::sun::star::uno::Reference<
                           com::sun::star::ucb::XCommandEnvironment >& inEnv ) = 0;

    virtual void    COPY( const ::rtl::OUString & inSource,
                          const ::rtl::OUString & inDestination,
                          const com::sun::star::uno::Reference<
                          com::sun::star::ucb::XCommandEnvironment >& inEnv,
                          sal_Bool inOverwrite = false ) = 0;

    virtual void    MOVE( const ::rtl::OUString & inSource,
                          const ::rtl::OUString & inDestination,
                          const com::sun::star::uno::Reference<
                          com::sun::star::ucb::XCommandEnvironment >& inEnv,
                          sal_Bool inOverwrite = false ) = 0;

    virtual void    DESTROY( const ::rtl::OUString & inUri,
                             const com::sun::star::uno::Reference<
                             com::sun::star::ucb::XCommandEnvironment >& inEnv ) = 0;

    // Note: Uncomment the following if locking support is required
    /*
    virtual void LOCK ( const Lock & inLock,
                          const com::sun::star::uno::Reference<
                          com::sun::star::ucb::XCommandEnvironment >& inEnv ) = 0;

    virtual void UNLOCK ( const Lock & inLock,
                            const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment >& inEnv) = 0;
    */
};

}; // namespace webdav_ucp

#endif // _DAVSESSION_HXX_

