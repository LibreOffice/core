/*************************************************************************
 *
 *  $RCSfile: headertablistbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:15:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
