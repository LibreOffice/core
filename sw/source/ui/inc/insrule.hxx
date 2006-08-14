/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: insrule.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:42:28 $
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
#ifndef _INSRULE_HXX
#define _INSRULE_HXX
#ifndef _NUM_HXX
#include "num.hxx"
#endif

class SwRulerValueSet;
class ValueSet;
/*-----------------14.02.97 12.30-------------------

--------------------------------------------------*/
class SwInsertGrfRulerDlg  : public SfxModalDialog
{
    FixedLine       aSelectionFL;
    OKButton        aOkPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;

    List            aGrfNames;
    String          sSimple;
    String          sRulers;
    USHORT          nSelPos;

    SwRulerValueSet* pExampleVS;

protected:
    DECL_LINK(SelectHdl, ValueSet*);
    DECL_LINK(DoubleClickHdl, ValueSet*);

public:
    SwInsertGrfRulerDlg( Window* pParent );
    ~SwInsertGrfRulerDlg();

    String          GetGraphicName();
    BOOL            IsSimpleLine() {return nSelPos == 1;}
    BOOL            HasImages() const {return 0 != aGrfNames.Count();}
};

#endif



