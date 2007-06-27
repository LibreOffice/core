/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loggerconfig.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:58:59 $
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

#ifndef EXTENSIONS_LOGGERCONFIG_HXX
#define EXTENSIONS_LOGGERCONFIG_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LOGGING_XLOGGER_HPP_
#include <com/sun/star/logging/XLogger.hpp>
#endif
/** === end UNO includes === **/

namespace comphelper
{
    class ComponentContext;
}

//........................................................................
namespace logging
{
//........................................................................

    /** initializes the given logger from the configuration

        The configuration node /org.openoffice.Office.Logging/Settings/<logger_name>
        is examined for this. If it does not yet exist, it will be created.

        The function creates a default handler and a default formatter, as specified in the
        configuration.

        This function is currently external to the logger instance. Perhaps it can, on the long
        run, be moved to the logger implementation - not sure if it's the best place.
    */
    void    initializeLoggerFromConfiguration(
                const ::comphelper::ComponentContext& _rContext,
                const ::com::sun::star::uno::Reference< ::com::sun::star::logging::XLogger >& _rxLogger
            );

//........................................................................
} // namespace logging
//........................................................................

#endif // EXTENSIONS_LOGGERCONFIG_HXX
