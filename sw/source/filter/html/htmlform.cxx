/*************************************************************************
 *
 *  $RCSfile: htmlform.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:41:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _IMGCONS_HXX //autogen
#include <vcl/imgcons.hxx>
#endif

#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_OBJUNO_HXX //autogen
#include <sfx2/objuno.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif

#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_LISTSOURCETYPE_HPP_
#include <com/sun/star/form/ListSourceType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITENCODING_HPP_
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITMETHOD_HPP_
#include <com/sun/star/form/FormSubmitMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif

#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _SWTABLE_HXX //autogen
#include <swtable.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _HTMLTBL_HXX //autogen
#include <htmltbl.hxx>
#endif

#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _VIEWSH_HXX //autogen
#include <viewsh.hxx>
#endif

#ifndef _UNODRAW_HXX //autogen
#include <unodraw.hxx>
#endif
#ifndef _UNOOBJ_HXX //autogen
#include <unoobj.hxx>
#endif
#include "dcontact.hxx"

#include "swcss1.hxx"
#include "swhtml.hxx"
#include "htmlform.hxx"


#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx> // helper for implementations
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::form;
using namespace ::rtl;

const sal_uInt16 TABINDEX_MIN = 0;
const sal_uInt16 TABINDEX_MAX = 32767;

static HTMLOptionEnum __FAR_DATA aHTMLFormMethodTable[] =
{
    { sHTML_METHOD_get,     FormSubmitMethod_GET    },
    { sHTML_METHOD_post,    FormSubmitMethod_POST   },
    { 0,                    0                       }
};

static HTMLOptionEnum __FAR_DATA aHTMLFormEncTypeTable[] =
{
    { sHTML_ET_url,         FormSubmitEncoding_URL          },
    { sHTML_ET_multipart,   FormSubmitEncoding_MULTIPART    },
    { sHTML_ET_text,        FormSubmitEncoding_TEXT         },
    { 0,                    0                               }
};

enum HTMLWordWrapMode { HTML_WM_OFF, HTML_WM_HARD, HTML_WM_SOFT };

static HTMLOptionEnum __FAR_DATA aHTMLTextAreaWrapTable[] =
{
    { sHTML_WW_off,     HTML_WM_OFF },
    { sHTML_WW_hard,    HTML_WM_HARD    },
    { sHTML_WW_soft,    HTML_WM_SOFT    },
    { sHTML_WW_physical,HTML_WM_HARD    },
    { sHTML_WW_virtual, HTML_WM_SOFT    },
    { 0,                0               }
};

HTMLEventType __FAR_DATA aEventTypeTable[] =
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

const sal_Char * __FAR_DATA aEventListenerTable[] =
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

const sal_Char * __FAR_DATA aEventMethodTable[] =
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

const sal_Char * __FAR_DATA aEventSDOptionTable[] =
{
    sHTML_O_SDonsubmit,
    sHTML_O_SDonreset,
    sHTML_O_SDonfocus,
    sHTML_O_SDonblur,
    sHTML_O_SDonclick,
    sHTML_O_SDonclick,
    sHTML_O_SDonchange,
    0
};

const sal_Char * __FAR_DATA aEventOptionTable[] =
{
    sHTML_O_onsubmit,
    sHTML_O_onreset,
    sHTML_O_onfocus,
    sHTML_O_onblur,
    sHTML_O_onclick,
    sHTML_O_onclick,
    sHTML_O_onchange,
    0
};

/*  */

class SwHTMLForm_Impl
{
    SwDocShell                  *pDocSh;

    SvKeyValueIterator          *pHeaderAttrs;

    // gecachte Interfaces
    Reference< drawing::XDrawPage >             xDrawPage;
    Reference< container::XIndexContainer >     xForms;
    Reference< drawing::XShapes >               xShapes;
    Reference< XMultiServiceFactory >           xServiceFactory;

    Reference< script::XEventAttacherManager >  xControlEventManager;
    Reference< script::XEventAttacherManager >  xFormEventManager;

    // Kontext-Informationen
    Reference< container::XIndexContainer >     xFormComps;
    Reference< beans::XPropertySet >            xFCompPropSet;
    Reference< drawing::XShape >                xShape;

    String                      sText;
    SvStringsDtor               aStringList;
    SvStringsDtor               aValueList;
    SvUShorts                   aSelectedList;

public:

    SwHTMLForm_Impl( SwDocShell *pDSh ) :
        pDocSh( pDSh ),
        pHeaderAttrs( pDSh ? pDSh->GetHeaderAttributes() : 0 )
    {
        ASSERT( pDocSh, "Keine DocShell, keine Controls" );
    }

    const Reference< XMultiServiceFactory >& GetServiceFactory();
    const Reference< drawing::XDrawPage >& GetDrawPage();
    const Reference< drawing::XShapes >& GetShapes();
    const Reference< script::XEventAttacherManager >& GetControlEventManager();
    const Reference< script::XEventAttacherManager >& GetFormEventManager();
    const Reference< container::XIndexContainer >& GetForms();

    const Reference< container::XIndexContainer >& GetFormComps() const
    {
        return xFormComps;
    }

    void SetFormComps( const Reference< container::XIndexContainer >& r )
    {
        xFormComps = r;
    }

    void ReleaseFormComps() { xFormComps = 0; xControlEventManager = 0; }

    const Reference< beans::XPropertySet >& GetFCompPropSet() const
    {
        return xFCompPropSet;
    }

    void SetFCompPropSet( const Reference< beans::XPropertySet >& r )
    {
        xFCompPropSet = r;
    }

    void ReleaseFCompPropSet() { xFCompPropSet = 0; }

    const Reference< drawing::XShape >& GetShape() const { return xShape; }
    void SetShape( const Reference< drawing::XShape >& r ) { xShape = r; }
    void ReleaseShape() { xShape = 0; }

    String& GetText() { return sText; }
    void EraseText() { sText = aEmptyStr; }

    SvStringsDtor& GetStringList() { return aStringList; }
    void EraseStringList()
    {
        aStringList.DeleteAndDestroy( 0, aStringList.Count() );
    }

    SvStringsDtor& GetValueList() { return aValueList; }
    void EraseValueList()
    {
        aValueList.DeleteAndDestroy( 0, aValueList.Count() );
    }

    SvUShorts& GetSelectedList() { return aSelectedList; }
    void EraseSelectedList()
    {
        aSelectedList.Remove( 0, aSelectedList.Count() );
    }

    SvKeyValueIterator *GetHeaderAttrs() const { return pHeaderAttrs; }
};

const Reference< XMultiServiceFactory >& SwHTMLForm_Impl::GetServiceFactory()
{
    if( !xServiceFactory.is() && pDocSh )
    {
        xServiceFactory =
            Reference< XMultiServiceFactory >( pDocSh->GetBaseModel(),
                                               UNO_QUERY );
        ASSERT( xServiceFactory.is(),
                "XServiceFactory nicht vom Model erhalten" );
    }
    return xServiceFactory;
}


const Reference< drawing::XDrawPage >& SwHTMLForm_Impl::GetDrawPage()
{
    if( !xDrawPage.is() && pDocSh )
    {
        Reference< drawing::XDrawPageSupplier > xTxtDoc( pDocSh->GetBaseModel(),
                                                         UNO_QUERY );
        ASSERT( xTxtDoc.is(),
                "drawing::XDrawPageSupplier nicht vom XModel erhalten" );
        xDrawPage = xTxtDoc->getDrawPage();
        ASSERT( xDrawPage.is(), "drawing::XDrawPage nicht erhalten" );
    }
    return xDrawPage;
}

const Reference< container::XIndexContainer >& SwHTMLForm_Impl::GetForms()
{
    if( !xForms.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {
            Reference< XFormsSupplier > xFormsSupplier( xDrawPage, UNO_QUERY );
            ASSERT( xFormsSupplier.is(),
                    "XFormsSupplier nicht vom drawing::XDrawPage erhalten" );

            Reference< container::XNameContainer > xNameCont =
                xFormsSupplier->getForms();
            xForms = Reference< container::XIndexContainer >( xNameCont,
                                                              UNO_QUERY );

            ASSERT( xForms.is(), "XForms nicht erhalten" );
        }
    }
    return xForms;
}


const Reference< drawing::XShapes > & SwHTMLForm_Impl::GetShapes()
{
    if( !xShapes.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {
            xShapes = Reference< drawing::XShapes >( xDrawPage, UNO_QUERY );
            ASSERT( xShapes.is(),
                    "XShapes nicht vom drawing::XDrawPage erhalten" );
        }
    }
    return xShapes;
}

const Reference< script::XEventAttacherManager >&
                                    SwHTMLForm_Impl::GetControlEventManager()
{
    if( !xControlEventManager.is() && xFormComps.is() )
    {
        xControlEventManager =
            Reference< script::XEventAttacherManager >( xFormComps, UNO_QUERY );
        ASSERT( xControlEventManager.is(),
    "Reference< XEventAttacherManager > nicht von xFormComps erhalten" );
    }

    return xControlEventManager;
}

const Reference< script::XEventAttacherManager >&
    SwHTMLForm_Impl::GetFormEventManager()
{
    if( !xFormEventManager.is() )
    {
        GetForms();
        if( xForms.is() )
        {
            xFormEventManager =
                Reference< script::XEventAttacherManager >( xForms, UNO_QUERY );
            ASSERT( xFormEventManager.is(),
        "Reference< XEventAttacherManager > nicht von xForms erhalten" );
        }
    }

    return xFormEventManager;
}

class SwHTMLImageWatcher :
    public cppu::WeakImplHelper2< awt::XImageConsumer, XEventListener >
{
    Reference< drawing::XShape >        xShape;     // das control
    Reference< XImageProducerSupplier > xSrc;
    Reference< awt::XImageConsumer >    xThis;      // man selbst
    sal_Bool                            bSetWidth;
    sal_Bool                            bSetHeight;

    void clear();

public:

    SwHTMLImageWatcher( const Reference< drawing::XShape > & rShape,
                        sal_Bool bWidth, sal_Bool bHeight );
    ~SwHTMLImageWatcher();

    // startProduction darf nicht im Konstruktor gerufen werden, weil
    // wir und ggf. selbst zerstoeren ... Deshlab eine eigene Methode.
    void start() { xSrc->getImageProducer()->startProduction(); }

    // UNO Anbindung

    // XImageConsumer
    virtual void SAL_CALL init( sal_Int32 Width, sal_Int32 Height)
        throw( uno::RuntimeException );
    virtual void SAL_CALL setColorModel(
            sal_Int16 BitCount, const uno::Sequence< sal_Int32 >& RGBAPal,
            sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask,
            sal_Int32 AlphaMask)
        throw( uno::RuntimeException );
    virtual void SAL_CALL setPixelsByBytes(
            sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height,
            const uno::Sequence< sal_Int8 >& ProducerData,
            sal_Int32 Offset, sal_Int32 Scansize)
        throw( uno::RuntimeException );
    virtual void SAL_CALL setPixelsByLongs(
            sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height,
            const uno::Sequence< sal_Int32 >& ProducerData,
            sal_Int32 Offset, sal_Int32 Scansize)
        throw( uno::RuntimeException );
    virtual void SAL_CALL complete(
            sal_Int32 Status,
            const Reference< awt::XImageProducer > & Producer)
        throw( uno::RuntimeException );

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException);
};

SwHTMLImageWatcher::SwHTMLImageWatcher(
        const Reference< drawing::XShape >& rShape,
        sal_Bool bWidth, sal_Bool bHeight ) :
    xShape( rShape ),
    bSetWidth( bWidth ), bSetHeight( bHeight )
{
    // Die Quelle des Images merken
    Reference< drawing::XControlShape > xControlShape( xShape, UNO_QUERY );
    Reference< awt::XControlModel > xControlModel(
            xControlShape->getControl() );
    xSrc = Reference< XImageProducerSupplier >( xControlModel, UNO_QUERY );
    ASSERT( xSrc.is(), "Kein XImageProducerSupplier" );

    // Als Event-Listener am Shape anmelden, damit wir es beim dispose
    // loslassen k”nnen ...
    Reference< XEventListener > xEvtLstnr = (XEventListener *)this;
    Reference< XComponent > xComp( xShape, UNO_QUERY );
    xComp->addEventListener( xEvtLstnr );

    // Zum Schluss halten wir noch eine Referenz auf uns selbst, damit
    // wir am Leben bleiben ... (eigentlich sollte das nicht neotig sein,
    // weil wir ja noch an diversen anderen Stellen angemeldet sind)
    xThis = (awt::XImageConsumer *)this;

    // und am ImageProducer anmelden, um die Groesse zu erehalten ...
    xSrc->getImageProducer()->addConsumer( xThis );
}

SwHTMLImageWatcher::~SwHTMLImageWatcher()
{
}

void SwHTMLImageWatcher::clear()
{
    // Am Shape als Event-Listener abmelden
    Reference< XEventListener > xEvtLstnr = (XEventListener *)this;
    Reference< XComponent > xComp( xShape, UNO_QUERY );
    xComp->removeEventListener( xEvtLstnr );

    // Am ImageProducer abmelden
    xSrc->getImageProducer()->removeConsumer( xThis );
}

//------------------------------------------------------------------------------

void SwHTMLImageWatcher::init( sal_Int32 Width, sal_Int32 Height )
    throw( uno::RuntimeException )
{
    ASSERT( bSetWidth || bSetHeight,
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

        Reference< beans::XPropertySet > xPropSet( xShape, UNO_QUERY );

        Reference< XUnoTunnel> xTunnel( xPropSet, UNO_QUERY );
        SwXShape *pSwShape = xTunnel.is()
                ? (SwXShape *)xTunnel->getSomething(SwXShape::getUnoTunnelId())
                : 0;

        ASSERT( pSwShape, "Wo ist das SW-Shape?" );
        if( pSwShape )
        {
            SwFrmFmt *pFrmFmt = pSwShape->GetFrmFmt();

            const SwDoc *pDoc = pFrmFmt->GetDoc();
            const SwPosition* pAPos = pFrmFmt->GetAnchor().GetCntntAnchor();
            SwNode *pANd;
            SwTableNode *pTblNd;
            if( pAPos &&
                0 != (pANd = pDoc->GetNodes()[pAPos->nNode]) &&
                0 != (pTblNd = pANd->FindTableNode()) )
            {
                sal_Bool bLastGrf = !pTblNd->GetTable().DecGrfsThatResize();
                SwHTMLTableLayout *pLayout =
                    pTblNd->GetTable().GetHTMLTableLayout();
                if( pLayout )
                {
                    sal_uInt16 nBrowseWidth =
                        pLayout->GetBrowseWidthByTable( *pDoc );

                    if( nBrowseWidth )
                        pLayout->Resize( nBrowseWidth, sal_True, sal_True,
                                         bLastGrf ? HTMLTABLE_RESIZE_NOW
                                                  : 500 );
                }
            }
        }
    }

    // uns selbst abmelden und loeschen
    clear();
    Reference< awt::XImageConsumer >  xTmp = (awt::XImageConsumer*)this;
    xThis = 0;
}

void SwHTMLImageWatcher::setColorModel(
        sal_Int16, const Sequence< sal_Int32 >&, sal_Int32, sal_Int32,
        sal_Int32, sal_Int32 )
    throw( uno::RuntimeException )
{
}

void SwHTMLImageWatcher::setPixelsByBytes(
        sal_Int32, sal_Int32, sal_Int32, sal_Int32,
        const Sequence< sal_Int8 >&, sal_Int32, sal_Int32 )
    throw( uno::RuntimeException )
{
}


void SwHTMLImageWatcher::setPixelsByLongs(
        sal_Int32, sal_Int32, sal_Int32, sal_Int32,
        const Sequence< sal_Int32 >&, sal_Int32, sal_Int32 )
    throw( uno::RuntimeException )
{
}


void SwHTMLImageWatcher::complete( sal_Int32 Status,
        const Reference< awt::XImageProducer >& )
    throw( uno::RuntimeException )
{
    if( IMAGEERROR == Status || IMAGEABORTED == Status )
    {
        // uns selbst abmelden und loeschen
        clear();
        Reference< awt::XImageConsumer > xTmp = (awt::XImageConsumer*)this;
        xThis = 0;
    }
}

void SwHTMLImageWatcher::disposing(const lang::EventObject& evt) throw ( ::com::sun::star::uno::RuntimeException)
{
    Reference< awt::XImageConsumer > xTmp;

    // Wenn das Shape verschwindet soll muessen wir es loslassen
    Reference< drawing::XShape > xTmpShape;
    if( evt.Source == xShape )
    {
        clear();
        xTmp = (awt::XImageConsumer*)this;
        xThis = 0;
    }
}

void SwHTMLParser::DeleteFormImpl()
{
    delete pFormImpl;
    pFormImpl = 0;
}

static void lcl_html_setFixedFontProperty(
        const Reference< beans::XPropertySet >& rPropSet )
{
    Font aFixedFont( OutputDevice::GetDefaultFont(
                                    DEFAULTFONT_FIXED, LANGUAGE_ENGLISH_US,
                                    DEFAULTFONT_FLAGS_ONLYONE )  );
    Any aTmp;
    aTmp <<= OUString( aFixedFont.GetName() );
    rPropSet->setPropertyValue( OUString::createFromAscii("FontName"), aTmp );

    aTmp <<= OUString( aFixedFont.GetStyleName() );
    rPropSet->setPropertyValue( OUString::createFromAscii("FontStyleName"),
                                aTmp );

    aTmp <<= (sal_Int16) aFixedFont.GetFamily();
    rPropSet->setPropertyValue( OUString::createFromAscii("FontFamily"), aTmp );

    aTmp <<= (sal_Int16) aFixedFont.GetCharSet();
    rPropSet->setPropertyValue( OUString::createFromAscii("FontCharset"),
                                aTmp );

    aTmp <<= (sal_Int16) aFixedFont.GetPitch();
    rPropSet->setPropertyValue( OUString::createFromAscii("FontPitch"), aTmp );

    float fVal(10.);
    aTmp.setValue( &fVal, ::getCppuType(&fVal ));
    rPropSet->setPropertyValue( OUString::createFromAscii("FontHeight"), aTmp );
}

class SwHTMLFormPendingStackData_Impl: public SwPendingStackData
{
    Reference< drawing::XShape >    xShape;
    Size            aTextSz;
    sal_Bool        bMinWidth;
    sal_Bool        bMinHeight;

public:

    SwHTMLFormPendingStackData_Impl(
            const Reference< drawing::XShape > & rShape, const Size& rTextSz,
            sal_Bool bMinW, sal_Bool bMinH ) :
        xShape( rShape ),
        aTextSz( rTextSz ),
        bMinWidth( bMinW ),
        bMinHeight( bMinH )
    {}

    const Reference< drawing::XShape >& GetShape() const { return xShape; }
    const Size& GetTextSize() const { return aTextSz; }
    sal_Bool IsMinWidth() const { return bMinWidth; }
    sal_Bool IsMinHeight() const { return bMinHeight; }
};

void SwHTMLParser::SetPendingControlSize( int nToken )
{
    ASSERT( pPendStack, "Wo ist der Pending Stack?" );
    SwHTMLFormPendingStackData_Impl *pData =
        (SwHTMLFormPendingStackData_Impl *)pPendStack->pData;

    SwPendingStack* pTmp = pPendStack->pNext;
    delete pPendStack;
    pPendStack = pTmp;
    ASSERT( !pPendStack, "Wo kommt der Pending-Stack her?" );

    SetControlSize( pData->GetShape(), pData->GetTextSize(),
                    pData->IsMinWidth(), pData->IsMinHeight(),
                    nToken );
    delete pData;
}

void SwHTMLParser::SetControlSize( const Reference< drawing::XShape >& rShape,
                                   const Size& rTextSz,
                                   sal_Bool bMinWidth,
                                   sal_Bool bMinHeight,
                                   int nToken )
{
    if( !rTextSz.Width() && !rTextSz.Height() && !bMinWidth  && !bMinHeight )
        return;

    // Um an den SwXShape* zu gelangen, brauchen wir ein Interface,
    // das auch vom SwXShape implementiert wird.
    Reference< beans::XPropertySet > xPropSet( rShape, UNO_QUERY );

    ViewShell *pVSh;
    pDoc->GetEditShell( &pVSh );
    if( !pVSh && !aLoadEnv.Is() )
    {
        // If there is no view shell by now and the doc shell is an internal
        // one, no view shell will be created. That for, we have to do that of
        // our own. This happens if a linked section is inserted or refreshed.
        SwDocShell *pDocSh = pDoc->GetDocShell();
        if( pDocSh && SFX_CREATE_MODE_INTERNAL == pDocSh->GetCreateMode() )
        {
            SFX_APP()->CreateViewFrame( *pDocSh, 0, sal_True );
            CallStartAction();
            pDoc->GetEditShell( &pVSh );
        }
    }
    if( !pVSh )
    {
        // If there is no view shell by now, but the document is loaded
        // asynchronous, the view shell will be created delayed because
        // the view was locked during the call to DocumentDetected. If this
        // is the case we wait until another call to our DataAvailable
        // link, because the SFX calls it if it creates the view.
        ASSERT( bDocInitalized, "DocumentDetected nocht nicht augerufen" );
        ASSERT( !IsParserWorking() || bDataAvailableLinkSet,
                "Keine ViewShell bei nicht gesetztem DataAvailable-Link" );
        if( IsParserWorking() && bDocInitalized && bDataAvailableLinkSet )
        {
            pPendStack = new SwPendingStack( nToken, pPendStack );
            pPendStack->pData =
                new SwHTMLFormPendingStackData_Impl( rShape, rTextSz,
                                                     bMinWidth, bMinHeight );
            SaveState( nToken );
            eState = SVPAR_PENDING;
        }

        return;
    }

    Reference< XUnoTunnel> xTunnel( xPropSet, UNO_QUERY );
    SwXShape *pSwShape = xTunnel.is()
        ? (SwXShape *)xTunnel->getSomething(SwXShape::getUnoTunnelId())
        : 0;

    ASSERT( pSwShape, "Wo ist das SW-Shape?" );

    // es muss ein Draw-Format sein
    SwFrmFmt *pFrmFmt = pSwShape->GetFrmFmt();
    ASSERT( RES_DRAWFRMFMT == pFrmFmt->Which(), "Kein DrawFrmFmt" );

    // Schauen, ob es ein SdrObject dafuer gibt
    const SdrObject *pObj = pFrmFmt->FindSdrObject();
    ASSERT( pObj, "SdrObject nicht gefunden" );
    ASSERT( FmFormInventor == pObj->GetObjInventor(), "falscher Inventor" );

    SdrUnoObj *pFormObj = PTR_CAST( SdrUnoObj, pObj );
    Reference< awt::XControl > xControl =
        pFormObj->GetUnoControl( pVSh->GetWin() );

    awt::Size aSz( rShape->getSize() );
    awt::Size aNewSz( 0, 0 );
    if( bMinWidth || bMinHeight )
    {
        Reference< awt::XLayoutConstrains > xLC( xControl, UNO_QUERY );
        awt::Size aTmpSz( xLC->getPreferredSize() );
        if( bMinWidth )
            aNewSz.Width = aTmpSz.Width;
        if( bMinHeight )
            aNewSz.Height = aTmpSz.Height;
    }
    if( rTextSz.Width() || rTextSz.Height())
    {
        Reference< awt::XTextLayoutConstrains > xLC( xControl, UNO_QUERY );
        ASSERT( xLC.is(), "kein XTextLayoutConstrains" );
        if( xLC.is() )
        {
            awt::Size aTmpSz( rTextSz.Width(), rTextSz.Height() );
            if( -1 == rTextSz.Width() )
            {
                aTmpSz.Width = 0;
                aTmpSz.Height = nSelectEntryCnt;
            }
            aTmpSz = xLC->getMinimumSize( aTmpSz.Width, aTmpSz.Height );
            if( rTextSz.Width() )
                aNewSz.Width = aTmpSz.Width;
            if( rTextSz.Height() )
                aNewSz.Height = aTmpSz.Height;
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
        const Reference< script::XEventAttacherManager > & rEvtMn,
        sal_uInt32 nPos, const SvxMacroTableDtor& rMacroTbl,
        const SvStringsDtor& rUnoMacroTbl,
        const SvStringsDtor& rUnoMacroParamTbl,
        const String& rType )
{
    // Erstmal muss die Anzahl der Events ermittelt werden ...
    sal_Int32 nEvents = 0;
    for( sal_uInt16 i=0; HTML_ET_END != aEventTypeTable[i]; i++ )
    {
        const SvxMacro *pMacro = rMacroTbl.Get( aEventTypeTable[i] );
        // Solange nicht alle Events implementiert sind, enthaelt die
        // Tabelle auch Leerstrings!
        if( pMacro && aEventListenerTable[i] )
            nEvents++;
    }
    for( i=0; i< rUnoMacroTbl.Count(); i++ )
    {
        const String& rStr = *rUnoMacroTbl[i];
        xub_StrLen nPos = 0;
        if( !rStr.GetToken( 0, '-', nPos ).Len() || STRING_NOTFOUND == nPos )
            continue;
        if( !rStr.GetToken( 0, '-', nPos ).Len() || STRING_NOTFOUND == nPos )
            continue;
        if( nPos < rStr.Len() )
            nEvents++;
    }

    if( 0==nEvents )
        return;

    Sequence<script::ScriptEventDescriptor> aDescs( nEvents );
    script::ScriptEventDescriptor* pDescs = aDescs.getArray();
    sal_Int32 nEvent = 0;

    for( i=0; HTML_ET_END != aEventTypeTable[i]; i++ )
    {
        const SvxMacro *pMacro = rMacroTbl.Get( aEventTypeTable[i] );
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

    for( i=0; i< rUnoMacroTbl.Count(); i++ )
    {
        const String& rStr = *rUnoMacroTbl[i];
        xub_StrLen nPos = 0;
        String sListener( rStr.GetToken( 0, '-', nPos ) );
        if( !sListener.Len() || STRING_NOTFOUND == nPos )
            continue;

        String sMethod( rStr.GetToken( 0, '-', nPos ) );
        if( !sMethod.Len() || STRING_NOTFOUND == nPos )
            continue;

        String sCode( rStr.Copy( nPos ) );
        if( !sCode.Len() )
            continue;

        script::ScriptEventDescriptor& rDesc = pDescs[nEvent++];
        rDesc.ListenerType = sListener;
        rDesc.EventMethod = sMethod;
        rDesc.ScriptType = rType;
        rDesc.ScriptCode = sCode;
        rDesc.AddListenerParam = OUString();

        if( rUnoMacroParamTbl.Count() )
        {
            String sSearch( sListener );
            sSearch += '-';
            sSearch += sMethod;
            sSearch += '-';
            xub_StrLen nLen = sSearch.Len();
            for( sal_uInt16 j=0; j < rUnoMacroParamTbl.Count(); j++ )
            {
                const String& rParam = *rUnoMacroParamTbl[j];
                if( rParam.CompareTo( sSearch, nLen ) == COMPARE_EQUAL &&
                    rParam.Len() > nLen )
                {
                    rDesc.AddListenerParam = rParam.Copy(nLen);
                    break;
                }
            }
        }
    }
    rEvtMn->registerScriptEvents( nPos, aDescs );
}

static void lcl_html_getEvents( const String& rOption, const String& rValue,
                                SvStringsDtor& rUnoMacroTbl,
                                SvStringsDtor& rUnoMacroParamTbl )
{
    if( rOption.CompareIgnoreCaseToAscii( sHTML_O_sdevent,
                            sizeof(sHTML_O_sdevent)-1 ) == COMPARE_EQUAL )
    {
        String *pEvent = new String( rOption.Copy(sizeof(sHTML_O_sdevent)-1) );
        *pEvent += '-';
        *pEvent += rValue;
        rUnoMacroTbl.Insert( pEvent, rUnoMacroTbl.Count() );
    }
    else if( rOption.CompareIgnoreCaseToAscii( sHTML_O_sdaddparam,
                            sizeof(sHTML_O_sdaddparam)-1 ) == COMPARE_EQUAL )
    {
        String *pParam =
                    new String( rOption.Copy( sizeof(sHTML_O_sdaddparam)-1 ) );
        *pParam += '-';
        *pParam += rValue;
        rUnoMacroParamTbl.Insert( pParam, rUnoMacroParamTbl.Count() );
    }
}

Reference< drawing::XShape > SwHTMLParser::InsertControl(
        const Reference< XFormComponent > & rFComp,
        const Reference< beans::XPropertySet > & rFCompPropSet,
        const Size& rSize, SwVertOrient eVertOri, SwHoriOrient eHoriOri,
        SfxItemSet& rCSS1ItemSet, SvxCSS1PropertyInfo& rCSS1PropInfo,
        const SvxMacroTableDtor& rMacroTbl, const SvStringsDtor& rUnoMacroTbl,
        const SvStringsDtor& rUnoMacroParamTbl, sal_Bool bSetFCompPropSet,
        sal_Bool bHidden )
{
    Reference< drawing::XShape >  xShape;

    const Reference< container::XIndexContainer > & rFormComps =
        pFormImpl->GetFormComps();
    Any aAny( &rFComp, ::getCppuType( (Reference< XFormComponent>*)0 ) );
    rFormComps->insertByIndex( rFormComps->getCount(), aAny );

    if( !bHidden )
    {
        Any aTmp;
        sal_uInt16 nLeftSpace = 0, nRightSpace = 0,
                      nUpperSpace = 0, nLowerSpace = 0;

        const Reference< XMultiServiceFactory > & rServiceFactory =
            pFormImpl->GetServiceFactory();
        if( !rServiceFactory.is() )
            return xShape;

        Reference< XInterface > xCreate =
            rServiceFactory ->createInstance(
                OUString::createFromAscii("com.sun.star.drawing.ControlShape"));
        if( !xCreate.is() )
            return xShape;

        xShape = Reference< drawing::XShape >( xCreate, UNO_QUERY );

        DBG_ASSERT( xShape.is(), "XShape nicht erhalten" );
        awt::Size aTmpSz;
        aTmpSz.Width  = rSize.Width();
        aTmpSz.Height = rSize.Height();
        xShape->setSize( aTmpSz );

        Reference< beans::XPropertySet > xShapePropSet( xCreate, UNO_QUERY );

        // linken/rechten Rand setzen
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_LR_SPACE, sal_True,
                                                     &pItem ) )
        {
            // Ggf. den Erstzeilen-Einzug noch plaetten
            const SvxLRSpaceItem *pLRItem = (const SvxLRSpaceItem *)pItem;
            SvxLRSpaceItem aLRItem( *pLRItem );
            aLRItem.SetTxtFirstLineOfst( 0 );
            if( rCSS1PropInfo.bLeftMargin )
            {
                nLeftSpace = TWIP_TO_MM100( aLRItem.GetLeft() );
                rCSS1PropInfo.bLeftMargin = sal_False;
            }
            if( rCSS1PropInfo.bRightMargin )
            {
                nRightSpace = TWIP_TO_MM100( aLRItem.GetRight() );
                rCSS1PropInfo.bRightMargin = sal_False;
            }
            rCSS1ItemSet.ClearItem( RES_LR_SPACE );
        }
        if( nLeftSpace || nRightSpace )
        {
            Any aTmp;
            aTmp <<= (sal_Int32)nLeftSpace;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "LeftMargin" ), aTmp );

            aTmp <<= (sal_Int32)nRightSpace;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "RightMargin" ), aTmp );
        }

        // oberen/unteren Rand setzen
        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_UL_SPACE, sal_True,
                                                     &pItem ) )
        {
            // Ggf. den Erstzeilen-Einzug noch plaetten
            const SvxULSpaceItem *pULItem = (const SvxULSpaceItem *)pItem;
            if( rCSS1PropInfo.bTopMargin )
            {
                nUpperSpace = TWIP_TO_MM100( pULItem->GetUpper() );
                rCSS1PropInfo.bTopMargin = sal_False;
            }
            if( rCSS1PropInfo.bBottomMargin )
            {
                nLowerSpace = TWIP_TO_MM100( pULItem->GetLower() );
                rCSS1PropInfo.bBottomMargin = sal_False;
            }

            rCSS1ItemSet.ClearItem( RES_UL_SPACE );
        }
        if( nUpperSpace || nLowerSpace )
        {
            uno::Any aTmp;
            aTmp <<= (sal_Int32)nUpperSpace;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "TopMargin" ), aTmp );

            aTmp <<= (sal_Int32)nLowerSpace;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "BottomMargin" ), aTmp );
        }

        Reference< beans::XPropertySetInfo > xPropSetInfo =
            rFCompPropSet->getPropertySetInfo();
        OUString sPropName = OUString::createFromAscii( "BackgroundColor" );
        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_BACKGROUND, sal_True,
                                                     &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            const Color &rColor = ((const SvxBrushItem *)pItem)->GetColor();
            /// OD 02.09.2002 #99657#
            /// copy color, if color is not "no fill"/"auto fill"
            if( rColor.GetColor() != COL_TRANSPARENT )
            {
                /// OD 02.09.2002 #99657#
                /// copy complete color with transparency
                aTmp <<= static_cast<sal_Int32>(rColor.GetColor());
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }

        }

        sPropName = OUString::createFromAscii( "TextColor" );
        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_CHRATR_COLOR, sal_True,
                                                     &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp <<= (sal_Int32)((const SvxColorItem *)pItem)->GetValue()
                                                         .GetRGBColor();
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        sPropName = OUString::createFromAscii( "FontHeight" );
        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_CHRATR_FONTSIZE,
                                                     sal_True, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            float fVal =
                ((float)((SvxFontHeightItem *)pItem)->GetHeight()) / 20.;
            aTmp.setValue( &fVal, ::getCppuType(&fVal));
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_CHRATR_FONT, sal_True,
                                                     &pItem ) )
        {
            const SvxFontItem *pFontItem = (SvxFontItem *)pItem;
            sPropName = OUString::createFromAscii( "FontName" );
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= OUString( pFontItem->GetFamilyName() );
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
            sPropName = OUString::createFromAscii( "FontStyleName" );
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= OUString( pFontItem->GetStyleName() );
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
            sPropName = OUString::createFromAscii( "FontFamily" );
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= (sal_Int16)pFontItem->GetFamily() ;
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
            sPropName = OUString::createFromAscii( "FontCharset" );
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= (sal_Int16)pFontItem->GetCharSet() ;
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
            sPropName = OUString::createFromAscii( "FontPitch" );
            if( xPropSetInfo->hasPropertyByName( sPropName ) )
            {
                aTmp <<= (sal_Int16)pFontItem->GetPitch() ;
                rFCompPropSet->setPropertyValue( sPropName, aTmp );
            }
        }

        sPropName = OUString::createFromAscii( "FontWeight" );
        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_CHRATR_WEIGHT,
                                                     sal_True, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            float fVal = VCLUnoHelper::ConvertFontWeight(
                    ((SvxWeightItem *)pItem)->GetWeight() );
            aTmp.setValue( &fVal, ::getCppuType(&fVal));
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        sPropName = OUString::createFromAscii( "FontSlant" );
        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_CHRATR_POSTURE,
                                                     sal_True, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp <<= (sal_Int16)((SvxPostureItem *)pItem)->GetPosture();
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        sPropName = OUString::createFromAscii( "FontUnderline" );
        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_CHRATR_UNDERLINE,
                                                     sal_True, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp <<= (sal_Int16)((SvxUnderlineItem *)pItem)->GetUnderline();
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        sPropName = OUString::createFromAscii( "FontStrikeout" );
        if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_CHRATR_CROSSEDOUT,
                                                     sal_True, &pItem ) &&
            xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp <<= (sal_Int16)((SvxCrossedOutItem *)pItem)->GetStrikeout();
            rFCompPropSet->setPropertyValue( sPropName, aTmp );
        }

        Reference< text::XTextRange >  xTxtRg;
        sal_Int16 nAnchorType = text::TextContentAnchorType_AS_CHARACTER;
        sal_Int16 nSurround;
        sal_Bool bSetPos = sal_False, bSetSurround = sal_False;
        sal_Int32 nXPos, nYPos;
        if( SVX_CSS1_POS_ABSOLUTE == rCSS1PropInfo.ePosition &&
            SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.eLeftType &&
            SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.eTopType )
        {
            const SwStartNode *pFlySttNd =
                pDoc->GetNodes()[pPam->GetPoint()->nNode]->FindFlyStartNode();

            if( pFlySttNd )
            {
                nAnchorType = text::TextContentAnchorType_AT_FRAME;
                SwPaM aPaM( *pFlySttNd );

                Reference< text::XText >  xDummyTxtRef; // unsauber, aber laut OS geht das ...
                xTxtRg = new SwXTextRange( aPaM, xDummyTxtRef );
            }
            else
            {
                nAnchorType = text::TextContentAnchorType_AT_PAGE;
            }
            nXPos = TWIP_TO_MM100( rCSS1PropInfo.nLeft ) + nLeftSpace;
            nYPos = TWIP_TO_MM100( rCSS1PropInfo.nTop ) + nUpperSpace;
            bSetPos = sal_True;

            nSurround = text::WrapTextMode_THROUGHT;
            bSetSurround = sal_True;
        }
        else if( SVX_ADJUST_LEFT == rCSS1PropInfo.eFloat ||
                 HORI_LEFT == eHoriOri )
        {
            nAnchorType = text::TextContentAnchorType_AT_PARAGRAPH;
            nXPos = nLeftSpace;
            nYPos = nUpperSpace;
            bSetPos = sal_True;
            nSurround = text::WrapTextMode_RIGHT;
            bSetSurround = sal_True;
        }
        else if( VERT_NONE != eVertOri )
        {
            sal_Int16 nVertOri;
            switch( eVertOri )
            {
            case VERT_NONE:
                nVertOri = text::VertOrientation::NONE;
                break;
            case VERT_TOP:
                nVertOri = text::VertOrientation::TOP;
                break;
            case VERT_CENTER:
                nVertOri = text::VertOrientation::CENTER;
                break;
            case VERT_BOTTOM:
                nVertOri = text::VertOrientation::BOTTOM;
                break;
            case VERT_CHAR_TOP:
                nVertOri = text::VertOrientation::CHAR_TOP;
                break;
            case VERT_CHAR_CENTER:
                nVertOri = text::VertOrientation::CHAR_CENTER;
                break;
            case VERT_CHAR_BOTTOM:
                nVertOri = text::VertOrientation::CHAR_BOTTOM;
                break;
            case VERT_LINE_TOP:
                nVertOri = text::VertOrientation::LINE_TOP;
                break;
            case VERT_LINE_CENTER:
                nVertOri = text::VertOrientation::LINE_CENTER;
                break;
            case VERT_LINE_BOTTOM:
                nVertOri = text::VertOrientation::LINE_BOTTOM;
                break;
            }
            aTmp <<= (sal_Int16)nVertOri ;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "VertOrient" ), aTmp );
        }

        aTmp <<= (sal_Int16)nAnchorType ;
        xShapePropSet->setPropertyValue(
                OUString::createFromAscii( "AnchorType" ), aTmp );

        if( text::TextContentAnchorType_AT_PAGE == nAnchorType )
        {
            aTmp <<= (sal_Int16) 1 ;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "AnchorPageNo" ), aTmp );
        }
        else
        {
            if( !xTxtRg.is() )
            {
                Reference< text::XText >  xDummyTxtRef; // unsauber, aber laut OS geht das ...
                xTxtRg = new SwXTextRange( *pPam, xDummyTxtRef );
            }

            aTmp.setValue( &xTxtRg,
                           ::getCppuType((Reference< text::XTextRange>*)0));
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "TextRange" ), aTmp );
        }

        if( bSetPos )
        {
            aTmp <<= (sal_Int16)text::HoriOrientation::NONE;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "HoriOrient" ), aTmp );
            aTmp <<= (sal_Int32)nXPos ;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "HoriOrientPosition" ), aTmp );

            aTmp <<= (sal_Int16)text::VertOrientation::NONE;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "VertOrient" ), aTmp );
            aTmp <<= (sal_Int32)nYPos ;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "VertOrientPosition" ), aTmp );
        }
        if( bSetSurround )
        {
            aTmp <<= (sal_Int16)nSurround ;
            xShapePropSet->setPropertyValue(
                    OUString::createFromAscii( "Surround" ), aTmp );
        }

        pFormImpl->GetShapes()->add(xShape);

        // Das Control-Model am Control-Shape setzen
        Reference< drawing::XControlShape > xControlShape( xShape, UNO_QUERY );
        Reference< awt::XControlModel >  xControlModel( rFComp, UNO_QUERY );
        xControlShape->setControl( xControlModel );
    }

    // Da beim Einfuegen der Controls der Fokus gesetzt wird, werden
    // auch schon Fokus-Events verschickt. Damit die nicht evtl. schon
    // vorhendene JavaSCript-Eents rufen, werden die Events nachtraeglich
    // gesetzt.
    if( rMacroTbl.Count() || rUnoMacroTbl.Count() )
    {
        lcl_html_setEvents( pFormImpl->GetControlEventManager(),
                            rFormComps->getCount() - 1,
                            rMacroTbl, rUnoMacroTbl, rUnoMacroParamTbl,
                            GetScriptTypeString(pFormImpl->GetHeaderAttrs()) );
    }

    if( bSetFCompPropSet )
    {
        pFormImpl->SetFCompPropSet( rFCompPropSet );
    }

    return xShape;
}

void SwHTMLParser::NewForm( sal_Bool bAppend )
{
    // Gibt es schon eine Form?
    if( pFormImpl && pFormImpl->GetFormComps().is() )
        return;

    if( bAppend )
    {
        if( pPam->GetPoint()->nContent.GetIndex() )
            AppendTxtNode( AM_SPACE );
        else
            AddParSpace();
    }

    if( !pFormImpl )
        pFormImpl = new SwHTMLForm_Impl( pDoc->GetDocShell() );

    String aAction( sBaseURL );
    String sName, sTarget;
    sal_uInt16 nEncType = FormSubmitEncoding_URL;
    sal_uInt16 nMethod = FormSubmitMethod_GET;
    SvxMacroTableDtor aMacroTbl;
    SvStringsDtor aUnoMacroTbl;
    SvStringsDtor aUnoMacroParamTbl;
    SvKeyValueIterator *pHeaderAttrs = pFormImpl->GetHeaderAttrs();
    ScriptType eDfltScriptType = GetScriptType( pHeaderAttrs );
    const String& rDfltScriptType = GetScriptTypeString( pHeaderAttrs );

    const HTMLOptions *pOptions = GetOptions();
    for( sal_uInt16 i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        ScriptType eScriptType = eDfltScriptType;
        sal_uInt16 nEvent;
        sal_Bool bSetEvent = sal_False;

        switch( pOption->GetToken() )
        {
        case HTML_O_ACTION:
            aAction = pOption->GetString();
            break;
        case HTML_O_METHOD:
            nMethod = pOption->GetEnum( aHTMLFormMethodTable, nMethod );
            break;
        case HTML_O_ENCTYPE:
            nEncType = pOption->GetEnum( aHTMLFormEncTypeTable, nEncType );
            break;
        case HTML_O_TARGET:
            sTarget = pOption->GetString();
            break;
        case HTML_O_NAME:
            sName = pOption->GetString();
            break;

        case HTML_O_SDONSUBMIT:
            eScriptType = STARBASIC;
        case HTML_O_ONSUBMIT:
            nEvent = HTML_ET_ONSUBMITFORM;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONRESET:
            eScriptType = STARBASIC;
        case HTML_O_ONRESET:
            nEvent = HTML_ET_ONRESETFORM;
            bSetEvent = sal_True;
            break;

        default:
            lcl_html_getEvents( pOption->GetTokenString(),
                                pOption->GetString(),
                                aUnoMacroTbl, aUnoMacroParamTbl );
            break;
        }

        if( bSetEvent )
        {
            String sEvent( pOption->GetString() );
            if( sEvent.Len() )
            {
                sEvent.ConvertLineEnd();
                String aScriptType;
                if( EXTENDED_STYPE==eScriptType )
                    aScriptType = rDfltScriptType;
                aMacroTbl.Insert( nEvent, new SvxMacro( sEvent, aScriptType,
                                  eScriptType ) );
            }
        }
    }

    const Reference< XMultiServiceFactory > & rSrvcMgr =
        pFormImpl->GetServiceFactory();
    if( !rSrvcMgr.is() )
        return;

    Reference< XInterface > xInt = rSrvcMgr->createInstance(
            OUString::createFromAscii( "com.sun.star.form.component.Form" ) );
    if( !xInt.is() )
        return;

    Reference< XForm >  xForm( xInt, UNO_QUERY );
    DBG_ASSERT( xForm.is(), "keine Form?" );

    Reference< container::XIndexContainer > xFormComps( xForm, UNO_QUERY );
    pFormImpl->SetFormComps( xFormComps );

    Reference< beans::XPropertySet > xFormPropSet( xForm, UNO_QUERY );

    Any aTmp;
    aTmp <<= OUString(sName);
    xFormPropSet->setPropertyValue( OUString::createFromAscii( "Name" ), aTmp );

    if( aAction.Len() )
    {
        aAction = INetURLObject::RelToAbs(aAction);
    }
    else
    {
        // Bei leerer URL das Directory nehmen
        INetURLObject aURLObj( aPathToFile );
        aAction = aURLObj.GetPartBeforeLastName();
    }
    aTmp <<= OUString(aAction);
    xFormPropSet->setPropertyValue( OUString::createFromAscii( "TargetURL" ),
                                    aTmp );

    FormSubmitMethod eMethod = (FormSubmitMethod)nMethod;
    aTmp.setValue( &eMethod, ::getCppuType((const FormSubmitMethod*)0) );
    xFormPropSet->setPropertyValue( OUString::createFromAscii( "SubmitMethod" ),
                                    aTmp );

     FormSubmitEncoding eEncType = (FormSubmitEncoding)nEncType;
    aTmp.setValue( &eEncType, ::getCppuType((const FormSubmitEncoding*)0) );
    xFormPropSet->setPropertyValue(
            OUString::createFromAscii( "SubmitEncoding" ), aTmp );

    if( sTarget.Len() )
    {
        aTmp <<= OUString(sTarget);
        xFormPropSet->setPropertyValue(
                OUString::createFromAscii( "TargetFrame" ), aTmp );
    }

    const Reference< container::XIndexContainer > & rForms =
        pFormImpl->GetForms();
    Any aAny( &xForm, ::getCppuType((Reference< XForm>*)0) );
    rForms->insertByIndex( rForms->getCount(), aAny );
    if( aMacroTbl.Count() )
        lcl_html_setEvents( pFormImpl->GetFormEventManager(),
                            rForms->getCount() - 1,
                            aMacroTbl, aUnoMacroTbl, aUnoMacroParamTbl,
                            rDfltScriptType );
}

void SwHTMLParser::EndForm( sal_Bool bAppend )
{
    if( pFormImpl && pFormImpl->GetFormComps().is() )
    {
        if( bAppend )
        {
            if( pPam->GetPoint()->nContent.GetIndex() )
                AppendTxtNode( AM_SPACE );
            else
                AddParSpace();
        }

        pFormImpl->ReleaseFormComps();
    }
}

void SwHTMLParser::InsertInput()
{
    if( pPendStack )
    {
        SetPendingControlSize( HTML_INPUT );
        return;
    }

    if( !pFormImpl || !pFormImpl->GetFormComps().is() )
        return;

    String sImgSrc, aId, aClass, aStyle, sText;
    String sName;
    SvxMacroTableDtor aMacroTbl;
    SvStringsDtor aUnoMacroTbl;
    SvStringsDtor aUnoMacroParamTbl;
    sal_uInt16 nSize = 0;
    sal_Int16 nMaxLen = 0;
    sal_Int16 nChecked = STATE_NOCHECK;
    sal_Int32 nTabIndex = TABINDEX_MAX + 1;
    HTMLInputType eType = HTML_IT_TEXT;
    sal_Bool bDisabled = sal_False, bValue = sal_False;
    sal_Bool bSetGrfWidth = sal_False, bSetGrfHeight = sal_False;
    sal_Bool bHidden = sal_False;
    long nWidth=0, nHeight=0;
    SwVertOrient eVertOri = VERT_TOP;
    SwHoriOrient eHoriOri = HORI_NONE;
    SvKeyValueIterator *pHeaderAttrs = pFormImpl->GetHeaderAttrs();
    ScriptType eDfltScriptType = GetScriptType( pHeaderAttrs );
    const String& rDfltScriptType = GetScriptTypeString( pHeaderAttrs );

    sal_uInt16 nKeepCRLFToken = HTML_O_VALUE;
    const HTMLOptions *pOptions = GetOptions( &nKeepCRLFToken );
    for( sal_uInt16 i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        ScriptType eScriptType = eDfltScriptType;
        sal_uInt16 nEvent;
        sal_Bool bSetEvent = sal_False;

        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        case HTML_O_TYPE:
            eType = pOption->GetInputType();
            break;
        case HTML_O_NAME:
            sName = pOption->GetString();
            break;
        case HTML_O_VALUE:
            sText = pOption->GetString();
            bValue = sal_True;
            break;
        case HTML_O_CHECKED:
            nChecked = STATE_CHECK;
            break;
        case HTML_O_DISABLED:
            bDisabled = sal_True;
            break;
        case HTML_O_MAXLENGTH:
            nMaxLen = (sal_Int16)pOption->GetNumber();
            break;
        case HTML_O_SIZE:
            nSize = (sal_uInt16)pOption->GetNumber();
            break;
        case HTML_O_SRC:
            sImgSrc = pOption->GetString();
            break;
        case HTML_O_WIDTH:
            // erstmal nur als Pixelwerte merken!
            nWidth = pOption->GetNumber();
            break;
        case HTML_O_HEIGHT:
            // erstmal nur als Pixelwerte merken!
            nHeight = pOption->GetNumber();
            break;
        case HTML_O_ALIGN:
            eVertOri =
                (SwVertOrient)pOption->GetEnum( aHTMLImgVAlignTable,
                                                eVertOri );
            eHoriOri =
                (SwHoriOrient)pOption->GetEnum( aHTMLImgHAlignTable,
                                                eHoriOri );
            break;
        case HTML_O_TABINDEX:
            // erstmal nur als Pixelwerte merken!
            nTabIndex = pOption->GetNumber();
            break;

        case HTML_O_SDONFOCUS:
            eScriptType = STARBASIC;
        case HTML_O_ONFOCUS:
            nEvent = HTML_ET_ONGETFOCUS;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONBLUR:               // eigtl. nur EDIT
            eScriptType = STARBASIC;
        case HTML_O_ONBLUR:
            nEvent = HTML_ET_ONLOSEFOCUS;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONCLICK:
            eScriptType = STARBASIC;
        case HTML_O_ONCLICK:
            nEvent = HTML_ET_ONCLICK;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONCHANGE:             // eigtl. nur EDIT
            eScriptType = STARBASIC;
        case HTML_O_ONCHANGE:
            nEvent = HTML_ET_ONCHANGE;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONSELECT:             // eigtl. nur EDIT
            eScriptType = STARBASIC;
        case HTML_O_ONSELECT:
            nEvent = HTML_ET_ONSELECT;
            bSetEvent = sal_True;
            break;

        default:
            lcl_html_getEvents( pOption->GetTokenString(),
                                pOption->GetString(),
                                aUnoMacroTbl, aUnoMacroParamTbl );
            break;
        }

        if( bSetEvent )
        {
            String sEvent( pOption->GetString() );
            if( sEvent.Len() )
            {
                sEvent.ConvertLineEnd();
                String aScriptType;
                if( EXTENDED_STYPE==eScriptType )
                    aScriptType = rDfltScriptType;
                aMacroTbl.Insert( nEvent, new SvxMacro( sEvent, aScriptType,
                                  eScriptType ) );
            }
        }
    }

    if( HTML_IT_IMAGE==eType )
    {
        // Image-Controls ohne Image-URL werden ignoriert (wie bei MS)
        if( !sImgSrc.Len() )
            return;
    }
#if 0
    else if( HTML_IT_HIDDEN!=eType )
    {
        // Obwohl Netscape und MS-IE nur Iamge-Buttons ausrichten koennen,
        // werten wir ALIGN fuer alle Controls aus.
        if( VERT_NONE == eVertOri )
            eVertOri = VERT_TOP;
    }
#else
    else
    {
        // ALIGN fuer alle Controls auszuwerten ist keine so gute Idee,
        // solange Absatz-gebundene Controls die Hoehe von Tabellen-Zellen
        // nicht beeinflussen
        // (#64110#, http://www.telekom.de/katalog-online/onlineshop.html)
        eVertOri = VERT_TOP;
        eHoriOri = HORI_NONE;
    }
#endif

    // Defaults entsprechen HTML_IT_TEXT
    const sal_Char *pType = "TextField";
    sal_Bool bKeepCRLFInValue = sal_False;
    switch( eType )
    {
    case HTML_IT_CHECKBOX:
        pType = "CheckBox";
        bKeepCRLFInValue = sal_True;
        break;

    case HTML_IT_RADIO:
        pType = "RadioButton";
        bKeepCRLFInValue = sal_True;
        break;

    case HTML_IT_PASSWORD:
        bKeepCRLFInValue = sal_True;
        break;

    case HTML_IT_BUTTON:
        bKeepCRLFInValue = sal_True;
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
        bKeepCRLFInValue = sal_True;
        break;
    }

    // Fuer ein par Controls mussen CR/LF noch aus dem VALUE
    // geloescht werden.
    if( !bKeepCRLFInValue )
    {
        sText.EraseAllChars( _CR );
        sText.EraseAllChars( _LF );
    }

    const Reference< XMultiServiceFactory > & rServiceFactory =
        pFormImpl->GetServiceFactory();
    if( !rServiceFactory.is() )
        return;

    String sServiceName(
            OUString::createFromAscii("com.sun.star.form.component.") );
    sServiceName.AppendAscii( pType );
    Reference< XInterface > xInt =
        rServiceFactory->createInstance( sServiceName );
    if( !xInt.is() )
        return;

    Reference< XFormComponent > xFComp( xInt, UNO_QUERY );
    if( !xFComp.is() )
        return;

    Reference< beans::XPropertySet > xPropSet( xFComp, UNO_QUERY );

    Any aTmp;
    aTmp <<= OUString(sName);
    xPropSet->setPropertyValue( OUString::createFromAscii( "Name" ), aTmp );

    if( HTML_IT_HIDDEN != eType  )
    {
        if( nTabIndex >= TABINDEX_MIN && nTabIndex <= TABINDEX_MAX  )
        {
            aTmp <<= (sal_Int16) (sal_Int16)nTabIndex ;
            xPropSet->setPropertyValue( OUString::createFromAscii( "TabIndex" ), aTmp );
        }

        if( bDisabled )
        {
            BOOL bFalse = sal_False;
            aTmp.setValue(&bFalse, ::getBooleanCppuType()  );
            xPropSet->setPropertyValue( OUString::createFromAscii( "Enabled" ), aTmp );
        }
    }

    aTmp <<= OUString(sText);

    Size aSz( 0, 0 );       // defaults
    Size aTextSz( 0, 0 );   // Text-Size
    sal_Bool bMinWidth = sal_False, bMinHeight = sal_False;
    sal_Bool bUseSize = sal_False;
    switch( eType )
    {
    case HTML_IT_CHECKBOX:
    case HTML_IT_RADIO:
        {
            if( !bValue )
                aTmp <<= OUString::createFromAscii( sHTML_on );
            xPropSet->setPropertyValue( OUString::createFromAscii( "RefValue" ),
                                        aTmp );
            aTmp <<= OUString();
            xPropSet->setPropertyValue( OUString::createFromAscii( "Label" ),
                                        aTmp );
            // #53559#: Beim RadioButton darf die DefaultChecked-Property
            // erst gesetzt werden, wenn das Control angelegt und ein
            // activateTabOrder gerufen wurde, weil es sonst noch zu der
            // vorhergehenden Gruppe gehoert.
            if( HTML_IT_CHECKBOX == eType )
            {
                aTmp <<= (sal_Int16) nChecked ;
                xPropSet->setPropertyValue(
                        OUString::createFromAscii( "DefaultState" ), aTmp );
            }

            SvxMacro *pMacro = aMacroTbl.Get( HTML_ET_ONCLICK );
            if( pMacro )
            {
                aMacroTbl.Remove( HTML_ET_ONCLICK );
                aMacroTbl.Insert( HTML_ET_ONCLICK_ITEM, pMacro );
            }
            // SIZE auszuwerten duerfte hier keinen Sinn machen???
            bMinWidth = bMinHeight = sal_True;
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
                           ::getCppuType((const FormButtonType*)0));
            xPropSet->setPropertyValue(
                    OUString::createFromAscii( "ButtonType" ), aTmp );

            aTmp <<= (sal_Int16) 0  ;
            xPropSet->setPropertyValue( OUString::createFromAscii( "Border" ),
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
                if( !sText.Len() )
                    sText.AssignAscii( sHTML_IT_submit );
                break;
            case HTML_IT_RESET:
                eButtonType = FormButtonType_RESET;
                if( !sText.Len() )
                    sText.AssignAscii( sHTML_IT_reset );
                break;
            }
            aTmp <<= OUString(sText);
            xPropSet->setPropertyValue( OUString::createFromAscii( "Label" ),
                                        aTmp );

            aTmp.setValue( &eButtonType,
                           ::getCppuType((const FormButtonType*)0));
            xPropSet->setPropertyValue(
                            OUString::createFromAscii( "ButtonType" ), aTmp );

            bMinWidth = bMinHeight = sal_True;
            bUseSize = sal_True;
        }
        break;

    case HTML_IT_PASSWORD:
    case HTML_IT_TEXT:
    case HTML_IT_FILE:
        if( HTML_IT_FILE != eType )
        {
        // Beim File-Control wird der VALUE aus Sicherheitsgruenden ignoriert.
            xPropSet->setPropertyValue(
                    OUString::createFromAscii( "DefaultText" ), aTmp );
            if( nMaxLen != 0 )
            {
                aTmp <<= (sal_Int16) nMaxLen ;
                xPropSet->setPropertyValue(
                        OUString::createFromAscii( "MaxTextLen" ), aTmp );
            }
        }

        if( HTML_IT_PASSWORD == eType )
        {
            aTmp <<= (sal_Int16)'*' ;
            xPropSet->setPropertyValue( OUString::createFromAscii( "EchoChar" ),
                                        aTmp );
        }

        lcl_html_setFixedFontProperty( xPropSet );

        if( !nSize )
            nSize = 20;
        aTextSz.Width() = nSize;
        bMinHeight = sal_True;
        break;

    case HTML_IT_HIDDEN:
        xPropSet->setPropertyValue( OUString::createFromAscii( "HiddenValue" ),
                                    aTmp );
        bHidden = sal_True;
        break;
    }

    if( bUseSize && nSize>0 )
    {
        if( Application::GetDefaultDevice() )
        {
            Size aNewSz( nSize, 0 );
            aNewSz = Application::GetDefaultDevice()
                        ->PixelToLogic( aNewSz, MapMode( MAP_100TH_MM ) );
            aSz.Width() = aNewSz.Width();
            ASSERT( !aTextSz.Width(), "Text-Breite ist gegeben" );
            bMinWidth = sal_False;
        }
    }

    SfxItemSet aCSS1ItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aCSS1PropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        ParseStyleOptions( aStyle, aId, aClass, aCSS1ItemSet, aCSS1PropInfo );
        if( aId.Len() )
            InsertBookmark( aId );
    }

    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eWidthType )
    {
        aSz.Width() = TWIP_TO_MM100( aCSS1PropInfo.nWidth );
        aTextSz.Width() = 0;
        bMinWidth = sal_False;
    }
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eHeightType )
    {
        aSz.Height() = TWIP_TO_MM100( aCSS1PropInfo.nHeight );
        aTextSz.Height() = 0;
        bMinHeight = sal_False;
    }

    // Beim Image-Button bei nicht gegebern Groesse einen sinnvollen Default
    // setzen
    if( HTML_IT_IMAGE== eType )
    {
        if( !aSz.Width() )
        {
            aSz.Width() = HTML_DFLT_IMG_WIDTH;
            bSetGrfWidth = sal_True;
            if( pTable != 0 )
                IncGrfsThatResizeTable();
        }
        if( !aSz.Height() )
        {
            aSz.Height() = HTML_DFLT_IMG_HEIGHT;
            bSetGrfHeight = sal_True;
        }
    }
    if( aSz.Width() < MINFLY )
        aSz.Width() = MINFLY;
    if( aSz.Height() < MINFLY )
        aSz.Height() = MINFLY;

    Reference< drawing::XShape > xShape = InsertControl(
                                             xFComp, xPropSet, aSz,
                                             eVertOri, eHoriOri,
                                             aCSS1ItemSet, aCSS1PropInfo,
                                             aMacroTbl, aUnoMacroTbl,
                                             aUnoMacroParamTbl, sal_False,
                                             bHidden );
    if( aTextSz.Width() || aTextSz.Height() || bMinWidth || bMinHeight )
    {
        ASSERT( !(bSetGrfWidth || bSetGrfHeight), "Grafikgroesse anpassen???" );
        SetControlSize( xShape, aTextSz, bMinWidth, bMinHeight, HTML_INPUT );
    }

    if( HTML_IT_RADIO == eType )
    {
        aTmp <<= (sal_Int16) nChecked ;
        xPropSet->setPropertyValue( OUString::createFromAscii( "DefaultState" ), aTmp );
    }

    if( HTML_IT_IMAGE == eType )
    {
        // Die URL erst nach dem Einfuegen setzen, weil sich der
        // Download der Grafik erst dann am XModel anmelden kann,
        // wenn das Control eingefuegt ist.
        aTmp <<= OUString(INetURLObject::RelToAbs(sImgSrc));
        xPropSet->setPropertyValue( OUString::createFromAscii( "ImageURL" ),
                                    aTmp );
    }

    if( bSetGrfWidth || bSetGrfHeight )
    {
        SwHTMLImageWatcher* pWatcher =
            new SwHTMLImageWatcher( xShape, bSetGrfWidth, bSetGrfHeight );
        Reference< awt::XImageConsumer > xCons = pWatcher;
        pWatcher->start();
    }
}

void SwHTMLParser::NewTextArea()
{
    if( pPendStack )
    {
        SetPendingControlSize( HTML_TEXTAREA_ON );
        return;
    }

    ASSERT( !bTextArea, "TextArea in TextArea???" );
    ASSERT( !pFormImpl || !pFormImpl->GetFCompPropSet().is(),
            "TextArea in Control???" );

    if( !pFormImpl || !pFormImpl->GetFormComps().is() )
    {
        // Spezialbehandlung fuer TextArea auch untem im Parser beenden
        FinishTextArea();
        return;
    }

    String aId, aClass, aStyle;
    String sName;
    sal_Int32 nTabIndex = TABINDEX_MAX + 1;
    SvxMacroTableDtor aMacroTbl;
    SvStringsDtor aUnoMacroTbl;
    SvStringsDtor aUnoMacroParamTbl;
    sal_uInt16 nRows = 0, nCols = 0;
    sal_uInt16 nWrap = HTML_WM_OFF;
    sal_Bool bDisabled = sal_False;
    SvKeyValueIterator *pHeaderAttrs = pFormImpl->GetHeaderAttrs();
    ScriptType eDfltScriptType = GetScriptType( pHeaderAttrs );
    const String& rDfltScriptType = GetScriptTypeString( pHeaderAttrs );

    const HTMLOptions *pOptions = GetOptions();
    for( sal_uInt16 i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        ScriptType eScriptType = eDfltScriptType;
        sal_uInt16 nEvent;
        sal_Bool bSetEvent = sal_False;

        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        case HTML_O_NAME:
            sName = pOption->GetString();
            break;
        case HTML_O_DISABLED:
            bDisabled = sal_True;
            break;
        case HTML_O_ROWS:
            nRows = (sal_uInt16)pOption->GetNumber();
            break;
        case HTML_O_COLS:
            nCols = (sal_uInt16)pOption->GetNumber();
            break;
        case HTML_O_WRAP:
            nWrap = pOption->GetEnum( aHTMLTextAreaWrapTable, nWrap );
            break;

        case HTML_O_TABINDEX:
            nTabIndex = pOption->GetSNumber();
            break;

        case HTML_O_SDONFOCUS:
            eScriptType = STARBASIC;
        case HTML_O_ONFOCUS:
            nEvent = HTML_ET_ONGETFOCUS;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONBLUR:
            eScriptType = STARBASIC;
        case HTML_O_ONBLUR:
            nEvent = HTML_ET_ONLOSEFOCUS;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONCLICK:
            eScriptType = STARBASIC;
        case HTML_O_ONCLICK:
            nEvent = HTML_ET_ONCLICK;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONCHANGE:
            eScriptType = STARBASIC;
        case HTML_O_ONCHANGE:
            nEvent = HTML_ET_ONCHANGE;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONSELECT:
            eScriptType = STARBASIC;
        case HTML_O_ONSELECT:
            nEvent = HTML_ET_ONSELECT;
            bSetEvent = sal_True;
            break;

        default:
            lcl_html_getEvents( pOption->GetTokenString(),
                                pOption->GetString(),
                                aUnoMacroTbl, aUnoMacroParamTbl );
            break;
        }

        if( bSetEvent )
        {
            String sEvent( pOption->GetString() );
            if( sEvent.Len() )
            {
                sEvent.ConvertLineEnd();
                if( EXTENDED_STYPE==eScriptType )
                    aScriptType = rDfltScriptType;
                aMacroTbl.Insert( nEvent, new SvxMacro( sEvent, aScriptType,
                                  eScriptType ) );
            }
        }
    }


    const Reference< lang::XMultiServiceFactory > & rSrvcMgr =
        pFormImpl->GetServiceFactory();
    if( !rSrvcMgr.is() )
    {
        FinishTextArea();
        return;
    }
    Reference< uno::XInterface >  xInt = rSrvcMgr->createInstance(
        OUString::createFromAscii( "com.sun.star.form.component.TextField" ) );
    if( !xInt.is() )
    {
        FinishTextArea();
        return;
    }

    Reference< XFormComponent > xFComp( xInt, UNO_QUERY );
    DBG_ASSERT( xFComp.is(), "keine FormComponent?" );

    Reference< beans::XPropertySet > xPropSet( xFComp, UNO_QUERY );

    Any aTmp;
    aTmp <<= OUString(sName);
    xPropSet->setPropertyValue( OUString::createFromAscii( "Name" ), aTmp );

    BOOL bTrue = sal_True;
    aTmp.setValue( &bTrue, ::getBooleanCppuType() );
    xPropSet->setPropertyValue( OUString::createFromAscii( "MultiLine" ),
                                aTmp );
    xPropSet->setPropertyValue( OUString::createFromAscii( "VScroll" ), aTmp );
    if( HTML_WM_OFF == nWrap )
        xPropSet->setPropertyValue( OUString::createFromAscii( "HScroll" ),
                                    aTmp );
    if( HTML_WM_HARD == nWrap )
        xPropSet->setPropertyValue(
                OUString::createFromAscii( "HardLineBreaks" ), aTmp );

    if( nTabIndex >= TABINDEX_MIN && nTabIndex <= TABINDEX_MAX  )
    {
        aTmp <<= (sal_Int16)nTabIndex ;
        xPropSet->setPropertyValue( OUString::createFromAscii( "TabIndex" ),
                                    aTmp );
    }

    lcl_html_setFixedFontProperty( xPropSet );

    if( bDisabled )
    {
        BOOL bFalse = sal_False;
        aTmp.setValue( &bFalse, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( OUString::createFromAscii( "Enabled" ),
                                    aTmp );
    }

    ASSERT( !pFormImpl->GetText().Len(), "Text ist nicht leer!" );

    if( !nCols )
        nCols = 20;
    if( !nRows )
        nRows = 1;

    Size aTextSz( nCols, nRows );

    SfxItemSet aCSS1ItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aCSS1PropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        ParseStyleOptions( aStyle, aId, aClass, aCSS1ItemSet, aCSS1PropInfo );
        if( aId.Len() )
            InsertBookmark( aId );
    }

    Size aSz( MINFLY, MINFLY );
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eWidthType )
    {
        aSz.Width() = TWIP_TO_MM100( aCSS1PropInfo.nWidth );
        aTextSz.Width() = 0;
    }
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eHeightType )
    {
        aSz.Height() = TWIP_TO_MM100( aCSS1PropInfo.nHeight );
        aTextSz.Height() = 0;
    }
    if( aSz.Width() < MINFLY )
        aSz.Width() = MINFLY;
    if( aSz.Height() < MINFLY )
        aSz.Height() = MINFLY;

    Reference< drawing::XShape > xShape = InsertControl( xFComp, xPropSet, aSz,
                                      VERT_TOP, HORI_NONE,
                                      aCSS1ItemSet, aCSS1PropInfo,
                                      aMacroTbl, aUnoMacroTbl,
                                      aUnoMacroParamTbl );
    if( aTextSz.Width() || aTextSz.Height() )
        SetControlSize( xShape, aTextSz, sal_False, sal_False,
                        HTML_TEXTAREA_ON );

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_TEXTAREA_ON );

    // und PRE/Listing/XMP voruebergehend aussetzen
    SplitPREListingXMP( pCntxt );
    PushContext( pCntxt );

    bTextArea = sal_True;
    bTAIgnoreNewPara = sal_True;
}

void SwHTMLParser::EndTextArea()
{
    ASSERT( bTextArea, "keine TextArea oder falscher Typ" );
    ASSERT( pFormImpl && pFormImpl->GetFCompPropSet().is(),
            "TextArea fehlt" );

    const Reference< beans::XPropertySet > & rPropSet =
        pFormImpl->GetFCompPropSet();

    Any aTmp;
    aTmp <<= OUString(pFormImpl->GetText());
    rPropSet->setPropertyValue( OUString::createFromAscii( "DefaultText" ),
                                aTmp );
    pFormImpl->EraseText();

    pFormImpl->ReleaseFCompPropSet();

    // den Kontext holen
    _HTMLAttrContext *pCntxt = PopContext( HTML_TEXTAREA_ON );
    if( pCntxt )
    {
        // und ggf. die Attribute beenden
        EndContext( pCntxt );
        delete pCntxt;
    }

    bTextArea = sal_False;
}


void SwHTMLParser::InsertTextAreaText( sal_uInt16 nToken )
{
    ASSERT( bTextArea, "keine TextArea oder falscher Typ" );
    ASSERT( pFormImpl && pFormImpl->GetFCompPropSet().is(),
            "TextArea fehlt" );

    String& rText = pFormImpl->GetText();
    switch( nToken)
    {
    case HTML_TEXTTOKEN:
        rText += aToken;
        break;
    case HTML_NEWPARA:
        if( !bTAIgnoreNewPara )
            rText += '\n';    // das ist hier richtig!!!
        break;
    default:
        rText += '<';
        rText += sSaveToken;
        if( aToken.Len() )
        {
            rText += ' ';
            rText += aToken;
        }
        rText += '>';
    }

    bTAIgnoreNewPara = sal_False;
}

void SwHTMLParser::NewSelect()
{
    if( pPendStack )
    {
        SetPendingControlSize( HTML_SELECT_ON );
        return;
    }

    ASSERT( !bSelect, "Select in Select???" );
    ASSERT( !pFormImpl || !pFormImpl->GetFCompPropSet().is(),
            "Select in Control???" );

    if( !pFormImpl || !pFormImpl->GetFormComps().is() )
        return;

    String aId, aClass, aStyle;
    String sName;
    sal_Int32 nTabIndex = TABINDEX_MAX + 1;
    SvxMacroTableDtor aMacroTbl;
    SvStringsDtor aUnoMacroTbl;
    SvStringsDtor aUnoMacroParamTbl;
    sal_Bool bMultiple = sal_False;
    sal_Bool bDisabled = sal_False;
    nSelectEntryCnt = 1;
    SvKeyValueIterator *pHeaderAttrs = pFormImpl->GetHeaderAttrs();
    ScriptType eDfltScriptType = GetScriptType( pHeaderAttrs );
    const String& rDfltScriptType = GetScriptTypeString( pHeaderAttrs );

    const HTMLOptions *pOptions = GetOptions();
    for( sal_uInt16 i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        ScriptType eScriptType = eDfltScriptType;
        sal_uInt16 nEvent;
        sal_Bool bSetEvent = sal_False;

        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        case HTML_O_NAME:
            sName = pOption->GetString();
            break;
        case HTML_O_MULTIPLE:
            bMultiple = sal_True;
            break;
        case HTML_O_DISABLED:
            bDisabled = sal_True;
            break;
        case HTML_O_SIZE:
            nSelectEntryCnt = (sal_uInt16)pOption->GetNumber();
            break;

        case HTML_O_TABINDEX:
            nTabIndex = pOption->GetSNumber();
            break;

        case HTML_O_SDONFOCUS:
            eScriptType = STARBASIC;
        case HTML_O_ONFOCUS:
            nEvent = HTML_ET_ONGETFOCUS;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONBLUR:
            eScriptType = STARBASIC;
        case HTML_O_ONBLUR:
            nEvent = HTML_ET_ONLOSEFOCUS;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONCLICK:
            eScriptType = STARBASIC;
        case HTML_O_ONCLICK:
            nEvent = HTML_ET_ONCLICK;
            bSetEvent = sal_True;
            break;

        case HTML_O_SDONCHANGE:
            eScriptType = STARBASIC;
        case HTML_O_ONCHANGE:
            nEvent = HTML_ET_ONCHANGE;
            bSetEvent = sal_True;
            break;

        default:
            lcl_html_getEvents( pOption->GetTokenString(),
                                pOption->GetString(),
                                aUnoMacroTbl, aUnoMacroParamTbl );
            break;
        }

        if( bSetEvent )
        {
            String sEvent( pOption->GetString() );
            if( sEvent.Len() )
            {
                sEvent.ConvertLineEnd();
                if( EXTENDED_STYPE==eScriptType )
                    aScriptType = rDfltScriptType;
                aMacroTbl.Insert( nEvent, new SvxMacro( sEvent, aScriptType,
                                  eScriptType ) );
            }
        }
    }

    const Reference< lang::XMultiServiceFactory > & rSrvcMgr =
        pFormImpl->GetServiceFactory();
    if( !rSrvcMgr.is() )
    {
        FinishTextArea();
        return;
    }
    Reference< uno::XInterface >  xInt = rSrvcMgr->createInstance(
        OUString::createFromAscii( "com.sun.star.form.component.ListBox" ) );
    if( !xInt.is() )
    {
        FinishTextArea();
        return;
    }

    Reference< XFormComponent > xFComp( xInt, UNO_QUERY );
    DBG_ASSERT(xFComp.is(), "keine FormComponent?")

    Reference< beans::XPropertySet >  xPropSet( xFComp, UNO_QUERY );

    Any aTmp;
    aTmp <<= OUString(sName);
    xPropSet->setPropertyValue( OUString::createFromAscii( "Name" ), aTmp );

    if( nTabIndex >= TABINDEX_MIN && nTabIndex <= TABINDEX_MAX  )
    {
        aTmp <<= (sal_Int16)nTabIndex ;
        xPropSet->setPropertyValue( OUString::createFromAscii( "TabIndex" ),
                                    aTmp );
    }

    if( bDisabled )
    {
        BOOL bFalse = sal_False;
        aTmp.setValue( &bFalse, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( OUString::createFromAscii( "Enabled" ),
                                    aTmp );
    }

    Size aTextSz( 0, 0 );
    sal_Bool bMinWidth = sal_True, bMinHeight = sal_True;
    if( !bMultiple && 1==nSelectEntryCnt )
    {
        BOOL bTrue = sal_True;
        aTmp.setValue( &bTrue, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( OUString::createFromAscii( "Dropdown" ),
                                    aTmp );
    }
    else
    {
        if( nSelectEntryCnt <= 1 )      // 4 Zeilen als default
            nSelectEntryCnt = 4;

        if( bMultiple )
        {
            BOOL bTrue = sal_True;
            aTmp.setValue( &bTrue, ::getBooleanCppuType() );
            xPropSet->setPropertyValue(
                    OUString::createFromAscii( "MultiSelection" ), aTmp );
        }
        aTextSz.Height() = nSelectEntryCnt;
        bMinHeight = sal_False;
    }

    SfxItemSet aCSS1ItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aCSS1PropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        ParseStyleOptions( aStyle, aId, aClass, aCSS1ItemSet, aCSS1PropInfo );
        if( aId.Len() )
            InsertBookmark( aId );
    }

    Size aSz( MINFLY, MINFLY );
    bFixSelectWidth = bFixSelectHeight = sal_True;
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eWidthType )
    {
        aSz.Width() = TWIP_TO_MM100( aCSS1PropInfo.nWidth );
        bFixSelectWidth = sal_False;
        bMinWidth = sal_False;
    }
    if( SVX_CSS1_LTYPE_TWIP== aCSS1PropInfo.eHeightType )
    {
        aSz.Height() = TWIP_TO_MM100( aCSS1PropInfo.nHeight );
        aTextSz.Height() = sal_False;
        bMinHeight = sal_False;
    }
    if( aSz.Width() < MINFLY )
        aSz.Width() = MINFLY;
    if( aSz.Height() < MINFLY )
        aSz.Height() = MINFLY;

    Reference< drawing::XShape >  xShape = InsertControl( xFComp, xPropSet, aSz,
                                      VERT_TOP, HORI_NONE,
                                      aCSS1ItemSet, aCSS1PropInfo,
                                      aMacroTbl, aUnoMacroTbl,
                                      aUnoMacroParamTbl );
    if( bFixSelectWidth )
        pFormImpl->SetShape( xShape );
    if( aTextSz.Height() || bMinWidth || bMinHeight )
        SetControlSize( xShape, aTextSz, bMinWidth, bMinHeight,
                        HTML_SELECT_ON );

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_SELECT_ON );

    // und PRE/Listing/XMP voruebergehend aussetzen
    SplitPREListingXMP( pCntxt );
    PushContext( pCntxt );

    bSelect = sal_True;
}

void SwHTMLParser::EndSelect()
{
    if( pPendStack )
    {
        SetPendingControlSize( HTML_SELECT_OFF );
        return;
    }

    ASSERT( bSelect, "keine Select" );
    ASSERT( pFormImpl && pFormImpl->GetFCompPropSet().is(),
            "kein Select-Control" );

    const Reference< beans::XPropertySet > & rPropSet =
        pFormImpl->GetFCompPropSet();

    // die Groesse anpassen
    Size aNewSz( MINFLY, MINFLY );

    sal_uInt16 nEntryCnt = pFormImpl->GetStringList().Count();
    if( nEntryCnt )
    {
        Sequence<OUString> aList( (sal_Int32)nEntryCnt );
        Sequence<OUString> aValueList( (sal_Int32)nEntryCnt );
        OUString *pStrings = aList.getArray();
        OUString *pValues = aValueList.getArray();
        for( sal_uInt16 i=0; i<nEntryCnt; i++ )
        {
            String sText( *pFormImpl->GetStringList()[i] );
            sText.EraseTrailingChars();
            pStrings[i] = sText;

            sText = *pFormImpl->GetValueList()[i];
            pValues[i] = sText;
        }

        Any aAny( &aList, ::getCppuType((uno::Sequence<OUString>*)0) );

        rPropSet->setPropertyValue(
                OUString::createFromAscii( "StringItemList" ), aAny );

        aAny <<= ListSourceType_VALUELIST;
        rPropSet->setPropertyValue(
                OUString::createFromAscii( "ListSourceType" ), aAny );

        aAny.setValue( &aValueList, ::getCppuType((uno::Sequence<OUString>*)0) );

        rPropSet->setPropertyValue( OUString::createFromAscii( "ListSource" ),
                                    aAny );

        sal_uInt16 nSelCnt = pFormImpl->GetSelectedList().Count();
        if( !nSelCnt && 1 == nSelectEntryCnt && nEntryCnt )
        {
            // In einer DropDown-Listbox sollte immer ein Eintrag selektiert
            // sein.
            pFormImpl->GetSelectedList().Insert( (sal_uInt16)0, (sal_uInt16)0 );
            nSelCnt = 1;
        }
        Sequence<sal_Int16> aSelList( (sal_Int32)nSelCnt );
        sal_Int16 *pSels = aSelList.getArray();
        for( i=0; i<nSelCnt; i++ )
        {
            pSels[i] = (sal_Int16)pFormImpl->GetSelectedList()[i];
        }
        aAny.setValue( &aSelList,
                       ::getCppuType((uno::Sequence<sal_Int16>*)0) );

        rPropSet->setPropertyValue(
                OUString::createFromAscii( "DefaultSelection" ), aAny );

        pFormImpl->EraseStringList();
        pFormImpl->EraseValueList();
    }

    pFormImpl->EraseSelectedList();

    if( bFixSelectWidth )
    {
        ASSERT( pFormImpl->GetShape().is(), "Kein Shape gemerkt" );
        Size aTextSz( -1, 0 );
        SetControlSize( pFormImpl->GetShape(), aTextSz, sal_False, sal_False,
                        HTML_SELECT_OFF );
    }

    pFormImpl->ReleaseFCompPropSet();

    // den Kontext holen
    _HTMLAttrContext *pCntxt = PopContext( HTML_SELECT_ON );
    if( pCntxt )
    {
        // und ggf. die Attribute beenden
        EndContext( pCntxt );
        delete pCntxt;
    }

    bSelect = sal_False;
}

void SwHTMLParser::InsertSelectOption()
{
    ASSERT( bSelect, "keine Select" );
    ASSERT( pFormImpl && pFormImpl->GetFCompPropSet().is(),
            "kein Select-Control" );

    bLBEntrySelected = sal_False;
    String aValue;

    const HTMLOptions *pOptions = GetOptions();
    for( sal_uInt16 i = pOptions->Count(); i; )
    {
        HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            // erstmal weglassen!!!
            break;
        case HTML_O_SELECTED:
            bLBEntrySelected = sal_True;
            break;
        case HTML_O_VALUE:
            aValue = pOption->GetString();
            if( !aValue.Len() )
                aValue.AssignAscii( "$$$empty$$$" );
            break;
        }
    }

    sal_uInt16 nEntryCnt = pFormImpl->GetStringList().Count();
    pFormImpl->GetStringList().Insert( new String( aEmptyStr ), nEntryCnt );
    pFormImpl->GetValueList().Insert( new String( aValue ), nEntryCnt );
    if( bLBEntrySelected )
        pFormImpl->GetSelectedList().Insert( nEntryCnt,
                                pFormImpl->GetSelectedList().Count() );
}

void SwHTMLParser::InsertSelectText()
{
    ASSERT( bSelect, "keine Select" );
    ASSERT( pFormImpl && pFormImpl->GetFCompPropSet().is(),
            "kein Select-Control" );

    sal_uInt16 nEntryCnt = pFormImpl->GetStringList().Count();
    if( nEntryCnt )
    {
        String& rText = *pFormImpl->GetStringList()[nEntryCnt-1];

        if( aToken.Len() && ' '==aToken.GetChar( 0 ) )
        {
            xub_StrLen nLen = rText.Len();
            if( !nLen || ' '==rText.GetChar( nLen-1 ))
                aToken.Erase( 0, 1 );
        }
        if( aToken.Len() )
            rText += aToken;
    }
}

