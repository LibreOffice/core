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
#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#define INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <bf_svtools/options.hxx>
#endif

namespace binfilter
{

//_________________________________________________________________________________________________________________
//	types, enums, ...
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr			The method GetList() returns a list of property values.
                    Use follow defines to seperate values by names.
*//*-*************************************************************************************************************/

#define	HISTORY_PROPERTYNAME_URL			::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"		))
#define	HISTORY_PROPERTYNAME_FILTER			::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Filter"	))
#define	HISTORY_PROPERTYNAME_TITLE			::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"		))
#define	HISTORY_PROPERTYNAME_PASSWORD		::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Password"	))

/*-************************************************************************************************************//**
    @descr			You can use these enum values to specify right history if you call ouer interface methods.
*//*-*************************************************************************************************************/

enum EHistoryType
{
    ePICKLIST       = 0,
    eHISTORY        = 1,
    eHELPBOOKMARKS  = 2
};

//_________________________________________________________________________________________________________________
//	forward declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short			forward declaration to our private date container implementation
    @descr			We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtHistoryOptions_Impl;

//_________________________________________________________________________________________________________________
//	declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short			collect informations about history features
    @descr          -

    @implements		-
    @base			-

    @devstatus		ready to use
*//*-*************************************************************************************************************/

class  SvtHistoryOptions: public Options
{
    //-------------------------------------------------------------------------------------------------------------
    //	public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //	constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short		standard constructor and destructor
            @descr		This will initialize an instance with default values.
                        We implement these class with a refcount mechanism! Every instance of this class increase it
                        at create and decrease it at delete time - but all instances use the same data container!
                        He is implemented as a static member ...

            @seealso	member m_nRefCount
            @seealso	member m_pDataContainer

            @param		-
            @return		-

            @onerror	-
        *//*-*****************************************************************************************************/

         SvtHistoryOptions();
        virtual ~SvtHistoryOptions();

        //---------------------------------------------------------------------------------------------------------
        //	interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short		interface methods to get and set value of config key "org.openoffice.Office.Common/History/..."
            @descr		key "PickList"	: The last used documents displayed in the file menu.
                        key "History"	: The last opened documents general.
*/

    //-------------------------------------------------------------------------------------------------------------
    //	private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short		return a reference to a static mutex
            @descr		Make these class threadsafe.

            @seealso	-

            @param		-
            @return		A reference to a static mutex member.

            @onerror	-
        *//*-*****************************************************************************************************/

         static ::osl::Mutex& GetOwnStaticMutex();

    //-------------------------------------------------------------------------------------------------------------
    //	private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*Attention

            Don't initialize these static member in these header!
            a) Double dfined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtHistoryOptions_Impl*	m_pDataContainer	;	/// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32				m_nRefCount			;	/// internal ref count mechanism

};		// class SvtHistoryOptions

}

#endif	// #ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
