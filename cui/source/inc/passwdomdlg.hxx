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


#ifndef _CUI_PASSWDOMDLG_HXX_
#define _CUI_PASSWDOMDLG_HXX_


#include <sfx2/basedlgs.hxx>

#include <memory>

//////////////////////////////////////////////////////////////////////

struct PasswordToOpenModifyDialog_Impl;

class PasswordToOpenModifyDialog : public SfxModalDialog
{
    std::auto_ptr< PasswordToOpenModifyDialog_Impl >  m_pImpl;

    // disallow use of copy c-tor and assignment operator
    PasswordToOpenModifyDialog( const PasswordToOpenModifyDialog & );
    PasswordToOpenModifyDialog & operator = ( const PasswordToOpenModifyDialog & );

public:
    PasswordToOpenModifyDialog( Window * pParent, sal_uInt16 nMinPasswdLen,
            sal_uInt16 nMaxPasswdLen /* 0 -> no max len enforced */,
            bool bIsPasswordToModify );
    virtual ~PasswordToOpenModifyDialog();

    // AbstractPasswordToOpenModifyDialog
    virtual String  GetPasswordToOpen() const;
    virtual String  GetPasswordToModify() const;
    virtual bool    IsRecommendToOpenReadonly() const;
};

//////////////////////////////////////////////////////////////////////

#endif

