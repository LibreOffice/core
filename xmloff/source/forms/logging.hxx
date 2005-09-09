/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: logging.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:14:08 $
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

#ifndef XMLOFF_FORMS_LOGGING_HXX
#define XMLOFF_FORMS_LOGGING_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#include <stack>

namespace rtl { class Logfile; }

//.........................................................................
namespace xmloff
{
//.........................................................................

#ifdef TIMELOG

    //=====================================================================
    //= OStackedLogging
    //=====================================================================
    class OStackedLogging
    {
    private:
        ::std::stack< ::rtl::Logfile* >     m_aLogger;

    protected:
        OStackedLogging() { }

    protected:
        void    enterContext( const sal_Char* _pContextName );
        void    leaveTopContext( );
    };

#define ENTER_LOG_CONTEXT( name ) enterContext( name )
#define LEAVE_LOG_CONTEXT( ) leaveTopContext( )

#else
    struct OStackedLogging { };

#define ENTER_LOG_CONTEXT( name )
#define LEAVE_LOG_CONTEXT( )

#endif

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // XMLOFF_FORMS_LOGGING_HXX

