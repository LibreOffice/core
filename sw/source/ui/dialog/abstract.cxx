/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: abstract.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:36:17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include "swtypes.hxx"
#include "hintids.hxx"

#include "dialog.hrc"
#include "abstract.hxx"
#include "abstract.hrc"


/*-----------------22.02.97 15.32-------------------

--------------------------------------------------*/

SwInsertAbstractDlg::SwInsertAbstractDlg( Window* pParent ) :
    SfxModalDialog(pParent, SW_RES(DLG_INSERT_ABSTRACT)),
    aFL     (this, SW_RES(FL_1       )),
    aLevelFT(this, SW_RES(FT_LEVEL   )),
    aLevelNF(this, SW_RES(NF_LEVEL   )),
    aParaFT (this, SW_RES(FT_PARA   )),
    aParaNF (this, SW_RES(NF_PARA   )),
    aDescFT (this, SW_RES(FT_DESC   )),
    aOkPB   (this, SW_RES(PB_OK     )),
    aCancelPB (this, SW_RES(PB_CANCEL   )),
    aHelpPB (this, SW_RES(PB_HELP   ))
{
    FreeResource();
}

/*-----------------22.02.97 15.32-------------------

--------------------------------------------------*/

SwInsertAbstractDlg::~SwInsertAbstractDlg()
{
}

/*-----------------22.02.97 15.34-------------------

--------------------------------------------------*/

BYTE SwInsertAbstractDlg::GetLevel() const
{
    return static_cast< BYTE >(aLevelNF.GetValue() - 1);
}
/*-----------------22.02.97 15.34-------------------

--------------------------------------------------*/

BYTE SwInsertAbstractDlg::GetPara() const
{
    return (BYTE) aParaNF.GetValue();
}






