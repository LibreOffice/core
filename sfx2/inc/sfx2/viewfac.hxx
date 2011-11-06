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


#ifndef _VIEWFAC_HXX
#define _VIEWFAC_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <tools/string.hxx>
#include <tools/resid.hxx>

class SfxViewFrame;
class SfxViewShell;
class Window;

typedef SfxViewShell* (*SfxViewCtor)(SfxViewFrame*, SfxViewShell*);
typedef void (*SfxViewInit)();

// CLASS -----------------------------------------------------------------
class SFX2_DLLPUBLIC SfxViewFactory
{
public:
    SfxViewFactory( SfxViewCtor fnC, SfxViewInit fnI,
                    sal_uInt16 nOrdinal, const sal_Char* asciiViewName );
    ~SfxViewFactory();

    SfxViewShell *CreateInstance(SfxViewFrame *pViewFrame, SfxViewShell *pOldSh);
    void          InitFactory();
    sal_uInt16        GetOrdinal() const { return nOrd; }

    /// returns a legacy view name. This is "view" with an appended ordinal/ID.
    String        GetLegacyViewName() const;

    /** returns a API-compatible view name.

        For details on which view names are specified, see the XModel2.getAvailableViewControllerNames
        documentation.
    */
    String        GetAPIViewName() const;

private:
    SfxViewCtor fnCreate;
    SfxViewInit fnInit;
    sal_uInt16      nOrd;
    const String    m_sViewName;
};

#endif

