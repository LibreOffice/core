/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: celltempl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:43:46 $
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

#ifndef SD_CELLTEMPL_HXX
#define SD_CELLTEMPL_HXX

#include <sfx2/styledlg.hxx>

class XColorTable;
class XGradientList;
class XHatchList;
class XBitmapList;
class SfxStyleSheetBase;
class SdrModel;

class SdPresCellTemplateDlg : public SfxStyleDialog
{
private:
    XColorTable*        mpColorTab;
    XGradientList*      mpGradientList;
    XHatchList*         mpHatchingList;
    XBitmapList*        mpBitmapList;

    virtual void                PageCreated( USHORT nId, SfxTabPage &rPage );
    virtual const SfxItemSet*   GetRefreshedSet();

public:
    SdPresCellTemplateDlg( SdrModel* pModel, Window* pParent, SfxStyleSheetBase& rStyleBase );
    ~SdPresCellTemplateDlg();

    const SfxItemSet* GetOutputItemSet() const;
};


#endif // SD_CELLTEMPL_HXX
