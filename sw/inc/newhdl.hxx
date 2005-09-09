/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newhdl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:03:03 $
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
#ifndef _NEWHDL_HXX
#define _NEWHDL_HXX

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif

#include "errhdl.hxx"       // hier kommt auch swtypes rein, was ich brauche

// ----------------------------------------------------------------------------

extern void SW_NEW_HDL();

class SwNewHdl
{
        friend void SW_NEW_HDL();
private:
        char*           pWarnMem;
        char*           pExceptMem;
        Link            aWarnLnk;
        Link            aErrLnk;
public:
        void    SetWarnLnk( const Link &rLink );
        void    SetErrLnk( const Link &rLink );

        BOOL    TryWarnMem();

        BOOL    Flush_WarnMem();
        BOOL    Flush_ExceptMem();

        SwNewHdl();
        ~SwNewHdl();
};

/******************************************************************************
 *  INLINE
 ******************************************************************************/
inline void SwNewHdl::SetWarnLnk( const Link &rLink )
{
    aWarnLnk = rLink;
}

inline void SwNewHdl::SetErrLnk( const Link &rLink )
{
    aErrLnk = rLink;
}

#endif
