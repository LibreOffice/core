/*************************************************************************
 *
 *  $RCSfile: pgfnote.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ma $ $Date: 2001-03-21 17:39:23 $
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
#ifndef _PGFNOTE_HXX
#define _PGFNOTE_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifdef VCL
#ifndef _CTRLBOX_HXX
#include <svtools/ctrlbox.hxx>
#endif
#else
#include "linbox.hxx"
#endif

/*--------------------------------------------------------------------
    Beschreibung:   Fussnoteneinstellungs-TabPage
 --------------------------------------------------------------------*/

class SwFootNotePage: public SfxTabPage
{
public:
    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static USHORT* GetRanges();

    virtual BOOL FillItemSet(SfxItemSet &rSet);
    virtual void Reset(const SfxItemSet &rSet);

private:
    SwFootNotePage(Window *pParent, const SfxItemSet &rSet);
    ~SwFootNotePage();

    RadioButton     aMaxHeightPageBtn;
    RadioButton     aMaxHeightBtn;
    MetricField     aMaxHeightEdit;
    FixedText       aDistLbl;
    MetricField     aDistEdit;
    FixedLine       aPosHeader;

    FixedText       aLinePosLbl;
    ListBox         aLinePosBox;
    FixedText       aLineTypeLbl;
    LineListBox     aLineTypeBox;
    FixedText       aLineWidthLbl;
    MetricField     aLineWidthEdit;
    FixedText       aLineDistLbl;
    MetricField     aLineDistEdit;
    FixedLine       aLineHeader;

    DECL_LINK( HeightPage, Button * );
    DECL_LINK( HeightMetric, Button * );
    DECL_LINK( HeightModify, MetricField * );

    long            lMaxHeight;

    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

};

#endif
