/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: marktree.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:58:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

