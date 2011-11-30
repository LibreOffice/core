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



#ifndef FPICKER_WIN32_VISTA_TYPES_HXX
#define FPICKER_WIN32_VISTA_TYPES_HXX

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "comptr.hxx"
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

typedef ComPtr< IFileDialog         , IID_IFileDialog                             > TFileDialog;
typedef ComPtr< IFileOpenDialog     , IID_IFileOpenDialog  , CLSID_FileOpenDialog > TFileOpenDialog;
typedef ComPtr< IFileSaveDialog     , IID_IFileSaveDialog  , CLSID_FileSaveDialog > TFileSaveDialog;
typedef ComPtr< IFileDialogEvents   , IID_IFileDialogEvents                       > TFileDialogEvents;
typedef ComPtr< IFileDialogCustomize, IID_IFileDialogCustomize                    > TFileDialogCustomize;

} // namespace vista
} // namespace win32
} // namespace fpicker

#undef css

#endif // FPICKER_WIN32_VISTA_TYPES_HXX
