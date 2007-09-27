/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numfmtlb.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:05:52 $
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
#ifndef _SWNUMFMTLB_HXX
#define _SWNUMFMTLB_HXX

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

class SwView;

class SW_DLLPUBLIC NumFormatListBox : public ListBox
{
    short               nCurrFormatType;
    USHORT              nStdEntry;
    BOOL                bOneArea;
    ULONG               nDefFormat;
    SwView*             pVw;
    SvNumberFormatter*  pOwnFormatter;
    LanguageType        eCurLanguage;
    BOOL                bShowLanguageControl; //determine whether the language control has
                                              //to be shown in the number format dialog
    BOOL                bUseAutomaticLanguage;//determine whether language is automatically assigned

    SW_DLLPRIVATE DECL_LINK( SelectHdl, ListBox * );

    SW_DLLPRIVATE double          GetDefValue(const short nFormatType) const;
    SW_DLLPRIVATE void            Init(short nFormatType, BOOL bUsrFmts);
    SW_DLLPRIVATE SwView*           GetView();

public:
    NumFormatListBox( Window* pWin, const ResId& rResId,
        short nFormatType = NUMBERFORMAT_NUMBER, ULONG nDefFmt = 0,
        BOOL bUsrFmts = TRUE );

    NumFormatListBox( Window* pWin, SwView* pView, const ResId& rResId,
        short nFormatType = NUMBERFORMAT_NUMBER, ULONG nDefFmt = 0,
        BOOL bUsrFmts = TRUE );

    ~NumFormatListBox();

    void            Clear();

    inline void     SetOneArea(BOOL bOnlyOne = TRUE) { bOneArea = bOnlyOne; }

    void            SetFormatType(const short nFormatType);
    inline short    GetFormatType() const { return nCurrFormatType; }
    void            SetDefFormat(const ULONG nDefFmt);
    ULONG           GetFormat() const;

    inline LanguageType GetCurLanguage() const { return eCurLanguage;}
    void                SetLanguage(LanguageType eSet)  { eCurLanguage = eSet;}

    void            SetAutomaticLanguage(BOOL bSet){bUseAutomaticLanguage = bSet;}
    BOOL            IsAutomaticLanguage()const {return bUseAutomaticLanguage;}

    void            SetShowLanguageControl(BOOL bSet){bShowLanguageControl = bSet;}

};


#endif

