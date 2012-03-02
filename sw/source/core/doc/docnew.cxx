/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#define ROLBCK_HISTORY_ONLY     // The fight against the CLOOK's
#include <doc.hxx>
#include <dcontact.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/text/XFlatParagraphIteratorProvider.hpp>

#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <rtl/logfile.hxx>
#include <rtl/random.h>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>

#include <svl/macitem.hxx>
#include <svx/svxids.hrc>
#include <svx/svdogrp.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svl/zforlist.hxx>
#include <unotools/compatibility.hxx>
#include <unotools/lingucfg.hxx>
#include <svx/svdpage.hxx>
#include <paratr.hxx>
#include <fchrfmt.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtfordr.hxx>
#include <fmtpdsc.hxx>
#include <pvprtdat.hxx>
#include <rootfrm.hxx>  // So that the RootDtor is being called
#include <layouter.hxx>
#include <pagedesc.hxx> // So that the PageDescs can be destroyed
#include <ndtxt.hxx>
#include <printdata.hxx>
#include <docfld.hxx>
#include <ftninfo.hxx>
#include <ftnidx.hxx>
#include <docstat.hxx>
#include <charfmt.hxx>
#include <frmfmt.hxx>
#include <rolbck.hxx>           // Undo attributes, SwHistory
#include <poolfmt.hxx>          // for the Pool template
#include <dbmgr.hxx>
#include <docsh.hxx>
#include <acorrect.hxx>         // for the automatic adding of exceptions
#include <visiturl.hxx>         // for the URLChange message
#include <docary.hxx>
#include <lineinfo.hxx>
#include <drawdoc.hxx>
#include <linkenum.hxx>
#include <fldupde.hxx>
#include <extinput.hxx>
#include <viewsh.hxx>
#include <doctxm.hxx>
#include <shellres.hxx>
#include <breakit.hxx>
#include <laycache.hxx>
#include <mvsave.hxx>
#include <istyleaccess.hxx>
#include <swstylemanager.hxx>
#include <IGrammarContact.hxx>
#include <tblsel.hxx>
#include <MarkManager.hxx>
#include <UndoManager.hxx>
#include <unochart.hxx>

#include <cmdid.h>              // for the default printer in SetJob


#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <pausethreadstarting.hxx>
#include <numrule.hxx>
#include <list.hxx>
#include <listfunc.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <sfx2/Metadatable.hxx>
#include <fmtmeta.hxx> // MetaFieldManager

using namespace ::com::sun::star;
using namespace ::com::sun::star::document;

const sal_Char sFrmFmtStr[] = "Frameformat";
const sal_Char sEmptyPageStr[] = "Empty Page";
const sal_Char sColumnCntStr[] = "Columncontainer";
const sal_Char sCharFmtStr[] = "Zeichenformat";
const sal_Char sTxtCollStr[] = "Textformatvorlage";
const sal_Char sGrfCollStr[] = "Graphikformatvorlage";

SV_IMPL_PTRARR( SwNumRuleTbl, SwNumRulePtr)
SV_IMPL_PTRARR( SwTxtFmtColls, SwTxtFmtCollPtr)
SV_IMPL_PTRARR( SwGrfFmtColls, SwGrfFmtCollPtr)

/*
 * global functions...
 */

void StartGrammarChecking( SwDoc &rDoc )
{
    // check for a visible view
    bool bVisible = false;
    const SwDocShell *pDocShell = rDoc.GetDocShell();
    SfxViewFrame    *pFrame = SfxViewFrame::GetFirst( pDocShell, sal_False );
    while (pFrame && !bVisible)
    {
        if (pFrame->IsVisible())
            bVisible = true;
        pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell, sal_False );
    }

    //!! only documents with visible views need to be checked
    //!! (E.g. don't check temporary documents created for printing, see printing of notes and selections.
    //!! Those get created on the fly and get hard deleted a bit later as well, and no one should have
    //!! a uno reference to them)
    if (bVisible)
    {
        uno::Reference< linguistic2::XProofreadingIterator > xGCIterator( rDoc.GetGCIterator() );
        if ( xGCIterator.is() )
        {
            uno::Reference< lang::XComponent >  xDoc( rDoc.GetDocShell()->GetBaseModel(), uno::UNO_QUERY );
            uno::Reference< text::XFlatParagraphIteratorProvider >  xFPIP( xDoc, uno::UNO_QUERY );

            // start automatic background checking if not active already
            if ( xFPIP.is() && !xGCIterator->isProofreading( xDoc ) )
                xGCIterator->startProofreading( xDoc, xFPIP );
        }
    }
}

/*
 * internal functions
 */
sal_Bool lcl_DelFmtIndizes( const SwFrmFmtPtr& rpFmt, void* )
{
    SwFmtCntnt &rFmtCntnt = (SwFmtCntnt&)rpFmt->GetCntnt();
    if ( rFmtCntnt.GetCntntIdx() )
        rFmtCntnt.SetNewCntntIdx( 0 );
    SwFmtAnchor &rFmtAnchor = (SwFmtAnchor&)rpFmt->GetAnchor();
    if ( rFmtAnchor.GetCntntAnchor() )
        rFmtAnchor.SetAnchor( 0 );
    return sal_True;
}

IGrammarContact* getGrammarContact( const SwTxtNode& rTxtNode )
{
    const SwDoc* pDoc = rTxtNode.GetDoc();
    if( !pDoc || pDoc->IsInDtor() )
        return 0;
    return pDoc->getGrammarContact();
}

// #i42634# Moved common code of SwReader::Read() and SwDocShell::UpdateLinks()
// to new SwDoc::UpdateLinks():

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
