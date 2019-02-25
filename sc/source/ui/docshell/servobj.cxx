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

#include <sot/formats.hxx>
#include <sfx2/app.hxx>
#include <sfx2/linkmgr.hxx>
#include <servobj.hxx>
#include <docsh.hxx>
#include <impex.hxx>
#include <brdcst.hxx>
#include <rangenam.hxx>
#include <unotools/charclass.hxx>

using namespace formula;

static bool lcl_FillRangeFromName( ScRange& rRange, ScDocShell* pDocSh, const OUString& rName )
{
    if (pDocSh)
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        ScRangeName* pNames = rDoc.GetRangeName();
        if (pNames)
        {
            const ScRangeData* pData = pNames->findByUpperName(ScGlobal::pCharClass->uppercase(rName));
            if (pData)
            {
                if ( pData->IsValidReference( rRange ) )
                    return true;
            }
        }
    }
    return false;
}

ScServerObjectSvtListenerForwarder::ScServerObjectSvtListenerForwarder(
        ScServerObject* pObjP)
    : pObj(pObjP)
{
}

ScServerObjectSvtListenerForwarder::~ScServerObjectSvtListenerForwarder()
{
    //! do NOT access pObj
}

void ScServerObjectSvtListenerForwarder::Notify( const SfxHint& rHint )
{
    pObj->Notify( aBroadcaster, rHint);
}

ScServerObject::ScServerObject( ScDocShell* pShell, const OUString& rItem ) :
    aForwarder( this ),
    pDocSh( pShell ),
    bRefreshListener( false )
{
    //  parse item string

    if ( lcl_FillRangeFromName( aRange, pDocSh, rItem ) )
    {
        aItemStr = rItem;               // must be parsed again on ref update
    }
    else
    {
        //  parse ref
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = ScDocShell::GetCurTab();
        aRange.aStart.SetTab( nTab );

        // For DDE link, we always must parse references using OOO A1 convention.

        if ( aRange.Parse( rItem, &rDoc, FormulaGrammar::CONV_OOO ) & ScRefFlags::VALID )
        {
            // area reference
        }
        else if ( aRange.aStart.Parse( rItem, &rDoc, FormulaGrammar::CONV_OOO ) & ScRefFlags::VALID )
        {
            // cell reference
            aRange.aEnd = aRange.aStart;
        }
        else
        {
            OSL_FAIL("ScServerObject: invalid item");
        }
    }

    pDocSh->GetDocument().GetLinkManager()->InsertServer( this );
    pDocSh->GetDocument().StartListeningArea( aRange, false, &aForwarder );

    StartListening(*pDocSh);           // to notice if DocShell gets deleted
    StartListening(*SfxGetpApp());     // for SfxHintId::ScAreasChanged
}

ScServerObject::~ScServerObject()
{
    Clear();
}

void ScServerObject::Clear()
{
    if (pDocSh)
    {
        ScDocShell* pTemp = pDocSh;
        pDocSh = nullptr;

        pTemp->GetDocument().EndListeningArea(aRange, false, &aForwarder);
        pTemp->GetDocument().GetLinkManager()->RemoveServer( this );
        EndListening(*pTemp);
        EndListening(*SfxGetpApp());
    }
}

void ScServerObject::EndListeningAll()
{
    aForwarder.EndListeningAll();
    SfxListener::EndListeningAll();
}

bool ScServerObject::GetData(
        css::uno::Any & rData /*out param*/,
        const OUString & rMimeType, bool /* bSynchron */ )
{
    if (!pDocSh)
        return false;

    // named ranges may have changed -> update aRange
    if ( !aItemStr.isEmpty() )
    {
        ScRange aNew;
        if ( lcl_FillRangeFromName( aNew, pDocSh, aItemStr ) && aNew != aRange )
        {
            aRange = aNew;
            bRefreshListener = true;
        }
    }

    if ( bRefreshListener )
    {
        //  refresh the listeners now (this is called from a timer)

        EndListeningAll();
        pDocSh->GetDocument().StartListeningArea( aRange, false, &aForwarder );
        StartListening(*pDocSh);
        StartListening(*SfxGetpApp());
        bRefreshListener = false;
    }

    OUString aDdeTextFmt = pDocSh->GetDdeTextFmt();
    ScDocument& rDoc = pDocSh->GetDocument();

    SotClipboardFormatId eFormatId = SotExchange::GetFormatIdFromMimeType( rMimeType );
    if (SotClipboardFormatId::STRING == eFormatId || SotClipboardFormatId::STRING_TSVC == eFormatId)
    {
        ScImportExport aObj( &rDoc, aRange );
        if( aDdeTextFmt[0] == 'F' )
            aObj.SetFormulas( true );
        if( aDdeTextFmt == "SYLK" || aDdeTextFmt == "FSYLK" )
        {
            OString aByteData;
            if( aObj.ExportByteString( aByteData, osl_getThreadTextEncoding(), SotClipboardFormatId::SYLK ) )
            {
                rData <<= css::uno::Sequence< sal_Int8 >(
                                        reinterpret_cast<const sal_Int8*>(aByteData.getStr()),
                                        aByteData.getLength() + 1 );
                return true;
            }
            return false;
        }
        if( aDdeTextFmt == "CSV" || aDdeTextFmt == "FCSV" )
            aObj.SetSeparator( ',' );
        /* TODO: STRING_TSVC could preserve line breaks with added quotes. */
        aObj.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::ToSpace, ' ', false ) );
        return aObj.ExportData( rMimeType, rData );
    }

    ScImportExport aObj( &rDoc, aRange );
    aObj.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::ToSpace, ' ', false ) );
    if( aObj.IsRef() )
        return aObj.ExportData( rMimeType, rData );
    return false;
}

void ScServerObject::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    bool bDataChanged = false;

    //  DocShell can't be tested via type info, because SfxHintId::Dying comes from the dtor
    if ( &rBC == pDocSh )
    {
        //  from DocShell, only SfxHintId::Dying is interesting
        if ( rHint.GetId() == SfxHintId::Dying )
        {
            pDocSh = nullptr;
            EndListening(*SfxGetpApp());
            //  don't access DocShell anymore for EndListening etc.
        }
    }
    else if (dynamic_cast<const SfxApplication*>( &rBC) !=  nullptr)
    {
        if ( !aItemStr.isEmpty() && rHint.GetId() == SfxHintId::ScAreasChanged )
        {
            //  check if named range was modified
            ScRange aNew;
            if ( lcl_FillRangeFromName( aNew, pDocSh, aItemStr ) && aNew != aRange )
                bDataChanged = true;
        }
    }
    else
    {
        //  must be from Area broadcasters

        const ScHint* pScHint = dynamic_cast<const ScHint*>( &rHint );
        if (pScHint && (pScHint->GetId() == SfxHintId::ScDataChanged))
            bDataChanged = true;
        else if (const ScAreaChangedHint *pChgHint = dynamic_cast<const ScAreaChangedHint*>(&rHint))      // position of broadcaster changed
        {
            const ScRange& aNewRange = pChgHint->GetRange();
            if ( aRange != aNewRange )
            {
                bRefreshListener = true;
                bDataChanged = true;
            }
        }
        else
        {
            if (rHint.GetId() == SfxHintId::Dying)
            {
                //  If the range is being deleted, listening must be restarted
                //  after the deletion is complete (done in GetData)
                bRefreshListener = true;
                bDataChanged = true;
            }
        }
    }

    if ( bDataChanged && HasDataLinks() )
        SvLinkSource::NotifyDataChanged();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
