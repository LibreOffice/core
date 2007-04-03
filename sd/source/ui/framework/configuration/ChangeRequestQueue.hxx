/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChangeRequestQueue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:43:47 $
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

#ifndef SD_FRAMEWORK_CHANGE_OPERATION_QUEUE_HXX
#define SD_FRAMEWORK_CHANGE_OPERATION_QUEUE_HXX

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCHANGEREQUEST_HPP_
#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#endif

#include <list>

namespace sd { namespace framework {


/** The ChangeRequestQueue stores the pending requests for changes to the
    requested configuration.  It is the task of the
    ChangeRequestQueueProcessor to process these requests.
*/
class ChangeRequestQueue
    : public ::std::list<com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfigurationChangeRequest> >
{
public:
    /** Create an empty queue.
    */
    ChangeRequestQueue (void);
};


} } // end of namespace sd::framework

#endif
