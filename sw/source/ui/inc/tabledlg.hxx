/*************************************************************************
 *
 *  $RCSfile: tabledlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:31:35 $
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
#include "swtablerep.hxx" //CHINA001
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
