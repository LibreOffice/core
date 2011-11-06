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


#ifndef DBAUI_WIZ_EXTENDPAGES_HXX
#define DBAUI_WIZ_EXTENDPAGES_HXX

#ifndef DBAUI_WIZ_TYPESELECT_HXX
#include "WTypeSelect.hxx"
#endif

class SvStream;
namespace dbaui
{
    // ========================================================
    // Wizard Page: OWizHTMLExtend
    // ========================================================
    class OWizHTMLExtend : public OWizTypeSelect
    {
    protected:
        virtual SvParser*   createReader(sal_Int32 _nRows);
    public:
        OWizHTMLExtend(Window* pParent, SvStream& _rStream)
            : OWizTypeSelect( pParent, &_rStream )
        {
        }

        static OWizTypeSelect* Create( Window* _pParent, SvStream& _rInput ) { return new OWizHTMLExtend( _pParent, _rInput ); }

        virtual ~OWizHTMLExtend(){}
    };
    // ========================================================
    // Wizard Page: OWizRTFExtend
    // ========================================================
    class OWizRTFExtend : public OWizTypeSelect
    {
    protected:
        virtual SvParser* createReader(sal_Int32 _nRows);
    public:
        OWizRTFExtend(Window* pParent,SvStream& _rStream)
            : OWizTypeSelect( pParent, &_rStream )
        {
        }

        static OWizTypeSelect* Create( Window* _pParent, SvStream& _rInput ) { return new OWizRTFExtend( _pParent, _rInput ); }

        virtual ~OWizRTFExtend(){}
    };

    // ========================================================
    // Wizard Page: OWizNormalExtend
    // ========================================================
    class OWizNormalExtend : public OWizTypeSelect
    {
    protected:
        virtual SvParser* createReader(sal_Int32 _nRows);
    public:
        OWizNormalExtend(Window* pParent);
    };
}
#endif // DBAUI_WIZ_EXTENDPAGES_HXX



