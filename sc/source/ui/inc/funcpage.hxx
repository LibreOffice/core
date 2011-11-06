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



#ifndef SC_FUNCPAGE_HXX
#define SC_FUNCPAGE_HXX

#include "funcutl.hxx"
#include "global.hxx"       // ScAddress
#include <svtools/stdctrl.hxx>
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>

#endif
#include <vcl/tabctrl.hxx>
#include "parawin.hxx"
#include <svtools/svtreebx.hxx>
#include "compiler.hxx"
#include "cell.hxx"


class ScViewData;
class ScFuncName_Impl;
class ScDocument;
class ScFuncDesc;

//============================================================================

#define LRU_MAX 10

//============================================================================
class ScListBox : public ListBox
{
protected:

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:
                    ScListBox( Window* pParent, const ResId& rResId );
};




//============================================================================
class ScFuncPage : public TabPage
{
private:

    Link            aDoubleClickLink;
    Link            aSelectionLink;
    FixedText       aFtCategory;
    ListBox         aLbCategory;
    FixedText       aFtFunction;
    ScListBox       aLbFunction;
    ImageButton     aIBFunction;

    const ScFuncDesc*   aLRUList[LRU_MAX];


                    DECL_LINK( SelHdl, ListBox* );
                    DECL_LINK( DblClkHdl, ListBox* );

protected:

    void            UpdateFunctionList();
    void            InitLRUList();


public:

                    ScFuncPage( Window* pParent);

    void            SetCategory(sal_uInt16 nCat);
    void            SetFunction(sal_uInt16 nFunc);
    void            SetFocus();
    sal_uInt16          GetCategory();
    sal_uInt16          GetFunction();
    sal_uInt16          GetFunctionEntryCount();

    sal_uInt16          GetFuncPos(const ScFuncDesc*);
    const ScFuncDesc*   GetFuncDesc( sal_uInt16 nPos ) const;
    String          GetSelFunctionName() const;

    void            SetDoubleClickHdl( const Link& rLink ) { aDoubleClickLink = rLink; }
    const Link&     GetDoubleClickHdl() const { return aDoubleClickLink; }

    void            SetSelectHdl( const Link& rLink ) { aSelectionLink = rLink; }
    const Link&     GetSelectHdl() const { return aSelectionLink; }

};


#endif

