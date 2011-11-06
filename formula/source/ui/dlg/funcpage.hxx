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



#ifndef FORMULA_FUNCPAGE_HXX
#define FORMULA_FUNCPAGE_HXX

#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#include <vcl/tabctrl.hxx>
#include <vcl/button.hxx>
#include <svtools/svtreebx.hxx>

#include <boost/shared_ptr.hpp>
#include <vector>
#include "ModuleHelper.hxx"
//============================================================================
namespace formula
{

class IFunctionDescription;
class IFunctionManager;
class IFunctionCategory;

//============================================================================
class FormulaListBox : public ListBox
{
protected:

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:
                    FormulaListBox( Window* pParent, WinBits nWinStyle = WB_BORDER );

                    FormulaListBox( Window* pParent, const ResId& rResId );

};



typedef const IFunctionDescription* TFunctionDesc;
//============================================================================
class FuncPage : public TabPage
{
private:
    OModuleClient   m_aModuleClient;
    Link            aDoubleClickLink;
    Link            aSelectionLink;
    FixedText       aFtCategory;
    ListBox         aLbCategory;
    FixedText       aFtFunction;
    FormulaListBox  aLbFunction;
    const IFunctionManager*
                    m_pFunctionManager;

    ::std::vector< TFunctionDesc >  aLRUList;
    rtl::OString    m_aHelpId;


    void impl_addFunctions(const IFunctionCategory* _pCategory);
                    DECL_LINK( SelHdl, ListBox* );
                    DECL_LINK( DblClkHdl, ListBox* );

protected:

    void            UpdateFunctionList();
    void            InitLRUList();


public:

                    FuncPage( Window* pParent,const IFunctionManager* _pFunctionManager);

    void            SetCategory(sal_uInt16 nCat);
    void            SetFunction(sal_uInt16 nFunc);
    void            SetFocus();
    sal_uInt16          GetCategory();
    sal_uInt16          GetFunction();
    sal_uInt16          GetFunctionEntryCount();

    sal_uInt16          GetFuncPos(const IFunctionDescription* _pDesc);
    const IFunctionDescription* GetFuncDesc( sal_uInt16 nPos ) const;
    String          GetSelFunctionName() const;

    void            SetDoubleClickHdl( const Link& rLink ) { aDoubleClickLink = rLink; }
    const Link&     GetDoubleClickHdl() const { return aDoubleClickLink; }

    void            SetSelectHdl( const Link& rLink ) { aSelectionLink = rLink; }
    const Link&     GetSelectHdl() const { return aSelectionLink; }

};

} // formula
#endif

