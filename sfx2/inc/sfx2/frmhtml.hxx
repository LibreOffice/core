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



#ifndef _FRMHTML_HXX
#define _FRMHTML_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <sfx2/frmdescr.hxx>
#include <sfx2/sfxhtml.hxx>

class SfxFrameHTMLParser;
class _SfxFrameHTMLContexts;
class SfxFrameSetObjectShell;
class SvULongs;
class SfxMedium;
namespace svtools { class AsynchronLink; }

class SFX2_DLLPUBLIC SfxFrameHTMLParser : public SfxHTMLParser
{
    friend class _SfxFrameHTMLContext;

protected:
   SfxFrameHTMLParser( SvStream& rStream, sal_Bool bIsNewDoc=sal_True, SfxMedium *pMediumPtr=0 ):
   SfxHTMLParser( rStream, bIsNewDoc, pMediumPtr ) {};

public:
    // Diese Methoden koennen auch von anderen Parsern benutzt werden
    static void             ParseFrameOptions(SfxFrameDescriptor*, const HTMLOptions*, const String& );

};

#endif

