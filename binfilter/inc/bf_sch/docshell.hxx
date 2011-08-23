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

#ifndef _SCH_DOCSHELL_HXX
#define _SCH_DOCSHELL_HXX

#include "schdll0.hxx"

#include <bf_sfx2/docfac.hxx>

#include <bf_sfx2/shell.hxx>

#include <bf_svx/flstitem.hxx>

#include <bf_sfx2/objsh.hxx>
#include <bf_sfx2/interno.hxx>
#include <schdll0.hxx>
#include <bf_sfx2/request.hxx>

#include <bf_so3/svstor.hxx>

#include "chtmodel.hxx"

#include <bf_sfx2/docfile.hxx>

#include "glob.hxx"

#include <com/sun/star/chart/ChartDataChangeEvent.hpp>
namespace binfilter {

#ifndef SO2_DECL_SVSTORAGESTREAM_DEFINED
#define SO2_DECL_SVSTORAGESTREAM_DEFINED
SO2_DECL_REF(SvStorageStream)
#endif

class FontList;
class SfxUndoManager;
class SfxStyleSheetBasePool;
class SdStyleSheetPool;
class SfxProgress;
class SfxPrinter;
class SfxMenuBarManager;

/*************************************************************************
|*
|* Document-Shell fuer Draw-Dokumente
|*
\************************************************************************/

class SchChartDocShell: public SfxObjectShell, public SfxInPlaceObject
{
 private:
    ChartModel*			pChDoc;
    SfxUndoManager* 	pUndoManager;
    SfxPrinter* 		pPrinter;
    FontList*			pFontList;
    SfxProgress*		pProgress;
    BOOL 				bInitNewNoNewDoc;
    BOOL 				bOwnPrinter;
    BOOL				mbClipboardExport;

    void Construct() throw();

     // #61907# bei deaktivierung des InPlaceEdit evtl. gebufferte neue Daten
    // übernehmen
 protected:

 public:
    TYPEINFO();

    SFX_DECL_OBJECTFACTORY_DLL(SchChartDocShell,SCH_MOD());

    SchChartDocShell(SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED) throw();
    ~SchChartDocShell() throw();


    virtual BOOL InitNew(SvStorage*) throw();
    virtual BOOL Load(SvStorage*) throw();
    virtual BOOL Save() throw();
    virtual BOOL SaveAs(SvStorage* pNewStor) throw();
    virtual BOOL SaveCompleted(SvStorage* pNewStor) throw();

    virtual Rectangle GetVisArea(USHORT nAspect) const throw();
    virtual void SetVisArea(const Rectangle& rRect) throw();


    virtual SfxUndoManager* GetUndoManager() throw();

    SfxPrinter* GetPrinter() throw();
    void SetPrinter( SfxPrinter *pNewPrinter, BOOL bIsDeletedHere = FALSE ) throw();

    virtual void     OnDocumentPrinterChanged(Printer* pNewPrinter) throw();

    /** @return if the chart is embedded in a document, this may be a virtual
                device for layout, or a printer, according to what is set in the
                container.  Otherwise GetPrinter() is called.
     */
    OutputDevice * GetRefDevice();

    ChartModel& GetDoc() { return *pChDoc; }
    ChartModel* GetModelPtr() { return pChDoc; }
    void SetModelPtr( ChartModel* pModel ) { pChDoc = pModel; }

    virtual SfxStyleSheetBasePool* GetStyleSheetPool() throw();

    void UpdateTablePointers() throw();

    virtual void FillClass(SvGlobalName* pClassName,
                           ULONG*  pFormat,
                           String* pAppName,
                           String* pFullTypeName,
                           String* pShortTypeName,
                           long    nFileFormat = SOFFICE_FILEFORMAT_CURRENT ) const throw();

    virtual ULONG GetMiscStatus() const throw();

    virtual void    SetModified( BOOL = TRUE ) throw();

    void DataModified( ::com::sun::star::chart::ChartDataChangeEvent& ) throw();

    /// if this flag is set the XML export always exports the table
    void SetClipboardExport( BOOL bSet = sal_False )
        { mbClipboardExport = bSet; }
    BOOL GetClipboardExport() const
        { return mbClipboardExport; }
    
};

SO2_DECL_IMPL_REF(SchChartDocShell)

} //namespace binfilter
#endif		// _SCH_DOCSHELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
