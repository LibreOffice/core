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


#ifndef INCLUDED_unotools_OPTIONSDLG_HXX
#define INCLUDED_unotools_OPTIONSDLG_HXX

#ifndef INCLUDED_unotoolsdllapi_H
#include "unotools/unotoolsdllapi.h"
#endif
#include <tools/string.hxx>
#include <unotools/options.hxx>

class SvtOptionsDlgOptions_Impl;

class UNOTOOLS_DLLPUBLIC SvtOptionsDialogOptions: public utl::detail::Options
{
private:
    SvtOptionsDlgOptions_Impl* m_pImp;

public:
                    SvtOptionsDialogOptions();
                    virtual ~SvtOptionsDialogOptions();

    sal_Bool        IsGroupHidden   (   const String& _rGroup ) const;
    sal_Bool        IsPageHidden    (   const String& _rPage,
                                        const String& _rGroup ) const;
    sal_Bool        IsOptionHidden  (   const String& _rOption,
                                        const String& _rPage,
                                        const String& _rGroup ) const;
};

#endif

