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



#ifndef _OFFAPP_INTERNATIONALOPTIONS_HXX_
#define _OFFAPP_INTERNATIONALOPTIONS_HXX_

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>

namespace offapp
{
    class InternationalOptionsPage : public SfxTabPage
    {
    private:
        struct IMPL;
        IMPL*               m_pImpl;

    protected:
                            InternationalOptionsPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        virtual sal_Bool        FillItemSet( SfxItemSet& _rSet );
        virtual void        Reset( const SfxItemSet& _rSet );

    public:
        virtual             ~InternationalOptionsPage();

        static SfxTabPage*  CreateSd( Window* _pParent, const SfxItemSet& _rAttrSet );
        static SfxTabPage*  CreateSc( Window* _pParent, const SfxItemSet& _rAttrSet );
    };

}   // /namespace offapp

#endif

