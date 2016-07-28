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

#ifndef INCLUDED_STARMATH_INC_DOCUMENT_HXX
#define INCLUDED_STARMATH_INC_DOCUMENT_HXX

#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include <sot/storage.hxx>
#include <svl/lstner.hxx>
#include <vcl/jobset.hxx>
#include <vcl/virdev.hxx>
#include <sax/fshelper.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/mathml/import.hxx>
#include <oox/export/utils.hxx>

#include <memory>
#include <set>

#include "format.hxx"
#include "parse.hxx"
#include "smmod.hxx"
#include "smdllapi.hxx"

class SmNode;
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
 * But the container maybe works with a different MapUnit than the server.
 * Referring to the MapMode the printer will be accordingly adjusted in the
 * constructor and restored in the destructor. This brings that this class
 * is always allowed to exists only a short time (e.g. while painting).
 * The control whether the printer is self-generated, gotten from the server
 * or is NULL then, is taken by the DocShell in the method GetPrt(), for
 * which the access is friend of the DocShell too.
 */

class SmDocShell;
class EditEngine;


class SmPrinterAccess
{
    VclPtr<Printer> pPrinter;
    VclPtr<OutputDevice> pRefDev;
public:
    explicit SmPrinterAccess( SmDocShell &rDocShell );
    ~SmPrinterAccess();
    Printer* GetPrinter()  { return pPrinter.get(); }
    OutputDevice* GetRefDev()  { return pRefDev.get(); }
};


void SetEditEngineDefaultFonts(SfxItemPool &rEditEngineItemPool);


class SM_DLLPUBLIC SmDocShell : public SfxObjectShell, public SfxListener
{
    friend class SmPrinterAccess;
    friend class SmCursor;

    OUString            maText;
    SmFormat            maFormat;
    SmParser            maParser;
    OUString            maAccText;
    SmTableNode        *mpTree;
    SfxItemPool        *mpEditEngineItemPool;
    EditEngine         *mpEditEngine;
    VclPtr<SfxPrinter>  mpPrinter;       //q.v. comment to SmPrinter Access!
    VclPtr<Printer>     mpTmpPrinter;    //ditto
    sal_uInt16          mnModifyCount;
    bool                mbFormulaArranged;
    std::unique_ptr<SmCursor> mpCursor;
    std::set< OUString >    maUsedSymbols;   // to export used symbols only when saving


    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    bool        WriteAsMathType3( SfxMedium& );

    virtual void        Draw(OutputDevice *pDevice,
                             const JobSetup & rSetup,
                             sal_uInt16 nAspect = ASPECT_CONTENT) override;

    virtual void        FillClass(SvGlobalName* pClassName,
                                  SotClipboardFormatId*  pFormat,
                                  OUString* pAppName,
                                  OUString* pFullTypeName,
                                  OUString* pShortTypeName,
                                  sal_Int32 nFileFormat,
                                  bool bTemplate = false ) const override;

    virtual sal_uLong   GetMiscStatus() const override;
    virtual void        OnDocumentPrinterChanged( Printer * ) override;
    virtual bool        InitNew( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;
    virtual bool        Load( SfxMedium& rMedium ) override;
    virtual bool        Save() override;
    virtual bool        SaveAs( SfxMedium& rMedium ) override;
    virtual bool        SaveCompleted( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;

    Printer             *GetPrt();
    OutputDevice*       GetRefDev();

    void                SetFormulaArranged(bool bVal) { mbFormulaArranged = bVal; }

    virtual bool        ConvertFrom(SfxMedium &rMedium) override;

    /** Called whenever the formula is changed
     * Deletes the current cursor
     */
    void                InvalidateCursor();

public:
    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START+1)

    SFX_DECL_OBJECTFACTORY();

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    explicit SmDocShell( SfxModelFlags i_nSfxCreationFlags );
    virtual     ~SmDocShell();

    virtual bool        ConvertTo( SfxMedium &rMedium ) override;

    // For unit tests, not intended to use in other context
    void SetGreekCharStyle(sal_Int16 nVal) { maFormat.SetGreekCharStyle(nVal); }

    static void LoadSymbols();
    static void SaveSymbols();

    void        ArrangeFormula();

    //Access for the View. This access is not for the OLE-case!
    //and for the communication with the SFX!
    //All internal printer uses should work with the SmPrinterAccess only
    bool        HasPrinter()    { return mpPrinter != nullptr; }
    SfxPrinter *GetPrinter()    { GetPrt(); return mpPrinter; }
    void        SetPrinter( SfxPrinter * );

    const OUString GetComment() const;

    // to replace chars that can not be saved with the document...
    void        ReplaceBadChars();

    void        UpdateText();
    void        SetText(const OUString& rBuffer);
    const OUString&  GetText() { return maText; }
    void        SetFormat(SmFormat& rFormat);
    const SmFormat&  GetFormat() { return maFormat; }

    void            Parse();
    SmParser &      GetParser() { return maParser; }
    const SmTableNode *GetFormulaTree() const  { return mpTree; }
    void            SetFormulaTree(SmTableNode *pTree) { mpTree = pTree; }

    const std::set< OUString > &    GetUsedSymbols() const  { return maUsedSymbols; }

    OUString        GetAccessibleText();

    EditEngine &    GetEditEngine();
    SfxItemPool &   GetEditEngineItemPool();

    void        DrawFormula(OutputDevice &rDev, Point &rPosition, bool bDrawSelection = false);
    Size        GetSize();

    void        Repaint();

    virtual ::svl::IUndoManager *GetUndoManager () override;

    static SfxItemPool& GetPool();

    void        Execute( SfxRequest& rReq );
    void        GetState(SfxItemSet &);

    virtual void SetVisArea (const Rectangle & rVisArea) override;
    virtual void SetModified(bool bModified = true) override;

    /** Get a cursor for modifying this document
     * @remarks Don't store this reference, a new cursor may be made...
     */
    SmCursor&   GetCursor();
    /** True, if cursor have previously been requested and thus
     * has some sort of position.
     */
    bool        HasCursor();

    bool writeFormulaOoxml(const ::sax_fastparser::FSHelperPtr& pSerializer,
            oox::core::OoxmlVersion version,
            oox::drawingml::DocumentType documentType);
    void writeFormulaRtf(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding);
    void readFormulaOoxml( oox::formulaimport::XmlStream& stream );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
