/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docstdlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:17:55 $
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
#ifndef _DOCSTDLG_HXX
#define _DOCSTDLG_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#include "docstat.hxx"

/*--------------------------------------------------------------------
    Beschreibung:   DocInfo jetzt als Page
 --------------------------------------------------------------------*/

class SwDocStatPage: public SfxTabPage
{
public:
    SwDocStatPage(Window *pParent, const SfxItemSet &rSet);
    ~SwDocStatPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);

protected:
    virtual BOOL    FillItemSet(      SfxItemSet &rSet);
    virtual void    Reset      (const SfxItemSet &rSet);

    DECL_LINK( UpdateHdl, PushButton*);

private:
    FixedText       aTableLbl;
    FixedText       aGrfLbl;
    FixedText       aOLELbl;
    FixedText       aPageLbl;
    FixedText       aParaLbl;
    FixedText       aWordLbl;
    FixedText       aCharLbl;
    FixedText       aLineLbl;

    FixedInfo       aTableNo;
    FixedInfo       aGrfNo;
    FixedInfo       aOLENo;
    FixedInfo       aPageNo;
    FixedInfo       aParaNo;
    FixedInfo       aWordNo;
    FixedInfo       aCharNo;
    FixedInfo       aLineNo;

    PushButton      aUpdatePB;
    SwDocStat       aDocStat;

    void            Update();

    using Window::SetData;
    void            SetData(const SwDocStat &rStat);
};

#endif
