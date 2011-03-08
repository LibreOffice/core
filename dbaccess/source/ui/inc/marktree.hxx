/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBAUI_MARKTREE_HXX_
#define _DBAUI_MARKTREE_HXX_

#include "dbtreelistbox.hxx"



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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
