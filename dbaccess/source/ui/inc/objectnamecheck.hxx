/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectnamecheck.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:33:01 $
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

#ifndef DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX
#define DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

namespace rtl { class OUString; }
namespace dbtools { class SQLExceptionInfo; }

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= IObjectNameCheck
    //====================================================================
    /** interface encapsulating the check for the validity of an object name
    */
    class IObjectNameCheck
    {
    public:
        /** determines whether a given object name is valid

            @param  _rObjectName
                the name to check
            @param  _out_rErrorToDisplay
                output parameter taking an error message describing why the name is not
                valid, if applicable.

            @return
                <TRUE/> if and only if the given name is valid.
        */
        virtual bool    isNameValid(
            const ::rtl::OUString& _rObjectName,
            ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay
        ) const = 0;

    public:
        virtual ~IObjectNameCheck() { }
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX
