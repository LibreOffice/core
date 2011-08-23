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

#ifndef XMLOFF_FORMS_LOGGING_HXX
#define XMLOFF_FORMS_LOGGING_HXX

#ifndef _SAL_TYPES_H_ 
#include <sal/types.h>
#endif
#include <stack>
namespace rtl { class Logfile; }
namespace binfilter {


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
        ::std::stack< ::rtl::Logfile* >		m_aLogger;

    protected:
        OStackedLogging() { }

    protected:
        void	enterContext( const sal_Char* _pContextName );
        void	leaveTopContext( );
    };

#define ENTER_LOG_CONTEXT( name ) enterContext( name )
#define LEAVE_LOG_CONTEXT( ) leaveTopContext( )

#else
    struct OStackedLogging { };

#define ENTER_LOG_CONTEXT( name )
#define LEAVE_LOG_CONTEXT( )

#endif

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // XMLOFF_FORMS_LOGGING_HXX

