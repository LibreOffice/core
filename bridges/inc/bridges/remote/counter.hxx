/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: counter.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _BRIDGES_REMOTE_COUNTER_H_
#define _BRIDGES_REMOTE_COUNTER_H_
#include <stdio.h>
#if OSL_DEBUG_LEVEL > 1
struct MyCounter
{
    MyCounter( sal_Char const *pName ) :
        m_nCounter( 0 ),
        m_pName ( pName )
        {
        }
    ~MyCounter()
        {
            if( m_nCounter ) {
                printf(
                    "%s : %ld left\n", m_pName,
                    sal::static_int_cast< long >(m_nCounter) );
            }
        }
    void acquire()
        { m_nCounter ++; }
    void release()
        { m_nCounter --; }


    sal_Int32 m_nCounter;
    sal_Char const *m_pName;
};
#endif

#endif
