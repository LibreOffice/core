/*************************************************************************
 *
 *  $RCSfile: SlsRequestPriorityClass.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 10:48:39 $
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

#ifndef SD_SLIDESORTER_CACHE_REQUEST_PRIORITY_CLASS_HXX
#define SD_SLIDESORTER_CACHE_REQUEST_PRIORITY_CLASS_HXX

namespace sd { namespace slidesorter { namespace cache {


/** Each request for a preview creation has a priority.  This enum defines
    the available priorities.  The special values MIN__CLASS and MAX__CLASS
    are/can be used for validation and have to be kept up-to-date.
*/
enum RequestPriorityClass
{
    MIN__CLASS = 0,

    // The slide is visible.  A preview does not yet exist.
    VISIBLE_NO_PREVIEW = MIN__CLASS,
    // The slide is visible.  A preview exists but is not up-to-date anymore.
    VISIBLE_OUTDATED_PREVIEW,
    // The slide is not visible.
    NOT_VISIBLE,

    MAX__CLASS = NOT_VISIBLE
};


} } } // end of namespace ::sd::slidesorter::cache

#endif
