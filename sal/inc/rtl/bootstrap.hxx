/*************************************************************************
 *
 *  $RCSfile: bootstrap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2001-08-30 11:51:36 $
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
#ifndef _RTL_BOOTSTRAP_HXX_
#define _RTL_BOOTSTRAP_HXX_
#include <rtl/ustring.hxx>
#include <rtl/bootstrap.h>

namespace rtl
{
    class Bootstrap
    {
        void * _handle;
    public:
        /**
           @see rtl_bootstrap_setIniFileName
         */
        static inline void SAL_CALL setIniFilename( const ::rtl::OUString &sFile );

        /**
           @param sName name of the bootstrap value. case insensitive.
           @param outValue (out parameter). On success contains the value, otherwise
                  an empty string.
           @return sal_False, if no value could be retrieved, otherwise sal_True
           @see rtl_bootstrap_get
         */
        static inline sal_Bool get(
            const ::rtl::OUString &sName,
            ::rtl::OUString &outValue );

        /**
           @param sName name of the bootstrap value. case insensitive.
           @param outValue (out parameter). Contains the value associated with sName.
           @param aDefault if none of the other methods retrieved a value, outValue
                           is assigned to a Default.

           @see rtl_bootstrap_get
         */
        static inline void get(
            const ::rtl::OUString &sName,
            ::rtl::OUString &outValue,
            const ::rtl::OUString &aDefault );


        inline Bootstrap();
        inline Bootstrap(const OUString & iniName);
        inline ~Bootstrap();

        inline sal_Bool getFrom(const ::rtl::OUString &sName,
                                ::rtl::OUString &outValue) const;


        inline void getFrom(const ::rtl::OUString &sName,
                            ::rtl::OUString &outValue,
                            const ::rtl::OUString &aDefault) const;

        inline void getIniName(::rtl::OUString & iniName) const;

    };

    //----------------------------------------------------------------------------
    // IMPLEMENTATION
    //----------------------------------------------------------------------------
    inline void Bootstrap::setIniFilename( const ::rtl::OUString &sFile )
    {
        rtl_bootstrap_setIniFileName( sFile.pData );
    }

    inline sal_Bool Bootstrap::get( const ::rtl::OUString &sName,
                                    ::rtl::OUString & outValue )
    {
        return rtl_bootstrap_get( sName.pData , &(outValue.pData) , 0 );
    }

    inline void Bootstrap::get( const ::rtl::OUString &sName,
                                ::rtl::OUString & outValue,
                                const ::rtl::OUString & sDefault )
    {
        rtl_bootstrap_get( sName.pData , &(outValue.pData) , sDefault.pData );
    }

    inline Bootstrap::Bootstrap()
    {
        _handle = 0;
    }

    inline Bootstrap::Bootstrap(const OUString & iniName)
    {
        if(iniName.getLength())
            _handle = rtl_bootstrap_args_open(iniName.pData);

        else
            _handle = 0;
    }

    inline Bootstrap::~Bootstrap()
    {
        rtl_bootstrap_args_close(_handle);
    }


    inline sal_Bool Bootstrap::getFrom(const ::rtl::OUString &sName,
                                       ::rtl::OUString &outValue) const
    {
        return rtl_bootstrap_get_from_handle(_handle, sName.pData, &outValue.pData, 0);
    }

    inline void Bootstrap::getFrom(const ::rtl::OUString &sName,
                                   ::rtl::OUString &outValue,
                                   const ::rtl::OUString &aDefault) const
    {
        rtl_bootstrap_get_from_handle(_handle, sName.pData, &outValue.pData, aDefault.pData);
    }

    inline void Bootstrap::getIniName(::rtl::OUString & iniName) const
    {
        rtl_bootstrap_get_iniName_from_handle(_handle, &iniName.pData);
    }

}
#endif
