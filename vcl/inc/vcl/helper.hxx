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



#ifndef _PSPRINT_HELPER_HXX_
#define _PSPRINT_HELPER_HXX_

#include <list>

#include "vcl/dllapi.h"

#include "rtl/ustring.hxx"


// forwards
namespace osl { class File; }

namespace psp {
typedef int fontID;

void VCL_DLLPUBLIC getPrinterPathList( std::list< rtl::OUString >& rPathList, const char* pSubDir );

// note: gcc 3.4.1 warns about visibility if we retunr a const rtl::OUString& here
// seems to be a bug in gcc, now we return an object instead of a reference
rtl::OUString VCL_DLLPUBLIC getFontPath();

bool VCL_DLLPUBLIC convertPfbToPfa( osl::File& rInFile, osl::File& rOutFile );

// normalized path (equivalent to realpath)
void VCL_DLLPUBLIC normPath( rtl::OString& rPath );

// splits rOrgPath into dirname and basename
// rOrgPath will be subject to normPath
void VCL_DLLPUBLIC splitPath( rtl::OString& rOrgPath, rtl::OString& rDir, rtl::OString& rBase );

enum whichOfficePath { NetPath, UserPath, ConfigPath };
// note: gcc 3.4.1 warns about visibility if we retunr a const rtl::OUString& here
// seems to be a bug in gcc, now we return an object instead of a reference
rtl::OUString VCL_DLLPUBLIC getOfficePath( enum whichOfficePath ePath );
} // namespace

#endif // _PSPRINT_HELPER_HXX_
