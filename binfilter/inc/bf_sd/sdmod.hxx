/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SDMOD_HXX
#define _SDMOD_HXX


#ifndef _SDDLL_HXX
#define _SD_DLL             // fuer SD_MOD()
#include "sddll.hxx"        // fuer SdModuleDummy
#endif
#include "glob.hxx"
#include "pres.hxx"
#include <bf_so3/svstor.hxx>
#include <bf_svtools/lstner.hxx>
#include <com/sun/star/text/WritingMode.hpp>
class SvFactory;
class OutputDevice;

namespace binfilter {

class SfxErrorHandler;
class SvNumberFormatter;
class SdOptions;
class BasicIDE;
class SdAppLinkHdl; //STRIP008 ;
class SvxErrorHandler;
class EditFieldInfo;
class SdDrawDocShell;
class SdView;
class SdPage;
class SdDrawDocument;
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

    SdOptions*				pImpressOptions;
    SdOptions*				pDrawOptions;
    SvStorageRef			xOptionStorage;
    BOOL					bAutoSave;
    SfxErrorHandler*		mpErrorHdl;
    OutputDevice*           mpVirtualRefDevice;

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

public:

                            TYPEINFO();
                            DECL_LINK( CalcFieldValueHdl, EditFieldInfo* );

                            SdModule(SvFactory* pDrawObjFact, SvFactory* pGraphicObjFact);
    virtual					~SdModule();

    virtual SfxModule*		Load();
    virtual void			Free();


    SdOptions*				GetSdOptions(DocumentType eDocType);

    OutputDevice* GetVirtualRefDevice (void);

    OutputDevice* GetRefDevice (SdDrawDocShell& rDocShell);
    ::com::sun::star::text::WritingMode GetDefaultWritingMode() const;
};




#ifndef SD_MOD
#define SD_MOD() ( *(SdModule**) GetAppData(BF_SHL_DRAW) )
#endif

} //namespace binfilter
#endif                                 // _SDMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
