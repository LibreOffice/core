/*************************************************************************
 *
 *  $RCSfile: smmod.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:24 $
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

#ifndef _SMMOD_HXX
#define _SMMOD_HXX


#ifndef _SMDLL_HXX
#define _SM_DLL             // fuer SD_MOD()
#include "smdll.hxx"        // fuer SdModuleDummy
#endif

class SvxErrorHandler;
class SvFactory;

class SmConfig;
class SmSymSetManager;
class SmModule;

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

class SmRectCache;

class SmModule : public SmModuleDummy
{
    SmConfig            *pConfig;
    SmSymSetManager     *pSymSetManager;
    SmRectCache         *pRectCache;

    virtual void FillStatusBar(StatusBar &rBar);

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START + 0);

    SmModule(SvFactory* pObjFact);
    virtual ~SmModule();

    virtual SfxModule * Load();
    virtual void        Free();

    SmConfig *          GetConfig()        { return pConfig; }
    SmSymSetManager *   GetSymSetManager() { return pSymSetManager; }
    SmRectCache *       GetRectCache()     { return pRectCache; }

    void GetState(SfxItemSet&);
    void InitManager();

    //virtuelle Methoden fuer den Optionendialog
    virtual SfxItemSet*  CreateItemSet( USHORT nId );
    virtual void         ApplyItemSet( USHORT nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet );
};


#define SM_MOD1() ( *(SmModule**) GetAppData(SHL_SM) )


#endif                                 // _SDMOD_HXX

