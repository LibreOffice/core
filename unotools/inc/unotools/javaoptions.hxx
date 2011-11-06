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


#ifndef _unotools_JAVAOPTIONS_HXX
#define _unotools_JAVAOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <sal/types.h>
#include <unotools/configitem.hxx>


// class SvtJavaOptions --------------------------------------------------

struct SvtJavaOptions_Impl;

class UNOTOOLS_DLLPUBLIC SvtJavaOptions : public utl::ConfigItem
{
    SvtJavaOptions_Impl* pImpl;
public:
    enum EOption
    {
        E_ENABLED,
        E_SECURITY,
        E_NETACCESS,
        E_USERCLASSPATH,
        E_EXECUTEAPPLETS
    };

    SvtJavaOptions();
    ~SvtJavaOptions();

    virtual void    Commit();

    sal_Bool        IsEnabled() const;
    sal_Bool        IsSecurity()const;
    sal_Int32       GetNetAccess() const;
    rtl::OUString&  GetUserClassPath()const;
    sal_Bool        IsExecuteApplets() const;

    void SetEnabled(sal_Bool bSet) ;
    void SetSecurity(sal_Bool bSet);
    void SetNetAccess(sal_Int32 nSet) ;
    void SetUserClassPath(const rtl::OUString& rSet);
    void SetExecuteApplets(sal_Bool bSet);

    sal_Bool IsReadOnly( EOption eOption ) const;
};

#endif //

