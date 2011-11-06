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


#ifndef _OPTLOAD_HXX
#define _OPTLOAD_HXX

#include <sfx2/tabdlg.hxx>

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <vcl/fixed.hxx>

class ScDocument;

class ScLoadOptPage : public SfxTabPage
{
private:
    FixedText   aLinkFT;
    RadioButton aAlwaysRB;
    RadioButton aRequestRB;
    RadioButton aNeverRB;
    CheckBox    aDocOnlyCB;
    GroupBox    aLinkGB;
    ScDocument *pDoc;

    DECL_LINK(  UpdateHdl, CheckBox* );

public:
                        ScLoadOptPage( Window* pParent,
                                         const SfxItemSet& rSet );
                        ~ScLoadOptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void                SetDocument(ScDocument*);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif


