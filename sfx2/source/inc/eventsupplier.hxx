/*************************************************************************
 *
 *  $RCSfile: eventsupplier.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dv $ $Date: 2001-02-21 09:53:27 $
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

#ifndef _SFX_EVENTSUPPLIER_HXX_
#define _SFX_EVENTSUPPLIER_HXX_

#ifndef  _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef  _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef  _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif

#ifndef  _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef  _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif

#ifndef  _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef  _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef  _SFX_SFXUNO_HXX
#include <sfxuno.hxx>
#endif

#include <vos/mutex.hxx>

//--------------------------------------------------------------------------------------------------------

#define NOSUCHELEMENTEXCEPTION      ::com::sun::star::container::NoSuchElementException
#define XNAMEREPLACE                ::com::sun::star::container::XNameReplace
#define DOCEVENTOBJECT              ::com::sun::star::document::EventObject
#define XEVENTBROADCASTER           ::com::sun::star::document::XEventBroadcaster
#define XDOCEVENTLISTENER           ::com::sun::star::document::XEventListener
#define EVENTOBJECT                 ::com::sun::star::lang::EventObject
#define ILLEGALARGUMENTEXCEPTION    ::com::sun::star::lang::IllegalArgumentException
#define WRAPPEDTARGETEXCEPTION      ::com::sun::star::lang::WrappedTargetException
#define ANY                         ::com::sun::star::uno::Any
#define REFERENCE                   ::com::sun::star::uno::Reference
#define RUNTIMEEXCEPTION            ::com::sun::star::uno::RuntimeException
#define SEQUENCE                    ::com::sun::star::uno::Sequence
#define UNOTYPE                     ::com::sun::star::uno::Type
#define OUSTRING                    ::rtl::OUString

//--------------------------------------------------------------------------------------------------------

class SfxObjectShell;
class SfxBaseModel;

//--------------------------------------------------------------------------------------------------------


class SfxEvents_Impl : public ::cppu::WeakImplHelper2< ::com::sun::star::container::XNameReplace, ::com::sun::star::document::XEventListener  >
{
    SEQUENCE< OUSTRING >            maEventNames;
    SEQUENCE< ANY >                 maEventData;
    REFERENCE< XEVENTBROADCASTER >  mxBroadcaster;
    ::osl::Mutex                    maMutex;
    SfxObjectShell                 *mpObjShell;

    SvxMacro*                   ConvertToMacro( ANY aElement );
    sal_Bool                    Warn_Impl();

public:
                                SfxEvents_Impl( SfxObjectShell* pShell,
                                                REFERENCE< XEVENTBROADCASTER > xBroadcaster );
                               ~SfxEvents_Impl();

    //  --- XNameReplace ---
    virtual void SAL_CALL       replaceByName( const OUSTRING & aName, const ANY & aElement )
                                    throw( ILLEGALARGUMENTEXCEPTION, NOSUCHELEMENTEXCEPTION,
                                           WRAPPEDTARGETEXCEPTION, RUNTIMEEXCEPTION );

    //  --- XNameAccess ( parent of XNameReplace ) ---
    virtual ANY SAL_CALL        getByName( const OUSTRING& aName )
                                    throw( NOSUCHELEMENTEXCEPTION, WRAPPEDTARGETEXCEPTION,
                                           RUNTIMEEXCEPTION );
    virtual SEQUENCE< OUSTRING > SAL_CALL getElementNames() throw ( RUNTIMEEXCEPTION );
    virtual sal_Bool SAL_CALL   hasByName( const OUSTRING& aName ) throw ( RUNTIMEEXCEPTION );

    //  --- XElementAccess ( parent of XNameAccess ) ---
    virtual UNOTYPE SAL_CALL    getElementType() throw ( RUNTIMEEXCEPTION );
    virtual sal_Bool SAL_CALL   hasElements() throw ( RUNTIMEEXCEPTION );

    // --- ::document::XEventListener ---
    virtual void SAL_CALL       notifyEvent( const DOCEVENTOBJECT& aEvent )
                                    throw( RUNTIMEEXCEPTION );

    // --- ::lang::XEventListener ---
    virtual void SAL_CALL       disposing( const EVENTOBJECT& Source )
                                    throw( RUNTIMEEXCEPTION );
};

#endif
