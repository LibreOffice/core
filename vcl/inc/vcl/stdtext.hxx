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



#ifndef _VCL_STDTEXT_HXX
#define _VCL_STDTEXT_HXX

#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>

class Window;

// ------------------
// - Standard-Texte -
// ------------------

#define STANDARD_TEXT_FIRST                     STANDARD_TEXT_SERVICE_NOT_AVAILABLE
#define STANDARD_TEXT_SERVICE_NOT_AVAILABLE     ((sal_uInt16)0)
#define STANDARD_TEXT_LAST                      STANDARD_TEXT_SERVICE_NOT_AVAILABLE

XubString VCL_DLLPUBLIC GetStandardText( sal_uInt16 nStdText );

// -------------------------------------
// - Hilfsmethoden fuer Standard-Texte -
// -------------------------------------

void VCL_DLLPUBLIC ShowServiceNotAvailableError( Window* pParent, const XubString& rServiceName, sal_Bool bError );

#endif  // _VCL_STDTEXT_HXX
