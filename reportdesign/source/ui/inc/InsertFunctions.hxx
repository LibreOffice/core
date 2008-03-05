/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InsertFunctions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:08:49 $
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
#ifndef RPT_INSERTFUNCTIONS_HXX_INCLUDED
#define RPT_INSERTFUNCTIONS_HXX_INCLUDED

#ifndef _EVENT_HXX //autogen
#include <vcl/event.hxx>
#endif
#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

class SdrView;
class Window;
class SdrModel;
class SdrObject;
namespace rptui
{
/************************************************************************/
    void InitializeChart( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& _xModel,
                          const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xObj);
// ---------
} // namespace rptui
// ---------
#endif      // RPT_INSERTFUNCTIONS_HXX_INCLUDED

