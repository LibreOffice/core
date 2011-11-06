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


#ifndef _SFX2_OPENGRF_HXX
#define _SFX2_OPENGRF_HXX

#include <memory>       // auto_ptr
#include <svtools/filter.hxx>
#include <sfx2/dllapi.h>

struct  SvxOpenGrf_Impl;

class SFX2_DLLPUBLIC SvxOpenGraphicDialog
{
public:
    SvxOpenGraphicDialog    ( const String& rTitle );
    ~SvxOpenGraphicDialog   ();

    short                   Execute();

    void                    SetPath( const String& rPath );
    void                    SetPath( const String& rPath, sal_Bool bLinkState );
    String                  GetPath() const;

    int                     GetGraphic(Graphic&) const;

    void                    EnableLink(sal_Bool);
    void                    AsLink(sal_Bool);
    sal_Bool                IsAsLink() const;

    String                  GetCurrentFilter() const;
    void                    SetCurrentFilter(const String&);

    void                    SetControlHelpIds( const sal_Int16* _pControlId, const char** _pHelpId );
private:
    // disable copy and assignment
    SFX2_DLLPRIVATE SvxOpenGraphicDialog    (const SvxOpenGraphicDialog&);
    SFX2_DLLPRIVATE SvxOpenGraphicDialog& operator = ( const SvxOpenGraphicDialog & );

    const std::auto_ptr< SvxOpenGrf_Impl >  mpImpl;
};

#endif // _SVX_OPENGRF_HXX

