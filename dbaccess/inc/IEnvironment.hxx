/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IEnvironment.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 07:48:19 $
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
