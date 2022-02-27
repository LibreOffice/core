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

#pragma once

#include <rtl/strbuf.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include <svl/lstner.hxx>
#include <svl/itempool.hxx>
#include <sax/fshelper.hxx>
#include <unotools/lingucfg.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/export/utils.hxx>

#include "format.hxx"
#include "node.hxx"
#include "parsebase.hxx"
#include "smdllapi.hxx"
#include "mathml/iterator.hxx"

class SfxPrinter;
class Printer;
class SmCursor;

namespace oox::formulaimport { class XmlStream; }

#define STAROFFICE_XML  "StarOffice XML (Math)"
inline constexpr OUStringLiteral MATHML_XML = u"MathML XML (Math)";

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
class SmEditEngine;

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


class SM_DLLPUBLIC SmDocShell final : public SfxObjectShell, public SfxListener
{
    friend class SmPrinterAccess;
    friend class SmCursor;

    OUString            maText;
    SmFormat            maFormat;
    OUString            maAccText;
    SvtLinguOptions     maLinguOptions;
    std::unique_ptr<SmTableNode> mpTree;
    SmMlElement* m_pMlElementTree;
    rtl::Reference<SfxItemPool> mpEditEngineItemPool;
    std::unique_ptr<SmEditEngine> mpEditEngine;
    VclPtr<SfxPrinter>  mpPrinter;       //q.v. comment to SmPrinter Access!
    VclPtr<Printer>     mpTmpPrinter;    //ditto
    sal_uInt16          mnModifyCount;
    bool                mbFormulaArranged;
    sal_uInt16          mnSmSyntaxVersion;
    std::unique_ptr<AbstractSmParser> maParser;
    std::unique_ptr<SmCursor> mpCursor;
    std::set< OUString >    maUsedSymbols;   // to export used symbols only when saving


    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    bool        WriteAsMathType3( SfxMedium& );

    virtual void        Draw(OutputDevice *pDevice,
                             const JobSetup & rSetup,
                             sal_uInt16 nAspect) override;

    virtual void        FillClass(SvGlobalName* pClassName,
                                  SotClipboardFormatId*  pFormat,
                                  OUString* pFullTypeName,
                                  sal_Int32 nFileFormat,
                                  bool bTemplate = false ) const override;

    virtual void        OnDocumentPrinterChanged( Printer * ) override;
    virtual bool        InitNew( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;
    virtual bool        Load( SfxMedium& rMedium ) override;
    virtual bool        Save() override;
    virtual bool        SaveAs( SfxMedium& rMedium ) override;

    Printer             *GetPrt();
    OutputDevice*       GetRefDev();

    void                SetFormulaArranged(bool bVal) { mbFormulaArranged = bVal; }

    virtual bool        ConvertFrom(SfxMedium &rMedium) override;

    /** Called whenever the formula is changed
     * Deletes the current cursor
     */
    void                InvalidateCursor();

public:
    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START+SfxInterfaceId(1))

    SFX_DECL_OBJECTFACTORY();

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    explicit SmDocShell( SfxModelFlags i_nSfxCreationFlags );
    virtual     ~SmDocShell() override;

    virtual bool        ConvertTo( SfxMedium &rMedium ) override;

    // For unit tests, not intended to use in other context
    void SetGreekCharStyle(sal_Int16 nVal) { maFormat.SetGreekCharStyle(nVal); }

    static void LoadSymbols();
    static void SaveSymbols();

    void        ArrangeFormula();

    //Access for the View. This access is not for the OLE-case!
    //and for the communication with the SFX!
    //All internal printer uses should work with the SmPrinterAccess only
    bool        HasPrinter() const { return mpPrinter != nullptr; }
    SfxPrinter *GetPrinter()    { GetPrt(); return mpPrinter; }
    void        SetPrinter( SfxPrinter * );

    OUString GetComment() const;

    // to replace chars that can not be saved with the document...
    void        ReplaceBadChars();

    void        UpdateText();
    void        SetText(const OUString& rBuffer);
    const OUString&  GetText() const { return maText; }
    void        SetFormat(SmFormat const & rFormat);
    const SmFormat&  GetFormat() const { return maFormat; }

    void            Parse();
    AbstractSmParser* GetParser() { return maParser.get(); }
    const SmTableNode *GetFormulaTree() const  { return mpTree.get(); }
    void            SetFormulaTree(SmTableNode *pTree) { mpTree.reset(pTree); }
    sal_uInt16      GetSmSyntaxVersion() const { return mnSmSyntaxVersion; }
    void            SetSmSyntaxVersion(sal_uInt16 nSmSyntaxVersion);

    const std::set< OUString > &    GetUsedSymbols() const  { return maUsedSymbols; }

    OUString const & GetAccessibleText();

    EditEngine &    GetEditEngine();

    void        DrawFormula(OutputDevice &rDev, Point &rPosition, bool bDrawSelection = false);
    Size        GetSize();

    void        Repaint();

    virtual SfxUndoManager *GetUndoManager () override;

    static SfxItemPool& GetPool();

    void        Execute( SfxRequest& rReq );
    void        GetState(SfxItemSet &);

    virtual void SetVisArea (const tools::Rectangle & rVisArea) override;
    virtual void SetModified(bool bModified = true) override;

    /** Get a cursor for modifying this document
     * @remarks Don't store this reference, a new cursor may be made...
     */
    SmCursor&   GetCursor();
    /** True, if cursor have previously been requested and thus
     * has some sort of position.
     */
    bool        HasCursor() const;

    void writeFormulaOoxml(const ::sax_fastparser::FSHelperPtr& pSerializer,
            oox::core::OoxmlVersion version,
            oox::drawingml::DocumentType documentType,
            const sal_Int8 nAlign);
    void writeFormulaRtf(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding);
    void readFormulaOoxml( oox::formulaimport::XmlStream& stream );

    void UpdateEditEngineDefaultFonts();

    SmMlElement* GetMlElementTree()  { return m_pMlElementTree; }
    void SetMlElementTree(SmMlElement* pMlElementTree) {
        mathml::SmMlIteratorFree(m_pMlElementTree);
        m_pMlElementTree = pMlElementTree;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
