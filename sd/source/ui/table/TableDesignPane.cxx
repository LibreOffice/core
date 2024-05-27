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

#include <string_view>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

#include <comphelper/sequence.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <svl/style.hxx>
#include <svl/stritem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <svx/svdetc.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdr/table/tabledesign.hxx>
#include <svx/sdr/table/tablecontroller.hxx>
#include <o3tl/enumrange.hxx>

#include <TableDesignPane.hxx>

#include <stlsheet.hxx>
#include <strings.hrc>
#include <sdresid.hxx>
#include <bitmaps.hlst>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <EventMultiplexer.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

namespace sd {

const sal_Int32 nPreviewColumns = 5;
const sal_Int32 nPreviewRows = 5;
const sal_Int32 nCellWidth = 12; // one pixel is shared with the next cell!
const sal_Int32 nCellHeight = 7; // one pixel is shared with the next cell!
const sal_Int32 nBitmapWidth = (nCellWidth * nPreviewColumns) - (nPreviewColumns - 1);
const sal_Int32 nBitmapHeight = (nCellHeight * nPreviewRows) - (nPreviewRows - 1);

const std::u16string_view gPropNames[CB_COUNT] =
{
    u"UseFirstRowStyle",
    u"UseLastRowStyle",
    u"UseBandingRowStyle",
    u"UseFirstColumnStyle",
    u"UseLastColumnStyle",
    u"UseBandingColumnStyle"
};

constexpr std::u16string_view aTableStyleBaseName = u"table";

TableDesignWidget::TableDesignWidget(weld::Builder& rBuilder, ViewShellBase& rBase)
    : mrBase(rBase)
    , m_xMenu(rBuilder.weld_menu(u"menu"_ustr))
    , m_xValueSet(new TableValueSet(rBuilder.weld_scrolled_window(u"previewswin"_ustr, true)))
    , m_xValueSetWin(new weld::CustomWeld(rBuilder, u"previews"_ustr, *m_xValueSet))
{
    m_xValueSet->SetStyle(m_xValueSet->GetStyle() | WB_NO_DIRECTSELECT | WB_FLATVALUESET | WB_ITEMBORDER);
    m_xValueSet->SetExtraSpacing(8);
    m_xValueSet->setModal(false);
    m_xValueSet->SetColor();
    m_xValueSet->SetSelectHdl(LINK(this, TableDesignWidget, implValueSetHdl));
    m_xValueSet->SetContextMenuHandler(LINK(this, TableDesignWidget, implContextMenuHandler));

    for (sal_uInt16 i = CB_HEADER_ROW; i <= CB_BANDED_COLUMNS; ++i)
    {
        m_aCheckBoxes[i] = rBuilder.weld_check_button(OUString(gPropNames[i]));
        m_aCheckBoxes[i]->connect_toggled(LINK(this, TableDesignWidget, implCheckBoxHdl));
    }

    // get current controller and initialize listeners
    try
    {
        mxView.set(mrBase.GetController(), UNO_QUERY);
        addListener();

        Reference< XController > xController( mrBase.GetController(), UNO_SET_THROW );
        Reference< XStyleFamiliesSupplier > xFamiliesSupp( xController->getModel(), UNO_QUERY_THROW );
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        mxTableFamily.set( xFamilies->getByName( u"table"_ustr ), UNO_QUERY_THROW );
        mxCellFamily.set( xFamilies->getByName( u"cell"_ustr ), UNO_QUERY_THROW );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationPane::CustomAnimationPane()" );
    }

    onSelectionChanged();
    updateControls();
}

TableDesignWidget::~TableDesignWidget()
{
    removeListener();
}

void TableDesignWidget::setDocumentModified()
{
    try
    {
        Reference<XController> xController(mrBase.GetController(), UNO_SET_THROW);
        Reference<util::XModifiable> xModifiable(xController->getModel(), UNO_QUERY_THROW);
        xModifiable->setModified(true);
    }
    catch (Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "TableDesignWidget::setDocumentModified()");
    }
}

IMPL_LINK(TableDesignWidget, implContextMenuHandler, const Point*, pPoint, void)
{
    auto nClickedItemId = pPoint ? m_xValueSet->GetItemId(*pPoint) : m_xValueSet->GetSelectedItemId();

    try
    {
        if (nClickedItemId > mxTableFamily->getCount())
            return;

        if (nClickedItemId)
        {
            Reference<XStyle> xStyle(mxTableFamily->getByIndex(nClickedItemId - 1), UNO_QUERY_THROW);

            m_xMenu->set_visible(u"clone"_ustr, true);
            m_xMenu->set_visible(u"format"_ustr, true);
            m_xMenu->set_visible(u"delete"_ustr, xStyle->isUserDefined());
            m_xMenu->set_visible(u"reset"_ustr, !xStyle->isUserDefined());
            m_xMenu->set_sensitive(u"reset"_ustr, Reference<util::XModifiable>(xStyle, UNO_QUERY_THROW)->isModified());
        }
        else
        {
            m_xMenu->set_visible(u"clone"_ustr, false);
            m_xMenu->set_visible(u"format"_ustr, false);
            m_xMenu->set_visible(u"delete"_ustr, false);
            m_xMenu->set_visible(u"reset"_ustr, false);
        }
    }
    catch (Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "TableDesignWidget::implContextMenuHandler()");
    }

    m_xValueSet->SelectItem(nClickedItemId);

    Point aPosition = pPoint ? *pPoint : m_xValueSet->GetItemRect(nClickedItemId).Center();
    OUString aCommand = m_xMenu->popup_at_rect(m_xValueSet->GetDrawingArea(), ::tools::Rectangle(aPosition, Size(1,1)));

    if (aCommand == "new")
        InsertStyle();
    else if (aCommand == "clone")
        CloneStyle();
    else if (aCommand == "delete")
        DeleteStyle();
    else if (aCommand == "reset")
        ResetStyle();
    else if (!aCommand.isEmpty())
        EditStyle(aCommand);
}

namespace
{
    OUString getNewStyleName(const Reference<XNameContainer>& rFamily, std::u16string_view rBaseName)
    {
        OUString aName(rBaseName);
        sal_Int32 nIndex = 1;
        while(rFamily->hasByName(aName))
        {
            aName = rBaseName + OUString::number(nIndex++);
        }

        return aName;
    }
}

void TableDesignWidget::InsertStyle()
{
    try
    {
        Reference<XSingleServiceFactory> xFactory(mxTableFamily, UNO_QUERY_THROW);
        Reference<XNameContainer> xTableFamily(mxTableFamily, UNO_QUERY_THROW);
        Reference<XNameReplace> xTableStyle(xFactory->createInstance(), UNO_QUERY_THROW);
        const OUString aName(getNewStyleName(xTableFamily, aTableStyleBaseName));
        xTableFamily->insertByName(aName, Any(xTableStyle));

        Reference<XStyle> xCellStyle(mxCellFamily->getByName(u"default"_ustr), UNO_QUERY_THROW);

        xTableStyle->replaceByName(u"body"_ustr, Any(xCellStyle));
        xTableStyle->replaceByName(u"odd-rows"_ustr , Any(xCellStyle));
        xTableStyle->replaceByName(u"odd-columns"_ustr , Any(xCellStyle));
        xTableStyle->replaceByName(u"first-row"_ustr , Any(xCellStyle));
        xTableStyle->replaceByName(u"first-column"_ustr , Any(xCellStyle));
        xTableStyle->replaceByName(u"last-row"_ustr , Any(xCellStyle));
        xTableStyle->replaceByName(u"last-column"_ustr , Any(xCellStyle));

        updateControls();
        selectStyle(aName);
        setDocumentModified();
    }
    catch (Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "TableDesignWidget::InsertStyle()");
    }
}

void TableDesignWidget::CloneStyle()
{
    try
    {
        Reference<XNameAccess> xSrcTableStyle(mxTableFamily->getByIndex(m_xValueSet->GetSelectedItemId() - 1), UNO_QUERY_THROW);

        Reference<XSingleServiceFactory> xFactory(mxTableFamily, UNO_QUERY_THROW);
        Reference<XNameContainer> xTableFamily(mxTableFamily, UNO_QUERY_THROW);
        Reference<XNameReplace> xDestTableStyle(xFactory->createInstance(), UNO_QUERY_THROW);
        const OUString aName(getNewStyleName(xTableFamily, aTableStyleBaseName));
        xTableFamily->insertByName(aName, Any(xDestTableStyle));

        auto aNames(xSrcTableStyle->getElementNames());
        for (const auto& name : aNames)
        {
            Reference<XStyle> xSrcCellStyle(xSrcTableStyle->getByName(name), UNO_QUERY);
            if (xSrcCellStyle && xSrcCellStyle->isUserDefined())
            {
                Reference<XSingleServiceFactory> xCellFactory(mxCellFamily, UNO_QUERY_THROW);
                Reference<XStyle> xDestCellStyle(xCellFactory->createInstance(), UNO_QUERY_THROW);
                xDestCellStyle->setParentStyle(xSrcCellStyle->getParentStyle());
                const OUString aStyleName(getNewStyleName(mxCellFamily, Concat2View(aName + "-" + name)));
                mxCellFamily->insertByName(aStyleName, Any(xDestCellStyle));

                rtl::Reference xSrcStyleSheet = static_cast<SdStyleSheet*>(xSrcCellStyle.get());
                rtl::Reference xDestStyleSheet = static_cast<SdStyleSheet*>(xDestCellStyle.get());

                xDestStyleSheet->GetItemSet().Put(xSrcStyleSheet->GetItemSet());

                xDestTableStyle->replaceByName(name, Any(xDestCellStyle));
            }
            else
                xDestTableStyle->replaceByName(name, Any(xSrcCellStyle));
        }

        updateControls();
        selectStyle(aName);
        setDocumentModified();
    }
    catch (Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "TableDesignWidget::CloneStyle()");
    }
}

void TableDesignWidget::ResetStyle()
{
    try
    {
        Reference<XIndexReplace> xTableStyle(mxTableFamily->getByIndex(m_xValueSet->GetSelectedItemId() - 1), UNO_QUERY_THROW);

        for (sal_Int32 i = 0; i < xTableStyle->getCount(); ++i)
        {
            Reference<XStyle> xCellStyle(xTableStyle->getByIndex(i), UNO_QUERY);
            while (xCellStyle && xCellStyle->isUserDefined() && !xCellStyle->getParentStyle().isEmpty())
                xCellStyle.set(mxCellFamily->getByName(xCellStyle->getParentStyle()), UNO_QUERY);

            xTableStyle->replaceByIndex(i, Any(xCellStyle));
        }

        endTextEditForStyle(xTableStyle);
        Reference<util::XModifiable>(xTableStyle, UNO_QUERY_THROW)->setModified(false);

        updateControls();
        setDocumentModified();
    }
    catch (Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "TableDesignWidget::ResetStyle()");
    }
}

void TableDesignWidget::DeleteStyle()
{
    try
    {
        Reference<XStyle> xTableStyle(mxTableFamily->getByIndex(m_xValueSet->GetSelectedItemId() - 1), UNO_QUERY_THROW);

        if (xTableStyle->isInUse())
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                m_xValueSet->GetDrawingArea(), VclMessageType::Question, VclButtonsType::YesNo, SdResId(STR_REMOVE_TABLESTYLE)));

            if (xBox->run() != RET_YES)
                return;

            endTextEditForStyle(xTableStyle);
        }

        Reference<XNameContainer>(mxTableFamily, UNO_QUERY_THROW)->removeByName(xTableStyle->getName());

        updateControls();
        setDocumentModified();
    }
    catch (Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "TableDesignWidget::DeleteStyle()");
    }
}

void TableDesignWidget::EditStyle(const OUString& rCommand)
{
    try
    {
        Reference<XNameReplace> xTableStyle(mxTableFamily->getByIndex(m_xValueSet->GetSelectedItemId() - 1), UNO_QUERY_THROW);
        Reference<XStyle> xCellStyle(xTableStyle->getByName(rCommand), UNO_QUERY_THROW);
        rtl::Reference xStyleSheet = static_cast<SdStyleSheet*>(xCellStyle.get());

        bool bUserDefined = xStyleSheet->IsEditable();
        if (!bUserDefined)
        {
            Reference<XSingleServiceFactory> xFactory(mxCellFamily, UNO_QUERY_THROW);
            xCellStyle.set(xFactory->createInstance(), UNO_QUERY_THROW);
            xCellStyle->setParentStyle(xStyleSheet->getName());
            xStyleSheet = static_cast<SdStyleSheet*>(xCellStyle.get());
        }

        SfxItemSet aNewAttr(xStyleSheet->GetItemSet());

        // merge drawing layer text distance items into SvxBoxItem used by the dialog
        SvxBoxItem aBoxItem(sdr::table::SvxTableController::TextDistancesToSvxBoxItem(aNewAttr));
        aNewAttr.Put(aBoxItem);

        // inner borders do not apply to a cell style
        SvxBoxInfoItem aBoxInfoItem(aNewAttr.Get(SDRATTR_TABLE_BORDER_INNER));
        aBoxInfoItem.SetTable(false);
        aNewAttr.Put(aBoxInfoItem);

        SdrView* pDrawView = mrBase.GetDrawView();
        if (!pDrawView)
            return;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact ? pFact->CreateSvxFormatCellsDialog(
            mrBase.GetFrameWeld(), aNewAttr, pDrawView->GetModel(), true) : nullptr);
        if (pDlg && pDlg->Execute() == RET_OK)
        {
            endTextEditForStyle(xTableStyle);

            if (!bUserDefined)
            {
                Reference<XNamed> xNamed(xTableStyle, UNO_QUERY_THROW);
                const OUString aStyleName(getNewStyleName(mxCellFamily, Concat2View(xNamed->getName() + "-" + rCommand)));
                mxCellFamily->insertByName(aStyleName, Any(xCellStyle));
                xTableStyle->replaceByName(rCommand, Any(xCellStyle));
            }

            SfxItemSet aNewSet(*pDlg->GetOutputItemSet());
            sdr::table::SvxTableController::SvxBoxItemToTextDistances(aBoxItem, aNewSet);
            sdr::properties::CleanupFillProperties(aNewSet);
            xStyleSheet->GetItemSet().Put(aNewSet);
            xStyleSheet->Broadcast(SfxHint(SfxHintId::DataChanged));

            updateControls();
            setDocumentModified();
        }
    }
    catch (Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "TableDesignWidget::EditStyle()");
    }
}

static SfxBindings* getBindings( ViewShellBase const & rBase )
{
    if( rBase.GetMainViewShell() && rBase.GetMainViewShell()->GetViewFrame() )
        return &rBase.GetMainViewShell()->GetViewFrame()->GetBindings();
    else
        return nullptr;
}

static SfxDispatcher* getDispatcher( ViewShellBase const & rBase )
{
    if( rBase.GetMainViewShell() && rBase.GetMainViewShell()->GetViewFrame() )
        return rBase.GetMainViewShell()->GetViewFrame()->GetDispatcher();
    else
        return nullptr;
}

IMPL_LINK_NOARG(TableDesignWidget, implValueSetHdl, ValueSet*, void)
{
    ApplyStyle();
}

void TableDesignWidget::ApplyStyle()
{
    try
    {
        OUString sStyleName;
        sal_Int32 nIndex = static_cast< sal_Int32 >( m_xValueSet->GetSelectedItemId() ) - 1;

        if( (nIndex >= 0) && (nIndex < mxTableFamily->getCount()) )
        {
            Reference< XNameAccess > xNames( mxTableFamily, UNO_QUERY_THROW );
            sStyleName = xNames->getElementNames()[nIndex];
        }
        else if (nIndex == mxTableFamily->getCount())
        {
            InsertStyle();
            return;
        }

        if( sStyleName.isEmpty() )
            return;

        if( mxSelectedTable.is() )
        {
            if (SdrView* pView = mrBase.GetDrawView())
            {
                if (pView->IsTextEdit())
                    pView->SdrEndTextEdit();

                SfxRequest aReq( SID_TABLE_STYLE, SfxCallMode::SYNCHRON, SfxGetpApp()->GetPool() );
                aReq.AppendItem( SfxStringItem( SID_TABLE_STYLE, sStyleName ) );

                const rtl::Reference< sdr::SelectionController >& xController( pView->getSelectionController() );
                if( xController.is() )
                    xController->Execute( aReq );

                SfxBindings* pBindings = getBindings( mrBase );
                if( pBindings )
                {
                    pBindings->Invalidate( SID_UNDO );
                    pBindings->Invalidate( SID_REDO );
                }
            }
            setDocumentModified();
        }
        else
        {
            SfxDispatcher* pDispatcher = getDispatcher( mrBase );
            SfxStringItem aArg( SID_TABLE_STYLE, sStyleName );
            pDispatcher->ExecuteList(SID_INSERT_TABLE, SfxCallMode::ASYNCHRON,
                    { &aArg });
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "TableDesignWidget::implValueSetHdl()");
    }
}

IMPL_LINK_NOARG(TableDesignWidget, implCheckBoxHdl, weld::Toggleable&, void)
{
    ApplyOptions();
    FillDesignPreviewControl();
}

void TableDesignWidget::ApplyOptions()
{
    static const sal_uInt16 gParamIds[CB_COUNT] =
    {
        ID_VAL_USEFIRSTROWSTYLE, ID_VAL_USELASTROWSTYLE, ID_VAL_USEBANDINGROWSTYLE,
        ID_VAL_USEFIRSTCOLUMNSTYLE, ID_VAL_USELASTCOLUMNSTYLE, ID_VAL_USEBANDINGCOLUMNSTYLE
    };

    if( !mxSelectedTable.is() )
        return;

    SfxRequest aReq( SID_TABLE_STYLE_SETTINGS, SfxCallMode::SYNCHRON, SfxGetpApp()->GetPool() );

    for( sal_uInt16 i = CB_HEADER_ROW; i <= CB_BANDED_COLUMNS; ++i )
    {
        aReq.AppendItem( SfxBoolItem( gParamIds[i], m_aCheckBoxes[i]->get_active() ) );
    }

    SdrView* pView = mrBase.GetDrawView();
    if( !pView )
        return;

    const rtl::Reference< sdr::SelectionController >& xController( pView->getSelectionController() );
    if( xController.is() )
    {
        xController->Execute( aReq );

        SfxBindings* pBindings = getBindings( mrBase );
        if( pBindings )
        {
            pBindings->Invalidate( SID_UNDO );
            pBindings->Invalidate( SID_REDO );
        }
    }
    setDocumentModified();
}

void TableDesignWidget::onSelectionChanged()
{
    Reference< XPropertySet > xNewSelection;

    if( mxView.is() ) try
    {
        Reference< XSelectionSupplier >  xSel( mxView, UNO_QUERY_THROW );
        Any aSel( xSel->getSelection() );
        Sequence< XShape > xShapeSeq;
        if( aSel >>= xShapeSeq )
        {
            if( xShapeSeq.getLength() == 1 )
                aSel <<= xShapeSeq[0];
        }
        else
        {
            Reference< XShapes > xShapes( aSel, UNO_QUERY );
            if( xShapes.is() && (xShapes->getCount() == 1) )
                aSel = xShapes->getByIndex(0);
        }

        Reference< XShapeDescriptor > xDesc( aSel, UNO_QUERY );
        if( xDesc.is() && ( xDesc->getShapeType() == "com.sun.star.drawing.TableShape" || xDesc->getShapeType() == "com.sun.star.presentation.TableShape" ) )
        {
            xNewSelection.set( xDesc, UNO_QUERY );
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::TableDesignWidget::onSelectionChanged()" );
    }

    if( mxSelectedTable != xNewSelection )
    {
        mxSelectedTable = xNewSelection;
        updateControls();
    }
}

bool TableValueSet::Command(const CommandEvent& rEvent)
{
    if (rEvent.GetCommand() != CommandEventId::ContextMenu)
        return ValueSet::Command(rEvent);

    maContextMenuHandler.Call(rEvent.IsMouseEvent() ? &rEvent.GetMousePosPixel() : nullptr);
    return true;
}

void TableValueSet::Resize()
{
    ValueSet::Resize();
    // Calculate the number of rows and columns.
    if( GetItemCount() <= 0 )
        return;

    Size aValueSetSize = GetOutputSizePixel();

    Image aImage = GetItemImage(GetItemId(0));
    Size aItemSize = aImage.GetSizePixel();

    aItemSize.AdjustHeight(10 );
    int nColumnCount = (aValueSetSize.Width() - GetScrollWidth()) / aItemSize.Width();
    if (nColumnCount < 1)
        nColumnCount = 1;

    int nRowCount = (GetItemCount() + nColumnCount - 1) / nColumnCount;
    if (nRowCount < 1)
        nRowCount = 1;

    int nVisibleRowCount = std::min(nRowCount, getMaxRowCount());

    SetColCount (static_cast<sal_uInt16>(nColumnCount));
    SetLineCount (static_cast<sal_uInt16>(nVisibleRowCount));

    if( !m_bModal )
    {
        WinBits nStyle = GetStyle() & ~WB_VSCROLL;
        if( nRowCount > nVisibleRowCount )
        {
            nStyle |= WB_VSCROLL;
        }
        SetStyle( nStyle );
    }
}

TableValueSet::TableValueSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow)
    : ValueSet(std::move(pScrolledWindow))
    , m_bModal(false)
{
}

void TableValueSet::StyleUpdated()
{
    updateSettings();
}

void TableValueSet::updateSettings()
{
    if( !m_bModal )
    {
        Color aColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
        SetColor(aColor);
        SetExtraSpacing(8);
    }
}

void TableDesignWidget::updateControls()
{
    static const bool gDefaults[CB_COUNT] = { true, false, true, false, false, false };

    const bool bHasTable = mxSelectedTable.is();

    for (sal_uInt16 i = CB_HEADER_ROW; i <= CB_BANDED_COLUMNS; ++i)
    {
        bool bUse = gDefaults[i];
        if( bHasTable ) try
        {
            mxSelectedTable->getPropertyValue( OUString(gPropNames[i]) ) >>= bUse;
        }
        catch( Exception& )
        {
            TOOLS_WARN_EXCEPTION( "sd", "sd::TableDesignWidget::updateControls()");
        }
        m_aCheckBoxes[i]->set_active(bUse);
        m_aCheckBoxes[i]->set_sensitive(bHasTable);
    }

    FillDesignPreviewControl();
    m_xValueSet->updateSettings();
    m_xValueSet->Resize();

    if( mxSelectedTable.is() )
    {
        Reference< XNamed > xNamed( mxSelectedTable->getPropertyValue( u"TableTemplate"_ustr ), UNO_QUERY );
        if( xNamed.is() )
            selectStyle(xNamed->getName());
    }
}

void TableDesignWidget::selectStyle(std::u16string_view rStyle)
{
    Reference< XNameAccess > xNames( mxTableFamily, UNO_QUERY );
    if( xNames.is() )
    {
        Sequence< OUString > aNames( xNames->getElementNames() );
        sal_Int32 nIndex = comphelper::findValue(aNames, rStyle);
        if (nIndex != -1)
            m_xValueSet->SelectItem(static_cast<sal_uInt16>(nIndex) + 1);
    }
}

void TableDesignWidget::endTextEditForStyle(const Reference<XInterface>& rStyle)
{
    if (!mxSelectedTable)
        return;

    Reference<XInterface> xTableStyle(mxSelectedTable->getPropertyValue(u"TableTemplate"_ustr), UNO_QUERY);
    if (xTableStyle != rStyle)
        return;

    SdrView* pDrawView = mrBase.GetDrawView();
    if (pDrawView && pDrawView->IsTextEdit())
        pDrawView->SdrEndTextEdit();
}

void TableDesignWidget::addListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,TableDesignWidget,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener( aLink );
}

void TableDesignWidget::removeListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,TableDesignWidget,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

IMPL_LINK(TableDesignWidget,EventMultiplexerListener,
    tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::CurrentPageChanged:
        case EventMultiplexerEventId::EditViewSelection:
            onSelectionChanged();
            break;

        case EventMultiplexerEventId::MainViewRemoved:
            mxView.clear();
            onSelectionChanged();
            break;

        case EventMultiplexerEventId::MainViewAdded:
            mxView.set( mrBase.GetController(), UNO_QUERY );
            onSelectionChanged();
            break;

        default: break;
    }
}

namespace {

struct CellInfo
{
    Color maCellColor;
    Color maTextColor;
    std::shared_ptr<SvxBoxItem> maBorder;

    explicit CellInfo( const Reference< XStyle >& xStyle );
};

}

CellInfo::CellInfo( const Reference< XStyle >& xStyle )
: maBorder(std::make_shared<SvxBoxItem>(SDRATTR_TABLE_BORDER))
{
    SfxStyleSheet* pStyleSheet = SfxUnoStyleSheet::getUnoStyleSheet( xStyle );
    if( !pStyleSheet )
        return;

    SfxItemSet& rSet = pStyleSheet->GetItemSet();

    // get style fill color
    maCellColor = GetDraftFillColor(rSet).value_or(COL_TRANSPARENT);

    // get style text color
    const SvxColorItem* pTextColor = rSet.GetItem(EE_CHAR_COLOR);
    if( pTextColor )
        maTextColor = pTextColor->GetValue();
    else
        maTextColor = COL_TRANSPARENT;

    // get border
    const SvxBoxItem* pBoxItem = rSet.GetItem( SDRATTR_TABLE_BORDER );
    if( pBoxItem )
        maBorder.reset(pBoxItem->Clone());
}

typedef std::vector< std::shared_ptr< CellInfo > > CellInfoVector;
typedef std::shared_ptr< CellInfo > CellInfoMatrix[nPreviewColumns * nPreviewRows];

namespace {

struct TableStyleSettings
{
    bool mbUseFirstRow;
    bool mbUseLastRow;
    bool mbUseFirstColumn;
    bool mbUseLastColumn;
    bool mbUseRowBanding;
    bool mbUseColumnBanding;

    TableStyleSettings()
        : mbUseFirstRow(true)
        , mbUseLastRow(false)
        , mbUseFirstColumn(false)
        , mbUseLastColumn(false)
        , mbUseRowBanding(true)
        , mbUseColumnBanding(false) {}
};

}

static void FillCellInfoVector( const Reference< XIndexAccess >& xTableStyle, CellInfoVector& rVector )
{
    DBG_ASSERT( xTableStyle.is() && (xTableStyle->getCount() == sdr::table::style_count ), "sd::FillCellInfoVector(), invalid table style!" );
    if( !xTableStyle.is() )
        return;

    try
    {
        rVector.resize( sdr::table::style_count );

        for( sal_Int32 nStyle = 0; nStyle < sdr::table::style_count; ++nStyle )
        {
            Reference< XStyle > xStyle( xTableStyle->getByIndex( nStyle ), UNO_QUERY );
            if( xStyle.is() )
                rVector[nStyle] = std::make_shared<CellInfo>( xStyle );
        }
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::FillCellInfoVector()");
    }
}

static void FillCellInfoMatrix( const CellInfoVector& rStyle, const TableStyleSettings& rSettings, CellInfoMatrix& rMatrix )
{
    for( sal_Int32 nRow = 0; nRow < nPreviewColumns; ++nRow )
    {
        const bool bFirstRow = rSettings.mbUseFirstRow && (nRow == 0);
        const bool bLastRow = rSettings.mbUseLastRow && (nRow == nPreviewColumns - 1);

        for( sal_Int32 nCol = 0; nCol < nPreviewColumns; ++nCol )
        {
            std::shared_ptr< CellInfo > xCellInfo;

            // first and last row win first, if used and available
            if( bFirstRow )
            {
                xCellInfo = rStyle[sdr::table::first_row_style];
            }
            else if( bLastRow )
            {
                xCellInfo = rStyle[sdr::table::last_row_style];
            }

            if( !xCellInfo )
            {
                // next come first and last column, if used and available
                if( rSettings.mbUseFirstColumn && (nCol == 0)  )
                {
                    xCellInfo = rStyle[sdr::table::first_column_style];
                }
                else if( rSettings.mbUseLastColumn && (nCol == nPreviewColumns-1) )
                {
                    xCellInfo = rStyle[sdr::table::last_column_style];
                }
            }

            if( !xCellInfo )
            {
                if( rSettings.mbUseRowBanding )
                {
                    if( (nRow & 1) == 0 )
                    {
                        xCellInfo = rStyle[sdr::table::even_rows_style];
                    }
                    else
                    {
                        xCellInfo = rStyle[sdr::table::odd_rows_style];
                    }
                }
            }

            if( !xCellInfo )
            {
                if( rSettings.mbUseColumnBanding )
                {
                    if( (nCol & 1) == 0 )
                    {
                        xCellInfo = rStyle[sdr::table::even_columns_style];
                    }
                    else
                    {
                        xCellInfo = rStyle[sdr::table::odd_columns_style];
                    }
                }
            }

            if( !xCellInfo )
            {
                // use default cell style if non found yet
                xCellInfo = rStyle[sdr::table::body_style];
            }

            rMatrix[(nCol * nPreviewColumns) + nRow] = xCellInfo;
        }
    }
}

static BitmapEx CreateDesignPreview( const Reference< XIndexAccess >& xTableStyle, const TableStyleSettings& rSettings, bool bIsPageDark )
{
    CellInfoVector aCellInfoVector(sdr::table::style_count);
    FillCellInfoVector( xTableStyle, aCellInfoVector );

    CellInfoMatrix aMatrix;
    FillCellInfoMatrix( aCellInfoVector, rSettings, aMatrix );

    // bbbbbbbbbbbb w = 12 pixel
    // bccccccccccb h = 7 pixel
    // bccccccccccb b = border color
    // bcttttttttcb c = cell color
    // bccccccccccb t = text color
    // bccccccccccb
    // bbbbbbbbbbbb

    ScopedVclPtr<VirtualDevice> pVirDev(VclPtr<VirtualDevice>::Create());
    Size aBmpSize(nBitmapWidth, nBitmapHeight);
    pVirDev->SetOutputSizePixel(aBmpSize);

    pVirDev->SetBackground( bIsPageDark ? COL_BLACK : COL_WHITE );
    pVirDev->Erase();

    // first draw cell background and text line previews
    sal_Int32 nY = 0;
    sal_Int32 nRow;
    for( nRow = 0; nRow < nPreviewRows; ++nRow, nY += nCellHeight-1 )
    {
        sal_Int32 nX = 0;
        for( sal_Int32 nCol = 0; nCol < nPreviewColumns; ++nCol, nX += nCellWidth-1 )
        {
            std::shared_ptr< CellInfo > xCellInfo(aMatrix[(nCol * nPreviewColumns) + nRow]);

            Color aTextColor( COL_AUTO );
            if( xCellInfo )
            {
                // fill cell background
                const ::tools::Rectangle aRect( nX, nY, nX + nCellWidth - 1, nY + nCellHeight - 1 );

                if( xCellInfo->maCellColor != COL_TRANSPARENT )
                {
                    pVirDev->SetFillColor( xCellInfo->maCellColor );
                    pVirDev->DrawRect( aRect );
                }

                aTextColor = xCellInfo->maTextColor;
            }

            // draw text preview line
            if( aTextColor == COL_AUTO )
                aTextColor = bIsPageDark ? COL_WHITE : COL_BLACK;
            pVirDev->SetLineColor( aTextColor );
            const Point aPnt1( nX + 2, nY + ((nCellHeight - 1 ) >> 1) );
            const Point aPnt2( nX + nCellWidth - 3, aPnt1.Y() );
            pVirDev->DrawLine( aPnt1, aPnt2 );
        }
    }

    // second draw border lines
    nY = 0;
    for( nRow = 0; nRow < nPreviewRows; ++nRow, nY += nCellHeight-1 )
    {
        sal_Int32 nX = 0;
        for( sal_Int32 nCol = 0; nCol < nPreviewColumns; ++nCol, nX += nCellWidth-1 )
        {
            std::shared_ptr< CellInfo > xCellInfo(aMatrix[(nCol * nPreviewColumns) + nRow]);

            if( xCellInfo )
            {
                const Point aPntTL( nX, nY );
                const Point aPntTR( nX + nCellWidth - 1, nY );
                const Point aPntBL( nX, nY + nCellHeight - 1 );
                const Point aPntBR( nX + nCellWidth - 1, nY + nCellHeight - 1 );

                sal_Int32 border_diffs[8] = { 0,-1, 0,1, -1,0, 1,0 };
                sal_Int32* pDiff = &border_diffs[0];

                // draw top border
                for( SvxBoxItemLine nLine : o3tl::enumrange<SvxBoxItemLine>() )
                {
                    const ::editeng::SvxBorderLine* pBorderLine = xCellInfo->maBorder->GetLine(nLine);
                    if( !pBorderLine || ((pBorderLine->GetOutWidth() == 0) && (pBorderLine->GetInWidth()==0)) )
                        continue;

                    sal_Int32 nBorderCol = nCol + *pDiff++;
                    sal_Int32 nBorderRow = nRow + *pDiff++;
                    if( (nBorderCol >= 0) && (nBorderCol < nPreviewColumns) && (nBorderRow >= 0) && (nBorderRow < nPreviewRows) )
                    {
                        // check border
                        std::shared_ptr< CellInfo > xBorderInfo(aMatrix[(nBorderCol * nPreviewColumns) + nBorderRow]);
                        if( xBorderInfo )
                        {
                            const ::editeng::SvxBorderLine* pBorderLine2 = xBorderInfo->maBorder->GetLine(static_cast<SvxBoxItemLine>(static_cast<int>(nLine)^1));
                            if( pBorderLine2 && pBorderLine2->HasPriority(*pBorderLine) )
                                continue; // other border line wins
                        }
                    }

                    pVirDev->SetLineColor( pBorderLine->GetColor() );
                    switch( nLine )
                    {
                    case SvxBoxItemLine::TOP: pVirDev->DrawLine( aPntTL, aPntTR ); break;
                    case SvxBoxItemLine::BOTTOM: pVirDev->DrawLine( aPntBL, aPntBR ); break;
                    case SvxBoxItemLine::LEFT: pVirDev->DrawLine( aPntTL, aPntBL ); break;
                    case SvxBoxItemLine::RIGHT: pVirDev->DrawLine( aPntTR, aPntBR ); break;
                    }
                }
            }
        }
    }

    return pVirDev->GetBitmapEx(Point(0,0), aBmpSize);
}

void TableDesignWidget::FillDesignPreviewControl()
{
    sal_uInt16 nSelectedItem = m_xValueSet->GetSelectedItemId();
    m_xValueSet->Clear();
    try
    {
        TableStyleSettings aSettings;
        if( mxSelectedTable.is() )
        {
            aSettings.mbUseFirstRow = m_aCheckBoxes[CB_HEADER_ROW]->get_active();
            aSettings.mbUseLastRow = m_aCheckBoxes[CB_TOTAL_ROW]->get_active();
            aSettings.mbUseRowBanding = m_aCheckBoxes[CB_BANDED_ROWS]->get_active();
            aSettings.mbUseFirstColumn = m_aCheckBoxes[CB_FIRST_COLUMN]->get_active();
            aSettings.mbUseLastColumn = m_aCheckBoxes[CB_LAST_COLUMN]->get_active();
            aSettings.mbUseColumnBanding = m_aCheckBoxes[CB_BANDED_COLUMNS]->get_active();
        }

        bool bIsPageDark = false;
        if( mxView.is() )
        {
            Reference< XPropertySet > xPageSet( mxView->getCurrentPage(), UNO_QUERY );
            if( xPageSet.is() )
            {
                xPageSet->getPropertyValue(u"IsBackgroundDark"_ustr) >>= bIsPageDark;
            }
        }

        sal_Int32 nCount = mxTableFamily->getCount();
        for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex ) try
        {
            Reference< XIndexAccess > xTableStyle( mxTableFamily->getByIndex( nIndex ), UNO_QUERY );
            if( xTableStyle.is() )
                m_xValueSet->InsertItem( sal::static_int_cast<sal_uInt16>( nIndex + 1 ), Image( CreateDesignPreview( xTableStyle, aSettings, bIsPageDark ) ) );
        }
        catch( Exception& )
        {
            TOOLS_WARN_EXCEPTION( "sd", "sd::TableDesignWidget::FillDesignPreviewControl()");
        }
        m_xValueSet->InsertItem(++nCount, Image(StockImage::Yes, BMP_INSERT_TABLESTYLE), SdResId(STR_INSERT_TABLESTYLE));

        sal_Int32 nCols = 3;
        sal_Int32 nRows = std::min<sal_Int32>((nCount+2)/3, TableValueSet::getMaxRowCount());
        m_xValueSet->SetColCount(nCols);
        m_xValueSet->SetLineCount(nRows);
        WinBits nStyle = m_xValueSet->GetStyle() & ~WB_VSCROLL;
        m_xValueSet->SetStyle(nStyle);

        m_xValueSet->SetOptimalSize();
        weld::DrawingArea* pDrawingArea = m_xValueSet->GetDrawingArea();
        Size aSize = pDrawingArea->get_preferred_size();
        aSize.AdjustWidth(10 * nCols);
        aSize.AdjustHeight(10 * nRows);
        pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

        m_xValueSet->Resize();
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::TableDesignWidget::FillDesignPreviewControl()");
    }
    m_xValueSet->SelectItem(nSelectedItem);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
