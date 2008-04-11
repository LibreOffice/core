/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptURI.hxx,v $
 * $Revision: 1.5 $
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

#ifndef __FRAMEWORK_STORAGE_SCRIPTURI_HXX_
#define __FRAMEWORK_STORAGE_SCRIPTURI_HXX_

#include <osl/mutex.hxx>
#include <rtl/ustring>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace scripting_impl {
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

struct Uri {
    bool valid;
    ::rtl::OUString uri;
    ::rtl::OUString location;
    ::rtl::OUString language;
    ::rtl::OUString functionName;
    ::rtl::OUString logicalName;
};
/**
 * Helper class for dealing with script URIs.
 */
class ScriptURI
{
public:
    ScriptURI( const ::rtl::OUString& scriptURI )
        throw ( css::lang::IllegalArgumentException );
    virtual ~ScriptURI()  SAL_THROW ( () );

    /**
     *  This function returns the location of the script
     *
     */
    virtual ::rtl::OUString  getLocation();

    /**
     *  This function returns the language of the script, eg. java,
     *  StarBasic,...
     *
     */
    virtual ::rtl::OUString  getLanguage();

    /**
     *  This function returns the language dependent function name of
     *  the script
     */
    virtual ::rtl::OUString  getFunctionName();

    /**
     *  This function returns the language independent logical name of
     *  the script
     */
    virtual ::rtl::OUString  getLogicalName();

    /**
     *  This function returns the full URI
     *
     */
    virtual ::rtl::OUString  getURI();

private:
    ::osl::Mutex m_mutex;

    /** @internal */
    sal_Bool m_valid;

    //the private strings
    /** the string representation of the this objects URI */
    ::rtl::OUString m_uri;
    /** the location of the script referred to by this URI */
    ::rtl::OUString m_location;
    /** the language of the script referred to by this URI */
    ::rtl::OUString m_language;
    /** the language dependent function name of the script referred to by this URI */
    ::rtl::OUString m_functionName;
    /** the language independent logical name of the script referred to by this URI */
    ::rtl::OUString m_logicalName;

    //attempt to parse the URI provided
    /** @internal */
    Uri parseIt();
    //set the members
    /** @internal */
    void set_values( Uri );
    bool isValid();
}
; // class ScriptURI

} //namespace script_uri

#endif // define __FRAMEWORK_STORAGE_SCRIPTURI_HXX_
