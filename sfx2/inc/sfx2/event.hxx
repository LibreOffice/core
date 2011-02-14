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
#ifndef _SFXEVENT_HXX
#define _SFXEVENT_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <tools/string.hxx>
#include <svl/hint.hxx>
#include <unotools/eventcfg.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XController2.hpp>

class SfxObjectShell;

//-------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxEventHint : public SfxHint
{
    SfxObjectShell*     pObjShell;
    ::rtl::OUString     aEventName;
    sal_uInt16              nEventId;

public:
    TYPEINFO();
    SfxEventHint( sal_uInt16 nId, const ::rtl::OUString& aName, SfxObjectShell *pObj = 0 )
                        :   pObjShell(pObj),
                            aEventName(aName),
                            nEventId(nId)
                        {}

    sal_uInt16              GetEventId() const
                        { return nEventId; }

    ::rtl::OUString     GetEventName() const
                        { return aEventName; }

    SfxObjectShell*     GetObjShell() const
                        { return pObjShell; }
};

//-------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxViewEventHint : public SfxEventHint
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 > xViewController;

public:
    TYPEINFO();

    SfxViewEventHint( sal_uInt16 nId, const ::rtl::OUString& aName, SfxObjectShell *pObj, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController )
                        : SfxEventHint( nId, aName, pObj )
                        , xViewController( xController, ::com::sun::star::uno::UNO_QUERY )
                        {}

    SfxViewEventHint( sal_uInt16 nId, const ::rtl::OUString& aName, SfxObjectShell *pObj, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 >& xController )
                        : SfxEventHint( nId, aName, pObj )
                        , xViewController( xController )
                        {}

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 > GetController() const
                        { return xViewController; }
};

//-------------------------------------------------------------------

class SfxNamedHint : public SfxHint
{
    String              _aEventName;
    SfxObjectShell*     _pObjShell;
    String              _aArgs;

public:
                        TYPEINFO();

                        SfxNamedHint( const String& rName,
                                      const String& rArgs,
                                      SfxObjectShell *pObj = 0  )
                        :   _aEventName( rName ),
                            _pObjShell( pObj),
                            _aArgs( rArgs )
                        {}

                        SfxNamedHint( const String& rName,
                                      SfxObjectShell *pObj = 0 )
                        :   _aEventName( rName ),
                            _pObjShell( pObj )
                        {}

    const String&       GetArgs() const { return _aArgs;}
    const String&       GetName() const { return _aEventName; }
    SfxObjectShell*     GetObjShell() const { return _pObjShell; }
};

class Printer;
class SfxPrintingHint : public SfxHint
{
    sal_Int32           nWhich;
    com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aOpts;
public:
                        TYPEINFO();
                        SfxPrintingHint( sal_Int32 nEvent, const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& rOpts )
                            : nWhich( nEvent )
                            , aOpts( rOpts )
                        {}

                        SfxPrintingHint( sal_Int32 nEvent )
                            : nWhich( nEvent )
                        {}

    sal_Int32           GetWhich() const { return nWhich; }
    const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& GetOptions() { return aOpts; }
};

#endif
