/*************************************************************************
 *
 *  $RCSfile: editstt2.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
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

#ifndef _EDITSTT2_HXX
#define _EDITSTT2_HXX

#include <editstat.hxx>

class InternalEditStatus : public EditStatus
{

public:
            InternalEditStatus() { ; }

    void    TurnOnFlags( ULONG nFlags )
                { nControlBits |= nFlags; }

    void    TurnOffFlags( ULONG nFlags )
                { nControlBits &= ~nFlags; }

    void    TurnOnStatusBits( ULONG nBits )
                { nStatusBits |= nBits; }

    void    TurnOffStatusBits( ULONG nBits )
                { nStatusBits &= ~nBits; }


    BOOL    UseCharAttribs() const
                { return ( ( nControlBits & EE_CNTRL_USECHARATTRIBS ) != 0 ); }

    BOOL    NotifyCursorMovements() const
                { return ( ( nControlBits & EE_CNTRL_CRSRLEFTPARA ) != 0 ); }

    BOOL    UseIdleFormatter() const
                { return ( ( nControlBits & EE_CNTRL_DOIDLEFORMAT) != 0 ); }

    BOOL    AllowPasteSpecial() const
                { return ( ( nControlBits & EE_CNTRL_PASTESPECIAL ) != 0 ); }

    BOOL    DoAutoIndenting() const
                { return ( ( nControlBits & EE_CNTRL_AUTOINDENTING ) != 0 ); }

    BOOL    DoUndoAttribs() const
                { return ( ( nControlBits & EE_CNTRL_UNDOATTRIBS ) != 0 ); }

    BOOL    OneCharPerLine() const
                { return ( ( nControlBits & EE_CNTRL_ONECHARPERLINE ) != 0 ); }

    BOOL    IsOutliner() const
                { return ( ( nControlBits & EE_CNTRL_OUTLINER ) != 0 ); }

    BOOL    IsOutliner2() const
                { return ( ( nControlBits & EE_CNTRL_OUTLINER2 ) != 0 ); }

    BOOL    IsAnyOutliner() const
                { return IsOutliner() || IsOutliner2(); }

    BOOL    DoNotUseColors() const
                { return ( ( nControlBits & EE_CNTRL_NOCOLORS ) != 0 ); }

    BOOL    AllowBigObjects() const
                { return ( ( nControlBits & EE_CNTRL_ALLOWBIGOBJS ) != 0 ); }

    BOOL    DoOnlineSpelling() const
                { return ( ( nControlBits & EE_CNTRL_ONLINESPELLING ) != 0 ); }

    BOOL    DoStretch() const
                { return ( ( nControlBits & EE_CNTRL_STRETCHING ) != 0 ); }

    BOOL    AutoPageSize() const
                { return ( ( nControlBits & EE_CNTRL_AUTOPAGESIZE ) != 0 ); }
    BOOL    AutoPageWidth() const
                { return ( ( nControlBits & EE_CNTRL_AUTOPAGESIZEX ) != 0 ); }
    BOOL    AutoPageHeight() const
                { return ( ( nControlBits & EE_CNTRL_AUTOPAGESIZEY ) != 0 ); }

    BOOL    MarkFields() const
                { return ( ( nControlBits & EE_CNTRL_MARKFIELDS ) != 0 ); }

    BOOL    DoRestoreFont() const
                { return ( ( nControlBits & EE_CNTRL_RESTOREFONT ) != 0 ); }

    BOOL    DoImportRTFStyleSheets() const
                { return ( ( nControlBits & EE_CNTRL_RTFSTYLESHEETS ) != 0 ); }

    BOOL    DoDrawRedLines() const
                { return ( ( nControlBits & EE_CNTRL_NOREDLINES ) == 0 ); }

    BOOL    DoAutoCorrect() const
                { return ( ( nControlBits & EE_CNTRL_AUTOCORRECT ) != 0 ); }

    BOOL    DoAutoComplete() const
                { return ( ( nControlBits & EE_CNTRL_AUTOCOMPLETE ) != 0 ); }

    BOOL    DoTabIndenting() const
                { return ( ( nControlBits & EE_CNTRL_TABINDENTING ) != 0 ); }

    BOOL    DoFormat100() const
                { return ( ( nControlBits & EE_CNTRL_FORMAT100 ) != 0 ); }

    BOOL    ULSpaceSummation() const
                { return ( ( nControlBits & EE_CNTRL_ULSPACESUMMATION ) != 0 ); }

    BOOL    ULSpaceFirstParagraph() const
                { return ( ( nControlBits & EE_CNTRL_ULSPACEFIRSTPARA ) != 0 ); }
};

#endif // _EDITSTT2_HXX

