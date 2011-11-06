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



#ifndef FPICKER_WIN32_VISTA_FILEPICKER_INTERNALNOTIFY_HXX
#define FPICKER_WIN32_VISTA_FILEPICKER_INTERNALNOTIFY_HXX

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "comptr.hxx"
#include "vistatypes.h"

#include <cppuhelper/basemutex.hxx>
#include <osl/interlck.h>

#include <shobjidl.h>

//-----------------------------------------------------------------------------
// namespace
//-----------------------------------------------------------------------------

#ifdef css
    #error "Clash on using CSS as namespace define."
#else
    #define css ::com::sun::star
#endif

namespace fpicker{
namespace win32{
namespace vista{

//-----------------------------------------------------------------------------
// types, const etcpp.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/** todo document me
 */
class IVistaFilePickerInternalNotify
{
    public:

        virtual void onAutoExtensionChanged (bool bChecked) = 0;

        virtual bool onFileTypeChanged( UINT nTypeIndex ) = 0;
};

}}}

#undef css

#endif FPICKER_WIN32_VISTA_FILEPICKER_INTERNALNOTIFY_HXX
