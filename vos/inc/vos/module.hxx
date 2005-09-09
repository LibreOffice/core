/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: module.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:00:02 $
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


#ifndef _VOS_MODULE_HXX_
#define _VOS_MODULE_HXX_

#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#   include <rtl/ustring.hxx>
#endif
#ifndef _OSL_MODULE_H_
#   include <osl/module.h>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif

class OModule : public OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OModule, vos));

public:

    /// default c'tor
    OModule();

    /// this c'tor is a combination of the default c'tor and load()
    OModule(const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT);
    virtual ~OModule();

    /// loads the specified module
    sal_Bool SAL_CALL load(const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT);

    /// unloads the currently loaded module
    void SAL_CALL unload();

    /// returns sal_True, if a module is loaded, sal_False otherwise
    sal_Bool SAL_CALL isLoaded();

#if 0
    // not implemented yet
    // returns the name of the currently loaded module or an empty string if none.
    ::rtl::OUString SAL_CALL getModuleName();
#endif

    /// returns a pointer to the specified Symbol if found, NULL otherwise
    void* SAL_CALL getSymbol(const ::rtl::OUString& strSymbolName);

protected:

    oslModule m_Module;
};

#ifdef _USE_NAMESPACE
}
#endif

#endif


