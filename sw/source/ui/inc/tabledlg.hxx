/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabledlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:07:26 $
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
#ifndef _TABLEDLG_HXX
#define _TABLEDLG_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif


#include "swtypes.hxx"
#include "swtablerep.hxx" //CHINA001

class SwWrtShell;
class SwTabCols;
struct TColumn;

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

//CHINA001 class SwTableRep
//CHINA001 {
//CHINA001 TColumn*     pTColumns;
//CHINA001
//CHINA001 SwTwips      nTblWidth;
//CHINA001 SwTwips      nSpace;
//CHINA001 SwTwips      nLeftSpace;
//CHINA001 SwTwips  nRightSpace;
//CHINA001 USHORT       nAlign;
//CHINA001 USHORT       nColCount;
//CHINA001 USHORT       nAllCols;
//CHINA001 USHORT       nWidthPercent;
//CHINA001 BOOL         bComplex : 1;
//CHINA001 BOOL         bLineSelected : 1;
//CHINA001 BOOL     bWidthChanged : 1;
//CHINA001 BOOL     bColsChanged : 1;
//CHINA001
//CHINA001 public:
//CHINA001 SwTableRep( const SwTabCols& rTabCol, BOOL bComplex );
//CHINA001 ~SwTableRep();
//CHINA001
//CHINA001 BOOL     FillTabCols( SwTabCols& rTabCol ) const;
//CHINA001
//CHINA001 SwTwips  GetLeftSpace() const            {return nLeftSpace;}
//CHINA001 void         SetLeftSpace(SwTwips nSet)      {nLeftSpace = nSet;}
//CHINA001
//CHINA001 SwTwips      GetRightSpace() const           {return nRightSpace;}
//CHINA001 void         SetRightSpace(SwTwips nSet)     {nRightSpace = nSet;}
//CHINA001
//CHINA001 SwTwips      GetWidth() const                {return nTblWidth;}
//CHINA001 void         SetWidth(SwTwips nSet)          {nTblWidth = nSet;}
//CHINA001
//CHINA001 USHORT       GetWidthPercent() const         {return nWidthPercent;}
//CHINA001 void         SetWidthPercent(USHORT nSet)    {nWidthPercent = nSet;}
//CHINA001
//CHINA001 USHORT       GetAlign() const                {return nAlign;}
//CHINA001 void         SetAlign(USHORT nSet)           {nAlign = nSet;}
//CHINA001
//CHINA001 BOOL     IsComplex() const               {return bComplex;}
//CHINA001 USHORT       GetColCount() const             {return nColCount;}
//CHINA001 USHORT       GetAllColCount() const          {return nAllCols;}
//CHINA001
//CHINA001 BOOL     HasColsChanged() const          {return bColsChanged;}
//CHINA001 void     SetColsChanged()                {bColsChanged = TRUE;}
//CHINA001
//CHINA001 BOOL     HasWidthChanged() const         {return bWidthChanged;}
//CHINA001 void         SetWidthChanged()               {bWidthChanged  = TRUE;}
//CHINA001
//CHINA001 BOOL         IsLineSelected() const          {return bLineSelected;}
//CHINA001 void     SetLineSelected(BOOL bSet)      {bLineSelected = bSet;}
//CHINA001
//CHINA001 SwTwips      GetSpace() const                { return nSpace;}
//CHINA001 void         SetSpace(SwTwips nSet)          {nSpace = nSet;}
//CHINA001
//CHINA001 TColumn*     GetColumns() const              {return pTColumns;}
//CHINA001 };

#endif
