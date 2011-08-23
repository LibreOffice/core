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

#ifndef _EDITSTT2_HXX
#define _EDITSTT2_HXX

#include <editstat.hxx>
namespace binfilter {

class InternalEditStatus : public EditStatus
{

public:
            InternalEditStatus() { ; }

    void	TurnOnFlags( ULONG nFlags )
                { nControlBits |= nFlags; }

    void	TurnOffFlags( ULONG nFlags )
                { nControlBits &= ~nFlags; }

    void	TurnOnStatusBits( ULONG nBits )
                { nStatusBits |= nBits; }

    void	TurnOffStatusBits( ULONG nBits )
                { nStatusBits &= ~nBits; }


    BOOL	UseCharAttribs() const
                { return ( ( nControlBits & EE_CNTRL_USECHARATTRIBS ) != 0 ); }

    BOOL	NotifyCursorMovements() const
                { return ( ( nControlBits & EE_CNTRL_CRSRLEFTPARA ) != 0 ); }

    BOOL	UseIdleFormatter() const
                { return ( ( nControlBits & EE_CNTRL_DOIDLEFORMAT) != 0 ); }

    BOOL	AllowPasteSpecial() const
                { return ( ( nControlBits & EE_CNTRL_PASTESPECIAL ) != 0 ); }

    BOOL	DoAutoIndenting() const
                { return ( ( nControlBits & EE_CNTRL_AUTOINDENTING ) != 0 ); }

    BOOL	DoUndoAttribs() const
                { return ( ( nControlBits & EE_CNTRL_UNDOATTRIBS ) != 0 ); }

    BOOL	OneCharPerLine() const
                { return ( ( nControlBits & EE_CNTRL_ONECHARPERLINE ) != 0 ); }

    BOOL	IsOutliner() const
                { return ( ( nControlBits & EE_CNTRL_OUTLINER ) != 0 ); }

    BOOL	IsOutliner2() const
                { return ( ( nControlBits & EE_CNTRL_OUTLINER2 ) != 0 ); }

    BOOL	IsAnyOutliner() const
                { return IsOutliner() || IsOutliner2(); }

    BOOL	DoNotUseColors() const
                { return ( ( nControlBits & EE_CNTRL_NOCOLORS ) != 0 ); }

    BOOL	AllowBigObjects() const
                { return ( ( nControlBits & EE_CNTRL_ALLOWBIGOBJS ) != 0 ); }

    BOOL	DoStretch() const
                { return ( ( nControlBits & EE_CNTRL_STRETCHING ) != 0 ); }

    BOOL	AutoPageSize() const
                { return ( ( nControlBits & EE_CNTRL_AUTOPAGESIZE ) != 0 ); }
    BOOL	AutoPageWidth() const
                { return ( ( nControlBits & EE_CNTRL_AUTOPAGESIZEX ) != 0 ); }
    BOOL	AutoPageHeight() const
                { return ( ( nControlBits & EE_CNTRL_AUTOPAGESIZEY ) != 0 ); }

    BOOL	MarkFields() const
                { return ( ( nControlBits & EE_CNTRL_MARKFIELDS ) != 0 ); }

    BOOL	DoRestoreFont() const
                { return ( ( nControlBits & EE_CNTRL_RESTOREFONT ) != 0 ); }

    BOOL	DoImportRTFStyleSheets() const
                { return ( ( nControlBits & EE_CNTRL_RTFSTYLESHEETS ) != 0 ); }

    BOOL	DoDrawRedLines() const
                { return ( ( nControlBits & EE_CNTRL_NOREDLINES ) == 0 ); }

    BOOL	DoAutoCorrect() const
                { return ( ( nControlBits & EE_CNTRL_AUTOCORRECT ) != 0 ); }

    BOOL	DoAutoComplete() const
                { return ( ( nControlBits & EE_CNTRL_AUTOCOMPLETE ) != 0 ); }

    BOOL	DoTabIndenting() const
                { return ( ( nControlBits & EE_CNTRL_TABINDENTING ) != 0 ); }

    BOOL	DoFormat100() const
                { return ( ( nControlBits & EE_CNTRL_FORMAT100 ) != 0 ); }
    
    BOOL	ULSpaceSummation() const
                { return ( ( nControlBits & EE_CNTRL_ULSPACESUMMATION ) != 0 ); }
    
    BOOL	ULSpaceFirstParagraph() const
                { return ( ( nControlBits & EE_CNTRL_ULSPACEFIRSTPARA ) != 0 ); }
};

}//end of namespace binfilter
#endif // _EDITSTT2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
