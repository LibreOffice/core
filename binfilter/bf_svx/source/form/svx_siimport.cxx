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

#ifndef _SVX_SIIMPORT_HXX
#include "siimport.hxx"
#endif

#ifndef _SVDORECT_HXX
#include "svdorect.hxx"
#endif

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _SVX_XFLCLIT_HXX
#include "xflclit.hxx"
#endif

#ifndef _SVX_XLNCLIT_HXX
#include "xlnclit.hxx"
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif
namespace binfilter {


//============================================================================
// defines
//============================================================================

const UINT32 SiInventor = UINT32('S')*0x00000001+
                          UINT32('D')*0x00000100+
                          UINT32('V')*0x00010000+
                          UINT32('C')*0x01000000;

#define OBJ_CHECKBOX  			((UINT16)0)
#define OBJ_RADIOBUTTON         ((UINT16)1)
#define OBJ_PUSHBUTTON          ((UINT16)2)
#define OBJ_SPINBUTTON          ((UINT16)3)
#define OBJ_FIXEDTEXT           ((UINT16)4)
#define OBJ_GROUPBOX            ((UINT16)5)
#define OBJ_LISTBOX             ((UINT16)6)
#define OBJ_COMBOBOX            ((UINT16)7)
#define OBJ_EDIT                ((UINT16)8)
#define OBJ_HSCROLLBAR          ((UINT16)9)
#define OBJ_VSCROLLBAR          ((UINT16)10)

#define OBJ_URLBUTTON           ((UINT16)25)


//============================================================================
// SiImportRect
//============================================================================

class SiImportRect : public SdrRectObj
{
public:
    SiImportRect();

    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);
};

//----------------------------------------------------------------------------

/*N*/ SiImportRect::SiImportRect() : SdrRectObj()
/*N*/ {
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ void SiImportRect::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	// SdrRectObj ueberspringen!!!!
/*N*/ 	SdrObject::ReadData( rHead, rIn );
/*N*/ 	SdrDownCompat aCompat( rIn, STREAM_READ );
/*N*/ 	aRect = aOutRect;
/*N*/ 	SfxItemSet aSet(pModel->GetItemPool());
/*N*/ 	aSet.Put(XFillStyleItem(XFILL_SOLID));
/*N*/ 	aSet.Put(XFillColorItem(String(), RGB_Color(COL_LIGHTRED)));
/*N*/ 	aSet.Put(XLineStyleItem(XLINE_SOLID));
/*N*/ 	aSet.Put(XLineColorItem(String(), RGB_Color(COL_BLACK)));
/*N*/ 
/*N*/     SetItemSet(aSet);
/*N*/ 	
/*N*/ 	SetXPolyDirty();
/*N*/ }


//============================================================================
// SiImportFactory
//============================================================================

/*N*/ SiImportFactory::SiImportFactory()
/*N*/ {
/*N*/ 	SdrObjFactory::InsertMakeObjectHdl( LINK(this, SiImportFactory, MakeObject) );
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ SiImportFactory::~SiImportFactory()
/*N*/ {
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ IMPL_LINK( SiImportFactory, MakeObject, SdrObjFactory *, pObjFactory )
/*N*/ {
/*N*/ 	if( pObjFactory->nInventor == SiInventor )
/*N*/ 	{
/*N*/ 		if( ((pObjFactory->nIdentifier >= OBJ_CHECKBOX) && (pObjFactory->nIdentifier <= OBJ_VSCROLLBAR)) ||
/*N*/ 			pObjFactory->nIdentifier == OBJ_URLBUTTON )
/*N*/ 		{
/*N*/             switch( pObjFactory->nIdentifier )
/*N*/ 			{
/*N*/ 				case OBJ_CHECKBOX:
/*N*/ 				case OBJ_RADIOBUTTON:
/*N*/ 				case OBJ_PUSHBUTTON:
/*N*/ 				case OBJ_SPINBUTTON:
/*N*/ 				case OBJ_FIXEDTEXT:
/*N*/ 				case OBJ_GROUPBOX:
/*N*/ 				case OBJ_LISTBOX:
/*N*/ 				case OBJ_COMBOBOX:
/*N*/ 				case OBJ_EDIT:
/*N*/ 				case OBJ_HSCROLLBAR:
/*N*/ 				case OBJ_VSCROLLBAR:
/*N*/ 				case OBJ_URLBUTTON:
/*N*/ 					pObjFactory->pNewObj = new SiImportRect();
/*N*/ 			}
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/ 	return 0;
/*N*/ }

//----------------------------------------------------------------------------



}
