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


#ifndef _SVTOOLS_CJKOPTIONS_HXX
#define _SVTOOLS_CJKOPTIONS_HXX

#include "svl/svldllapi.h"
#include <sal/types.h>
#include <unotools/options.hxx>

class SvtCJKOptions_Impl;

// class SvtCJKOptions --------------------------------------------------

class SVL_DLLPUBLIC SvtCJKOptions: public utl::detail::Options
{
private:
    SvtCJKOptions_Impl*    pImp;

public:

    enum EOption
    {
        E_CJKFONT,
        E_VERTICALTEXT,
        E_ASIANTYPOGRAPHY,
        E_JAPANESEFIND,
        E_RUBY,
        E_CHANGECASEMAP,
        E_DOUBLELINES,
        E_EMPHASISMARKS,
        E_VERTICALCALLOUT,
        E_ALL               // special one for IsAnyEnabled()/SetAll() functionality
    };

    // bDontLoad is for referencing purposes only
    SvtCJKOptions(sal_Bool bDontLoad = sal_False);
    virtual ~SvtCJKOptions();

    sal_Bool IsCJKFontEnabled() const;
    sal_Bool IsVerticalTextEnabled() const;
    sal_Bool IsAsianTypographyEnabled() const;
    sal_Bool IsJapaneseFindEnabled() const;
    sal_Bool IsRubyEnabled() const;
    sal_Bool IsChangeCaseMapEnabled() const;
    sal_Bool IsDoubleLinesEnabled() const;
    sal_Bool IsEmphasisMarksEnabled() const;
    sal_Bool IsVerticalCallOutEnabled() const;

    void        SetAll(sal_Bool bSet);
    sal_Bool    IsAnyEnabled() const;
    sal_Bool    IsReadOnly(EOption eOption) const;
};

#endif // _SVTOOLS_CJKOPTIONS_HXX

