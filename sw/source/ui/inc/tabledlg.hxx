/*************************************************************************
 *
 *  $RCSfile: tabledlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:42 $
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
#ifndef _TABLEDLG_HXX
#define _TABLEDLG_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#include "swtypes.hxx"

class SwWrtShell;
class SwTabCols;
struct TColumn;

void lcl_ItemSetToTableParam( const SfxItemSet&, SwWrtShell& );
extern const USHORT __FAR_DATA aUITableAttrRange[];

/*-------------------------------------------------------
 Tabellendialog
--------------------------------------------------------- */
class SwTableTabDlg : public SfxTabDialog
{
    SwWrtShell* pShell;
    USHORT      nHtmlMode;

    virtual void        PageCreated(USHORT nId, SfxTabPage& rPage);
public:
     SwTableTabDlg( Window* pParent, SfxItemPool& Pool,
                    const SfxItemSet* pItemSet, SwWrtShell* pSh );
};

class SwTableRep
{
    TColumn*    pTColumns;

    SwTwips     nTblWidth;
    SwTwips     nSpace;
    SwTwips     nLeftSpace;
    SwTwips     nRightSpace;
    USHORT      nAlign;
    USHORT      nColCount;
    USHORT      nAllCols;
    USHORT      nWidthPercent;
    BOOL        bComplex : 1;
    BOOL        bLineSelected : 1;
    BOOL        bWidthChanged : 1;
    BOOL        bColsChanged : 1;

public:
    SwTableRep( const SwTabCols& rTabCol, BOOL bComplex );
    ~SwTableRep();

    BOOL        FillTabCols( SwTabCols& rTabCol ) const;

    SwTwips     GetLeftSpace() const            {return nLeftSpace;}
    void        SetLeftSpace(SwTwips nSet)      {nLeftSpace = nSet;}

    SwTwips     GetRightSpace() const           {return nRightSpace;}
    void        SetRightSpace(SwTwips nSet)     {nRightSpace = nSet;}

    SwTwips     GetWidth() const                {return nTblWidth;}
    void        SetWidth(SwTwips nSet)          {nTblWidth = nSet;}

    USHORT      GetWidthPercent() const         {return nWidthPercent;}
    void        SetWidthPercent(USHORT nSet)    {nWidthPercent = nSet;}

    USHORT      GetAlign() const                {return nAlign;}
    void        SetAlign(USHORT nSet)           {nAlign = nSet;}

    BOOL        IsComplex() const               {return bComplex;}
    USHORT      GetColCount() const             {return nColCount;}
    USHORT      GetAllColCount() const          {return nAllCols;}

    BOOL        HasColsChanged() const          {return bColsChanged;}
    void        SetColsChanged()                {bColsChanged = TRUE;}

    BOOL        HasWidthChanged() const         {return bWidthChanged;}
    void        SetWidthChanged()               {bWidthChanged  = TRUE;}

    BOOL        IsLineSelected() const          {return bLineSelected;}
    void        SetLineSelected(BOOL bSet)      {bLineSelected = bSet;}

    SwTwips     GetSpace() const                { return nSpace;}
    void        SetSpace(SwTwips nSet)          {nSpace = nSet;}

    TColumn*    GetColumns() const              {return pTColumns;}
};

#endif
