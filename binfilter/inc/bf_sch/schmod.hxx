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

#ifndef _SCHMOD_HXX
#define _SCHMOD_HXX

#include <bf_svtools/lstner.hxx>

#include "schdll0.hxx"        // fuer SchModuleDummy
#include "schdll.hxx"
#include "objfac.hxx"

#include "glob.hxx"
#include "schopt.hxx"
class SvFactory;
namespace binfilter {

class SvxErrorHandler;
class E3dObjFactory;
class SchTransferable;

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

class SchModule : public SchModuleDummy, public SfxListener
{
protected:
    XOutdevItemPool*	pXOutDevPool;

    SchDragServer*		pDragData;
    SchDragServer*		pClipboardData;
    // new clipborad format
    SchTransferable*	mpTransferDragDrop;
    SchTransferable*	mpTransferClipboard;
    SchTransferable*	mpTransferSelectionClipbd;

    SchObjFactory*		pSchObjFactory;
    E3dObjFactory*		pE3dFactory;
    SchOptions*			pChartOptions;

public:
    TYPEINFO();

    SchModule(SvFactory* pObjFact);
    virtual ~SchModule();

    SchDragServer *GetDragData()							{ return pDragData; }
    void SetDragData(SchDragServer* pData)					{ pDragData = pData; }

    SchDragServer *GetClipboardData()						{ return pClipboardData; }
    void SetClipboardData(SchDragServer* pData)				{ pClipboardData = pData; }

    // new clipboard format
    SchTransferable* GetDragTransferable()					{ return mpTransferDragDrop; }
    void SetDragTransferable( SchTransferable* pTrans )		{ mpTransferDragDrop = pTrans; }

    SchTransferable* GetClipboardTransferable()				{ return mpTransferClipboard; }
    void SetClipboardTransferable( SchTransferable* pTrans) { mpTransferClipboard = pTrans; }

    SchTransferable* GetSelectionClipboardTransferable()	{ return mpTransferSelectionClipbd; }
    void SetSelectionClipboardTransferable( SchTransferable* pTrans) { mpTransferSelectionClipbd = pTrans; }

    SchOptions*          GetSchOptions();

    // virtual methods for options dialog (impl. see appopt.cxx)

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};




#ifndef SCH_MOD1
#define SCH_MOD1() ( *(SchModule**) GetAppData(BF_SHL_SCH) )
#endif

} //namespace binfilter
#endif                                 // _SCHMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
