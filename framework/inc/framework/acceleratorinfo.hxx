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



#ifndef __FRAMEWORK_HELPER_ACCELERATORINFO_HXX_
#define __FRAMEWORK_HELPER_ACCELERATORINFO_HXX_

#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/keycod.hxx>
#include <rtl/ustring.hxx>
#include <framework/fwedllapi.h>

namespace framework
{

typedef ::rtl::OUString ( *pfunc_getCommandURLFromKeyCode)( const KeyCode& );
typedef KeyCode ( *pfunc_getKeyCodeFromCommandURL )( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& aURL );

pfunc_getCommandURLFromKeyCode SAL_CALL SetCommandURLFromKeyCode( pfunc_getCommandURLFromKeyCode );
pfunc_getKeyCodeFromCommandURL SAL_CALL SetKeyCodeFromCommandURL( pfunc_getKeyCodeFromCommandURL );

::rtl::OUString SAL_CALL GetCommandURLFromKeyCode( const KeyCode& aKeyCode );
KeyCode GetKeyCodeFromCommandURL( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const rtl::OUString& aCommandURL );

}

#endif // __FRAMEWORK_HELPER_ACCELERATORINFO_HXX_
