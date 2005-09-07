/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: headertablistbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:02:44 $
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

#ifndef _HEADERTABLISTBOX_HXX
#define _HEADERTABLISTBOX_HXX

#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif


class _HeaderTabListBox : public Control
{
private:
    SvHeaderTabListBox          maListBox;
    HeaderBar                   maHeaderBar;
protected:
    DECL_LINK( HeaderEndDrag_Impl, HeaderBar* );
    virtual long                Notify( NotifyEvent& rNEvt );
public:
                                _HeaderTabListBox( Window* pParent, const ResId& rId );
    virtual                     ~_HeaderTabListBox();

    inline SvHeaderTabListBox&  GetListBox( void );
    inline HeaderBar&           GetHeaderBar( void );

    void                        ConnectElements( void );
                                    // should be called after all manipulations on elements are done
                                    // calcs real sizes depending on sizes of this
    void                        Show( BOOL bVisible = TRUE, USHORT nFlags = 0 );    // same meaning as Windows::Show()
    void                        Enable( BOOL bEnable = TRUE, BOOL bChild = TRUE );  // same meaning as Windows::Enable()
};

inline SvHeaderTabListBox& _HeaderTabListBox::GetListBox( void )
{
    return maListBox;
}

inline HeaderBar& _HeaderTabListBox::GetHeaderBar( void )
{
    return maHeaderBar;
}


#endif
