/*************************************************************************
 *
 *  $RCSfile: servobj.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:55 $
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

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <sot/formats.hxx>
#include <sfx2/app.hxx>
#include <svx/linkmgr.hxx>
#include "servobj.hxx"
#include "docsh.hxx"
#include "impex.hxx"
#include "brdcst.hxx"
#include "rangenam.hxx"
#include "sc.hrc"               // SC_HINT_AREAS_CHANGED

// -----------------------------------------------------------------------

BOOL lcl_FillRangeFromName( ScRange& rRange, ScDocShell* pDocSh, const String& rName )
{
    if (pDocSh)
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        ScRangeName* pNames = pDoc->GetRangeName();
        if (pNames)
        {
            USHORT nPos;
            if( pNames->SearchName( rName, nPos ) )
            {
                ScRangeData* pData = (*pNames)[ nPos ];
                if ( pData->IsReference( rRange ) )
                    return TRUE;
            }
        }
    }
    return FALSE;
}

ScServerObject::ScServerObject( ScDocShell* pShell, const String& rItem ) :
    pDocSh( pShell ),
    bRefreshListener( FALSE )
{
    //  parse item string

    if ( lcl_FillRangeFromName( aRange, pDocSh, rItem ) )
    {
        aItemStr = rItem;               // must be parsed again on ref update
    }
    else
    {
        //  parse ref
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = pDocSh->GetCurTab();
        aRange.aStart.SetTab( nTab );

        if ( aRange.Parse( rItem, pDoc ) & SCA_VALID )
        {
            // area reference
        }
        else if ( aRange.aStart.Parse( rItem, pDoc ) & SCA_VALID )
        {
            // cell reference
            aRange.aEnd = aRange.aStart;
        }
        else
        {
            DBG_ERROR("ScServerObject: invalid item");
        }
    }

    pDocSh->GetDocument()->GetLinkManager()->InsertServer( this );
    pDocSh->GetDocument()->StartListeningArea( aRange, this );

    StartListening(*pDocSh);        // um mitzubekommen, wenn die DocShell geloescht wird
    StartListening(*SFX_APP());     // for SC_HINT_AREAS_CHANGED
}

__EXPORT ScServerObject::~ScServerObject()
{
    Clear();
}

void ScServerObject::Clear()
{
    if (pDocSh)
    {
        ScDocShell* pTemp = pDocSh;
        pDocSh = NULL;

        pTemp->GetDocument()->EndListeningArea( aRange, this );
        pTemp->GetDocument()->GetLinkManager()->RemoveServer( this );
        EndListening(*pTemp);
        EndListening(*SFX_APP());
    }
}

BOOL __EXPORT ScServerObject::GetData( SvData* pData )      // wie ScDocShell::DdeGetData
{
    if (!pDocSh)
        return FALSE;

    // named ranges may have changed -> update aRange
    if ( aItemStr.Len() )
    {
        ScRange aNew;
        if ( lcl_FillRangeFromName( aNew, pDocSh, aItemStr ) && aNew != aRange )
        {
            aRange = aNew;
            bRefreshListener = TRUE;
        }
    }

    if ( bRefreshListener )
    {
        //  refresh the listeners now (this is called from a timer)

        EndListeningAll();
        pDocSh->GetDocument()->StartListeningArea( aRange, this );
        StartListening(*pDocSh);
        StartListening(*SFX_APP());
        bRefreshListener = FALSE;
    }

    String aDdeTextFmt = pDocSh->GetDdeTextFmt();
    ScDocument* pDoc = pDocSh->GetDocument();

    if( pData->GetFormat() == FORMAT_STRING )
    {
        ScImportExport aObj( pDoc, aRange );
        if( aDdeTextFmt.GetChar(0) == 'F' )
            aObj.SetFormulas( TRUE );
        if( aDdeTextFmt.EqualsAscii( "SYLK" ) || aDdeTextFmt.EqualsAscii( "FSYLK" ) )
        {
            String aData;
            if( aObj.ExportString( aData, SOT_FORMATSTR_ID_SYLK ) )
            {
                ByteString aByteData( aData, gsl_getSystemTextEncoding() );
                pData->SetData( (void*) aByteData.GetBuffer(), aByteData.Len() + 1 );
                return 1;
            }
            else
                return 0;
        }
        if( aDdeTextFmt.EqualsAscii( "CSV" ) || aDdeTextFmt.EqualsAscii( "FCSV" ) )
            aObj.SetSeparator( ',' );
        return aObj.ExportData( *pData ) ? 1 : 0;
    }
    ScImportExport aObj( pDoc, aRange );
    if( aObj.IsRef() )
        return aObj.ExportData( *pData ) ? 1 : 0;
    else
        return 0;
}

BOOL __EXPORT ScServerObject::SetData( SvData& )
{
    //  erstmal nicht
    return FALSE;
}

void __EXPORT ScServerObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                         const SfxHint& rHint, const TypeId& rHintType )
{
    BOOL bDataChanged = FALSE;

    if (rBC.ISA(ScDocShell))
    {
        //  from DocShell, only SFX_HINT_DYING is interesting
        if ( rHint.ISA(SfxSimpleHint) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
            Clear();
    }
    else if (rBC.ISA(SfxApplication))
    {
        if ( aItemStr.Len() && rHint.ISA(SfxSimpleHint) &&
                ((const SfxSimpleHint&)rHint).GetId() == SC_HINT_AREAS_CHANGED )
        {
            //  check if named range was modified
            ScRange aNew;
            if ( lcl_FillRangeFromName( aNew, pDocSh, aItemStr ) && aNew != aRange )
                bDataChanged = TRUE;
        }
    }
    else
    {
        //  must be from Area broadcasters

        const ScHint* pScHint = PTR_CAST( ScHint, &rHint );
        if( pScHint && (pScHint->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING)) )
            bDataChanged = TRUE;
        else if (rHint.ISA(ScAreaChangedHint))      // position of broadcaster changed
        {
            ScRange aNewRange = ((const ScAreaChangedHint&)rHint).GetRange();
            if ( aRange != aNewRange )
            {
                bRefreshListener = TRUE;
                bDataChanged = TRUE;
            }
        }
        else if (rHint.ISA(SfxSimpleHint))
        {
            USHORT nId = ((const SfxSimpleHint&)rHint).GetId();
            if (nId == SFX_HINT_DYING)
            {
                //  If the range is being deleted, listening must be restarted
                //  after the deletion is complete (done in GetData)
                bRefreshListener = TRUE;
                bDataChanged = TRUE;
            }
        }
    }

    if ( bDataChanged && GetSelectorCount() )
    {
        SvData aSvData;
        DataChanged( aSvData );
    }
}





