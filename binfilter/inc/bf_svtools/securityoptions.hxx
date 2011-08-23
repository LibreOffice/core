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
#ifndef INCLUDED_SVTOOLS_SECURITYOPTIONS_HXX
#define INCLUDED_SVTOOLS_SECURITYOPTIONS_HXX

//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
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
//	forward declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short			forward declaration to our private date container implementation
    @descr			We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtSecurityOptions_Impl;

//_________________________________________________________________________________________________________________
//	declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          These values present modes to handle StarOffice basic scripts.
                    see GetBasicMode/SetBasicMode() for further informations
*//*-*************************************************************************************************************/

enum EBasicSecurityMode
{
    eNEVER_EXECUTE	= 0,
    eFROM_LIST		= 1,
    eALWAYS_EXECUTE = 2
};

/*-************************************************************************************************************//**
    @short			collect informations about security features
    @descr          -

    @implements		-
    @base			-

    @ATTENTION		This class is partially threadsafe.

    @devstatus		ready to use
*//*-*************************************************************************************************************/

class  SvtSecurityOptions: public Options
{
    //-------------------------------------------------------------------------------------------------------------
    //  public types
    //-------------------------------------------------------------------------------------------------------------
    public:

        enum EOption
        {
            E_SECUREURLS,
            E_BASICMODE,					// xmlsec05 depricated
            E_EXECUTEPLUGINS,				// xmlsec05 depricated
            E_WARNING,						// xmlsec05 depricated
            E_CONFIRMATION,					// xmlsec05 depricated
            E_DOCWARN_SAVEORSEND,
            E_DOCWARN_SIGNING,
            E_DOCWARN_PRINT,
            E_DOCWARN_CREATEPDF,
            E_DOCWARN_REMOVEPERSONALINFO,
            E_DOCWARN_RECOMMENDPASSWORD,
            E_MACRO_SECLEVEL,
            E_MACRO_TRUSTEDAUTHORS,
            E_MACRO_DISABLE,
            E_CTRLCLICK_HYPERLINK
        };

        enum MacroAction
        {
            MA_DONTRUN = 0,
            MA_CONFIRM,
            MA_RUN
        };

        typedef ::com::sun::star::uno::Sequence< ::rtl::OUString > Certificate;

        /*
        // MT: Doesn't work for sequence...
        struct Certificate
        {
            ::rtl::OUString SubjectName;
            ::rtl::OUString SerialNumber;
            ::rtl::OUString RawData;
        };
        */

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

         SvtSecurityOptions();
        virtual ~SvtSecurityOptions();

    //-------------------------------------------------------------------------------------------------------------
    //	private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short		return a reference to a static mutex
            @descr		These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.

            @seealso	-

            @param		-
            @return		A reference to a static mutex member.

            @onerror	-
        *//*-*****************************************************************************************************/

         static ::osl::Mutex& GetInitMutex();

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

        static SvtSecurityOptions_Impl*	m_pDataContainer	;	/// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32				m_nRefCount			;	/// internal ref count mechanism

};		// class SvtSecurityOptions

}

#endif	// #ifndef INCLUDED_SVTOOLS_SECURITYOPTIONS_HXX
