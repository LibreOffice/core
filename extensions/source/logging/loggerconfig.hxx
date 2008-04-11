/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: loggerconfig.hxx,v $
 * $Revision: 1.3 $
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

#ifndef EXTENSIONS_LOGGERCONFIG_HXX
#define EXTENSIONS_LOGGERCONFIG_HXX

/** === begin UNO includes === **/
#include <com/sun/star/logging/XLogger.hpp>
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
