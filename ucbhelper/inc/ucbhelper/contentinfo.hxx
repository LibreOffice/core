/*************************************************************************
 *
 *  $RCSfile: contentinfo.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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

#ifndef _UCBHELPER_CONTENTINFO_HXX
#define _UCBHELPER_CONTENTINFO_HXX

#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

namespace ucb {

//============================================================================
//
// class PropertySetInfo.
//
//============================================================================

class ContentImplHelper;

/**
  * This class provides a propertyset info ( the complete implementation of
  * the interface XPropertySetInfo ) for an object derived from class
  * ucb::ContentImplHelper. The implementation takes care about Additional
  * Core Properties that may have been added to the content.
  */
class PropertySetInfo :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::beans::XPropertySetInfo
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                m_xSMgr;
    com::sun::star::uno::Sequence< com::sun::star::beans::Property >*
                                m_pProps;
    vos::OMutex                 m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    sal_Bool queryProperty( const rtl::OUString& rName,
                            com::sun::star::beans::Property& rProp );

public:
    PropertySetInfo( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                     ContentImplHelper* pContent );
    virtual ~PropertySetInfo();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XPropertySetInfo
    virtual com::sun::star::uno::Sequence<
                com::sun::star::beans::Property > SAL_CALL
    getProperties()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::beans::Property SAL_CALL
    getPropertyByName( const rtl::OUString& aName )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasPropertyByName( const rtl::OUString& Name )
        throw( com::sun::star::uno::RuntimeException );

    // Non-Interface methods.
    void reset();
};

//============================================================================
//
// class CommandProcessorInfo.
//
//============================================================================

/**
  * This class provides a command info ( the complete implementation of
  * the interface XCommandInfo ) for an object derived from class
  * ucb::ContentImplHelper.
  */
class CommandProcessorInfo :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::ucb::XCommandInfo
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                m_xSMgr;
    com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >*
                                m_pCommands;
    vos::OMutex                 m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    sal_Bool queryCommand( const rtl::OUString& rName,
                           com::sun::star::ucb::CommandInfo& rCommand );
    sal_Bool queryCommand( sal_Int32 nHandle,
                           com::sun::star::ucb::CommandInfo& rCommand );

public:
    CommandProcessorInfo( const com::sun::star::uno::Reference<
                    com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                 ContentImplHelper* pContent );
    virtual ~CommandProcessorInfo();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XCommandInfo
    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::CommandInfo > SAL_CALL
    getCommands()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::ucb::CommandInfo SAL_CALL
    getCommandInfoByName( const rtl::OUString& Name )
        throw( com::sun::star::ucb::UnsupportedCommandException,
        com::sun::star::uno::RuntimeException );
    virtual com::sun::star::ucb::CommandInfo SAL_CALL
    getCommandInfoByHandle( sal_Int32 Handle )
        throw( com::sun::star::ucb::UnsupportedCommandException,
        com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasCommandByName( const rtl::OUString& Name )
        throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasCommandByHandle( sal_Int32 Handle )
        throw( com::sun::star::uno::RuntimeException );

    // Non-Interface methods.
    void reset();
};

} // namespace ucb

#endif /* !_UCBHELPER_CONTENTINFO_HXX */
