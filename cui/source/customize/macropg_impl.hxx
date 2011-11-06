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



#ifndef _MACROPG_IMPL_HXX
#define _MACROPG_IMPL_HXX

class _SvxMacroTabPage_Impl
{
public:
                                    _SvxMacroTabPage_Impl( const SfxItemSet& rAttrSet );
                                    ~_SvxMacroTabPage_Impl();

    FixedText*                      pAssignFT;
    PushButton*                     pAssignPB;
    PushButton*                     pAssignComponentPB;
    PushButton*                     pDeletePB;
    Image*                          pMacroImg;
    Image*                          pComponentImg;
    Image*                          pMacroImg_h;
    Image*                          pComponentImg_h;
    String*                         pStrEvent;
    String*                         pAssignedMacro;
    _HeaderTabListBox*              pEventLB;
    sal_Bool                            bReadOnly;
    sal_Bool                            bIDEDialogMode;
};

class AssignComponentDialog : public ModalDialog
{
private:
    FixedText       maMethodLabel;
    Edit            maMethodEdit;
    OKButton        maOKButton;
    CancelButton    maCancelButton;
    HelpButton      maHelpButton;

    ::rtl::OUString maURL;

    DECL_LINK(ButtonHandler, Button *);

public:
    AssignComponentDialog( Window * pParent, const ::rtl::OUString& rURL );
    ~AssignComponentDialog();

    ::rtl::OUString getURL( void ) const
        { return maURL; }
};

#endif
