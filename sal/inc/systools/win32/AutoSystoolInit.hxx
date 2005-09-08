/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AutoSystoolInit.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:48:24 $
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

#ifndef _AUTOSYSTOOLINIT_HXX_
#define _AUTOSYSTOOLINIT_HXX_

#ifndef _WINDOWS_
#include <windows.h>
#endif

//------------------------------------------------------------------------
// class used to automatically initialize/deinitialize the systools
//------------------------------------------------------------------------

class OAutoSystoolInit
{
    typedef void ( WINAPI *LPFNINIT_T   )( );
    typedef void ( WINAPI *LPFNDEINIT_T )( );

public:
    OAutoSystoolInit( LPFNINIT_T lpfnInit, LPFNDEINIT_T lpfnDeInit ) :
        m_lpfnDeInit( lpfnDeInit )
    {
        if ( NULL != lpfnInit )
            lpfnInit( );
    }

    ~OAutoSystoolInit( )
    {
        if ( NULL != m_lpfnDeInit )
            m_lpfnDeInit( );
    }

private:
    LPFNDEINIT_T m_lpfnDeInit; // address of the deinit function
};

#endif
