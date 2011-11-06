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



#ifndef _SALAQUAFPICKER_HXX_
#define _SALAQUAFPICKER_HXX_

//_____________________________________________________________________________
//  includes of other projects
//_____________________________________________________________________________

#include <osl/mutex.hxx>

#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HXX_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HXX_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#include "ControlHelper.hxx"

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class SalAquaPicker
{
public:
    // constructor
    SalAquaPicker();
    virtual ~SalAquaPicker();

    int run();
    int runandwaitforresult();

    inline rtl::OUString getDisplayDirectory() { return m_sDisplayDirectory; }

    inline ControlHelper* getControlHelper() const {
        return m_pControlHelper;
    }

protected:

    rtl::OUString m_sDisplayDirectory;

    NSSavePanel *m_pDialog;

    ControlHelper *m_pControlHelper;

    osl::Mutex m_rbHelperMtx;

    // The type of dialog
    enum NavigationServices_DialogType {
        NAVIGATIONSERVICES_OPEN,
        NAVIGATIONSERVICES_SAVE,
        NAVIGATIONSERVICES_DIRECTORY
    };

    NavigationServices_DialogType m_nDialogType;

    void implsetTitle( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::uno::RuntimeException );

    void implsetDisplayDirectory( const rtl::OUString& rDirectory )
        throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

    rtl::OUString implgetDisplayDirectory(  )
        throw( com::sun::star::uno::RuntimeException );

    void implInitialize( );

};

#endif
