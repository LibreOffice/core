/*************************************************************************
 *
 *  $RCSfile: sdmod.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-28 17:54:27 $
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

#ifndef _SDMOD_HXX
#define _SDMOD_HXX


#ifndef _SDDLL_HXX
#define _SD_DLL             // fuer SD_MOD()
#include "sddll.hxx"        // fuer SdModuleDummy
#endif

#ifndef _SD_GLOB_HXX
#include "glob.hxx"
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

class SdOptions;
class BasicIDE;
class SvxSearchItem;
class SdAppLinkHdl; ;
class SvxErrorHandler;
class SdDataObject;
class EditFieldInfo;
class SvFactory;

// ----------------------
// - SdOptionStreamMode -
// ----------------------

enum SdOptionStreamMode
{
    SD_OPTION_LOAD = 0,
    SD_OPTION_STORE = 1
};

/*************************************************************************
|*
|* This subclass of <SfxModule> (which is a subclass of <SfxShell>) is
|* linked to the DLL. One instance of this class exists while the DLL is
|* loaded.
|*
|* SdModule is like to be compared with the <SfxApplication>-subclass.
|*
|* Remember: Don`t export this class! It uses DLL-internal symbols.
|*
\************************************************************************/

class SdModule : public SdModuleDummy, public SfxListener
{
protected:
    SdOptions*          pImpressOptions;
    SdOptions*          pDrawOptions;
    SvxSearchItem*      pSearchItem;
    SvStorageRef        xOptionStorage;

    BOOL                bAutoSave;
    BOOL                bWaterCan;

    virtual BOOL        QueryUnload();
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDAPP);

                        SdModule(SvFactory* pDrawObjFact, SvFactory* pGraphicObjFact);
    virtual             ~SdModule();

    SdDataObject*       pDragData;
    SdDataObject*       pClipboardData;

    void                Execute(SfxRequest& rReq);
    void                GetState(SfxItemSet&);

    virtual SfxModule*  Load();
    virtual void        Free();

    virtual void        FillStatusBar(StatusBar& rBar);
    virtual SfxFileDialog*  CreateDocFileDialog( ULONG nBits,
                                                 const SfxObjectFactory& rFact,
                                                 const SfxItemSet* pSet );

    SdOptions*          GetSdOptions(DocumentType eDocType);

    SvStorageStreamRef  GetOptionStream( const String& rOptionName,
                                         SdOptionStreamMode eMode );

    BOOL                GetWaterCan() const { return bWaterCan; }
    void                SetWaterCan( BOOL bWC ) { bWaterCan = bWC; }

    SvxSearchItem*      GetSearchItem() { return (pSearchItem); }
    void                SetSearchItem(SvxSearchItem* pItem) { pSearchItem = pItem; }

    DECL_LINK(CalcFieldValueHdl, EditFieldInfo*);

    //virtuelle Methoden fuer den Optionendialog
    virtual SfxItemSet*  CreateItemSet( USHORT nId );
    virtual void         ApplyItemSet( USHORT nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet );
};




#ifndef SD_MOD
#define SD_MOD() ( *(SdModule**) GetAppData(SHL_DRAW) )
#endif

#endif                                 // _SDMOD_HXX

