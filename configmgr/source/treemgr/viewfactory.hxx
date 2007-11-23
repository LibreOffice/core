/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewfactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:49:21 $
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

#ifndef CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_
#define CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace data { class TreeSegment; }

    namespace view
    {
    // Different standard (static) strategies
    //---------------------------------------------------------------------
        typedef rtl::Reference<ViewStrategy> ViewStrategyRef;

        /// provides a factory for read-only node implementations
        ViewStrategyRef createReadOnlyStrategy();
        /// provides a factory for nodes that cache changes temporarily
        ViewStrategyRef createDeferredChangeStrategy();
        /// provides a factory for immediately commiting node implementations
        ViewStrategyRef createDirectAccessStrategy(data::TreeSegment const & _aTreeSegment);
    //---------------------------------------------------------------------
    }

//-----------------------------------------------------------------------------

}

#endif // CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_
