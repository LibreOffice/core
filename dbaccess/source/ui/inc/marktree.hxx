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



#ifndef _DBAUI_MARKTREE_HXX_
#define _DBAUI_MARKTREE_HXX_

#ifndef DBAUI_DBTREELISTBOX_HXX
#include "dbtreelistbox.hxx"
#endif



//.........................................................................
namespace dbaui
{
//.........................................................................

//========================================================================
//= OMarkableTreeListBox
//========================================================================
/** a tree list box where all entries can be marked (with a checkbox) and
    unmarked. In addition, inner nodes know a third state which applies
    if some, but not all of their descendants are marked.
*/
class OMarkableTreeListBox : public DBTreeListBox
{
    SvLBoxButtonData*   m_pCheckButton;
    Link                m_aCheckButtonHandler;

public:
    OMarkableTreeListBox( Window* pParent
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
                            , WinBits nWinStyle=0 );
    OMarkableTreeListBox( Window* pParent
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
                            , const ResId& rResId );
    ~OMarkableTreeListBox();

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    CheckButtonHdl();
    void            CheckButtons();     // make the button states consistent (bottom-up)

    /// the handler given is called whenever the check state of one or more items changed
    void SetCheckHandler(const Link& _rHdl) { m_aCheckButtonHandler = _rHdl; }

protected:
    virtual void Paint(const Rectangle& _rRect);
    virtual void checkedButton_noBroadcast(SvLBoxEntry* _pEntry);

    SvButtonState   implDetermineState(SvLBoxEntry* _pEntry);
        // determines the check state of the given entry, by analyzing the states of all descendants

private:
    void InitButtonData();
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_MARKTREE_HXX_

