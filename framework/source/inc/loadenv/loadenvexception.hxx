/*************************************************************************
 *
 *  $RCSfile: loadenvexception.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 14:36:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_LOADENV_LOADENVEXCEPTION_HXX_
#define __FRAMEWORK_LOADENV_LOADENVEXCEPTION_HXX_

//_______________________________________________
// includes of own project

//_______________________________________________
// includes of uno interface

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

//_______________________________________________
// includes of an other project

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

//_______________________________________________
// namespace

namespace framework{

#ifndef css // conflict with define :-(
namespace css = ::com::sun::star;
#endif

//_______________________________________________
// definitions

/** @short  specify an exception, which can be used inside the
            load environment only.

    @descr  Of course outside code must wrapp it, to transport
            the occured information to its caller.

    @author as96863
 */
class LoadEnvException
{
    //___________________________________________
    // const

    public:

        /** @short  Can be used as an ID for an instance of a LoadEnvException.
            @descr  To prevent errors on adding/removing/changing such IDs here,
                    an enum field is used. Its int values are self organized ...
         */
        enum EIDs
        {
            /** @short  The specified URL/Stream/etcpp. can not be handled by a LoadEnv instance. */
            ID_UNSUPPORTED_CONTENT,

            /** @short  It was not possible to get access to global filter configuration.
                @descr  Might som neccsessary services could not be created. */
            ID_NO_CONFIG_ACCESS,

            /** @short  Some data obtained from the filter configuration seems to incorrect.
                @descr  Might a filter-type relation ship seem to be damaged. */
            ID_INVALID_FILTER_CONFIG,

            /** @short  indicates a corrupted media descriptor.
                @descr  Some parts are required - some other ones are optional. Such exception
                        should be thrown, if a required item does not exists. */
            ID_INVALID_MEDIADESCRIPTOR,

            /** @short  Its similar to an uno::RuntimeException ....
                @descr  But such runtime exception can break the whole office code.
                        So its capsulated to this specialized load environment only.
                        Mostly it indicates a missing but needed resource ... e.g the
                        global desktop reference! */
            ID_INVALID_ENVIRONMENT,

            /** @short  indicates a failed search for the right target frame. */
            ID_NO_TARGET_FOUND,

            /** @short  An already existing document was found inside a target frame.
                        But its controller could not be suspended successfully. Thats
                        why the new load request was cancelled. The document could not
                        be replaced. */
            ID_COULD_NOT_SUSPEND_CONTROLLER,

            /** @short  TODO */
            ID_COULD_NOT_REACTIVATE_CONTROLLER,

            /** @short  inidcates an already running load operation. Of yourse the same
                        instance cant be used for multiple load requests at the same time.
             */
            ID_STILL_RUNNING,

            /** @short  sometiems we cant specify the reason for an error, because we
                        was interrupted by an called code in an unexpected way ...
             */
            ID_GENERAL_ERROR
        };

    //___________________________________________
    // member

    public:

        /** @short  contains a suitable message, which describes the reason for this
                    exception. */
        ::rtl::OString m_sMessage;

        /** @short  An ID, which make this exception unique among others. */
        sal_Int32 m_nID;

        /** @short  Contains the original exception, if any occured. */
        css::uno::Any m_exOriginal;

        /** TODO
             Experimental use! May it can be usefully to know, if an exception was already
             catched and handled by an interaction and was might be rethrowed! */
        sal_Bool m_bHandled;

    //___________________________________________
    // interface

    public:

        /** @short  initialize a new instance with an ID.
            @descr  Some other items of this exception
                    (e.g. a suitable message) will be generated
                    automaticly.

            @param  nID
                    One of the defined const IDs of this class.
         */
        LoadEnvException(sal_Int32 nID)
        {
            m_nID = nID;
        }

        //_______________________________________

        /** @short  initialize a new instance with an ID
                    an wrap a detected exception into this one.
            @descr  Some other items of this exception
                    (e.g. a suitable message) will be generated
                    automaticly.

            @param  nID
                    One of the defined const IDs of this class.

            @param  exUno
                    the original catched uno exception.
         */
        LoadEnvException(      sal_Int32            nID  ,
                         const css::uno::Exception& exUno)
        {
            m_nID          = nID  ;
            m_exOriginal <<= exUno;
        }

        //_______________________________________

        /** @short  destruct an instance of this exception.
         */
        ~LoadEnvException()
        {
            m_sMessage = ::rtl::OString();
            m_nID      = 0;
            m_bHandled = false;
            m_exOriginal.clear();
        }
};

} // namespace framework

#endif // __FRAMEWORK_LOADENV_LOADENVEXCEPTION_HXX_
