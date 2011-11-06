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


#ifndef _SVX_MULTIFIL_HXX
#define _SVX_MULTIFIL_HXX

// include ---------------------------------------------------------------

#include "multipat.hxx"

// #97807# ----------------------------------------------------
#include <ucbhelper/content.hxx>
#include <map>

// class SvxMultiFileDialog ----------------------------------------------

class SvxMultiFileDialog : public SvxMultiPathDialog
{
private:
    // #97807# -------------------------------------
    std::map< String, ::ucbhelper::Content >   aFileContentMap;

    DECL_LINK( AddHdl_Impl, PushButton * );
    DECL_LINK( DelHdl_Impl, PushButton * );

public:
    SvxMultiFileDialog( Window* pParent, sal_Bool bEmptyAllowed = sal_False );
    ~SvxMultiFileDialog();

    String  GetFiles() const { return SvxMultiPathDialog::GetPath(); }
    void    SetFiles( const String& rPath ) { SvxMultiPathDialog::SetPath(rPath); aDelBtn.Enable(); }
};


#endif

