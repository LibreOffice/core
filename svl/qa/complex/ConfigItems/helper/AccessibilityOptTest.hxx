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



#ifndef  SVTOOLS_ACCESSIBILITYOPTTEST_HXX
#define  SVTOOLS_ACCESSIBILITYOPTTEST_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <svl/accessibilityoptions.hxx>

namespace css = ::com::sun::star;

class AccessibilityOptTest
{
public:
    AccessibilityOptTest();
    ~AccessibilityOptTest();

    void impl_checkAccessibilityOptions();

private:
    void impl_checkGetAutoDetectSystemHC();
    void impl_checkGetIsForPagePreviews();
    void impl_checkGetIsHelpTipsDisappear();
    void impl_checkGetIsAllowAnimatedGraphics();
    void impl_checkGetIsAllowAnimatedText();
    void impl_checkGetIsAutomaticFontColor();
    void impl_checkGetIsSystemFont();
    void impl_checkGetHelpTipSeconds();
    void impl_checkIsSelectionInReadonly();

    void impl_checkSetAutoDetectSystemHC();
    void impl_checkSetIsForPagePreviews();
    void impl_checkSetIsHelpTipsDisappear();
    void impl_checkSetIsAllowAnimatedGraphics();
    void impl_checkSetIsAllowAnimatedText();
    void impl_checkSetIsAutomaticFontColor();
    void impl_checkSetIsSystemFont();
    void impl_checkSetHelpTipSeconds();
    void impl_checkSetSelectionInReadonly();

private:
    css::uno::Reference< css::container::XNameAccess > m_xCfg;
    SvtAccessibilityOptions aAccessibilityOpt;
};

#endif // #ifndef  SVTOOLS_ACCESSIBILITYOPTTEST_HXX
