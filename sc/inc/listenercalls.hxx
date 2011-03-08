/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SC_LISTENERCALLS_HXX
#define SC_LISTENERCALLS_HXX

#include <list>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/EventObject.hpp>

namespace com { namespace sun { namespace star {
    namespace util {
        class XModifyListener;
    }
    namespace lang {
        struct EventObject;
    }
} } }


struct ScUnoListenerEntry
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener >   xListener;
    ::com::sun::star::lang::EventObject             aEvent;

    ScUnoListenerEntry( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XModifyListener >& rL,
                        const ::com::sun::star::lang::EventObject& rE ) :
        xListener( rL ),
        aEvent( rE )
    {}
};


/** ScUnoListenerCalls stores notifications to XModifyListener that can't be processed
    during BroadcastUno and calls them together at the end.
*/
class ScUnoListenerCalls
{
private:
    ::std::list<ScUnoListenerEntry> aEntries;

public:
                ScUnoListenerCalls();
                ~ScUnoListenerCalls();

    void        Add( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XModifyListener >& rListener,
                        const ::com::sun::star::lang::EventObject& rEvent );
    void        ExecuteAndClear();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
