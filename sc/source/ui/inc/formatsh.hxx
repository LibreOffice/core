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



#ifndef SC_FORMATSH_HXX
#define SC_FORMATSH_HXX

#include <sfx2/shell.hxx>
#include "shellids.hxx"
#include <sfx2/module.hxx>
#include <svx/svdmark.hxx>

class ScViewData;

class ScFormatShell: public SfxShell
{
    ScViewData* pViewData;

protected:

    ScViewData*         GetViewData(){return pViewData;}
    const ScViewData*   GetViewData() const {return pViewData;}

public:

    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_FORMAT_SHELL)

                ScFormatShell(ScViewData* pData);
    virtual     ~ScFormatShell();

    void        ExecuteNumFormat( SfxRequest& rReq );
    void        GetNumFormatState( SfxItemSet& rSet );

    void        ExecuteAttr( SfxRequest& rReq );
    void        GetAttrState( SfxItemSet& rSet );

    void        ExecuteAlignment( SfxRequest& rReq );

    void        ExecuteTextAttr( SfxRequest& rReq );
    void        GetTextAttrState( SfxItemSet& rSet );

    void        GetAlignState( SfxItemSet& rSet );
    void        GetBorderState( SfxItemSet& rSet );

    void        ExecuteStyle( SfxRequest& rReq );
    void        GetStyleState( SfxItemSet& rSet );

    void        ExecuteTextDirection( SfxRequest& rReq );
    void        GetTextDirectionState( SfxItemSet& rSet );

    void        ExecFormatPaintbrush( SfxRequest& rReq );
    void        StateFormatPaintbrush( SfxItemSet& rSet );

    void        ExecViewOptions( SfxRequest& rReq );
    void        GetViewOptions( SfxItemSet& rSet );
};

#endif
