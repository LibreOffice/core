/*************************************************************************
 *
 *  $RCSfile: appbas.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ab $ $Date: 2001-03-03 16:48:38 $
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

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _BASRDLL_HXX //autogen
#include <basic/basrdll.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef __SBX_SBX_FACTORY_HXX //autogen
#include <svtools/sbxfac.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SB_SBMETH_HXX //autogen
#include <basic/sbmeth.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#if SUPD<613//MUSTINI
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVTDEMO_HXX //autogen
#include <svtools/svtdemo.hxx>
#endif
#ifndef _REGCODE_HXX
//#include <tools/regcode.hxx>
#endif
#ifndef _SB_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include <svtools/undoopt.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/useroptions.hxx>

#pragma hdrstop

#include "appuno.hxx"
#include "module.hxx"
#include "arrdecl.hxx"
#include "app.hxx"
#include "sfxtypes.hxx"
#include "sfxresid.hxx"
#include "msg.hxx"
#include "msgpool.hxx"
#include "progress.hxx"
#include "objsh.hxx"
#include "objitem.hxx"
#include "viewfrm.hxx"
#include "viewsh.hxx"
#include "dispatch.hxx"
#include "tplpitem.hxx"
#include "minfitem.hxx"
#include "app.hrc"
#include "evntconf.hxx"
#include "macrconf.hxx"
#include "request.hxx"
#include "docinf.hxx"
#include "dinfdlg.hxx"
#include "appdata.hxx"
#include "appbas.hxx"
#include "sfxhelp.hxx"
#include "stbmgr.hxx"
#include "appimp.hxx"
#include "basmgr.hxx"
#include "dlgcont.hxx"
#include "helper.hxx"

#define ITEMID_SEARCH SID_SEARCH_ITEM

#include "srchitem.hxx"
#ifndef _VOS_SOCKET_HXX_ //autogen
#include <vos/socket.hxx>
#endif

#define SFX_TYPEMAP
#define Selection
#include "sfxslots.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;

// #ifndef STR_VERSION_ID
// #define STR_VERSION_ID 1
// #endif
// #ifndef STR_VERSION_TYPE
// #define STR_VERSION_TYPE 1
// #endif

//=========================================================================
/*ASDBG
const SfxConstant __FAR_DATA aConstants[] =
{
    SFX_USHORT_CONSTANT( "AlignBottom",         ALIGN_BOTTOM ),
    SFX_USHORT_CONSTANT( "AlignTop",            ALIGN_TOP ),
    SFX_USHORT_CONSTANT( "CharSetANSI",         CHARSET_ANSI ),
    SFX_USHORT_CONSTANT( "CharSetDontKnow",     CHARSET_DONTKNOW ),
    SFX_USHORT_CONSTANT( "CharSetIBMPC437",     CHARSET_IBMPC_437 ),
    SFX_USHORT_CONSTANT( "CharSetIBMPC850",     CHARSET_IBMPC_850 ),
    SFX_USHORT_CONSTANT( "CharSetIBMPC860",     CHARSET_IBMPC_860 ),
    SFX_USHORT_CONSTANT( "CharSetIBMPC861",     CHARSET_IBMPC_861 ),
    SFX_USHORT_CONSTANT( "CharSetIBMPC863",     CHARSET_IBMPC_863 ),
    SFX_USHORT_CONSTANT( "CharSetIBMPC865",     CHARSET_IBMPC_865 ),
    SFX_USHORT_CONSTANT( "CharSetIBMPC",        CHARSET_IBMPC ),
    SFX_USHORT_CONSTANT( "CharSetMac",          CHARSET_MAC ),
    SFX_USHORT_CONSTANT( "CharSetSymbol",       CHARSET_SYMBOL ),
    SFX_USHORT_CONSTANT( "CharSetSystem",       RTL_TEXTENCODING_UTF8 ),
    SFX_USHORT_CONSTANT( "FamilyDecorative",    FAMILY_DECORATIVE ),
    SFX_USHORT_CONSTANT( "FamilyDontknow",      FAMILY_DONTKNOW ),
    SFX_USHORT_CONSTANT( "FamilyModern",        FAMILY_MODERN ),
    SFX_USHORT_CONSTANT( "FamilyRoman",         FAMILY_ROMAN ),
    SFX_USHORT_CONSTANT( "FamilyScript",        FAMILY_SCRIPT ),
    SFX_USHORT_CONSTANT( "FamilySwiss",         FAMILY_SWISS ),
    SFX_USHORT_CONSTANT( "FamilySystem",        FAMILY_SYSTEM ),
    SFX_USHORT_CONSTANT( "GradientAxial",       GRADIENT_AXIAL ),
    SFX_USHORT_CONSTANT( "GradientElliptical",  GRADIENT_ELLIPTICAL ),
    SFX_USHORT_CONSTANT( "GradientLinear",      GRADIENT_LINEAR ),
    SFX_USHORT_CONSTANT( "GradientRadial",      GRADIENT_RADIAL ),
    SFX_USHORT_CONSTANT( "GradientRect",        GRADIENT_RECT ),
    SFX_USHORT_CONSTANT( "GradientSquare",      GRADIENT_SQUARE ),
    SFX_USHORT_CONSTANT( "ItalicNone",          ITALIC_NONE ),
    SFX_USHORT_CONSTANT( "ItalicOblique",       ITALIC_OBLIQUE ),
    SFX_USHORT_CONSTANT( "StrikeThroughDouble", STRIKEOUT_DOUBLE ),
    SFX_USHORT_CONSTANT( "StrikeThroughNone",   STRIKEOUT_NONE ),
    SFX_USHORT_CONSTANT( "StrikeThroughSingle", STRIKEOUT_SINGLE ),
    SFX_USHORT_CONSTANT( "UnderlineDotted",     UNDERLINE_DOTTED ),
    SFX_USHORT_CONSTANT( "UnderlineDouble",     UNDERLINE_DOUBLE ),
    SFX_USHORT_CONSTANT( "UnderlineNone",       UNDERLINE_NONE ),
    SFX_USHORT_CONSTANT( "UnderlineSingle",     UNDERLINE_SINGLE ),
    SFX_USHORT_CONSTANT( "UnitFoot",            FUNIT_FOOT ),
    SFX_USHORT_CONSTANT( "UnitInch",            FUNIT_INCH ),
    SFX_USHORT_CONSTANT( "UnitMile",            FUNIT_MILE ),
    SFX_USHORT_CONSTANT( "UnitPercent",         FUNIT_PERCENT ),
    SFX_USHORT_CONSTANT( "UnitPoint",           FUNIT_POINT ),
    SFX_USHORT_CONSTANT( "UnitTwip",            FUNIT_TWIP ),
    SFX_USHORT_CONSTANT( "Unit_cm",             FUNIT_CM ),
    SFX_USHORT_CONSTANT( "Unit_m",              FUNIT_M ),
    SFX_USHORT_CONSTANT( "Unit_mm",             FUNIT_MM ),
    SFX_USHORT_CONSTANT( "WeightBlack",         WEIGHT_BLACK ),
    SFX_USHORT_CONSTANT( "WeightBold",          WEIGHT_BOLD ),
    SFX_USHORT_CONSTANT( "WeightDontKnow",      WEIGHT_DONTKNOW ),
    SFX_USHORT_CONSTANT( "WeightLight",         WEIGHT_LIGHT ),
    SFX_USHORT_CONSTANT( "WeightMedium",        WEIGHT_MEDIUM ),
    SFX_USHORT_CONSTANT( "WeightNormal",        WEIGHT_NORMAL ),
    SFX_USHORT_CONSTANT( "WeightSemibold",      WEIGHT_SEMIBOLD ),
    SFX_USHORT_CONSTANT( "WeightSemilight",     WEIGHT_SEMILIGHT ),
    SFX_USHORT_CONSTANT( "WeightThin",          WEIGHT_THIN ),
    SFX_USHORT_CONSTANT( "WeightUltrabold",     WEIGHT_ULTRABOLD ),
    SFX_USHORT_CONSTANT( "WeightUltralight",    WEIGHT_ULTRALIGHT )
};
*/
//========================================================================

StarBASIC* SfxApplication::GetBasic_Impl() const
{
    return pImp->pBasicMgr ? pImp->pBasicMgr->GetLib(0) : NULL;
}

//------------------------------------------------------------------------
String lcl_GetVersionString(ResMgr* pImpResMgr)
{
    ResId aVerId( RID_BUILDVERSION, pImpResMgr );
    ResMgr *pResMgr = pImpResMgr->IsAvailable(
        aVerId.SetRT( RSC_STRING ) )
                    ? pImpResMgr
                    : 0;
    aVerId.SetResMgr( pResMgr );
    if ( !Resource::GetResManager()->IsAvailable( aVerId ) )
        DBG_ERROR( "No RID_BUILD_VERSION in label-resource-dll" );
    String aVersion( aVerId );
    aVersion.Erase( 0, aVersion.Search( ':' ) + 1 );
    aVersion.Erase( aVersion.Search( ')' ) );
    return aVersion;
}

//------------------------------------------------------------------------
/*(mba)
SbxObject* SfxSbxObjectFactory_Impl::CreateObject( const String &rType )
{
    if ( COMPARE_EQUAL == rType.CompareIgnoreCaseToAscii( "DocumentInfo" ) )
        return new SfxDocumentInfoObject_Impl;
    if ( COMPARE_EQUAL == rType.CompareIgnoreCaseToAscii( "Progress" ) )
        return SfxProgress::CreateObject_Impl();
    return 0;
}*/

//=========================================================================
/*
SfxSelectionObject::SfxSelectionObject
(
    SfxViewFrame *pFrame               // ein Frame als Parent
)

//  [Beschreibung]

//  interner Ctor f"ur Default-Behandlung


:   SfxShellSubObject( pFrame, SFX_TYPELIB_INFO(Selection), "Selection" ),
   _pFrame( pFrame )
{
    SetFlag( SBX_EXTSEARCH | SBX_DONTSTORE ); // unsichtbar
}

//-------------------------------------------------------------------------

SfxSelectionObject::SfxSelectionObject
(
    SfxViewFrame *pFrame,               // ein Frame als Parent
    const String& rClassName,
    const SfxTypeLibImpl &rTypeLibInfo
)

:   SfxShellSubObject( pFrame, rClassName, rTypeLibInfo, "Selection" ),
   _pFrame( pFrame )
{
    SetFlag( SBX_EXTSEARCH | SBX_DONTSTORE ); // unsichtbar
}

//-------------------------------------------------------------------------

SvDispatch* SfxSelectionObject::GetAggObject( sal_uInt16 nIdx )

{
    // selbst?
    if ( 0 == nIdx )
        return SfxShellSubObject::GetAggObject( 0 );

    // die oberhalb der ::com::sun::star::sdbcx::View auf dem Dispatcher befindlichen Shells indizieren
    SfxDispatcher *pDispat = _pFrame->GetDispatcher();
    SfxShell *pSh = pDispat->GetShell( nIdx-1 );
    if ( !pSh || pSh->ISA(SfxObjectShell) )
        return 0;
    return (SvDispatch*) pSh->GetSbxObject();
}

//-------------------------------------------------------------------------

SbxVariable* SfxSelectionObject::Find
(
    const String&   rName,
    SbxClassType    t
)
{
    // einige Member selbst bedienen
    String aName( rName.ToLowerAscii() );
    if ( aName == "name" || aName == "classname" )
        return SfxShellSubObject::Find( rName, t );

    // nur die oberhalb der ::com::sun::star::sdbcx::View auf dem Dispatcher liegenden Shells durchsuchen
    SfxDispatcher *pDispat = _pFrame->GetDispatcher();
    pDispat->Flush();
    SfxShell *pSh;
    for ( sal_uInt16 n = 0;
          ( pSh = pDispat->GetShell( n ) ) && !pSh->ISA(SfxViewFrame);
          ++n )
    {
        SbxObject *pObj = pSh->GetSbxObject();
        if ( pObj )
        {
            if ( ( t == SbxCLASS_DONTCARE || t == SbxCLASS_OBJECT ) &&
                 pObj->GetName() == rName )
                return pObj;
            else
            {
                SbxVariable *pRes = pObj->Find( rName, t );
                if ( pRes )
                    return pRes;
            }
        }
    }

    return 0;
}

//-------------------------------------------------------------------------

SbxVariable* SfxSelectionObject::FindUserData
(
    sal_uInt32 nId
)
{
    // nur die oberhalb der ::com::sun::star::sdbcx::View auf dem Dispatcher liegenden Shells durchsuchen
    SfxDispatcher *pDispat = _pFrame->GetDispatcher();
    SfxShell *pSh;
    for ( sal_uInt16 n = 0;
          ( pSh = pDispat->GetShell( n ) ) && !pSh->ISA(SfxViewFrame);
          ++n )
    {
        SbxObject *pObj = pSh->GetSbxObject();
        if ( pObj )
        {
            SbxVariable *pRes = pObj->FindUserData( nId );
            if ( pRes )
                return pRes;
        }
    }

    return 0;
}

//=========================================================================

class SfxConstants_Impl: public SbxObject
{
    const char*         _pPrefix;
    const SfxConstant*  _pConsts;
    sal_uInt16              _nCount;

public:
                        SfxConstants_Impl( const char *pPrefix,
                                           const SfxConstant *pConsts,
                                           sal_uInt16 n );
    SbxVariable*        Find( const String& rName, SbxClassType t );
};

//-------------------------------------------------------------------------

SfxConstants_Impl::SfxConstants_Impl
(
    const char*         pPrefix,
    const SfxConstant*  pConsts,
    sal_uInt16              nCount
)
:   SbxObject( "Constants" ),
    _pPrefix( pPrefix ),
    _pConsts( pConsts ),
    _nCount( nCount )
{
    SetFlag( SBX_EXTSEARCH | SBX_DONTSTORE ); // unsichtbar
    SetName( String( (long) this ) ); // random
}

//-------------------------------------------------------------------------

SbxVariable* SfxConstants_Impl::Find
(
    const String&   rName,
    SbxClassType    t
)
{
    // stimmt Typ und Prefix?
    String aPrefix;
    if ( _pPrefix )
        aPrefix += _pPrefix;
    if ( ( SbxCLASS_DONTCARE != t && SbxCLASS_PROPERTY != t ) ||
         ( aPrefix.Len() && aPrefix.Len() != rName.Match( _pPrefix ) ) )
        return 0;

    // schon drin?
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if( pRes )
        return pRes;

    // sonst suchen
    String aSearched( rName );
    aSearched.ToLowerAscii();
    for ( sal_uInt16 n = 0; n < _nCount; ++n )
    {
        // gefunden?
        const SfxConstant &rConst = _pConsts[n];
        String aName( aPrefix );
        aName += rConst.pName;
        if ( aSearched == aName.ToLowerAscii() )
        {
            // on-demand erzeugen
            SbxVariable *pVar = Make( aName, SbxCLASS_PROPERTY, rConst.eType );
            switch ( rConst.eType )
            {
                case SbxUSHORT:
                    pVar->PutUShort( (sal_uInt16)(int) rConst.pData );
                    break;

                case SbxCHAR:
                    pVar->PutChar( (char)(int) rConst.pData );
                    break;

                case SbxBYTE:
                    pVar->PutByte( (sal_Int8)(int) rConst.pData );
                    break;

                case SbxBOOL:
                    pVar->PutBool( (sal_Bool)(int) rConst.pData );
                    break;

                case SbxINTEGER:
                    pVar->PutInteger( (short)(int) rConst.pData );
                    break;

                case SbxLONG:
                    pVar->PutLong( (long) rConst.pData );
                    break;

                case SbxUINT:
                    pVar->PutUShort( (sal_uInt16)(int) rConst.pData );
                    break;

                case SbxULONG:
                    pVar->PutULong( (sal_uInt32) rConst.pData );
                    break;

                case SbxLPSTR:
                    pVar->PutString( (char*) rConst.pData );
                    break;

                default:
                    DBG_ERROR( "type of constant not supported" );
            }

            pVar->ResetFlag( SBX_WRITE );
            pVar->SetFlag( SBX_CONST );
            pVar->SetFlag( SBX_FIXED );
            return pVar;
        }
    }

    return 0;
}
*/
//=========================================================================
sal_uInt16 SfxApplication::SaveBasicManager() const
{
    // Save Dialog Container
    sal_Bool bComplete = sal_False;
    pImp->pDialogContainer->storeLibraries( bComplete );

    // MT: #47347# AppBasicDir ist jetzt ein PATH!
    // Ncht den vom BasicManager, falls inzwischen der Pfad geaendert wurde !?
    // So wird natuerlich auch das erste Dir genommen, wenn der BasicManager
    // vorher im zweiten gefunden wurde...
    String aBasicPath( SvtPathOptions().GetBasicPath() );
    INetURLObject aAppBasicObj( aBasicPath.GetToken(1) );
    aAppBasicObj.insertName( Application::GetAppName() );
    aAppBasicObj.setExtension( DEFINE_CONST_UNICODE( "sbl" ) );
    String aAppBasicPath( aAppBasicObj.GetMainURL() );
    SvStorageRef aStor = new SvStorage( aAppBasicPath );
    if ( aStor->GetError() == 0 )
    {
        String aBaseURL = INetURLObject::GetBaseURL();
        INetURLObject::SetBaseURL( aAppBasicObj.GetMainURL() );
        pImp->pBasicMgr->Store( *aStor );
        INetURLObject::SetBaseURL( aBaseURL );
    }
    if ( aStor->GetError() != 0 )
    {
        ErrorBox aBox(GetTopWindow(), SfxResId(MSG_ERR_WRITE_SBL));
        String aMsg( SearchAndReplace( aBox.GetMessText(), 0x0040, aAppBasicPath ) ); // 40h = '@'
        aBox.SetMessText( aMsg );
        aBox.Execute();
    }

    return (sal_uInt16)aStor->GetError();
}

//--------------------------------------------------------------------

void SfxApplication::RegisterBasicConstants
(
    const char*        pPrefix, // Prefix vor Konstanten-Namen
    const SfxConstant* pConsts, // Array von <SfxConstant> Instanzen
    sal_uInt16 nCount               // Anahl der Kontanten in pConsts
)

/*  [Beschreibung]

    Diese Methode meldet Konstanten beim BASIC an. Sie sollte on-demand
    (in GetSbxObject() der Applikation) gerufen werden. Das Array mu\s
    alphabetisch nach den Namen sortiert sein!

    Durch den Prefix kann Speicher gespart und das Suchen beschleunigt
    werden. Im StarOffice soll der Prefix "so" verwendet werden.


    [Beispiel]

    const SfxConstant __FAR_DATA aConstants[] =
    {
        SFX_BOOL_CONSTANT( "False", sal_False ),
        SFX_BOOL_CONSTANT( "True",  sal_True ),
    };

    ...
    SFX_APP()->RegisterBasicConstants( 0, aConstants, 2 );
    ...

*/

{
//  DBG_ASSERT( pImp->pBasicMgr, "no basic available" );

//  pImp->pBasicMgr->GetLib(0)->Insert(
//              new SfxConstants_Impl( pPrefix, pConsts, nCount ) );
}

//--------------------------------------------------------------------

SbxVariable* MakeVariable( StarBASIC *pBas, SbxObject *pObject,
           const char *pName, sal_uInt32 nSID, SbxDataType eType, SbxClassType eClassType )
{
    SbxVariable *pVar = pBas->Make( String::CreateFromAscii(pName), eClassType, eType ); //SbxCLASS_PROPERTY
    pVar->SetUserData( nSID );
    pVar->SetFlag( SBX_DONTSTORE );
    pObject->StartListening( pVar->GetBroadcaster() );
    return pVar;
}

//--------------------------------------------------------------------

BasicManager* SfxApplication::GetBasicManager()
{
//  DBG_ASSERT( pAppData_Impl->nBasicCallLevel != 0,
//              "unnecessary call to GetBasicManager() - inefficient!" );
    if ( pAppData_Impl->nBasicCallLevel == 0 )
        // sicherheitshalber
        EnterBasicCall();

    if ( !pImp->pBasicMgr )
    {
        // Directory bestimmen
        SvtPathOptions aPathCFG;
        String aAppBasicDir( aPathCFG.GetBasicPath() );
        if ( !aAppBasicDir.Len() )
            aPathCFG.SetBasicPath( String::CreateFromAscii("$(prog)") );

        // #58293# soffice.new nur im ::com::sun::star::sdbcx::User-Dir suchen => erstes Verzeichnis
        String aAppFirstBasicDir = aAppBasicDir.GetToken(1);
        sal_Bool bBasicUpdated = sal_False;

        // Basic erzeugen und laden
        // MT: #47347# AppBasicDir ist jetzt ein PATH!
        INetURLObject aAppBasic( SvtPathOptions().SubstituteVariable( String::CreateFromAscii("$(progurl)") ) );
        aAppBasic.insertName( Application::GetAppName() );
        aAppBasic.setExtension( DEFINE_CONST_UNICODE( "sbl" ) );
        String aAppBasicFile, aNewBasicFile;

        // Direkt nach der Installation gibt es ggf. _nur_ eine SOFFICE.NEW
        aAppBasicFile = aAppBasic.getName();
        if ( !aPathCFG.SearchFile( aAppBasicFile, SvtPathOptions::PATH_BASIC ) )
        {
            INetURLObject aNewBasic = aAppBasic;
            aNewBasic.setExtension( DEFINE_CONST_UNICODE( "new" ) );

            aNewBasicFile = aNewBasic.getName();
            if( aPathCFG.SearchFile( aNewBasicFile, SvtPathOptions::PATH_BASIC ) )
            {
                aAppBasic = INetURLObject( aNewBasicFile );
                aAppBasic.setExtension( DEFINE_CONST_UNICODE( "sbl" ) );
                SfxContentHelper::MoveTo( aNewBasicFile, aAppBasic.GetMainURL() );
                bBasicUpdated = sal_True;
            }
        }
        else
        {
            aAppBasic = INetURLObject( aAppBasicFile );
        }

        SvStorageRef aStor = new SvStorage( aAppBasic.GetMainURL(), STREAM_READ | STREAM_SHARE_DENYWRITE );
        SfxBasicManager* pSfxBasicManager;
        if ( aStor.Is() && 0 == aStor->GetError() )
        {
            SfxErrorContext aErrContext( ERRCTX_SFX_LOADBASIC, Application::GetAppName() );
            String aOldBaseURL = INetURLObject::GetBaseURL();
            INetURLObject::SetBaseURL( aAppBasic.GetMainURL() );
            pSfxBasicManager = new SfxBasicManager( *aStor, NULL, &aAppBasicDir );
            pImp->pBasicMgr = pSfxBasicManager;
            pImp->pBasicMgr->SetStorageName( aAppBasic.PathToFileName() );

            // ggf. nach einem Channel-Update den BasicManager aktualisieren
            INetURLObject aNewBasic = aAppBasic;
            aNewBasic.setExtension( DEFINE_CONST_UNICODE( "new" ) );

            aNewBasicFile = aNewBasic.getName();
            if( aPathCFG.SearchFile( aNewBasicFile, SvtPathOptions::PATH_BASIC ) )
            {
                aNewBasic = INetURLObject( aNewBasicFile );
                SvStorageRef xTmpStor = new SvStorage( aNewBasic.PathToFileName(), STREAM_READ | STREAM_SHARE_DENYWRITE );
                pImp->pBasicMgr->Merge( *xTmpStor );
                bBasicUpdated = sal_True;
                xTmpStor.Clear();   // Sonst kein Kill() moeglich
                SfxContentHelper::Kill( aNewBasicFile ); // SOFFICE.NEW loeschen
            }
            INetURLObject::SetBaseURL( aOldBaseURL );
            if ( pImp->pBasicMgr->HasErrors() )
            {
                // handle errors
                BasicError *pErr = pImp->pBasicMgr->GetFirstError();
                while ( pErr )
                {
                    // show message to user
                    if ( ERRCODE_BUTTON_CANCEL ==
                         ErrorHandler::HandleError( pErr->GetErrorId() ) )
                    {
                        // user wants to break loading of BASIC-manager
                        delete pImp->pBasicMgr;
                        aStor.Clear();
                        break;
                    }
                    pErr = pImp->pBasicMgr->GetNextError();
                }
            }
        }
        if ( !aStor.Is() || 0 != aStor->GetError() )
        {
            pSfxBasicManager = new SfxBasicManager( new StarBASIC, &aAppBasicDir );
            pImp->pBasicMgr = pSfxBasicManager;

            // Als Destination das erste Dir im Pfad:
            String aFileName( aAppBasic.getName() );
            aAppBasic = INetURLObject( aAppBasicDir.GetToken(1) );
            DBG_ASSERT( aAppBasic.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
            aAppBasic.insertName( aFileName );
            pImp->pBasicMgr->SetStorageName( aAppBasic.PathToFileName() );
        }

        aStor.Clear();
        if ( bBasicUpdated )
            SaveBasicManager();

        // globale Variablen
        StarBASIC *pBas = pImp->pBasicMgr->GetLib(0);
        sal_Bool bBasicWasModified = pBas->IsModified();
/*
        MakeVariable( pBas, pAppObj, "ActiveModule", SID_ACTIVEMODULE|0x20000  );
        MakeVariable( pBas, pAppObj, "ActiveDocument", SID_ACTIVEDOCUMENT|0x20000 );
        MakeVariable( pBas, pAppObj, "ActiveWindow", SID_ACTIVEWINDOW|0x20000 );
        MakeVariable( pBas, pAppObj, "ActiveTask", SID_ACTIVETASK|0x20000 );
        MakeVariable( pBas, pAppObj, "Application", SID_APPLICATION );
        MakeVariable( pBas, pAppObj, "Selection", SID_SELECTION|0x20000 );
        MakeVariable( pBas, pAppObj, "Documents", SID_DOCUMENTS|0x40000 );
        MakeVariable( pBas, pAppObj, "Windows", SID_WINDOWS|0x40000 );
        MakeVariable( pBas, pAppObj, "StarWriter", SID_STARWRITER|0x20000 );
        MakeVariable( pBas, pAppObj, "StarDraw", SID_STARDRAW|0x20000 );
        MakeVariable( pBas, pAppObj, "StarCalc", SID_STARCALC|0x20000 );
        MakeVariable( pBas, pAppObj, "LanguageId", SID_LANGUAGEID, SbxUSHORT );
        MakeVariable( pBas, pAppObj, "CountryId", SID_COUNTRYID, SbxUSHORT );
        // MI: wenn der drin ist, findet BASIC die Datenbank-Wizard Makros
        // nicht mehr - warum auch immer
        MakeVariable( pBas, pAppObj, "ThisDocument", SID_THISDOCUMENT|0x20000 );
        MakeVariable( pBas, pAppObj, "ThisWindow", SID_THISWINDOW|0x20000 );
        MakeVariable( pBas, pAppObj, "ThisComponent", 0x20000  );
        MakeVariable( pBas, pAppObj, "SubstPathVars", SID_SUBSTPATHVARS,SbxSTRING, SbxCLASS_METHOD );
*/
        // ::com::sun::star::frame::Desktop
        Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr = ::comphelper::getProcessServiceFactory();
        Any aDesktop;
        Reference< XDesktop > xDesktop( xSMgr->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")), UNO_QUERY );
        aDesktop <<= xDesktop ;
        SbxObjectRef xUnoObj = GetSbUnoObject( DEFINE_CONST_UNICODE("StarDesktop"), aDesktop );
        xUnoObj->SetFlag( SBX_DONTSTORE );
        pBas->Insert( xUnoObj );
        //pBas->setRoot( xDesktop );

        // Dialog container
        SfxDialogContainer* pDlgCont = new SfxDialogContainer( sal_True );
        pDlgCont->acquire();    // Hold via UNO
        pImp->pDialogContainer = pDlgCont;
        pSfxBasicManager->SetDialogContainer( pImp->pDialogContainer );

        Reference< XLibraryContainer > xDlgCont = static_cast< XLibraryContainer* >( pDlgCont );
        Any aDlgCont;
        aDlgCont <<= xDlgCont;
        xUnoObj = GetSbUnoObject( DEFINE_CONST_UNICODE("Dialogs"), aDlgCont );
        pBas->Insert( xUnoObj );

        // Konstanten
//ASDBG     RegisterBasicConstants( "so", aConstants, sizeof(aConstants)/sizeof(SfxConstant) );

        // Durch MakeVariable wird das Basic modifiziert.
        if ( !bBasicWasModified )
            pBas->SetModified( sal_False );
    }
    return pImp->pBasicMgr;
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxApplication::GetDialogContainer()
{
    Reference< XLibraryContainer > xRet
        = static_cast< XLibraryContainer* >( pImp->pDialogContainer );
    return xRet;
}

//--------------------------------------------------------------------

StarBASIC* SfxApplication::GetBasic()
{
    return GetBasicManager()->GetLib(0);
}

//--------------------------------------------------------------------

FASTBOOL SfxApplication::IsInBasicCall() const
{
    return 0 != pAppData_Impl->nBasicCallLevel;
}

//--------------------------------------------------------------------

void SfxApplication::EnterBasicCall()
{
    if ( 1 == ++pAppData_Impl->nBasicCallLevel )
    {
        DBG_TRACE( "SfxShellObject: BASIC-on-demand" );

        // das kann l"anger dauern, da Progress nicht geht, wenigstens Sanduhr
//(mba)/task        SfxWaitCursor aWait;

        // zuerst das BASIC laden
        GetBasic();
/*
        // als erstes SfxShellObject das SbxObject der SfxApplication erzeugen
        SbxObject *pSbx = GetSbxObject();
        DBG_ASSERT( pSbx, "SfxShellObject: can't create SbxObject for SfxApplication" );

        // die SbxObjects aller Module erzeugen
        SfxModuleArr_Impl& rArr = GetModules_Impl();
        for ( sal_uInt16 n = 0; n < rArr.Count(); ++n )
        {
            SfxModule *pMod = rArr.GetObject(n);
            if ( pMod->IsLoaded() )
            {
                pSbx = pMod->GetSbxObject();
                DBG_ASSERT( pSbx, "SfxModule: can't create SbxObject" );
            }
        }

        // die SbxObjects aller Tasks erzeugen
        for ( SfxTask *pTask = SfxTask::GetFirst(); pTask; pTask = SfxTask::GetNext( *pTask ) )
            pTask->GetSbxObject();

        // die SbxObjects aller SfxObjectShells erzeugen (ggf. Frame-los!)
        for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst( NULL, sal_False );
              pObjSh;
              pObjSh = SfxObjectShell::GetNext(*pObjSh, NULL, sal_False) )
        {
            // kein IP-Object oder wenn doch dann initialisiert?
            SvStorageRef aStorage;
            if ( !pObjSh->IsHandsOff() )
                aStorage = pObjSh->GetStorage();
            if ( !pObjSh->GetInPlaceObject() || aStorage.Is() )
            {
                DBG( DbgOutf( "SfxShellObject: BASIC-on-demand for %s",
                              pObjSh->SfxShell::GetName().GetBuffer() ) );
                pSbx = pObjSh->GetSbxObject();
                DBG_ASSERT( pSbx, "SfxShellObject: can't create SbxObject" );
            }
        }

        // die SbxObjects der SfxShells auf den Stacks der Frames erzeugen
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst(0,0,sal_False);
              pFrame;
              pFrame = SfxViewFrame::GetNext(*pFrame,0,0,sal_False) )
        {
            // den Dispatcher des Frames rausholen
            SfxDispatcher *pDispat = pFrame->GetDispatcher();
            pDispat->Flush();

            // "uber alle SfxShells auf dem Stack des Dispatchers iterieren
            // Frame selbst wird ausgespart, da er indirekt angezogen wird,
            // sofern er ein Dokument enth"alt.
            for ( sal_uInt16 nStackIdx = pDispat->GetShellLevel(*pFrame);
                  0 != nStackIdx;
                  --nStackIdx )
            {
                DBG( DbgOutf( "SfxShellObject: BASIC-on-demand for level %u", nStackIdx-1 ); )
                pSbx = pDispat->GetShell(nStackIdx - 1)->GetSbxObject();
                DBG_ASSERT( pSbx, "SfxShellObject: can't create SbxObject" );
            }

            if ( !pFrame->GetObjectShell() )
            {
                DBG( DbgOutf( "SfxShellObject: BASIC-on-demand for empty frame" ); )
                pSbx = pFrame->GetSbxObject();
                DBG_ASSERT( pSbx, "SfxShellObject: can't create SbxObject" );
            }
        }
*/
        // Factories anmelden
//        SbxBase::AddFactory( new SfxSbxObjectFactory_Impl );
    }
}

//--------------------------------------------------------------------

void SfxApplication::LeaveBasicCall()
{
    --pAppData_Impl->nBasicCallLevel;
}

//--------------------------------------------------------------------

void SfxApplication::EventExec_Impl( SfxRequest &rReq, SfxObjectShell *pObjSh )
{
    // SID nur einmal holen
    sal_uInt16 nSID = rReq.GetSlot();

    SFX_REQUEST_ARG(rReq, pItem, SfxStringItem, nSID, sal_False);
    if ( pItem )
    {
        String aArg( pItem->GetValue() );
        GetEventConfig()->ConfigureEvent( nSID, aArg, pObjSh );
    }
}

//-------------------------------------------------------------------------

void SfxApplication::EventState_Impl
(
    sal_uInt16          nSID,
    SfxItemSet&     rSet,
    SfxObjectShell* pObjSh
)
{
    // Config auslesen
    SfxMacroConfig *pMC = GetMacroConfig();
    SfxEventConfiguration *pEC = GetEventConfig();
    const SvxMacro* pMacro = pEC->GetMacroForEventId( nSID, pObjSh );

    // "Library.Modul.Method" zusammensetzen
    String aRet;
    if ( pMacro )
        aRet = pMacro->GetMacName();
    rSet.Put( SfxStringItem( nSID, aRet ) );
}

//-------------------------------------------------------------------------
void SfxApplication::PropExec_Impl( SfxRequest &rReq )
{
#if SUPD<613//MUSTINI
    SfxIniManager *pIniMgr = GetIniManager();
#endif
    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16 nSID = rReq.GetSlot();
    switch ( nSID )
    {
        case SID_CREATE_BASICOBJECT:
        {
            SFX_REQUEST_ARG(rReq, pItem, SfxStringItem, nSID, sal_False);
            if ( pItem )
            {
                SbxObject* pObject = SbxBase::CreateObject( pItem->GetValue() );
                pObject->AddRef();
//(mba)                rReq.SetReturnValue( SfxObjectItem( 0, pObject ) );
                rReq.Done();
            }
            break;
        }

        case SID_DELETE_BASICOBJECT:
        {
            SFX_REQUEST_ARG(rReq, pItem, SfxObjectItem, nSID, sal_False);
            if ( pItem )
            {
//(mba)                SbxObject* pObject = pItem->GetObject();
//(mba)                pObject->ReleaseRef();
            }
            break;
        }

        case SID_WIN_POSSIZE:
        {
            break;
        }

        case SID_DEFAULTFILEPATH:
        {
            SFX_REQUEST_ARG(rReq, pPathItem, SfxStringItem, nSID, sal_False);
#if SUPD<613//MUSTINI
            pIniMgr->Set(pPathItem->GetValue(), SFX_KEY_WORK_PATH );
#else
            SvtPathOptions().SetWorkPath( pPathItem->GetValue() );
#endif
            break;
        }

        case SID_INTERACTIVEMODE:
        {
            break;
        }

        case SID_ATTR_UNDO_COUNT:
        {
            SFX_REQUEST_ARG(rReq, pCountItem, SfxUInt16Item, nSID, sal_False);
            SvtUndoOptions().SetUndoCount( pCountItem->GetValue() );
            break;
        }

#if 0
        case SID_ATTR_METRIC:
        {
            SFX_REQUEST_ARG(rReq, pMetricItem, SfxUInt16Item, nSID, sal_False);
            GetOptions().SetMetric( (FieldUnit) pMetricItem->GetValue() );
            break;
        }
#endif

        case SID_WIN_VISIBLE:
        {
            break;
        }

        case SID_ON_STARTAPP:
        case SID_ON_CLOSEAPP:
        case SID_ON_CREATEDOC:
        case SID_ON_OPENDOC:
        case SID_ON_PREPARECLOSEDOC:
        case SID_ON_CLOSEDOC:
        case SID_ON_SAVEDOC:
        case SID_ON_SAVEASDOC:
        case SID_ON_ACTIVATEDOC:
        case SID_ON_DEACTIVATEDOC:
        case SID_ON_PRINTDOC:
        case SID_ON_SAVEDOCDONE:
        case SID_ON_SAVEASDOCDONE:
            EventExec_Impl( rReq, 0 );
            break;

        case SID_STATUSBARTEXT:
        {
            SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, nSID, sal_False);
            String aText = pStringItem->GetValue();
            if ( aText.Len() )
                GetpApp()->ShowStatusText( aText );
            else
                GetpApp()->HideStatusText();
            break;
        }

        case SID_HELP:
        {
            break;
        }

        case SID_PLAYMACRO:
            PlayMacro_Impl( rReq, GetBasic() );
            break;

        case SID_OFFICE_PALK:
        {
            SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, nSID, sal_False);
            if ( pStringItem )
            {
                String aPALK = pStringItem->GetValue();
                sal_Bool bCheck = SvDemo::CheckPALK(aPALK, String());
                rReq.SetReturnValue(SfxBoolItem(nSID, bCheck ));
                if( aPALK.Len() && bCheck )
                    SvDemo::SetAuthorizedState(Application::GetAppName(), aPALK );
            }
            else
                SbxBase::SetError( SbxERR_WRONG_ARGS );
            break;
        }

#if SUPD<613//MUSTINI
        case SID_OFFICE_PRIVATE_USE:
        case SID_OFFICE_COMMERCIAL_USE:
        {
            SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, nSID, sal_False);

            if ( pStringItem )
            {
                String aValue = pIniMgr->Get( SFX_KEY_USINGOFFICE );

                if ( !aValue.Len() || aValue.Len() != 2 )
                    aValue = DEFINE_CONST_UNICODE("00");

                sal_uInt16 nPos = SID_OFFICE_PRIVATE_USE == nSID ? 0 : 1;
                String aNew = pStringItem->GetValue().Copy( 0, 1 );
                aValue.Replace( nPos, aNew.Len(), aNew );
                pIniMgr->Set( aValue, SFX_KEY_USINGOFFICE );
            }
            break;
        }
#else
        case SID_OFFICE_PRIVATE_USE:
        case SID_OFFICE_COMMERCIAL_USE:
        {
            DBG_ASSERT( sal_False, "SfxApplication::PropExec_Impl()\nSID_OFFICE_PRIVATE_USE & SID_OFFICE_COMMERCIAL_USE are obsolete!\n" );
            break;
        }
#endif
        case SID_OFFICE_CUSTOMERNUMBER:
        {
            SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, nSID, sal_False);

            if ( pStringItem )
#if SUPD<613//MUSTINI
                pIniMgr->Set( pStringItem->GetValue(),
                              SFX_KEY_CUSTOMERNUMBER );
#else
                SvtUserOptions().SetCustomerNumber( pStringItem->GetValue() );
#endif
            break;
        }
    }
}

//-------------------------------------------------------------------------
void SfxApplication::PropState_Impl( SfxItemSet &rSet )
{
    SfxViewFrame *pFrame = SfxViewFrame::Current();
#if SUPD<613//MUSTINI
    SfxIniManager *pIniMgr = GetIniManager();
#endif
    SfxWhichIter aIter(rSet);
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {
            case SID_THISDOCUMENT:
            {
                rSet.Put( SfxObjectItem( SID_THISDOCUMENT, pAppData_Impl->pThisDocument ? pAppData_Impl->pThisDocument : SfxObjectShell::Current() ) );
                break;
            }

            case SID_THISWINDOW:
            {
                rSet.Put( SfxObjectItem( SID_THISWINDOW, pAppData_Impl->pThisDocument ? SfxViewFrame::GetFirst( pAppData_Impl->pThisDocument ) : SfxViewFrame::Current() ) );
                break;
            }
            case SID_PROGNAME:
                rSet.Put( SfxStringItem( SID_PROGNAME, GetName() ) );
                break;

            case SID_ACTIVEDOCUMENT:
                rSet.Put( SfxObjectItem( SID_ACTIVEDOCUMENT, SfxObjectShell::Current() ) );
                break;

            case SID_APPLICATION:
                rSet.Put( SfxObjectItem( SID_APPLICATION, this ) );
                break;

            case SID_STARWRITER:
            {
                SfxModule *pMod = (*(SfxModule**) GetAppData(SHL_WRITER))->Load();
                if ( pMod ) HACK(muss vom Modul selbst gemacht werden)
                    pMod->SetName( DEFINE_CONST_UNICODE("StarWriter") );
                rSet.Put( SfxObjectItem( nSID, pMod ) );
                break;
            }

            case SID_STARDRAW:
            {
                SfxModule *pMod = (*(SfxModule**) GetAppData(SHL_DRAW))->Load();
                if ( pMod ) HACK(muss vom Modul selbst gemacht werden)
                    pMod->SetName( DEFINE_CONST_UNICODE("StarDraw") );
                rSet.Put( SfxObjectItem( nSID, pMod  ) );
                break;
            }

            case SID_STARCALC:
            {
                SfxModule *pMod = (*(SfxModule**) GetAppData(SHL_CALC))->Load();
                if ( pMod ) HACK(muss vom Modul selbst gemacht werden)
                    pMod->SetName( DEFINE_CONST_UNICODE("StarCalc") );
                rSet.Put( SfxObjectItem( nSID, pMod  ) );
                break;
            }

            case SID_ACTIVEMODULE:
            {
                SfxShell *pSh = GetActiveModule();
                if ( !pSh )
                    pSh = this;
                rSet.Put( SfxObjectItem( SID_ACTIVEMODULE, pSh ) );
                break;
            }

            case SID_ACTIVEWINDOW:
                rSet.Put( SfxObjectItem( SID_ACTIVEWINDOW, pFrame ) );
                break;

            case SID_SELECTION:
                break;

            case SID_WIN_POSSIZE:
            {
                break;
            }

            case SID_CAPTION:
                break;

            case SID_DEFAULTFILEPATH:
#if SUPD<613//MUSTINI
                rSet.Put( SfxStringItem( SID_DEFAULTFILEPATH, pIniMgr->Get(SFX_KEY_WORK_PATH) ) );
#else
                rSet.Put( SfxStringItem( SID_DEFAULTFILEPATH, SvtPathOptions().GetWorkPath() ) );
#endif
                break;

            case SID_PROGFILENAME:
                rSet.Put( SfxStringItem( SID_PROGFILENAME, Application::GetAppFileName() ) );
                break;

            case SID_PROGPATH:
#if SUPD<613//MUSTINI
                rSet.Put( SfxStringItem( SID_PROGPATH, pIniMgr->GetProgramPath() ) );
#else
                rSet.Put( SfxStringItem( SID_PROGPATH, SvtPathOptions().SubstituteVariable( String::CreateFromAscii("$(prog)") ) ) );
#endif
                break;

            case SID_INTERACTIVEMODE:
                rSet.Put( SfxBoolItem( SID_INTERACTIVEMODE, !IsDispatcherLocked() ) );
                break;

            case SID_ATTR_UNDO_COUNT:
                rSet.Put( SfxUInt16Item( SID_ATTR_UNDO_COUNT, SvtUndoOptions().GetUndoCount() ) );
                break;

#if 0
            case SID_ATTR_METRIC:
                rSet.Put( SfxByteItem( SID_ATTR_METRIC, (sal_Int8) GetOptions().GetMetric() ) );
                break;
#endif

            case SID_WIN_VISIBLE:
                break;

            case SID_ON_STARTAPP:
            case SID_ON_CLOSEAPP:
            case SID_ON_CREATEDOC:
            case SID_ON_OPENDOC:
            case SID_ON_PREPARECLOSEDOC:
            case SID_ON_CLOSEDOC:
            case SID_ON_SAVEDOC:
            case SID_ON_SAVEASDOC:
            case SID_ON_ACTIVATEDOC:
            case SID_ON_DEACTIVATEDOC:
            case SID_ON_PRINTDOC:
            case SID_ON_SAVEDOCDONE:
            case SID_ON_SAVEASDOCDONE:
                EventState_Impl( nSID, rSet, 0 );

            case SID_UPDATE_VERSION:
                rSet.Put( SfxUInt32Item( SID_UPDATE_VERSION, SUPD ) );
                break;

            case SID_BUILD_VERSION:
            {
                String aVersion = lcl_GetVersionString(pAppData_Impl->pLabelResMgr);
                rSet.Put( SfxUInt32Item( SID_BUILD_VERSION, (sal_uInt32) aVersion.ToInt32() ) );
                break;
            }

            case SID_STATUSBARTEXT:
            {
                rSet.Put( SfxStringItem( SID_STATUSBARTEXT, GetStatusBarManager()->GetStatusBar()->GetText() ) );
                break;
            }
            case SID_OFFICE_VERSION_ID:
            {
                String sVersionString;
                ResId aVerId( STR_VERSION_ID, pAppData_Impl->pLabelResMgr );
                if(pAppData_Impl->pLabelResMgr->IsAvailable(    aVerId.SetRT( RSC_STRING ) ))
                    sVersionString += String(aVerId);
                else
                    sVersionString += DEFINE_CONST_UNICODE("50"); // wenn man keine Iso-Dll haben sollte und vorher nicht abgeraucht ist
                String sOS(S2U(getenv("OS")));
                String sProc(S2U(getenv("CPU")));

                sal_uInt16 nSystem = SYSTEM_ID;
                if(nSystem < 10)
                    sVersionString += 0x0030 ; // '0' = 30h
                sVersionString += String::CreateFromInt32( nSystem );
                String sBuildVersion = lcl_GetVersionString(pAppData_Impl->pLabelResMgr);
                sal_uInt16 nLen = sBuildVersion.Len();
                if(nLen > 4)
                {
                    DBG_ERROR("Buildversion laenger als 4 Zeichen ?")
                    sBuildVersion.Erase(4, nLen - 4);
                }
                else if(nLen < 4)
                {
                    DBG_ERROR("Buildversion kuerzer als 4 Zeichen ?")

                    while(5 > nLen++)
                        sBuildVersion.Insert(DEFINE_CONST_UNICODE("0"), 0);
                }
                sVersionString += sBuildVersion;
                String sResStr(S2U(ResMgr::GetLang()));
                if( sResStr.Len() < 2)
                    sResStr.Insert(DEFINE_CONST_UNICODE("0"), 0);
                sVersionString += sResStr;

                aVerId = ResId( STR_VERSION_TYPE, pAppData_Impl->pLabelResMgr );
                if(pAppData_Impl->pLabelResMgr->IsAvailable(    aVerId.SetRT( RSC_STRING ) ))
                    sVersionString += String(aVerId);
                else
                    sVersionString += DEFINE_CONST_UNICODE("99"); // wenn man keine Iso-Dll haben sollte

                sal_uInt16 nDemoKind = GetDemoKind();
                switch(nDemoKind)
                {
                    case  SFX_DEMOKIND_FULL  : sVersionString += '1'; break;
                    case  SFX_DEMOKIND_TRYBUY: sVersionString += '2'; break;
                    case  SFX_DEMOKIND_DEMO  : sVersionString += '3'; break;
                    default: sVersionString += '0';
                }

                sVersionString += Application::IsRemoteServer() ? '1' : '0';
                rSet.Put(SfxStringItem(nSID, sVersionString));

            }
            break;

            case  SID_OFFICE_PLK:
            {
                String sPLK(SvDemo::GetPLK());
                rSet.Put(SfxStringItem(nSID, sPLK));
            }
            break;

            case SID_OLD_PALK:
            {
                rSet.Put(SfxStringItem(nSID, SvDemo::GetOldPALK()));
            }
            break;

#if SUPD<613//MUSTINI
            case SID_OFFICE_PRIVATE_USE:
            case SID_OFFICE_COMMERCIAL_USE:
            {
                String aUsing = pIniMgr->Get( SFX_KEY_USINGOFFICE );

                if ( !aUsing.Len() || aUsing.Len() != 2 )
                    aUsing = DEFINE_CONST_UNICODE("00");

                sal_uInt16 nPos = SID_OFFICE_PRIVATE_USE == nSID ? 0 : 1;
                String aRet = aUsing.Copy( nPos, 1 );
                rSet.Put( SfxStringItem( nSID, aRet ) );
                break;
            }
#else
            case SID_OFFICE_PRIVATE_USE:
            case SID_OFFICE_COMMERCIAL_USE:
            {
                DBG_ASSERT( sal_False, "SfxApplication::PropState_Impl()\nSID_OFFICE_PRIVATE_USE & SID_OFFICE_COMMERCIAL_USE are obsolete!\n" );
                break;
            }
#endif

            case SID_OFFICE_CUSTOMERNUMBER:
            {
#if SUPD<613//MUSTINI
                String aCustomerNumber = pIniMgr->Get(SFX_KEY_CUSTOMERNUMBER);
                rSet.Put( SfxStringItem( nSID, aCustomerNumber ) );
#else
                rSet.Put( SfxStringItem( nSID, SvtUserOptions().GetCustomerNumber() ) );
#endif
                break;
            }
        }
    }
}

//--------------------------------------------------------------------
void SfxApplication::MacroExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();
    if ( SfxMacroConfig::IsMacroSlot( rReq.GetSlot() ) )
    {
        // SlotId referenzieren, damit nicht im Execute der Slot abgeschossen
        // werden kann
        GetMacroConfig()->RegisterSlotId(rReq.GetSlot());
        SFX_REQUEST_ARG(rReq, pArgs, SfxStringItem,
                        rReq.GetSlot(), sal_False);
        String aArgs;
        if( pArgs ) aArgs = pArgs->GetValue();
        if ( GetMacroConfig()->ExecuteMacro(rReq.GetSlot(), aArgs ) )
            rReq.Done();
        GetMacroConfig()->ReleaseSlotId(rReq.GetSlot());
    }
}

//--------------------------------------------------------------------
void SfxApplication::MacroState_Impl( SfxItemSet& rSet )
{
    DBG_MEMTEST();

   // Gestrichen, da zu teuer! Ausserdem waere ein EnterBasicCall noetig
/*
    const sal_uInt16 *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges && *pRanges, "Set ohne Bereich");
    while ( *pRanges )
    {
        for(sal_uInt16 nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            sal_Bool bOK = sal_False;
            if ( SfxMacroConfig::IsMacroSlot( nWhich ) )
                bOK = GetMacroConfig()->CheckMacro(nWhich);

            if ( !bOK )
                rSet.DisableItem(nWhich);
        }

        ++pRanges;
    }
 */
}

//-------------------------------------------------------------------------

void SfxApplication::PlayMacro_Impl( SfxRequest &rReq, StarBASIC *pBasic )
{
    EnterBasicCall();
    sal_Bool bOK = sal_False;

    // Makro und asynch-Flag
    SFX_REQUEST_ARG(rReq,pMacro,SfxStringItem,SID_STATEMENT,sal_False);
    SFX_REQUEST_ARG(rReq,pAsynch,SfxBoolItem,SID_ASYNCHRON,sal_False);

    if ( pAsynch && pAsynch->GetValue() )
    {
        // asynchron ausf"uhren
        GetDispatcher_Impl()->Execute( SID_PLAYMACRO, SFX_CALLMODE_ASYNCHRON, pMacro, 0L );
        rReq.Done();
    }
    else if ( pMacro )
    {
        // Statement aufbereiten
        DBG_ASSERT( pBasic, "no BASIC found" ) ;
        String aStatement( '[' );
        aStatement += pMacro->GetValue();
        aStatement += ']';

        // P"aventiv den Request abschlie\sen, da er ggf. zerst"ort wird
        rReq.Done();
        rReq.ReleaseArgs();

        // Statement ausf"uhren
        SbxVariable* pRet = pBasic->Execute( aStatement );
        bOK = 0 == SbxBase::GetError();
        SbxBase::ResetError();
    }

    LeaveBasicCall();
    rReq.SetReturnValue(SfxBoolItem(0,bOK));
}


