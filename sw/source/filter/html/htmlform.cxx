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

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmltokn.h>
#include <svl/urihelper.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <svx/svdouno.hxx>
#include <svx/fmglob.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XImageConsumer.hpp>
#include <com/sun/star/awt/ImageStatus.hpp>
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <fmtanchr.hxx>
#include <htmltbl.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <unodraw.hxx>
#include <unotextrange.hxx>
#include "dcontact.hxx"

#include "swcss1.hxx"
#include "swhtml.hxx"
#include "htmlform.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::form;

const sal_uInt16 TABINDEX_MIN = 0;
const sal_uInt16 TABINDEX_MAX = 32767;

static HTMLOptionEnum aHTMLFormMethodTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_METHOD_get,       FormSubmitMethod_GET    },
    { OOO_STRING_SVTOOLS_HTML_METHOD_post,  FormSubmitMethod_POST   },
    { nullptr,                    0                       }
};

static HTMLOptionEnum aHTMLFormEncTypeTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_ET_url,           FormSubmitEncoding_URL          },
    { OOO_STRING_SVTOOLS_HTML_ET_multipart, FormSubmitEncoding_MULTIPART    },
    { OOO_STRING_SVTOOLS_HTML_ET_text,      FormSubmitEncoding_TEXT         },
    { nullptr,                    0                               }
};

enum HTMLWordWrapMode { HTML_WM_OFF, HTML_WM_HARD, HTML_WM_SOFT };

static HTMLOptionEnum aHTMLTextAreaWrapTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_WW_off,       HTML_WM_OFF },
    { OOO_STRING_SVTOOLS_HTML_WW_hard,  HTML_WM_HARD    },
    { OOO_STRING_SVTOOLS_HTML_WW_soft,  HTML_WM_SOFT    },
    { OOO_STRING_SVTOOLS_HTML_WW_physical,HTML_WM_HARD  },
    { OOO_STRING_SVTOOLS_HTML_WW_virtual,   HTML_WM_SOFT    },
    { nullptr,                0               }
};

HTMLEventType aEventTypeTable[] =
{
    HTML_ET_ONSUBMITFORM,
    HTML_ET_ONRESETFORM,
    HTML_ET_ONGETFOCUS,
    HTML_ET_ONLOSEFOCUS,
    HTML_ET_ONCLICK,
    HTML_ET_ONCLICK_ITEM,
    HTML_ET_ONCHANGE,
    HTML_ET_ONSELECT,
    HTML_ET_END
};

const sal_Char * aEventListenerTable[] =
{
    "XSubmitListener",
    "XResetListener",
    "XFocusListener",
    "XFocusListener",
    "XApproveActionListener",
    "XItemListener",
    "XChangeListener",
    ""
};

const sal_Char * aEventMethodTable[] =
{
    "approveSubmit",
    "approveReset",
    "focusGained",
    "focusLost",
    "approveAction",
    "itemStateChanged",
    "changed",
    ""
};

const sal_Char * aEventSDOptionTable[] =
{
    OOO_STRING_SVTOOLS_HTML_O_SDonsubmit,
    OOO_STRING_SVTOOLS_HTML_O_SDonreset,
    OOO_STRING_SVTOOLS_HTML_O_SDonfocus,
    OOO_STRING_SVTOOLS_HTML_O_SDonblur,
    OOO_STRING_SVTOOLS_HTML_O_SDonclick,
    OOO_STRING_SVTOOLS_HTML_O_SDonclick,
    OOO_STRING_SVTOOLS_HTML_O_SDonchange,
    nullptr
};

const sal_Char * aEventOptionTable[] =
{
    OOO_STRING_SVTOOLS_HTML_O_onsubmit,
    OOO_STRING_SVTOOLS_HTML_O_onreset,
    OOO_STRING_SVTOOLS_HTML_O_onfocus,
    OOO_STRING_SVTOOLS_HTML_O_onblur,
    OOO_STRING_SVTOOLS_HTML_O_onclick,
    OOO_STRING_SVTOOLS_HTML_O_onclick,
    OOO_STRING_SVTOOLS_HTML_O_onchange,
    nullptr
};

class SwHTMLForm_Impl
{
    SwDocShell                  *pDocSh;

    SvKeyValueIterator          *pHeaderAttrs;

    // gecachte Interfaces
    uno::Reference< drawing::XDrawPage >            xDrawPage;
    uno::Reference< container::XIndexContainer >    xForms;
    uno::Reference< drawing::XShapes >              xShapes;
    uno::Reference< XMultiServiceFactory >          xServiceFactory;

    uno::Reference< script::XEventAttacherManager >     xControlEventManager;
    uno::Reference< script::XEventAttacherManager >     xFormEventManager;

    // Kontext-Informationen
    uno::Reference< container::XIndexContainer >    xFormComps;
    uno::Reference< beans::XPropertySet >           xFCompPropSet;
    uno::Reference< drawing::XShape >               xShape;

    OUString                    sText;
    std::vector<OUString>         aStringList;
    std::vector<OUString>         aValueList;
    std::vector<sal_uInt16>     aSelectedList;

public:
    explicit SwHTMLForm_Impl( SwDocShell *pDSh ) :
        pDocSh( pDSh ),
        pHeaderAttrs( pDSh ? pDSh->GetHeaderAttributes() : nullptr )
    {
        OSL_ENSURE( pDocSh, "Keine DocShell, keine Controls" );
    }

    const uno::Reference< XMultiServiceFactory >& GetServiceFactory();
    const uno::Reference< drawing::XDrawPage >& GetDrawPage();
    const uno::Reference< drawing::XShapes >& GetShapes();
    const uno::Reference< script::XEventAttacherManager >& GetControlEventManager();
    const uno::Reference< script::XEventAttacherManager >& GetFormEventManager();
    const uno::Reference< container::XIndexContainer >& GetForms();

    const uno::Reference< container::XIndexContainer >& GetFormComps() const
    {
        return xFormComps;
    }

    void SetFormComps( const uno::Reference< container::XIndexContainer >& r )
    {
        xFormComps = r;
    }

    void ReleaseFormComps() { xFormComps = nullptr; xControlEventManager = nullptr; }

    const uno::Reference< beans::XPropertySet >& GetFCompPropSet() const
    {
        return xFCompPropSet;
    }

    void SetFCompPropSet( const uno::Reference< beans::XPropertySet >& r )
    {
        xFCompPropSet = r;
    }

    void ReleaseFCompPropSet() { xFCompPropSet = nullptr; }

    const uno::Reference< drawing::XShape >& GetShape() const { return xShape; }
    void SetShape( const uno::Reference< drawing::XShape >& r ) { xShape = r; }

    OUString& GetText() { return sText; }
    void EraseText() { sText = aEmptyOUStr; }

    std::vector<OUString>& GetStringList() { return aStringList; }
    void EraseStringList()
    {
        aStringList.clear();
    }

    std::vector<OUString>& GetValueList() { return aValueList; }
    void EraseValueList()
    {
        aValueList.clear();
    }

    std::vector<sal_uInt16>& GetSelectedList() { return aSelectedList; }
    void EraseSelectedList()
    {
        aSelectedList.clear();
    }

    SvKeyValueIterator *GetHeaderAttrs() const { return pHeaderAttrs; }
};

const uno::Reference< XMultiServiceFactory >& SwHTMLForm_Impl::GetServiceFactory()
{
    if( !xServiceFactory.is() && pDocSh )
    {
        xServiceFactory =
            uno::Reference< XMultiServiceFactory >( pDocSh->GetBaseModel(),
                                               UNO_QUERY );
        OSL_ENSURE( xServiceFactory.is(),
                "XServiceFactory nicht vom Model erhalten" );
    }
    return xServiceFactory;
}

const uno::Reference< drawing::XDrawPage >& SwHTMLForm_Impl::GetDrawPage()
{
    if( !xDrawPage.is() && pDocSh )
    {
        uno::Reference< drawing::XDrawPageSupplier > xTextDoc( pDocSh->GetBaseModel(),
                                                         UNO_QUERY );
        OSL_ENSURE( xTextDoc.is(),
                "drawing::XDrawPageSupplier nicht vom XModel erhalten" );
        xDrawPage = xTextDoc->getDrawPage();
        OSL_ENSURE( xDrawPage.is(), "drawing::XDrawPage nicht erhalten" );
    }
    return xDrawPage;
}

const uno::Reference< container::XIndexContainer >& SwHTMLForm_Impl::GetForms()
{
    if( !xForms.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {
            uno::Reference< XFormsSupplier > xFormsSupplier( xDrawPage, UNO_QUERY );
            OSL_ENSURE( xFormsSupplier.is(),
                    "XFormsSupplier nicht vom drawing::XDrawPage erhalten" );

            uno::Reference< container::XNameContainer > xNameCont =
                xFormsSupplier->getForms();
            xForms.set( xNameCont, UNO_QUERY );

            OSL_ENSURE( xForms.is(), "XForms nicht erhalten" );
        }
    }
    return xForms;
}

const uno::Reference< drawing::XShapes > & SwHTMLForm_Impl::GetShapes()
{
    if( !xShapes.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {
            xShapes.set( xDrawPage, UNO_QUERY );
            OSL_ENSURE( xShapes.is(),
                    "XShapes nicht vom drawing::XDrawPage erhalten" );
        }
    }
    return xShapes;
}

const uno::Reference< script::XEventAttacherManager >&
                                    SwHTMLForm_Impl::GetControlEventManager()
{
    if( !xControlEventManager.is() && xFormComps.is() )
    {
        xControlEventManager =
            uno::Reference< script::XEventAttacherManager >( xFormComps, UNO_QUERY );
        OSL_ENSURE( xControlEventManager.is(),
    "uno::Reference< XEventAttacherManager > nicht von xFormComps erhalten" );
    }

    return xControlEventManager;
}

const uno::Reference< script::XEventAttacherManager >&
    SwHTMLForm_Impl::GetFormEventManager()
{
    if( !xFormEventManager.is() )
    {
        GetForms();
        if( xForms.is() )
        {
            xFormEventManager =
                uno::Reference< script::XEventAttacherManager >( xForms, UNO_QUERY );
            OSL_ENSURE( xFormEventManager.is(),
        "uno::Reference< XEventAttacherManager > nicht von xForms erhalten" );
        }
    }

    return xFormEventManager;
}

class SwHTMLImageWatcher :
    public cppu::WeakImplHelper< awt::XImageConsumer, XEventListener >
{
    uno::Reference< drawing::XShape >       xShape;     // das control
    uno::Reference< XImageProducerSupplier >    xSrc;
    uno::Reference< awt::XImageConsumer >   xThis;      // man selbst
    bool                            bSetWidth;
    bool                            bSetHeight;

    void clear();

public:
    SwHTMLImageWatcher( const uno::Reference< drawing::XShape > & rShape,
                        bool bWidth, bool bHeight );
    virtual ~SwHTMLImageWatcher();

    // startProduction darf nicht im Konstruktor gerufen werden, weil
    // wir und ggf. selbst zerstoeren ... Deshlab eine eigene Methode.
    void start() { xSrc->getImageProducer()->startProduction(); }

    // UNO Anbindung

    // XImageConsumer
    virtual void SAL_CALL init( sal_Int32 Width, sal_Int32 Height)
        throw( uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setColorModel(
            sal_Int16 BitCount, const uno::Sequence< sal_Int32 >& RGBAPal,
            sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask,
            sal_Int32 AlphaMask)
        throw( uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setPixelsByBytes(
            sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height,
            const uno::Sequence< sal_Int8 >& ProducerData,
            sal_Int32 Offset, sal_Int32 Scansize)
        throw( uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setPixelsByLongs(
            sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height,
            const uno::Sequence< sal_Int32 >& ProducerData,
            sal_Int32 Offset, sal_Int32 Scansize)
        throw( uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL complete(
            sal_Int32 Status,
            const uno::Reference< awt::XImageProducer > & Producer)
        throw( uno::RuntimeException, std::exception ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& Source ) throw ( uno::RuntimeException, std::exception) override;
};

SwHTMLImageWatcher::SwHTMLImageWatcher(
        const uno::Reference< drawing::XShape >& rShape,
        bool bWidth, bool bHeight ) :
    xShape( rShape ),
    bSetWidth( bWidth ), bSetHeight( bHeight )
{
    // Die Quelle des Images merken
    uno::Reference< drawing::XControlShape > xControlShape( xShape, UNO_QUERY );
    uno::Reference< awt::XControlModel > xControlModel(
            xControlShape->getControl() );
    xSrc.set( xControlModel, UNO_QUERY );
    OSL_ENSURE( xSrc.is(), "Kein XImageProducerSupplier" );

    // Als Event-Listener am Shape anmelden, damit wir es beim dispose
    // loslassen ko"onnen ...
    uno::Reference< XEventListener > xEvtLstnr = static_cast<XEventListener *>(this);
    uno::Reference< XComponent > xComp( xShape, UNO_QUERY );
    xComp->addEventListener( xEvtLstnr );

    // Zum Schluss halten wir noch eine Referenz auf uns selbst, damit
    // wir am Leben bleiben ... (eigentlich sollte das nicht neotig sein,
    // weil wir ja noch an diversen anderen Stellen angemeldet sind)
    xThis = static_cast<awt::XImageConsumer *>(this);

    // und am ImageProducer anmelden, um die Groesse zu erehalten ...
    xSrc->getImageProducer()->addConsumer( xThis );
}

SwHTMLImageWatcher::~SwHTMLImageWatcher()
{
}

void SwHTMLImageWatcher::clear()
{
    // Am Shape als Event-Listener abmelden
    uno::Reference< XEventListener > xEvtLstnr = static_cast<XEventListener *>(this);
    uno::Reference< XComponent > xComp( xShape, UNO_QUERY );
    xComp->removeEventListener( xEvtLstnr );

    // Am ImageProducer abmelden
    uno::Reference<awt::XImageProducer> xProd = xSrc->getImageProducer();
    if( xProd.is() )
        xProd->removeConsumer( xThis );
}

void SwHTMLImageWatcher::init( sal_Int32 Width, sal_Int32 Height )
    throw( uno::RuntimeException, std::exception )
{
    OSL_ENSURE( bSetWidth || bSetHeight,
            "Breite oder Hoehe muss angepasst werden" );

    // Wenn keine Breite oder Hoehe angegeben ist, ist das das init von
    // der leeren Grafik, die angezeigt wird, bevor der Stream einer
    // asynchron anzuzeigenden Grfik verfuegbar ist.
    if( !Width && !Height )
        return;

    awt::Size aNewSz;
    aNewSz.Width = Width;
    aNewSz.Height = Height;
    if( Application::GetDefaultDevice() )
    {
        Size aTmp(aNewSz.Width, aNewSz.Height);
        aTmp = Application::GetDefaultDevice()
                    ->PixelToLogic( aTmp, MapMode( MAP_100TH_MM ) );
        aNewSz.Width = aTmp.Width();
        aNewSz.Height = aTmp.Height();
    }

    if( !bSetWidth || !bSetHeight )
    {
        awt::Size aSz( xShape->getSize() );
        if( bSetWidth && aNewSz.Height )
        {
            aNewSz.Width *= aSz.Height;
            aNewSz.Width /= aNewSz.Height;
            aNewSz.Height = aSz.Height;
        }
        if( bSetHeight && aNewSz.Width )
        {
            aNewSz.Height *= aSz.Width;
            aNewSz.Height /= aNewSz.Width;
            aNewSz.Width = aSz.Width;
        }
    }
    if( aNewSz.Width < MINFLY )
        aNewSz.Width = MINFLY;
    if( aNewSz.Height < MINFLY )
        aNewSz.Height = MINFLY;

    xShape->setSize( aNewSz );
    if( bSetWidth )
    {
        // Wenn das Control in einer Tabelle verankert ist, muesen
        // die Tabellen-Spalten neu berechnet werden

        // Um an den SwXShape* zu gelangen, brauchen wir ein Interface,
        // das auch vom SwXShape implementiert wird.

        uno::Reference< beans::XPropertySet > xPropSet( xShape, UNO_QUERY );
        uno::Reference< XUnoTunnel> xTunnel( xPropSet, UNO_QUERY );
                SwXShape *pSwShape = xTunnel.is() ?
                    reinterpret_cast< SwXShape * >( sal::static_int_cast< sal_IntPtr>(
                    xTunnel->getSomething(SwXShape::getUnoTunnelId()) ))
                : nullptr;

        OSL_ENSURE( pSwShape, "Wo ist das SW-Shape?" );
        if( pSwShape )
        {
            SwFrameFormat *pFrameFormat = pSwShape->GetFrameFormat();

            const SwDoc *pDoc = pFrameFormat->GetDoc();
            const SwPosition* pAPos = pFrameFormat->GetAnchor().GetContentAnchor();
            SwNode *pANd;
            SwTableNode *pTableNd;
            if( pAPos &&
                nullptr != (pANd = & pAPos->nNode.GetNode()) &&
                nullptr != (pTableNd = pANd->FindTableNode()) )
            {
                const bool bLastGrf = !pTableNd->GetTable().DecGrfsThatResize();
                SwHTMLTableLayout *pLayout =
                    pTableNd->GetTable().GetHTMLTableLayout();
                if( pLayout )
                {
                    const sal_uInt16 nBrowseWidth =
                        pLayout->GetBrowseWidthByTable( *pDoc );

                    if ( nBrowseWidth )
                    {
                        pLayout->Resize( nBrowseWidth, true, true,
                                         bLastGrf ? HTMLTABLE_RESIZE_NOW
                                                  : 500 );
                    }
                }
            }
        }
    }

    // uns selbst abmelden und loeschen
    clear();
    uno::Reference< awt::XImageConsumer >  xTmp = static_cast<awt::XImageConsumer*>(this);
    xThis = nullptr;
}

void SwHTMLImageWatcher::setColorModel(
        sal_Int16, const Sequence< sal_Int32 >&, sal_Int32, sal_Int32,
        sal_Int32, sal_Int32 )
    throw( uno::RuntimeException, std::exception )
{
}

void SwHTMLImageWatcher::setPixelsByBytes(
        sal_Int32, sal_Int32, sal_Int32, sal_Int32,
        const Sequence< sal_Int8 >&, sal_Int32, sal_Int32 )
    throw( uno::RuntimeException, std::exception )
{
}

void SwHTMLImageWatcher::setPixelsByLongs(
        sal_Int32, sal_Int32, sal_Int32, sal_Int32,
        const Sequence< sal_Int32 >&, sal_Int32, sal_Int32 )
    throw( uno::RuntimeException, std::exception )
{
}

void SwHTMLImageWatcher::complete( sal_Int32 Status,
        const uno::Reference< awt::XImageProducer >& )
    throw( uno::RuntimeException, std::exception )
{
    if( awt::ImageStatus::IMAGESTATUS_ERROR == Status || awt::ImageStatus::IMAGESTATUS_ABORTED == Status )
    {
        // uns selbst abmelden und loeschen
        clear();
        uno::Reference< awt::XImageConsumer > xTmp = static_cast<awt::XImageConsumer*>(this);
        xThis = nullptr;
    }
}

void SwHTMLImageWatcher::disposing(const lang::EventObject& evt) throw ( uno::RuntimeException, std::exception)
{
    uno::Reference< awt::XImageConsumer > xTmp;

    // Wenn das Shape verschwindet soll muessen wir es loslassen
    uno::Reference< drawing::XShape > xTmpShape;
    if( evt.Source == xShape )
    {
        clear();
        xTmp = static_cast<awt::XImageConsumer*>(this);
        xThis = nullptr;
    }
}

void SwHTMLParser::DeleteFormImpl()
{
    delete m_pFormImpl;
    m_pFormImpl = nullptr;
}

static void lcl_html_setFixedFontProperty(
        const uno::Reference< beans::XPropertySet >& rPropSet )
{
    vcl::Font aFixedFont( OutputDevice::GetDefaultFont(
                                    DefaultFontType::FIXED, LANGUAGE_ENGLISH_US,
                                    GetDefaultFontFlags::OnlyOne )  );
    Any aTmp;
    aTmp <<= OUString( aFixedFont.GetFamilyName() );
    rPropSet->setPropertyValue("FontName", aTmp );

    aTmp <<= OUString( aFixedFont.GetStyleName() );
    rPropSet->setPropertyValue("FontStyleName",
                                aTmp );

    aTmp <<= (sal_Int16) aFixedFont.GetFamily();
    rPropSet->setPropertyValue("FontFamily", aTmp );

    aTmp <<= (sal_Int16) aFixedFont.GetCharSet();
    rPropSet->setPropertyValue("FontCharset",
                                aTmp );

    aTmp <<= (sal_Int16) aFixedFont.GetPitch();
    rPropSet->setPropertyValue("FontPitch", aTmp );

    float fVal(10.);
    aTmp.setValue( &fVal, cppu::UnoType<decltype(fVal)>::get());
    rPropSet->setPropertyValue("FontHeight", aTmp );
}

void SwHTMLParser::SetControlSize( const uno::Reference< drawing::XShape >& rShape,
                                   const Size& rTextSz,
                                   bool bMinWidth,
                                   bool bMinHeight )
{
    if( !rTextSz.Width() && !rTextSz.Height() && !bMinWidth  && !bMinHeight )
        return;

    // Um an den SwXShape* zu gelangen, brauchen wir ein Interface,
    // das auch vom SwXShape implementiert wird.
    uno::Reference< beans::XPropertySet > xPropSet( rShape, UNO_QUERY );

    SwViewShell *pVSh = m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    if( !pVSh && !m_nEventId )
    {
        // If there is no view shell by now and the doc shell is an internal
        // one, no view shell will be created. That for, we have to do that of
        // our own. This happens if a linked section is inserted or refreshed.
        SwDocShell *pDocSh = m_pDoc->GetDocShell();
        if( pDocSh )
        {
            if ( pDocSh->GetMedium() )
            {
                // if there is no hidden property in the MediaDescriptor it should be removed after loading
                const SfxBoolItem* pHiddenItem = SfxItemSet::GetItem<SfxBoolItem>(pDocSh->GetMedium()->GetItemSet(), SID_HIDDEN, false);
                m_bRemoveHidden = ( pHiddenItem == nullptr || !pHiddenItem->GetValue() );
            }

            m_pTempViewFrame = SfxViewFrame::LoadHiddenDocument( *pDocSh, 0 );
            CallStartAction();
            pVSh = m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
            // this ridiculous hack also enables Undo, so turn it off again
            m_pDoc->GetIDocumentUndoRedo().DoUndo(false);
        }
    }

    uno::Reference< XUnoTunnel> xTunnel( xPropSet, UNO_QUERY );
    SwXShape *pSwShape = xTunnel.is() ?
        reinterpret_cast< SwXShape *>( sal::static_int_cast< sal_IntPtr >(
            xTunnel->getSomething(SwXShape::getUnoTunnelId()) ))
        : nullptr;

    OSL_ENSURE( pSwShape, "Wo ist das SW-Shape?" );

    // es muss ein Draw-Format sein
    SwFrameFormat *pFrameFormat = pSwShape ? pSwShape->GetFrameFormat() : nullptr ;
    OSL_ENSURE( pFrameFormat && RES_DRAWFRMFMT == pFrameFormat->Which(), "Kein DrawFrameFormat" );

    // Schauen, ob es ein SdrObject dafuer gibt
    const SdrObject *pObj = pFrameFormat ? pFrameFormat->FindSdrObject() : nullptr;
    OSL_ENSURE( pObj, "SdrObject nicht gefunden" );
    OSL_ENSURE( pObj && FmFormInventor == pObj->GetObjInventor(), "falscher Inventor" );

    const SdrView* pDrawView = pVSh ? pVSh->GetDrawView() : nullptr;

    const SdrUnoObj *pFormObj = dynamic_cast<const SdrUnoObj*>( pObj  );
    uno::Reference< awt::XControl > xControl;
    if ( pDrawView && pVSh->GetWin() && pFormObj )
        xControl = pFormObj->GetUnoControl( *pDrawView, *pVSh->GetWin() );

    awt::Size aSz( rShape->getSize() );
    awt::Size aNewSz( 0, 0 );

    // #i71248# ensure we got a XControl before applying corrections
    if(xControl.is())
    {
        if( bMinWidth || bMinHeight )
        {
            uno::Reference< awt::XLayoutConstrains > xLC( xControl, UNO_QUERY );
            awt::Size aTmpSz( xLC->getPreferredSize() );
            if( bMinWidth )
                aNewSz.Width = aTmpSz.Width;
            if( bMinHeight )
                aNewSz.Height = aTmpSz.Height;
        }
        if( rTextSz.Width() || rTextSz.Height())
        {
            uno::Reference< awt::XTextLayoutConstrains > xLC( xControl, UNO_QUERY );
            OSL_ENSURE( xLC.is(), "kein XTextLayoutConstrains" );
            if( xLC.is() )
            {
                awt::Size aTmpSz( rTextSz.Width(), rTextSz.Height() );
                if( -1 == rTextSz.Width() )
                {
                    aTmpSz.Width = 0;
                    aTmpSz.Height = m_nSelectEntryCnt;
                }
                aTmpSz = xLC->getMinimumSize( static_cast< sal_Int16 >(aTmpSz.Width), static_cast< sal_Int16 >(aTmpSz.Height) );
                if( rTextSz.Width() )
                    aNewSz.Width = aTmpSz.Width;
                if( rTextSz.Height() )
                    aNewSz.Height = aTmpSz.Height;
            }
        }
    }

    if( Application::GetDefaultDevice() )
    {
        Size aTmpSz( aNewSz.Width, aNewSz.Height );
        aTmpSz = Application::GetDefaultDevice()
                        ->PixelToLogic( aTmpSz, MapMode( MAP_100TH_MM ) );
        aNewSz.Width  = aTmpSz.Width();
        aNewSz.Height = aTmpSz.Height();
    }
    if( aNewSz.Width )
    {
        if( aNewSz.Width < MINLAY )
            aNewSz.Width = MINLAY;
        aSz.Width = aNewSz.Width;
    }
    if( aNewSz.Height )
    {
        if( aNewSz.Height < MINLAY )
            aNewSz.Height = MINLAY;
        aSz.Height = aNewSz.Height;
    }

    rShape->setSize( aSz );
}

static void lcl_html_setEvents(
        const uno::Reference< script::XEventAttacherManager > & rEvtMn,
        sal_uInt32 nPos, const SvxMacroTableDtor& rMacroTable,
        const std::vector<OUString>& rUnoMacroTable,
        const std::vector<OUString>& rUnoMacroParamTable,
        const OUString& rType )
{
    // Erstmal muss die Anzahl der Events ermittelt werden ...
    sal_Int32 nEvents = 0;

    for( int i = 0; HTML_ET_END != aEventTypeTable[i]; ++i )
    {
        const SvxMacro *pMacro = rMacroTable.Get( aEventTypeTable[i] );
        // Solange nicht alle Events implementiert sind, enthaelt die
        // Tabelle auch Leerstrings!
        if( pMacro && aEventListenerTable[i] )
            nEvents++;
    }
    for( const auto &rStr : rUnoMacroTable )
    {
        sal_Int32 nIndex = 0;
        if( rStr.getToken( 0, '-', nIndex ).isEmpty() || -1 == nIndex )
            continue;
        if( rStr.getToken( 0, '-', nIndex ).isEmpty() || -1 == nIndex )
            continue;
        if( nIndex < rStr.getLength() )
            nEvents++;
    }

    if( 0==nEvents )
        return;

    Sequence<script::ScriptEventDescriptor> aDescs( nEvents );
    script::ScriptEventDescriptor* pDescs = aDescs.getArray();
    sal_Int32 nEvent = 0;

    for( int i=0; HTML_ET_END != aEventTypeTable[i]; ++i )
    {
        const SvxMacro *pMacro = rMacroTable.Get( aEventTypeTable[i] );
        if( pMacro && aEventListenerTable[i] )
        {
            script::ScriptEventDescriptor& rDesc = pDescs[nEvent++];
            rDesc.ListenerType =
                    OUString::createFromAscii(aEventListenerTable[i]);
            rDesc.EventMethod = OUString::createFromAscii(aEventMethodTable[i]);
            rDesc.ScriptType = pMacro->GetLanguage();
            rDesc.ScriptCode = pMacro->GetMacName();
        }
    }

    for( const auto &rStr : rUnoMacroTable )
    {
        sal_Int32 nIndex = 0;
        OUString sListener( rStr.getToken( 0, '-', nIndex ) );
        if( sListener.isEmpty() || -1 == nIndex )
            continue;

        OUString sMethod( rStr.getToken( 0, '-', nIndex ) );
        if( sMethod.isEmpty() || -1 == nIndex )
            continue;

        OUString sCode( rStr.copy( nIndex ) );
        if( sCode.isEmpty() )
            continue;

        script::ScriptEventDescriptor& rDesc = pDescs[nEvent++];
        rDesc.ListenerType = sListener;
        rDesc.EventMethod = sMethod;
        rDesc.ScriptType = rType;
        rDesc.ScriptCode = sCode;
        rDesc.AddListenerParam.clear();

        if(!rUnoMacroParamTable.empty())
        {
            OUString sSearch( sListener );
            sSearch += "-" +sMethod + "-";
            sal_Int32 nLen = sSearch.getLength();
            for(size_t j = 0; j < rUnoMacroParamTable.size(); ++j)
            {
                const OUString& rParam = rUnoMacroParamTable[j];
                if( rParam.startsWith( sSearch ) && rParam.getLength() > nLen )
                {
                    rDesc.AddListenerParam = rParam.copy(nLen);
                    break;
                }
            }
        }
    }
    rEvtMn->registerScriptEvents( nPos, aDescs );
}

static void lcl_html_getEvents( const OUString& rOption, const OUString& rValue,
                                std::vector<OUString>& rUnoMacroTable,
                                std::vector<OUString>& rUnoMacroParamTable )
{
    if( rOption.startsWithIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_sdevent ) )
    {
        OUString aEvent( rOption.copy( strlen( OOO_STRING_SVTOOLS_HTML_O_sdevent ) ) );
        aEvent += "-" + rValue;
        rUnoMacroTable.push_back(aEvent);
    }
    else if( rOption.startsWithIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_sdaddparam ) )
    {
        OUString aParam( rOption.copy( strlen( OOO_STRING_SVTOOLS_HTML_O_sdaddparam ) ) );
        aParam += "-" + rValue;
        rUnoMacroParamTable.push_back(aParam);
    }
}

uno::Reference< drawing::XShape > SwHTMLParser::InsertControl(
        const uno::Reference< XFormComponent > & rFComp,
        const uno::Reference< beans::XPropertySet > & rFCompPropSet,
        const Size& rSize, sal_Int16 eVertOri, sal_Int16 eHoriOri,
        SfxItemSet& rCSS1ItemSet, SvxCSS1PropertyInfo& rCSS1PropInfo,
        const SvxMacroTableDtor& rMacroTable, const std::vector<OUString>& rUnoMacroTable,
        const std::vector<OUString>& rUnoMacroParamTable, bool bSetFCompPropSet,
        bool bHidden )
{
    uno::Reference< drawing::XShape >  xShape;

    const uno::Reference< container::XIndexContainer > & rFormComps =
        m_pFormImpl->GetFormComps();
    Any aAny( &rFComp, cppu::UnoType<XFormComponent>::get());
    rFormComps->insertByIndex( rFormComps->getCount(), aAny );

    if( !bHidden )
    {
        Any aTmp;
        sal_Int32 nLeftSpace = 0;
        sal_Int32 nRightSpace = 0;
        sal_Int32 nUpperSpace = 0;
        sal_Int32 nLowerSpace = 0;

        const uno::Reference< XMultiServiceFactory > & rServiceFactory =
            m_pFormImpl->GetServiceFactory();
        if( !rServiceFactory.is() )
            return xShape;

        uno::Reference< XInterface > xCreate = rServiceFactory->createInstance( "com.sun.star.drawing.ControlShape" );
        if( !xCreate.is() )
            return xShape;

        xShape.set( xCreate, UNO_QUERY );

        OSL_ENSURE( xShape.is(), "XShape nicht erhalten" );
        awt::Size aTmpSz;
        aTmpSz.Width  = rSize.Width();
        aTmpSz.Height = rSize.Height();
        xShape->setSize( aTmpSz );

        uno::Reference< beans::XPropertySet > xShapePropSet( xCreate, UNO_QUERY );

        // linken/rechten Rand setzen
        const SfxPoolItem *pItem;
        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_LR_SPACE, true,
                                                     &pItem ) )
        {
            // Ggf. den Erstzeilen-Einzug noch plaetten
            const SvxLRSpaceItem *pLRItem = static_cast<const SvxLRSpaceItem *>(pItem);
            SvxLRSpaceItem aLRItem( *pLRItem );
            aLRItem.SetTextFirstLineOfst( 0 );
            if( rCSS1PropInfo.bLeftMargin )
            {
                nLeftSpace = convertTwipToMm100( aLRItem.GetLeft() );
                rCSS1PropInfo.bLeftMargin = false;
            }
            if( rCSS1PropInfo.bRightMargin )
            {
                nRightSpace = convertTwipToMm100( aLRItem.GetRight() );
                rCSS1PropInfo.bRightMargin = false;
            }
            rCSS1ItemSet.ClearItem( RES_LR_SPACE );
        }
        if( nLeftSpace || nRightSpace )
        {
            Any aAny2;
            aAny2 <<= nLeftSpace;
            xShapePropSet->setPropertyValue("LeftMargin", aAny2 );

            aAny2 <<= nRightSpace;
            xShapePropSet->setPropertyValue("RightMargin", aAny2 );
        }

        // oberen/unteren Rand setzen
        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_UL_SPACE, true,
                                                     &pItem ) )
        {
            // Ggf. den Erstzeilen-Einzug noch plaetten
            const SvxULSpaceItem *pULItem = static_cast<const SvxULSpaceItem *>(pItem);
            if( rCSS1PropInfo.bTopMargin )
            {
                nUpperSpace = convertTwipToMm100( pULItem->GetUpper() );
                rCSS1PropInfo.bTopMargin = false;
            }
            if( rCSS1PropInfo.bBottomMargin )
            {
                nLowerSpace = convertTwipToMm100( pULItem->GetLower() );
                rCSS1PropInfo.bBottomMargin = false;
            }

            rCSS1ItemSet.ClearItem( RES_UL_SPACE );
        }
        if( nUpperSpace || nLowerSpace )
        {
            uno::Any aAny2;
            aAny2 <<= nUpperSpace;
            xShapePropSet->setPropertyValue("TopMargin", aAny2 );

            aAny2 <<= nLowerSpace;
            xShapePropSet->setPropertyValue("BottomMargin", aAny2 );
        }

        uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
            rFCompPropSet->getPropertySetInfo();
        OUString sPropName = "BackgroundColor";
        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_BACKGROUND, true,
                                                     &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            const Color &rColor = static_cast<const SvxBrushItem *>(pItem)->GetColor();
            /// copy color, if color is not "no fill"/"auto fill"
            if( rColor.GetColor() != COL_TRANSPARENT )
            {
                /// copy complete color with transparency
                aTmp <<= static_cast<sal_Int32>(rColor.GetColor());
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }

        }

        sPropName = "TextColor";
        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_CHRATR_COLOR, true,
                                                     &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp <<= (sal_Int32)static_cast<const SvxColorItem *>(pItem)->GetValue()
                                                         .GetRGBColor();
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        sPropName = "FontHeight";
        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_CHRATR_FONTSIZE,
                                                     true, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            float fVal = static_cast< float >(
                (static_cast<const SvxFontHeightItem *>(pItem)->GetHeight()) / 20.0 );
            aTmp.setValue( &fVal, cppu::UnoType<decltype(fVal)>::get());
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_CHRATR_FONT, true,
                                                     &pItem ) )
        {
            const SvxFontItem *pFontItem = static_cast<const SvxFontItem *>(pItem);
            sPropName = "FontName";
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= OUString( pFontItem->GetFamilyName() );
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
            sPropName = "FontStyleName";
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= OUString( pFontItem->GetStyleName() );
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
            sPropName = "FontFamily";
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= (sal_Int16)pFontItem->GetFamily() ;
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
            sPropName = "FontCharset";
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= (sal_Int16)pFontItem->GetCharSet() ;
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
            sPropName = "FontPitch";
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= (sal_Int16)pFontItem->GetPitch() ;
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
        }

        sPropName = "FontWeight";
        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_CHRATR_WEIGHT,
                                                     true, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            float fVal = VCLUnoHelper::ConvertFontWeight(
                    static_cast<const SvxWeightItem *>(pItem)->GetWeight() );
            aTmp.setValue( &fVal, cppu::UnoType<decltype(fVal)>::get());
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        sPropName = "FontSlant";
        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_CHRATR_POSTURE,
                                                     true, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp <<= (sal_Int16)static_cast<const SvxPostureItem *>(pItem)->GetPosture();
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        sPropName = "FontUnderline";
        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_CHRATR_UNDERLINE,
                                                     true, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp <<= (sal_Int16)static_cast<const SvxUnderlineItem *>(pItem)->GetLineStyle();
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        sPropName = "FontStrikeout";
        if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_CHRATR_CROSSEDOUT,
                                                     true, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp <<= (sal_Int16)static_cast<const SvxCrossedOutItem *>(pItem)->GetStrikeout();
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        uno::Reference< text::XTextRange >  xTextRg;
        sal_Int16 nAnchorType = text::TextContentAnchorType_AS_CHARACTER;
        bool bSetPos = false, bSetSurround = false;
        sal_Int32 nXPos = 0, nYPos = 0;
        sal_Int16 nSurround = text::WrapTextMode_NONE;
        if( SVX_CSS1_POS_ABSOLUTE == rCSS1PropInfo.ePosition &&
            SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.eLeftType &&
            SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.eTopType )
        {
            const SwStartNode *pFlySttNd =
                m_pPam->GetPoint()->nNode.GetNode().FindFlyStartNode();

            if( pFlySttNd )
            {
                nAnchorType = text::TextContentAnchorType_AT_FRAME;
                SwPaM aPaM( *pFlySttNd );

                uno::Reference< text::XText >  xDummyTextRef; // unsauber, aber laut OS geht das ...
                xTextRg = new SwXTextRange( aPaM, xDummyTextRef );
            }
            else
            {
                nAnchorType = text::TextContentAnchorType_AT_PAGE;
            }
            nXPos = convertTwipToMm100( rCSS1PropInfo.nLeft ) + nLeftSpace;
            nYPos = convertTwipToMm100( rCSS1PropInfo.nTop ) + nUpperSpace;
            bSetPos = true;

            nSurround = text::WrapTextMode_THROUGHT;
            bSetSurround = true;
        }
        else if( SVX_ADJUST_LEFT == rCSS1PropInfo.eFloat ||
                 text::HoriOrientation::LEFT == eHoriOri )
        {
            nAnchorType = text::TextContentAnchorType_AT_PARAGRAPH;
            nXPos = nLeftSpace;
            nYPos = nUpperSpace;
            bSetPos = true;
            nSurround = text::WrapTextMode_RIGHT;
            bSetSurround = true;
        }
        else if( text::VertOrientation::NONE != eVertOri )
        {
            sal_Int16 nVertOri = text::VertOrientation::NONE;
            switch( eVertOri )
            {
            case text::VertOrientation::TOP:
                nVertOri = text::VertOrientation::TOP;
                break;
            case text::VertOrientation::CENTER:
                nVertOri = text::VertOrientation::CENTER;
                break;
            case text::VertOrientation::BOTTOM:
                nVertOri = text::VertOrientation::BOTTOM;
                break;
            case text::VertOrientation::CHAR_TOP:
                nVertOri = text::VertOrientation::CHAR_TOP;
                break;
            case text::VertOrientation::CHAR_CENTER:
                nVertOri = text::VertOrientation::CHAR_CENTER;
                break;
            case text::VertOrientation::CHAR_BOTTOM:
                nVertOri = text::VertOrientation::CHAR_BOTTOM;
                break;
            case text::VertOrientation::LINE_TOP:
                nVertOri = text::VertOrientation::LINE_TOP;
                break;
            case text::VertOrientation::LINE_CENTER:
                nVertOri = text::VertOrientation::LINE_CENTER;
                break;
            case text::VertOrientation::LINE_BOTTOM:
                nVertOri = text::VertOrientation::LINE_BOTTOM;
                break;
            // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
            case text::VertOrientation::NONE:
                nVertOri = text::VertOrientation::NONE;
                break;
            }
            aTmp <<= (sal_Int16)nVertOri ;
            xShapePropSet->setPropertyValue("VertOrient", aTmp );
        }

        aTmp <<= (sal_Int16)nAnchorType ;
        xShapePropSet->setPropertyValue("AnchorType", aTmp );

        if( text::TextContentAnchorType_AT_PAGE == nAnchorType )
        {
            aTmp <<= (sal_Int16) 1 ;
            xShapePropSet->setPropertyValue("AnchorPageNo", aTmp );
        }
        else
        {
            if( !xTextRg.is() )
            {
                uno::Reference< text::XText >  xDummyTextRef; // unsauber, aber laut OS geht das ...
                xTextRg = new SwXTextRange( *m_pPam, xDummyTextRef );
            }

            aTmp.setValue( &xTextRg,
                           cppu::UnoType<text::XTextRange>::get());
            xShapePropSet->setPropertyValue("TextRange", aTmp );
        }

        if( bSetPos )
        {
            aTmp <<= (sal_Int16)text::HoriOrientation::NONE;
            xShapePropSet->setPropertyValue("HoriOrient", aTmp );
            aTmp <<= (sal_Int32)nXPos ;
            xShapePropSet->setPropertyValue("HoriOrientPosition", aTmp );

            aTmp <<= (sal_Int16)text::VertOrientation::NONE;
            xShapePropSet->setPropertyValue("VertOrient", aTmp );
            aTmp <<= (sal_Int32)nYPos ;
            xShapePropSet->setPropertyValue("VertOrientPosition", aTmp );
        }
        if( bSetSurround )
        {
            aTmp <<= (sal_Int16)nSurround ;
            xShapePropSet->setPropertyValue("Surround", aTmp );
        }

        m_pFormImpl->GetShapes()->add(xShape);

        // Das Control-Model am Control-Shape setzen
        uno::Reference< drawing::XControlShape > xControlShape( xShape, UNO_QUERY );
        uno::Reference< awt::XControlModel >  xControlModel( rFComp, UNO_QUERY );
        xControlShape->setControl( xControlModel );
    }

    // Da beim Einfuegen der Controls der Fokus gesetzt wird, werden
    // auch schon Fokus-Events verschickt. Damit die nicht evtl. schon
    // vorhendene JavaSCript-Eents rufen, werden die Events nachtraeglich
    // gesetzt.
    if( !rMacroTable.empty() || !rUnoMacroTable.empty() )
    {
        lcl_html_setEvents( m_pFormImpl->GetControlEventManager(),
                            rFormComps->getCount() - 1,
                            rMacroTable, rUnoMacroTable, rUnoMacroParamTable,
                            GetScriptTypeString(m_pFormImpl->GetHeaderAttrs()) );
    }

    if( bSetFCompPropSet )
    {
        m_pFormImpl->SetFCompPropSet( rFCompPropSet );
    }

    return xShape;
}

void SwHTMLParser::NewForm( bool bAppend )
{
    // Gibt es schon eine Form?
    if( m_pFormImpl && m_pFormImpl->GetFormComps().is() )
        return;

    if( bAppend )
    {
        if( m_pPam->GetPoint()->nContent.GetIndex() )
            AppendTextNode( AM_SPACE );
        else
            AddParSpace();
    }

    if( !m_pFormImpl )
        m_pFormImpl = new SwHTMLForm_Impl( m_pDoc->GetDocShell() );

    OUString aAction( m_sBaseURL );
    OUString sName, sTarget;
    sal_uInt16 nEncType = FormSubmitEncoding_URL;
    sal_uInt16 nMethod = FormSubmitMethod_GET;
    SvxMacroTableDtor aMacroTable;
    std::vector<OUString> aUnoMacroTable;
    std::vector<OUString> aUnoMacroParamTable;
    SvKeyValueIterator *pHeaderAttrs = m_pFormImpl->GetHeaderAttrs();
    ScriptType eDfltScriptType = GetScriptType( pHeaderAttrs );
    const OUString& rDfltScriptType = GetScriptTypeString( pHeaderAttrs );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        ScriptType eScriptType2 = eDfltScriptType;
        sal_uInt16 nEvent = 0;
        bool bSetEvent = false;

        switch( rOption.GetToken() )
        {
        case HTML_O_ACTION:
            aAction = rOption.GetString();
            break;
        case HTML_O_METHOD:
            nMethod = rOption.GetEnum( aHTMLFormMethodTable, nMethod );
            break;
        case HTML_O_ENCTYPE:
            nEncType = rOption.GetEnum( aHTMLFormEncTypeTable, nEncType );
            break;
        case HTML_O_TARGET:
            sTarget = rOption.GetString();
            break;
        case HTML_O_NAME:
            sName = rOption.GetString();
            break;

        case HTML_O_SDONSUBMIT:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONSUBMIT:
            nEvent = HTML_ET_ONSUBMITFORM;
            bSetEvent = true;
            break;

        case HTML_O_SDONRESET:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONRESET:
            nEvent = HTML_ET_ONRESETFORM;
            bSetEvent = true;
            break;

        default:
            lcl_html_getEvents( rOption.GetTokenString(),
                                rOption.GetString(),
                                aUnoMacroTable, aUnoMacroParamTable );
            break;
        }

        if( bSetEvent )
        {
            OUString sEvent( rOption.GetString() );
            if( !sEvent.isEmpty() )
            {
                sEvent = convertLineEnd(sEvent, GetSystemLineEnd());
                OUString aScriptType2;
                if( EXTENDED_STYPE==eScriptType2 )
                    aScriptType2 = rDfltScriptType;
                aMacroTable.Insert( nEvent, SvxMacro( sEvent, aScriptType2, eScriptType2 ) );
            }
        }
    }

    const uno::Reference< XMultiServiceFactory > & rSrvcMgr =
        m_pFormImpl->GetServiceFactory();
    if( !rSrvcMgr.is() )
        return;

    uno::Reference< XInterface > xInt = rSrvcMgr->createInstance(
        "com.sun.star.form.component.Form" );
    if( !xInt.is() )
        return;

    uno::Reference< XForm >  xForm( xInt, UNO_QUERY );
    OSL_ENSURE( xForm.is(), "keine Form?" );

    uno::Reference< container::XIndexContainer > xFormComps( xForm, UNO_QUERY );
    m_pFormImpl->SetFormComps( xFormComps );

    uno::Reference< beans::XPropertySet > xFormPropSet( xForm, UNO_QUERY );

    Any aTmp;
    aTmp <<= OUString(sName);
    xFormPropSet->setPropertyValue("Name", aTmp );

    if( !aAction.isEmpty() )
    {
        aAction = URIHelper::SmartRel2Abs(INetURLObject(m_sBaseURL), aAction, Link<OUString *, bool>(), false);
    }
    else
    {
        // Bei leerer URL das Directory nehmen
        INetURLObject aURLObj( m_aPathToFile );
        aAction = aURLObj.GetPartBeforeLastName();
    }
    aTmp <<= OUString(aAction);
    xFormPropSet->setPropertyValue("TargetURL",
                                    aTmp );

    FormSubmitMethod eMethod = (FormSubmitMethod)nMethod;
    aTmp.setValue( &eMethod, cppu::UnoType<FormSubmitMethod>::get());
    xFormPropSet->setPropertyValue("SubmitMethod",
                                    aTmp );

     FormSubmitEncoding eEncType = (FormSubmitEncoding)nEncType;
    aTmp.setValue( &eEncType, cppu::UnoType<FormSubmitEncoding>::get());
    xFormPropSet->setPropertyValue("SubmitEncoding", aTmp );

    if( !sTarget.isEmpty() )
    {
        aTmp <<= sTarget;
        xFormPropSet->setPropertyValue( "TargetFrame", aTmp );
    }

    const uno::Reference< container::XIndexContainer > & rForms =
        m_pFormImpl->GetForms();
    Any aAny( &xForm, cppu::UnoType<XForm>::get());
    rForms->insertByIndex( rForms->getCount(), aAny );
    if( !aMacroTable.empty() )
        lcl_html_setEvents( m_pFormImpl->GetFormEventManager(),
                            rForms->getCount() - 1,
                            aMacroTable, aUnoMacroTable, aUnoMacroParamTable,
                            rDfltScriptType );
}

void SwHTMLParser::EndForm( bool bAppend )
{
    if( m_pFormImpl && m_pFormImpl->GetFormComps().is() )
    {
        if( bAppend )
        {
            if( m_pPam->GetPoint()->nContent.GetIndex() )
                AppendTextNode( AM_SPACE );
            else
                AddParSpace();
        }

        m_pFormImpl->ReleaseFormComps();
    }
}

void SwHTMLParser::InsertInput()
{
    assert(m_pPendStack == nullptr);

    if( !m_pFormImpl || !m_pFormImpl->GetFormComps().is() )
        return;

    OUString sImgSrc, aId, aClass, aStyle, sName;
    OUString sText;
    SvxMacroTableDtor aMacroTable;
    std::vector<OUString> aUnoMacroTable;
    std::vector<OUString> aUnoMacroParamTable;
    sal_uInt16 nSize = 0;
    sal_Int16 nMaxLen = 0;
    sal_Int16 nChecked = TRISTATE_FALSE;
    sal_Int32 nTabIndex = TABINDEX_MAX + 1;
    HTMLInputType eType = HTML_IT_TEXT;
    bool bDisabled = false, bValue = false;
    bool bSetGrfWidth = false, bSetGrfHeight = false;
    bool bHidden = false;
    long nWidth=0, nHeight=0;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    SvKeyValueIterator *pHeaderAttrs = m_pFormImpl->GetHeaderAttrs();
    ScriptType eDfltScriptType = GetScriptType( pHeaderAttrs );
    const OUString& rDfltScriptType = GetScriptTypeString( pHeaderAttrs );

    sal_uInt16 nKeepCRLFToken = HTML_O_VALUE;
    const HTMLOptions& rHTMLOptions = GetOptions( &nKeepCRLFToken );
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        ScriptType eScriptType2 = eDfltScriptType;
        sal_uInt16 nEvent = 0;
        bool bSetEvent = false;

        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_TYPE:
            eType = rOption.GetInputType();
            break;
        case HTML_O_NAME:
            sName = rOption.GetString();
            break;
        case HTML_O_VALUE:
            sText = rOption.GetString();
            bValue = true;
            break;
        case HTML_O_CHECKED:
            nChecked = TRISTATE_TRUE;
            break;
        case HTML_O_DISABLED:
            bDisabled = true;
            break;
        case HTML_O_MAXLENGTH:
            nMaxLen = (sal_Int16)rOption.GetNumber();
            break;
        case HTML_O_SIZE:
            nSize = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_SRC:
            sImgSrc = rOption.GetString();
            break;
        case HTML_O_WIDTH:
            // erstmal nur als Pixelwerte merken!
            nWidth = rOption.GetNumber();
            break;
        case HTML_O_HEIGHT:
            // erstmal nur als Pixelwerte merken!
            nHeight = rOption.GetNumber();
            break;
        case HTML_O_ALIGN:
            eVertOri =
                rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
            eHoriOri =
                rOption.GetEnum( aHTMLImgHAlignTable, eHoriOri );
            break;
        case HTML_O_TABINDEX:
            // erstmal nur als Pixelwerte merken!
            nTabIndex = rOption.GetNumber();
            break;

        case HTML_O_SDONFOCUS:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONFOCUS:
            nEvent = HTML_ET_ONGETFOCUS;
            bSetEvent = true;
            break;

        case HTML_O_SDONBLUR:               // eigtl. nur EDIT
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONBLUR:
            nEvent = HTML_ET_ONLOSEFOCUS;
            bSetEvent = true;
            break;

        case HTML_O_SDONCLICK:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONCLICK:
            nEvent = HTML_ET_ONCLICK;
            bSetEvent = true;
            break;

        case HTML_O_SDONCHANGE:             // eigtl. nur EDIT
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONCHANGE:
            nEvent = HTML_ET_ONCHANGE;
            bSetEvent = true;
            break;

        case HTML_O_SDONSELECT:             // eigtl. nur EDIT
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONSELECT:
            nEvent = HTML_ET_ONSELECT;
            bSetEvent = true;
            break;

        default:
            lcl_html_getEvents( rOption.GetTokenString(),
                                rOption.GetString(),
                                aUnoMacroTable, aUnoMacroParamTable );
            break;
        }

        if( bSetEvent )
        {
            OUString sEvent( rOption.GetString() );
            if( !sEvent.isEmpty() )
            {
                sEvent = convertLineEnd(sEvent, GetSystemLineEnd());
                OUString aScriptType2;
                if( EXTENDED_STYPE==eScriptType2 )
                    aScriptType2 = rDfltScriptType;
                aMacroTable.Insert( nEvent, SvxMacro( sEvent, aScriptType2, eScriptType2 ) );
            }
        }
    }

    if( HTML_IT_IMAGE==eType )
    {
        // Image-Controls ohne Image-URL werden ignoriert (wie bei MS)
        if( sImgSrc.isEmpty() )
            return;
    }
    else
    {
        // ALIGN fuer alle Controls auszuwerten ist keine so gute Idee,
        // solange Absatz-gebundene Controls die Hoehe von Tabellen-Zellen
        // nicht beeinflussen
        eVertOri = text::VertOrientation::TOP;
        eHoriOri = text::HoriOrientation::NONE;
    }

    // Defaults entsprechen HTML_IT_TEXT
    const sal_Char *pType = "TextField";
    bool bKeepCRLFInValue = false;
    switch( eType )
    {
    case HTML_IT_CHECKBOX:
        pType = "CheckBox";
        bKeepCRLFInValue = true;
        break;

    case HTML_IT_RADIO:
        pType = "RadioButton";
        bKeepCRLFInValue = true;
        break;

    case HTML_IT_PASSWORD:
        bKeepCRLFInValue = true;
        break;

    case HTML_IT_BUTTON:
        bKeepCRLFInValue = true;
    case HTML_IT_SUBMIT:
    case HTML_IT_RESET:
        pType = "CommandButton";
        break;

    case HTML_IT_IMAGE:
        pType = "ImageButton";
        break;

    case HTML_IT_FILE:
        pType = "FileControl";
        break;

    case HTML_IT_HIDDEN:
        pType = "HiddenControl";
        bKeepCRLFInValue = true;
        break;
    default:
        ;
    }

    // Fuer ein par Controls mussen CR/LF noch aus dem VALUE
    // geloescht werden.
    if( !bKeepCRLFInValue )
    {
        sText = comphelper::string::remove(sText, '\r');
        sText = comphelper::string::remove(sText, '\n');
    }

    const uno::Reference< XMultiServiceFactory > & rServiceFactory =
        m_pFormImpl->GetServiceFactory();
    if( !rServiceFactory.is() )
        return;

    OUString sServiceName("com.sun.star.form.component.");
    sServiceName += OUString::createFromAscii(pType);
    uno::Reference< XInterface > xInt =
        rServiceFactory->createInstance( sServiceName );
    if( !xInt.is() )
        return;

    uno::Reference< XFormComponent > xFComp( xInt, UNO_QUERY );
    if( !xFComp.is() )
        return;

    uno::Reference< beans::XPropertySet > xPropSet( xFComp, UNO_QUERY );

    Any aTmp;
    aTmp <<= OUString(sName);
    xPropSet->setPropertyValue("Name", aTmp );

    if( HTML_IT_HIDDEN != eType  )
    {
        if( nTabIndex >= TABINDEX_MIN && nTabIndex <= TABINDEX_MAX  )
        {
            aTmp <<= (sal_Int16) (sal_Int16)nTabIndex ;
            xPropSet->setPropertyValue("TabIndex", aTmp );
        }

        if( bDisabled )
        {
            xPropSet->setPropertyValue("Enabled", makeAny(false) );
        }
    }

    aTmp <<= sText;

    Size aSz( 0, 0 );       // defaults
    Size aTextSz( 0, 0 );   // Text-Size
    bool bMinWidth = false, bMinHeight = false;
    bool bUseSize = false;
    switch( eType )
    {
    case HTML_IT_CHECKBOX:
    case HTML_IT_RADIO:
        {
            if( !bValue )
                aTmp <<= OUString( OOO_STRING_SVTOOLS_HTML_on );
            xPropSet->setPropertyValue("RefValue",
                                        aTmp );
            aTmp <<= OUString();
            xPropSet->setPropertyValue("Label",
                                        aTmp );
            // Beim RadioButton darf die DefaultChecked-Property
            // erst gesetzt werden, wenn das Control angelegt und ein
            // activateTabOrder gerufen wurde, weil es sonst noch zu der
            // vorhergehenden Gruppe gehoert.
            if( HTML_IT_CHECKBOX == eType )
            {
                aTmp <<= (sal_Int16) nChecked ;
                xPropSet->setPropertyValue("DefaultState", aTmp );
            }

            const SvxMacro* pMacro = aMacroTable.Get( HTML_ET_ONCLICK );
            if( pMacro )
            {
                aMacroTable.Insert( HTML_ET_ONCLICK_ITEM, *pMacro );
                aMacroTable.Erase( HTML_ET_ONCLICK );
            }
            // SIZE auszuwerten duerfte hier keinen Sinn machen???
            bMinWidth = bMinHeight = true;
        }
        break;

    case HTML_IT_IMAGE:
        {
            // SIZE = WIDTH
            aSz.Width() = nSize ? nSize : nWidth;
            aSz.Width() = nWidth;
            aSz.Height() = nHeight;
            if( (aSz.Width() || aSz.Height()) && Application::GetDefaultDevice() )
            {
                aSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aSz, MapMode( MAP_100TH_MM ) );
            }
             FormButtonType eButtonType = FormButtonType_SUBMIT;
            aTmp.setValue( &eButtonType,
                           cppu::UnoType<FormButtonType>::get());
            xPropSet->setPropertyValue("ButtonType", aTmp );

            aTmp <<= (sal_Int16) 0  ;
            xPropSet->setPropertyValue("Border",
                                        aTmp );
        }
        break;

    case HTML_IT_BUTTON:
    case HTML_IT_SUBMIT:
    case HTML_IT_RESET:
        {
             FormButtonType eButtonType;
            switch( eType )
            {
            case HTML_IT_BUTTON:
                eButtonType = FormButtonType_PUSH;
                break;
            case HTML_IT_SUBMIT:
                eButtonType = FormButtonType_SUBMIT;
                if (sText.isEmpty())
                    sText = OOO_STRING_SVTOOLS_HTML_IT_submit;
                break;
            case HTML_IT_RESET:
                eButtonType = FormButtonType_RESET;
                if (sText.isEmpty())
                    sText = OOO_STRING_SVTOOLS_HTML_IT_reset;
                break;
            default:
                ;
            }
            aTmp <<= sText;
            xPropSet->setPropertyValue("Label",
                                        aTmp );

            aTmp.setValue( &eButtonType,
                           cppu::UnoType<FormButtonType>::get());
            xPropSet->setPropertyValue("ButtonType", aTmp );

            bMinWidth = bMinHeight = true;
            bUseSize = true;
        }
        break;

    case HTML_IT_PASSWORD:
    case HTML_IT_TEXT:
    case HTML_IT_FILE:
        if( HTML_IT_FILE != eType )
        {
        // Beim File-Control wird der VALUE aus Sicherheitsgruenden ignoriert.
            xPropSet->setPropertyValue("DefaultText", aTmp );
            if( nMaxLen != 0 )
            {
                aTmp <<= (sal_Int16) nMaxLen ;
                xPropSet->setPropertyValue("MaxTextLen", aTmp );
            }
        }

        if( HTML_IT_PASSWORD == eType )
        {
            aTmp <<= (sal_Int16)'*' ;
            xPropSet->setPropertyValue("EchoChar", aTmp );
        }

        lcl_html_setFixedFontProperty( xPropSet );

        if( !nSize )
            nSize = 20;
        aTextSz.Width() = nSize;
        bMinHeight = true;
        break;

    case HTML_IT_HIDDEN:
        xPropSet->setPropertyValue("HiddenValue", aTmp );
        bHidden = true;
        break;
    default:
        ;
    }

    if( bUseSize && nSize>0 )
    {
        if( Application::GetDefaultDevice() )
        {
            Size aNewSz( nSize, 0 );
            aNewSz = Application::GetDefaultDevice()
                        ->PixelToLogic( aNewSz, MapMode( MAP_100TH_MM ) );
            aSz.Width() = aNewSz.Width();
            OSL_ENSURE( !aTextSz.Width(), "Text-Breite ist gegeben" );
            bMinWidth = false;
        }
    }

    SfxItemSet aCSS1ItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aCSS1PropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        ParseStyleOptions( aStyle, aId, aClass, aCSS1ItemSet, aCSS1PropInfo );
        if( !aId.isEmpty() )
            InsertBookmark( aId );
    }

    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eWidthType )
    {
        aSz.Width() = convertTwipToMm100( aCSS1PropInfo.nWidth );
        aTextSz.Width() = 0;
        bMinWidth = false;
    }
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eHeightType )
    {
        aSz.Height() = convertTwipToMm100( aCSS1PropInfo.nHeight );
        aTextSz.Height() = 0;
        bMinHeight = false;
    }

    // Beim Image-Button bei nicht gegebern Groesse einen sinnvollen Default
    // setzen
    if( HTML_IT_IMAGE== eType )
    {
        if( !aSz.Width() )
        {
            aSz.Width() = HTML_DFLT_IMG_WIDTH;
            bSetGrfWidth = true;
            if( m_pTable != nullptr )
                IncGrfsThatResizeTable();
        }
        if( !aSz.Height() )
        {
            aSz.Height() = HTML_DFLT_IMG_HEIGHT;
            bSetGrfHeight = true;
        }
    }
    if( aSz.Width() < MINFLY )
        aSz.Width() = MINFLY;
    if( aSz.Height() < MINFLY )
        aSz.Height() = MINFLY;

    uno::Reference< drawing::XShape > xShape = InsertControl(
                                             xFComp, xPropSet, aSz,
                                             eVertOri, eHoriOri,
                                             aCSS1ItemSet, aCSS1PropInfo,
                                             aMacroTable, aUnoMacroTable,
                                             aUnoMacroParamTable, false,
                                             bHidden );
    if( aTextSz.Width() || aTextSz.Height() || bMinWidth || bMinHeight )
    {
        OSL_ENSURE( !(bSetGrfWidth || bSetGrfHeight), "Grafikgroesse anpassen???" );
        SetControlSize( xShape, aTextSz, bMinWidth, bMinHeight );
    }

    if( HTML_IT_RADIO == eType )
    {
        aTmp <<= (sal_Int16) nChecked ;
        xPropSet->setPropertyValue("DefaultState", aTmp );
    }

    if( HTML_IT_IMAGE == eType )
    {
        // Die URL erst nach dem Einfuegen setzen, weil sich der
        // Download der Grafik erst dann am XModel anmelden kann,
        // wenn das Control eingefuegt ist.
        aTmp <<= OUString( URIHelper::SmartRel2Abs(INetURLObject(m_sBaseURL), sImgSrc, Link<OUString *, bool>(), false));
        xPropSet->setPropertyValue("ImageURL",
                                    aTmp );
    }

    if( bSetGrfWidth || bSetGrfHeight )
    {
        SwHTMLImageWatcher* pWatcher =
            new SwHTMLImageWatcher( xShape, bSetGrfWidth, bSetGrfHeight );
        uno::Reference< awt::XImageConsumer > xCons = pWatcher;
        pWatcher->start();
    }
}

void SwHTMLParser::NewTextArea()
{
    assert(m_pPendStack == nullptr);

    OSL_ENSURE( !m_bTextArea, "TextArea in TextArea???" );
    OSL_ENSURE( !m_pFormImpl || !m_pFormImpl->GetFCompPropSet().is(),
            "TextArea in Control???" );

    if( !m_pFormImpl || !m_pFormImpl->GetFormComps().is() )
    {
        // Spezialbehandlung fuer TextArea auch untem im Parser beenden
        FinishTextArea();
        return;
    }

    OUString aId, aClass, aStyle;
    OUString sName;
    sal_Int32 nTabIndex = TABINDEX_MAX + 1;
    SvxMacroTableDtor aMacroTable;
    std::vector<OUString> aUnoMacroTable;
    std::vector<OUString> aUnoMacroParamTable;
    sal_uInt16 nRows = 0, nCols = 0;
    sal_uInt16 nWrap = HTML_WM_OFF;
    bool bDisabled = false;
    SvKeyValueIterator *pHeaderAttrs = m_pFormImpl->GetHeaderAttrs();
    ScriptType eDfltScriptType = GetScriptType( pHeaderAttrs );
    const OUString& rDfltScriptType = GetScriptTypeString( pHeaderAttrs );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        ScriptType eScriptType2 = eDfltScriptType;
        sal_uInt16 nEvent = 0;
        bool bSetEvent = false;

        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_NAME:
            sName = rOption.GetString();
            break;
        case HTML_O_DISABLED:
            bDisabled = true;
            break;
        case HTML_O_ROWS:
            nRows = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_COLS:
            nCols = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_WRAP:
            nWrap = rOption.GetEnum( aHTMLTextAreaWrapTable, nWrap );
            break;

        case HTML_O_TABINDEX:
            nTabIndex = rOption.GetSNumber();
            break;

        case HTML_O_SDONFOCUS:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONFOCUS:
            nEvent = HTML_ET_ONGETFOCUS;
            bSetEvent = true;
            break;

        case HTML_O_SDONBLUR:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONBLUR:
            nEvent = HTML_ET_ONLOSEFOCUS;
            bSetEvent = true;
            break;

        case HTML_O_SDONCLICK:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONCLICK:
            nEvent = HTML_ET_ONCLICK;
            bSetEvent = true;
            break;

        case HTML_O_SDONCHANGE:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONCHANGE:
            nEvent = HTML_ET_ONCHANGE;
            bSetEvent = true;
            break;

        case HTML_O_SDONSELECT:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONSELECT:
            nEvent = HTML_ET_ONSELECT;
            bSetEvent = true;
            break;

        default:
            lcl_html_getEvents( rOption.GetTokenString(),
                                rOption.GetString(),
                                aUnoMacroTable, aUnoMacroParamTable );
            break;
        }

        if( bSetEvent )
        {
            OUString sEvent( rOption.GetString() );
            if( !sEvent.isEmpty() )
            {
                sEvent = convertLineEnd(sEvent, GetSystemLineEnd());
                if( EXTENDED_STYPE==eScriptType2 )
                    m_aScriptType = rDfltScriptType;
                aMacroTable.Insert( nEvent, SvxMacro( sEvent, m_aScriptType, eScriptType2 ) );
            }
        }
    }

    const uno::Reference< lang::XMultiServiceFactory > & rSrvcMgr =
        m_pFormImpl->GetServiceFactory();
    if( !rSrvcMgr.is() )
    {
        FinishTextArea();
        return;
    }
    uno::Reference< uno::XInterface >  xInt = rSrvcMgr->createInstance(
        "com.sun.star.form.component.TextField" );
    if( !xInt.is() )
    {
        FinishTextArea();
        return;
    }

    uno::Reference< XFormComponent > xFComp( xInt, UNO_QUERY );
    OSL_ENSURE( xFComp.is(), "keine FormComponent?" );

    uno::Reference< beans::XPropertySet > xPropSet( xFComp, UNO_QUERY );

    Any aTmp;
    aTmp <<= OUString(sName);
    xPropSet->setPropertyValue("Name", aTmp );

    aTmp <<= true;
    xPropSet->setPropertyValue("MultiLine", aTmp );
    xPropSet->setPropertyValue("VScroll", aTmp );
    if( HTML_WM_OFF == nWrap )
        xPropSet->setPropertyValue("HScroll", aTmp );
    if( HTML_WM_HARD == nWrap )
        xPropSet->setPropertyValue("HardLineBreaks", aTmp );

    if( nTabIndex >= TABINDEX_MIN && nTabIndex <= TABINDEX_MAX  )
    {
        aTmp <<= (sal_Int16)nTabIndex ;
        xPropSet->setPropertyValue("TabIndex", aTmp );
    }

    lcl_html_setFixedFontProperty( xPropSet );

    if( bDisabled )
    {
        xPropSet->setPropertyValue("Enabled", makeAny(false) );
    }

    OSL_ENSURE( m_pFormImpl->GetText().isEmpty(), "Text ist nicht leer!" );

    if( !nCols )
        nCols = 20;
    if( !nRows )
        nRows = 1;

    Size aTextSz( nCols, nRows );

    SfxItemSet aCSS1ItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aCSS1PropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        ParseStyleOptions( aStyle, aId, aClass, aCSS1ItemSet, aCSS1PropInfo );
        if( !aId.isEmpty() )
            InsertBookmark( aId );
    }

    Size aSz( MINFLY, MINFLY );
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eWidthType )
    {
        aSz.Width() = convertTwipToMm100( aCSS1PropInfo.nWidth );
        aTextSz.Width() = 0;
    }
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eHeightType )
    {
        aSz.Height() = convertTwipToMm100( aCSS1PropInfo.nHeight );
        aTextSz.Height() = 0;
    }
    if( aSz.Width() < MINFLY )
        aSz.Width() = MINFLY;
    if( aSz.Height() < MINFLY )
        aSz.Height() = MINFLY;

    uno::Reference< drawing::XShape > xShape = InsertControl( xFComp, xPropSet, aSz,
                                      text::VertOrientation::TOP, text::HoriOrientation::NONE,
                                      aCSS1ItemSet, aCSS1PropInfo,
                                      aMacroTable, aUnoMacroTable,
                                      aUnoMacroParamTable );
    if( aTextSz.Width() || aTextSz.Height() )
        SetControlSize( xShape, aTextSz, false, false );

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_TEXTAREA_ON );

    // und PRE/Listing/XMP voruebergehend aussetzen
    SplitPREListingXMP( pCntxt );
    PushContext( pCntxt );

    m_bTextArea = true;
    m_bTAIgnoreNewPara = true;
}

void SwHTMLParser::EndTextArea()
{
    OSL_ENSURE( m_bTextArea, "keine TextArea oder falscher Typ" );
    OSL_ENSURE( m_pFormImpl && m_pFormImpl->GetFCompPropSet().is(),
            "TextArea fehlt" );

    const uno::Reference< beans::XPropertySet > & rPropSet =
        m_pFormImpl->GetFCompPropSet();

    Any aTmp;
    aTmp <<= m_pFormImpl->GetText();
    rPropSet->setPropertyValue("DefaultText", aTmp );
    m_pFormImpl->EraseText();

    m_pFormImpl->ReleaseFCompPropSet();

    // den Kontext holen
    _HTMLAttrContext *pCntxt = PopContext( HTML_TEXTAREA_ON );
    if( pCntxt )
    {
        // und ggf. die Attribute beenden
        EndContext( pCntxt );
        delete pCntxt;
    }

    m_bTextArea = false;
}

void SwHTMLParser::InsertTextAreaText( sal_uInt16 nToken )
{
    OSL_ENSURE( m_bTextArea, "keine TextArea oder falscher Typ" );
    OSL_ENSURE( m_pFormImpl && m_pFormImpl->GetFCompPropSet().is(),
            "TextArea fehlt" );

    OUString& rText = m_pFormImpl->GetText();
    switch( nToken)
    {
    case HTML_TEXTTOKEN:
        rText += aToken;
        break;
    case HTML_NEWPARA:
        if( !m_bTAIgnoreNewPara )
            rText += "\n";    // das ist hier richtig!!!
        break;
    default:
        rText += "<";
        rText += sSaveToken;
        if( !aToken.isEmpty() )
        {
            rText += " ";
            rText += aToken;
        }
        rText += ">";
    }

    m_bTAIgnoreNewPara = false;
}

void SwHTMLParser::NewSelect()
{
    assert(m_pPendStack == nullptr);

    OSL_ENSURE( !m_bSelect, "Select in Select???" );
    OSL_ENSURE( !m_pFormImpl || !m_pFormImpl->GetFCompPropSet().is(),
            "Select in Control???" );

    if( !m_pFormImpl || !m_pFormImpl->GetFormComps().is() )
        return;

    OUString aId, aClass, aStyle;
    OUString sName;
    sal_Int32 nTabIndex = TABINDEX_MAX + 1;
    SvxMacroTableDtor aMacroTable;
    std::vector<OUString> aUnoMacroTable;
    std::vector<OUString> aUnoMacroParamTable;
    bool bMultiple = false;
    bool bDisabled = false;
    m_nSelectEntryCnt = 1;
    SvKeyValueIterator *pHeaderAttrs = m_pFormImpl->GetHeaderAttrs();
    ScriptType eDfltScriptType = GetScriptType( pHeaderAttrs );
    const OUString& rDfltScriptType = GetScriptTypeString( pHeaderAttrs );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        ScriptType eScriptType2 = eDfltScriptType;
        sal_uInt16 nEvent = 0;
        bool bSetEvent = false;

        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_NAME:
            sName = rOption.GetString();
            break;
        case HTML_O_MULTIPLE:
            bMultiple = true;
            break;
        case HTML_O_DISABLED:
            bDisabled = true;
            break;
        case HTML_O_SIZE:
            m_nSelectEntryCnt = (sal_uInt16)rOption.GetNumber();
            break;

        case HTML_O_TABINDEX:
            nTabIndex = rOption.GetSNumber();
            break;

        case HTML_O_SDONFOCUS:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONFOCUS:
            nEvent = HTML_ET_ONGETFOCUS;
            bSetEvent = true;
            break;

        case HTML_O_SDONBLUR:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONBLUR:
            nEvent = HTML_ET_ONLOSEFOCUS;
            bSetEvent = true;
            break;

        case HTML_O_SDONCLICK:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONCLICK:
            nEvent = HTML_ET_ONCLICK;
            bSetEvent = true;
            break;

        case HTML_O_SDONCHANGE:
            eScriptType2 = STARBASIC;
            SAL_FALLTHROUGH;
        case HTML_O_ONCHANGE:
            nEvent = HTML_ET_ONCHANGE;
            bSetEvent = true;
            break;

        default:
            lcl_html_getEvents( rOption.GetTokenString(),
                                rOption.GetString(),
                                aUnoMacroTable, aUnoMacroParamTable );
            break;
        }

        if( bSetEvent )
        {
            OUString sEvent( rOption.GetString() );
            if( !sEvent.isEmpty() )
            {
                sEvent = convertLineEnd(sEvent, GetSystemLineEnd());
                if( EXTENDED_STYPE==eScriptType2 )
                    m_aScriptType = rDfltScriptType;
                aMacroTable.Insert( nEvent, SvxMacro( sEvent, m_aScriptType, eScriptType2 ) );
            }
        }
    }

    const uno::Reference< lang::XMultiServiceFactory > & rSrvcMgr =
        m_pFormImpl->GetServiceFactory();
    if( !rSrvcMgr.is() )
    {
        FinishTextArea();
        return;
    }
    uno::Reference< uno::XInterface >  xInt = rSrvcMgr->createInstance(
        "com.sun.star.form.component.ListBox" );
    if( !xInt.is() )
    {
        FinishTextArea();
        return;
    }

    uno::Reference< XFormComponent > xFComp( xInt, UNO_QUERY );
    OSL_ENSURE(xFComp.is(), "keine FormComponent?");

    uno::Reference< beans::XPropertySet >  xPropSet( xFComp, UNO_QUERY );

    Any aTmp;
    aTmp <<= OUString(sName);
    xPropSet->setPropertyValue("Name", aTmp );

    if( nTabIndex >= TABINDEX_MIN && nTabIndex <= TABINDEX_MAX  )
    {
        aTmp <<= (sal_Int16)nTabIndex ;
        xPropSet->setPropertyValue("TabIndex", aTmp );
    }

    if( bDisabled )
    {
        xPropSet->setPropertyValue("Enabled", makeAny(false) );
    }

    Size aTextSz( 0, 0 );
    bool bMinWidth = true, bMinHeight = true;
    if( !bMultiple && 1==m_nSelectEntryCnt )
    {
        xPropSet->setPropertyValue("Dropdown", makeAny(true) );
    }
    else
    {
        if( m_nSelectEntryCnt <= 1 )      // 4 Zeilen als default
            m_nSelectEntryCnt = 4;

        if( bMultiple )
        {
            xPropSet->setPropertyValue("MultiSelection", makeAny(true) );
        }
        aTextSz.Height() = m_nSelectEntryCnt;
        bMinHeight = false;
    }

    SfxItemSet aCSS1ItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aCSS1PropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        ParseStyleOptions( aStyle, aId, aClass, aCSS1ItemSet, aCSS1PropInfo );
        if( !aId.isEmpty() )
            InsertBookmark( aId );
    }

    Size aSz( MINFLY, MINFLY );
    m_bFixSelectWidth = m_bFixSelectHeight = true;
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eWidthType )
    {
        aSz.Width() = convertTwipToMm100( aCSS1PropInfo.nWidth );
        m_bFixSelectWidth = false;
        bMinWidth = false;
    }
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eHeightType )
    {
        aSz.Height() = convertTwipToMm100( aCSS1PropInfo.nHeight );
        aTextSz.Height() = 0;
        bMinHeight = false;
    }
    if( aSz.Width() < MINFLY )
        aSz.Width() = MINFLY;
    if( aSz.Height() < MINFLY )
        aSz.Height() = MINFLY;

    uno::Reference< drawing::XShape >  xShape = InsertControl( xFComp, xPropSet, aSz,
                                      text::VertOrientation::TOP, text::HoriOrientation::NONE,
                                      aCSS1ItemSet, aCSS1PropInfo,
                                      aMacroTable, aUnoMacroTable,
                                      aUnoMacroParamTable );
    if( m_bFixSelectWidth )
        m_pFormImpl->SetShape( xShape );
    if( aTextSz.Height() || bMinWidth || bMinHeight )
        SetControlSize( xShape, aTextSz, bMinWidth, bMinHeight );

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_SELECT_ON );

    // und PRE/Listing/XMP voruebergehend aussetzen
    SplitPREListingXMP( pCntxt );
    PushContext( pCntxt );

    m_bSelect = true;
}

void SwHTMLParser::EndSelect()
{
    assert(m_pPendStack == nullptr);

    OSL_ENSURE( m_bSelect, "keine Select" );
    OSL_ENSURE( m_pFormImpl && m_pFormImpl->GetFCompPropSet().is(),
            "kein Select-Control" );

    const uno::Reference< beans::XPropertySet > & rPropSet =
        m_pFormImpl->GetFCompPropSet();

    size_t nEntryCnt = m_pFormImpl->GetStringList().size();
    if(!m_pFormImpl->GetStringList().empty())
    {
        Sequence<OUString> aList( (sal_Int32)nEntryCnt );
        Sequence<OUString> aValueList( (sal_Int32)nEntryCnt );
        OUString *pStrings = aList.getArray();
        OUString *pValues = aValueList.getArray();

        for(size_t i = 0; i < nEntryCnt; ++i)
        {
            OUString sText(m_pFormImpl->GetStringList()[i]);
            sText = comphelper::string::stripEnd(sText, ' ');
            pStrings[i] = sText;

            sText = m_pFormImpl->GetValueList()[i];
            pValues[i] = sText;
        }

        rPropSet->setPropertyValue("StringItemList", Any(aList) );

        rPropSet->setPropertyValue("ListSourceType", Any(ListSourceType_VALUELIST) );

        rPropSet->setPropertyValue("ListSource", Any(aValueList) );

        size_t nSelCnt = m_pFormImpl->GetSelectedList().size();
        if( !nSelCnt && 1 == m_nSelectEntryCnt && nEntryCnt )
        {
            // In einer DropDown-Listbox sollte immer ein Eintrag selektiert
            // sein.
            m_pFormImpl->GetSelectedList().insert( m_pFormImpl->GetSelectedList().begin(), 0 );
            nSelCnt = 1;
        }
        Sequence<sal_Int16> aSelList( (sal_Int32)nSelCnt );
        sal_Int16 *pSels = aSelList.getArray();
        for(size_t i = 0; i < nSelCnt; ++i)
        {
            pSels[i] = (sal_Int16)m_pFormImpl->GetSelectedList()[i];
        }
        rPropSet->setPropertyValue("DefaultSelection", Any(aSelList) );

        m_pFormImpl->EraseStringList();
        m_pFormImpl->EraseValueList();
    }

    m_pFormImpl->EraseSelectedList();

    if( m_bFixSelectWidth )
    {
        OSL_ENSURE( m_pFormImpl->GetShape().is(), "Kein Shape gemerkt" );
        Size aTextSz( -1, 0 );
        SetControlSize( m_pFormImpl->GetShape(), aTextSz, false, false );
    }

    m_pFormImpl->ReleaseFCompPropSet();

    // den Kontext holen
    _HTMLAttrContext *pCntxt = PopContext( HTML_SELECT_ON );
    if( pCntxt )
    {
        // und ggf. die Attribute beenden
        EndContext( pCntxt );
        delete pCntxt;
    }

    m_bSelect = false;
}

void SwHTMLParser::InsertSelectOption()
{
    OSL_ENSURE( m_bSelect, "keine Select" );
    OSL_ENSURE( m_pFormImpl && m_pFormImpl->GetFCompPropSet().is(),
            "kein Select-Control" );

    m_bLBEntrySelected = false;
    OUString aValue;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            // erstmal weglassen!!!
            break;
        case HTML_O_SELECTED:
            m_bLBEntrySelected = true;
            break;
        case HTML_O_VALUE:
            aValue = rOption.GetString();
            if( aValue.isEmpty() )
                aValue = "$$$empty$$$";
            break;
        }
    }

    sal_uInt16 nEntryCnt = m_pFormImpl->GetStringList().size();
    m_pFormImpl->GetStringList().push_back(aEmptyOUStr);
    m_pFormImpl->GetValueList().push_back(aValue);
    if( m_bLBEntrySelected )
    {
        m_pFormImpl->GetSelectedList().push_back( nEntryCnt );
    }
}

void SwHTMLParser::InsertSelectText()
{
    OSL_ENSURE( m_bSelect, "keine Select" );
    OSL_ENSURE( m_pFormImpl && m_pFormImpl->GetFCompPropSet().is(),
            "kein Select-Control" );

    if(!m_pFormImpl->GetStringList().empty())
    {
        OUString& rText = m_pFormImpl->GetStringList().back();

        if( !aToken.isEmpty() && ' '==aToken[ 0 ] )
        {
            sal_Int32 nLen = rText.getLength();
            if( !nLen || ' '==rText[nLen-1])
                aToken = aToken.replaceAt( 0, 1, "" );
        }
        if( !aToken.isEmpty() )
            rText += aToken;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
