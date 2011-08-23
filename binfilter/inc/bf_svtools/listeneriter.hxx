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
#ifndef _SVT_LISTENERITER_HXX
#define _SVT_LISTENERITER_HXX

#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif

namespace binfilter
{

class SvtListener;
class SvtListenerBase;
class SvtBroadcaster;

//-------------------------------------------------------------------------

class  SvtListenerIter
{
    friend class SvtListenerBase;

    SvtBroadcaster& rRoot;
    SvtListenerBase *pAkt, *pDelNext;

    // for the update of all iterator's, if a listener is added or removed
    // at the same time.
    static SvtListenerIter *pListenerIters;
    SvtListenerIter *pNxtIter;
    TypeId aSrchId;				// fuer First/Next - suche diesen Type

     static void RemoveListener( SvtListenerBase& rDel,
                                               SvtListenerBase* pNext );

public:
    SvtListenerIter( SvtBroadcaster& );
    ~SvtListenerIter();

    const SvtBroadcaster& GetBroadcaster() const	{ return rRoot; }
          SvtBroadcaster& GetBroadcaster() 			{ return rRoot; }

    SvtListener* GoNext();			// to the next

    SvtListener* GoStart(); 		// to the start of the list

    int IsChanged() const 		{ return pDelNext != pAkt; }
};

}

#endif

