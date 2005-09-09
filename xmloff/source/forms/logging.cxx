/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: logging.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:13:53 $
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
#include "logging.hxx"
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

#ifdef TIMELOG
    //=====================================================================
    //= OStackedLogging
    //=====================================================================
    //---------------------------------------------------------------------
    void OStackedLogging::enterContext( const sal_Char* _pContextName )
    {
        m_aLogger.push( new ::rtl::Logfile( _pContextName ) );
    }

    //---------------------------------------------------------------------
    void OStackedLogging::leaveTopContext( )
    {
        delete m_aLogger.top();
        m_aLogger.pop();
    }
#endif

//.........................................................................
}   // namespace xmloff
//.........................................................................


