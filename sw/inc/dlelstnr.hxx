/*************************************************************************
 *
 *  $RCSfile: dlelstnr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tl $ $Date: 2001-02-27 14:53:05 $
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

#ifndef _DLELSTNR_HXX_
#define _DLELSTNR_HXX_

#include <cppuhelper/weak.hxx>

#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLISTEVENTLISTENER_HPP_
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XLINGUSERVICEEVENTLISTENER_HPP_
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx> // helper for implementations
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

///////////////////////////////////////////////////////////////////////////
// SwLinguServiceEventListener
// is a EventListener that triggers spellchecking
// and hyphenation when relevant changes (to the
// dictionaries of the dictionary list, or properties) were made.
//

class SwLinguServiceEventListener :
    public cppu::WeakImplHelper2
    <
        com::sun::star::linguistic2::XLinguServiceEventListener,
        com::sun::star::frame::XTerminateListener
    >
{
    com::sun::star::uno::Reference<
        com::sun::star::frame::XDesktop >                   xDesktop;
    com::sun::star::uno::Reference<
        com::sun::star::linguistic2::XLinguServiceManager > xLngSvcMgr;

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
    virtual void    SAL_CALL processDictionaryListEvent(
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


#endif

