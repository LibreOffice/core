/*************************************************************************
 *
 *  $RCSfile: document.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: cmc $ $Date: 2000-11-15 10:47:20 $
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

#pragma hdrstop

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#include <sot/formats.hxx>
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SO_CLSIDS_HXX //autogen
#include <so3/clsids.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVTOOLS_FSTATHELPER_HXX
#include <svtools/fstathelper.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SFXECODE_HXX //autogen
#include <svtools/sfxecode.hxx>
#endif
#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif

#ifndef STARMATH_HRC
#include <starmath.hrc>
#endif
#ifndef DOCUMENT_HXX
#include <document.hxx>
#endif
#ifndef UNOMODEL_HXX
#include <unomodel.hxx>
#endif
#ifndef CONFIG_HXX
#include <config.hxx>
#endif
#ifndef SYMBOL_HXX
#include <symbol.hxx>
#endif
#ifndef TOOLBOX_HXX
#include <toolbox.hxx>
#endif
#ifndef DIALOG_HXX
#include <dialog.hxx>
#endif
#ifndef ACTION_HXX
#include <action.hxx>
#endif
#ifndef VIEW_HXX
#include <view.hxx>
#endif
#ifndef FORMAT_HXX
#include <format.hxx>
#endif
#ifndef MATHTYPE_HXX
#include <mathtype.hxx>
#endif
#ifndef MATHML_HXX
#include <mathml.hxx>
#endif



using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

#ifndef SO2_DECL_SVSTORAGESTREAM_DEFINED
#define SO2_DECL_SVSTORAGESTREAM_DEFINED
SO2_DECL_REF(SvStorageStream)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

#define DOCUMENT_BUFFER_SIZE    (USHORT)32768

static const char __FAR_DATA pStarMathDoc[] = "StarMathDocument";


/**************************************************************************/
/*
**
**  CLASS IMPLEMENTATION
**
**/

#define SmDocShell
#include "smslots.hxx"

TYPEINIT1( SmDocShell, SfxObjectShell );

SFX_IMPL_INTERFACE(SmDocShell, SfxObjectShell, SmResId(0))
{
    SFX_POPUPMENU_REGISTRATION(SmResId(RID_VIEWMENU));
    SFX_POPUPMENU_REGISTRATION(SmResId(RID_COMMANDMENU));
//    SFX_OBJECTBAR_REGISTRATION(
//        SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
//        SmResId(RID_DEFAULTTOOLBOX));
}

#if 0
SFX_IMPL_OBJECTFACTORY(SmDocShell, SFXOBJECTSHELL_STD_NORMAL, smath, SvGlobalName(SO3_SM_CLASSID_50) )
{
   SFX_SIMPLE_FILTER_REGISTRATION( C2S("StarMath 5.0"), C2S("*.smf"),
                           SFX_FILTER_OWN | SFX_FILTER_IMPORT | SFX_FILTER_EXPORT,
                           SOT_FORMATSTR_ID_STARMATH_50,
                           C2S("SVsm0.smf"), C2S("OS2"), 0, C2S("0"), String());

//JP 13.06.00: das fehlt vwohl noch, oder??
//  SfxObjectFactory& rFactory = (SfxObjectFactory&)Factory();
//  SfxFactoryFilterContainer *pFltContainer = rFactory.GetFilterContainer( FALSE );
//  rFactory.GetFilterContainer()->SetDetectFilter( &SmDLL::DetectFilter );

   // FG: Sonst gibts keine Hilfe im Math  #38447#
   Factory().RegisterHelpFile (C2S("smath.svh"));
}
#else
SFX_IMPL_OBJECTFACTORY_DLL(SmDocShell, smath, SvGlobalName(SO3_SM_CLASSID_50), Sm);
#endif

void SmDocShell::SFX_NOTIFY(SfxBroadcaster&, const TypeId&,
                    const SfxHint& rHint, const TypeId&)
{
    switch (((SfxSimpleHint&)rHint).GetId())
    {
        case HINT_CONFIGCHANGED:
        {
            SmModule *pp = SM_MOD1();

            if (pp->GetConfig()->IsAutoRedraw())
            {   nModifyCount++;     //! merkwrdig...
                                    // ohne dies wird die Grafik letztlich
                                    // nicht geupdatet
                Resize();
            }
            break;
        }

        case HINT_FORMATCHANGED:
            SetFormulaArranged(FALSE);
            nModifyCount++;     //! merkwrdig...
                                // ohne dies wird die Grafik letztlich
                                // nicht geupdatet
            Resize();
            break;
    }
}

void SmDocShell::LoadSymbols()
{
    SmModule *pp = SM_MOD1();
    String sURL( pp->GetConfig()->GetSymbolFile() );

    SvtPathOptions aOpt;
    sURL = aOpt.SubstituteVariable( sURL );
    if( !FStatHelper::IsDocument( sURL ) )
        aOpt.SearchFile( sURL, SvtPathOptions::PATH_USERCONFIG );
    pp->GetSymSetManager()->Load( sURL );
}

const String &SmDocShell::GetTitle() const
{
    return ((SmDocShell *) this)->GetDocInfo().GetTitle();
}



const String &SmDocShell::GetComment() const
{
    return ((SmDocShell *) this)->GetDocInfo().GetComment();
}



void SmDocShell::SetText(const String& rBuffer)
{
    if (rBuffer != aText)
    {
        aText = rBuffer;
        Parse();
        SetFormulaArranged(FALSE);
        Resize();
        SmViewShell *pViewSh = SmGetActiveView();
        if (pViewSh)
            pViewSh->GetViewFrame()->GetBindings().Invalidate(SID_TEXT);
        SetModified(TRUE);
    }
}

void SmDocShell::SetFormat(SmFormat& rFormat)
{
    aFormat = rFormat;
    SetFormulaArranged(FALSE);
    SmViewShell *pViewSh = SmGetActiveView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetBindings().Invalidate(SID_GRAPHIC);
    SetModified(TRUE);
}


void SmDocShell::Parse()
{
    if (pTree)
        delete pTree;
    pTree = aInterpreter.Parse(aText);
    nModifyCount++;
}


void SmDocShell::ArrangeFormula()
{
    //! Nur für die Dauer der Existenz dieses Objekts sind am Drucker die
    //! richtigen Einstellungen garantiert.
    SmPrinterAccess  aPrtAcc(*this);

    OutputDevice    *pOutDev = aPrtAcc.GetPrinter();

#ifndef PRODUCT
    if (!pOutDev)
        DBG_WARNING("Sm : kein Drucker vorhanden");
#endif

    // falls nötig ein anderes OutputDevice holen für das formatiert wird
    if (!pOutDev)
    {
        SmViewShell *pView = SmGetActiveView();
        if (pView)
            pOutDev = &pView->GetGraphicWindow();
        else
        {   pOutDev = new VirtualDevice;
            pOutDev->SetMapMode( MapMode(MAP_100TH_MM) );
        }
    }
    DBG_ASSERT(pOutDev->GetMapMode().GetMapUnit() == MAP_100TH_MM,
               "Sm : falscher MapMode");

    const SmFormat &rFormat = GetFormat();
    pTree->Prepare(rFormat);
    pTree->Arrange(*pOutDev, rFormat);

    SetFormulaArranged(TRUE);
}


void SmDocShell::Draw(OutputDevice &rDev, Point &rPosition)
{
    if (!pTree)
        Parse();
    DBG_ASSERT(pTree, "Sm : NULL pointer");

    if (!IsFormulaArranged())
        ArrangeFormula();

    //Problem: Was passiert mit dem WYSIWYG? Wir haben waehrend wir inplace aktiv
    //sind kein Referenzdevice und sind auch nicht darauf ausgerichtet. Es kann
    //also jetzt eine Differenz zwischen der VisArea (spricht die Groesse im Client)
    //und der jetzt vorliegenden Groese geben.
    //Idee: Die Differenz koennte, zumindest behelfsmaessig, mit SmNod::SetSize
    //angepasst werden.

    rPosition.X() += aFormat.GetDistance( DIS_LEFTSPACE );
    rPosition.Y() += aFormat.GetDistance( DIS_TOPSPACE  );
    pTree->Draw(rDev, rPosition);
}



Size SmDocShell::GetSize()
{
    Size aRet;

    if (!pTree)
        Parse();

    if (pTree)
    {
        if (!IsFormulaArranged())
            ArrangeFormula();
        aRet = pTree->GetSize();

        if ( !aRet.Width() )
            aRet.Width() = 2000;
        else
            aRet.Width()  += aFormat.GetDistance( DIS_LEFTSPACE ) +
                             aFormat.GetDistance( DIS_RIGHTSPACE );
        if ( !aRet.Height() )
            aRet.Height() = 1000;
        else
            aRet.Height() += aFormat.GetDistance( DIS_TOPSPACE ) +
                             aFormat.GetDistance( DIS_BOTTOMSPACE );
    }

    return aRet;
}

////////////////////////////////////////

SmPrinterAccess::SmPrinterAccess( SmDocShell &rDocShell )
{
    if ( 0 != (pPrinter = rDocShell.GetPrt()) )
    {
        pPrinter->Push( PUSH_MAPMODE );
        if ( rDocShell.GetProtocol().IsInPlaceActive() ||
             SFX_CREATE_MODE_EMBEDDED == rDocShell.GetCreateMode() )
        {
            // if it is an embedded object (without it's own printer)
            // we change the MapMode temporarily.
            //!If it is a document with it's own printer the MapMode should
            //!be set correct (once) elsewhere(!), in order to avoid numerous
            //!superfluous pushing and poping of the MapMode when using
            //!this class.

            const MapUnit eOld = pPrinter->GetMapMode().GetMapUnit();
            if ( MAP_100TH_MM != eOld )
            {
                MapMode aMap( pPrinter->GetMapMode() );
                aMap.SetMapUnit( MAP_100TH_MM );
                Point aTmp( aMap.GetOrigin() );
                aTmp.X() = OutputDevice::LogicToLogic( aTmp.X(), eOld, MAP_100TH_MM );
                aTmp.Y() = OutputDevice::LogicToLogic( aTmp.Y(), eOld, MAP_100TH_MM );
                aMap.SetOrigin( aTmp );
                pPrinter->SetMapMode( aMap );
            }
        }
    }
}

SmPrinterAccess::~SmPrinterAccess()
{
    if ( pPrinter )
        pPrinter->Pop();
}

////////////////////////////////////////

Printer *SmDocShell::GetPrt()
{
    if ( GetProtocol().IsInPlaceActive() ||
         SFX_CREATE_MODE_EMBEDDED == GetCreateMode() )
    {
        //Normalerweise wird der Printer vom Server besorgt. Wenn dieser aber
        //keinen liefert (weil etwa noch keine connection da ist), kann es
        //dennoch sein, dass wir den Printer kennen, denn dieser wird in
        //OnDocumentPrinterChanged vom Server durchgereicht und dann temporaer
        //festgehalten.
        Printer *pPrt = GetDocumentPrinter();
        if ( !pPrt )
            pPrt = pTmpPrinter;
        return pPrt;
    }
    else if ( !pPrinter )
    {
        SfxItemSet *pOptions =
            new SfxItemSet(GetPool(),
                           SID_PRINTSIZE,       SID_PRINTSIZE,
                           SID_PRINTZOOM,       SID_PRINTZOOM,
                           SID_PRINTTITLE,      SID_PRINTTITLE,
                           SID_PRINTTEXT,       SID_PRINTTEXT,
                           SID_PRINTFRAME,      SID_PRINTFRAME,
                           SID_NO_RIGHT_SPACES, SID_NO_RIGHT_SPACES,
                           0);

        SmModule *pp = SM_MOD1();
        pp->GetConfig()->ConfigToItemSet(*pOptions);
        pPrinter = new SfxPrinter(pOptions);
        pPrinter->SetMapMode( MapMode(MAP_100TH_MM) );
    }
    return pPrinter;
}


void SmDocShell::SetPrinter( SfxPrinter *pNew )
{
    delete pPrinter;
    pPrinter = pNew;    //Eigentumsuebergang!
    pPrinter->SetMapMode( MapMode(MAP_100TH_MM) );
    SetFormulaArranged(FALSE);
    SM_MOD1()->GetRectCache()->Reset();
    Resize();
}

void SmDocShell::OnDocumentPrinterChanged( Printer *pPrt )
{
    pTmpPrinter = pPrt;
    SetFormulaArranged(FALSE);
    SM_MOD1()->GetRectCache()->Reset();
    Size aOldSize = GetVisArea().GetSize();
    Resize();
    if ( aOldSize != GetVisArea().GetSize() )
        SetModified( TRUE );
    pTmpPrinter = 0;
}

void SmDocShell::Resize()
{
    Size aVisSize = GetSize();

    BOOL bIsEnabled = IsEnableSetModified();
    if ( bIsEnabled )
        EnableSetModified( FALSE );

    SetUpdateTimeout( 0L );
    SetVisAreaSize( aVisSize );
    SmViewShell *pViewSh = SmGetActiveView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetBindings().Invalidate(SID_GRAPHIC);

    if ( bIsEnabled )
        EnableSetModified( bIsEnabled );
}


SmDocShell::SmDocShell(SfxObjectCreateMode eMode) :
    SfxObjectShell(eMode),
    aDataTypeList(SvEmbeddedObject::GetStdFormatList()),
    pTree( 0 ),
    pPrinter( 0 ),
    pTmpPrinter( 0 ),
    nModifyCount( 0 ),
    bIsFormulaArranged( FALSE )
{
    SetPool(&SFX_APP()->GetPool());

    SmModule *pp = SM_MOD1();
    aFormat = pp->GetConfig()->GetFormat();

    StartListening(aFormat);
    StartListening(*pp->GetConfig());
    LoadSymbols();

    SetShell(this);
    SetModel( new SmModel(this) );  //! das hier mit new erzeugte Model brauch
                                    //! im Destruktor nicht explizit gelöscht werden.
                                    //! Dies erledigt das Sfx.
    pUndoMgr = new SfxUndoManager;
    aDataTypeList.Append(SvDataType(FORMAT_STRING, MEDIUM_MEMORY));
}



SmDocShell::~SmDocShell()
{
    SmModule *pp = SM_MOD1();

    EndListening(aFormat);
    EndListening(*pp->GetConfig());

    SaveSymbols();

    delete pUndoMgr;
    delete pTree;
    delete pPrinter;
}



BOOL SmDocShell::GetData(SvData *pData)
{
    if ( FORMAT_STRING == pData->GetFormat() )
    {
        pData->SetData(GetText());
        return TRUE;
    }

    return SfxInPlaceObject::GetData(pData);
}

BOOL SmDocShell::SetData( SvData *pData )
{
    if ( FORMAT_STRING == pData->GetFormat() )
    {
        String aData;
        if ( pData->GetData( aData ) )
        {
            SetText( aData );
            return TRUE;
        }
    }
    return SfxInPlaceObject::SetData( pData );
}

void SmDocShell::Convert40To50Txt()
    // adapts the text 'aText' to be parsed from 4.0 version, to changes made
    // in 5.0 version. (Namely that functions do not need arguments any more)
    // Example: "2 over sin x" acts very different in 4.0 and 5.0
{
    if (pTree)
        delete pTree;

    // parse in old 4.0 style and make changes for 5.0 style
    BOOL  bVal = aInterpreter.IsConvert40To50();
    aInterpreter.SetConvert40To50(TRUE);
    pTree = aInterpreter.Parse(aText);
    aInterpreter.SetConvert40To50(bVal);

    // get new to 5.0 version converted text
    aText = aInterpreter.GetText();

    // clean up tree parsed in old style
    if (pTree)
    {   delete pTree;
        pTree = NULL;
    }
}


BOOL SmDocShell::ConvertFrom(SfxMedium &rMedium)
{
    BOOL     bSuccess = FALSE;
    if (rMedium.GetFilter()->GetFilterName().EqualsAscii(
                    "MathML XML (Math)" ))
    {
        SmXMLWrapper aEquation(GetModel());
        DBG_ASSERT(!pTree,"pTree not NULL");
        if (pTree)
            delete pTree;
        pTree = aEquation.Import(rMedium);
        if (pTree)
        {
            //ToDo
            //aText = pTree->CreateTextFromTree();
            bSuccess = TRUE;
        }
    }
    else if( rMedium.IsStorage() && rMedium.GetStorage()->IsStream(
        C2S( "Equation Native" )))
    {
        // is this a MathType Storage?
        MathType aEquation( aText );
        bSuccess = aEquation.Parse( rMedium.GetStorage() );
    }
    else
    {
        SvStream *pStream = rMedium.GetInStream();
        if (pStream)
        {
            bSuccess = ImportSM20File( pStream );
            rMedium.CloseInStream();
        }
    }
    FinishedLoading( SFX_LOADED_ALL );
    return bSuccess;
}



BOOL SmDocShell::InsertFrom(SfxMedium &rMedium)
{
    BOOL     bSuccess = FALSE;
    SvStream *pStream = rMedium.GetInStream();

    if (pStream)
    {
        bSuccess = ImportSM20File( pStream, TRUE );
        rMedium.CloseInStream();
    }
    return bSuccess;
}



BOOL SmDocShell::InitNew(SvStorage * pStor)
{
    BOOL bRet = FALSE;
    if (SfxInPlaceObject::InitNew(pStor))
    {
        bRet = TRUE;
        SetVisArea(Rectangle(Point(0, 0), Size(2000, 1000)));
        if (pStor)
        {
            aDocStream = pStor->OpenStream(String::CreateFromAscii(pStarMathDoc));
            aDocStream->SetVersion (pStor->GetVersion ());
            GetPool().SetFileFormatVersion(USHORT(pStor->GetVersion()));

            if (! aDocStream )
                bRet = FALSE;
        }
    }
    return bRet;
}


BOOL SmDocShell::Load(SvStorage *pStor)
{
    BOOL bRet = FALSE;
    if( SfxInPlaceObject::Load( pStor ))
    {
        String aTmpStr( C2S( "Equation Native" ));
        if( pStor->IsStream( aTmpStr ))
        {
            // is this a MathType Storage?
            MathType aEquation(aText);
            bRet = (1 == aEquation.Parse(pStor));
        }
        else
        {
            bRet = 0 != Try3x (pStor, STREAM_READWRITE);

            if( !bRet )
            {
                pStor->Remove(String::CreateFromAscii(pStarMathDoc));
                bRet = 0 != Try2x (pStor, STREAM_READWRITE);
                pStor->Remove(C2S("\1Ole10Native"));
            }
            else
            {
                if ( pStor->GetVersion() <= SOFFICE_FILEFORMAT_40 )
                    Convert40To50Txt();
            }
        }
    }
    FinishedLoading( SFX_LOADED_ALL );
    return bRet;
}



BOOL SmDocShell::Insert(SvStorage *pStor)
{
    String aTemp = aText;
    BOOL bRet = TRUE;

    if (!Try3x (pStor, STREAM_STD_READ))
    {
        pStor->Remove (String::CreateFromAscii(pStarMathDoc));
        bRet = !Try2x (pStor, STREAM_STD_READ);
        pStor->Remove(C2S("\1Ole10Native"));
    }

    if ( bRet )
    {
        aTemp += aText;
        aText  = aTemp;

        if ( pStor->GetVersion() <= SOFFICE_FILEFORMAT_40 )
            Convert40To50Txt();

        Parse();
        SetModified(TRUE);
        SmViewShell *pViewSh = SmGetActiveView();
        if (pViewSh)
        {
             SfxBindings &rBnd =    pViewSh->GetViewFrame()->GetBindings();
            rBnd.Invalidate(SID_GRAPHIC);
            rBnd.Invalidate(SID_TEXT);
        }
    }
    return bRet;
}

//------------------------------------------------------------------

void SmDocShell::ImplSave( SvStorageStreamRef xStrm )
{
    ByteString exString (ExportString(aText));
    SmModule *pp = SM_MOD1();

    *xStrm  << SM304AIDENT << SM50VERSION
            << 'T';
    xStrm->WriteByteString(exString);
    *xStrm  << 'F' << aFormat
            << 'S' << pp->GetSymSetManager()->GetAccessedSymbols()
            << '\0';
}

BOOL SmDocShell::Save()
{
    if ( SfxInPlaceObject::Save() && aDocStream.Is() )
    {
        aDocStream->Seek(0);
        ImplSave( aDocStream );
        return TRUE;
    }
    return FALSE;
}


BOOL SmDocShell::SaveAs(SvStorage * pNewStor)
{
    BOOL bRet = FALSE;
    if ( SfxInPlaceObject::SaveAs( pNewStor ) )
    {
        SvStorageStreamRef aStm = pNewStor->OpenStream(
                                    String::CreateFromAscii(pStarMathDoc));
        aStm->SetVersion( pNewStor->GetVersion() );
        GetPool().SetFileFormatVersion( USHORT( pNewStor->GetVersion() ));
        aStm->SetBufferSize(DOCUMENT_BUFFER_SIZE);
        aStm->SetKey( pNewStor->GetKey() ); // Passwort setzen

        if ( aStm.Is() )
        {
            ImplSave( aStm );
            bRet = TRUE;
        }
    }
    return bRet;
}

BOOL SmDocShell::ConvertTo( SfxMedium &rMedium )
{
    BOOL bRet = FALSE;
    const SfxFilter* pFlt = rMedium.GetFilter();
    if( pFlt )
    {
        if(pFlt->GetFilterName().EqualsAscii(
            "MathML XML (Math)" ))
        {
            SmXMLWrapper aEquation(GetModel());
            bRet = aEquation.Export(rMedium,pTree);
        }
        else if( pFlt->GetFilterName().EqualsAscii("MathType 3.x"))
            bRet = WriteAsMathType3( rMedium );
    }
    return bRet;
}

BOOL SmDocShell::SaveCompleted(SvStorage * pStor)
{
    if( SfxInPlaceObject::SaveCompleted( pStor ))
    {
        if (! pStor)
            return TRUE;

        aDocStream = pStor->OpenStream(String::CreateFromAscii(pStarMathDoc));
        aDocStream->SetVersion (pStor->GetVersion ());
        GetPool().SetFileFormatVersion(USHORT(pStor->GetVersion()));
        aDocStream->SetBufferSize(DOCUMENT_BUFFER_SIZE);
        aDocStream->SetKey( pStor->GetKey() ); // Passwort setzen
        return aDocStream.Is();
    }
    return FALSE;
}



BOOL SmDocShell::ImportSM20File(SvStream *pStream, BOOL bInsert)
{
    void ReadSM20SymSet(SvStream*, SmSymSet*);

    char         cTag;
    ULONG        lIdent, lVersion;
    long         lTime;
    ULONG        lDate;
    String       aBuffer;
    SmSymSet    *pSymbolSet;
    ULONG        FilePos = pStream->Tell();

    *pStream >> lIdent >> lVersion;

    if (lIdent == FRMIDENT)
    {
        DBG_ASSERT((lVersion == FRMVERSION), "Illegal file version");

        *pStream >> cTag;
        rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
        while (cTag && !pStream->IsEof())
        {
            switch (cTag)
            {
                case 'T':
                    pStream->ReadByteString(aBuffer, eEnc);
                    if (! bInsert)
                    {
                        aText = aBuffer;
                        Parse();
                    }
                    else
                    {
                        SmViewShell *pViewSh = SmGetActiveView();
                        if (pViewSh)
                            pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                                    SID_INSERTTEXT, SFX_CALLMODE_STANDARD,
                                    new SfxStringItem(SID_INSERTTEXT, aBuffer), 0L);
                    }
                    break;

                case 'D':
                    {
                        pStream->ReadByteString(aBuffer, eEnc);
                        pStream->ReadByteString(aBuffer, eEnc);
                        *pStream >> lDate >> lTime;
                        pStream->ReadByteString(aBuffer, eEnc);
                        *pStream >> lDate >> lTime;
                        pStream->ReadByteString(aBuffer, eEnc);
                    }
                    break;

                case 'F':
                    aFormat.ReadSM20Format(*pStream);
                    aFormat.From300To304a ();
                    break;

                case 'S':
                    pSymbolSet = new SmSymSet();
                    ReadSM20SymSet(pStream, pSymbolSet);
                    if (! bInsert)
                    {
                        SmModule *pp = SM_MOD1();
                        pp->GetSymSetManager()->AppendExtraSymbolSet(pSymbolSet);
                    }
                    else
                        delete pSymbolSet;
                    break;

                default:
                    DBG_ASSERT((cTag != 0), "Illegal data tag");
            }
            *pStream >> cTag;
        }

        return TRUE;
    }

    pStream->Seek(FilePos);

    return FALSE;
}


void SmDocShell::Execute(SfxRequest& rReq)
{
    SfxBindings *pBindings  = NULL;
    SmViewShell *pViewSh    = SmGetActiveView();
    if (pViewSh)
        pBindings = &pViewSh->GetViewFrame()->GetBindings();
    switch (rReq.GetSlot())
    {
        case SID_TEXTMODE:
        {
            SmFormat  &rFormat = GetFormat();
            rFormat.SetTextmode(!rFormat.IsTextmode());
            rFormat.RequestApplyChanges();
            break;
        }

        case SID_SYMBOLS_LOAD:
        {
            SmModule *pp = SM_MOD1();

            SfxSimpleFileDialog  *pFileDialog =
                    new SfxSimpleFileDialog(0, WinBits(WB_OPEN | WB_3DLOOK));

#ifdef MAC
            pFileDialog->AddFilter(SmResId(RID_SYMBOLFILESSTR), SmResId(RID_FILESYMTYP));
            pFileDialog->AddFilter(SmResId(RID_ALLFILESSTR), C2S("****"));

            pFileDialog->SetCurFilter(SmResId(RID_SYMBOLFILESSTR));
#else
            String aExt( C2S("*.sms" ));
            pFileDialog->AddFilter(SmResId(RID_SYMBOLFILESSTR), aExt);
            pFileDialog->AddFilter(SmResId(RID_ALLFILESSTR), C2S("*.*"));
            pFileDialog->SetCurFilter(SmResId(RID_SYMBOLFILESSTR));
            pFileDialog->SetDefaultExt(aExt);
#endif

            pFileDialog->SetPath(pp->GetConfig()->GetSymbolFile());

            if ( RET_OK == pFileDialog->Execute() )
            {
                // save old symbols and sets if necessary
                if (pp->GetSymSetManager()->IsModified())
                    pp->GetSymSetManager()->Save();
                // load new symbols and sets from file
                INetURLObject aURLObj;
                aURLObj.SetSmartProtocol( INET_PROT_FILE );
                aURLObj.SetSmartURL( pFileDialog->GetPath() );
                pp->GetSymSetManager()->Load( aURLObj.GetMainURL() );
                // make that file the new default symbolfile
                pp->GetConfig()->SetSymbolFile( pFileDialog->GetPath() );
            }

            delete pFileDialog;
        }
        break;

        case SID_AUTO_REDRAW :
        {
            SmModule *pp = SM_MOD1();
            BOOL bRedraw = pp->GetConfig()->IsAutoRedraw();
            pp->GetConfig()->SetAutoRedraw(!bRedraw);
            break;
        }

        case SID_SYMBOLS_CATALOGUE:
        {
            SmSymSetManager *pSymSetMgr    = SM_MOD1()->GetSymSetManager();
            SmSymbolDialog(NULL, *pSymSetMgr).Execute();
            RestartFocusTimer();
            break;
        }

        case SID_TOOLBOX:
        {
            SmViewShell *pView = SmGetActiveView();
            pView->GetViewFrame()->ToggleChildWindow(
                    SmToolBoxWrapper::GetChildWindowId() );
            break;
        }

        case SID_INSERT_FORMULA:
        {
            SfxMedium *pMedium = SFX_APP()->
                    InsertDocumentDialog(SFXWB_INSERT | WB_OPEN | WB_3DLOOK,
                                         SmDocShell::Factory());

            if (pMedium != NULL)
            {
                if (pMedium->IsStorage())
                    Insert(pMedium->GetStorage());
                else
                    InsertFrom(*pMedium);
                delete pMedium;

                SetFormulaArranged(FALSE);
                // Fenster anpassen, neuzeichnen, ModifyCount erhöhen,...
                if (pBindings)
                    pBindings->Invalidate(SID_GRAPHIC);
            }
            RestartFocusTimer();
            rReq.SetReturnValue (SfxBoolItem (rReq.GetSlot(), TRUE));
            break;
        }

        case SID_LOADSYMBOLS:
            LoadSymbols();
            break;

        case SID_SAVESYMBOLS:
            SaveSymbols();
            break;

        case SID_FONT:
        {
            SmFontTypeDialog *pFontTypeDialog = new SmFontTypeDialog(NULL);

            pFontTypeDialog->ReadFrom(GetFormat());
            if (pFontTypeDialog->Execute() == RET_OK)
            {
                SmFormat& rOldFormat  = GetFormat();

                pFontTypeDialog->WriteTo(GetFormat());
                GetUndoManager()->AddUndoAction(
                        new SmFormatAction(this, rOldFormat, GetFormat()));

                if (aText.Len ())
                {
                    SetModified(TRUE);
                    if (pBindings)
                        pBindings->Invalidate(SID_GRAPHIC);
                }
                else SetModified (FALSE);
            }
            delete pFontTypeDialog;
            RestartFocusTimer ();
            break;
        }

        case SID_FONTSIZE:
        {
            SmFontSizeDialog *pFontSizeDialog = new SmFontSizeDialog(NULL);

            pFontSizeDialog->ReadFrom(GetFormat());
            if (pFontSizeDialog->Execute() == RET_OK)
            {
                SmFormat& rOldFormat  = GetFormat();

                pFontSizeDialog->WriteTo(GetFormat());

                GetUndoManager()->AddUndoAction(
                        new SmFormatAction(this, rOldFormat, GetFormat()));

                if (aText.Len ())
                {
                    SetModified(TRUE);
                    if (pBindings)
                        pBindings->Invalidate(SID_GRAPHIC);
                }
                else SetModified (FALSE);
            }
            delete pFontSizeDialog;
            RestartFocusTimer ();
            break;
        }

        case SID_DISTANCE:
        {
            SmDistanceDialog *pDistanceDialog = new SmDistanceDialog(NULL);

            pDistanceDialog->ReadFrom(GetFormat());
            if (pDistanceDialog->Execute() == RET_OK)
            {
                SmFormat& rOldFormat  = GetFormat();

                pDistanceDialog->WriteTo(GetFormat());

                GetUndoManager()->AddUndoAction(
                        new SmFormatAction(this, rOldFormat, GetFormat()));

                if (aText.Len ())
                {
                    SetModified(TRUE);
                    if (pBindings)
                        pBindings->Invalidate(SID_GRAPHIC);
                }
                else
                    SetModified (FALSE);
            }
            delete pDistanceDialog;
            RestartFocusTimer ();
            break;
        }

        case SID_ALIGN:
        {
            SmAlignDialog *pAlignDialog = new SmAlignDialog(NULL);

            pAlignDialog->ReadFrom(GetFormat());
            if (pAlignDialog->Execute() == RET_OK)
            {
                SmFormat& rOldFormat  = GetFormat();

                pAlignDialog->WriteTo(GetFormat());
                SmModule *pp = SM_MOD1();
                pAlignDialog->WriteTo(pp->GetConfig()->GetFormat());

                GetUndoManager()->AddUndoAction(
                        new SmFormatAction(this, rOldFormat, GetFormat()));

                if (aText.Len ())
                {
                    SetModified(TRUE);
                    if (pBindings)
                        pBindings->Invalidate(SID_GRAPHIC);
                }
                else
                    SetModified (FALSE);
            }
            delete pAlignDialog;
            RestartFocusTimer ();
            break;
        }

        case SID_TEXT:
        {
            const SfxStringItem& rItem =
                (const SfxStringItem&)rReq.GetArgs()->Get(SID_TEXT);

            if (GetText() != rItem.GetValue())
            {
                GetUndoManager()->AddUndoAction(
                         new SmEditAction(this, GetText(), rItem.GetValue()));

                SetText(rItem.GetValue());
            }
            break;
        }

        case SID_COPYOBJECT:
        {
            SvDataMemberObjectRef ObjRef = CreateSnapshot();
            ObjRef->CopyClipboard();
            break;
        }

        case SID_PASTEOBJECT:
        {
            SvDataObjectRef ObjRef = SvDataObject::PasteClipboard();

            if (ObjRef.Is())
            {
                SvObjectDescriptor ObjDesc(ObjRef);

                if (ObjDesc.GetClassName() == *GetSvFactory())
                {
                    SvStorageRef StorageRef(new SvStorage(String()));
                    SvData       ObjData(SOT_FORMATSTR_ID_EMBED_SOURCE);

                    ObjData.SetData(StorageRef, TRANSFER_REFERENCE);

                    if (ObjRef->GetDataHere(&ObjData))
                        Insert(StorageRef);
                }
            }

            break;
        }
    }
}


void SmDocShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);

    for (USHORT nWh = aIter.FirstWhich();  0 != nWh;  nWh = aIter.NextWhich())
    {
        switch (nWh)
        {
            case SID_TEXTMODE:
            {
                rSet.Put(SfxBoolItem(SID_TEXTMODE, GetFormat().IsTextmode()));
                break;
            }

            case SID_DOCTEMPLATE :
                rSet.DisableItem (SID_DOCTEMPLATE);
                break;

            case SID_AUTO_REDRAW :
            {
                SmModule  *pp = SM_MOD1();
                BOOL       bRedraw = pp->GetConfig()->IsAutoRedraw();

                rSet.Put (SfxBoolItem(SID_AUTO_REDRAW, bRedraw));
                break;
            }

            case SID_TOOLBOX:
            {
                BOOL bState = FALSE;
                SmViewShell *pView = SmGetActiveView();
                SfxChildWindow *pChildWnd = pView->GetViewFrame()->
                        GetChildWindow( SmToolBoxWrapper::GetChildWindowId() );

                if (pChildWnd  &&  pChildWnd->GetWindow()->IsVisible())
                    bState = TRUE;
                rSet.Put(SfxBoolItem(SID_TOOLBOX, bState));
                break;
            }

            case SID_MODIFYSTATUS:
            {
                char cMod = ' ';
                if (IsModified())
                    cMod = '*';
                rSet.Put(SfxStringItem(SID_MODIFYSTATUS, String(cMod)));
                break;
            }

            case SID_TEXT:
                rSet.Put(SfxStringItem(SID_TEXT, GetText()));
                break;

            case SID_GRAPHIC:
                rSet.Put(SfxInt16Item(SID_GRAPHIC, nModifyCount));
                break;
        }
    }
}



void SmDocShell::FillRegInfo(SvEmbeddedRegistryInfo * pInfo)
{
    SfxInPlaceObject::FillRegInfo(pInfo);
    pInfo->aObjName.AssignAscii( RTL_CONSTASCII_STRINGPARAM(pStarMathDoc));
    pInfo->nMajorVers = 5;
    pInfo->nMinorVers = 0;
    pInfo->aHumanShortTypeName = String(SmResId(RID_DOCUMENTSTR));
    pInfo->nShell = REG_SHELL_OPEN | REG_SHELL_PRINT | REG_SHELL_PRINTTO;
    pInfo->aExt.AssignAscii( RTL_CONSTASCII_STRINGPARAM(".SMF"));
    pInfo->nIconResId = REG_ICON_RESID_MATH;
}



SfxUndoManager *SmDocShell::GetUndoManager ()
{
    if (! pUndoMgr)
    {
        pUndoMgr = new SfxUndoManager;
        SetUndoManager(pUndoMgr);
    }
    return pUndoMgr;
}



void SmDocShell::SaveSymbols()
{
    SmModule *pp = SM_MOD1();
    pp->GetSymSetManager()->Save();
}



void SmDocShell::RestartFocusTimer ()
{
    SmCmdBoxWrapper *pWrapper = NULL;
    SmViewShell *pView = SmGetActiveView();
    if (pView)
        pWrapper = (SmCmdBoxWrapper *) pView->GetViewFrame()->
                GetChildWindow( SmCmdBoxWrapper::GetChildWindowId() );

    if (pWrapper)
        pWrapper->RestartFocusTimer ();
}



void SmDocShell::Draw(OutputDevice *pDevice,
                      const JobSetup &,
                      USHORT nAspect)
{
    pDevice->IntersectClipRegion(GetVisArea());
    Point atmppoint;
    Draw(*pDevice, atmppoint);
}



void SmDocShell::HandsOff()
{
    SfxInPlaceObject::HandsOff();
    aDocStream.Clear();
}



SvDataMemberObjectRef SmDocShell::CreateSnapshot ()
{
    return SfxInPlaceObject::CreateSnapshot ();
}



SfxItemPool& SmDocShell::GetPool()
{
    return SFX_APP()->GetPool();
}

const SvDataTypeList& SmDocShell::GetTypeList() const
{
    return aDataTypeList;
}

void SmDocShell::SetVisArea (const Rectangle & rVisArea)
{
    Rectangle aNewRect (rVisArea);

    aNewRect.SetPos(Point ());

    if (! aNewRect.Right ()) aNewRect.Right () = 2000;
    if (! aNewRect.Bottom ()) aNewRect.Bottom () = 1000;

    BOOL bIsEnabled = IsEnableSetModified();
    if ( bIsEnabled )
        EnableSetModified( FALSE );

    // If outplace editing, then dont resize the OutplaceWindow. But the
    // ObjectShell has to resize. Bug 56470
    BOOL bUnLockFrame;
    if( GetProtocol().IsEmbed() && !GetProtocol().IsInPlaceActive() &&
        GetFrame() )
    {
        GetFrame()->LockAdjustPosSizePixel();
        bUnLockFrame = TRUE;
    }
    else
        bUnLockFrame = FALSE;

    SfxInPlaceObject::SetVisArea( aNewRect );

    if( bUnLockFrame )
        GetFrame()->UnlockAdjustPosSizePixel();

    if ( bIsEnabled )
        EnableSetModified( bIsEnabled );
}


BOOL SmDocShell::Try3x (SvStorage *pStor,
                        StreamMode eMode)

{
    BOOL bRet = FALSE;

    SvStorageStreamRef aTempStream = pStor->OpenStream(
                                String::CreateFromAscii(pStarMathDoc), eMode);
    aTempStream->SetVersion (pStor->GetVersion ());
    GetPool().SetFileFormatVersion (USHORT(pStor->GetVersion()));
    aTempStream->SetBufferSize(DOCUMENT_BUFFER_SIZE);
    aTempStream->SetKey( pStor->GetKey() ); // Passwort setzen

    if (aTempStream->GetError() == 0)
    {
        SvStream*    pSvStream = aTempStream;
        char         cTag;
        ULONG        lIdent, lVersion;
        long         lTime;
        ULONG        lDate;
        String       aBuffer;
        SmSymSet    *pSymbolSet;

        *pSvStream >> lIdent >> lVersion;

        if ((lIdent == SM30IDENT) || (lIdent == SM30BIDENT) || (lIdent == SM304AIDENT))
        {
            DBG_ASSERT((lVersion == SM30VERSION) ||
                       (lVersion == SM50VERSION), "Illegal file version");

            *pSvStream >> cTag;
            rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
            while (cTag && !pSvStream->IsEof())
            {
                switch (cTag)
                {
                    case 'T':
                        pSvStream->ReadByteString(aText, eEnc);
                        Parse();
                        break;

                    case 'D':
                        pSvStream->ReadByteString(aBuffer, eEnc);
                        pSvStream->ReadByteString(aBuffer, eEnc);
                        *pSvStream >> lDate >> lTime;
                        pSvStream->ReadByteString(aBuffer, eEnc);
                        *pSvStream >> lDate >> lTime;
                        pSvStream->ReadByteString(aBuffer, eEnc);
                        break;

                    case 'F':
                        *pSvStream >> aFormat;
                        if (lIdent != SM304AIDENT)
                            aFormat.From300To304a ();
                        else if ( lVersion == SM30VERSION )
                        {
                            aFormat.SetDistance(DIS_LEFTSPACE, 100);
                            aFormat.SetDistance(DIS_RIGHTSPACE, 100);
                            aFormat.SetDistance(DIS_TOPSPACE, 100);
                            aFormat.SetDistance(DIS_BOTTOMSPACE, 100);
                        }
                        break;

                    case 'S':
                    {
                        pSymbolSet = new SmSymSet();
                        *pSvStream >> *pSymbolSet;

                        SmModule *pp = SM_MOD1();

                        pp->GetSymSetManager()->AppendExtraSymbolSet(pSymbolSet);
                        break;
                    }

                    default:
                        DBG_ASSERT((cTag != 0), "Illegal data tag");
                }
                *pSvStream >> cTag;
            }

            bRet = TRUE;
            aDocStream = aTempStream;
        }
    }

    if (!bRet)
    {
        // kein Passwort gesetzt --> Datei marode
        if (pStor->GetKey().Len() == 0)
        {
            SetError(ERRCODE_SFX_DOLOADFAILED);
        }
        // Passwort gesetzt --> war wohl falsch
        else
        {
            SetError(ERRCODE_SFX_WRONGPASSWORD);
        }
    }

    return bRet;
}



BOOL SmDocShell::Try2x (SvStorage *pStor,
                        StreamMode eMode)
{
    SvStorageStreamRef aTempStream = pStor->OpenStream(C2S("\1Ole10Native"), eMode);
    aTempStream->SetVersion (pStor->GetVersion ());
    GetPool().SetFileFormatVersion(USHORT(pStor->GetVersion ()));

    if (aTempStream->GetError() == SVSTREAM_OK)
    {
        void ReadSM20SymSet(SvStream*, SmSymSet*);

        SvStream*    pSvStream = aTempStream;
        char         cTag;
        ULONG        lIdent, lVersion;
        long         lTime;
        ULONG        lDate;
        UINT32       lDataSize;
        String       aBuffer;
        SmSymSet    *pSymbolSet;

        *pSvStream >> lDataSize >> lIdent >> lVersion;

        if (lIdent == FRMIDENT)
        {
            DBG_ASSERT((lVersion == FRMVERSION), "Illegal file version");

            *pSvStream >> cTag;
            rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
            while (cTag && !pSvStream->IsEof())
            {
                switch (cTag)
                {
                    case 'T':
                        pSvStream->ReadByteString(aBuffer, eEnc);
                        aText = aBuffer;
                        Parse();
                        break;

                    case 'D':
                        {
                            pSvStream->ReadByteString(aBuffer, eEnc);
                            pSvStream->ReadByteString(aBuffer, eEnc);
                            *pSvStream >> lDate >> lTime;
                            pSvStream->ReadByteString(aBuffer, eEnc);
                            *pSvStream >> lDate >> lTime;
                            pSvStream->ReadByteString(aBuffer, eEnc);
                        }
                        break;

                    case 'F':
                        {
                            //SmFormat aFormat;
                            aFormat.ReadSM20Format(*pSvStream);
                            aFormat.From300To304a ();
                        }
                        break;

                    case 'S':
                    {
                        pSymbolSet = new SmSymSet();
                        ReadSM20SymSet(pSvStream, pSymbolSet);

                        SmModule *pp = SM_MOD1();

                        pp->GetSymSetManager()->AppendExtraSymbolSet(pSymbolSet);
                        break;
                    }

                    default:
                        DBG_ASSERT((cTag != 0), "Illegal data tag");
                }
                *pSvStream >> cTag;
            }

            return TRUE;
        }
    }

    return FALSE;
}



void SmDocShell::UIActivate (BOOL bActivate)
{
    if (bActivate)
    {
        SfxInPlaceObject::UIActivate (bActivate);
        SmCmdBoxWrapper *pWrapper = NULL;
        SmViewShell *pView = SmGetActiveView();
        if (pView)
            pWrapper = (SmCmdBoxWrapper *)pView->GetViewFrame()->
                    GetChildWindow( SmCmdBoxWrapper::GetChildWindowId() );

        if (pWrapper)
            pWrapper->Grab ();
    }
    else
    {
        SmViewShell *pViewSh = SmGetActiveView();
        if (pViewSh)
        {
            pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                    SID_GETEDITTEXT, SFX_CALLMODE_STANDARD,
                    new SfxVoidItem (SID_GETEDITTEXT), 0L);
            SfxInPlaceObject::UIActivate (bActivate);
            Resize();
        }
    }
}



void SmDocShell::FillClass(SvGlobalName* pClassName,
                           ULONG*  pFormat,
                           String* pAppName,
                           String* pFullTypeName,
                           String* pShortTypeName,
                           long    nFileFormat) const
{
    SfxInPlaceObject::FillClass(pClassName, pFormat, pAppName, pFullTypeName,
                                pShortTypeName, nFileFormat);

    if (nFileFormat == SOFFICE_FILEFORMAT_31)
    {
        *pClassName     = SvGlobalName(SO3_SM_CLASSID_30);
        *pFormat        = SOT_FORMATSTR_ID_STARMATH;
        pAppName->AssignAscii( RTL_CONSTASCII_STRINGPARAM("Smath 3.1"));
        *pFullTypeName  = String(SmResId(STR_MATH_DOCUMENT_FULLTYPE_31));
        *pShortTypeName = String(SmResId(RID_DOCUMENTSTR));
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_40)
    {
        *pClassName     = SvGlobalName(SO3_SM_CLASSID_40);
        *pFormat        = SOT_FORMATSTR_ID_STARMATH_40;
        *pFullTypeName  = String(SmResId(STR_MATH_DOCUMENT_FULLTYPE_40));
        *pShortTypeName = String(SmResId(RID_DOCUMENTSTR));
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_NOW)
    {
        *pFullTypeName  = String(SmResId(STR_MATH_DOCUMENT_FULLTYPE_50));
        *pShortTypeName = String(SmResId(RID_DOCUMENTSTR));
    }
}

ULONG SmDocShell::GetMiscStatus() const
{
    return SfxInPlaceObject::GetMiscStatus() | SVOBJ_MISCSTATUS_NOTRESIZEABLE
                                             | SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE;
}

void SmDocShell::SetModified(BOOL bModified)
{
    SfxObjectShell::SetModified(bModified);
    Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
}

BOOL SmDocShell::WriteAsMathType3( SfxMedium& rMedium )
{
    MathType aEquation( aText, pTree );
    BOOL bRet = 0 != aEquation.ConvertFromStarMath( rMedium.GetStorage() );
    return bRet;
}



