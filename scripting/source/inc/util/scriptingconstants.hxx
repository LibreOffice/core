/*************************************************************************
 *
 *  $RCSfile: scriptingconstants.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-10-22 14:04:27 $
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
#ifndef _COM_SUN_STAR_SCRIPTING_UTIL_SCRIPTINGCONSTANTS_HXX_
#define _COM_SUN_STAR_SCRIPTING_UTIL_SCRIPTINGCONSTANTS_HXX_

namespace scripting_constants
{

class ScriptingConstantsPool
{
public:
    const ::rtl::OUString DOC_REF;
    const ::rtl::OUString DOC_STORAGE_ID;
    const ::rtl::OUString DOC_URI;
    const ::rtl::OUString RESOLVED_STORAGE_ID;
    const ::rtl::OUString SCRIPT_INFO;
    const ::rtl::OUString SCRIPTSTORAGEMANAGER_SERVICE;
    const sal_Int32 SHARED_STORAGE_ID;
    const sal_Int32 USER_STORAGE_ID;
    const sal_Int32 DOC_STORAGE_ID_NOT_SET;

    static ScriptingConstantsPool& instance()
    {
        static ScriptingConstantsPool *pPool = 0;
        if( ! pPool )
        {
            ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
            if( ! pPool )
            {
                static ScriptingConstantsPool pool;
                pPool = &pool;
            }
        }
        return *pPool;
    }
private:
    ScriptingConstantsPool( const ScriptingConstantsPool & );
    ScriptingConstantsPool& operator = ( const ScriptingConstantsPool & );
    ScriptingConstantsPool()
        : DOC_REF( RTL_CONSTASCII_USTRINGPARAM( "SCRIPTING_DOC_REF" ) ),
        DOC_STORAGE_ID( RTL_CONSTASCII_USTRINGPARAM(
            "SCRIPTING_DOC_STORAGE_ID" ) ),
        DOC_URI( RTL_CONSTASCII_USTRINGPARAM( "SCRIPTING_DOC_URI" ) ),
        RESOLVED_STORAGE_ID( RTL_CONSTASCII_USTRINGPARAM(
            "SCRIPTING_RESOLVED_STORAGE_ID" ) ),
        SCRIPT_INFO( RTL_CONSTASCII_USTRINGPARAM( "SCRIPT_INFO" ) ),
        SCRIPTSTORAGEMANAGER_SERVICE( RTL_CONSTASCII_USTRINGPARAM(
            "/singletons/com.sun.star.script.framework.storage.theScriptStorageManager" ) ),
        SHARED_STORAGE_ID( 0 ), USER_STORAGE_ID( 1 ),
        DOC_STORAGE_ID_NOT_SET( -1 )
    {}
};

}
#endif
