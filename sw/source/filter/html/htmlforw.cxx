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

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/script/XEventAttacher.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <hintids.hxx>
#include <o3tl/any.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svl/macitem.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svl/urihelper.hxx>
#include <vcl/unohelp.hxx>
#include <svx/svdouno.hxx>
#include <svx/fmglob.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <docsh.hxx>
#include <fmtanchr.hxx>
#include <docary.hxx>
#include <viewsh.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <ndtxt.hxx>
#include <flypos.hxx>
#include "wrthtml.hxx"
#include "htmlfly.hxx"
#include "htmlform.hxx"
#include <frmfmt.hxx>
#include <rtl/strbuf.hxx>
#include <memory>

using namespace ::com::sun::star;

const HtmlFrmOpts HTML_FRMOPTS_CONTROL   =
    HtmlFrmOpts::NONE;
const HtmlFrmOpts HTML_FRMOPTS_CONTROL_CSS1  =
    HtmlFrmOpts::SAlign |
    HtmlFrmOpts::SSize |
    HtmlFrmOpts::SSpace |
    HtmlFrmOpts::BrClear;
const HtmlFrmOpts HTML_FRMOPTS_IMG_CONTROL   =
    HtmlFrmOpts::Align |
    HtmlFrmOpts::BrClear;
const HtmlFrmOpts HTML_FRMOPTS_IMG_CONTROL_CSS1 =
    HtmlFrmOpts::SAlign |
    HtmlFrmOpts::SSpace;

static void lcl_html_outEvents( SvStream& rStrm,
                         const uno::Reference< form::XFormComponent >& rFormComp,
                         bool bCfgStarBasic,
                         rtl_TextEncoding eDestEnc,
                         OUString *pNonConvertableChars )
{
    uno::Reference< container::XChild > xChild( rFormComp, uno::UNO_QUERY );
    uno::Reference< uno::XInterface > xParentIfc = xChild->getParent();
    OSL_ENSURE( xParentIfc.is(), "lcl_html_outEvents: no parent interface" );
    if( !xParentIfc.is() )
        return;
    uno::Reference< container::XIndexAccess > xIndexAcc( xParentIfc, uno::UNO_QUERY );
    uno::Reference< script::XEventAttacherManager > xEventManager( xParentIfc,
                                                              uno::UNO_QUERY );
    if( !xIndexAcc.is() || !xEventManager.is() )
        return;

    // and search for the position of the ControlModel within
    sal_Int32 nCount = xIndexAcc->getCount(), nPos;
    for( nPos = 0 ; nPos < nCount; nPos++ )
    {
        uno::Any aTmp = xIndexAcc->getByIndex(nPos);
        if( auto x1 = o3tl::tryAccess<uno::Reference<form::XFormComponent>>(aTmp) )

        {
            if( rFormComp == *x1 )
                break;
        }
        else if( auto x2 = o3tl::tryAccess<uno::Reference<form::XForm>>(aTmp) )
        {
            uno::Reference< form::XFormComponent > xFC( *x2, uno::UNO_QUERY );
            if( rFormComp == xFC )
                break;
        }
        else
        {
            OSL_ENSURE( false, "lcl_html_outEvents: wrong reflection" );
        }
    }

    if( nPos == nCount )
        return;

    uno::Sequence< script::ScriptEventDescriptor > aDescs =
            xEventManager->getScriptEvents( nPos );
    nCount = aDescs.getLength();
    if( !nCount )
        return;

    const script::ScriptEventDescriptor *pDescs = aDescs.getConstArray();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        ScriptType eScriptType = EXTENDED_STYPE;
        OUString aScriptType( pDescs[i].ScriptType );
        if( aScriptType.equalsIgnoreAsciiCase(SVX_MACRO_LANGUAGE_JAVASCRIPT) )
            eScriptType = JAVASCRIPT;
        else if( aScriptType.equalsIgnoreAsciiCase(SVX_MACRO_LANGUAGE_STARBASIC ) )
            eScriptType = STARBASIC;
        if( JAVASCRIPT != eScriptType && !bCfgStarBasic )
            continue;

        OUString sListener( pDescs[i].ListenerType );
        if (!sListener.isEmpty())
        {
            const sal_Int32 nIdx { sListener.lastIndexOf('.')+1 };
            if (nIdx>0)
            {
                if (nIdx<sListener.getLength())
                {
                    sListener = sListener.copy(nIdx);
                }
                else
                {
                    sListener.clear();
                }
            }
        }
        OUString sMethod( pDescs[i].EventMethod );

        const sal_Char *pOpt = nullptr;
        for( int j=0; aEventListenerTable[j]; j++ )
        {
            if( sListener.equalsAscii( aEventListenerTable[j] ) &&
                sMethod.equalsAscii( aEventMethodTable[j] ) )
            {
                pOpt = (STARBASIC==eScriptType ? aEventSDOptionTable
                                               : aEventOptionTable)[j];
                break;
            }
        }

        OString sOut = " ";
        if( pOpt && (EXTENDED_STYPE != eScriptType ||
                     pDescs[i].AddListenerParam.isEmpty()) )
            sOut += OString(pOpt);
        else
        {
            sOut += OString(OOO_STRING_SVTOOLS_HTML_O_sdevent) +
                OUStringToOString(sListener, RTL_TEXTENCODING_ASCII_US) + "-" +
                OUStringToOString(sMethod, RTL_TEXTENCODING_ASCII_US);
        }
        sOut += "=\"";
        rStrm.WriteOString( sOut );
        HTMLOutFuncs::Out_String( rStrm, pDescs[i].ScriptCode, eDestEnc, pNonConvertableChars );
        rStrm.WriteChar( '\"' );
        if( EXTENDED_STYPE == eScriptType &&
            !pDescs[i].AddListenerParam.isEmpty() )
        {
            sOut = " " OOO_STRING_SVTOOLS_HTML_O_sdaddparam +
                OUStringToOString(sListener, RTL_TEXTENCODING_ASCII_US) + "-" +
                OUStringToOString(sMethod, RTL_TEXTENCODING_ASCII_US) + "=\"";
            rStrm.WriteOString( sOut );
            HTMLOutFuncs::Out_String( rStrm, pDescs[i].AddListenerParam,
                                      eDestEnc, pNonConvertableChars );
            rStrm.WriteChar( '\"' );
        }
    }
}

static bool lcl_html_isHTMLControl( sal_Int16 nClassId )
{
    bool bRet = false;

    switch( nClassId )
    {
    case form::FormComponentType::TEXTFIELD:
    case form::FormComponentType::COMMANDBUTTON:
    case form::FormComponentType::RADIOBUTTON:
    case form::FormComponentType::CHECKBOX:
    case form::FormComponentType::LISTBOX:
    case form::FormComponentType::IMAGEBUTTON:
    case form::FormComponentType::FILECONTROL:
        bRet = true;
        break;
    }

    return bRet;
}

bool SwHTMLWriter::HasControls() const
{
    sal_uInt32 nStartIdx = m_pCurrentPam->GetPoint()->nNode.GetIndex();
    size_t i = 0;

    // Skip all controls in front of the current paragraph
    while ( i < m_aHTMLControls.size() && m_aHTMLControls[i]->nNdIdx < nStartIdx )
        ++i;

    return i < m_aHTMLControls.size() && m_aHTMLControls[i]->nNdIdx == nStartIdx;
}

void SwHTMLWriter::OutForm( bool bTag_On, const SwStartNode *pStartNd )
{
    if( m_bPreserveForm )   // we are in a table or an area with form spanned over it
        return;

    if( !bTag_On )
    {
        // end the form when all controls are output
        if( mxFormComps.is() &&
            mxFormComps->getCount() == m_nFormCntrlCnt )
        {
            OutForm( false, mxFormComps );
            mxFormComps.clear();
        }
        return;
    }

    uno::Reference< container::XIndexContainer > xNewFormComps;
    sal_uInt32 nStartIdx = pStartNd ? pStartNd->GetIndex()
                                    : m_pCurrentPam->GetPoint()->nNode.GetIndex();

    // skip controls before the interesting area
    size_t i = 0;
    while ( i < m_aHTMLControls.size() && m_aHTMLControls[i]->nNdIdx < nStartIdx )
        ++i;

    if( !pStartNd )
    {
        // Check for a single node: there it's only interesting, if there is
        // a control for the node and to which form it belongs.
        if( i < m_aHTMLControls.size() &&
            m_aHTMLControls[i]->nNdIdx == nStartIdx )
            xNewFormComps = m_aHTMLControls[i]->xFormComps;
    }
    else
    {
        // we iterate over a table/an area: we're interested in:
        // - if there are controls with different start nodes
        // - if there is a form, with controls which aren't all in the table/area

        uno::Reference< container::XIndexContainer > xCurrentFormComps;// current form in table
        const SwStartNode *pCurrentStNd = nullptr; // and the start node of a Control
        sal_Int32 nCurrentCtrls = 0;   // and the found controls in it
        sal_uInt32 nEndIdx =  pStartNd->EndOfSectionIndex();
        for( ; i < m_aHTMLControls.size() &&
            m_aHTMLControls[i]->nNdIdx <= nEndIdx; i++ )
        {
            const SwStartNode *pCntrlStNd =
                m_pDoc->GetNodes()[m_aHTMLControls[i]->nNdIdx]->StartOfSectionNode();

            if( xCurrentFormComps.is() )
            {
                // already inside a form ...
                if( xCurrentFormComps==m_aHTMLControls[i]->xFormComps )
                {
                    // ... and the control is also inside ...
                    if( pCurrentStNd!=pCntrlStNd )
                    {
                        // ... but it's inside another cell:
                        // Then open a form above the table
                        xNewFormComps = xCurrentFormComps;
                        break;
                    }
                    nCurrentCtrls = nCurrentCtrls + m_aHTMLControls[i]->nCount;
                }
                else
                {
                    // ... but the Control is in another cell:
                    // There we act as if we open a new from and continue searching.
                    xCurrentFormComps = m_aHTMLControls[i]->xFormComps;
                    pCurrentStNd = pCntrlStNd;
                    nCurrentCtrls = m_aHTMLControls[i]->nCount;
                }
            }
            else
            {
                // We aren't in a form:
                // There we act as if we open a form.
                xCurrentFormComps = m_aHTMLControls[i]->xFormComps;
                pCurrentStNd = pCntrlStNd;
                nCurrentCtrls = m_aHTMLControls[i]->nCount;
            }
        }
        if( !xNewFormComps.is() && xCurrentFormComps.is() &&
            nCurrentCtrls != xCurrentFormComps->getCount() )
        {
            // A form should be opened in the table/area which isn't completely
            // inside the table. Then we must also now open the form.
            xNewFormComps = xCurrentFormComps;
        }
    }

    if( xNewFormComps.is() &&
        (!mxFormComps.is() || xNewFormComps != mxFormComps) )
    {
        // A form should be opened ...
        if( mxFormComps.is() )
        {
            // ... but a form is still open: That is in every case an error,
            // but we'll close the old form nevertheless.
            OutForm( false, mxFormComps );

            //!!!nWarn = 1; // Control will be assigned to wrong form
        }

        mxFormComps = xNewFormComps;

        OutForm( true, mxFormComps );
        uno::Reference< beans::XPropertySet >  xTmp;
        OutHiddenControls( mxFormComps, xTmp );
    }
}

void SwHTMLWriter::OutHiddenForms()
{
    // Without DrawModel there can't be controls. Then you also can't access the
    // document via UNO, because otherwise a DrawModel would be created.
    if( !m_pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
        return;

    SwDocShell *pDocSh = m_pDoc->GetDocShell();
    if( !pDocSh )
        return;

    uno::Reference< drawing::XDrawPageSupplier > xDPSupp( pDocSh->GetBaseModel(),
                                                     uno::UNO_QUERY );
    OSL_ENSURE( xDPSupp.is(), "XTextDocument not received from XModel" );
    uno::Reference< drawing::XDrawPage > xDrawPage = xDPSupp->getDrawPage();

    OSL_ENSURE( xDrawPage.is(), "XDrawPage not received" );
    if( !xDrawPage.is() )
        return;

    uno::Reference< form::XFormsSupplier > xFormsSupplier( xDrawPage, uno::UNO_QUERY );
    OSL_ENSURE( xFormsSupplier.is(),
            "XFormsSupplier not received from XDrawPage" );

    uno::Reference< container::XNameContainer > xTmp = xFormsSupplier->getForms();
    OSL_ENSURE( xTmp.is(), "XForms not received" );
    uno::Reference< container::XIndexContainer > xForms( xTmp, uno::UNO_QUERY );
    OSL_ENSURE( xForms.is(), "XForms without container::XIndexContainer?" );

    sal_Int32 nCount = xForms->getCount();
    for( sal_Int32 i=0; i<nCount; i++)
    {
        uno::Any aTmp = xForms->getByIndex( i );
        if( auto x = o3tl::tryAccess<uno::Reference<form::XForm>>(aTmp) )
            OutHiddenForm( *x );
        else
        {
            OSL_ENSURE( false, "OutHiddenForms: wrong reflection" );
        }
    }
}

void SwHTMLWriter::OutHiddenForm( const uno::Reference< form::XForm > & rForm )
{
    uno::Reference< container::XIndexContainer > xFormComps( rForm, uno::UNO_QUERY );
    if( !xFormComps.is() )
        return;

    sal_Int32 nCount = xFormComps->getCount();
    bool bHiddenOnly = nCount > 0, bHidden = false;
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        uno::Any aTmp = xFormComps->getByIndex( i );
        auto xFormComp = o3tl::tryAccess<uno::Reference<form::XFormComponent>>(
            aTmp);
        OSL_ENSURE( xFormComp, "OutHiddenForm: wrong reflection" );
        if( !xFormComp )
            continue;

        uno::Reference< form::XForm > xForm( *xFormComp, uno::UNO_QUERY );
        if( xForm.is() )
            OutHiddenForm( xForm );

        if( bHiddenOnly )
        {
            uno::Reference< beans::XPropertySet >  xPropSet( *xFormComp, uno::UNO_QUERY );
            OUString sPropName("ClassId");
            if( xPropSet->getPropertySetInfo()->hasPropertyByName( sPropName ) )
            {
                uno::Any aAny2 = xPropSet->getPropertyValue( sPropName );
                if( auto n = o3tl::tryAccess<sal_Int16>(aAny2) )
                {
                    if( form::FormComponentType::HIDDENCONTROL == *n )
                        bHidden = true;
                    else if( lcl_html_isHTMLControl( *n ) )
                        bHiddenOnly = false;
                }
            }
        }
    }

    if( bHidden && bHiddenOnly )
    {
        OutForm( true, xFormComps );
        uno::Reference< beans::XPropertySet > xTmp;
        OutHiddenControls( xFormComps, xTmp );
        OutForm( false, xFormComps );
    }
}

void SwHTMLWriter::OutForm( bool bOn,
                const uno::Reference< container::XIndexContainer > & rFormComps )
{
    m_nFormCntrlCnt = 0;

    if( !bOn )
    {
        DecIndentLevel(); // indent content of form
        if( m_bLFPossible )
            OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_form, false );
        m_bLFPossible = true;

        return;
    }

    // the new form is opened
    if( m_bLFPossible )
        OutNewLine();
    OString sOut = "<" OOO_STRING_SVTOOLS_HTML_form;

    uno::Reference< beans::XPropertySet > xFormPropSet( rFormComps, uno::UNO_QUERY );

    uno::Any aTmp = xFormPropSet->getPropertyValue( "Name" );
    if( auto s = o3tl::tryAccess<OUString>(aTmp) )
    {
        if( !s->isEmpty() )
        {
            sOut += " " OOO_STRING_SVTOOLS_HTML_O_name "=\"";
            Strm().WriteOString( sOut );
            HTMLOutFuncs::Out_String( Strm(), *s,
                                      m_eDestEnc, &m_aNonConvertableCharacters );
            sOut = "\"";
        }
    }

    aTmp = xFormPropSet->getPropertyValue( "TargetURL" );
    if( auto s = o3tl::tryAccess<OUString>(aTmp) )
    {
        if ( !s->isEmpty() )
        {
            sOut += " " OOO_STRING_SVTOOLS_HTML_O_action "=\"";
            Strm().WriteOString( sOut );
            OUString aURL
                = URIHelper::simpleNormalizedMakeRelative( GetBaseURL(), *s);
            HTMLOutFuncs::Out_String( Strm(), aURL, m_eDestEnc, &m_aNonConvertableCharacters );
            sOut = "\"";
        }
    }

    aTmp = xFormPropSet->getPropertyValue( "SubmitMethod" );
    if( auto eMethod = o3tl::tryAccess<form::FormSubmitMethod>(aTmp) )
    {
        if( form::FormSubmitMethod_POST==*eMethod )
        {
            sOut += " " OOO_STRING_SVTOOLS_HTML_O_method "=\""
                OOO_STRING_SVTOOLS_HTML_METHOD_post "\"";
        }
    }
    aTmp = xFormPropSet->getPropertyValue( "SubmitEncoding" );
    if( auto eEncType = o3tl::tryAccess<form::FormSubmitEncoding>(aTmp) )
    {
        const sal_Char *pStr = nullptr;
        switch( *eEncType )
        {
        case form::FormSubmitEncoding_MULTIPART:
            pStr = OOO_STRING_SVTOOLS_HTML_ET_multipart;
            break;
        case form::FormSubmitEncoding_TEXT:
            pStr = OOO_STRING_SVTOOLS_HTML_ET_text;
            break;
        default:
            ;
        }

        if( pStr )
        {
            sOut += " " OOO_STRING_SVTOOLS_HTML_O_enctype "=\"" +
                OString(pStr) + "\"";
        }
    }

    aTmp = xFormPropSet->getPropertyValue( "TargetFrame" );
    if( auto s = o3tl::tryAccess<OUString>(aTmp) )
    {
        if (!s->isEmpty() )
        {
            sOut += " " OOO_STRING_SVTOOLS_HTML_O_target "=\"";
            Strm().WriteOString( sOut );
            HTMLOutFuncs::Out_String( Strm(), *s,
                                      m_eDestEnc, &m_aNonConvertableCharacters );
            sOut = "\"";
        }
    }

    Strm().WriteOString( sOut );
    uno::Reference< form::XFormComponent > xFormComp( rFormComps, uno::UNO_QUERY );
    lcl_html_outEvents( Strm(), xFormComp, m_bCfgStarBasic, m_eDestEnc, &m_aNonConvertableCharacters );
    Strm().WriteChar( '>' );

    IncIndentLevel(); // indent content of form
    m_bLFPossible = true;
}

void SwHTMLWriter::OutHiddenControls(
        const uno::Reference< container::XIndexContainer > & rFormComps,
        const uno::Reference< beans::XPropertySet > & rPropSet )
{
    sal_Int32 nCount = rFormComps->getCount();
    sal_Int32 nPos = 0;
    if( rPropSet.is() )
    {
        bool bDone = false;

        uno::Reference< form::XFormComponent > xFC( rPropSet, uno::UNO_QUERY );
        for( nPos=0; !bDone && nPos < nCount; nPos++ )
        {
            uno::Any aTmp = rFormComps->getByIndex( nPos );
            auto x = o3tl::tryAccess<uno::Reference<form::XFormComponent>>(aTmp);
            OSL_ENSURE( x,
                    "OutHiddenControls: wrong reflection" );
            bDone = x && *x == xFC;
        }
    }

    for( ; nPos < nCount; nPos++ )
    {
        uno::Any aTmp = rFormComps->getByIndex( nPos );
        auto xFC = o3tl::tryAccess<uno::Reference<form::XFormComponent>>(aTmp);
        OSL_ENSURE( xFC,
                "OutHiddenControls: wrong reflection" );
        if( !xFC )
            continue;
        uno::Reference< beans::XPropertySet > xPropSet( *xFC, uno::UNO_QUERY );

        OUString sPropName = "ClassId";
        if( !xPropSet->getPropertySetInfo()->hasPropertyByName( sPropName ) )
            continue;

        aTmp = xPropSet->getPropertyValue( sPropName );
        auto n = o3tl::tryAccess<sal_Int16>(aTmp);
        if( !n )
            continue;

        if( form::FormComponentType::HIDDENCONTROL == *n )
        {
            if( m_bLFPossible )
                OutNewLine( true );
            OString sOut = "<" OOO_STRING_SVTOOLS_HTML_input " "
                OOO_STRING_SVTOOLS_HTML_O_type "=\""
                OOO_STRING_SVTOOLS_HTML_IT_hidden "\"";

            aTmp = xPropSet->getPropertyValue( "Name" );
            if( auto s = o3tl::tryAccess<OUString>(aTmp) )
            {
                if( !s->isEmpty() )
                {
                    sOut += " " OOO_STRING_SVTOOLS_HTML_O_name "=\"";
                    Strm().WriteOString( sOut );
                    HTMLOutFuncs::Out_String( Strm(), *s,
                                              m_eDestEnc, &m_aNonConvertableCharacters );
                    sOut = "\"";
                }
            }
            aTmp = xPropSet->getPropertyValue( "HiddenValue" );
            if( auto s = o3tl::tryAccess<OUString>(aTmp) )
            {
                if( !s->isEmpty() )
                {
                    sOut += " " OOO_STRING_SVTOOLS_HTML_O_value "=\"";
                    Strm().WriteOString( sOut );
                    HTMLOutFuncs::Out_String( Strm(), *s,
                                              m_eDestEnc, &m_aNonConvertableCharacters );
                    sOut = "\"";
                }
            }
            sOut += ">";
            Strm().WriteOString( sOut );

            m_nFormCntrlCnt++;
        }
        else if( lcl_html_isHTMLControl( *n ) )
        {
            break;
        }
    }
}

// here are the output routines, thus the form::Forms are bundled:

const SdrObject *SwHTMLWriter::GetHTMLControl( const SwDrawFrameFormat& rFormat )
{
    // it must be a Draw-Format
    OSL_ENSURE( RES_DRAWFRMFMT == rFormat.Which(),
            "GetHTMLControl only allow for Draw-Formats" );

    // Look if a SdrObject exists for it
    const SdrObject *pObj = rFormat.FindSdrObject();
    if( !pObj || SdrInventor::FmForm != pObj->GetObjInventor() )
        return nullptr;

    const SdrUnoObj& rFormObj = dynamic_cast<const SdrUnoObj&>(*pObj);
    const uno::Reference< awt::XControlModel >&  xControlModel =
            rFormObj.GetUnoControlModel();

    OSL_ENSURE( xControlModel.is(), "UNO-Control without model" );
    if( !xControlModel.is() )
        return nullptr;

    uno::Reference< beans::XPropertySet >  xPropSet( xControlModel, uno::UNO_QUERY );

    OUString sPropName("ClassId");
    if( !xPropSet->getPropertySetInfo()->hasPropertyByName( sPropName ) )
        return nullptr;

    uno::Any aTmp = xPropSet->getPropertyValue( sPropName );
    if( auto n = o3tl::tryAccess<sal_Int16>(aTmp) )
    {
        if( lcl_html_isHTMLControl( *n ) )
        {
            return pObj;
        }
    }

    return nullptr;
}

static void GetControlSize(const SdrUnoObj& rFormObj, Size& rSz, SwDoc *pDoc)
{
    SwViewShell *pVSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    if( !pVSh )
        return;

    uno::Reference< awt::XControl >  xControl;
    SdrView* pDrawView = pVSh->GetDrawView();
    OSL_ENSURE( pDrawView && pVSh->GetWin(), "no DrawView or window!" );
    if ( pDrawView && pVSh->GetWin() )
        xControl = rFormObj.GetUnoControl( *pDrawView, *pVSh->GetWin() );
    uno::Reference< awt::XTextLayoutConstrains > xLC( xControl, uno::UNO_QUERY );
    OSL_ENSURE( xLC.is(), "no XTextLayoutConstrains" );
    if( !xLC.is() )
        return;

    sal_Int16 nCols=0, nLines=0;
    xLC->getColumnsAndLines( nCols, nLines );
    rSz.setWidth( nCols );
    rSz.setHeight( nLines );
}

Writer& OutHTML_DrawFrameFormatAsControl( Writer& rWrt,
                                     const SwDrawFrameFormat& rFormat,
                                     const SdrUnoObj& rFormObj,
                                     bool bInCntnr )
{
    const uno::Reference< awt::XControlModel >& xControlModel =
        rFormObj.GetUnoControlModel();

    OSL_ENSURE( xControlModel.is(), "UNO-Control without model" );
    if( !xControlModel.is() )
        return rWrt;

    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();

    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    rHTMLWrt.m_nFormCntrlCnt++;

    enum Tag { TAG_INPUT, TAG_SELECT, TAG_TEXTAREA, TAG_NONE };
    static char const * const TagNames[] = {
        OOO_STRING_SVTOOLS_HTML_input, OOO_STRING_SVTOOLS_HTML_select,
        OOO_STRING_SVTOOLS_HTML_textarea };
    Tag eTag = TAG_INPUT;
    enum Type {
        TYPE_TEXT, TYPE_PASSWORD, TYPE_CHECKBOX, TYPE_RADIO, TYPE_FILE,
        TYPE_SUBMIT, TYPE_IMAGE, TYPE_RESET, TYPE_BUTTON, TYPE_NONE };
    static char const * const TypeNames[] = {
        OOO_STRING_SVTOOLS_HTML_IT_text, OOO_STRING_SVTOOLS_HTML_IT_password,
        OOO_STRING_SVTOOLS_HTML_IT_checkbox, OOO_STRING_SVTOOLS_HTML_IT_radio,
        OOO_STRING_SVTOOLS_HTML_IT_file, OOO_STRING_SVTOOLS_HTML_IT_submit,
        OOO_STRING_SVTOOLS_HTML_IT_image, OOO_STRING_SVTOOLS_HTML_IT_reset,
        OOO_STRING_SVTOOLS_HTML_IT_button };
    Type eType = TYPE_NONE;
    OUString sValue;
    OString sOptions;
    bool bEmptyValue = false;
    uno::Any aTmp = xPropSet->getPropertyValue( "ClassId" );
    sal_Int16 nClassId = *o3tl::doAccess<sal_Int16>(aTmp);
    HtmlFrmOpts nFrameOpts = HTML_FRMOPTS_CONTROL;
    switch( nClassId )
    {
    case form::FormComponentType::CHECKBOX:
    case form::FormComponentType::RADIOBUTTON:
        eType = (form::FormComponentType::CHECKBOX == nClassId
                    ? TYPE_CHECKBOX : TYPE_RADIO);
        aTmp = xPropSet->getPropertyValue( "DefaultState" );
        if( auto n = o3tl::tryAccess<sal_Int16>(aTmp) )
        {
            if ( TRISTATE_FALSE != *n )
            {
                sOptions += " " OOO_STRING_SVTOOLS_HTML_O_checked;
                sOptions += "=\"";
                sOptions += OString(OOO_STRING_SVTOOLS_HTML_O_checked);
                sOptions += "\"";
            }
        }

        aTmp = xPropSet->getPropertyValue( "RefValue" );
        if( auto rVal = o3tl::tryAccess<OUString>(aTmp) )

        {
            if( rVal->isEmpty() )
                bEmptyValue = true;
            else if( *rVal != OOO_STRING_SVTOOLS_HTML_on )
                sValue = *rVal;
        }
        break;

    case form::FormComponentType::COMMANDBUTTON:
        {
            form::FormButtonType eButtonType = form::FormButtonType_PUSH;
            aTmp = xPropSet->getPropertyValue( "ButtonType" );
            if( auto t = o3tl::tryAccess<form::FormButtonType>(aTmp) )
                eButtonType = *t;

            switch( eButtonType )
            {
            case form::FormButtonType_RESET:
                eType = TYPE_RESET;
                break;
            case form::FormButtonType_SUBMIT:
                eType = TYPE_SUBMIT;
                break;
            case form::FormButtonType_PUSH:
            default:
                eType = TYPE_BUTTON;
            }

            aTmp = xPropSet->getPropertyValue( "Label" );
            if( auto s = o3tl::tryAccess<OUString>(aTmp) )
            {
                if( !s->isEmpty() )
                {
                    sValue = *s;
                }
            }
        }
        break;

    case form::FormComponentType::LISTBOX:
        if( rHTMLWrt.m_bLFPossible )
            rHTMLWrt.OutNewLine( true );
        eTag = TAG_SELECT;
        aTmp = xPropSet->getPropertyValue( "Dropdown" );
        if( auto b1 = o3tl::tryAccess<bool>(aTmp) )
        {
            if( !*b1 )
            {
                Size aSz( 0, 0 );
                GetControlSize( rFormObj, aSz, rWrt.m_pDoc );

                // How many are visible ??
                if( aSz.Height() )
                {
                    sOptions += " " OOO_STRING_SVTOOLS_HTML_O_size "=\"" +
                        OString::number(static_cast<sal_Int32>(aSz.Height())) + "\"";
                }

                auto aTmp2 = xPropSet->getPropertyValue( "MultiSelection" );
                if( auto b2 = o3tl::tryAccess<bool>(aTmp2) )
                {
                    if ( *b2 )
                    {
                        sOptions += " " OOO_STRING_SVTOOLS_HTML_O_multiple;
                    }
                }
            }
        }
        break;

    case form::FormComponentType::TEXTFIELD:
        {
            Size aSz( 0, 0 );
            GetControlSize( rFormObj, aSz, rWrt.m_pDoc );

            bool bMultiLine = false;
            OUString sMultiLine("MultiLine");
            if( xPropSetInfo->hasPropertyByName( sMultiLine ) )
            {
                aTmp = xPropSet->getPropertyValue( sMultiLine );
                auto b = o3tl::tryAccess<bool>(aTmp);
                bMultiLine = b && *b;
            }

            if( bMultiLine )
            {
                if( rHTMLWrt.m_bLFPossible )
                    rHTMLWrt.OutNewLine( true );
                eTag = TAG_TEXTAREA;

                if( aSz.Height() )
                {
                    sOptions += " " OOO_STRING_SVTOOLS_HTML_O_rows "=\"" +
                        OString::number(static_cast<sal_Int32>(aSz.Height())) + "\"";
                }
                if( aSz.Width() )
                {
                    sOptions += " " OOO_STRING_SVTOOLS_HTML_O_cols "=\"" +
                        OString::number(static_cast<sal_Int32>(aSz.Width())) + "\"";
                }

                aTmp = xPropSet->getPropertyValue( "HScroll" );
                if( aTmp.getValueType() == cppu::UnoType<void>::get() ||
                    (aTmp.getValueType() == cppu::UnoType<bool>::get() &&
                    !*o3tl::forceAccess<bool>(aTmp)) )
                {
                    const sal_Char *pWrapStr = nullptr;
                    auto aTmp2 = xPropSet->getPropertyValue( "HardLineBreaks" );
                    auto b = o3tl::tryAccess<bool>(aTmp2);
                    pWrapStr = (b && *b) ? OOO_STRING_SVTOOLS_HTML_WW_hard
                                         : OOO_STRING_SVTOOLS_HTML_WW_soft;
                    sOptions += " " OOO_STRING_SVTOOLS_HTML_O_wrap "=\"" +
                        OString(pWrapStr) + "\"";
                }
            }
            else
            {
                eType = TYPE_TEXT;
                OUString sEchoChar("EchoChar");
                if( xPropSetInfo->hasPropertyByName( sEchoChar ) )
                {
                    aTmp = xPropSet->getPropertyValue( sEchoChar );
                    if( auto n = o3tl::tryAccess<sal_Int16>(aTmp) )
                    {
                        if( *n != 0 )
                            eType = TYPE_PASSWORD;
                    }
                }

                if( aSz.Width() )
                {
                    sOptions += " " OOO_STRING_SVTOOLS_HTML_O_size "=\"" +
                        OString::number(static_cast<sal_Int32>(aSz.Width())) + "\"";
                }

                aTmp = xPropSet->getPropertyValue( "MaxTextLen" );
                if( auto n = o3tl::tryAccess<sal_Int16>(aTmp) )
                {
                    if( *n != 0 )
                    {
                        sOptions += " " OOO_STRING_SVTOOLS_HTML_O_maxlength "=\"" +
                            OString::number(static_cast<sal_Int32>(*n)) + "\"";
                    }
                }

                if( xPropSetInfo->hasPropertyByName( "DefaultText" ) )
                {
                    aTmp = xPropSet->getPropertyValue( "DefaultText" );
                    if( auto s = o3tl::tryAccess<OUString>(aTmp) )
                    {
                        if( !s->isEmpty() )
                        {
                            sValue = *s;
                        }
                    }
                }
            }
        }
        break;

    case form::FormComponentType::FILECONTROL:
        {
            Size aSz( 0, 0 );
            GetControlSize( rFormObj, aSz, rWrt.m_pDoc );
            eType = TYPE_FILE;

            if( aSz.Width() )
            {
                sOptions += " " OOO_STRING_SVTOOLS_HTML_O_size "=\"" +
                    OString::number(static_cast<sal_Int32>(aSz.Width())) + "\"";
            }

            // VALUE vim form: don't export because of security reasons
        }
        break;

    case form::FormComponentType::IMAGEBUTTON:
        eType = TYPE_IMAGE;
        nFrameOpts = HTML_FRMOPTS_IMG_CONTROL;
        break;

    default:                // doesn't know HTML
        eTag = TAG_NONE;    // therefore skip it
        break;
    }

    if( eTag == TAG_NONE )
        return rWrt;

    OString sOut = "<" + OString(TagNames[eTag]);
    if( eType != TYPE_NONE )
    {
        sOut += " " OOO_STRING_SVTOOLS_HTML_O_type "=\"" +
            OString(TypeNames[eType]) + "\"";
    }

    aTmp = xPropSet->getPropertyValue("Name");
    if( auto s = o3tl::tryAccess<OUString>(aTmp) )
    {
        if( !s->isEmpty() )
        {
            sOut += " " OOO_STRING_SVTOOLS_HTML_O_name "=\"";
            rWrt.Strm().WriteOString( sOut );
            HTMLOutFuncs::Out_String( rWrt.Strm(), *s,
                                      rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            sOut = "\"";
        }
    }

    aTmp = xPropSet->getPropertyValue("Enabled");
    if( auto b = o3tl::tryAccess<bool>(aTmp) )
    {
        if( !*b )
        {
            sOut += " " OOO_STRING_SVTOOLS_HTML_O_disabled;
        }
    }

    if( !sValue.isEmpty() || bEmptyValue )
    {
        sOut += " " OOO_STRING_SVTOOLS_HTML_O_value "=\"";
        rWrt.Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_String( rWrt.Strm(), sValue, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
        sOut = "\"";
    }

    sOut += " " + sOptions;

    if( TYPE_IMAGE == eType )
    {
        aTmp = xPropSet->getPropertyValue( "ImageURL" );
        if( auto s = o3tl::tryAccess<OUString>(aTmp) )
        {
            if( !s->isEmpty() )
            {
                sOut += " " OOO_STRING_SVTOOLS_HTML_O_src "=\"";
                rWrt.Strm().WriteOString( sOut );

                HTMLOutFuncs::Out_String( rWrt.Strm(),
                            URIHelper::simpleNormalizedMakeRelative( rWrt.GetBaseURL(), *s),
                            rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
                sOut = "\"";
            }
        }

        Size aTwipSz( rFormObj.GetLogicRect().GetSize() );
        Size aPixelSz( 0, 0 );
        if( (aTwipSz.Width() || aTwipSz.Height()) &&
            Application::GetDefaultDevice() )
        {
            aPixelSz =
                Application::GetDefaultDevice()->LogicToPixel( aTwipSz,
                                                    MapMode(MapUnit::MapTwip) );
            if( !aPixelSz.Width() && aTwipSz.Width() )
                aPixelSz.setWidth( 1 );
            if( !aPixelSz.Height() && aTwipSz.Height() )
                aPixelSz.setHeight( 1 );
        }

        if( aPixelSz.Width() )
        {
            sOut += " " OOO_STRING_SVTOOLS_HTML_O_width "=\"" +
                OString::number(static_cast<sal_Int32>(aPixelSz.Width())) + "\"";
        }

        if( aPixelSz.Height() )
        {
            sOut += " " OOO_STRING_SVTOOLS_HTML_O_height "=\"" +
                OString::number(static_cast<sal_Int32>(aPixelSz.Height())) + "\"";
        }
    }

    aTmp = xPropSet->getPropertyValue( "TabIndex" );
    if( auto n = o3tl::tryAccess<sal_Int16>(aTmp) )
    {
        sal_Int16 nTabIndex = *n;
        if( nTabIndex > 0 )
        {
            if( nTabIndex >= 32767 )
                nTabIndex = 32767;

            sOut += " " OOO_STRING_SVTOOLS_HTML_O_tabindex "=\"" +
                OString::number(static_cast<sal_Int32>(nTabIndex)) + "\"";
        }
    }

    if( !sOut.isEmpty() )
        rWrt.Strm().WriteOString( sOut );

    OSL_ENSURE( !bInCntnr, "Container is not supported for Controls" );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_DRAW ) && !bInCntnr )
    {
        // If Character-Objects can't be positioned absolutely,
        // then delete the corresponding flag.
        nFrameOpts |= (TYPE_IMAGE == eType
                            ? HTML_FRMOPTS_IMG_CONTROL_CSS1
                            : HTML_FRMOPTS_CONTROL_CSS1);
    }
    OString aEndTags;
    if( nFrameOpts != HtmlFrmOpts::NONE )
        aEndTags = rHTMLWrt.OutFrameFormatOptions(rFormat, OUString(), nFrameOpts);

    if( rHTMLWrt.m_bCfgOutStyles )
    {
        bool bEdit = TAG_TEXTAREA == eTag || TYPE_FILE == eType ||
                     TYPE_TEXT == eType;

        SfxItemSet aItemSet( rHTMLWrt.m_pDoc->GetAttrPool(), svl::Items<RES_CHRATR_BEGIN,
                             RES_CHRATR_END>{} );
        if( xPropSetInfo->hasPropertyByName( "BackgroundColor" ) )
        {
            aTmp = xPropSet->getPropertyValue( "BackgroundColor" );
            if( auto n = o3tl::tryAccess<sal_Int32>(aTmp) )
            {
                Color aCol(*n);
                aItemSet.Put( SvxBrushItem( aCol, RES_CHRATR_BACKGROUND ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "TextColor" ) )
        {
            aTmp = xPropSet->getPropertyValue( "TextColor" );
            if( auto n = o3tl::tryAccess<sal_Int32>(aTmp) )
            {
                Color aColor( *n );
                aItemSet.Put( SvxColorItem( aColor, RES_CHRATR_COLOR ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontHeight" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontHeight" );
            if( auto nHeight = o3tl::tryAccess<float>(aTmp) )

            {
                if( *nHeight > 0  && (!bEdit || !rtl::math::approxEqual(*nHeight, 10.0)) )
                    aItemSet.Put( SvxFontHeightItem( sal_Int16(*nHeight * 20.), 100, RES_CHRATR_FONTSIZE ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontName" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontName" );
            if( auto aFName = o3tl::tryAccess<OUString>(aTmp) )
            {
                if( !aFName->isEmpty() )
                {
                    vcl::Font aFixedFont( OutputDevice::GetDefaultFont(
                                        DefaultFontType::FIXED, LANGUAGE_ENGLISH_US,
                                        GetDefaultFontFlags::OnlyOne ) );
                    if( !bEdit || *aFName != aFixedFont.GetFamilyName() )
                    {
                        FontFamily eFamily = FAMILY_DONTKNOW;
                        if( xPropSetInfo->hasPropertyByName( "FontFamily" ) )
                        {
                            auto aTmp2 = xPropSet->getPropertyValue( "FontFamily" );
                            if( auto n = o3tl::tryAccess<sal_Int16>(aTmp2) )
                                eFamily = static_cast<FontFamily>(*n);
                        }
                        SvxFontItem aItem(eFamily, *aFName, OUString(), PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, RES_CHRATR_FONT);
                        aItemSet.Put( aItem );
                    }
                }
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontWeight" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontWeight" );
            if( auto x = o3tl::tryAccess<float>(aTmp) )
            {
                FontWeight eWeight =
                    vcl::unohelper::ConvertFontWeight( *x );
                if( eWeight != WEIGHT_DONTKNOW && eWeight != WEIGHT_NORMAL )
                    aItemSet.Put( SvxWeightItem( eWeight, RES_CHRATR_WEIGHT ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontSlant" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontSlant" );
            if( auto n = o3tl::tryAccess<sal_Int16>(aTmp) )
            {
                FontItalic eItalic = static_cast<FontItalic>(*n);
                if( eItalic != ITALIC_DONTKNOW && eItalic != ITALIC_NONE )
                    aItemSet.Put( SvxPostureItem( eItalic, RES_CHRATR_POSTURE ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontLineStyle" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontLineStyle" );
            if( auto n = o3tl::tryAccess<sal_Int16>(aTmp) )
            {
                FontLineStyle eUnderline = static_cast<FontLineStyle>(*n);
                if( eUnderline != LINESTYLE_DONTKNOW  &&
                    eUnderline != LINESTYLE_NONE )
                    aItemSet.Put( SvxUnderlineItem( eUnderline, RES_CHRATR_UNDERLINE ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontStrikeout" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontStrikeout" );
            if( auto n = o3tl::tryAccess<sal_Int16>(aTmp) )
            {
                FontStrikeout eStrikeout = static_cast<FontStrikeout>(*n);
                if( eStrikeout != STRIKEOUT_DONTKNOW &&
                    eStrikeout != STRIKEOUT_NONE )
                    aItemSet.Put( SvxCrossedOutItem( eStrikeout, RES_CHRATR_CROSSEDOUT ) );
            }
        }

        rHTMLWrt.OutCSS1_FrameFormatOptions( rFormat, nFrameOpts, &rFormObj,
                                        &aItemSet );
    }

    uno::Reference< form::XFormComponent >  xFormComp( xControlModel, uno::UNO_QUERY );
    lcl_html_outEvents( rWrt.Strm(), xFormComp, rHTMLWrt.m_bCfgStarBasic,
                        rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );

    rWrt.Strm().WriteChar( '>' );

    if( TAG_SELECT == eTag )
    {
        aTmp = xPropSet->getPropertyValue( "StringItemList" );
        if( auto aList = o3tl::tryAccess<uno::Sequence<OUString>>(aTmp) )
        {
            rHTMLWrt.IncIndentLevel(); // the content of Select can be indented
            sal_Int32 nCnt = aList->getLength();
            const OUString *pStrings = aList->getConstArray();

            const OUString *pValues = nullptr;
            sal_Int32 nValCnt = 0;
            auto aTmp2 = xPropSet->getPropertyValue( "ListSource" );
            uno::Sequence<OUString> aValList;
            if( auto s = o3tl::tryAccess<uno::Sequence<OUString>>(aTmp2) )
            {
                aValList = *s;
                nValCnt = aValList.getLength();
                pValues = aValList.getConstArray();
            }

            uno::Any aSelTmp = xPropSet->getPropertyValue( "DefaultSelection" );
            const sal_Int16 *pSels = nullptr;
            sal_Int32 nSel = 0;
            sal_Int32 nSelCnt = 0;
            uno::Sequence<sal_Int16> aSelList;
            if( auto s = o3tl::tryAccess<uno::Sequence<sal_Int16>>(aSelTmp) )
            {
                aSelList = *s;
                nSelCnt = aSelList.getLength();
                pSels = aSelList.getConstArray();
            }

            for( sal_Int32 i = 0; i < nCnt; i++ )
            {
                OUString sVal;
                bool bSelected = false, bEmptyVal = false;
                if( i < nValCnt )
                {
                    const OUString& rVal = pValues[i];
                    if( rVal == "$$$empty$$$" )
                        bEmptyVal = true;
                    else
                        sVal = rVal;
                }

                bSelected = (nSel < nSelCnt) && pSels[nSel] == i;
                if( bSelected )
                    nSel++;

                rHTMLWrt.OutNewLine(); // every Option gets its own line
                sOut = "<" OOO_STRING_SVTOOLS_HTML_option;
                if( !sVal.isEmpty() || bEmptyVal )
                {
                    sOut += " " OOO_STRING_SVTOOLS_HTML_O_value "=\"";
                    rWrt.Strm().WriteOString( sOut );
                    HTMLOutFuncs::Out_String( rWrt.Strm(), sVal,
                        rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
                    sOut = "\"";
                }
                if( bSelected )
                    sOut += " " OOO_STRING_SVTOOLS_HTML_O_selected;

                sOut += ">";
                rWrt.Strm().WriteOString( sOut );

                HTMLOutFuncs::Out_String( rWrt.Strm(), pStrings[i],
                                          rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            }
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_option, false );

            rHTMLWrt.DecIndentLevel();
            rHTMLWrt.OutNewLine();// the </SELECT> gets its own line
        }
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_select, false );
    }
    else if( TAG_TEXTAREA == eTag )
    {
        // In TextAreas no additional spaces or LF may be exported!
        OUString sVal;
        aTmp = xPropSet->getPropertyValue( "DefaultText" );
        if( auto s = o3tl::tryAccess<OUString>(aTmp) )
        {
            if( !s->isEmpty() )
            {
                sVal = *s;
            }
        }
        if( !sVal.isEmpty() )
        {
            sVal = convertLineEnd(sVal, LINEEND_LF);
            sal_Int32 nPos = 0;
            while ( nPos != -1 )
            {
                if( nPos )
                    rWrt.Strm().WriteCharPtr( SAL_NEWLINE_STRING );
                OUString aLine = sVal.getToken( 0, 0x0A, nPos );
                HTMLOutFuncs::Out_String( rWrt.Strm(), aLine,
                                        rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            }
        }
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_textarea, false );
    }
    else if( TYPE_CHECKBOX == eType || TYPE_RADIO == eType )
    {
        aTmp = xPropSet->getPropertyValue("Label");
        if( auto s = o3tl::tryAccess<OUString>(aTmp) )
        {
            if( !s->isEmpty() )
            {
                HTMLOutFuncs::Out_String( rWrt.Strm(), *s,
                    rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters ).WriteChar( ' ' );
            }
        }
    }

    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteCharPtr( aEndTags.getStr() );

    // Controls aren't bound to a paragraph, therefore don't output LF anymore!
    rHTMLWrt.m_bLFPossible = false;

    if( rHTMLWrt.mxFormComps.is() )
        rHTMLWrt.OutHiddenControls( rHTMLWrt.mxFormComps, xPropSet );
    return rWrt;
}

/**
 * Find out if a format belongs to a control and if yes return its form.
 */
static void AddControl( HTMLControls& rControls,
                        const SdrUnoObj& rFormObj,
                        sal_uInt32 nNodeIdx )
{
    const uno::Reference< awt::XControlModel >& xControlModel =
            rFormObj.GetUnoControlModel();
    if( !xControlModel.is() )
        return;

    uno::Reference< form::XFormComponent >  xFormComp( xControlModel, uno::UNO_QUERY );
    uno::Reference< uno::XInterface >  xIfc = xFormComp->getParent();
    uno::Reference< form::XForm >  xForm(xIfc, uno::UNO_QUERY);

    OSL_ENSURE( xForm.is(), "Where is the form?" );
    if( xForm.is() )
    {
        uno::Reference< container::XIndexContainer >  xFormComps( xForm, uno::UNO_QUERY );
        std::unique_ptr<HTMLControl> pHCntrl(new HTMLControl( xFormComps, nNodeIdx ));
        HTMLControls::const_iterator it = rControls.find( pHCntrl.get() );
        if( it == rControls.end() )
            rControls.insert( std::move(pHCntrl) );
        else
        {
            if( (*it)->xFormComps==xFormComps )
                (*it)->nCount++;
        }
    }
}

void SwHTMLWriter::GetControls()
{
    // Idea: first off collect the paragraph- and character-bound controls.
    // In the process for every control the paragraph position and VCForm are
    // saved in an array.
    // With that array it's possible to find out where form::Forms must be
    // opened and closed.

    if( m_pHTMLPosFlyFrames )
    {
        // collect the paragraph-bound controls
        for( size_t i=0; i<m_pHTMLPosFlyFrames->size(); i++ )
        {
            const SwHTMLPosFlyFrame* pPosFlyFrame = (*m_pHTMLPosFlyFrames)[ i ].get();
            if( HtmlOut::Control != pPosFlyFrame->GetOutFn() )
                continue;

            const SdrObject *pSdrObj = pPosFlyFrame->GetSdrObject();
            OSL_ENSURE( pSdrObj, "Where is the SdrObject?" );
            if( !pSdrObj )
                continue;

            AddControl( m_aHTMLControls, dynamic_cast<const SdrUnoObj&>(*pSdrObj),
                        pPosFlyFrame->GetNdIndex().GetIndex() );
        }
    }

    // and now the ones in a character-bound frame
    const SwFrameFormats* pSpzFrameFormats = m_pDoc->GetSpzFrameFormats();
    for( size_t i=0; i<pSpzFrameFormats->size(); i++ )
    {
        const SwFrameFormat *pFrameFormat = (*pSpzFrameFormats)[i];
        if( RES_DRAWFRMFMT != pFrameFormat->Which() )
            continue;

        const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
        const SwPosition *pPos = rAnchor.GetContentAnchor();
        if ((RndStdIds::FLY_AS_CHAR != rAnchor.GetAnchorId()) || !pPos)
            continue;

        const SdrObject *pSdrObj =
            SwHTMLWriter::GetHTMLControl( *static_cast<const SwDrawFrameFormat*>(pFrameFormat) );
        if( !pSdrObj )
            continue;

        AddControl( m_aHTMLControls, dynamic_cast<const SdrUnoObj&>(*pSdrObj), pPos->nNode.GetIndex() );
    }
}

HTMLControl::HTMLControl(
        const uno::Reference< container::XIndexContainer > & rFormComps,
        sal_uInt32 nIdx ) :
    xFormComps( rFormComps ), nNdIdx( nIdx ), nCount( 1 )
{}

HTMLControl::~HTMLControl()
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
