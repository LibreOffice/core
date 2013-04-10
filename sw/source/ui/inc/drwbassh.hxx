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


#ifndef _SWDRWBASSH_HXX
#define _SWDRWBASSH_HXX
#include "basesh.hxx"

class SwView;
class SfxItemSet;
class SwDrawBase;
class AbstractSvxNameDialog;
struct SvxSwFrameValidation;

class SwDrawBaseShell: public SwBaseShell
{
    SwDrawBase* pDrawActual;

    sal_uInt16      eDrawMode;
    sal_Bool        bRotate : 1;
    sal_Bool        bSelMove: 1;

    DECL_LINK( CheckGroupShapeNameHdl, AbstractSvxNameDialog* );
    DECL_LINK(ValidatePosition, SvxSwFrameValidation* );
public:
                SwDrawBaseShell(SwView &rShell);
    virtual     ~SwDrawBaseShell();

    SFX_DECL_INTERFACE(SW_DRAWBASESHELL)
    TYPEINFO();

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        GetDrawAttrStateForIFBX( SfxItemSet& rSet );
    void        DisableState(SfxItemSet &rSet)               { Disable(rSet);}
    sal_Bool        Disable(SfxItemSet& rSet, sal_uInt16 nWhich = 0);

    void        StateStatusline(SfxItemSet &rSet);

};


#endif
