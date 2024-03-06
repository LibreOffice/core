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

#include <sal/config.h>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/uno/Any.h>

#include <comphelper/fileformat.h>
#include <comphelper/accessibletexthelper.hxx>
#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <unotools/eventcfg.hxx>
#include <sfx2/event.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/classids.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <svl/slstitm.hxx>
#include <svl/hint.hxx>
#include <svl/stritem.hxx>
#include <svl/undo.hxx>
#include <svl/whiter.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/virdev.hxx>
#include <tools/mapunit.hxx>
#include <vcl/settings.hxx>

#include <document.hxx>
#include <action.hxx>
#include <dialog.hxx>
#include <format.hxx>
#include <parse.hxx>
#include <starmath.hrc>
#include <strings.hrc>
#include <smmod.hxx>
#include <symbol.hxx>
#include <unomodel.hxx>
#include <utility.hxx>
#include <view.hxx>
#include "mathtype.hxx"
#include "ooxmlexport.hxx"
#include "ooxmlimport.hxx"
#include "rtfexport.hxx"
#include <mathmlimport.hxx>
#include <mathmlexport.hxx>
#include <svx/svxids.hrc>
#include <cursor.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <visitors.hxx>
#include "accessibility.hxx"
#include <cfgitem.hxx>
#include <utility>
#include <oox/mathml/imexport.hxx>
#include <ElementsDockingWindow.hxx>
#include <smediteng.hxx>
#include <editeng/editund2.hxx>

#define ShellClass_SmDocShell
#include <smslots.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;


SFX_IMPL_SUPERCLASS_INTERFACE(SmDocShell, SfxObjectShell)

void SmDocShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("view");
}

void SmDocShell::SetSmSyntaxVersion(sal_Int16 nSmSyntaxVersion)
{
    mnSmSyntaxVersion = nSmSyntaxVersion;
    maParser.reset(starmathdatabase::GetVersionSmParser(mnSmSyntaxVersion));
}

SFX_IMPL_OBJECTFACTORY(SmDocShell, SvGlobalName(SO3_SM_CLASSID), "smath" )

void SmDocShell::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::MathFormatChanged)
    {
        SetFormulaArranged(false);

        mnModifyCount++;     //! see comment for SID_GRAPHIC_SM in SmDocShell::GetState

        Repaint();
    }
}

void SmDocShell::LoadSymbols()
{
    SmModule *pp = SM_MOD();
    pp->GetSymbolManager().Load();
}


OUString SmDocShell::GetComment() const
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());
    return xDocProps->getDescription();
}


void SmDocShell::SetText(const OUString& rBuffer)
{
    if (rBuffer == maText)
        return;

    bool bIsEnabled = IsEnableSetModified();
    if( bIsEnabled )
        EnableSetModified( false );

    maText = rBuffer;
    SetFormulaArranged( false );

    Parse();

    SmViewShell *pViewSh = SmGetActiveView();
    if (pViewSh)
    {
        pViewSh->GetViewFrame().GetBindings().Invalidate(SID_TEXT);
        if ( SfxObjectCreateMode::EMBEDDED == GetCreateMode() )
        {
            // have SwOleClient::FormatChanged() to align the modified formula properly
            // even if the visible area does not change (e.g. when formula text changes from
            // "{a over b + c} over d" to "d over {a over b + c}"
            SfxGetpApp()->NotifyEvent(SfxEventHint( SfxEventHintId::VisAreaChanged, GlobalEventConfig::GetEventName(GlobalEventId::VISAREACHANGED), this));

            Repaint();
        }
        else
            pViewSh->GetGraphicWidget().Invalidate();
    }

    if ( bIsEnabled )
        EnableSetModified( bIsEnabled );
    SetModified();

    // launch accessible event if necessary
    SmGraphicAccessible *pAcc = pViewSh ? pViewSh->GetGraphicWidget().GetAccessible_Impl() : nullptr;
    if (pAcc)
    {
        Any aOldValue, aNewValue;
        if ( comphelper::OCommonAccessibleText::implInitTextChangedEvent( maText, rBuffer, aOldValue, aNewValue ) )
        {
            pAcc->LaunchEvent( AccessibleEventId::TEXT_CHANGED,
                    aOldValue, aNewValue );
        }
    }

    if ( GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
        OnDocumentPrinterChanged(nullptr);
}

void SmDocShell::SetFormat(SmFormat const & rFormat)
{
    maFormat = rFormat;
    SetFormulaArranged( false );
    SetModified();

    mnModifyCount++;     //! see comment for SID_GRAPHIC_SM in SmDocShell::GetState

    // don't use SmGetActiveView since the view shell might not be active (0 pointer)
    // if for example the Basic Macro dialog currently has the focus. Thus:
    SfxViewFrame* pFrm = SfxViewFrame::GetFirst( this );
    while (pFrm)
    {
        pFrm->GetBindings().Invalidate(SID_GRAPHIC_SM);
        pFrm = SfxViewFrame::GetNext( *pFrm, this );
    }
}

OUString const & SmDocShell::GetAccessibleText()
{
    ArrangeFormula();
    if (maAccText.isEmpty())
    {
        OSL_ENSURE( mpTree, "Tree missing" );
        if (mpTree)
        {
            OUStringBuffer aBuf;
            mpTree->GetAccessibleText(aBuf);
            maAccText = aBuf.makeStringAndClear();
        }
    }
    return maAccText;
}

void SmDocShell::Parse()
{
    mpTree.reset();
    ReplaceBadChars();
    mpTree = maParser->Parse(maText);
    mnModifyCount++;     //! see comment for SID_GRAPHIC_SM in SmDocShell::GetState
    SetFormulaArranged( false );
    InvalidateCursor();
    maUsedSymbols = maParser->GetUsedSymbols();
}


void SmDocShell::ArrangeFormula()
{
    if (mbFormulaArranged)
        return;

    // Only for the duration of the existence of this object the correct settings
    // at the printer are guaranteed!
    SmPrinterAccess  aPrtAcc(*this);
    OutputDevice* pOutDev = aPrtAcc.GetRefDev();

    SAL_WARN_IF( !pOutDev, "starmath", "!! SmDocShell::ArrangeFormula: reference device missing !!");

    // if necessary get another OutputDevice for which we format
    if (!pOutDev)
    {
        if (SmViewShell *pView = SmGetActiveView())
            pOutDev = &pView->GetGraphicWidget().GetDrawingArea()->get_ref_device();
        else
        {
            pOutDev = &SM_MOD()->GetDefaultVirtualDev();
            pOutDev->SetMapMode( MapMode(SmMapUnit()) );
        }
    }
    OSL_ENSURE(pOutDev->GetMapMode().GetMapUnit() == SmMapUnit(),
               "Sm : wrong MapMode");

    const SmFormat &rFormat = GetFormat();
    mpTree->Prepare(rFormat, *this, 0);

    pOutDev->Push(vcl::PushFlags::TEXTLAYOUTMODE | vcl::PushFlags::TEXTLANGUAGE);

    // We want the device to always be LTR, we handle RTL formulas ourselves.
    bool bOldRTL = pOutDev->IsRTLEnabled();
    pOutDev->EnableRTL(false);

    // For RTL formulas, we want the brackets to be mirrored.
    bool bRTL = GetFormat().IsRightToLeft();
    pOutDev->SetLayoutMode(bRTL ? vcl::text::ComplexTextLayoutFlags::BiDiRtl
                                : vcl::text::ComplexTextLayoutFlags::Default);

    // Numbers should not be converted, for now.
    pOutDev->SetDigitLanguage( LANGUAGE_ENGLISH );

    mpTree->Arrange(*pOutDev, rFormat);

    pOutDev->EnableRTL(bOldRTL);
    pOutDev->Pop();

    SetFormulaArranged(true);

    // invalidate accessible text
    maAccText.clear();
}

void SmDocShell::UpdateEditEngineDefaultFonts()
{
    SmEditEngine::setSmItemPool(mpEditEngineItemPool.get(), maLinguOptions);
}

EditEngine& SmDocShell::GetEditEngine()
{
    if (!mpEditEngine)
    {
        //!
        //! see also SmEditWindow::DataChanged !
        //!
        mpEditEngineItemPool = EditEngine::CreatePool();
        SmEditEngine::setSmItemPool(mpEditEngineItemPool.get(), maLinguOptions);
        mpEditEngine.reset( new SmEditEngine( mpEditEngineItemPool.get() ) );
        mpEditEngine->EraseVirtualDevice();

        // set initial text if the document already has some...
        // (may be the case when reloading a doc)
        OUString aTxt( GetText() );
        if (!aTxt.isEmpty())
            mpEditEngine->SetText( aTxt );
        mpEditEngine->ClearModifyFlag();
    }
    return *mpEditEngine;
}


void SmDocShell::DrawFormula(OutputDevice &rDev, Point &rPosition, bool bDrawSelection)
{
    if (!mpTree)
        Parse();
    OSL_ENSURE(mpTree, "Sm : NULL pointer");

    ArrangeFormula();

    bool bRTL = GetFormat().IsRightToLeft();

    // Problem: What happens to WYSIWYG? While we're active inplace, we don't have a reference
    // device and aren't aligned to that either. So now there can be a difference between the
    // VisArea (i.e. the size within the client) and the current size.
    // Idea: The difference could be adapted with SmNod::SetSize (no long-term solution)

    rPosition.AdjustX(maFormat.GetDistance( DIS_LEFTSPACE ) );
    rPosition.AdjustY(maFormat.GetDistance( DIS_TOPSPACE  ) );

    Point aPosition(rPosition);
    if (bRTL && rDev.GetOutDevType() != OUTDEV_WINDOW)
        aPosition.AdjustX(GetSize().Width()
                          - maFormat.GetDistance(DIS_LEFTSPACE)
                          - maFormat.GetDistance(DIS_RIGHTSPACE));

    //! in case of high contrast-mode (accessibility option!)
    //! the draw mode needs to be set to default, because when embedding
    //! Math for example in Calc in "a over b" the fraction bar may not
    //! be visible else. More generally: the FillColor may have been changed.
    DrawModeFlags nOldDrawMode = DrawModeFlags::Default;
    bool bRestoreDrawMode = false;
    if (OUTDEV_WINDOW == rDev.GetOutDevType() &&
        rDev.GetOwnerWindow()->GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        nOldDrawMode = rDev.GetDrawMode();
        rDev.SetDrawMode( DrawModeFlags::Default );
        bRestoreDrawMode = true;
    }

    rDev.Push(vcl::PushFlags::TEXTLAYOUTMODE | vcl::PushFlags::TEXTLANGUAGE);

    // We want the device to always be LTR, we handle RTL formulas ourselves.
    bool bOldRTL = rDev.IsRTLEnabled();
    if (rDev.GetOutDevType() == OUTDEV_WINDOW)
        rDev.EnableRTL(bRTL);
    else
        rDev.EnableRTL(false);

    auto nLayoutFlags = vcl::text::ComplexTextLayoutFlags::Default;
    if (bRTL)
    {
        // For RTL formulas, we want the brackets to be mirrored.
        nLayoutFlags |= vcl::text::ComplexTextLayoutFlags::BiDiRtl;
        if (rDev.GetOutDevType() == OUTDEV_WINDOW)
            nLayoutFlags |= vcl::text::ComplexTextLayoutFlags::TextOriginLeft;
    }

    rDev.SetLayoutMode(nLayoutFlags);

    // Numbers should not be converted, for now.
    rDev.SetDigitLanguage( LANGUAGE_ENGLISH );

    //Set selection if any
    if(mpCursor && bDrawSelection){
        mpCursor->AnnotateSelection();
        SmSelectionDrawingVisitor(rDev, mpTree.get(), aPosition);
    }

    //Drawing using visitor
    SmDrawingVisitor(rDev, aPosition, mpTree.get(), GetFormat());

    rDev.EnableRTL(bOldRTL);
    rDev.Pop();

    if (bRestoreDrawMode)
        rDev.SetDrawMode( nOldDrawMode );
}

Size SmDocShell::GetSize()
{
    Size aRet;

    if (!mpTree)
        Parse();

    if (mpTree)
    {
        ArrangeFormula();
        aRet = mpTree->GetSize();

        if ( !aRet.Width() || aRet.Width() == 1 )
            aRet.setWidth( 2000 );
        else
            aRet.AdjustWidth(maFormat.GetDistance( DIS_LEFTSPACE ) +
                             maFormat.GetDistance( DIS_RIGHTSPACE ) );
        if ( !aRet.Height() )
            aRet.setHeight( 1000 );
        else
            aRet.AdjustHeight(maFormat.GetDistance( DIS_TOPSPACE ) +
                             maFormat.GetDistance( DIS_BOTTOMSPACE ) );
    }

    return aRet;
}

void SmDocShell::InvalidateCursor(){
    mpCursor.reset();
}

SmCursor& SmDocShell::GetCursor(){
    if(!mpCursor)
        mpCursor.reset(new SmCursor(mpTree.get(), this));
    return *mpCursor;
}

bool SmDocShell::HasCursor() const { return mpCursor != nullptr; }

SmPrinterAccess::SmPrinterAccess( SmDocShell &rDocShell )
{
    pPrinter = rDocShell.GetPrt();
    if ( pPrinter )
    {
        pPrinter->Push( vcl::PushFlags::MAPMODE );
        if ( SfxObjectCreateMode::EMBEDDED == rDocShell.GetCreateMode() )
        {
            // if it is an embedded object (without its own printer)
            // we change the MapMode temporarily.
            //!If it is a document with its own printer the MapMode should
            //!be set correct (once) elsewhere(!), in order to avoid numerous
            //!superfluous pushing and popping of the MapMode when using
            //!this class.

            const MapUnit eOld = pPrinter->GetMapMode().GetMapUnit();
            if ( SmMapUnit() != eOld )
            {
                MapMode aMap( pPrinter->GetMapMode() );
                aMap.SetMapUnit( SmMapUnit() );
                Point aTmp( aMap.GetOrigin() );
                aTmp.setX( OutputDevice::LogicToLogic( aTmp.X(), eOld, SmMapUnit() ) );
                aTmp.setY( OutputDevice::LogicToLogic( aTmp.Y(), eOld, SmMapUnit() ) );
                aMap.SetOrigin( aTmp );
                pPrinter->SetMapMode( aMap );
            }
        }
    }
    pRefDev = rDocShell.GetRefDev();
    if ( !pRefDev || pPrinter.get() == pRefDev.get() )
        return;

    pRefDev->Push( vcl::PushFlags::MAPMODE );
    if ( SfxObjectCreateMode::EMBEDDED != rDocShell.GetCreateMode() )
        return;

    // if it is an embedded object (without its own printer)
    // we change the MapMode temporarily.
    //!If it is a document with its own printer the MapMode should
    //!be set correct (once) elsewhere(!), in order to avoid numerous
    //!superfluous pushing and popping of the MapMode when using
    //!this class.

    const MapUnit eOld = pRefDev->GetMapMode().GetMapUnit();
    if ( SmMapUnit() != eOld )
    {
        MapMode aMap( pRefDev->GetMapMode() );
        aMap.SetMapUnit( SmMapUnit() );
        Point aTmp( aMap.GetOrigin() );
        aTmp.setX( OutputDevice::LogicToLogic( aTmp.X(), eOld, SmMapUnit() ) );
        aTmp.setY( OutputDevice::LogicToLogic( aTmp.Y(), eOld, SmMapUnit() ) );
        aMap.SetOrigin( aTmp );
        pRefDev->SetMapMode( aMap );
    }
}

SmPrinterAccess::~SmPrinterAccess()
{
    if ( pPrinter )
        pPrinter->Pop();
    if ( pRefDev && pRefDev != pPrinter )
        pRefDev->Pop();
}

Printer* SmDocShell::GetPrt()
{
    if (SfxObjectCreateMode::EMBEDDED == GetCreateMode())
    {
        // Normally the server provides the printer. But if it doesn't provide one (e.g. because
        // there is no connection) it still can be the case that we know the printer because it
        // has been passed on by the server in OnDocumentPrinterChanged and being kept temporarily.
        Printer* pPrt = GetDocumentPrinter();
        if (!pPrt && mpTmpPrinter)
            pPrt = mpTmpPrinter;
        return pPrt;
    }
    else if (!mpPrinter)
    {
        auto pOptions = std::make_unique<SfxItemSetFixed<
                SID_PRINTTITLE, SID_PRINTZOOM,
                SID_NO_RIGHT_SPACES, SID_SAVE_ONLY_USED_SYMBOLS,
                SID_AUTO_CLOSE_BRACKETS, SID_SMEDITWINDOWZOOM,
                SID_INLINE_EDIT_ENABLE, SID_INLINE_EDIT_ENABLE>>(GetPool());
        SmModule *pp = SM_MOD();
        pp->GetConfig()->ConfigToItemSet(*pOptions);
        mpPrinter = VclPtr<SfxPrinter>::Create(std::move(pOptions));
        mpPrinter->SetMapMode(MapMode(SmMapUnit()));
    }
    return mpPrinter;
}

OutputDevice* SmDocShell::GetRefDev()
{
    if (SfxObjectCreateMode::EMBEDDED == GetCreateMode())
    {
        OutputDevice* pOutDev = GetDocumentRefDev();
        if (pOutDev)
            return pOutDev;
    }

    return GetPrt();
}

void SmDocShell::SetPrinter( SfxPrinter *pNew )
{
    mpPrinter.disposeAndClear();
    mpPrinter = pNew;    //Transfer ownership
    mpPrinter->SetMapMode( MapMode(SmMapUnit()) );
    SetFormulaArranged(false);
    Repaint();
}

void SmDocShell::OnDocumentPrinterChanged( Printer *pPrt )
{
    mpTmpPrinter = pPrt;
    SetFormulaArranged(false);
    Size aOldSize = GetVisArea().GetSize();
    Repaint();
    if( aOldSize != GetVisArea().GetSize() && !maText.isEmpty() )
        SetModified();
    mpTmpPrinter = nullptr;
}

void SmDocShell::Repaint()
{
    bool bIsEnabled = IsEnableSetModified();
    if (bIsEnabled)
        EnableSetModified( false );

    SetFormulaArranged(false);

    Size aVisSize = GetSize();
    SetVisAreaSize(aVisSize);
    if (SmViewShell* pViewSh = SmGetActiveView())
        pViewSh->GetGraphicWidget().Invalidate();

    if (bIsEnabled)
        EnableSetModified(bIsEnabled);
}

SmDocShell::SmDocShell( SfxModelFlags i_nSfxCreationFlags )
    : SfxObjectShell(i_nSfxCreationFlags)
    , m_pMlElementTree(nullptr)
    , mpPrinter(nullptr)
    , mpTmpPrinter(nullptr)
    , mnModifyCount(0)
    , mbFormulaArranged(false)
    , mnSmSyntaxVersion(SM_MOD()->GetConfig()->GetDefaultSmSyntaxVersion())
{
    SvtLinguConfig().GetOptions(maLinguOptions);

    SetPool(&SfxGetpApp()->GetPool());

    SmModule *pp = SM_MOD();
    maFormat = pp->GetConfig()->GetStandardFormat();

    StartListening(maFormat);
    StartListening(*pp->GetConfig());

    SetBaseModel(new SmModel(this));
    SetSmSyntaxVersion(mnSmSyntaxVersion);

    SetMapUnit(SmMapUnit());
}

SmDocShell::~SmDocShell()
{
    SmModule *pp = SM_MOD();

    EndListening(maFormat);
    EndListening(*pp->GetConfig());

    mpCursor.reset();
    mpEditEngine.reset();
    mpEditEngineItemPool.clear();
    mpPrinter.disposeAndClear();

    mathml::SmMlIteratorFree(m_pMlElementTree);
}

bool SmDocShell::ConvertFrom(SfxMedium &rMedium)
{
    bool     bSuccess = false;
    const OUString& rFltName = rMedium.GetFilter()->GetFilterName();

    OSL_ENSURE( rFltName != STAROFFICE_XML, "Wrong filter!");

    if ( rFltName == MATHML_XML )
    {
        if (mpTree)
        {
            mpTree.reset();
            InvalidateCursor();
        }
        rtl::Reference<SmModel> xModel(dynamic_cast<SmModel*>(GetModel().get()));
        SmXMLImportWrapper aEquation(xModel);
        aEquation.useHTMLMLEntities(true);
        bSuccess = ( ERRCODE_NONE == aEquation.Import(rMedium) );
    }
    else
    {
        SvStream *pStream = rMedium.GetInStream();
        if ( pStream )
        {
            if ( SotStorage::IsStorageFile( pStream ) )
            {
                rtl::Reference<SotStorage> aStorage = new SotStorage(pStream, false);
                if ( aStorage->IsStream("Equation Native") )
                {
                    // is this a MathType Storage?
                    OUStringBuffer aBuffer;
                    MathType aEquation(aBuffer);
                    bSuccess = aEquation.Parse( aStorage.get() );
                    if ( bSuccess )
                    {
                        maText = aBuffer.makeStringAndClear();
                        Parse();
                    }
                }
            }
        }
    }

    if ( GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        SetFormulaArranged( false );
        Repaint();
    }

    FinishedLoading();
    return bSuccess;
}


bool SmDocShell::InitNew( const uno::Reference < embed::XStorage >& xStorage )
{
    bool bRet = false;
    if ( SfxObjectShell::InitNew( xStorage ) )
    {
        bRet = true;
        SetVisArea(tools::Rectangle(Point(0, 0), Size(2000, 1000)));
    }
    return bRet;
}


bool SmDocShell::Load( SfxMedium& rMedium )
{
    bool bRet = false;
    if( SfxObjectShell::Load( rMedium ))
    {
        uno::Reference < embed::XStorage > xStorage = GetMedium()->GetStorage();
        if (xStorage->hasByName("content.xml") && xStorage->isStreamElement("content.xml"))
        {
            // is this a fabulous math package ?
            rtl::Reference<SmModel> xModel(dynamic_cast<SmModel*>(GetModel().get()));
            SmXMLImportWrapper aEquation(xModel);
            auto nError = aEquation.Import(rMedium);
            bRet = ERRCODE_NONE == nError;
            SetError(nError);
        }
    }

    if ( GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        SetFormulaArranged( false );
        Repaint();
    }

    FinishedLoading();
    return bRet;
}


bool SmDocShell::Save()
{
    //! apply latest changes if necessary
    UpdateText();

    if ( SfxObjectShell::Save() )
    {
        if (!mpTree)
            Parse();
        if( mpTree )
            ArrangeFormula();

        SmXMLExportWrapper aEquation(GetModel());
        aEquation.SetFlat(false);
        return aEquation.Export(*GetMedium());
    }

    return false;
}

/*
 * replace bad characters that can not be saved. (#i74144)
 * */
void SmDocShell::ReplaceBadChars()
{
    bool bReplace = false;

    if (!mpEditEngine)
        return;

    OUStringBuffer aBuf( mpEditEngine->GetText() );

    for (sal_Int32 i = 0;  i < aBuf.getLength();  ++i)
    {
        if (aBuf[i] < ' ' && aBuf[i] != '\r' && aBuf[i] != '\n' && aBuf[i] != '\t')
        {
            aBuf[i] = ' ';
            bReplace = true;
        }
    }

    if (bReplace)
        maText = aBuf.makeStringAndClear();
}


void SmDocShell::UpdateText()
{
    if (mpEditEngine && mpEditEngine->IsModified())
    {
        OUString aEngTxt( mpEditEngine->GetText() );
        if (GetText() != aEngTxt)
            SetText( aEngTxt );
    }
}


bool SmDocShell::SaveAs( SfxMedium& rMedium )
{
    bool bRet = false;

    //! apply latest changes if necessary
    UpdateText();

    if ( SfxObjectShell::SaveAs( rMedium ) )
    {
        if (!mpTree)
            Parse();
        if( mpTree )
            ArrangeFormula();

        SmXMLExportWrapper aEquation(GetModel());
        aEquation.SetFlat(false);
        bRet = aEquation.Export(rMedium);
    }
    return bRet;
}

bool SmDocShell::ConvertTo( SfxMedium &rMedium )
{
    bool bRet = false;
    std::shared_ptr<const SfxFilter> pFlt = rMedium.GetFilter();
    if( pFlt )
    {
        if( !mpTree )
            Parse();
        if( mpTree )
            ArrangeFormula();

        const OUString& rFltName = pFlt->GetFilterName();
        if(rFltName == STAROFFICE_XML)
        {
            SmXMLExportWrapper aEquation(GetModel());
            aEquation.SetFlat(false);
            bRet = aEquation.Export(rMedium);
        }
        else if(rFltName == MATHML_XML)
        {
            SmXMLExportWrapper aEquation(GetModel());
            aEquation.SetFlat(true);
            aEquation.SetUseHTMLMLEntities(true);
            bRet = aEquation.Export(rMedium);
        }
        else if (pFlt->GetFilterName() == "MathType 3.x")
            bRet = WriteAsMathType3( rMedium );
    }
    return bRet;
}

void SmDocShell::writeFormulaOoxml(
        ::sax_fastparser::FSHelperPtr const& pSerializer,
        oox::core::OoxmlVersion const version,
        oox::drawingml::DocumentType const documentType,
        const sal_Int8 nAlign)
{
    if( !mpTree )
        Parse();
    if( mpTree )
        ArrangeFormula();
    SmOoxmlExport aEquation(mpTree.get(), version, documentType);
    if(documentType == oox::drawingml::DOCUMENT_DOCX)
        aEquation.ConvertFromStarMath( pSerializer, nAlign);
    else
        aEquation.ConvertFromStarMath(pSerializer, oox::FormulaImExportBase::eFormulaAlign::INLINE);
}

void SmDocShell::writeFormulaRtf(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding)
{
    if (!mpTree)
        Parse();
    if (mpTree)
        ArrangeFormula();
    SmRtfExport aEquation(mpTree.get());
    aEquation.ConvertFromStarMath(rBuffer, nEncoding);
}

void SmDocShell::readFormulaOoxml( oox::formulaimport::XmlStream& stream )
{
    SmOoxmlImport aEquation( stream );
    SetText( aEquation.ConvertToStarMath());
}

void SmDocShell::Execute(SfxRequest& rReq)
{
    switch (rReq.GetSlot())
    {
        case SID_TEXTMODE:
        {
            SmFormat aOldFormat  = GetFormat();
            SmFormat aNewFormat( aOldFormat );
            aNewFormat.SetTextmode(!aOldFormat.IsTextmode());

            SfxUndoManager *pTmpUndoMgr = GetUndoManager();
            if (pTmpUndoMgr)
                pTmpUndoMgr->AddUndoAction(
                    std::make_unique<SmFormatAction>(this, aOldFormat, aNewFormat));

            SetFormat( aNewFormat );
            Repaint();
        }
        break;

        case SID_AUTO_REDRAW :
        {
            SmModule *pp = SM_MOD();
            bool bRedraw = pp->GetConfig()->IsAutoRedraw();
            pp->GetConfig()->SetAutoRedraw(!bRedraw);
        }
        break;

        case SID_LOADSYMBOLS:
            LoadSymbols();
        break;

        case SID_SAVESYMBOLS:
            SaveSymbols();
        break;

        case SID_FONT:
        {
            // get device used to retrieve the FontList
            OutputDevice *pDev = GetPrinter();
            if (!pDev || pDev->GetFontFaceCollectionCount() == 0)
                pDev = &SM_MOD()->GetDefaultVirtualDev();
            OSL_ENSURE (pDev, "device for font list missing" );

            SmFontTypeDialog aFontTypeDialog(rReq.GetFrameWeld(), pDev);

            SmFormat aOldFormat  = GetFormat();
            aFontTypeDialog.ReadFrom( aOldFormat );
            if (aFontTypeDialog.run() == RET_OK)
            {
                SmFormat aNewFormat( aOldFormat );

                aFontTypeDialog.WriteTo(aNewFormat);
                SfxUndoManager *pTmpUndoMgr = GetUndoManager();
                if (pTmpUndoMgr)
                    pTmpUndoMgr->AddUndoAction(
                        std::make_unique<SmFormatAction>(this, aOldFormat, aNewFormat));

                SetFormat( aNewFormat );
                Repaint();
            }
        }
        break;

        case SID_FONTSIZE:
        {
            SmFontSizeDialog aFontSizeDialog(rReq.GetFrameWeld());

            SmFormat aOldFormat  = GetFormat();
            aFontSizeDialog.ReadFrom( aOldFormat );
            if (aFontSizeDialog.run() == RET_OK)
            {
                SmFormat aNewFormat( aOldFormat );

                aFontSizeDialog.WriteTo(aNewFormat);

                SfxUndoManager *pTmpUndoMgr = GetUndoManager();
                if (pTmpUndoMgr)
                    pTmpUndoMgr->AddUndoAction(
                        std::make_unique<SmFormatAction>(this, aOldFormat, aNewFormat));

                SetFormat( aNewFormat );
                Repaint();
            }
        }
        break;

        case SID_DISTANCE:
        {
            SmDistanceDialog aDistanceDialog(rReq.GetFrameWeld());

            SmFormat aOldFormat  = GetFormat();
            aDistanceDialog.ReadFrom( aOldFormat );
            if (aDistanceDialog.run() == RET_OK)
            {
                SmFormat aNewFormat( aOldFormat );

                aDistanceDialog.WriteTo(aNewFormat);

                SfxUndoManager *pTmpUndoMgr = GetUndoManager();
                if (pTmpUndoMgr)
                    pTmpUndoMgr->AddUndoAction(
                        std::make_unique<SmFormatAction>(this, aOldFormat, aNewFormat));

                SetFormat( aNewFormat );
                Repaint();
            }
        }
        break;

        case SID_ALIGN:
        {
            SmAlignDialog aAlignDialog(rReq.GetFrameWeld());

            SmFormat aOldFormat  = GetFormat();
            aAlignDialog.ReadFrom( aOldFormat );
            if (aAlignDialog.run() == RET_OK)
            {
                SmFormat aNewFormat( aOldFormat );

                aAlignDialog.WriteTo(aNewFormat);

                SmModule *pp = SM_MOD();
                SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
                aAlignDialog.WriteTo( aFmt );
                pp->GetConfig()->SetStandardFormat( aFmt );

                SfxUndoManager *pTmpUndoMgr = GetUndoManager();
                if (pTmpUndoMgr)
                    pTmpUndoMgr->AddUndoAction(
                        std::make_unique<SmFormatAction>(this, aOldFormat, aNewFormat));

                SetFormat( aNewFormat );
                Repaint();
            }
        }
        break;

        case SID_TEXT:
        {
            const SfxStringItem& rItem = rReq.GetArgs()->Get(SID_TEXT);
            if (GetText() != rItem.GetValue())
                SetText(rItem.GetValue());
        }
        break;

        case SID_UNDO:
        case SID_REDO:
        {
            SfxUndoManager* pTmpUndoMgr = GetUndoManager();
            if( pTmpUndoMgr )
            {
                sal_uInt16 nId = rReq.GetSlot(), nCnt = 1;
                const SfxItemSet* pArgs = rReq.GetArgs();
                const SfxPoolItem* pItem;
                if( pArgs && SfxItemState::SET == pArgs->GetItemState( nId, false, &pItem ))
                    nCnt = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

                bool (SfxUndoManager::*fnDo)();

                size_t nCount;
                if( SID_UNDO == rReq.GetSlot() )
                {
                    nCount = pTmpUndoMgr->GetUndoActionCount();
                    fnDo = &SfxUndoManager::Undo;
                }
                else
                {
                    nCount = pTmpUndoMgr->GetRedoActionCount();
                    fnDo = &SfxUndoManager::Redo;
                }

                try
                {
                    for( ; nCnt && nCount; --nCnt, --nCount )
                        (pTmpUndoMgr->*fnDo)();
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("starmath");
                }
            }
            Repaint();
            UpdateText();
            SfxViewFrame* pFrm = SfxViewFrame::GetFirst( this );
            while( pFrm )
            {
                SfxBindings& rBind = pFrm->GetBindings();
                rBind.Invalidate(SID_UNDO);
                rBind.Invalidate(SID_REDO);
                rBind.Invalidate(SID_REPEAT);
                rBind.Invalidate(SID_CLEARHISTORY);
                pFrm = SfxViewFrame::GetNext( *pFrm, this );
            }
        }
        break;
    }

    rReq.Done();
}


void SmDocShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);

    for (sal_uInt16 nWh = aIter.FirstWhich();  0 != nWh;  nWh = aIter.NextWhich())
    {
        switch (nWh)
        {
        case SID_TEXTMODE:
            rSet.Put(SfxBoolItem(SID_TEXTMODE, GetFormat().IsTextmode()));
            break;

        case SID_DOCTEMPLATE :
            rSet.DisableItem(SID_DOCTEMPLATE);
            break;

        case SID_AUTO_REDRAW :
            {
                SmModule  *pp = SM_MOD();
                bool       bRedraw = pp->GetConfig()->IsAutoRedraw();

                rSet.Put(SfxBoolItem(SID_AUTO_REDRAW, bRedraw));
            }
            break;

        case SID_MODIFYSTATUS:
            {
                sal_Unicode cMod = ' ';
                if (IsModified())
                    cMod = '*';
                rSet.Put(SfxStringItem(SID_MODIFYSTATUS, OUString(cMod)));
            }
            break;

        case SID_TEXT:
            rSet.Put(SfxStringItem(SID_TEXT, GetText()));
            break;

        case SID_GRAPHIC_SM:
            //! very old (pre UNO) and ugly hack to invalidate the SmGraphicWidget.
            //! If mnModifyCount gets changed then the call below will implicitly notify
            //! SmGraphicController::StateChanged and there the window gets invalidated.
            //! Thus all the 'mnModifyCount++' before invalidating this slot.
            rSet.Put(SfxInt16Item(SID_GRAPHIC_SM, mnModifyCount));
            break;

        case SID_UNDO:
        case SID_REDO:
            {
                SfxViewFrame* pFrm = SfxViewFrame::GetFirst( this );
                if( pFrm )
                    pFrm->GetSlotState( nWh, nullptr, &rSet );
                else
                    rSet.DisableItem( nWh );
            }
            break;

        case SID_GETUNDOSTRINGS:
        case SID_GETREDOSTRINGS:
            {
                SfxUndoManager* pTmpUndoMgr = GetUndoManager();
                if( pTmpUndoMgr )
                {
                    OUString(SfxUndoManager::*fnGetComment)( size_t, bool const ) const;

                    size_t nCount;
                    if( SID_GETUNDOSTRINGS == nWh )
                    {
                        nCount = pTmpUndoMgr->GetUndoActionCount();
                        fnGetComment = &SfxUndoManager::GetUndoActionComment;
                    }
                    else
                    {
                        nCount = pTmpUndoMgr->GetRedoActionCount();
                        fnGetComment = &SfxUndoManager::GetRedoActionComment;
                    }
                    if (nCount)
                    {
                        OUStringBuffer aBuf;
                        for (size_t n = 0; n < nCount; ++n)
                        {
                            aBuf.append((pTmpUndoMgr->*fnGetComment)( n, SfxUndoManager::TopLevel ));
                            aBuf.append('\n');
                        }

                        SfxStringListItem aItem( nWh );
                        aItem.SetString( aBuf.makeStringAndClear() );
                        rSet.Put( aItem );
                    }
                }
                else
                    rSet.DisableItem( nWh );
            }
            break;
        }
    }
}


SfxUndoManager *SmDocShell::GetUndoManager()
{
    if (!mpEditEngine)
        GetEditEngine();
    return &mpEditEngine->GetUndoManager();
}


void SmDocShell::SaveSymbols()
{
    SmModule *pp = SM_MOD();
    pp->GetSymbolManager().Save();
}


void SmDocShell::Draw(OutputDevice *pDevice,
                      const JobSetup &,
                      sal_uInt16 /*nAspect*/,
                      bool /*bOutputForScreen*/)
{
    pDevice->IntersectClipRegion(GetVisArea());
    Point atmppoint;
    DrawFormula(*pDevice, atmppoint);
}

SfxItemPool& SmDocShell::GetPool()
{
    return SfxGetpApp()->GetPool();
}

void SmDocShell::SetVisArea(const tools::Rectangle & rVisArea)
{
    tools::Rectangle aNewRect(rVisArea);

    aNewRect.SetPos(Point());

    if (aNewRect.IsWidthEmpty())
        aNewRect.SetRight( 2000 );
    if (aNewRect.IsHeightEmpty())
        aNewRect.SetBottom( 1000 );

    bool bIsEnabled = IsEnableSetModified();
    if ( bIsEnabled )
        EnableSetModified( false );

    //TODO/LATER: it's unclear how this interacts with the SFX code
    // If outplace editing, then don't resize the OutplaceWindow. But the
    // ObjectShell has to resize.
    bool bUnLockFrame;
    if( GetCreateMode() == SfxObjectCreateMode::EMBEDDED && !IsInPlaceActive() && GetFrame() )
    {
        GetFrame()->LockAdjustPosSizePixel();
        bUnLockFrame = true;
    }
    else
        bUnLockFrame = false;

    SfxObjectShell::SetVisArea( aNewRect );

    if( bUnLockFrame )
        GetFrame()->UnlockAdjustPosSizePixel();

    if ( bIsEnabled )
        EnableSetModified( bIsEnabled );
}


void SmDocShell::FillClass(SvGlobalName* pClassName,
                           SotClipboardFormatId*  pFormat,
                           OUString* pFullTypeName,
                           sal_Int32 nFileFormat,
                           bool bTemplate /* = false */) const
{
    if (nFileFormat == SOFFICE_FILEFORMAT_60 )
    {
        *pClassName     = SvGlobalName(SO3_SM_CLASSID_60);
        *pFormat        = SotClipboardFormatId::STARMATH_60;
        *pFullTypeName  = SmResId(STR_MATH_DOCUMENT_FULLTYPE_CURRENT);
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_8 )
    {
        *pClassName     = SvGlobalName(SO3_SM_CLASSID_60);
        *pFormat        = bTemplate ? SotClipboardFormatId::STARMATH_8_TEMPLATE : SotClipboardFormatId::STARMATH_8;
        *pFullTypeName  = SmResId(STR_MATH_DOCUMENT_FULLTYPE_CURRENT);
    }
}

void SmDocShell::SetModified(bool bModified)
{
    if( IsEnableSetModified() )
    {
        SfxObjectShell::SetModified( bModified );
        Broadcast(SfxHint(SfxHintId::DocChanged));
    }
}

bool SmDocShell::WriteAsMathType3( SfxMedium& rMedium )
{
    OUStringBuffer aTextAsBuffer(maText);
    MathType aEquation(aTextAsBuffer, mpTree.get());
    return aEquation.ConvertFromStarMath( rMedium );
}

void SmDocShell::SetRightToLeft(bool bRTL)
{
    SmFormat aOldFormat = GetFormat();
    if (aOldFormat.IsRightToLeft() == bRTL)
        return;

    SmFormat aNewFormat(aOldFormat);
    aNewFormat.SetRightToLeft(bRTL);

    SfxUndoManager* pTmpUndoMgr = GetUndoManager();
    if (pTmpUndoMgr)
        pTmpUndoMgr->AddUndoAction(
            std::make_unique<SmFormatAction>(this, aOldFormat, aNewFormat));

    SetFormat(aNewFormat);
    Repaint();
}

static Size GetTextLineSize(OutputDevice const& rDevice, const OUString& rLine)
{
    Size aSize(rDevice.GetTextWidth(rLine), rDevice.GetTextHeight());
    const tools::Long nTabPos = rLine.isEmpty() ? 0 : rDevice.approximate_digit_width() * 8;

    if (nTabPos)
    {
        aSize.setWidth(0);
        sal_Int32 nPos = 0;
        do
        {
            if (nPos > 0)
                aSize.setWidth(((aSize.Width() / nTabPos) + 1) * nTabPos);

            const OUString aText = rLine.getToken(0, '\t', nPos);
            aSize.AdjustWidth(rDevice.GetTextWidth(aText));
        } while (nPos >= 0);
    }

    return aSize;
}

static Size GetTextSize(OutputDevice const& rDevice, std::u16string_view rText,
                        tools::Long MaxWidth)
{
    Size aSize;
    Size aTextSize;
    if (rText.empty())
        return aTextSize;

    sal_Int32 nPos = 0;
    do
    {
        OUString aLine(o3tl::getToken(rText, 0, '\n', nPos));
        aLine = aLine.replaceAll("\r", "");

        aSize = GetTextLineSize(rDevice, aLine);

        if (aSize.Width() > MaxWidth)
        {
            do
            {
                OUString aText;
                sal_Int32 m = aLine.getLength();
                sal_Int32 nLen = m;

                for (sal_Int32 n = 0; n < nLen; n++)
                {
                    sal_Unicode cLineChar = aLine[n];
                    if ((cLineChar == ' ') || (cLineChar == '\t'))
                    {
                        aText = aLine.copy(0, n);
                        if (GetTextLineSize(rDevice, aText).Width() < MaxWidth)
                            m = n;
                        else
                            break;
                    }
                }

                aText = aLine.copy(0, m);
                aLine = aLine.replaceAt(0, m, u"");
                aSize = GetTextLineSize(rDevice, aText);
                aTextSize.AdjustHeight(aSize.Height());
                aTextSize.setWidth(std::clamp(aSize.Width(), aTextSize.Width(), MaxWidth));

                aLine = comphelper::string::stripStart(aLine, ' ');
                aLine = comphelper::string::stripStart(aLine, '\t');
                aLine = comphelper::string::stripStart(aLine, ' ');
            } while (!aLine.isEmpty());
        }
        else
        {
            aTextSize.AdjustHeight(aSize.Height());
            aTextSize.setWidth(std::max(aTextSize.Width(), aSize.Width()));
        }
    } while (nPos >= 0);

    return aTextSize;
}

static void DrawTextLine(OutputDevice& rDevice, const Point& rPosition, const OUString& rLine)
{
    Point aPoint(rPosition);
    const tools::Long nTabPos = rLine.isEmpty() ? 0 : rDevice.approximate_digit_width() * 8;

    if (nTabPos)
    {
        sal_Int32 nPos = 0;
        do
        {
            if (nPos > 0)
                aPoint.setX(((aPoint.X() / nTabPos) + 1) * nTabPos);

            OUString aText = rLine.getToken(0, '\t', nPos);
            rDevice.DrawText(aPoint, aText);
            aPoint.AdjustX(rDevice.GetTextWidth(aText));
        } while (nPos >= 0);
    }
    else
        rDevice.DrawText(aPoint, rLine);
}

static void DrawText(OutputDevice& rDevice, const Point& rPosition, std::u16string_view rText,
                     sal_uInt16 MaxWidth)
{
    if (rText.empty())
        return;

    Point aPoint(rPosition);
    Size aSize;

    sal_Int32 nPos = 0;
    do
    {
        OUString aLine(o3tl::getToken(rText, 0, '\n', nPos));
        aLine = aLine.replaceAll("\r", "");
        aSize = GetTextLineSize(rDevice, aLine);
        if (aSize.Width() > MaxWidth)
        {
            do
            {
                OUString aText;
                sal_Int32 m = aLine.getLength();
                sal_Int32 nLen = m;

                for (sal_Int32 n = 0; n < nLen; n++)
                {
                    sal_Unicode cLineChar = aLine[n];
                    if ((cLineChar == ' ') || (cLineChar == '\t'))
                    {
                        aText = aLine.copy(0, n);
                        if (GetTextLineSize(rDevice, aText).Width() < MaxWidth)
                            m = n;
                        else
                            break;
                    }
                }
                aText = aLine.copy(0, m);
                aLine = aLine.replaceAt(0, m, u"");

                DrawTextLine(rDevice, aPoint, aText);
                aPoint.AdjustY(aSize.Height());

                aLine = comphelper::string::stripStart(aLine, ' ');
                aLine = comphelper::string::stripStart(aLine, '\t');
                aLine = comphelper::string::stripStart(aLine, ' ');
            } while (GetTextLineSize(rDevice, aLine).Width() > MaxWidth);

            // print the remaining text
            if (!aLine.isEmpty())
            {
                DrawTextLine(rDevice, aPoint, aLine);
                aPoint.AdjustY(aSize.Height());
            }
        }
        else
        {
            DrawTextLine(rDevice, aPoint, aLine);
            aPoint.AdjustY(aSize.Height());
        }
    } while (nPos >= 0);
}

void SmDocShell::Impl_Print(OutputDevice& rOutDev, const SmPrintUIOptions& rPrintUIOptions,
                tools::Rectangle aOutRect)
{
    const bool bIsPrintTitle = rPrintUIOptions.getBoolValue(PRTUIOPT_TITLE_ROW, true);
    const bool bIsPrintFrame = rPrintUIOptions.getBoolValue(PRTUIOPT_BORDER, true);
    const bool bIsPrintFormulaText = rPrintUIOptions.getBoolValue(PRTUIOPT_FORMULA_TEXT, true);
    SmPrintSize ePrintSize(static_cast<SmPrintSize>(
        rPrintUIOptions.getIntValue(PRTUIOPT_PRINT_FORMAT, PRINT_SIZE_NORMAL)));
    const sal_uInt16 nZoomFactor
        = static_cast<sal_uInt16>(rPrintUIOptions.getIntValue(PRTUIOPT_PRINT_SCALE, 100));

    rOutDev.Push();
    rOutDev.SetLineColor(COL_BLACK);

    // output text on top
    if (bIsPrintTitle)
    {
        Size aSize600(0, 600);
        Size aSize650(0, 650);
        vcl::Font aFont(FAMILY_DONTKNOW, aSize600);

        aFont.SetAlignment(ALIGN_TOP);
        aFont.SetWeight(WEIGHT_BOLD);
        aFont.SetFontSize(aSize650);
        aFont.SetColor(COL_BLACK);
        rOutDev.SetFont(aFont);

        Size aTitleSize(GetTextSize(rOutDev, GetTitle(), aOutRect.GetWidth() - 200));

        aFont.SetWeight(WEIGHT_NORMAL);
        aFont.SetFontSize(aSize600);
        rOutDev.SetFont(aFont);

        Size aDescSize(GetTextSize(rOutDev, GetComment(), aOutRect.GetWidth() - 200));

        if (bIsPrintFrame)
            rOutDev.DrawRect(tools::Rectangle(
                aOutRect.TopLeft(), Size(aOutRect.GetWidth(), 100 + aTitleSize.Height() + 200
                                                                  + aDescSize.Height() + 100)));
        aOutRect.AdjustTop(200);

        // output title
        aFont.SetWeight(WEIGHT_BOLD);
        aFont.SetFontSize(aSize650);
        rOutDev.SetFont(aFont);
        Point aPoint(aOutRect.Left() + (aOutRect.GetWidth() - aTitleSize.Width()) / 2,
                     aOutRect.Top());
        DrawText(rOutDev, aPoint, GetTitle(),
                 sal::static_int_cast<sal_uInt16>(aOutRect.GetWidth() - 200));
        aOutRect.AdjustTop(aTitleSize.Height() + 200);

        // output description
        aFont.SetWeight(WEIGHT_NORMAL);
        aFont.SetFontSize(aSize600);
        rOutDev.SetFont(aFont);
        aPoint.setX(aOutRect.Left() + (aOutRect.GetWidth() - aDescSize.Width()) / 2);
        aPoint.setY(aOutRect.Top());
        DrawText(rOutDev, aPoint, GetComment(),
                 sal::static_int_cast<sal_uInt16>(aOutRect.GetWidth() - 200));
        aOutRect.AdjustTop(aDescSize.Height() + 300);
    }

    // output text on bottom
    if (bIsPrintFormulaText)
    {
        vcl::Font aFont(FAMILY_DONTKNOW, Size(0, 600));
        aFont.SetAlignment(ALIGN_TOP);
        aFont.SetColor(COL_BLACK);

        // get size
        rOutDev.SetFont(aFont);

        Size aSize(GetTextSize(rOutDev, GetText(), aOutRect.GetWidth() - 200));

        aOutRect.AdjustBottom(-(aSize.Height() + 600));

        if (bIsPrintFrame)
            rOutDev.DrawRect(tools::Rectangle(
                aOutRect.BottomLeft(), Size(aOutRect.GetWidth(), 200 + aSize.Height() + 200)));

        Point aPoint(aOutRect.Left() + (aOutRect.GetWidth() - aSize.Width()) / 2,
                     aOutRect.Bottom() + 300);
        DrawText(rOutDev, aPoint, GetText(),
                 sal::static_int_cast<sal_uInt16>(aOutRect.GetWidth() - 200));
        aOutRect.AdjustBottom(-200);
    }

    if (bIsPrintFrame)
        rOutDev.DrawRect(aOutRect);

    aOutRect.AdjustTop(100);
    aOutRect.AdjustLeft(100);
    aOutRect.AdjustBottom(-100);
    aOutRect.AdjustRight(-100);

    Size aSize(GetSize());

    MapMode OutputMapMode;
    switch (ePrintSize)
    {
        case PRINT_SIZE_NORMAL:
            OutputMapMode = MapMode(SmMapUnit());
            break;

        case PRINT_SIZE_SCALED:
            if (!aSize.IsEmpty())
            {
                sal_uInt16 nZ
                    = std::min(o3tl::convert(aOutRect.GetWidth(), 100, aSize.Width()),
                               o3tl::convert(aOutRect.GetHeight(), 100, aSize.Height()));
                if (bIsPrintFrame && nZ > MINZOOM)
                    nZ -= 10;
                Fraction aFraction(std::clamp(nZ, MINZOOM, MAXZOOM), 100);

                OutputMapMode = MapMode(SmMapUnit(), Point(), aFraction, aFraction);
            }
            else
                OutputMapMode = MapMode(SmMapUnit());
            break;

        case PRINT_SIZE_ZOOMED:
        {
            Fraction aFraction(nZoomFactor, 100);

            OutputMapMode = MapMode(SmMapUnit(), Point(), aFraction, aFraction);
            break;
        }
    }

    aSize = OutputDevice::LogicToLogic(aSize, OutputMapMode, MapMode(SmMapUnit()));

    Point aPos(aOutRect.Left() + (aOutRect.GetWidth() - aSize.Width()) / 2,
               aOutRect.Top() + (aOutRect.GetHeight() - aSize.Height()) / 2);

    aPos = OutputDevice::LogicToLogic(aPos, MapMode(SmMapUnit()), OutputMapMode);
    aOutRect = OutputDevice::LogicToLogic(aOutRect, MapMode(SmMapUnit()), OutputMapMode);

    rOutDev.SetMapMode(OutputMapMode);
    rOutDev.SetClipRegion(vcl::Region(aOutRect));
    DrawFormula(rOutDev, aPos);
    rOutDev.SetClipRegion();

    rOutDev.Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
