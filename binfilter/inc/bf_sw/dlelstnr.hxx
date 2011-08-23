/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _DLELSTNR_HXX_
#define _DLELSTNR_HXX_


#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLISTEVENTLISTENER_HPP_
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XLINGUSERVICEEVENTLISTENER_HPP_
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>	// helper for implementations
#endif


namespace com { namespace sun { namespace star {
    namespace linguistic2 {
        class XDictionaryList;
        class XLinguServiceManager;
    }
    namespace frame {
        class XTerminateListener;
    }
} } }
namespace binfilter {

///////////////////////////////////////////////////////////////////////////
// SwLinguServiceEventListener
// is a EventListener that triggers spellchecking
// and hyphenation when relevant changes (to the
// dictionaries of the dictionary list, or properties) were made.
//

class SwLinguServiceEventListener : 
    public cppu::WeakImplHelper2
    <
        ::com::sun::star::linguistic2::XLinguServiceEventListener,
        ::com::sun::star::frame::XTerminateListener
    >
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDesktop >					xDesktop;
    ::com::sun::star::uno::Reference< 
        ::com::sun::star::linguistic2::XLinguServiceManager >	xLngSvcMgr;

    // disallow use of copy-constructor and assignment operator
    SwLinguServiceEventListener(const SwLinguServiceEventListener &);
    SwLinguServiceEventListener & operator = (const SwLinguServiceEventListener &);

public:
    SwLinguServiceEventListener();
    virtual ~SwLinguServiceEventListener();

    // XEventListener
    virtual void SAL_CALL disposing( 
            const ::com::sun::star::lang::EventObject& rEventObj ) 
        throw(::com::sun::star::uno::RuntimeException);

    // XDictionaryListEventListener
    virtual void 	SAL_CALL processDictionaryListEvent(
            const ::com::sun::star::linguistic2::DictionaryListEvent& rDicListEvent) 
        throw( ::com::sun::star::uno::RuntimeException );

    // XLinguServiceEventListener
    virtual void SAL_CALL processLinguServiceEvent( 
            const ::com::sun::star::linguistic2::LinguServiceEvent& rLngSvcEvent ) 
        throw(::com::sun::star::uno::RuntimeException);

    // XTerminateListener
    virtual void SAL_CALL queryTermination( 
            const ::com::sun::star::lang::EventObject& rEventObj ) 
        throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( 
            const ::com::sun::star::lang::EventObject& rEventObj ) 
        throw(::com::sun::star::uno::RuntimeException);
};


} //namespace binfilter
#endif

