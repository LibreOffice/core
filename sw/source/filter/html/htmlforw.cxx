/*************************************************************************
 *
 *  $RCSfile: htmlforw.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 12:39:50 $
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

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITENCODING_HPP_
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITMETHOD_HPP_
#include <com/sun/star/form/FormSubmitMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHER_HPP_
#include <com/sun/star/script/XEventAttacher.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#endif

#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
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
#ifndef _UNOOBJ_HXX //autogen
#include <unoobj.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif


#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

#ifndef _VIEWSH_HXX //autogen
#include <viewsh.hxx>
#endif
#include "pam.hxx"
#include "doc.hxx"
#include "ndtxt.hxx"
#include "dcontact.hxx"
#include "flypos.hxx"
#include "wrthtml.hxx"
#include "htmlfly.hxx"
#include "htmlform.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
/*  */

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


/*  */

struct HTMLControl
{
    // die Form, zu der das Control gehoert
    Reference< container::XIndexContainer > xFormComps;
    sal_uInt32 nNdIdx;              // der Node, in dem es verankert ist
    xub_StrLen nCount;              // wie viele Controls sind in dem Node

    HTMLControl( const Reference< container::XIndexContainer > & rForm,
                 sal_uInt32 nIdx );
    ~HTMLControl();

    // operatoren fuer das Sort-Array
    sal_Bool operator==( const HTMLControl& rCtrl )
    {
        return nNdIdx == rCtrl.nNdIdx;
    }
    sal_Bool operator<( const HTMLControl& rCtrl )
    {
        return nNdIdx < rCtrl.nNdIdx;
    }
};

SV_IMPL_OP_PTRARR_SORT( HTMLControls, HTMLControl* )

/*  */

void lcl_html_outEvents( SvStream& rStrm,
                         const Reference< form::XFormComponent > rFormComp,
                         sal_Bool bCfgStarBasic,
                         rtl_TextEncoding eDestEnc )
{
    Reference< container::XChild > xChild( rFormComp, UNO_QUERY );
    Reference< XInterface > xParentIfc = xChild->getParent();
    ASSERT( xParentIfc.is(), "lcl_html_outEvents: no parent interface" );
    if( !xParentIfc.is() )
        return;
    Reference< container::XIndexAccess > xIndexAcc( xParentIfc, UNO_QUERY );
    Reference< script::XEventAttacherManager > xEventManager( xParentIfc,
                                                              UNO_QUERY );
    if( !xIndexAcc.is() || !xEventManager.is() )
        return;

    // Und die Position des ControlModel darin suchen
    sal_Int32 nCount = xIndexAcc->getCount(), nPos;
    for( nPos = 0 ; nPos < nCount; nPos++ )
    {
        Any aTmp = xIndexAcc->getByIndex(nPos);
        ASSERT( aTmp.getValueType() ==
                        ::getCppuType( (Reference<form::XFormComponent>*)0 ) ||
                aTmp.getValueType() ==
                        ::getCppuType( (Reference<form::XForm>*)0 ),
                "lcl_html_outEvents: falsche Reflection" );
        if( aTmp.getValueType() ==
                    ::getCppuType( (Reference< form::XFormComponent >*)0) )

        {
            if( rFormComp ==
                    *(Reference< form::XFormComponent > *)aTmp.getValue() )
                break;
        }
        else if( aTmp.getValueType() ==
                            ::getCppuType( (Reference< form::XForm>*)0) )
        {
            Reference< form::XFormComponent > xFC(
                *(Reference< form::XForm > *)aTmp.getValue(), UNO_QUERY );
            if( rFormComp == xFC )
                break;
        }
    }

    if( nPos == nCount )
        return;

    Sequence< script::ScriptEventDescriptor > aDescs =
            xEventManager->getScriptEvents( nPos );
    nCount = aDescs.getLength();
    if( !nCount )
        return;

    const script::ScriptEventDescriptor *pDescs = aDescs.getConstArray();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        ScriptType eScriptType = EXTENDED_STYPE;
        String aScriptType( pDescs[i].ScriptType );
        if( aScriptType.EqualsIgnoreCaseAscii(SVX_MACRO_LANGUAGE_JAVASCRIPT) )
            eScriptType = JAVASCRIPT;
        else if( aScriptType.EqualsIgnoreCaseAscii(SVX_MACRO_LANGUAGE_STARBASIC ) )
            eScriptType = STARBASIC;
        if( JAVASCRIPT != eScriptType && !bCfgStarBasic )
            continue;

        String sListener( pDescs[i].ListenerType );
        xub_StrLen nTok = sListener.GetTokenCount( '.' );
        if( nTok )
            sListener = sListener.GetToken( nTok-1, '.' );
        String sMethod( pDescs[i].EventMethod );

        const sal_Char *pOpt = 0;
        for( sal_uInt16 j=0; aEventListenerTable[j]; j++ )
        {
            if( sListener.EqualsAscii( aEventListenerTable[j] ) &&
                sMethod.EqualsAscii( aEventMethodTable[j] ) )
            {
                pOpt = (STARBASIC==eScriptType ? aEventSDOptionTable
                                               : aEventOptionTable)[j];
                break;
            }
        }

        ByteString sOut( ' ' );
        if( pOpt && (EXTENDED_STYPE != eScriptType ||
                     !pDescs[i].AddListenerParam.getLength()) )
            sOut += pOpt;
        else
            (((sOut += sHTML_O_sdevent)
                += ByteString( sListener, RTL_TEXTENCODING_ASCII_US)) += '-')
                 += ByteString( sMethod, RTL_TEXTENCODING_ASCII_US);
        sOut += "=\"";
        rStrm << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rStrm, pDescs[i].ScriptCode, eDestEnc );
        rStrm << '\"';
        if( EXTENDED_STYPE == eScriptType &&
            pDescs[i].AddListenerParam.getLength() )
        {
            (((((sOut = ' ') += sHTML_O_sdaddparam)
                += ByteString( sListener, RTL_TEXTENCODING_ASCII_US)) += '-')
                 += ByteString( sMethod, RTL_TEXTENCODING_ASCII_US))
                += "=\"";
            rStrm << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rStrm, pDescs[i].AddListenerParam,
                                      eDestEnc );
            rStrm << '\"';
        }
    }
}

sal_Bool lcl_html_isHTMLControl( sal_Int16 nClassId )
{
    sal_Bool bRet = sal_False;

    switch( nClassId )
    {
    case form::FormComponentType::TEXTFIELD:
    case form::FormComponentType::COMMANDBUTTON:
    case form::FormComponentType::RADIOBUTTON:
    case form::FormComponentType::CHECKBOX:
    case form::FormComponentType::LISTBOX:
    case form::FormComponentType::IMAGEBUTTON:
    case form::FormComponentType::FILECONTROL:
        bRet = sal_True;
        break;
    }

    return bRet;
}

sal_Bool SwHTMLWriter::HasControls() const
{
    sal_uInt32 nStartIdx = pCurPam->GetPoint()->nNode.GetIndex();

    // Skip all controls in front of the current paragraph
    for( sal_uInt16 i=0; i < aHTMLControls.Count() &&
        aHTMLControls[i]->nNdIdx < nStartIdx; i++ )
        ;

    return i < aHTMLControls.Count() && aHTMLControls[i]->nNdIdx == nStartIdx;
}

void SwHTMLWriter::OutForm( sal_Bool bTagOn, const SwStartNode *pStartNd )
{
    if( bPreserveForm )     // wir sind in einer Tabelle oder einem Bereich
        return;             // ueber dem eine Form aufgespannt wurde

    if( !bTagOn )
    {
        // die Form beenden wenn alle Controls ausgegeben wurden
        if( pxFormComps && pxFormComps->is() &&
            (*pxFormComps)->getCount() == nFormCntrlCnt )
        {
            OutForm( sal_False, *pxFormComps );
            (*pxFormComps) = 0;
        }
        return;
    }

    Reference< container::XIndexContainer > xNewFormComps;  // die neue Form
    sal_uInt32 nStartIdx = pStartNd ? pStartNd->GetIndex()
                                       : pCurPam->GetPoint()->nNode.GetIndex();

    // Ueberspringen von Controls vor dem interesanten Bereich
    for( sal_uInt16 i=0; i < aHTMLControls.Count() &&
        aHTMLControls[i]->nNdIdx < nStartIdx; i++ )
        ;

    if( !pStartNd )
    {
        // Check fuer einen einzelnen Node: da ist nur interessant, ob
        // es zu dem Node ein Control gibt und zu welcher Form es gehoert
        if( i < aHTMLControls.Count() &&
            aHTMLControls[i]->nNdIdx == nStartIdx )
            xNewFormComps = aHTMLControls[i]->xFormComps;
    }
    else
    {
        // wir klappern eine Tabelle/einen Bereich ab: hier interessiert uns:
        // - ob es Controls mit unterschiedlichen Start-Nodes gibt
        // - ob es eine Form gibt, fuer die nicht alle Controls in der
        //   Tabelle/dem Bereich liegen

        Reference< container::XIndexContainer > xCurrentFormComps;// die aktuelle Form in der Tabelle
        const SwStartNode *pCurrentStNd = 0; // und der Start-Node eines Ctrls
        xub_StrLen nCurrentCtrls = 0;   // und die in ihr gefundenen Controls
        sal_uInt32 nEndIdx =  pStartNd->EndOfSectionIndex();
        for( ; i < aHTMLControls.Count() &&
            aHTMLControls[i]->nNdIdx <= nEndIdx; i++ )
        {
            const SwStartNode *pCntrlStNd =
                pDoc->GetNodes()[aHTMLControls[i]->nNdIdx]->FindStartNode();

            if( xCurrentFormComps.is() )
            {
                // Wir befinden uns bereits in einer Form ...
                if( xCurrentFormComps==aHTMLControls[i]->xFormComps )
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
                    nCurrentCtrls += aHTMLControls[i]->nCount;
                }
                else
                {
                    // ... aber das Control liegt in einer anderen Zelle:
                    // Da tun wir so, als ob wir eine neue Form aufmachen
                    // und suchen weiter.
                    xCurrentFormComps = aHTMLControls[i]->xFormComps;
                    pCurrentStNd = pCntrlStNd;
                    nCurrentCtrls = aHTMLControls[i]->nCount;
                }
            }
            else
            {
                // Wir befinden uns noch in keiner Form:
                // Da tun wir mal so, als ob wie wir die Form aufmachen.
                xCurrentFormComps = aHTMLControls[i]->xFormComps;
                pCurrentStNd = pCntrlStNd;
                nCurrentCtrls = aHTMLControls[i]->nCount;
            }
        }
        if( !xNewFormComps.is() && xCurrentFormComps.is() &&
            nCurrentCtrls != xCurrentFormComps->getCount() )
        {
            // In der Tablle/dem Bereich sollte eine Form aufgemacht werden,
            // die nicht vollstaendig in der Tabelle liegt. Dan muessen
            // wie die Form jetzt ebenfalls oeffen.
            xNewFormComps = xCurrentFormComps;
        }
    }

    if( xNewFormComps.is() &&
        (!pxFormComps || !(xNewFormComps == *pxFormComps)) )
    {
        // Es soll eine Form aufgemacht werden ...
        if( pxFormComps && pxFormComps->is() )
        {
            // .. es ist aber noch eine Form offen: Das ist in
            // jedem Fall eine Fehler, aber wir schliessen die alte
            // Form trotzdem
            OutForm( sal_False, *pxFormComps );

            //!!!nWarn = 1; // Control wird falscher Form zugeordnet
        }

        if( !pxFormComps )
            pxFormComps = new Reference< container::XIndexContainer > ;
        *pxFormComps = xNewFormComps;

        OutForm( sal_True, *pxFormComps );
        Reference< beans::XPropertySet >  xTmp;
        OutHiddenControls( *pxFormComps, xTmp );
    }
}

void SwHTMLWriter::OutHiddenForms()
{
    // Ohne DrawModel kann es auch keine Controls geben. Dann darf man
    // auch nicht per UNO auf das Dok zugreifen, weil sonst ein DrawModel
    // angelegt wird.
    if( !pDoc->GetDrawModel() )
        return;

    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( !pDocSh )
        return;

    Reference< drawing::XDrawPageSupplier > xDPSupp( pDocSh->GetBaseModel(),
                                                     UNO_QUERY );
    ASSERT( xDPSupp.is(), "XTextDocument nicht vom XModel erhalten" );
    Reference< drawing::XDrawPage > xDrawPage = xDPSupp->getDrawPage();

    ASSERT( xDrawPage.is(), "XDrawPage nicht erhalten" );
    if( !xDrawPage.is() )
        return;

    Reference< form::XFormsSupplier > xFormsSupplier( xDrawPage, UNO_QUERY );
    ASSERT( xFormsSupplier.is(),
            "XFormsSupplier nicht vom XDrawPage erhalten" );

    Reference< container::XNameContainer > xTmp = xFormsSupplier->getForms();
    ASSERT( xTmp.is(), "XForms nicht erhalten" );
    Reference< container::XIndexContainer > xForms( xTmp, UNO_QUERY );
    ASSERT( xForms.is(), "XForms ohne container::XIndexContainer?" );

    sal_Int32 nCount = xForms->getCount();
    for( sal_Int32 i=0; i<nCount; i++)
    {
        Any aTmp = xForms->getByIndex( i );
        ASSERT( aTmp.getValueType() ==
                        ::getCppuType((Reference< form::XForm >*)0),
                "OutHiddenForms: falsche Reflection" );
        if( aTmp.getValueType() ==
                    ::getCppuType((Reference< form::XForm >*)0) )
            OutHiddenForm( *(Reference< form::XForm > *)aTmp.getValue() );
    }
}

void SwHTMLWriter::OutHiddenForm( const Reference< form::XForm > & rForm )
{
    Reference< container::XIndexContainer > xFormComps( rForm, UNO_QUERY );
    if( !xFormComps.is() )
        return;

    sal_Int32 nCount = xFormComps->getCount();
    sal_Bool bHiddenOnly = nCount > 0, bHidden = sal_False;
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        Any aTmp = xFormComps->getByIndex( i );
        ASSERT( aTmp.getValueType() ==
                        ::getCppuType((Reference<form::XFormComponent>*)0),
                "OutHiddenForm: falsche Reflection" );
        if( aTmp.getValueType() !=
                    ::getCppuType((Reference<form::XFormComponent>*)0) )
            continue;

        Reference< form::XFormComponent > xFormComp =
            *(Reference< form::XFormComponent > *)aTmp.getValue();
        Reference< form::XForm > xForm( xFormComp, UNO_QUERY );
        if( xForm.is() )
            OutHiddenForm( xForm );

        if( bHiddenOnly )
        {
            Reference< beans::XPropertySet >  xPropSet( xFormComp, UNO_QUERY );
            OUString sPropName = OUString::createFromAscii( "ClassId" );
            if( xPropSet->getPropertySetInfo()->hasPropertyByName( sPropName ) )
            {
                Any aTmp = xPropSet->getPropertyValue( sPropName );
                if( aTmp.getValueType() == ::getCppuType((sal_Int16*)0) )
                {
                    if( form::FormComponentType::HIDDENCONTROL ==
                                                *(sal_Int16*)aTmp.getValue() )
                        bHidden = sal_True;
                    else if( lcl_html_isHTMLControl(
                                            *(sal_Int16*)aTmp.getValue() ) )
                        bHiddenOnly = sal_False;
                }
            }
        }
    }

    if( bHidden && bHiddenOnly )
    {
        OutForm( sal_True, xFormComps );
        Reference< beans::XPropertySet > xTmp;
        OutHiddenControls( xFormComps, xTmp );
        OutForm( sal_False, xFormComps );
    }
}

void SwHTMLWriter::OutForm( sal_Bool bOn,
                const Reference< container::XIndexContainer > & rFormComps )
{
    nFormCntrlCnt = 0;

    if( !bOn )
    {
        DecIndentLevel(); // Inhalt der Form einruecken
        if( bLFPossible )
            OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_form, sal_False );
        bLFPossible = sal_True;

        return;
    }

    // die neue Form wird geoeffnet
    if( bLFPossible )
        OutNewLine();
    ByteString sOut( '<' );
    sOut += sHTML_form;

    Reference< beans::XPropertySet > xFormPropSet( rFormComps, UNO_QUERY );

    Any aTmp = xFormPropSet->getPropertyValue(
                                    OUString::createFromAscii( "Name" ) );
    if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
        ((OUString*)aTmp.getValue())->getLength() )
    {
        ((sOut += ' ') += sHTML_O_name) += "=\"";
        Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( Strm(), *(OUString*)aTmp.getValue(),
                                  eDestEnc );
        sOut = '\"';
    }

    aTmp = xFormPropSet->getPropertyValue(
                    OUString::createFromAscii( "TargetURL" ) );
    if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
        ((OUString*)aTmp.getValue())->getLength() )
    {
        ((sOut += ' ') += sHTML_O_action) += "=\"";
        Strm() << sOut.GetBuffer();
        String aURL( *(OUString*)aTmp.getValue() );
        aURL = INetURLObject::AbsToRel( aURL, INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_UNAMBIGUOUS);
        HTMLOutFuncs::Out_String( Strm(), aURL, eDestEnc );
        sOut = '\"';
    }

    aTmp = xFormPropSet->getPropertyValue(
                    OUString::createFromAscii( "SubmitMethod" ) );
    if( aTmp.getValueType() == ::getCppuType((const form::FormSubmitMethod*)0) )
    {
         form::FormSubmitMethod eMethod =
                *( form::FormSubmitMethod*)aTmp.getValue();
        if( form::FormSubmitMethod_POST==eMethod )
        {
            ((((sOut += ' ')
                += sHTML_O_method) += "=\"")
                += sHTML_METHOD_post) += '\"';
        }
    }
    aTmp = xFormPropSet->getPropertyValue(
                    OUString::createFromAscii( "SubmitEncoding" ) );
    if( aTmp.getValueType()==::getCppuType((const form::FormSubmitEncoding*)0) )
    {
         form::FormSubmitEncoding eEncType =
                    *( form::FormSubmitEncoding*)aTmp.getValue();
        const sal_Char *pStr = 0;
        switch( eEncType )
        {
        case form::FormSubmitEncoding_MULTIPART:
            pStr = sHTML_ET_multipart;
            break;
        case form::FormSubmitEncoding_TEXT:
            pStr = sHTML_ET_text;
            break;
        }

        if( pStr )
        {
            ((((sOut += ' ')
                += sHTML_O_enctype) += "=\"")
                += pStr) += '\"';
        }
    }

    aTmp = xFormPropSet->getPropertyValue(
                        OUString::createFromAscii( "TargetFrame" ) );
    if( aTmp.getValueType() == ::getCppuType((const OUString*)0)&&
        ((OUString*)aTmp.getValue())->getLength() )
    {
        ((sOut += ' ') += sHTML_O_target) += "=\"";
        Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( Strm(), *(OUString*)aTmp.getValue(),
                                  eDestEnc );
        sOut = '\"';
    }

    Strm() << sOut.GetBuffer();
    Reference< form::XFormComponent > xFormComp( rFormComps, UNO_QUERY );
    lcl_html_outEvents( Strm(), xFormComp, bCfgStarBasic, eDestEnc );
    Strm() << '>';

    IncIndentLevel(); // Inhalt der Form einruecken
    bLFPossible = sal_True;
}

void SwHTMLWriter::OutHiddenControls(
        const Reference< container::XIndexContainer > & rFormComps,
        const Reference< beans::XPropertySet > & rPropSet )
{
    sal_Int32 nCount = rFormComps->getCount();
    sal_Int32 nPos = 0;
    sal_Bool bDone = sal_False;
    if( rPropSet.is() )
    {
        Reference< form::XFormComponent > xFC( rPropSet, UNO_QUERY );
        for( nPos=0; !bDone && nPos < nCount; nPos++ )
        {
            Any aTmp = rFormComps->getByIndex( nPos );
            ASSERT( aTmp.getValueType() ==
                        ::getCppuType((Reference< form::XFormComponent>*)0),
                    "OutHiddenControls: falsche Reflection" );
            bDone = aTmp.getValueType() ==
                        ::getCppuType((Reference< form::XFormComponent>*)0) &&
                    *(Reference< form::XFormComponent > *)aTmp.getValue() ==
                        xFC;
        }
    }

    for( ; nPos < nCount; nPos++ )
    {
        Any aTmp = rFormComps->getByIndex( nPos );
        ASSERT( aTmp.getValueType() ==
                        ::getCppuType((Reference< form::XFormComponent>*)0),
                "OutHiddenControls: falsche Reflection" );
        if( aTmp.getValueType() !=
                    ::getCppuType((Reference< form::XFormComponent>*)0) )
            continue;
        Reference< form::XFormComponent > xFC =
                *(Reference< form::XFormComponent > *)aTmp.getValue();
        Reference< beans::XPropertySet > xPropSet( xFC, UNO_QUERY );

        OUString sPropName = OUString::createFromAscii( "ClassId" );
        if( !xPropSet->getPropertySetInfo()->hasPropertyByName( sPropName ) )
            continue;

        aTmp = xPropSet->getPropertyValue( sPropName );
        if( aTmp.getValueType() != ::getCppuType((const sal_Int16*)0) )
            continue;

        if( form::FormComponentType::HIDDENCONTROL ==
                                            *(sal_Int16*) aTmp.getValue() )
        {
            if( bLFPossible )
                OutNewLine( sal_True );
            ByteString sOut( '<' );
            ((((sOut += sHTML_input) += ' ') +=
                sHTML_O_type) += '=') += sHTML_IT_hidden;

            aTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "Name" ) );
            if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
                ((OUString*)aTmp.getValue())->getLength() )
            {
                (( sOut += ' ' ) += sHTML_O_name ) += "=\"";
                Strm() << sOut.GetBuffer();
                HTMLOutFuncs::Out_String( Strm(), *(OUString*)aTmp.getValue(),
                                          eDestEnc );
                sOut = '\"';
            }
            aTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "HiddenValue" ) );
            if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
                ((OUString*)aTmp.getValue())->getLength() )
            {
                ((sOut += ' ') += sHTML_O_value) += "=\"";
                Strm() << sOut.GetBuffer();
                HTMLOutFuncs::Out_String( Strm(), *(OUString*)aTmp.getValue(),
                                          eDestEnc );
                sOut = '\"';
            }
            sOut += '>';
            Strm() << sOut.GetBuffer();

            nFormCntrlCnt++;
        }
        else if( lcl_html_isHTMLControl( *(sal_Int16*) aTmp.getValue() ) )
        {
            break;
        }
    }
}

/*  */

// hier folgen die Ausgabe-Routinen, dadurch sind die form::Forms gebuendelt:

const SdrObject *SwHTMLWriter::GetHTMLControl( const SwDrawFrmFmt& rFmt )
{
    // es muss ein Draw-Format sein
    ASSERT( RES_DRAWFRMFMT == rFmt.Which(),
            "GetHTMLControl nuer fuer Draw-Formate erlaubt" );

    // Schauen, ob es ein SdrObject dafuer gibt
    const SdrObject *pObj = rFmt.FindSdrObject();
    if( !pObj || FmFormInventor != pObj->GetObjInventor() )
        return 0;

    SdrUnoObj *pFormObj = PTR_CAST( SdrUnoObj, pObj );
    Reference< awt::XControlModel >  xControlModel =
            pFormObj->GetUnoControlModel();

    ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
    if( !xControlModel.is() )
        return 0;

    Reference< beans::XPropertySet >  xPropSet( xControlModel, UNO_QUERY );

    OUString sPropName = OUString::createFromAscii( "ClassId" );
    if( !xPropSet->getPropertySetInfo()->hasPropertyByName( sPropName ) )
        return 0;

    Any aTmp = xPropSet->getPropertyValue( sPropName );
    if( aTmp.getValueType() == ::getCppuType((const sal_Int16*)0)&&
        lcl_html_isHTMLControl( *(sal_Int16*) aTmp.getValue() ) )
    {
        return pObj;
    }

    return 0;
}

static void GetControlSize( const SdrObject& rSdrObj, Size& rSz,
                            SwDoc *pDoc )
{
    ViewShell *pVSh = 0;
    pDoc->GetEditShell( &pVSh );
    if( !pVSh )
        return;

    SdrUnoObj *pFormObj = PTR_CAST( SdrUnoObj, &rSdrObj );
    Reference< awt::XControl >  xControl =
            pFormObj->GetUnoControl( pVSh->GetWin() );
    Reference< awt::XTextLayoutConstrains > xLC( xControl, UNO_QUERY );
    ASSERT( xLC.is(), "kein XTextLayoutConstrains" );
    if( !xLC.is() )
        return;

    sal_Int16 nCols=0, nLines=0;
    xLC->getColumnsAndLines( nCols, nLines );
    rSz.Width() = nCols;
    rSz.Height() = nLines;
}

Writer& OutHTML_DrawFrmFmtAsControl( Writer& rWrt,
                                     const SwDrawFrmFmt& rFmt,
                                     const SdrObject& rSdrObject,
                                     sal_Bool bInCntnr )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    SdrUnoObj *pFormObj = PTR_CAST( SdrUnoObj, &rSdrObject );
    Reference< awt::XControlModel > xControlModel =
        pFormObj->GetUnoControlModel();

    ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
    if( !xControlModel.is() )
        return rWrt;

    Reference< beans::XPropertySet > xPropSet( xControlModel, UNO_QUERY );

//!!!   if( rHTMLWrt.pForm != pVCSbxCtrl->GetVCForm() )
//!!!       rHTMLWrt.nWarn = 1; // Control wird falscher Form zugeordnet
    rHTMLWrt.nFormCntrlCnt++;

    const sal_Char *pTag = sHTML_input, *pType = 0;
    OUString sValue;
    ByteString sOptions;
    sal_Bool bEmptyValue = sal_False;
    Any aTmp = xPropSet->getPropertyValue(
                    OUString::createFromAscii( "ClassId" ) );
    sal_Int16 nClassId = *(sal_Int16*) aTmp.getValue();
    sal_uInt32 nFrmOpts = HTML_FRMOPTS_CONTROL;
    switch( nClassId )
    {
    case form::FormComponentType::CHECKBOX:
    case form::FormComponentType::RADIOBUTTON:
        pType = (form::FormComponentType::CHECKBOX == nClassId
                    ? sHTML_IT_checkbox
                    : sHTML_IT_radio);
        aTmp = xPropSet->getPropertyValue(
                        OUString::createFromAscii( "DefaultState" ) );
        if( aTmp.getValueType() == ::getCppuType((const sal_Int16*)0) &&
            STATE_NOCHECK != *(sal_Int16*) aTmp.getValue() )
        {
            (sOptions += ' ') += sHTML_O_checked;
        }

        aTmp = xPropSet->getPropertyValue(
                        OUString::createFromAscii( "RefValue" ) );
        if( aTmp.getValueType() == ::getCppuType((const OUString*)0) )

        {
            const OUString& rVal = *(OUString*)aTmp.getValue();
            if( !rVal.len() )
                bEmptyValue = sal_True;
            else if( rVal.compareToAscii( sHTML_on ) != 0 )
                sValue = rVal;
        }
        break;

    case form::FormComponentType::COMMANDBUTTON:
        {
            form::FormButtonType eButtonType = form::FormButtonType_PUSH;
            aTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "ButtonType" ) );
            if( aTmp.getValueType() ==
                            ::getCppuType((const form::FormButtonType*)0) )
                eButtonType = *( form::FormButtonType*)aTmp.getValue();

            switch( eButtonType )
            {
            case form::FormButtonType_RESET:
                pType = sHTML_IT_reset;
                break;
            case form::FormButtonType_SUBMIT:
                pType = sHTML_IT_submit;
                break;
            case form::FormButtonType_PUSH:
            default:
                pType = sHTML_IT_button;
            }

            aTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "Label" ) );
            if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
                ((OUString*)aTmp.getValue())->getLength() )
            {
                sValue = *(OUString*)aTmp.getValue();
            }
        }
        break;

    case form::FormComponentType::LISTBOX:
        if( rHTMLWrt.bLFPossible )
            rHTMLWrt.OutNewLine( sal_True );
        pTag = sHTML_select;
        aTmp = xPropSet->getPropertyValue(
                        OUString::createFromAscii( "Dropdown" ) );
        if( aTmp.getValueType() == ::getBooleanCppuType() &&
            !*(sal_Bool*)aTmp.getValue() )
        {
            Size aSz( 0, 0 );
            GetControlSize( rSdrObject, aSz, rWrt.pDoc );

            // wieviele sind sichtbar ??
            if( aSz.Height() )
                (((sOptions += ' ' ) += sHTML_O_size ) += '=' )
                    += ByteString::CreateFromInt32( aSz.Height() );

            aTmp = xPropSet->getPropertyValue(
                        OUString::createFromAscii( "MultiSelection" ) );
            if( aTmp.getValueType() == ::getBooleanCppuType() &&
                *(sal_Bool*)aTmp.getValue() )
            {
                (sOptions += ' ' ) += sHTML_O_multiple;
            }
        }
        break;

    case form::FormComponentType::TEXTFIELD:
        {
            Size aSz( 0, 0 );
            GetControlSize( rSdrObject, aSz, rWrt.pDoc );

            sal_Bool bMultiLine = sal_False;
            aTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "MultiLine" ) );
            bMultiLine = aTmp.getValueType() == ::getBooleanCppuType() &&
                         *(sal_Bool*)aTmp.getValue();

            if( bMultiLine )
            {
                if( rHTMLWrt.bLFPossible )
                    rHTMLWrt.OutNewLine( sal_True );
                pTag = sHTML_textarea;

                if( aSz.Height() )
                    (((sOptions += ' ' ) += sHTML_O_rows ) += '=' )
                        += ByteString::CreateFromInt32( aSz.Height() );
                if( aSz.Width() )
                    (((sOptions += ' ' ) += sHTML_O_cols ) += '=' )
                        += ByteString::CreateFromInt32( aSz.Width() );

                aTmp = xPropSet->getPropertyValue(
                                OUString::createFromAscii( "HScroll" ) );
                if( aTmp.getValueType() == ::getVoidCppuType() ||
                    (aTmp.getValueType() == ::getBooleanCppuType() &&
                    !*(sal_Bool*)aTmp.getValue()) )
                {
                    const sal_Char *pWrapStr = 0;
                    aTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "HardLineBreaks" ) );
                    pWrapStr =
                        (aTmp.getValueType() == ::getBooleanCppuType() &&
                        *(sal_Bool*)aTmp.getValue()) ? sHTML_WW_hard
                                                     : sHTML_WW_soft;
                    (((sOptions += ' ') += sHTML_O_wrap) += '=') += pWrapStr;
                }
            }
            else
            {
                aTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "EchoChar" ) );
                if( aTmp.getValueType() == ::getCppuType((const sal_Int16*)0) &&
                    *(sal_Int16*)aTmp.getValue() != 0 )
                    pType = sHTML_IT_password;
                else
                    pType = sHTML_IT_text;

                if( aSz.Width() )
                    (((sOptions += ' ' ) += sHTML_O_size ) += '=' )
                        += ByteString::CreateFromInt32( aSz.Width() );

                aTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "MaxTextLen" ) );
                if( aTmp.getValueType() == ::getCppuType((const sal_Int16*)0) &&
                    *(sal_Int16*) aTmp.getValue() != 0 )
                {
                    (((sOptions += ' ' ) += sHTML_O_maxlength ) += '=' )
                        += ByteString::CreateFromInt32(
                                *(sal_Int16*) aTmp.getValue() );
                }

                aTmp = xPropSet->getPropertyValue(
                                OUString::createFromAscii( "DefaultText" ) );
                if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
                    ((OUString*)aTmp.getValue())->getLength() )
                {
                    sValue = *(OUString*)aTmp.getValue();
                }
            }
        }
        break;

    case form::FormComponentType::FILECONTROL:
        {
            Size aSz( 0, 0 );
            GetControlSize( rSdrObject, aSz, rWrt.pDoc );
            pType = sHTML_IT_file;

            if( aSz.Width() )
                (((sOptions += ' ' ) += sHTML_O_size ) += '=' )
                    += ByteString::CreateFromInt32( aSz.Width() );

            // VALUE vim form aus Sicherheitsgruenden nicht exportieren
        }
        break;


    case form::FormComponentType::IMAGEBUTTON:
        pType = sHTML_IT_image;
        nFrmOpts = HTML_FRMOPTS_IMG_CONTROL;
        break;

#if 0
    case FORMCOMPONENT_URLBUTTON:
        {
            // in einen Hyperlink wandeln
            ((((sOut = '<') += sHTML_anchor) += ' ' ) += sHTML_O_href )
                += "=\"";
            rWrt.Strm() << sOut.GetStr();
            HTMLOutFuncs::Out_String( rWrt.Strm(),
                INetURLObject::AbsToRel( ((VCURLButton*)pVCSbxCtrl)->GetURL(),
                                        INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_UNAMBIGUOUS),
                rHTMLWrt.eDestEnc, rHTMLWrt.eDestEnc );
            sOut = '\"';
            const String& rTarget =
                ((VCURLButton*)pVCSbxCtrl)->GetTargetFrame();
            if( rTarget.Len() )
            {
                ((sOut += ' ') += sHTML_O_target) += "=\"";
                rWrt.Strm() << sOut.GetStr();
                HTMLOutFuncs::Out_String( rWrt.Strm(), rTarget,
                rHTMLWrt.eDestEnc, rHTMLWrt.eDestEnc );
                sOut = '\"';
            }
            sOut += ">";
            rWrt.Strm() << sOut.GetStr();

            HTMLOutFuncs::Out_String( rWrt.Strm(),
                                      ((VCURLButton*)pVCSbxCtrl)->GetText(),
                rHTMLWrt.eDestEnc, rHTMLWrt.eDestEnc );
            HTMLOutFuncs::Out_Tag( rWrt.Strm(), sHTML_anchor, sal_False ) << ' ';
        }
        break;
#endif
    default:                // kennt HTML nicht
        pTag = 0;        // also ueberspringen
        break;
    }

    if( !pTag )
        return rWrt;

    ByteString sOut( '<' );
    sOut += pTag;
    if( pType )
        (((sOut += ' ') += sHTML_O_type) += '=') += pType;

    aTmp = xPropSet->getPropertyValue( OUString::createFromAscii( "Name" ) );
    if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
        ((OUString*)aTmp.getValue())->getLength() )
    {
        (( sOut += ' ' ) += sHTML_O_name ) += "=\"";
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rWrt.Strm(), *(OUString*)aTmp.getValue(),
                                  rHTMLWrt.eDestEnc );
        sOut = '\"';
    }

    aTmp = xPropSet->getPropertyValue( OUString::createFromAscii( "Enabled" ) );
    if( aTmp.getValueType() == ::getBooleanCppuType() &&
        !*(sal_Bool*)aTmp.getValue() )
    {
        (( sOut += ' ' ) += sHTML_O_disabled );
    }

    if( sValue.getLength() || bEmptyValue )
    {
        ((sOut += ' ') += sHTML_O_value) += "=\"";
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rWrt.Strm(), sValue, rHTMLWrt.eDestEnc );
        sOut = '\"';
    }

    sOut += sOptions;

    if( sHTML_IT_image == pType )
    {
        aTmp = xPropSet->getPropertyValue(
                    OUString::createFromAscii( "ImageURL" ) );
        if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
            ((OUString*)aTmp.getValue())->getLength() )
        {
            ((sOut += ' ') += sHTML_O_src) += "=\"";
            rWrt.Strm() << sOut.GetBuffer();

            HTMLOutFuncs::Out_String( rWrt.Strm(),
                        INetURLObject::AbsToRel( *(OUString*)aTmp.getValue(),
                                        INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_UNAMBIGUOUS),
                        rHTMLWrt.eDestEnc );
            sOut = '\"';
        }

        Size aTwipSz( rSdrObject.GetLogicRect().GetSize() );
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
            (((sOut += ' ') += sHTML_O_width) += '=')
                += ByteString::CreateFromInt32( aPixelSz.Width() );

        if( aPixelSz.Height() )
            (((sOut += ' ') += sHTML_O_height) += '=')
                += ByteString::CreateFromInt32( aPixelSz.Height() );
    }

    aTmp = xPropSet->getPropertyValue(
                    OUString::createFromAscii( "TabIndex" ) );
    if( aTmp.getValueType() == ::getCppuType((const sal_Int16*)0) )
    {
        sal_Int16 nTabIndex = *(sal_Int16*) aTmp.getValue();
        if( nTabIndex > 0 )
        {
            if( nTabIndex >= 32767 )
                nTabIndex = 32767;

            (((sOut += ' ') += sHTML_O_tabindex) += '=')
                += ByteString::CreateFromInt32( nTabIndex );
        }
    }

    if( sOut.Len() )
    {
        rWrt.Strm() << sOut.GetBuffer();
        sOut.Erase();
    }

    ASSERT( !bInCntnr, "Container wird fuer Controls nicht unterstuertzt" );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_DRAW ) && !bInCntnr )
    {
        // Wenn Zeichen-Objekte nicht absolut positioniert werden duerfen,
        // das entsprechende Flag loeschen.
        nFrmOpts |= (sHTML_IT_image == pType
                            ? HTML_FRMOPTS_IMG_CONTROL_CSS1
                            : HTML_FRMOPTS_CONTROL_CSS1);
    }
    ByteString aEndTags;
    if( nFrmOpts != 0 )
        rHTMLWrt.OutFrmFmtOptions( rFmt, aEmptyStr, aEndTags, nFrmOpts );

    if( rHTMLWrt.bCfgOutStyles )
    {
        sal_Bool bEdit = sHTML_textarea == pTag || sHTML_IT_file == pType ||
                     sHTML_IT_text == pType;

        Reference< beans::XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
        SfxItemSet aItemSet( rHTMLWrt.pDoc->GetAttrPool(), RES_CHRATR_BEGIN,
                             RES_CHRATR_END );
        OUString sPropName = OUString::createFromAscii( "BackgroundColor" );
        if( xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp = xPropSet->getPropertyValue( sPropName );
            if( aTmp.getValueType() == ::getCppuType((const sal_Int32*)0) )
            {
                Color aCol(*(sal_Int32*)aTmp .getValue());
                aItemSet.Put( SvxBrushItem( aCol, RES_CHRATR_BACKGROUND ) );
            }
        }
        sPropName = OUString::createFromAscii( "TextColor" );
        if( xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp = xPropSet->getPropertyValue( sPropName );
            if( aTmp.getValueType() == ::getCppuType((const sal_Int32*)0) )
            {
                Color aColor( *(sal_Int32*)aTmp .getValue() );
                aItemSet.Put( SvxColorItem( aColor ) );
            }
        }
        sPropName = OUString::createFromAscii( "FontHeight" );
        if( xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp = xPropSet->getPropertyValue( sPropName );
            if( aTmp.getValueType() == ::getCppuType((const float*)0) )

            {
                float nHeight = *(float*)aTmp.getValue();
                if( nHeight > 0  && (!bEdit || nHeight != 10.) )
                    aItemSet.Put( SvxFontHeightItem( sal_Int16(nHeight * 20.) ) );
            }
        }
        sPropName = OUString::createFromAscii( "FontName" );
        if( xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp = xPropSet->getPropertyValue( sPropName );
            if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
                ((OUString*)aTmp.getValue())->getLength() )
            {
                Font aFixedFont( System::GetStandardFont( STDFONT_FIXED ) );
                String aFName( *(OUString*)aTmp.getValue() );
                if( !bEdit || aFName != aFixedFont.GetName() )
                {
                    FontFamily eFamily = FAMILY_DONTKNOW;
                    sPropName = OUString::createFromAscii( "FontFamily" );
                    if( xPropSetInfo->hasPropertyByName( sPropName ) )
                    {
                        aTmp = xPropSet->getPropertyValue( sPropName );
                        if( aTmp.getValueType() == ::getCppuType((const sal_Int16*)0))
                            eFamily = (FontFamily)*(sal_Int16*) aTmp.getValue();
                    }
                    SvxFontItem aItem( eFamily, aFName, aEmptyStr );
                    aItemSet.Put( aItem );
                }
            }
        }
        sPropName = OUString::createFromAscii( "FontWeight" );
        if( xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp = xPropSet->getPropertyValue( sPropName );
            if( aTmp.getValueType() == ::getCppuType((const float*)0) )
            {
                FontWeight eWeight =
                    VCLUnoHelper::ConvertFontWeight( *(float*)aTmp.getValue() );
                if( eWeight != WEIGHT_DONTKNOW && eWeight != WEIGHT_NORMAL )
                    aItemSet.Put( SvxWeightItem( eWeight ) );
            }
        }
        sPropName = OUString::createFromAscii( "FontSlant" );
        if( xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp = xPropSet->getPropertyValue( sPropName );
            if( aTmp.getValueType() == ::getCppuType((const sal_Int16*)0))
            {
                FontItalic eItalic = (FontItalic)*(sal_Int16*)aTmp.getValue();
                if( eItalic != ITALIC_DONTKNOW && eItalic != ITALIC_NONE )
                    aItemSet.Put( SvxPostureItem( eItalic ) );
            }
        }
        sPropName = OUString::createFromAscii( "FontUnderline" );
        if( xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp = xPropSet->getPropertyValue( sPropName );
            if( aTmp.getValueType() == ::getCppuType((const sal_Int16*)0) )
            {
                FontUnderline eUnderline =
                    (FontUnderline)*(sal_Int16*)aTmp.getValue();
                if( eUnderline != UNDERLINE_DONTKNOW  &&
                    eUnderline != UNDERLINE_NONE )
                    aItemSet.Put( SvxUnderlineItem( eUnderline ) );
            }
        }
        sPropName = OUString::createFromAscii( "FontStrikeout" );
        if( xPropSetInfo->hasPropertyByName( sPropName ) )
        {
            aTmp = xPropSet->getPropertyValue( sPropName );
            if( aTmp.getValueType() == ::getCppuType((const sal_Int16*)0))
            {
                FontStrikeout eStrikeout =
                    (FontStrikeout)*(sal_Int16*)aTmp.getValue();
                if( eStrikeout != STRIKEOUT_DONTKNOW &&
                    eStrikeout != STRIKEOUT_NONE )
                    aItemSet.Put( SvxCrossedOutItem( eStrikeout ) );
            }
        }

        rHTMLWrt.OutCSS1_FrmFmtOptions( rFmt, nFrmOpts, &rSdrObject,
                                        &aItemSet );
    }

    Reference< form::XFormComponent >  xFormComp( xControlModel, UNO_QUERY );
    lcl_html_outEvents( rWrt.Strm(), xFormComp, rHTMLWrt.bCfgStarBasic,
                        rHTMLWrt.eDestEnc );

    rWrt.Strm() << '>';

    if( sHTML_select == pTag )
    {
        aTmp = xPropSet->getPropertyValue(
                    OUString::createFromAscii( "StringItemList" ) );
        if( aTmp.getValueType() == ::getCppuType((Sequence<OUString>*)0) )
        {
            rHTMLWrt.IncIndentLevel(); // der Inhalt von Select darf
                                       // eingerueckt werden
            Sequence<OUString> aList( *(Sequence<OUString>*)aTmp.getValue() );
            sal_Int32 nCnt = aList.getLength();
            const OUString *pStrings = aList.getConstArray();

            const OUString *pValues = 0;
            sal_Int32 nValCnt = 0;
            aTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "ListSource" ) );
            Sequence<OUString> aValList;
            if( aTmp.getValueType() == ::getCppuType((Sequence<OUString>*)0) )
            {
                aValList = *(Sequence<OUString>*)aTmp.getValue();
                nValCnt = aValList.getLength();
                pValues = aValList.getConstArray();
            }

            Any aSelTmp = xPropSet->getPropertyValue(
                            OUString::createFromAscii( "DefaultSelection" ) );
            const sal_Int16 *pSels = 0;
            sal_Int32 nSel = 0;
            sal_Int32 nSelCnt = 0;
            Sequence<sal_Int16> aSelList;
            if( aSelTmp.getValueType() ==::getCppuType((Sequence<sal_Int16>*)0))
            {
                aSelList = *(Sequence<sal_Int16>*)aSelTmp.getValue();
                nSelCnt = aSelList.getLength();
                pSels = aSelList.getConstArray();
            }

            for( sal_Int32 i = 0; i < nCnt; i++ )
            {
                OUString sValue;
                sal_Bool bSelected = sal_False, bEmptyVal = sal_False;
                if( i < nValCnt )
                {
                    const OUString& rVal = pValues[i];
                    if( rVal.compareToAscii( "$$$empty$$$" ) == 0 )
                        bEmptyVal = sal_True;
                    else
                        sValue = rVal;
                }

                bSelected = (nSel < nSelCnt) && pSels[nSel] == i;
                if( bSelected )
                    nSel++;

                rHTMLWrt.OutNewLine(); // jede Option bekommt eine eigene Zeile
                (sOut = '<') += sHTML_option;
                if( sValue.getLength() || bEmptyVal )
                {
                    ((sOut += ' ') += sHTML_O_value) += "=\"";
                    rWrt.Strm() << sOut.GetBuffer();
                    HTMLOutFuncs::Out_String( rWrt.Strm(), sValue,
                        rHTMLWrt.eDestEnc );
                    sOut = '\"';
                }
                if( bSelected )
                    (sOut += ' ') += sHTML_O_selected;

                sOut += '>';
                rWrt.Strm() << sOut.GetBuffer();

                HTMLOutFuncs::Out_String( rWrt.Strm(), pStrings[i],
                                          rHTMLWrt.eDestEnc );
            }
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_option, sal_False );

            rHTMLWrt.DecIndentLevel();
            rHTMLWrt.OutNewLine();// das </SELECT> bekommt eine eigene Zeile
        }
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_select, sal_False );
    }
    else if( sHTML_textarea == pTag )
    {
        // In TextAreas duerfen keine zusaetzlichen Spaces oder LF exportiert
        // werden!
        String sValue;
        aTmp = xPropSet->getPropertyValue(
                        OUString::createFromAscii( "DefaultText" ) );
        if( aTmp.getValueType() == ::getCppuType((const OUString*)0)&&
            ((OUString*)aTmp.getValue())->getLength() )
        {
            sValue = String( *(OUString*)aTmp.getValue() );
        }
        if( sValue.Len() )
        {
            sValue.ConvertLineEnd( LINEEND_LF );
            xub_StrLen nPos = 0;
            while ( nPos != STRING_NOTFOUND )
            {
                if( nPos )
                    rWrt.Strm() << SwHTMLWriter::sNewLine;
                String aLine = sValue.GetToken( 0, 0x0A, nPos );
                HTMLOutFuncs::Out_String( rWrt.Strm(), aLine,
                                        rHTMLWrt.eDestEnc );
            }
        }
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_textarea, sal_False );
    }
    else if( sHTML_IT_checkbox == pType || sHTML_IT_radio == pType )
    {
        aTmp = xPropSet->getPropertyValue( OUString::createFromAscii("Label") );
        if( aTmp.getValueType() == ::getCppuType((const OUString*)0) &&
            ((OUString*)aTmp.getValue())->getLength() )
        {
            sValue = *(OUString*)aTmp.getValue();
            HTMLOutFuncs::Out_String( rWrt.Strm(), sValue,
                rHTMLWrt.eDestEnc ) << ' ';
        }
    }

    if( aEndTags.Len() )
        rWrt.Strm() << aEndTags.GetBuffer();

    // Controls sind nicht absatz-gebunden, deshalb kein LF mehr ausgeben!
    rHTMLWrt.bLFPossible = sal_False;

    if( rHTMLWrt.pxFormComps && rHTMLWrt.pxFormComps->is() )
        rHTMLWrt.OutHiddenControls( *rHTMLWrt.pxFormComps, xPropSet );
    return rWrt;
}

/*  */

// Ermitteln, ob eine Format zu einem Control gehoert und wenn ja
// dessen Form zurueckgeben
static void AddControl( HTMLControls& rControls,
                        const SdrObject *pSdrObj,
                        sal_uInt32 nNodeIdx )
{
    SdrUnoObj *pFormObj = PTR_CAST( SdrUnoObj, pSdrObj );
    ASSERT( pFormObj, "Doch kein FormObj" );
    Reference< awt::XControlModel > xControlModel =
            pFormObj->GetUnoControlModel();
    if( !xControlModel.is() )
        return;

    Reference< form::XFormComponent >  xFormComp( xControlModel, UNO_QUERY );
    Reference< XInterface >  xIfc = xFormComp->getParent();
    Reference< form::XForm >  xForm(xIfc, UNO_QUERY);

    ASSERT( xForm.is(), "Wo ist die Form?" );
    if( xForm.is() )
    {
        Reference< container::XIndexContainer >  xFormComps( xForm, UNO_QUERY );
        HTMLControl *pHCntrl = new HTMLControl( xFormComps, nNodeIdx );
        if( !rControls.C40_PTR_INSERT( HTMLControl, pHCntrl ) )
        {
            sal_uInt16 nPos = 0;
            if( rControls.Seek_Entry(pHCntrl,&nPos) &&
                rControls[nPos]->xFormComps==xFormComps )
                rControls[nPos]->nCount++;
            delete pHCntrl;
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
    sal_uInt16 i;
    if( pHTMLPosFlyFrms )
    {
        // die absatz-gebundenen Controls einsammeln
        for( i=0; i<pHTMLPosFlyFrms->Count(); i++ )
        {
            const SwHTMLPosFlyFrm* pPosFlyFrm = pHTMLPosFlyFrms->GetObject( i );
            if( HTML_OUT_CONTROL != pPosFlyFrm->GetOutFn() )
                continue;

            const SdrObject *pSdrObj = pPosFlyFrm->GetSdrObject();
            ASSERT( pSdrObj, "Wo ist das SdrObject?" );
            if( !pSdrObj )
                continue;

            AddControl( aHTMLControls, pSdrObj,
                        pPosFlyFrm->GetNdIndex().GetIndex() );
        }
    }

    // und jetzt die in einem zeichengebundenen Rahmen
    const SwSpzFrmFmts* pSpzFrmFmts = pDoc->GetSpzFrmFmts();
    for( i=0; i<pSpzFrmFmts->Count(); i++ )
    {
        const SwFrmFmt *pFrmFmt = (*pSpzFrmFmts)[i];
        if( RES_DRAWFRMFMT != pFrmFmt->Which() )
            continue;

        const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
        const SwPosition *pPos = rAnchor.GetCntntAnchor();
        if( FLY_IN_CNTNT != rAnchor.GetAnchorId() || !pPos )
            continue;

        const SdrObject *pSdrObj =
            SwHTMLWriter::GetHTMLControl( *(const SwDrawFrmFmt*)pFrmFmt );
        if( !pSdrObj )
            continue;

        AddControl( aHTMLControls, pSdrObj, pPos->nNode.GetIndex() );
    }
}

/*  */

HTMLControl::HTMLControl(
        const Reference< container::XIndexContainer > & rFormComps,
        sal_uInt32 nIdx ) :
    xFormComps( rFormComps ), nNdIdx( nIdx ), nCount( 1 )
{}


HTMLControl::~HTMLControl()
{}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmlforw.cxx,v 1.3 2001-03-23 12:39:50 jl Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/10/20 13:43:01  jp
      use correct INetURL-Decode enum

      Revision 1.1.1.1  2000/09/18 17:14:55  hr
      initial import

      Revision 1.77  2000/09/18 16:04:45  willem.vandorp
      OpenOffice header added.

      Revision 1.76  2000/08/18 13:03:18  jp
      don't export escaped URLs

      Revision 1.75  2000/06/26 09:52:10  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.74  2000/04/28 14:29:11  mib
      unicode

      Revision 1.73  2000/04/11 10:46:30  mib
      unicode

      Revision 1.72  2000/03/31 06:16:25  os
      UNO III: toolkit includes

      Revision 1.71  2000/03/30 07:31:01  os
      UNO III

      Revision 1.70  2000/03/21 15:06:17  os
      UNOIII

      Revision 1.69  2000/03/03 15:21:00  os
      StarView remainders removed

      Revision 1.68  2000/02/14 13:59:45  mib
      #70473#: unicode

      Revision 1.67  2000/02/10 09:51:54  mib
      #68571#: text/plain encoding for submit

      Revision 1.66  1999/11/22 14:25:04  os
      headers added

      Revision 1.65  1999/11/19 16:40:20  os
      modules renamed

*************************************************************************/

