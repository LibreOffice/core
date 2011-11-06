/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
