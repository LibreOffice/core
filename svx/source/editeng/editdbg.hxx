/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editdbg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-06-19 15:57:32 $
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

#ifndef _EDITDBG_HXX
#define _EDITDBG_HXX

#include <svtools/solar.hrc>
#include <tools/string.hxx>
#include <stdio.h>

class EditEngine;
class ParaPortion;
class EditUndoList;
class TextPortionList;
class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;

ByteString  DbgOutItem( const SfxItemPool& rPool, const SfxPoolItem& rItem );
void        DbgOutItemSet( FILE* fp, const SfxItemSet& rSet, BOOL bSearchInParent, BOOL bShowALL );

class SVX_DLLPUBLIC EditDbg
{
public:
    static void         ShowEditEngineData( EditEngine* pEditEngine, BOOL bInfoBox = TRUE );
    static void         ShowPortionData( ParaPortion* pPortion );
    static ByteString   GetPortionInfo( ParaPortion* pPPortion );
    static ByteString   GetTextPortionInfo( TextPortionList& rPortions );
    static ByteString   GetUndoDebStr( EditUndoList* pUndoList );
};


#endif // _EDITDBG_HXX
