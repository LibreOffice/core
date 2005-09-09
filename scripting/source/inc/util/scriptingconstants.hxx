/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scriptingconstants.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:26:22 $
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
