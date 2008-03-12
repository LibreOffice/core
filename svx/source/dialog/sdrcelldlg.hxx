/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrcelldlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:40:43 $
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

#ifndef _SVX_FORMAT_CELLS_DLG_HXX
#define _SVX_FORMAT_CELLS_DLG_HXX


#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

class XColorTable;
class XGradientList;
class XHatchList;
class XBitmapList;
class SdrModel;

class SvxFormatCellsDialog : public SfxTabDialog
{
private:
    const SfxItemSet&   mrOutAttrs;

    XColorTable*        mpColorTab;
    XGradientList*      mpGradientList;
    XHatchList*         mpHatchingList;
    XBitmapList*        mpBitmapList;

protected:
    virtual void Apply();

public:
    SvxFormatCellsDialog( Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel );
    ~SvxFormatCellsDialog();

    virtual void PageCreated( USHORT nId, SfxTabPage &rPage );

};

#endif // _SVX_FORMAT_CELLS_DLG_HXX


