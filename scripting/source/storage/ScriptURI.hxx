/*************************************************************************
 *
 *  $RCSfile: ScriptURI.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dfoster $ $Date: 2003-07-22 14:27:59 $
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
