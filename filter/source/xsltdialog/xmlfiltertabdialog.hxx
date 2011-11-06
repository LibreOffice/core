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


#ifndef _XMLFILTERTABDIALOG_HXX_
#define _XMLFILTERTABDIALOG_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <sfx2/tabdlg.hxx>

class Window;
class ResMgr;

class filter_info_impl;
class XMLFilterTabPageBasic;
class XMLFilterTabPageXSLT;

class XMLFilterTabDialog: public TabDialog
{
public:
    XMLFilterTabDialog( Window *pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF, const filter_info_impl* pInfo );
    virtual ~XMLFilterTabDialog();

    ResMgr& getResMgr() { return mrResMgr; }

    bool onOk();

    filter_info_impl* getNewFilterInfo() const;

private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

    DECL_LINK( ActivatePageHdl, TabControl * );
    DECL_LINK( DeactivatePageHdl, TabControl * );
    DECL_LINK( OkHdl, Button * );
    DECL_LINK( CancelHdl, Button * );

    ResMgr& mrResMgr;

    const filter_info_impl* mpOldInfo;
    filter_info_impl* mpNewInfo;

    TabControl      maTabCtrl;
    OKButton        maOKBtn;
    CancelButton    maCancelBtn;
    HelpButton      maHelpBtn;

    XMLFilterTabPageBasic*  mpBasicPage;
    XMLFilterTabPageXSLT* mpXSLTPage;
};


#endif
