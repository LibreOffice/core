/*************************************************************************
 *
 *  $RCSfile: doctempl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:32 $
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

#include <limits.h>
#if ! defined( MAC ) && ! defined( UNX )
#include <io.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#include <tools/fsys.hxx>
#include <tools/string.hxx>

#include "doctempl.hxx"
#include "docfac.hxx"
#include "docfile.hxx"
#include "objsh.hxx"
#include "inimgr.hxx"
#include "sfxtypes.hxx"
#include "app.hxx"
#include "inimgr.hxx"
#include "sfxresid.hxx"
#include "docfilt.hxx"
#include "fltfnc.hxx"
#include "doc.hrc"
#include "ucbhelp.hxx"
#include "openflag.hxx"

//========================================================================

const char __FAR_DATA   pHeader[] =         "Sfx Document Template Directory";
const char __FAR_DATA   pFileFileName[] =   "sfx.tpl";
const char __FAR_DATA   pDirFileName[] =    "sfx.tlx";
const char __FAR_DATA   pBakExtension[] =   "bak";
const char              pMGName[] =         "mg.bad";
const char*             cDefExtension =     "*.vor";
const char              cDefWildcard =      '*';

#define CTRL_Z  ((char)26)

#define VERSION 4

// Wird nur noch gebraucht, um die Datein beim Scannen zu "uberspringen
// LongNameList_Impl needs this function, too!
const String FileName_Impl(BOOL bDir)
{
    return bDir ? String::CreateFromAscii( pDirFileName ) :
                  String::CreateFromAscii( pFileFileName );
}

// ------------------------------------------------------------------------
class LongNameList_Impl : public List
{
    List*   mpNames;

public:
             LongNameList_Impl( const String& rDirName );
            ~LongNameList_Impl();

    String  GetLongName( const String& rShortName ) const;
    BOOL    ReadHeader( SvStream& rStream ) const;
};

//------------------------------------------------------------------------
class OpenNotifier_Impl : public SfxListener
{
    BOOL                    bEnd;
//(dv)  CntAnchorRef            xAnchor;
//(dv)  CntOpenMode             eMode;

public:
                            OpenNotifier_Impl( /*(dv)const CntAnchorRef& rAnchor, CntOpenMode eOpenMode */ )
                                : bEnd( FALSE )
//(dv)                              , xAnchor( rAnchor )
//(dv)                              , eMode( eOpenMode )
                            {
#if 0   //(dv)
                                StartListening( *xAnchor );
                                xAnchor->Put( SfxVoidItem( WID_GETDATA ) );
                                xAnchor->MakeVisible( TRUE );
                                xAnchor->MarkAsRoot();
                                xAnchor->DontThread();
                                xAnchor->Put( CntOpenModeItem(WID_OPEN,eMode) );
#endif  //(dv)
                            }

    virtual void            Notify( SfxBroadcaster&, const SfxHint& );
    BOOL                    IsComplete() const
                            { return bEnd; }
};

void OpenNotifier_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
#if 0   //(dv)
    if( rHint.ISA(CntStatusHint) )
    {
        const CntStatusHint& rStatusHint = (const CntStatusHint&)rHint;
        USHORT nWhich = rStatusHint.GetRequest() ? rStatusHint.GetRequest()->Which() : 0;
        const CntStatus eStatus = rStatusHint.GetStatus();
        if ( nWhich == WID_OPEN  && ( eStatus == CNT_STATUS_DONE || eStatus == CNT_STATUS_ERROR ) )
        {
            bEnd = TRUE;
            EndListening( *xAnchor );
        }
    }
#endif  //(dv)
}

class SfxTemplateDir;


class SfxTemplateDirEntry
{
//(dv)  CntAnchorRef        xAnchor;
    String              aLongName;  // logischer Name des Benutzers
    String              aFileName;  // Dateiname (ohne Pfad)
    SfxTemplateDir*     pParent;    // Parentverzeichnis
    DirEntry*           pPath;      // vollstaendiger Pfad ohne Name, wenn Dir.
    SfxTemplateDir*     pContent;
    SfxObjectShellLock  xObjShell;
    SvStorageRef        xStor;
    BOOL                bDidConvert;
    BOOL                bIsOwner;
    BOOL                bDefault;
    BOOL                bIsRoot;

public:
                            SfxTemplateDirEntry( const String &rName,
                                    SfxTemplateDir &rParent );
                            SfxTemplateDirEntry( const String &rName,
                                    const String &rFileName,
                                    SfxTemplateDir &rParent );
                            SfxTemplateDirEntry( const String &rName,
                                    const DirEntry &rPath );
                            SfxTemplateDirEntry( const String &rName,
                                    const String &rFileName,
                                    const DirEntry &rPath, BOOL bRoot = FALSE );
                            SfxTemplateDirEntry( const SfxTemplateDirEntry & );
                            ~SfxTemplateDirEntry();

    BOOL                    IsLoaded(){ return pContent!= 0; }
    BOOL                    IsDefault() { return bDefault; }
    BOOL                    IsRoot() { return bIsRoot; }

    const SfxTemplateDir&   GetParent() const { return *pParent; }
    SfxTemplateDir&         GetParent() { return *pParent; }
    SfxTemplateDir&         GetContent();
    void                    SetContent(SfxTemplateDir *pCnt);

    const String&           GetLongName() const { return aLongName; }
    const String&           GetFileName() const { return aFileName; }
    void                    SetLongName(const String &rStr) {aLongName = rStr;}
    BOOL                    SetLongAndFileName( const String &rLong);
    void                    SetFileName(const String &rStr) { aFileName = rStr; }
    DirEntry                GetFull() const;
    DirEntry                GetPath() const;
    SfxObjectShellRef       CreateObjectShell();
    SfxObjectShellRef       GetObjectShell() const { return (SfxObjectShellRef)(SfxObjectShell*) xObjShell; }
    void                    SetObjectShell( SfxObjectShell *pSh )
                            { xObjShell = pSh; }
    BOOL                    DeleteObjectShell();
#if 0   //(dv)
    void                    SetAnchor( const CntAnchorRef& rAnchor )
                            { xAnchor = rAnchor; }
#endif  //(dv)
};

//------------------------------------------------------------------------

typedef SfxTemplateDirEntry* SfxTemplateDirEntryPtr;
SV_DECL_PTRARR_DEL(_SfxTemplateDir, SfxTemplateDirEntryPtr, 16, 8)
SV_IMPL_PTRARR(_SfxTemplateDir, SfxTemplateDirEntryPtr)

//------------------------------------------------------------------------

class SfxTemplateDir: public _SfxTemplateDir
{
    DirEntry        aPath;          // vollstaendiger Pfad zum Verzeichnis
    BOOL            bDir;           // Directory?

public:
    const SfxTemplateDirEntry*  Find_Impl(const DirEntry &rDirEntry) const;

                                SfxTemplateDir()
                                    : bDir ( FALSE )
                                {}

                                SfxTemplateDir( const DirEntry &rPath )
                                    : aPath(rPath)
                                    , bDir( FALSE )
                                {}

                                SfxTemplateDir( const DirEntry &rPath,
                                        BOOL bDirectory, BOOL bScan=2 );
                                SfxTemplateDir( const SfxTemplateDir & );

    void                        Scan(BOOL bDirectory, BOOL bSave = FALSE);
    const DirEntry&             GetPath() const { return aPath; }
    DirEntry&                   GetPath() { return aPath; }

    // anhand des neuen TemplateDirs aktualisieren
    BOOL                        Freshen(const SfxTemplateDir &rNew);
    BOOL                        IsDir()
                                    {return bDir;}
};

//------------------------------------------------------------------------

void MakeFileName_Impl(DirEntry &rEntry,
                              const String &rLongName, BOOL bDir)
{
    String aFName(rLongName);
    if(!bDir)
        aFName+= DEFINE_CONST_UNICODE( ".vor" );
    rEntry.MakeShortName(aFName, FSYS_KIND_NONE);
}

//------------------------------------------------------------------------

SfxTemplateDirEntry::SfxTemplateDirEntry
(
    const String&       rName,
    SfxTemplateDir&     rP
)

/*  [Beschreibung]

    Konstuktor;
    der Dateiname und der logische Name sind identisch

*/

:   aLongName(rName),
    aFileName(rName),
    pContent(0),
    pPath(0),
    pParent(&rP),
    bIsRoot( FALSE ),
    bDefault( FALSE )
{}

//------------------------------------------------------------------------

SfxTemplateDirEntry::SfxTemplateDirEntry
(
    const String&       rName,
    const String&       rFileName,
    SfxTemplateDir&     rP
)
/*  [Beschreibung]

    Konstuktor;
    der Dateiname und der logische Name sind unterschiedlich

*/

:   aLongName(rName),
    aFileName(rFileName),
    pContent(0),
    pPath(0),
    pParent(&rP),
    bIsRoot( FALSE ),
    bDefault( FALSE )
{}

//------------------------------------------------------------------------

SfxTemplateDirEntry::SfxTemplateDirEntry
(
    const String &rName,    //  Der Stream, aus dem gelesen werden soll
    const DirEntry &rP      //  Parentverzeichnis
)

/*  [Beschreibung]

    Konstuktor;
    der Dateiname und der logische Name sind identisch
*/

:   aLongName(rName),
    aFileName(rName),
    pContent(0),
    pPath(new DirEntry(rP)),
    pParent(0),
    bIsRoot( FALSE ),
    bDefault( FALSE )
{}

//------------------------------------------------------------------------

SfxTemplateDirEntry::SfxTemplateDirEntry
(
    const String&       rName,
    const String&       rFileName,
    const DirEntry&     rP,          // Parentverzeichnis
    BOOL bRoot
)

/*  [Beschreibung]

    Konstuktor;
    der Dateiname und der logische Name sind unterschiedlich
*/

:   aLongName(rName),
    aFileName(rFileName),
    pContent(0),
    pPath(new DirEntry(rP)),
    pParent(0),
    bIsRoot( bRoot ),
    bDefault( FALSE )
{}

//------------------------------------------------------------------------

SfxTemplateDirEntry::SfxTemplateDirEntry(const SfxTemplateDirEntry &rCopy)

/*  [Beschreibung]

    Copy Konstruktor

*/

:   aLongName(rCopy.aLongName),
    aFileName(rCopy.aFileName),
    pParent(rCopy.pParent),
    pPath(rCopy.pPath? new DirEntry(*rCopy.pPath): 0),
    pContent(rCopy.pContent? new SfxTemplateDir(*rCopy.pContent): 0),
    bDefault( rCopy.bDefault )
{}

//------------------------------------------------------------------------

SfxTemplateDirEntry::~SfxTemplateDirEntry()

/*  [Beschreibung]

    Destruktor

*/

{
    DeleteObjectShell();
    delete pContent;
    delete pPath;
    pParent = 0;
}

//------------------------------------------------------------------------

SfxObjectShellRef SfxTemplateDirEntry::CreateObjectShell()

/*  [Beschreibung]

    Anlegen einer DokumentShell zu diesem Entry; das Entry beinhaltet
    den Dateinamen.


    [R"uckgabewert]

    SfxObjectShellRef     Referenz auf die DokumentShell


    [Querverweise]

    <SfxTemplateDirEntry::DeleteObjectShell()>
*/

{
    if(!xObjShell.Is())
    {
        bIsOwner=FALSE;
        BOOL bDum = FALSE;
        SfxApplication *pSfxApp = SFX_APP();
        xObjShell = pSfxApp->DocAlreadyLoaded(GetFull().GetFull(),TRUE,bDum);
        if(!xObjShell.Is())
        {
            bIsOwner=TRUE;
            SfxMedium *pMed=new SfxMedium(
                GetFull().GetFull(),(STREAM_READ | STREAM_SHARE_DENYWRITE),
                FALSE, TRUE, 0);
            const SfxFilter* pFilter = NULL;
            if( pSfxApp->GetFilterMatcher().GuessFilter(
                *pMed, &pFilter, SFX_FILTER_TEMPLATE, 0 ) ||
                pFilter && !pFilter->IsOwnFormat() ||
                pFilter && !pFilter->UsesStorage() )
            {
                SfxErrorContext aEc(ERRCTX_SFX_LOADTEMPLATE,
                                    GetFull().GetFull(
                                        FSYS_STYLE_HOST,FALSE,20));
                delete pMed;
                bDidConvert=TRUE;
                ULONG lErr;
                if ( xObjShell.Is() )
                    if(lErr=(pSfxApp->LoadTemplate(
                        xObjShell,GetFull().GetFull())!=ERRCODE_NONE))
                        ErrorHandler::HandleError(lErr);

            }
            else
            {
                const SfxObjectFactory &rFactory =
                    ((SfxFactoryFilterContainer*)pFilter->GetFilterContainer())
                    ->GetFactory();
                delete pMed;
                bDidConvert=FALSE;
                xStor = new SvStorage(
                    GetFull().GetFull(),
                    STREAM_READWRITE | STREAM_NOCREATE |
                    STREAM_SHARE_DENYALL, STORAGE_TRANSACTED);
                if ( pFilter )
                    xStor->SetVersion( pFilter->GetVersion() );
                if ( SVSTREAM_OK == xStor->GetError() )
                {
                    xObjShell = (SfxObjectShell *)
                        rFactory.CreateObject(SFX_CREATE_MODE_ORGANIZER);
                    if ( xObjShell.Is() )
                    {
                        xObjShell->DoInitNew(0);
                        if(!xObjShell->LoadFrom(xStor))
                            xObjShell.Clear();
                        else
                        {
                            xObjShell->DoHandsOff();
                            xObjShell->DoSaveCompleted(xStor);
                        }
                    }
                }
            }
        }
    }
    return (SfxObjectShellRef)(SfxObjectShell*) xObjShell;
}

//------------------------------------------------------------------------

BOOL SfxTemplateDirEntry::DeleteObjectShell()

/*  [Beschreibung]

    "oschen der DokumentShell dieses Entrys, sofern eine DokumentShell
    angelegt wurde


    [R"uckgabewert]

    BOOL                FALSE:  es ist ein Fehler beim Speichern aufgetreten;
                        TRUE:   das Dokument wurde gespeichert


    [Querverweise]

    <SfxTemplateDirEntry::CreateObjectShell()>
*/

{
    BOOL bRet = TRUE;
    if ( xObjShell.Is() )
    {
        if( xObjShell->IsModified() )
        {
            //Hier speichern wir auch, falls die Vorlage in Bearbeitung ist...
            bRet=FALSE;
            if ( bIsOwner )
                if( bDidConvert)
                {
                    bRet=xObjShell->PreDoSaveAs_Impl(
                        GetFull().GetFull(),
                        xObjShell->GetFactory().GetFilter(0)->GetName(),0 );
                }
                else
                {
                if(xObjShell->Save())
                    bRet=xStor->Commit();
                else
                    bRet=FALSE;
                }
        }
        if(bRet)
        {
            xObjShell.Clear();
            xStor.Clear();
        }
    }
    return bRet;
}

//------------------------------------------------------------------------

BOOL SfxTemplateDirEntry::SetLongAndFileName(const String &rStr)

/*  [Beschreibung]

    Setzen der logischen Namens; geschieht dies auf einem Device,
    da"s lange Namen unterst"utzt, wird auch die Datei physikalisch
    auf diesen logischen Namen umbenannt.


    [Querverweise]
    <SfxDocTemplateDirEntry::GetLongName()>
*/

{
    if(aLongName==rStr)
        return TRUE;
    DirEntry aActName(GetFull());
    DirEntry aNew(aActName.GetPath());

    MakeFileName_Impl(aNew,rStr,!pParent || pParent->IsDir());

    String aNewFileName(aNew.GetFull());

    if(aNew==aActName)
    {
        aLongName=rStr;
        aFileName = aNewFileName;
        return TRUE;
    }

    FSysError eErr;
    eErr=aActName.MoveTo(aNew);
    if ( FSYS_ERR_OK == eErr )
    {
        aLongName = rStr;
        aFileName = DirEntry( aNewFileName ).GetName();

        // Pfade in den Verwaltungsdaten anpassen
        if ( pContent )
        {
            const USHORT nCount = pContent->Count();
            for ( USHORT i = 0; i < nCount; ++i )
            {
                SfxTemplateDir *pParent = (*pContent)[i]->pParent;
                if ( pParent )
                    pParent->GetPath().SetName(aFileName);
            }
            pContent->GetPath().SetName(aFileName);
        }

        return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------------------

DirEntry SfxTemplateDirEntry::GetPath() const

/*  [Beschreibung]

    Liefert den Pfad zu diesem Eintry (Pfad + FileName == vollst"andiger
    Pfad zu der diesem Entry zugeordneten Datei)

    [Querverweise]
    <SfxDocTemplateDirEntry::GetFull()>
*/

{
    return pParent? pParent->GetPath(): *pPath;
}

//------------------------------------------------------------------------

DirEntry SfxTemplateDirEntry::GetFull() const

/*  [Beschreibung]

    Liefert den Pfad plus Dateiname zu diesem Eintry

    [Querverweise]
    <SfxDocTemplateDirEntry::GetPath()>
*/

{
      DirEntry aEntry(GetPath());
      aEntry += DirEntry(aFileName);
      return aEntry;
}

//------------------------------------------------------------------------

SfxTemplateDir &SfxTemplateDirEntry::GetContent()

/*  [Beschreibung]

    Zugriff auf den Inhalt eines Bereiches; der Bereich wird erst
    bei Bedarf eingelesen.

*/

{
    DBG_ASSERT(!pParent || pParent->IsDir(), "Ist kein Dir");
    if ( !pContent )
        pContent = new SfxTemplateDir(GetFull(), FALSE);
    return *pContent;
}



//------------------------------------------------------------------------

void SfxTemplateDirEntry::SetContent(SfxTemplateDir *pCnt)

/*  [Beschreibung]

    Setzt den Inhalt eines Bereiches; der alte Bereich wird
    gel"oscht

*/

{
    delete pContent;
    pContent = pCnt;
}

//------------------------------------------------------------------------

SfxTemplateDirEntry* SeekEntry_Impl(SfxTemplateDir &rDir, const String &rLongName, USHORT * = 0);

SfxTemplateDirEntry* SeekEntry_Impl
(
    SfxTemplateDir&     rDir,       //  Das Directory, in dem gesucht werden soll
    const String&       rLongName,  /*  der logische Name, dessen Eintrag gefunden
                                        werden soll */

    USHORT*             pUS         /*  Optional, enth"alt, falls gefunden, den Index
                                        des Eintrags in rDir. */

)

/*  [Beschreibung]

    Sucht einen Eintrag anhand seines logischen Namens


    [R"uckgabewert]

    SfxTemplateDirEntry*    Pointer auf den gefundenen Eintrag oder NULL,
                            wenn nicht gefunden.

*/

{
    BOOL bCaseSensitive = rDir.GetPath().IsCaseSensitive();
    const USHORT nCount = rDir.Count();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        const String& rName = rDir[i]->GetLongName();
        if (  bCaseSensitive && rName == rLongName ||
             !bCaseSensitive && rName.CompareIgnoreCaseToAscii( rLongName ) == COMPARE_EQUAL )
        {
            if ( pUS )
                *pUS = i;
            return rDir[i];
        }
    }
    return 0;
}

//------------------------------------------------------------------------

SfxTemplateDir::SfxTemplateDir
(
    const DirEntry& rPath,
    BOOL            bDirectory,
    BOOL            bScan
)

/*  [Beschreibung]

    Konstruktor
    Wenn Directory, alle Verzeichnisse dieses Directories
    sonst alle Vorlagen in diesem Directory

    [Querverweise]
    <SfxDocTemplateDir::Scan()>
    <SfxDocTemplateDir::Load(const DirEntry &rFile, BOOL bDirectory)>
*/

:   _SfxTemplateDir()
    ,bDir(bDirectory)
    ,aPath(rPath)
{
    if(!FileStat(rPath).IsKind(FSYS_KIND_DIR)) return;

    if( bScan==2 || !bScan )
        Scan(bDirectory, TRUE);
    else
        Scan(bDirectory, FALSE);
}

//------------------------------------------------------------------------

void  SfxTemplateDir::Scan
(
    BOOL    bDirectory,     // Flag, Directory oder Dateien in diesem Directory
    BOOL    bSave           // Flag, speichern der sfx.tpl Files
)

/*  [Beschreibung]

    Liest ein Directory / den Inhalt eines Directorys von Platte ein
*/

{
    LongNameList_Impl   *pDirList = NULL;
    DirEntry aActDir;
    aActDir.ToAbs();
    DirEntry aTmpPath(aPath);
    aTmpPath.SetCWD();
    const String aDirWild('*');
    const String aDefDirWild(cDefWildcard);
    const DirEntry aWild(bDirectory? aDirWild: aDefDirWild);
    Dir aDir(aWild, bDirectory? FSYS_KIND_DIR: FSYS_KIND_FILE, FSYS_SORT_END);
    DBG_ASSERT(aDir.Count() < USHRT_MAX, "zu viele DirEntraege");
    const USHORT nCount = (USHORT)aDir.Count();
    if(bDirectory)
    {
         const SfxTemplateDirEntryPtr pE=new SfxTemplateDirEntry(SfxResId(STR_STANDARD),String(),aTmpPath,TRUE);
         Insert(pE, 0 );
         pDirList = new LongNameList_Impl( aPath.GetFull( FSYS_STYLE_URL, TRUE ) );
    }

    for ( USHORT i = 0; i < nCount; ++i )
    {
        const DirEntry aDirEntry(aDir[i]);
        const String aFileName(aDirEntry.GetName());
        if ( aFileName.Len() &&
             aFileName.CompareIgnoreCaseToAscii(pFileFileName)!=COMPARE_EQUAL &&
             aFileName.CompareIgnoreCaseToAscii(pDirFileName)!=COMPARE_EQUAL &&
             aFileName.GetChar(0) != '.' )
        {
            const SfxFilter* ppFilter = NULL;

            if ( !bDirectory )
            {
/*
                SfxMedium aSfxMedium( aDirEntry.GetName(),
                                      STREAM_READ | STREAM_SHARE_DENYNONE,
                                      FALSE );
                if( SFX_APP()->GetFilterMatcher().GuessFilter(
                                    aSfxMedium,
                                    &ppFilter,
                                    SFX_FILTER_IMPORT | SFX_FILTER_TEMPLATE )
                    || !ppFilter
                    || !ppFilter->IsAllowedAsTemplate() )
                   continue;
 */
            }
            else if ( aFileName.CompareIgnoreCaseToAscii( "wizard" ) == COMPARE_EQUAL ||
                      aFileName.CompareIgnoreCaseToAscii( "internal" ) == COMPARE_EQUAL )
                // hide internal template folder
                continue;

            String aLongName;

            // try to get the long name of the entry. To get the long name of files
            // we have to look into these files by getting the document info. The
            // long names of directories are stored in a file named sfx.tlx. The
            // pDirList should contain the data of that file. When we haven't found
            // a long name, the long name will be set to the file name.
            if ( !bDirectory )
            {
                DirEntry aName( aPath );
                aName += aFileName;
                SvStorageRef aStor = new SvStorage( aName.GetFull(), SFX_STREAM_READONLY );
                if ( aStor->GetError() == SVSTREAM_OK )
                {
                    SfxDocumentInfo aInfo;
                    BOOL bOK = aInfo.Load( aStor );
                    if ( bOK )
                        aLongName = aInfo.GetTitle();
                }

                if ( !aLongName.Len() )
                    aLongName = aName.GetBase();
            }
            else
            {
                if ( pDirList )
                    aLongName = pDirList->GetLongName( aFileName );
                if ( !aLongName.Len() )
                    aLongName = aFileName;
            }

            const SfxTemplateDirEntryPtr pEntry = bDirectory?
                new SfxTemplateDirEntry(aLongName, aFileName, aTmpPath):
                new SfxTemplateDirEntry(aLongName, aFileName, *this);
            const String &rLongName = pEntry->GetLongName();
            const International aInter(GetpApp()->GetAppInternational());
            const USHORT nCount = Count();
                // initial sortiert einfuegen
            USHORT ii;
            for ( ii = bDirectory ? 1 : 0; //Ist Standard mit drinnen?
                  ii < Count() && COMPARE_LESS ==
                        aInter.Compare( (*this)[ii]->GetLongName(), rLongName);
                  ++ii)
                ;
            Insert(pEntry, ii);
        }
    }

    if ( bSave && Count() )
    {
        DirEntry aFile(aTmpPath);
    }

    DirEntry aDumDir(FSYS_FLAG_ABSROOT);
    aDumDir.SetCWD();
    aActDir.SetCWD();

    delete pDirList;
}

//------------------------------------------------------------------------

SfxTemplateDir::SfxTemplateDir ( const SfxTemplateDir &rCopy ) :

/*  [Beschreibung]

    Copy- Konstruktor

*/

    _SfxTemplateDir( (BYTE)rCopy.Count(), 1 ),

    bDir( rCopy.bDir ),
    aPath( rCopy.aPath )
{
    const USHORT nCount = rCopy.Count();
    for (USHORT i = 0; i < nCount; ++i)
    {
        const SfxTemplateDirEntryPtr pEntry =
            new SfxTemplateDirEntry( *rCopy[i] );
        Insert( pEntry, i );
    }
}

//------------------------------------------------------------------------

const SfxTemplateDirEntry *SfxTemplateDir::Find_Impl
(
    const DirEntry& rDirEntry   /*  der Dateiname, dessen zugeordneter Eintrag
                                    gefunden werden soll. */

)   const

/*  [Beschreibung]

    Sucht einen Eintrag anhand seines Dateinamens


    [R"uckgabewert]                 Pointer auf den gefundenen Eintrag oder NULL,
                                    wenn nicht gefunden.

*/

{
    const USHORT nCount = Count();
    for (USHORT i = 0; i < nCount; ++i)
    {
        const SfxTemplateDirEntry *pEntry = (*this)[i];
//      const DirEntry aFileName(pEntry->GetFileName());
        if(pEntry->GetFull() == rDirEntry)
            return pEntry;
    }
     return 0;
}

//------------------------------------------------------------------------

BOOL SfxTemplateDir::Freshen
(
    const SfxTemplateDir&   rNew    /*  Das Directory, gegen das dieses Directoy
                                        abgeglichen werden soll */

)

/*  [Beschreibung]

    Dieses Directory anhand des TemplateDirs rNew aktualisieren
    (Aktualisieren aus Benutzersicht; das aktuelle Directory
    wird gegen ein neu eingelesenes abgeglichen


    [R"uckgabewert]

    BOOL                    TRUE
                            wurde modifiziert

                            FALSE
                            wurde nicht modifiziert
*/

{
    USHORT i = 0;
    BOOL bModified = FALSE;
    // alle die entfernen, wo die Datei physikalisch nicht
    // mehr vorhanden ist
    while(i < Count())
    {
        SfxTemplateDirEntry *pEntry = (*this)[i];
        if(!rNew.Find_Impl(pEntry->GetFull()))
        {
            Remove(i, 1);
            delete pEntry;
            bModified = TRUE;
        }
        else
            ++i;
    }
    // alle die aufnehmen, wo der Dateiname aus rNew in der aktuellen
    // Version nicht vorhanden ist
    const USHORT nCount = rNew.Count();
    for(i = 0; i < nCount; ++i)
    {
        const SfxTemplateDirEntry *pEntry = rNew[i];
        if(!Find_Impl(pEntry->GetFull()))
        {
            SfxTemplateDirEntry *pNew = new SfxTemplateDirEntry(*pEntry);
/*          if(&pEntry->GetParent())
                pNew->GetParent()=*this;*/
            Insert(pNew, Count());
            bModified = TRUE;
        }
    }
    return bModified;
}

//------------------------------------------------------------------------

String SfxDocumentTemplates::GetFullRegionName
(
    USHORT nIdx                     // Index des Bereiches
)   const

/*  [Beschreibung]

    Liefert den logischen Namen eines Bereiches Plus seinem  Pfad


    [R"uckgabewert]                 Referenz auf diesen Namen

*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    String aName=(*pDirs)[nIdx]->GetLongName();

    USHORT nCount=pDirs->Count();
    for(USHORT i=0;i<nCount;i++)
    {
        if(i!=nIdx && (*pDirs)[i]->GetLongName()==aName)
        {
            DirEntry aParent( (*pDirs)[nIdx]->GetFull() );
            if ( !(*pDirs)[nIdx]->IsRoot() )
                aParent.CutName();
            aName += DEFINE_CONST_UNICODE( " (" );
            aName += aParent.GetFull( FSYS_STYLE_HOST, FALSE, 10 );
            aName += DEFINE_CONST_UNICODE( ")" );
            break;
        }
    }
    return aName;
}

//------------------------------------------------------------------------

const String& SfxDocumentTemplates::GetRegionName
(
    USHORT nIdx                 // Index des Bereiches
)   const

/*  [Beschreibung]

    Liefert den logischen Namen eines Bereiches


    [R"uckgabewert]

    const String&                   Referenz auf diesen Namen

*/
{
    DBG_ASSERT( pDirs, "not initialized" );
    return (*pDirs)[nIdx]->GetLongName();
}


//------------------------------------------------------------------------

USHORT SfxDocumentTemplates::GetRegionNo
(
    const String &rRegion       // Name der Region
)   const

/*  [Beschreibung]

    Liefert den Index f"ur einen logischen Namen eines Bereiches.


    [R"uckgabewert]

    USHORT          Index von 'rRegion' oder USHRT_MAX falls unbekannt

*/
{
    DBG_ASSERT( pDirs, "not initialized" );
    for ( USHORT n = 0; n < pDirs->Count(); ++n )
        if ( (*pDirs)[n]->GetLongName() == rRegion )
            return n;
    return USHRT_MAX;
}


//------------------------------------------------------------------------

USHORT SfxDocumentTemplates::GetRegionCount() const

/*  [Beschreibung]

    Liefert die Anzahl der Bereiche


    [R"uckgabewert]

    USHORT                  Anzahl der Bereiche

*/
{
     return pDirs->Count();
}

//------------------------------------------------------------------------

USHORT SfxDocumentTemplates::GetCount
(
    const String&   rName   /*  Name des Bereiches, dessen Eintrags-
                                anzahl ermittelt werden soll */

)   const

/*  [Beschreibung]

    Liefert die Anzahl der Eintr"age eines Bereiches


    [R"uckgabewert]

    USHORT                      Anzahl der Eintr"age

*/

{
    DBG_ASSERT( pDirs, "not initialized" );

    SfxTemplateDirEntry *pEntry = SeekEntry_Impl(*pDirs, rName);
    DBG_ASSERT(pEntry , "Eintrag nicht gefunden");
    return pEntry? pEntry->GetContent().Count(): 0;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::IsRegionLoaded( USHORT nIdx ) const
{
    return (*pDirs)[nIdx]->IsLoaded();
}

//------------------------------------------------------------------------

USHORT SfxDocumentTemplates::GetCount
(
    USHORT nRegion              /*  Index des Bereiches, dessen Eintrags-
                                    anzahl ermittelt werden soll */

)   const

/*  [Beschreibung]

    Liefert die Anzahl der Eintr"age eines Bereiches


    [R"uckgabewert]                 Anzahl der Eintr"age

*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    DBG_ASSERT(nRegion < GetRegionCount(), "Regions ueberindiziert");
    return (*pDirs)[nRegion]->GetContent().Count();
}

//------------------------------------------------------------------------

const String& SfxDocumentTemplates::GetName
(
    USHORT nRegion,     //  Index des Bereiches, in dem der Eintrag liegt
    USHORT nIdx         //  Index des Eintrags
)   const

/*  [Beschreibung]

    Liefert den logischen Namen eines Eintrags eines Bereiches


    [R"uckgabewert]

    const String&           Name des Eintrags

*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    return (*pDirs)[nRegion]->GetContent()[nIdx]->GetLongName();
}

//------------------------------------------------------------------------

const String& SfxDocumentTemplates::GetFileName
(
    USHORT nRegion,     //  Index des Bereiches, in dem der Eintrag liegt
    USHORT nIdx         //  Index des Eintrags
)   const

/*  [Beschreibung]

    Liefert den Dateinamen eines Eintrags eines Bereiches

    [R"uckgabewert]                 Dateiname des Eintrags

*/
{
    DBG_ASSERT( pDirs, "not initialized" );
    return (*pDirs)[nRegion]->GetContent()[nIdx]->GetFileName();
}

//------------------------------------------------------------------------

String SfxDocumentTemplates::GetPath
(
    USHORT  nRegion,    //  Index des Bereiches, in dem der Eintrag liegt
    USHORT  nIdx        //  Index des Eintrags
)   const

/*  [Beschreibung]

    Liefert den Dateinamen mit vollst"andigem Pfad zu der einem
    Eintrag zugeordneten Datei


    [R"uckgabewert]

    String                  Dateiname mit vollst"andigem Pfad

*/
{
    DBG_ASSERT( pDirs, "not initialized" );
    SfxTemplateDirEntry *pDirectory = (*pDirs)[nRegion];
    SfxTemplateDirEntry *pEntry =  pDirectory->GetContent()[nIdx];
    return pEntry->GetFull().GetFull();
}

//------------------------------------------------------------------------

String SfxDocumentTemplates::GetTemplatePath
(
    USHORT          nRegion,    //  Index des Bereiches, in dem der Eintrag liegt
    const String&   rLongName   //  logischer Name des Eintrags
)   const

/*  [Beschreibung]

    Liefert den Dateinamen mit vollst"andigem Pfad zu der einem
    Eintrag zugeordneten Datei


    [R"uckgabewert]

    String                          Dateiname mit vollst"andigem Pfad

*/
{
    DBG_ASSERT( pDirs, "not initialized" );
    SfxTemplateDirEntry *pDirectory = (*pDirs)[nRegion];
    DirEntry aPath(pDirectory->GetFull());
    const SfxTemplateDirEntry *pEntry = SeekEntry_Impl(pDirectory->GetContent(), rLongName);

    // Vorlage mit diesem Langnamen existiert bereits
    // => deren Dateiname muss verwendet werden
    if(pEntry)
        aPath += DirEntry(pEntry->GetFileName());
    else
        // Dateiname generieren
        MakeFileName_Impl(aPath, rLongName, pDirectory->GetContent().IsDir());
    aPath.ToAbs();
    return aPath.GetFull();
}

//------------------------------------------------------------------------

String SfxDocumentTemplates::GetDefaultTemplatePath
(
    const String&
    rLongName
)

/*  [Beschreibung]

    Liefert den Standardpfad zu Dokumentvorlagen


    [R"uckgabewert]

    String                  Standardpfad zu Dokumentvorlagen

*/
{
    DBG_ASSERT( pDirs, "not initialized" );
    const String aSep(';');
    cDelim = aSep.GetChar(0);
    DBG_ASSERT(aDirs.GetTokenCount(cDelim), "Keine Bereiche");
    DirEntry aPath(aDirs.GetToken(0, cDelim));

    // Verzeichnis anlegen
    if(!aPath.MakeDir())
        return String();
    MakeFileName_Impl(aPath, rLongName, TRUE);
    SfxTemplateDir  *pEntry = new SfxTemplateDir;
    SfxTemplateDirEntryPtr pDirEntry =
        new SfxTemplateDirEntry( String( '.' ), aPath.GetPath() );
    pDirEntry->SetContent(new SfxTemplateDir(aPath.GetPath()));
    pEntry->Insert(pDirEntry, pEntry->Count());
    pDirs->Insert(pEntry, pDirs->Count());
    return aPath.GetFull();
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::SaveDir
(
    SfxTemplateDir& rDir        //  das zu speichernde Directory
)

/*  [Beschreibung]

    Speichert das Directory rDir


    [R"uckgabewert]

    BOOL                        FALSE,
                                Schreibfehler

                                TRUE
                                gespeichert

*/

{
    return TRUE;
}

//------------------------------------------------------------------------

void SfxDocumentTemplates::NewTemplate
(
    USHORT          nRegion,    /*  Index des Bereiches, in dem die Vorlage
                                    angelegt werden soll */

    const String&   rLongName,  //  logischer Name der neuen Vorlage
    const String&   rFileName   //  Dateiname der neuen Vorlage
)

/*  [Beschreibung]

    Eintragen einer neuen Dokumentvorlage in die Verwaltungsstrukturen
    Das "Uberschreiben einer Vorlage gleichen Namens wird
    verhindert (!! Fehlermeldung)

*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    SfxTemplateDirEntry *pDirectory = (*pDirs)[nRegion];
    SfxTemplateDir &rEntry = pDirectory->GetContent();

    // Eintrag existiert bereits
    if(SeekEntry_Impl(pDirectory->GetContent(), rLongName))
        return;
    const SfxTemplateDirEntryPtr pNewEntry =
        new SfxTemplateDirEntry(rLongName, rFileName, rEntry);
    rEntry.Insert(pNewEntry, rEntry.Count());
    SaveDir(rEntry);
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::CopyOrMove
(
    USHORT  nTargetRegion,      //  Index des Zielbereiches
    USHORT  nTargetIdx,         //  Index Zielposition
    USHORT  nSourceRegion,      //  Index des Quellbereiches
    USHORT  nSourceIdx,         /*  Index der zu kopierenden / zu verschiebenden
                                    Dokumentvorlage */
    BOOL    bMove               //  kopieren / verschieben
)

/*  [Beschreibung]

    Kopieren oder Verschieben einer Dokumentvorlage


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef2uhrt werden

    [Querverweise]

    <SfxDocumentTemplates::Move(USHORT,USHORT,USHORT,USHORT)>
    <SfxDocumentTemplates::Copy(USHORT,USHORT,USHORT,USHORT)>
*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    if(nSourceIdx == USHRT_MAX) // keine Verzeichnisse verschieben, kopieren
        return FALSE ;
    SfxTemplateDirEntry *pTargetDirectory = (*pDirs)[nTargetRegion];
    SfxTemplateDirEntry *pSourceDirectory = (*pDirs)[nSourceRegion];
     SfxTemplateDir &rTargetDir = pTargetDirectory->GetContent();
    SfxTemplateDir &rSourceDir = pSourceDirectory->GetContent();
    SfxTemplateDirEntry *pSourceEntry = rSourceDir[nSourceIdx];

    // Verschieben einer Vorlage innerhalb eines Bereiches
    // --> nur Verwaltungsdaten aktualisieren
    if ( bMove && nTargetRegion == nSourceRegion && nSourceIdx != USHRT_MAX )
    {
        if(nTargetIdx == USHRT_MAX)
            nTargetIdx = 0;
        const SfxTemplateDirEntryPtr pEntry = rTargetDir[nSourceIdx];
        rTargetDir.Insert(pEntry, nTargetIdx);
        if(nTargetIdx < nSourceIdx)
            ++nSourceIdx;
        rTargetDir.Remove(nSourceIdx);
        return SaveDir(rTargetDir);
    }

    if(nTargetIdx == USHRT_MAX)
        nTargetIdx = rTargetDir.Count();
        // Eintrag existiert bereits
    if(SeekEntry_Impl(rTargetDir, pSourceEntry->GetLongName()))
        return FALSE;
    DirEntry aSource(pSourceEntry->GetFull());
    DirEntry aDest(rTargetDir.GetPath());
    MakeFileName_Impl(
        aDest, rSourceDir[nSourceIdx]->GetLongName(), rTargetDir.IsDir());

    // wenn aktuell das File geoeffnet ist,
    // muss es hinterher wieder geoeffnet werden
    SfxObjectShellRef xObjRef = pSourceEntry->GetObjectShell();
    const BOOL bOpen = xObjRef.Is();
    BOOL bOk = FALSE;
    SvStorageRef aNewStor;
    if(bOpen)
    {
        if(bMove)
        {
            xObjRef->DoHandsOff();
            const FSysError eErrno = aSource.MoveTo(aDest);
            bOk = FSYS_ERR_OK == eErrno;
        }
        else
        {
            // kopieren als speichern unter neuem Namen
            aNewStor = new SvStorage(aDest.GetFull());
            bOk = xObjRef->DoSaveAs(aNewStor);
            // alten Storage wieder einsetzen -> Kopieren ist fertig
            xObjRef->DoSaveCompleted( (SfxMedium*) 0 );
        }
    }
    else
    {
        const FSysError eErrno =
            bMove? aSource.MoveTo(aDest): aSource.CopyTo(aDest, FSYS_ACTION_COPYFILE);
        bOk = FSYS_ERR_OK == eErrno;
    }

    if(bOk)
    {
        // neuen Eintrag einfuegen
        // und Directory Eintraege schreiben
        SfxTemplateDirEntryPtr pNewEntry = new SfxTemplateDirEntry(
                rSourceDir[nSourceIdx]->GetLongName(),
                aDest.GetName(),
                rTargetDir);

        rTargetDir.Insert(pNewEntry, nTargetIdx);
        SaveDir(rTargetDir);

        // gfs. alten Eintrag loeschen
        if(bMove)
        {
            delete rSourceDir[nSourceIdx];
            rSourceDir.Remove(nSourceIdx);
            SaveDir(rSourceDir);

            // Zielstorage einsetzen
            if(bOpen)
            {
                // ObjectShell anlegen
                const SfxObjectFactory &rFactory = SfxObjectFactory::GetDefaultFactory();
                SfxObjectShellRef xObj =
                        rFactory.CreateObject(SFX_CREATE_MODE_ORGANIZER);
                pNewEntry->SetObjectShell( xObj );
                if ( xObj.Is() )
                {
                    xObj->DoInitNew(0);
                    xObj->LoadFrom(aNewStor);
                }
            }
        }
    }
    return bOk;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::Move
(
    USHORT nTargetRegion,       //  Index des Zielbereiches
    USHORT nTargetIdx,          //  Index Zielposition
    USHORT nSourceRegion,       //  Index des Quellbereiches
    USHORT nSourceIdx           /*  Index der zu kopierenden / z uverschiebenden
                                    Dokumentvorlage */
)

/*  [Beschreibung]

    Verschieben einer Dokumentvorlage


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef2uhrt werden

    [Querverweise]

    <SfxDocumentTemplates::CopyOrMove(USHORT,USHORT,USHORT,USHORT,BOOL)>
*/
{
    DBG_ASSERT( pDirs, "not initialized" );
    return CopyOrMove(nTargetRegion, nTargetIdx, nSourceRegion, nSourceIdx, TRUE);
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::Copy
(
    USHORT nTargetRegion,       //  Index des Zielbereiches
    USHORT nTargetIdx,          //  Index Zielposition
    USHORT nSourceRegion,       //  Index des Quellbereiches
    USHORT nSourceIdx           /*  Index der zu kopierenden / z uverschiebenden
                                    Dokumentvorlage */
)

/*  [Beschreibung]

    Kopieren einer Dokumentvorlage


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden

    [Querverweise]

    <SfxDocumentTemplates::CopyOrMove(USHORT,USHORT,USHORT,USHORT,BOOL)>
*/

{
    return CopyOrMove(nTargetRegion, nTargetIdx, nSourceRegion, nSourceIdx, FALSE);
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::CopyTo
(
    USHORT          nRegion,    /*  Bereich der Vorlage, die exportiert werden
                                    soll  */
    USHORT          nIdx,       /*  Index der Vorlage, die exportiert werden
                                    soll */
    const String&   rName       /*  Dateiname, unter dem die Vorlage angelegt
                                    werden soll */
)   const

/*  [Beschreibung]

    Exportieren einer Dokumentvorlage in das Dateisystem


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden


    [Querverweise]

    <SfxDocumentTemplates::CopyFrom(USHORT,USHORT,String&)>
*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    SfxTemplateDirEntry *pSourceDirectory = (*pDirs)[nRegion];
    SfxTemplateDir &rSourceDir = pSourceDirectory->GetContent();
    SfxTemplateDirEntry *pSourceEntry = rSourceDir[nIdx];
    DirEntry aSource(pSourceEntry->GetFull());
    DirEntry aDest(rName);
    FSysError eErr = aSource.CopyTo(aDest, FSYS_ACTION_COPYFILE);
    return FSYS_ERR_OK == eErr;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::CopyFrom
(
    USHORT      nRegion,        /*  Bereich, in den die Vorlage importiert
                                    werden soll */
    USHORT      nIdx,           //  Index der neuen Vorlage in diesem Bereich
    String&     rName           /*  Dateiname der Vorlage, die importiert
                                    werden soll, als out-Parameter der (auto-
                                    matisch aus dem Dateinamen generierte)
                                    logische Name der Vorlage */
)

/*  [Beschreibung]

    Importieren einer Dokumentvorlage aus dem Dateisystem


    [R"uckgabewert]                 Erfolg (TRUE) oder Mi"serfpTargetDirectory->GetContent());
}


//------------------------------------------------------------------------
#pragmaolg (FALSE)

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden

    [Querverweise]

    <SfxDocumentTemplates::CopyTo(USHORT,USHORT,const String&)>
*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    // Datei kopieren auf temp. Namen
    // automatischen Namen erzeugen
    // Vorlage in die Struktur einfuegen
    // Struktur speichern
    // Vorlage in die Listbox einfuegen (geschieht in der rufenden Funktion)
    DirEntry aSource(rName);
    SfxTemplateDirEntry *pTargetDirectory = (*pDirs)[nRegion];
    SfxTemplateDir &rTargetDir=pTargetDirectory->GetContent();
    DirEntry aTarget(pTargetDirectory->GetFull());
    rName=aSource.GetBase();
    MakeFileName_Impl(aTarget, rName, FALSE);

    FSysError eErr = aSource.CopyTo(aTarget, FSYS_ACTION_COPYFILE);
    if(FSYS_ERR_OK != eErr)
        return FALSE;
    rName = aSource.GetBase();
    const SfxTemplateDirEntryPtr pNewEntry = new SfxTemplateDirEntry(rName,
            aTarget.GetFull(), aTarget.GetPath());
    rTargetDir.Insert(pNewEntry, nIdx+1);
    return SaveDir(rTargetDir);
}


//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::KillDir
(
    SfxTemplateDir& rDir    //  das zu l"oschen de Directory
)

/*  [Beschreibung]

    "oschen des Verzeichnisses rDir incl. seines Inhaltes;
    dies geschieht sowohl auf der Platte als auch in den Verwaltungsdaten.


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden
*/
{
    DBG_ASSERT( pDirs, "not initialized" );

    // alle Vorlagen in dem Verzeichnis loeschen
    USHORT i = 0;
    BOOL bOk = TRUE;
    while(i < rDir.Count())
    {
        SfxTemplateDirEntry *pEntry = rDir[i];
        pEntry->DeleteObjectShell();
        DirEntry aFile(pEntry->GetFull());
        const FSysError eErr = aFile.Kill();
        if(FSYS_ERR_OK == eErr || !aFile.Exists()) //! FSYS_ERR_NOTEXISTS == eErr)
        {
            rDir.Remove(i);
            delete pEntry;
        }
        else
        {
            ++i;
            bOk = FALSE;
        }
    }
    if(bOk)
    {
        FSysError eErr = rDir.GetPath().Kill();
        bOk = (FSYS_ERR_OK == eErr || !rDir.GetPath().Exists()); //!! FSYS_ERR_NOTEXISTS == eErr;
    }
    else
        // bzw. im Fehlerfall aktualisieren
        SaveDir(rDir);
    return bOk;
}

//------------------------------------------------------------------------

USHORT MakeRegionRelative_Impl
(
    USHORT* pDirCount,          /*  Array mit der Anzahl der Eintr"age eines
                                    jeden Bereiches */
    USHORT nRegion              //  der absolute Index
)

/*  [Beschreibung]

    Macht einen Eintrag relativ zu seinem Bereich


    [R"uckgabewert]

    USHORT                          der Index relativ zu seinem Bereich

*/
{
    USHORT *pIter = pDirCount;
    while(*pIter <= nRegion)
        nRegion -= *pIter++;
    return nRegion;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::Delete
(
    USHORT nRegion,             //  Index des Bereiches
    USHORT nIdx                 /*  Index des Eintrags oder USHRT_MAX,
                                    wenn ein Verzeichnis gemeint ist. */
)

/*  [Beschreibung]

    "oschen eines Eintrags oder eines Verzeichnisses


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden


    [Querverweise]

    <SfxDocumentTemplates::InsertDir(const String&,USHORT)>
    <SfxDocumentTemplates::KillDir(SfxTemplateDir&)>
    <SfxDocumentTemplates::SaveDir(SfxTemplateDir&)>
*/

{
    DBG_ASSERT( pDirs, "not initialized" );

    SfxTemplateDirEntry* pEntry = (*pDirs)[nRegion];
    BOOL bOk = FALSE;

    // handelt es sich im ein Verzeichnis?
    if ( USHRT_MAX == nIdx )
    {
        bOk = KillDir( pEntry->GetContent() );
        if ( bOk )
        {
/*!!! PB: #66330# nicht mehr n"otig, da jetzt die Template-Komponente verwendet wird
            USHORT nIdx=nRegion;
            USHORT *pIter = pDirCount;
            while(*pIter <= nIdx)
                nIdx -= *pIter++;
            (*pIter)--;
            DirEntry aFileName(pEntry->GetPath());
            DirEntry aTplName( aFileName );
            aTplName+=DirEntry(pDirFileName);
            if( aTplName.Exists() )
            {
                SfxTemplateDir aDir(aFileName, TRUE);
                SfxTemplateDirEntry *pTmpEntry = aDir[nIdx];
                aDir.Remove(nIdx);
                delete pTmpEntry;
                bOk = SaveDir(aDir);
            }
            if ( bOk )
*/
            {
                pDirs->Remove(nRegion);
                delete pEntry;
            }
        }
    }
    else
    {
        // es handelt sich um eine einzelne Vorlage
        SfxTemplateDir& rDir = pEntry->GetContent();
        SfxTemplateDirEntry* pEntryToDelete = rDir[nIdx];
        pEntryToDelete->DeleteObjectShell();
        DirEntry aFile( pEntryToDelete->GetFull() );
        const FSysError eErr = aFile.Kill();
        if ( FSYS_ERR_OK == eErr || !aFile.Exists() )   //!!! FSYS_ERR_NOTEXISTS == eErr
        {
            rDir.Remove(nIdx);
            bOk = SaveDir(rDir);
            delete pEntryToDelete;
        }
    }
    return bOk;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::InsertDir
(
    const String&   rText,      //  der logische Name des neuen Bereiches
    USHORT          nRegion     //  Index des Bereiches
)

/*  [Beschreibung]

    Einf"ugen eines Verzeichnisses


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden

    [Querverweise]

    <SfxDocumentTemplates::KillDir(SfxTemplateDir&)>
    <SfxDocumentTemplates::SaveDir(SfxTemplateDir&)>
*/
{
    DBG_ASSERT( pDirs, "not initialized" );

    DirEntry aPath((*pDirs)[nRegion-1]->GetPath());
    DirEntry aTmpName(aPath);
    SfxTemplateDir aDir(aPath.GetFull(), TRUE);
    MakeFileName_Impl(aTmpName,  rText, TRUE);
    String aName=aTmpName.GetName();
    if( aName.GetChar(aName.Len()-1) == '.')
        aName=String(aName,0,aName.Len()-1);
    if(aTmpName.MakeDir())
    {
        SfxTemplateDirEntryPtr pEntry = new SfxTemplateDirEntry(
                rText, aName, aTmpName.GetPath() );
        pDirs->Insert(pEntry, nRegion);

        // Directory-File im uebergeordneteten Directory aktualisieren

        USHORT nLocalRegion=nRegion;
        USHORT *pIter = pDirCount;
        while(*pIter < nLocalRegion)
            nLocalRegion -= *pIter++;

        (*pIter)++;

        aDir.Insert(pEntry, nLocalRegion);
        SaveDir(pEntry->GetContent());

        const BOOL bOk = SaveDir(aDir);


        aDir.Remove(nLocalRegion);
        if(!bOk)
        {
            pDirs->Remove(nRegion);
            delete pEntry;
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::SetName
(
    const String&   rName,      //  Der zu setzende Name
    USHORT          nRegion,    //  Index des Bereiches
    USHORT          nIdx        /*  Index des Eintrags oder USHRT_MAX,
                                    wenn ein Verzeichnis gemeint ist. */
)

/*  [Beschreibung]

    "Andern des Namens eines Eintrags oder eines Verzeichnisses


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden

*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    SfxTemplateDirEntry *pEntry = (*pDirs)[nRegion];

    // Vorlage wird umbenannt
    if(USHRT_MAX != nIdx)
    {
        SfxTemplateDir &rDir = pEntry->GetContent();
        pEntry = rDir[nIdx];
        if(!pEntry->SetLongAndFileName(rName))
            return FALSE;
        SaveDir(rDir);
        return TRUE;
    }
    else
    {
        if(!pEntry->SetLongAndFileName(rName))
            return FALSE;

        // Verzeichnis einlesen und mit den neuen Daten speichern
        SfxTemplateDir aDir(pEntry->GetPath(), TRUE);
        if(aDir.IsDir())
        aDir[MakeRegionRelative_Impl(pDirCount, nRegion)]->SetLongName(rName);
        aDir[MakeRegionRelative_Impl(pDirCount, nRegion)]->SetFileName(
            pEntry->GetFileName());
        SaveDir(aDir);
        return TRUE;
    }
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::Rescan()

/*  [Beschreibung]

    Abgleich des Verwaltungsdaten mit dem aktuellen Zustand auf der Platte.
    Die logischen Namen, zu denen keine Datei mit existiert, werden aus
    der Verwaltungsstruktur entfernt; Dateien, zu denen kein Eintrag
    existiert, werden aufgenommen.


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden


    [Querverweise]

    <SfxTemplateDir::Scan(BOOL bDirectory, BOOL bSave)>
    <SfxTemplateDir::Freshen(const SfxTemplateDir &rNew)>
*/
{
    DBG_ASSERT( pDirs, "not initialized" );

//(mba)/task    SfxWaitCursor aWait;

    const USHORT nCount = pDirs->Count();
    BOOL bOk = TRUE;

    USHORT i;
    for(i = 0; i < nCount; ++i)
    {
        SfxTemplateDirEntry *pEntry = (*pDirs)[i];
        SfxTemplateDir &rCntnt = pEntry->GetContent();
        SfxTemplateDir aNew(rCntnt.GetPath());
        aNew.Scan(FALSE);
        if(rCntnt.Freshen(aNew))
            bOk |= SaveDir(rCntnt);
    }

    USHORT nMax=aDirs.GetTokenCount(cDelim);
    for(i=0;i<nMax;i++)
    {
        SfxTemplateDir *pOldDir = new SfxTemplateDir(
            aDirs.GetToken(i, cDelim), TRUE, FALSE);
        SfxTemplateDir *pNewDir = new SfxTemplateDir(
            aDirs.GetToken(i, cDelim), TRUE, TRUE);
        if(pOldDir->Freshen(*pNewDir))
            bOk |= SaveDir(*pOldDir);
        delete pNewDir;
        delete pOldDir;
    }

    DELETEZ(pDirs);
    DELETEZ(pDirCount);

    Construct();

    return bOk;
}

//------------------------------------------------------------------------

SfxObjectShellRef SfxDocumentTemplates::CreateObjectShell
(
    USHORT nRegion,         //  Index des Bereiches
    USHORT nIdx             //  Index des Eintrags
)

/*  [Beschreibung]

    Zugriff auf die DokumentShell eines Eintrags


    [R"uckgabewert]

    SfxObjectShellRef         Referenz auf die ObjectShell


    [Querverweise]

    <SfxTemplateDirEntry::CreateObjectShell()>
    <SfxDocumentTemplates::DeleteObjectShell(USHORT, USHORT)>
*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    return (*pDirs)[nRegion]->GetContent()[nIdx]->CreateObjectShell();
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::DeleteObjectShell
(
    USHORT nRegion,         //  Index des Bereiches
    USHORT nIdx             //  Index des Eintrags
)

/*  [Beschreibung]

    Freigeben der ObjectShell eines Eintrags


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden

    [Querverweise]

    <SfxTemplateDirEntry::DeleteObjectShell()>
    <SfxDocumentTemplates::CreateObjectShell(USHORT, USHORT)>
*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    return (*pDirs)[nRegion]->GetContent()[nIdx]->DeleteObjectShell();
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::GetFull
(
    const String &rRegion,      // Der Name des Bereiches
    const String &rName,        // Der Name der Vorlage
    String &rPath               // Out: Pfad + Dateiname
)

/*  [Beschreibung]

    Liefert Pfad + Dateiname zu der durch rRegion und rName bezeichneten
    Vorlage


    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden


    [Querverweise]

    <SfxDocumentTemplates::GetLogicNames(const String&,String&,String&)>
*/

{
    DBG_ASSERT( pDirs, "not initialized" );

    const SfxTemplateDirEntry* pEntry = NULL;
    const USHORT nCount = pDirs->Count();
    BOOL bFQ = FALSE;
    USHORT nPos = rRegion.Len();
    while( --nPos != USHRT_MAX && rRegion.GetChar( nPos ) != '(' )
        ;
    if ( nPos != USHRT_MAX && rRegion.GetChar( USHORT(rRegion.Len() - 1) ) == ')' )
        bFQ = TRUE;

    for ( USHORT i = 0; i < nCount; ++i )
    {
        String aName;
        if( bFQ )
            aName = GetFullRegionName( i );
        else
            aName = GetRegionName( i );
        if( aName == rRegion )
        {
            pEntry = (*pDirs)[i];
            SfxTemplateDir& rDir = ( (SfxTemplateDirEntry*)pEntry )->GetContent();
            pEntry = SeekEntry_Impl( rDir, rName );
            if ( pEntry )
                rPath = pEntry->GetFull().GetFull();
            if ( pEntry || bFQ )
                break;
        }
    }

    return ( pEntry != NULL );
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::GetLogicNames
(
    const String &rPath,            // vollst"andiger Pfad zu der Vorlage
    String &rRegion,                // Out: der Bereichsname
    String &rName                   // Out: der Vorlagenname
) const

/*  [Beschreibung]

    Liefert Pfad und logischen Namen zu der durch rPath bezeichneten
    Vorlage

    [R"uckgabewert]

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden


    [Querverweise]

    <SfxDocumentTemplates::GetFull(const String&,const String&,DirEntry&)>
*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    DirEntry aPath(rPath);
    aPath.ToAbs();
    const DirEntry aPathTo(aPath.GetPath());
    const DirEntry aFileName(aPath.GetName());

    const SfxTemplateDirEntry *pEntry = 0;
    const USHORT nCount = pDirs->Count();
    for(USHORT i = 0; i < nCount; ++i)
    {
        SfxTemplateDirEntry *p = (*pDirs)[i];
        if (p->GetFull() == aPathTo ) // GetFull, da Directory
        {
            pEntry = p;
            break;
         }
    }
    if(!pEntry)
        return FALSE;
    rRegion =   pEntry->GetLongName();
    SfxTemplateDir &rDir = ((SfxTemplateDirEntry *)pEntry)->GetContent();
    pEntry = rDir.Find_Impl(aPath.GetFull());
    if(pEntry)
        rName = pEntry->GetLongName();
    return pEntry != 0;
}

//------------------------------------------------------------------------

SfxDocumentTemplates::SfxDocumentTemplates()

/*  [Beschreibung]

    Konstruktor
*/

:   aDirs(SFX_INIMANAGER()->Get(SFX_KEY_TEMPLATE_PATH)),
    pDirs(0)
{
}

//-------------------------------------------------------------------------

void SfxDocumentTemplates::Construct()

//  verz"ogerter Aufbau der Verwaltungsdaten

{
    // schon von jemandem anders konstruiert?
    if ( pDirs )
        return;

    pDirs = new SfxTemplateDir;
    cDelim = ';'; // absichtlich hart verdrahtet
    pDirCount = new USHORT [aDirs.GetTokenCount(cDelim)+1];
    SfxTemplateDir *pTmp=CreateRootTemplateDir(pDirCount);
    pDirs->Insert(pTmp,0);
    pTmp->Remove(0, pTmp->Count()); // Loeschen der Pointer verhindern
    delete pTmp;
    DirEntry aDir(aDirs.GetToken(0,cDelim));
    aDir+=DirEntry( String::CreateFromAscii( pMGName ) );
    if(aDir.Exists())
    {
        if ( aDir.Kill() == ERRCODE_NONE )
            Rescan();
    }
#if 0   //(dv)
    CntAnchorRef xAnchor = new CntAnchor( NULL, ".component:Template/" );

#ifdef TF_UCB
    DBG_ERRORFILE( "GetInterface NIY!!!" );
#else
    if ( xAnchor->GetInterface() )
    {
        OpenNotifier_Impl aOpen( xAnchor, CNT_OPEN_FOLDERS );
        while ( !aOpen.IsComplete() )
            Application::Yield();
    }
#endif

    for ( USHORT n=0; n<xAnchor->SubAnchorCount(); n++ )
    {
        CntAnchorRef xRegion = xAnchor->GetSubAnchor( n );
        String aStr = xRegion->GetViewURL();
        USHORT nPos = aStr.Search( '/' );
        aStr.Cut(0,nPos+2);     // .component:template/_ abschneiden

        xRegion->Put( SfxVoidItem( WID_GETDATA ) );
        const CntStringItem& rItem = (const CntStringItem&) xRegion->Get( WID_TITLE);

        SfxTemplateDirEntry *pEntry = 0;
        const USHORT nCount = pDirs->Count();
        for( USHORT i = 0; i < nCount; ++i )
        {
            DirEntry aDir( (*pDirs)[i]->GetFull() );
            String aName( aDir.GetFull( FSYS_STYLE_URL ) );
            if( !aDir.IsCaseSensitive() )
                aName.ToLower();

            if ( aName == aStr )
            {
                pEntry = (*pDirs)[i];
                break;
            }
        }

        if( pEntry )
        {
            pEntry->SetLongName( rItem.GetValue() );
            pEntry->SetAnchor( xRegion );
        }
    }
#endif  //(dv)
}

//------------------------------------------------------------------------
SfxTemplateDir *SfxDocumentTemplates::CreateRootTemplateDir(USHORT *pIter, SfxTemplateDir **ppDirs, BOOL bScan ) const
{
    DBG_ASSERT( pDirs, "not initialized" );

    SfxTemplateDir *pDirs=new SfxTemplateDir;
    const USHORT nRegions = aDirs.GetTokenCount(cDelim);
    BOOL bDelPPDirs=FALSE;
    if(!ppDirs)
    {
        ppDirs = new SfxTemplateDir *[nRegions];
        bDelPPDirs = TRUE;
    }

    // rueckwaerts einfuegen,
    // da AutoPiloten die Wizard-Templates vorne erwarten, jedoch in den
    // Standard-Pfad reinschreiben wollen
    for(USHORT i = 0; i < nRegions; ++i)
    {
        ppDirs[i] = new SfxTemplateDir(aDirs.GetToken(i, cDelim), TRUE, bScan);
        if(!ppDirs[i]->Count())
        {
            const String aStd(SfxResId(STR_STANDARD));
            const DirEntry aStdPath(aDirs.GetToken(i, cDelim));
            DirEntry aStandard(aStdPath);
            aStandard.MakeDir();

            SfxTemplateDirEntryPtr pStd =
                new SfxTemplateDirEntry(aStd, String(), aStdPath,TRUE);

            ppDirs[i]->Insert( pStd, 0);

            SaveDir(pStd->GetContent());

            SaveDir(*ppDirs[i]);

        }

        pDirs->Insert( ppDirs[i], 0 );
        *pIter++ = ppDirs[i]->Count();
    }

    if(bDelPPDirs)
        delete[]ppDirs;
    return pDirs;
}

//------------------------------------------------------------------------

SfxDocumentTemplates::~SfxDocumentTemplates()

/*  [Beschreibung]

    Destruktor
    Freigeben der Verwaltungsdaten
*/

{
    DBG_ASSERT( pDirs, "not initialized" );
    delete pDirs;
    delete pDirCount;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
struct LongNameListRec
{
    String maShortName;
    String maLongName;

    LongNameListRec( const String& rShort,
                     const String& rLong )
            : maShortName( rShort )
            , maLongName( rLong )
    {}
};

// ------------------------------------------------------------------------
LongNameList_Impl::LongNameList_Impl( const String& rDirName )
{
    String aName = rDirName;

    aName.Append( FileName_Impl( TRUE ) );

    mpNames = 0;

    SfxMedium aMedium( aName, STREAM_STD_READ, TRUE );
    SvStream *pStream = aMedium.GetInStream();

    if ( pStream )
    {
        char        cDir;
        USHORT      nCount, i;
        ByteString  aLongName;  // logischer Name des Benutzers
        ByteString  aFileName;  // Dateiname (ohne Pfad)
        LongNameListRec *pNew;

        if ( ! ReadHeader( *pStream ) )
            return;

        *pStream >> cDir;
        *pStream >> nCount;

        mpNames = new List( nCount, 10 );

        CharSet eCharSet = pStream->GetStreamCharSet();

        for( i = 0; i < nCount; ++i )
        {
            pStream->ReadByteString( aLongName );
            pStream->ReadByteString( aFileName );
            pNew = new LongNameListRec( String( aFileName, eCharSet ),
                                        String( aLongName, eCharSet ) );
            mpNames->Insert( pNew );
        }
    }
}

// ------------------------------------------------------------------------
LongNameList_Impl::~LongNameList_Impl()
{
    if ( mpNames )
    {
        LongNameListRec *pData = (LongNameListRec *) mpNames->First();

        while ( pData )
        {
            delete pData;
            pData = (LongNameListRec *) mpNames->Next();
        }

        delete mpNames;
    }
}

// ------------------------------------------------------------------------
String LongNameList_Impl::GetLongName( const String& rShortName ) const
{
    String aLongName;
    LongNameListRec *pData;

    if ( mpNames )
        pData = (LongNameListRec *) mpNames->First();
    else
        pData = NULL;

    while ( pData )
    {
        if ( pData->maShortName == rShortName )
        {
            aLongName = pData->maLongName;
            break;
        }
        else
            pData = (LongNameListRec *) mpNames->Next();
    }

    return aLongName;
}

// ------------------------------------------------------------------------
BOOL LongNameList_Impl::ReadHeader( SvStream& rStream ) const
{
    ByteString  aHeader;
    USHORT      nUS = strlen( pHeader );
    USHORT      nVersion;
    CharSet     eCharSet;
    char        c;

    rStream.Read( aHeader.AllocBuffer( nUS ), nUS );
    rStream >> c >> nVersion >> nUS;

    // CharSet einlesen und am Stream setzen
    eCharSet = (CharSet)nUS;
    rStream.SetStreamCharSet(eCharSet);

    if( aHeader != pHeader )
        return FALSE;
    if( nVersion != VERSION )
        return FALSE;

    return TRUE;
}


