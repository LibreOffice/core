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

#ifndef DOCUMENT_HXX
#define DOCUMENT_HXX

#define SMDLL   1

#include <sot/storage.hxx>
#include <sot/sotref.hxx>
#include <sfx2/objsh.hxx>
#include <svl/lstner.hxx>
#include <sfx2/docfac.hxx>
#include <vcl/virdev.hxx>

#include "format.hxx"
#include "parse.hxx"
#include "smmod.hxx"

#include <vcl/jobset.hxx>

class SmNode;
class SfxMenuBarManager;
class SfxPrinter;
class Printer;
class SmCursor;

#define HINT_DATACHANGED    1004

#define SM30BIDENT   ((ULONG)0x534D3033L)
#define SM30IDENT    ((ULONG)0x30334d53L)
#define SM304AIDENT  ((ULONG)0x34303330L)
#define SM30VERSION  ((ULONG)0x00010000L)
#define SM50VERSION  ((ULONG)0x00010001L)   //Difference to SM30VERSION is
                                            //the new border in the format.

#define FRMIDENT    ((ULONG)0x03031963L)
#define FRMVERSION  ((ULONG)0x00010001L)

#define STAROFFICE_XML  "StarOffice XML (Math)"
#define MATHML_XML      "MathML XML (Math)"

/* Access to printer should happen through this class only
 * ==========================================================================
 *
 * The printer can belong to the document or the OLE-Container. If the document
 * is an OLE-Document the printer generally belongs to the container too.
 * But the container mayby works with a different MapUnit than the server.
 * Referring to the MapMode the printer will be accordingly adjusted in the
 * constructor and restored in the destructor. This brings that this class
 * is always allowed to exists only a short time (e.g. while painting).
 * The control whether the printer is self-generated, gotten from the server
 * or is NULL then, is taken by the DocShell in the method GetPrt(), for
 * which the access is friend of the DocShell too.
 */

class SmDocShell;
class EditEngine;

////////////////////////////////////////////////////////////

class SmPrinterAccess
{
    Printer* pPrinter;
    OutputDevice* pRefDev;
public:
    SmPrinterAccess( SmDocShell &rDocShell );
    ~SmPrinterAccess();
    Printer* GetPrinter()  { return pPrinter; }
    OutputDevice* GetRefDev()  { return pRefDev; }
};


////////////////////////////////////////////////////////////

void SetEditEngineDefaultFonts(
        EditEngine &rEditEngine,
        SfxItemPool &rEditEngineItemPool );

////////////////////////////////////////////////////////////

class SmDocShell : public SfxObjectShell, public SfxListener
{
    friend class SmPrinterAccess;
    friend class SmModel;
    friend class SmCursor;

    String              aText;
    SmFormat            aFormat;
    SmParser            aInterpreter;
    String              aAccText;
    SmNode             *pTree;
    SfxMenuBarManager  *pMenuMgr;
    SfxItemPool        *pEditEngineItemPool;
    EditEngine         *pEditEngine;
    SfxPrinter         *pPrinter;       //q.v. comment to SmPrinter Access!
    Printer            *pTmpPrinter;    //ditto
    long                nLeftBorder,
                        nRightBorder,
                        nTopBorder,
                        nBottomBorder;
    USHORT              nModifyCount;
    bool                bIsFormulaArranged;
    SmCursor           *pCursor;



    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                        const SfxHint& rHint, const TypeId& rHintType);

    bool        WriteAsMathType3( SfxMedium& );

    virtual void        Draw(OutputDevice *pDevice,
                             const JobSetup & rSetup,
                             USHORT nAspect = ASPECT_CONTENT);

    virtual void        FillClass(SvGlobalName* pClassName,
                                  sal_uInt32*  pFormat,
                                  String* pAppName,
                                  String* pFullTypeName,
                                  String* pShortTypeName,
                                  sal_Int32 nFileFormat,
                                  sal_Bool bTemplate = sal_False ) const;

    virtual BOOL        SetData( const String& rData );
    virtual ULONG       GetMiscStatus() const;
    virtual void        OnDocumentPrinterChanged( Printer * );
    virtual sal_Bool    InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    virtual BOOL        Load( SfxMedium& rMedium );
            void        ImplSave(  SvStorageStreamRef xStrm  );
    virtual BOOL        Save();
    virtual BOOL        SaveAs( SfxMedium& rMedium );
    virtual BOOL        ConvertTo( SfxMedium &rMedium );
    virtual BOOL        SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    Printer             *GetPrt();
    OutputDevice*       GetRefDev();

    bool                IsFormulaArranged() const { return bIsFormulaArranged; }
    void                SetFormulaArranged(bool bVal) { bIsFormulaArranged = bVal; }

    virtual BOOL        ConvertFrom(SfxMedium &rMedium);

    /** Called whenever the formula is changed
     * Deletes the current cursor
     */
    void                InvalidateCursor();

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START+1)
    using SotObject::GetInterface;
    SFX_DECL_OBJECTFACTORY();

                SmDocShell( const sal_uInt64 i_nSfxCreationFlags );
    virtual     ~SmDocShell();

    void        LoadSymbols();
    void        SaveSymbols();

    void        ArrangeFormula();

    //Access for the View. This access is not for the OLE-case!
    //and for the communication with the SFX!
    //All internal printer uses should work with the SmPrinterAccess only
    bool        HasPrinter()    { return 0 != pPrinter; }
    SfxPrinter *GetPrinter()    { GetPrt(); return pPrinter; }
    void        SetPrinter( SfxPrinter * );

    const String GetComment() const;

    // to replace chars that can not be saved with the document...
    sal_Bool    ReplaceBadChars();

    void        UpdateText();
    void        SetText(const String& rBuffer);
    String&     GetText() { return (aText); }
    void        SetFormat(SmFormat& rFormat);
    const SmFormat&  GetFormat() { return (aFormat); }

    void            Parse();
    SmParser &      GetParser() { return aInterpreter; }
    const SmNode *  GetFormulaTree() const  { return pTree; }
    void            SetFormulaTree(SmNode *&rTree) { pTree = rTree; }

    String          GetAccessibleText();

    EditEngine &    GetEditEngine();
    SfxItemPool &   GetEditEngineItemPool();

    void        DrawFormula(OutputDevice &rDev, Point &rPosition, bool bDrawSelection = false);
    Size        GetSize();

    void        Repaint();

    virtual     SfxUndoManager *GetUndoManager ();

    virtual     SfxItemPool& GetPool() const;

    void        Execute( SfxRequest& rReq );
    void        GetState(SfxItemSet &);

    virtual void SetVisArea (const Rectangle & rVisArea);
    virtual void SetModified(BOOL bModified);

    /** Get a cursor for modifying this document
     * @remarks Don't store this reference, a new cursor may be made...
     */
    SmCursor&   GetCursor();
    /** True, if cursor have previously been requested and thus
     * has some sort of position.
     */
    bool        HasCursor() { return pCursor != NULL; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
