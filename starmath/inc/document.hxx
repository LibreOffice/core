/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef DOCUMENT_HXX
#define DOCUMENT_HXX

#define SMDLL   1

#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include <sot/sotref.hxx>
#include <sot/storage.hxx>
#include <svl/lstner.hxx>
#include <vcl/jobset.hxx>
#include <vcl/virdev.hxx>
#include <sax/fshelper.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/mathml/import.hxx>

#include <set>

#include "format.hxx"
#include "parse.hxx"
#include "smmod.hxx"

class SmNode;
class SfxMenuBarManager;
class SfxPrinter;
class Printer;
class SmCursor;

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

void SetEditEngineDefaultFonts(SfxItemPool &rEditEngineItemPool);

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
    sal_uInt16          nModifyCount;
    bool                bIsFormulaArranged;
    SmCursor           *pCursor;
    std::set< rtl::OUString >    aUsedSymbols;   // to export used symbols only when saving



    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                        const SfxHint& rHint, const TypeId& rHintType);

    bool        WriteAsMathType3( SfxMedium& );

    virtual void        Draw(OutputDevice *pDevice,
                             const JobSetup & rSetup,
                             sal_uInt16 nAspect = ASPECT_CONTENT);

    virtual void        FillClass(SvGlobalName* pClassName,
                                  sal_uInt32*  pFormat,
                                  String* pAppName,
                                  String* pFullTypeName,
                                  String* pShortTypeName,
                                  sal_Int32 nFileFormat,
                                  sal_Bool bTemplate = sal_False ) const;

    virtual sal_Bool        SetData( const String& rData );
    virtual sal_uLong       GetMiscStatus() const;
    virtual void        OnDocumentPrinterChanged( Printer * );
    virtual sal_Bool    InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    virtual sal_Bool        Load( SfxMedium& rMedium );
            void        ImplSave(  SvStorageStreamRef xStrm  );
    virtual sal_Bool        Save();
    virtual sal_Bool        SaveAs( SfxMedium& rMedium );
    virtual sal_Bool        ConvertTo( SfxMedium &rMedium );
    virtual sal_Bool        SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    Printer             *GetPrt();
    OutputDevice*       GetRefDev();

    bool                IsFormulaArranged() const { return bIsFormulaArranged; }
    void                SetFormulaArranged(bool bVal) { bIsFormulaArranged = bVal; }

    virtual sal_Bool        ConvertFrom(SfxMedium &rMedium);

    /** Called whenever the formula is changed
     * Deletes the current cursor
     */
    void                InvalidateCursor();

    bool writeFormulaOoxml( ::sax_fastparser::FSHelperPtr pSerializer, oox::core::OoxmlVersion version );
    void writeFormulaRtf(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding);
    void readFormulaOoxml( oox::formulaimport::XmlStream& stream );

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

    const std::set< rtl::OUString > &    GetUsedSymbols() const  { return aUsedSymbols; }

    String          GetAccessibleText();

    EditEngine &    GetEditEngine();
    SfxItemPool &   GetEditEngineItemPool();

    void        DrawFormula(OutputDevice &rDev, Point &rPosition, bool bDrawSelection = false);
    Size        GetSize();

    void        Repaint();

    virtual     ::svl::IUndoManager *GetUndoManager ();

    virtual     SfxItemPool& GetPool() const;

    void        Execute( SfxRequest& rReq );
    void        GetState(SfxItemSet &);

    virtual void SetVisArea (const Rectangle & rVisArea);
    virtual void SetModified(sal_Bool bModified);

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
