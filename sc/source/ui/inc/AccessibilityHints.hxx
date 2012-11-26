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



#ifndef SC_ACCESSIBILITYHINTS_HXX
#define SC_ACCESSIBILITYHINTS_HXX

#include "viewdata.hxx"
#include <com/sun/star/uno/XInterface.hpp>
#include <svl/smplhint.hxx>
#include <svl/hint.hxx>

#define SC_HINT_ACC_SIMPLE_START    SFX_HINT_USER00
#define SC_HINT_ACC_TABLECHANGED    SC_HINT_ACC_SIMPLE_START + 1
#define SC_HINT_ACC_CURSORCHANGED   SC_HINT_ACC_SIMPLE_START + 2
#define SC_HINT_ACC_VISAREACHANGED  SC_HINT_ACC_SIMPLE_START + 3
#define SC_HINT_ACC_ENTEREDITMODE   SC_HINT_ACC_SIMPLE_START + 4
#define SC_HINT_ACC_LEAVEEDITMODE   SC_HINT_ACC_SIMPLE_START + 5
#define SC_HINT_ACC_MAKEDRAWLAYER   SC_HINT_ACC_SIMPLE_START + 6
#define SC_HINT_ACC_WINDOWRESIZED   SC_HINT_ACC_SIMPLE_START + 7

class ScAccWinFocusLostHint : public SfxHint
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                xOldAccessible;
public:
                ScAccWinFocusLostHint(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xOld );
                ~ScAccWinFocusLostHint();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                GetOldAccessible() const { return xOldAccessible; }
};

class ScAccWinFocusGotHint : public SfxHint
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                xNewAccessible;
public:
                ScAccWinFocusGotHint(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xNew );
                ~ScAccWinFocusGotHint();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                GetNewAccessible() const { return xNewAccessible; }
};

class ScAccGridWinFocusLostHint : public ScAccWinFocusLostHint
{
    ScSplitPos  eOldGridWin;
public:
                ScAccGridWinFocusLostHint( ScSplitPos eOldGridWin,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xOld );
                ~ScAccGridWinFocusLostHint();

    ScSplitPos  GetOldGridWin() const { return eOldGridWin; }
};

class ScAccGridWinFocusGotHint : public ScAccWinFocusGotHint
{
    ScSplitPos  eNewGridWin;
public:
                ScAccGridWinFocusGotHint( ScSplitPos eNewGridWin,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xNew );
                ~ScAccGridWinFocusGotHint();

    ScSplitPos  GetNewGridWin() const { return eNewGridWin; }
};

#endif
