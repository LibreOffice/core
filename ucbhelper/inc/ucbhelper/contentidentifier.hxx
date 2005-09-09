/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contentidentifier.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:27:32 $
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

#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#define _UCBHELPER_CONTENTIDENTIFIER_HXX

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIER_HPP_
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#endif

#ifndef INCLUDED_UCBHELPERDLLAPI_H
#include "ucbhelper/ucbhelperdllapi.h"
#endif

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace rtl {
    class OUString;
}

namespace ucb
{

struct ContentIdentifier_Impl;

//=========================================================================

/**
  * This class implements a simple identifier object for UCB contents.
  * It mainly stores and returns the URL as it was passed to the constructor -
  * The only difference is that the URL scheme will be lower cased. This can
  * be done, because URL schemes are never case sensitive.
  */
class UCBHELPER_DLLPUBLIC ContentIdentifier :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                  public com::sun::star::ucb::XContentIdentifier
{
public:
    ContentIdentifier( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                       const rtl::OUString& rURL );
    ContentIdentifier( const rtl::OUString& rURL );
    virtual ~ContentIdentifier();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    acquire() throw();
    virtual void SAL_CALL
    release() throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );

    // XContentIdentifier
    virtual rtl::OUString SAL_CALL
    getContentIdentifier()
        throw( com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL
    getContentProviderScheme()
        throw( com::sun::star::uno::RuntimeException );

private:
    ContentIdentifier_Impl* m_pImpl;
};

} /* namespace ucb */

#endif /* !_UCBHELPER_CONTENTIDENTIFIER_HXX */
