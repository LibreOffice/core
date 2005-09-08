/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listenercalls.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:44:04 $
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

#ifndef SC_LISTENERCALLS_HXX
#define SC_LISTENERCALLS_HXX

#include <list>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif

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

