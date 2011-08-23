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
