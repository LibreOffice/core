/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IEnvironment.hxx,v $
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
#ifndef DBAUI_IENVIRONMENT_HXX
#define DBAUI_IENVIRONMENT_HXX

#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif

namespace dbaui
{
    // interface class for a generell environment
    class IEnvironment
    {
    protected:
    public:
        /** appends an error in the current environment.
            @param  _aException
                contains a description of the error or the error directly
        */
        virtual void appendError(const ::com::sun::star::sdbc::SQLException& _aException) = 0;

        /** clears the error state.
        */
        virtual void clearError() = 0;

        /** set the current error in the given parameter.
            @param  _rException
                will contain the current error
        */
        virtual void getError(::com::sun::star::sdbc::SQLException& _rException ) const = 0;

        /** @retrun
            returns <TRUE/> when an error was set otherwise <FALSE/>
        */
        virtual sal_Bool hasError() const = 0;

        /** gives access to the currently used connection
            @return
                the currently used connection.
        */
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& getConnection() const = 0;
    };
}
#endif // DBAUI_IENVIRONMENT_HXX
