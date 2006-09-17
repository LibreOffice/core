/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lateinitthread.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:37:56 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "lateinitthread.hxx"

//_______________________________________________
// includes

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

/*-----------------------------------------------
    14.08.2003 09:31
-----------------------------------------------*/
LateInitThread::LateInitThread()
{
}

/*-----------------------------------------------
    14.08.2003 08:42
-----------------------------------------------*/
LateInitThread::~LateInitThread()
{
}

/*-----------------------------------------------
    28.10.2003 09:30
-----------------------------------------------*/
void SAL_CALL LateInitThread::run()
{
    // sal_True => It indicates using of this method by this thread
    // The filter cache use this information to show an assertion
    // for "optimization failure" in case the first calli of loadAll()
    // was not this thread ...

    // Further please dont catch any exception here.
    // May be they show the problem of a corrupted filter
    // configuration, which is handled inside our event loop or desktop.main()!

    ::salhelper::SingletonRef< FilterCache > rCache;
    rCache->load(FilterCache::E_CONTAINS_ALL, sal_True);
}

    } // namespace config
} // namespace filter
