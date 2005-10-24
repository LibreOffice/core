/*************************************************************************
 *
 *  $RCSfile: SlsCacheConfiguration.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:40:37 $
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

#ifndef SD_SLIDESORTER_CACHE_CONFIGURATION_HXX
#define SD_SLIDESORTER_CACHE_CONFIGURATION_HXX

#include <com/sun/star/uno/Any.hxx>
#include <vcl/timer.hxx>
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace sd { namespace slidesorter { namespace cache {

/** A very simple and easy-to-use access to configuration entries regarding
    the slide sorter cache.
*/
class CacheConfiguration
{
public:
    /** Return an instance to this class.  The reference is released after 5
        seconds.  Subsequent calls to this function will create a new
        instance.
    */
    static ::boost::shared_ptr<CacheConfiguration> Instance (void);

    /** Look up the specified value in
        MultiPaneGUI/SlideSorter/PreviewCache.   When the specified value
        does not exist then an empty Any is returned.
    */
    ::com::sun::star::uno::Any GetValue (const ::rtl::OUString& rName);

private:
    static ::boost::shared_ptr<CacheConfiguration> mpInstance;
    /** When a caller holds a reference after we have released ours we use
        this weak pointer to avoid creating a new instance.
    */
    static ::boost::weak_ptr<CacheConfiguration> mpWeakInstance;
    static Timer maReleaseTimer;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess> mxCacheNode;

    CacheConfiguration (void);

    DECL_LINK(TimerCallback, Timer*);
};

} } } // end of namespace ::sd::slidesorter::cache

#endif
