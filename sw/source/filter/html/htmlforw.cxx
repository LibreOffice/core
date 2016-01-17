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
#include <comphelper/string.hxx>
#include <hintids.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svl/macitem.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svl/urihelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
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
#include "pam.hxx"
#include "doc.hxx"
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include "ndtxt.hxx"
#include "flypos.hxx"
#include "wrthtml.hxx"
#include "htmlfly.hxx"
#include "htmlform.hxx"
#include "frmfmt.hxx"
#include <rtl/strbuf.hxx>
#include <memory>

using namespace ::com::sun::star;

const sal_uInt32 HTML_FRMOPTS_CONTROL   =
    0;
const sal_uInt32 HTML_FRMOPTS_CONTROL_CSS1  =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SIZE |
    HTML_FRMOPT_S_SPACE |
    HTML_FRMOPT_BRCLEAR;
const sal_uInt32 HTML_FRMOPTS_IMG_CONTROL   =
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_BRCLEAR;
const sal_uInt32 HTML_FRMOPTS_IMG_CONTROL_CSS1 =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SPACE;

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

    // Und die Position des ControlModel darin suchen
    sal_Int32 nCount = xIndexAcc->getCount(), nPos;
    for( nPos = 0 ; nPos < nCount; nPos++ )
    {
        uno::Any aTmp = xIndexAcc->getByIndex(nPos);
        OSL_ENSURE( aTmp.getValueType() ==
                        cppu::UnoType<form::XFormComponent>::get()||
                aTmp.getValueType() ==
                        cppu::UnoType<form::XForm>::get(),
                "lcl_html_outEvents: falsche Reflection" );
        if( aTmp.getValueType() ==
                    cppu::UnoType<form::XFormComponent>::get())

        {
            if( rFormComp ==
                    *static_cast<uno::Reference< form::XFormComponent > const *>(aTmp.getValue()) )
                break;
        }
        else if( aTmp.getValueType() ==
                            cppu::UnoType<form::XForm>::get())
        {
            uno::Reference< form::XFormComponent > xFC(
                *static_cast<uno::Reference< form::XForm > const *>(aTmp.getValue()), uno::UNO_QUERY );
            if( rFormComp == xFC )
                break;
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
        sal_Int32 nTok = comphelper::string::getTokenCount(sListener, '.');
        if( nTok )
            sListener = sListener.getToken( nTok-1, '.' );
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
            sOut = " " + OString(OOO_STRING_SVTOOLS_HTML_O_sdaddparam) +
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
    sal_uInt32 nStartIdx = pCurPam->GetPoint()->nNode.GetIndex();
    size_t i = 0;

    // Skip all controls in front of the current paragraph
    while ( i < m_aHTMLControls.size() && m_aHTMLControls[i]->nNdIdx < nStartIdx )
        ++i;

    return i < m_aHTMLControls.size() && m_aHTMLControls[i]->nNdIdx == nStartIdx;
}

void SwHTMLWriter::OutForm( bool bTag_On, const SwStartNode *pStartNd )
{
    if( m_bPreserveForm )     // wir sind in einer Tabelle oder einem Bereich
        return;             // ueber dem eine Form aufgespannt wurde

    if( !bTag_On )
    {
        // die Form beenden wenn alle Controls ausgegeben wurden
        if( mxFormComps.is() &&
            mxFormComps->getCount() == m_nFormCntrlCnt )
        {
            OutForm( false, mxFormComps );
            mxFormComps.clear();
        }
        return;
    }

    uno::Reference< container::XIndexContainer > xNewFormComps;  // die neue Form
    sal_uInt32 nStartIdx = pStartNd ? pStartNd->GetIndex()
                                    : pCurPam->GetPoint()->nNode.GetIndex();

    // Ueberspringen von Controls vor dem interesanten Bereich
    size_t i = 0;
    while ( i < m_aHTMLControls.size() && m_aHTMLControls[i]->nNdIdx < nStartIdx )
        ++i;

    if( !pStartNd )
    {
        // Check fuer einen einzelnen Node: da ist nur interessant, ob
        // es zu dem Node ein Control gibt und zu welcher Form es gehoert
        if( i < m_aHTMLControls.size() &&
            m_aHTMLControls[i]->nNdIdx == nStartIdx )
            xNewFormComps = m_aHTMLControls[i]->xFormComps;
    }
    else
    {
        // wir klappern eine Tabelle/einen Bereich ab: hier interessiert uns:
        // - ob es Controls mit unterschiedlichen Start-Nodes gibt
        // - ob es eine Form gibt, fuer die nicht alle Controls in der
        //   Tabelle/dem Bereich liegen

        uno::Reference< container::XIndexContainer > xCurrentFormComps;// die aktuelle Form in der Tabelle
        const SwStartNode *pCurrentStNd = nullptr; // und der Start-Node eines Ctrls
        sal_Int32 nCurrentCtrls = 0;   // und die in ihr gefundenen Controls
        sal_uInt32 nEndIdx =  pStartNd->EndOfSectionIndex();
        for( ; i < m_aHTMLControls.size() &&
            m_aHTMLControls[i]->nNdIdx <= nEndIdx; i++ )
        {
            const SwStartNode *pCntrlStNd =
                pDoc->GetNodes()[m_aHTMLControls[i]->nNdIdx]->StartOfSectionNode();

            if( xCurrentFormComps.is() )
            {
                // Wir befinden uns bereits in einer Form ...
                if( xCurrentFormComps==m_aHTMLControls[i]->xFormComps )
                {
                    // ... und das Control befindet sich auch darin ...
                    if( pCurrentStNd!=pCntrlStNd )
                    {
                        // ... aber es liegt in einer anderen Zelle:
                        // Dann muessen eir eine Form ueber der Tabelle
                        // aufmachen
                        xNewFormComps = xCurrentFormComps;
                        break;
                    }
                    nCurrentCtrls = nCurrentCtrls + m_aHTMLControls[i]->nCount;
                }
                else
                {
                    // ... aber das Control liegt in einer anderen Zelle:
                    // Da tun wir so, als ob wir eine neue Form aufmachen
                    // und suchen weiter.
                    xCurrentFormComps = m_aHTMLControls[i]->xFormComps;
                    pCurrentStNd = pCntrlStNd;
                    nCurrentCtrls = m_aHTMLControls[i]->nCount;
                }
            }
            else
            {
                // Wir befinden uns noch in keiner Form:
                // Da tun wir mal so, als ob wie wir die Form aufmachen.
                xCurrentFormComps = m_aHTMLControls[i]->xFormComps;
                pCurrentStNd = pCntrlStNd;
                nCurrentCtrls = m_aHTMLControls[i]->nCount;
            }
        }
        if( !xNewFormComps.is() && xCurrentFormComps.is() &&
            nCurrentCtrls != xCurrentFormComps->getCount() )
        {
            // In der Table/dem Bereich sollte eine Form aufgemacht werden,
            // die nicht vollstaendig in der Tabelle liegt. Dan muessen
            // wie die Form jetzt ebenfalls oeffen.
            xNewFormComps = xCurrentFormComps;
        }
    }

    if( xNewFormComps.is() &&
        (!mxFormComps.is() || !(xNewFormComps == mxFormComps)) )
    {
        // Es soll eine Form aufgemacht werden ...
        if( mxFormComps.is() )
        {
            // .. es ist aber noch eine Form offen: Das ist in
            // jedem Fall eine Fehler, aber wir schliessen die alte
            // Form trotzdem
            OutForm( false, mxFormComps );

            //!!!nWarn = 1; // Control wird falscher Form zugeordnet
        }

        mxFormComps = xNewFormComps;

        OutForm( true, mxFormComps );
        uno::Reference< beans::XPropertySet >  xTmp;
        OutHiddenControls( mxFormComps, xTmp );
    }
}

void SwHTMLWriter::OutHiddenForms()
{
    // Ohne DrawModel kann es auch keine Controls geben. Dann darf man
    // auch nicht per UNO auf das Dok zugreifen, weil sonst ein DrawModel
    // angelegt wird.
    if( !pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
        return;

    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( !pDocSh )
        return;

    uno::Reference< drawing::XDrawPageSupplier > xDPSupp( pDocSh->GetBaseModel(),
                                                     uno::UNO_QUERY );
    OSL_ENSURE( xDPSupp.is(), "XTextDocument nicht vom XModel erhalten" );
    uno::Reference< drawing::XDrawPage > xDrawPage = xDPSupp->getDrawPage();

    OSL_ENSURE( xDrawPage.is(), "XDrawPage nicht erhalten" );
    if( !xDrawPage.is() )
        return;

    uno::Reference< form::XFormsSupplier > xFormsSupplier( xDrawPage, uno::UNO_QUERY );
    OSL_ENSURE( xFormsSupplier.is(),
            "XFormsSupplier nicht vom XDrawPage erhalten" );

    uno::Reference< container::XNameContainer > xTmp = xFormsSupplier->getForms();
    OSL_ENSURE( xTmp.is(), "XForms nicht erhalten" );
    uno::Reference< container::XIndexContainer > xForms( xTmp, uno::UNO_QUERY );
    OSL_ENSURE( xForms.is(), "XForms ohne container::XIndexContainer?" );

    sal_Int32 nCount = xForms->getCount();
    for( sal_Int32 i=0; i<nCount; i++)
    {
        uno::Any aTmp = xForms->getByIndex( i );
        OSL_ENSURE( aTmp.getValueType() ==
                        cppu::UnoType<form::XForm>::get(),
                "OutHiddenForms: falsche Reflection" );
        if( aTmp.getValueType() ==
                    cppu::UnoType<form::XForm>::get())
            OutHiddenForm( *static_cast<uno::Reference< form::XForm > const *>(aTmp.getValue()) );
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
        OSL_ENSURE( aTmp.getValueType() ==
                        cppu::UnoType<form::XFormComponent>::get(),
                "OutHiddenForm: falsche Reflection" );
        if( aTmp.getValueType() !=
                    cppu::UnoType<form::XFormComponent>::get())
            continue;

        uno::Reference< form::XFormComponent > xFormComp =
            *static_cast<uno::Reference< form::XFormComponent > const *>(aTmp.getValue());
        uno::Reference< form::XForm > xForm( xFormComp, uno::UNO_QUERY );
        if( xForm.is() )
            OutHiddenForm( xForm );

        if( bHiddenOnly )
        {
            uno::Reference< beans::XPropertySet >  xPropSet( xFormComp, uno::UNO_QUERY );
            OUString sPropName("ClassId");
            if( xPropSet->getPropertySetInfo()->hasPropertyByName( sPropName ) )
            {
                uno::Any aAny2 = xPropSet->getPropertyValue( sPropName );
                if( aAny2.getValueType() == ::cppu::UnoType<sal_Int16>::get() )
                {
                    if( form::FormComponentType::HIDDENCONTROL ==
                                                *static_cast<sal_Int16 const *>(aAny2.getValue()) )
                        bHidden = true;
                    else if( lcl_html_isHTMLControl(
                                            *static_cast<sal_Int16 const *>(aAny2.getValue()) ) )
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
        DecIndentLevel(); // Inhalt der Form einruecken
        if( m_bLFPossible )
            OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_form, false );
        m_bLFPossible = true;

        return;
    }

    // die neue Form wird geoeffnet
    if( m_bLFPossible )
        OutNewLine();
    OString sOut = "<" + OString(OOO_STRING_SVTOOLS_HTML_form);

    uno::Reference< beans::XPropertySet > xFormPropSet( rFormComps, uno::UNO_QUERY );

    uno::Any aTmp = xFormPropSet->getPropertyValue( "Name" );
    if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
        !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_name) + "=\"";
        Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_String( Strm(), *static_cast<OUString const *>(aTmp.getValue()),
                                  m_eDestEnc, &m_aNonConvertableCharacters );
        sOut = "\"";
    }

    aTmp = xFormPropSet->getPropertyValue( "TargetURL" );
    if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
        !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_action) + "=\"";
        Strm().WriteOString( sOut );
        OUString aURL( *static_cast<OUString const *>(aTmp.getValue()) );
        aURL = URIHelper::simpleNormalizedMakeRelative( GetBaseURL(), aURL);
        HTMLOutFuncs::Out_String( Strm(), aURL, m_eDestEnc, &m_aNonConvertableCharacters );
        sOut = "\"";
    }

    aTmp = xFormPropSet->getPropertyValue( "SubmitMethod" );
    if( aTmp.getValueType() == cppu::UnoType<form::FormSubmitMethod>::get())
    {
        form::FormSubmitMethod eMethod =
                *static_cast<form::FormSubmitMethod const *>(aTmp.getValue());
        if( form::FormSubmitMethod_POST==eMethod )
        {
            sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_method) + "=\"" +
                OString(OOO_STRING_SVTOOLS_HTML_METHOD_post) + "\"";
        }
    }
    aTmp = xFormPropSet->getPropertyValue( "SubmitEncoding" );
    if( aTmp.getValueType()==cppu::UnoType<form::FormSubmitEncoding>::get())
    {
        form::FormSubmitEncoding eEncType =
                    *static_cast<form::FormSubmitEncoding const *>(aTmp.getValue());
        const sal_Char *pStr = nullptr;
        switch( eEncType )
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
            sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_enctype) + "=\"" +
                OString(pStr) + "\"";
        }
    }

    aTmp = xFormPropSet->getPropertyValue( "TargetFrame" );
    if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get()&&
        !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_target) + "=\"";
        Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_String( Strm(), *static_cast<OUString const *>(aTmp.getValue()),
                                  m_eDestEnc, &m_aNonConvertableCharacters );
        sOut = "\"";
    }

    Strm().WriteOString( sOut );
    uno::Reference< form::XFormComponent > xFormComp( rFormComps, uno::UNO_QUERY );
    lcl_html_outEvents( Strm(), xFormComp, m_bCfgStarBasic, m_eDestEnc, &m_aNonConvertableCharacters );
    Strm().WriteChar( '>' );

    IncIndentLevel(); // Inhalt der Form einruecken
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
            OSL_ENSURE( aTmp.getValueType() ==
                        cppu::UnoType<form::XFormComponent>::get(),
                    "OutHiddenControls: falsche Reflection" );
            bDone = aTmp.getValueType() ==
                        cppu::UnoType<form::XFormComponent>::get()&&
                    *static_cast<uno::Reference< form::XFormComponent > const *>(aTmp.getValue()) ==
                        xFC;
        }
    }

    for( ; nPos < nCount; nPos++ )
    {
        uno::Any aTmp = rFormComps->getByIndex( nPos );
        OSL_ENSURE( aTmp.getValueType() ==
                        cppu::UnoType<form::XFormComponent>::get(),
                "OutHiddenControls: falsche Reflection" );
        if( aTmp.getValueType() !=
                    cppu::UnoType<form::XFormComponent>::get())
            continue;
        uno::Reference< form::XFormComponent > xFC =
                *static_cast<uno::Reference< form::XFormComponent > const *>(aTmp.getValue());
        uno::Reference< beans::XPropertySet > xPropSet( xFC, uno::UNO_QUERY );

        OUString sPropName = "ClassId";
        if( !xPropSet->getPropertySetInfo()->hasPropertyByName( sPropName ) )
            continue;

        aTmp = xPropSet->getPropertyValue( sPropName );
        if( aTmp.getValueType() != ::cppu::UnoType<sal_Int16>::get() )
            continue;

        if( form::FormComponentType::HIDDENCONTROL ==
                                            *static_cast<sal_Int16 const *>(aTmp.getValue()) )
        {
            if( m_bLFPossible )
                OutNewLine( true );
            OString sOut = "<" + OString(OOO_STRING_SVTOOLS_HTML_input) + " " +
                OString(OOO_STRING_SVTOOLS_HTML_O_type) + "=\"" +
                OString(OOO_STRING_SVTOOLS_HTML_IT_hidden) + "\"";

            aTmp = xPropSet->getPropertyValue( "Name" );
            if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
                !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
            {
                sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_name) + "=\"";
                Strm().WriteOString( sOut );
                HTMLOutFuncs::Out_String( Strm(), *static_cast<OUString const *>(aTmp.getValue()),
                                          m_eDestEnc, &m_aNonConvertableCharacters );
                sOut = "\"";
            }
            aTmp = xPropSet->getPropertyValue( "HiddenValue" );
            if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
                !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
            {
                sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_value) + "=\"";
                Strm().WriteOString( sOut );
                HTMLOutFuncs::Out_String( Strm(), *static_cast<OUString const *>(aTmp.getValue()),
                                          m_eDestEnc, &m_aNonConvertableCharacters );
                sOut = "\"";
            }
            sOut += ">";
            Strm().WriteOString( sOut );

            m_nFormCntrlCnt++;
        }
        else if( lcl_html_isHTMLControl( *static_cast<sal_Int16 const *>(aTmp.getValue()) ) )
        {
            break;
        }
    }
}

// hier folgen die Ausgabe-Routinen, dadurch sind die form::Forms gebuendelt:

const SdrObject *SwHTMLWriter::GetHTMLControl( const SwDrawFrameFormat& rFormat )
{
    // es muss ein Draw-Format sein
    OSL_ENSURE( RES_DRAWFRMFMT == rFormat.Which(),
            "GetHTMLControl nuer fuer Draw-Formate erlaubt" );

    // Schauen, ob es ein SdrObject dafuer gibt
    const SdrObject *pObj = rFormat.FindSdrObject();
    if( !pObj || FmFormInventor != pObj->GetObjInventor() )
        return nullptr;

    const SdrUnoObj& rFormObj = dynamic_cast<const SdrUnoObj&>(*pObj);
    uno::Reference< awt::XControlModel >  xControlModel =
            rFormObj.GetUnoControlModel();

    OSL_ENSURE( xControlModel.is(), "UNO-Control ohne Model" );
    if( !xControlModel.is() )
        return nullptr;

    uno::Reference< beans::XPropertySet >  xPropSet( xControlModel, uno::UNO_QUERY );

    OUString sPropName("ClassId");
    if( !xPropSet->getPropertySetInfo()->hasPropertyByName( sPropName ) )
        return nullptr;

    uno::Any aTmp = xPropSet->getPropertyValue( sPropName );
    if( aTmp.getValueType() == ::cppu::UnoType<sal_Int16>::get()&&
        lcl_html_isHTMLControl( *static_cast<sal_Int16 const *>(aTmp.getValue()) ) )
    {
        return pObj;
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
    OSL_ENSURE( xLC.is(), "kein XTextLayoutConstrains" );
    if( !xLC.is() )
        return;

    sal_Int16 nCols=0, nLines=0;
    xLC->getColumnsAndLines( nCols, nLines );
    rSz.Width() = nCols;
    rSz.Height() = nLines;
}

Writer& OutHTML_DrawFrameFormatAsControl( Writer& rWrt,
                                     const SwDrawFrameFormat& rFormat,
                                     const SdrUnoObj& rFormObj,
                                     bool bInCntnr )
{
    uno::Reference< awt::XControlModel > xControlModel =
        rFormObj.GetUnoControlModel();

    OSL_ENSURE( xControlModel.is(), "UNO-Control ohne Model" );
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
    sal_Int16 nClassId = *static_cast<sal_Int16 const *>(aTmp.getValue());
    sal_uInt32 nFrameOpts = HTML_FRMOPTS_CONTROL;
    switch( nClassId )
    {
    case form::FormComponentType::CHECKBOX:
    case form::FormComponentType::RADIOBUTTON:
        eType = (form::FormComponentType::CHECKBOX == nClassId
                    ? TYPE_CHECKBOX : TYPE_RADIO);
        aTmp = xPropSet->getPropertyValue( "DefaultState" );
        if( aTmp.getValueType() == ::cppu::UnoType<sal_Int16>::get() &&
            TRISTATE_FALSE != *static_cast<sal_Int16 const *>(aTmp.getValue()) )
        {
            sOptions += " " + OString(OOO_STRING_SVTOOLS_HTML_O_checked);
            sOptions += "=\"";
            sOptions += OString(OOO_STRING_SVTOOLS_HTML_O_checked);
            sOptions += "\"";
        }

        aTmp = xPropSet->getPropertyValue( "RefValue" );
        if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() )

        {
            const OUString& rVal = *static_cast<OUString const *>(aTmp.getValue());
            if( rVal.isEmpty() )
                bEmptyValue = true;
            else if( rVal != OOO_STRING_SVTOOLS_HTML_on )
                sValue = rVal;
        }
        break;

    case form::FormComponentType::COMMANDBUTTON:
        {
            form::FormButtonType eButtonType = form::FormButtonType_PUSH;
            aTmp = xPropSet->getPropertyValue( "ButtonType" );
            if( aTmp.getValueType() ==
                            ::cppu::UnoType<form::FormButtonType>::get() )
                eButtonType = *static_cast<form::FormButtonType const *>(aTmp.getValue());

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
            if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
                !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
            {
                sValue = *static_cast<OUString const *>(aTmp.getValue());
            }
        }
        break;

    case form::FormComponentType::LISTBOX:
        if( rHTMLWrt.m_bLFPossible )
            rHTMLWrt.OutNewLine( true );
        eTag = TAG_SELECT;
        aTmp = xPropSet->getPropertyValue( "Dropdown" );
        if( aTmp.getValueType() == cppu::UnoType<bool>::get() &&
            !*static_cast<sal_Bool const *>(aTmp.getValue()) )
        {
            Size aSz( 0, 0 );
            GetControlSize( rFormObj, aSz, rWrt.pDoc );

            // wieviele sind sichtbar ??
            if( aSz.Height() )
            {
                sOptions += " " + OString(OOO_STRING_SVTOOLS_HTML_O_size) + "=\"" +
                    OString::number(static_cast<sal_Int32>(aSz.Height())) + "\"";
            }

            aTmp = xPropSet->getPropertyValue( "MultiSelection" );
            if( aTmp.getValueType() == cppu::UnoType<bool>::get() &&
                *static_cast<sal_Bool const *>(aTmp.getValue()) )
            {
                sOptions += " " + OString(OOO_STRING_SVTOOLS_HTML_O_multiple);
            }
        }
        break;

    case form::FormComponentType::TEXTFIELD:
        {
            Size aSz( 0, 0 );
            GetControlSize( rFormObj, aSz, rWrt.pDoc );

            bool bMultiLine = false;
            OUString sMultiLine("MultiLine");
            if( xPropSetInfo->hasPropertyByName( sMultiLine ) )
            {
                aTmp = xPropSet->getPropertyValue( sMultiLine );
                bMultiLine = aTmp.getValueType() == cppu::UnoType<bool>::get() &&
                             *static_cast<sal_Bool const *>(aTmp.getValue());
            }

            if( bMultiLine )
            {
                if( rHTMLWrt.m_bLFPossible )
                    rHTMLWrt.OutNewLine( true );
                eTag = TAG_TEXTAREA;

                if( aSz.Height() )
                {
                    sOptions += " " + OString(OOO_STRING_SVTOOLS_HTML_O_rows) + "=\"" +
                        OString::number(static_cast<sal_Int32>(aSz.Height())) + "\"";
                }
                if( aSz.Width() )
                {
                    sOptions += " " + OString(OOO_STRING_SVTOOLS_HTML_O_cols) + "=\"" +
                        OString::number(static_cast<sal_Int32>(aSz.Width())) + "\"";
                }

                aTmp = xPropSet->getPropertyValue( "HScroll" );
                if( aTmp.getValueType() == cppu::UnoType<void>::get() ||
                    (aTmp.getValueType() == cppu::UnoType<bool>::get() &&
                    !*static_cast<sal_Bool const *>(aTmp.getValue())) )
                {
                    const sal_Char *pWrapStr = nullptr;
                    aTmp = xPropSet->getPropertyValue( "HardLineBreaks" );
                    pWrapStr =
                        (aTmp.getValueType() == cppu::UnoType<bool>::get() &&
                        *static_cast<sal_Bool const *>(aTmp.getValue())) ? OOO_STRING_SVTOOLS_HTML_WW_hard
                                                     : OOO_STRING_SVTOOLS_HTML_WW_soft;
                    sOptions += " " + OString(OOO_STRING_SVTOOLS_HTML_O_wrap) + "=\"" +
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
                    if( aTmp.getValueType() == ::cppu::UnoType<sal_Int16>::get() &&
                        *static_cast<sal_Int16 const *>(aTmp.getValue()) != 0 )
                        eType = TYPE_PASSWORD;
                }

                if( aSz.Width() )
                {
                    sOptions += " " + OString(OOO_STRING_SVTOOLS_HTML_O_size) + "=\"" +
                        OString::number(static_cast<sal_Int32>(aSz.Width())) + "\"";
                }

                aTmp = xPropSet->getPropertyValue( "MaxTextLen" );
                if( aTmp.getValueType() == ::cppu::UnoType<sal_Int16>::get() &&
                    *static_cast<sal_Int16 const *>(aTmp.getValue()) != 0 )
                {
                    sOptions += " " + OString(OOO_STRING_SVTOOLS_HTML_O_maxlength) + "=\"" +
                        OString::number(static_cast<sal_Int32>(*static_cast<sal_Int16 const *>(aTmp.getValue()))) + "\"";
                }

                if( xPropSetInfo->hasPropertyByName( "DefaultText" ) )
                {
                    aTmp = xPropSet->getPropertyValue( "DefaultText" );
                    if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
                        !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
                    {
                        sValue = *static_cast<OUString const *>(aTmp.getValue());
                    }
                }
            }
        }
        break;

    case form::FormComponentType::FILECONTROL:
        {
            Size aSz( 0, 0 );
            GetControlSize( rFormObj, aSz, rWrt.pDoc );
            eType = TYPE_FILE;

            if( aSz.Width() )
            {
                sOptions += " " + OString(OOO_STRING_SVTOOLS_HTML_O_size) + "=\"" +
                    OString::number(static_cast<sal_Int32>(aSz.Width())) + "\"";
            }

            // VALUE vim form aus Sicherheitsgruenden nicht exportieren
        }
        break;

    case form::FormComponentType::IMAGEBUTTON:
        eType = TYPE_IMAGE;
        nFrameOpts = HTML_FRMOPTS_IMG_CONTROL;
        break;

    default:                // kennt HTML nicht
        eTag = TAG_NONE;        // also ueberspringen
        break;
    }

    if( eTag == TAG_NONE )
        return rWrt;

    OString sOut = "<" + OString(TagNames[eTag]);
    if( eType != TYPE_NONE )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_type) + "=\"" +
            OString(TypeNames[eType]) + "\"";
    }

    aTmp = xPropSet->getPropertyValue("Name");
    if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
        !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_name) + "=\"";
        rWrt.Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_String( rWrt.Strm(), *static_cast<OUString const *>(aTmp.getValue()),
                                  rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
        sOut = "\"";
    }

    aTmp = xPropSet->getPropertyValue("Enabled");
    if( aTmp.getValueType() == cppu::UnoType<bool>::get() &&
        !*static_cast<sal_Bool const *>(aTmp.getValue()) )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_disabled);
    }

    if( !sValue.isEmpty() || bEmptyValue )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_value) + "=\"";
        rWrt.Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_String( rWrt.Strm(), sValue, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
        sOut = "\"";
    }

    sOut += " " + sOptions;

    if( TYPE_IMAGE == eType )
    {
        aTmp = xPropSet->getPropertyValue( "ImageURL" );
        if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
            !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
        {
            sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_src) + "=\"";
            rWrt.Strm().WriteOString( sOut );

            HTMLOutFuncs::Out_String( rWrt.Strm(),
                        URIHelper::simpleNormalizedMakeRelative( rWrt.GetBaseURL(), *static_cast<OUString const *>(aTmp.getValue())),
                        rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            sOut = "\"";
        }

        Size aTwipSz( rFormObj.GetLogicRect().GetSize() );
        Size aPixelSz( 0, 0 );
        if( (aTwipSz.Width() || aTwipSz.Height()) &&
            Application::GetDefaultDevice() )
        {
            aPixelSz =
                Application::GetDefaultDevice()->LogicToPixel( aTwipSz,
                                                    MapMode(MAP_TWIP) );
            if( !aPixelSz.Width() && aTwipSz.Width() )
                aPixelSz.Width() = 1;
            if( !aPixelSz.Height() && aTwipSz.Height() )
                aPixelSz.Height() = 1;
        }

        if( aPixelSz.Width() )
        {
            sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_width) + "=\"" +
                OString::number(static_cast<sal_Int32>(aPixelSz.Width())) + "\"";
        }

        if( aPixelSz.Height() )
        {
            sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_height) + "=\"" +
                OString::number(static_cast<sal_Int32>(aPixelSz.Height())) + "\"";
        }
    }

    aTmp = xPropSet->getPropertyValue( "TabIndex" );
    if( aTmp.getValueType() == ::cppu::UnoType<sal_Int16>::get() )
    {
        sal_Int16 nTabIndex = *static_cast<sal_Int16 const *>(aTmp.getValue());
        if( nTabIndex > 0 )
        {
            if( nTabIndex >= 32767 )
                nTabIndex = 32767;

            sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_tabindex) + "=\"" +
                OString::number(static_cast<sal_Int32>(nTabIndex)) + "\"";
        }
    }

    if( !sOut.isEmpty() )
        rWrt.Strm().WriteOString( sOut );

    OSL_ENSURE( !bInCntnr, "Container wird fuer Controls nicht unterstuertzt" );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_DRAW ) && !bInCntnr )
    {
        // Wenn Zeichen-Objekte nicht absolut positioniert werden duerfen,
        // das entsprechende Flag loeschen.
        nFrameOpts |= (TYPE_IMAGE == eType
                            ? HTML_FRMOPTS_IMG_CONTROL_CSS1
                            : HTML_FRMOPTS_CONTROL_CSS1);
    }
    OString aEndTags;
    if( nFrameOpts != 0 )
        aEndTags = rHTMLWrt.OutFrameFormatOptions( rFormat, aEmptyOUStr, nFrameOpts );

    if( rHTMLWrt.m_bCfgOutStyles )
    {
        bool bEdit = TAG_TEXTAREA == eTag || TYPE_FILE == eType ||
                     TYPE_TEXT == eType;

        SfxItemSet aItemSet( rHTMLWrt.pDoc->GetAttrPool(), RES_CHRATR_BEGIN,
                             RES_CHRATR_END );
        if( xPropSetInfo->hasPropertyByName( "BackgroundColor" ) )
        {
            aTmp = xPropSet->getPropertyValue( "BackgroundColor" );
            if( aTmp.getValueType() == ::cppu::UnoType<sal_Int32>::get() )
            {
                Color aCol(*static_cast<sal_Int32 const *>(aTmp .getValue()));
                aItemSet.Put( SvxBrushItem( aCol, RES_CHRATR_BACKGROUND ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "TextColor" ) )
        {
            aTmp = xPropSet->getPropertyValue( "TextColor" );
            if( aTmp.getValueType() == ::cppu::UnoType<sal_Int32>::get() )
            {
                Color aColor( *static_cast<sal_Int32 const *>(aTmp .getValue()) );
                aItemSet.Put( SvxColorItem( aColor, RES_CHRATR_COLOR ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontHeight" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontHeight" );
            if( aTmp.getValueType() == cppu::UnoType<float>::get())

            {
                float nHeight = *static_cast<float const *>(aTmp.getValue());
                if( nHeight > 0  && (!bEdit || nHeight != 10.) )
                    aItemSet.Put( SvxFontHeightItem( sal_Int16(nHeight * 20.), 100, RES_CHRATR_FONTSIZE ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontName" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontName" );
            if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
                !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
            {
                vcl::Font aFixedFont( OutputDevice::GetDefaultFont(
                                    DefaultFontType::FIXED, LANGUAGE_ENGLISH_US,
                                    GetDefaultFontFlags::OnlyOne ) );
                OUString aFName( *static_cast<OUString const *>(aTmp.getValue()) );
                if( !bEdit || aFName != aFixedFont.GetFamilyName() )
                {
                    FontFamily eFamily = FAMILY_DONTKNOW;
                    if( xPropSetInfo->hasPropertyByName( "FontFamily" ) )
                    {
                        aTmp = xPropSet->getPropertyValue( "FontFamily" );
                        if( aTmp.getValueType() == ::cppu::UnoType<sal_Int16>::get())
                            eFamily = (FontFamily)*static_cast<sal_Int16 const *>(aTmp.getValue());
                    }
                    SvxFontItem aItem( eFamily, aFName, aEmptyOUStr, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, RES_CHRATR_FONT );
                    aItemSet.Put( aItem );
                }
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontWeight" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontWeight" );
            if( aTmp.getValueType() == cppu::UnoType<float>::get())
            {
                FontWeight eWeight =
                    VCLUnoHelper::ConvertFontWeight( *static_cast<float const *>(aTmp.getValue()) );
                if( eWeight != WEIGHT_DONTKNOW && eWeight != WEIGHT_NORMAL )
                    aItemSet.Put( SvxWeightItem( eWeight, RES_CHRATR_WEIGHT ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontSlant" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontSlant" );
            if( aTmp.getValueType() == ::cppu::UnoType<sal_Int16>::get())
            {
                FontItalic eItalic = (FontItalic)*static_cast<sal_Int16 const *>(aTmp.getValue());
                if( eItalic != ITALIC_DONTKNOW && eItalic != ITALIC_NONE )
                    aItemSet.Put( SvxPostureItem( eItalic, RES_CHRATR_POSTURE ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontUnderline" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontUnderline" );
            if( aTmp.getValueType() == ::cppu::UnoType<sal_Int16>::get() )
            {
                FontUnderline eUnderline =
                    (FontUnderline)*static_cast<sal_Int16 const *>(aTmp.getValue());
                if( eUnderline != UNDERLINE_DONTKNOW  &&
                    eUnderline != UNDERLINE_NONE )
                    aItemSet.Put( SvxUnderlineItem( eUnderline, RES_CHRATR_UNDERLINE ) );
            }
        }
        if( xPropSetInfo->hasPropertyByName( "FontStrikeout" ) )
        {
            aTmp = xPropSet->getPropertyValue( "FontStrikeout" );
            if( aTmp.getValueType() == ::cppu::UnoType<sal_Int16>::get())
            {
                FontStrikeout eStrikeout =
                    (FontStrikeout)*static_cast<sal_Int16 const *>(aTmp.getValue());
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
        if( aTmp.getValueType() == cppu::UnoType<uno::Sequence<OUString>>::get() )
        {
            rHTMLWrt.IncIndentLevel(); // der Inhalt von Select darf
                                       // eingerueckt werden
            uno::Sequence<OUString> aList( *static_cast<uno::Sequence<OUString> const *>(aTmp.getValue()) );
            sal_Int32 nCnt = aList.getLength();
            const OUString *pStrings = aList.getConstArray();

            const OUString *pValues = nullptr;
            sal_Int32 nValCnt = 0;
            aTmp = xPropSet->getPropertyValue( "ListSource" );
            uno::Sequence<OUString> aValList;
            if( aTmp.getValueType() == cppu::UnoType<uno::Sequence<OUString>>::get() )
            {
                aValList = *static_cast<uno::Sequence<OUString> const *>(aTmp.getValue());
                nValCnt = aValList.getLength();
                pValues = aValList.getConstArray();
            }

            uno::Any aSelTmp = xPropSet->getPropertyValue( "DefaultSelection" );
            const sal_Int16 *pSels = nullptr;
            sal_Int32 nSel = 0;
            sal_Int32 nSelCnt = 0;
            uno::Sequence<sal_Int16> aSelList;
            if( aSelTmp.getValueType() ==cppu::UnoType<uno::Sequence<sal_Int16>>::get())
            {
                aSelList = *static_cast<uno::Sequence<sal_Int16> const *>(aSelTmp.getValue());
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

                rHTMLWrt.OutNewLine(); // jede Option bekommt eine eigene Zeile
                sOut = "<" + OString(OOO_STRING_SVTOOLS_HTML_option);
                if( !sVal.isEmpty() || bEmptyVal )
                {
                    sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_value) + "=\"";
                    rWrt.Strm().WriteOString( sOut );
                    HTMLOutFuncs::Out_String( rWrt.Strm(), sVal,
                        rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
                    sOut = "\"";
                }
                if( bSelected )
                    sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_selected);

                sOut += ">";
                rWrt.Strm().WriteOString( sOut );

                HTMLOutFuncs::Out_String( rWrt.Strm(), pStrings[i],
                                          rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            }
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_option, false );

            rHTMLWrt.DecIndentLevel();
            rHTMLWrt.OutNewLine();// das </SELECT> bekommt eine eigene Zeile
        }
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_select, false );
    }
    else if( TAG_TEXTAREA == eTag )
    {
        // In TextAreas duerfen keine zusaetzlichen Spaces oder LF exportiert
        // werden!
        OUString sVal;
        aTmp = xPropSet->getPropertyValue( "DefaultText" );
        if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get()&&
            !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
        {
            sVal = *static_cast<OUString const *>(aTmp.getValue());
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
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_textarea, false );
    }
    else if( TYPE_CHECKBOX == eType || TYPE_RADIO == eType )
    {
        aTmp = xPropSet->getPropertyValue("Label");
        if( aTmp.getValueType() == ::cppu::UnoType<OUString>::get() &&
            !static_cast<const OUString*>(aTmp.getValue())->isEmpty() )
        {
            sValue = *static_cast<OUString const *>(aTmp.getValue());
            HTMLOutFuncs::Out_String( rWrt.Strm(), sValue,
                rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters ).WriteChar( ' ' );
        }
    }

    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteCharPtr( aEndTags.getStr() );

    // Controls sind nicht absatz-gebunden, deshalb kein LF mehr ausgeben!
    rHTMLWrt.m_bLFPossible = false;

    if( rHTMLWrt.mxFormComps.is() )
        rHTMLWrt.OutHiddenControls( rHTMLWrt.mxFormComps, xPropSet );
    return rWrt;
}

// Ermitteln, ob eine Format zu einem Control gehoert und wenn ja
// dessen Form zurueckgeben
static void AddControl( HTMLControls& rControls,
                        const SdrUnoObj& rFormObj,
                        sal_uInt32 nNodeIdx )
{
    uno::Reference< awt::XControlModel > xControlModel =
            rFormObj.GetUnoControlModel();
    if( !xControlModel.is() )
        return;

    uno::Reference< form::XFormComponent >  xFormComp( xControlModel, uno::UNO_QUERY );
    uno::Reference< uno::XInterface >  xIfc = xFormComp->getParent();
    uno::Reference< form::XForm >  xForm(xIfc, uno::UNO_QUERY);

    OSL_ENSURE( xForm.is(), "Wo ist die Form?" );
    if( xForm.is() )
    {
        uno::Reference< container::XIndexContainer >  xFormComps( xForm, uno::UNO_QUERY );
        std::unique_ptr<HTMLControl> pHCntrl(new HTMLControl( xFormComps, nNodeIdx ));
        HTMLControls::const_iterator it = rControls.find( pHCntrl.get() );
        if( it == rControls.end() )
            rControls.insert( pHCntrl.release() );
        else
        {
            if( (*it)->xFormComps==xFormComps )
                (*it)->nCount++;
        }
    }
}

void SwHTMLWriter::GetControls()
{
    // Idee: die absatz- und zeichengebundenen Controls werden erst einmal
    // eingesammelt. Dabei wird fuer jedes Control des Absatz-Position
    // und VCForm in einem Array gemerkt.
    // Ueber dieses Array laesst sich dann feststellen, wo form::Forms geoeffnet
    // und geschlossen werden muessen.

    if( m_pHTMLPosFlyFrames )
    {
        // die absatz-gebundenen Controls einsammeln
        for( size_t i=0; i<m_pHTMLPosFlyFrames->size(); i++ )
        {
            const SwHTMLPosFlyFrame* pPosFlyFrame = (*m_pHTMLPosFlyFrames)[ i ];
            if( HTML_OUT_CONTROL != pPosFlyFrame->GetOutFn() )
                continue;

            const SdrObject *pSdrObj = pPosFlyFrame->GetSdrObject();
            OSL_ENSURE( pSdrObj, "Wo ist das SdrObject?" );
            if( !pSdrObj )
                continue;

            AddControl( m_aHTMLControls, dynamic_cast<const SdrUnoObj&>(*pSdrObj),
                        pPosFlyFrame->GetNdIndex().GetIndex() );
        }
    }

    // und jetzt die in einem zeichengebundenen Rahmen
    const SwFrameFormats* pSpzFrameFormats = pDoc->GetSpzFrameFormats();
    for( size_t i=0; i<pSpzFrameFormats->size(); i++ )
    {
        const SwFrameFormat *pFrameFormat = (*pSpzFrameFormats)[i];
        if( RES_DRAWFRMFMT != pFrameFormat->Which() )
            continue;

        const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
        const SwPosition *pPos = rAnchor.GetContentAnchor();
        if ((FLY_AS_CHAR != rAnchor.GetAnchorId()) || !pPos)
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
