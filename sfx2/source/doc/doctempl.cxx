/*************************************************************************
 *
 *  $RCSfile: doctempl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dv $ $Date: 2000-11-27 08:55:57 $
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
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#include <tools/string.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include <ucbhelper/content.hxx>

#include <com/sun/star/uno/Reference.h>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFO_HPP_
#include <com/sun/star/ucb/ContentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif


using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace rtl;
using namespace ucb;


#include "doctempl.hxx"
#include "docfac.hxx"
#include "docfile.hxx"
#include "objsh.hxx"
#include "sfxtypes.hxx"
#include "app.hxx"
#include "inimgr.hxx"
#include "sfxresid.hxx"
#include "docfilt.hxx"
#include "fltfnc.hxx"
#include "doc.hrc"
#include "ucbhelp.hxx"
#include "openflag.hxx"
#include "ucbhelp.hxx"

//========================================================================

const char __FAR_DATA   pHeader[] =         "Sfx Document Template Directory";
const char __FAR_DATA   pFileFileName[] =   "sfx.tpl";
const char __FAR_DATA   pDirFileName[] =    "sfx.tlx";
const char              pMGName[] =         "mg.bad";
const char              cDefWildcard =      '*';

#define HIERARCHIE_ROOT_URL     "vnd.sun.star.hier:/"
#define TEMPLATE_ROOT_URL       "vnd.sun.star.hier:/templates"
#define TEMPLATE_DIR_NAME       "templates"
#define TITLE                   "Title"
#define IS_FOLDER               "IsFolder"
#define TARGET_URL              "TargetURL"
#define TYPE_FOLDER             "application/vnd.sun.star.hier-folder"
#define TYPE_LINK               "application/vnd.sun.star.hier-link"
#define TYPE_FSYS_FOLDER        "application/vnd.sun.staroffice.fsys-folder"

#define TARGET_DIR_URL          "TargetDirURL"
#define PARENT_DIR_URL          "ParentDirURL"
#define REAL_NAME               "RealName"
#define COMMAND_DELETE          "delete"
#define COMMAND_TRANSFER        "transfer"

#define C_DELIM                 ';'

//========================================================================

class EntryData_Impl
{
    OUString            maTitle;
    OUString            maOwnURL;
    OUString            maTargetURL;
    sal_Bool            mbChecked;

public:
                        EntryData_Impl( const OUString& rTitle );

    const OUString&     GetTitle() const { return maTitle; }
    const OUString&     GetTargetURL() const { return maTargetURL; }
    const OUString&     GetHierarchyURL() const { return maOwnURL; }

    void                SetTitle( const OUString& rTitle ) { maTitle = rTitle; }
    void                SetTargetURL( const OUString& rTargetURL );
    void                SetHierarchyURL( const OUString& rURL) { maOwnURL = rURL; }

    int                 Compare( const OUString& rTitle ) const;
};

DECLARE_LIST( EntryList_Impl, EntryData_Impl* );

// ------------------------------------------------------------------------

class RegionData_Impl
{
    EntryList_Impl      maEntries;
    OUString            maTitle;
    OUString            maRealTitle;
    OUString            maOwnURL;
    OUString            maParentURL;
    OUString            maTargetURL;
    Content             maTargetContent;
    Content             maHierarchyContent;
    sal_Bool            mbChecked;

private:
    long                GetEntryPos( const OUString& rTitle,
                                     sal_Bool& rFound ) const;
public:
                        RegionData_Impl( const OUString& rTitle,
                                         const OUString& rRealTitle );
                        ~RegionData_Impl();

    EntryData_Impl*     GetEntry( const OUString& rName ) const;
    EntryData_Impl*     GetEntry( USHORT nIndex ) const;

    const OUString&     GetTitle() const { return maTitle; }
    const OUString&     GetRealTitle() const { return maRealTitle; }
    const OUString&     GetTargetURL() const { return maTargetURL; }
    const OUString&     GetHierarchyURL() const { return maOwnURL; }

    USHORT              GetCount() const;

    void                SetTitle( const OUString& rTitle ) { maTitle = rTitle; }
    void                SetRealTitle( const OUString& rTitle ) { maRealTitle = rTitle; }
    void                SetTargetURL( const OUString& rTargetURL );
    void                SetParentURL( const OUString& rParentURL );
    void                SetHierarchyURL( const OUString& rURL) { maOwnURL = rURL; }

    void                SetTargetContent( const Content& rTarget ) { maTargetContent = rTarget; }
    void                SetHierarchyContent( const Content& rContent ) { maHierarchyContent = rContent; }

    void                AddEntry( Content& rParentFolder,
                                  const OUString& rTitle,
                                  const OUString& rTargetURL );
    void                DeleteEntry( USHORT nIndex );

    void                SetChecked( sal_Bool bChecked ) { mbChecked = bChecked; }
    int                 Compare( const OUString& rTitle ) const
                            { return maTitle.compareTo( rTitle ); }
    int                 Compare( RegionData_Impl* pCompareWith,
                                 sal_Bool* pbNamesAreEqual = NULL ) const;
};

DECLARE_LIST( RegionList_Impl, RegionData_Impl* );

// ------------------------------------------------------------------------
class SfxDocTemplate_Impl
{
    ::osl::Mutex        maMutex;
    String              maDirs;
    RegionList_Impl     maRegions;
    long                mnRefCount;
    sal_Bool            mbConstructed;

private:
    sal_Bool            InsertOrMarkRegion( RegionData_Impl *pData );

public:
                        SfxDocTemplate_Impl();
                        ~SfxDocTemplate_Impl();

    void                Construct( const String& rDirURLs );
    sal_Bool            Rescan();
    void                CreateFromHierarchy( Content &rTemplRoot );
    void                CreateFromDirs();

    void                AddRegion( const OUString& rTitle,
                                   const OUString& rRealTitle,
                                   const OUString& rTargetURL,
                                   Content& rContent );
    void                DeleteRegion( USHORT nIndex );

    USHORT              GetRegionCount() const
                            { return maRegions.Count(); }
    RegionData_Impl*    GetRegion( const OUString& rName ) const;
    RegionData_Impl*    GetRegion( USHORT nIndex ) const;
    RegionData_Impl*    GetRegionByPath( const String& rPath ) const;
    void                GetFolders( Content& rRoot, Content& rFolder, ULONG nIndex );
    void                GetTemplates( Content& rTargetFolder,
                                      Content& rParentFolder,
                                      RegionData_Impl* pRegion,
                                      const OUString& rParentURL );

    long                GetRegionPos( const OUString& rTitle,
                                      sal_Bool& rFound ) const;

    sal_Bool            GetTemplateRoot( Content &rTemplRoot, sal_Bool &rNew ) const;
    sal_Bool            GetTemplateDir( USHORT nIndex, Content &rTemplateDir ) const;

    sal_Bool            InsertNewRegionToHierarchy(
                                        Content &rRoot,
                                        const OUString &rTitle,
                                        const OUString &rTargetURL,
                                        USHORT nIndex );
    sal_Bool            InsertNewRegionToFolder(
                                        Content &rRoot,
                                        const OUString &rTitle );
};

SfxDocTemplate_Impl *gpTemplateData = 0;
String  gaEmptyString;

// ------------------------------------------------------------------------

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

//------------------------------------------------------------------------
#if 0
//dv!
void MakeFileName_Impl(DirEntry &rEntry,
                              const String &rLongName, BOOL bDir)
{
    String aFName(rLongName);
    if(!bDir)
        aFName+= DEFINE_CONST_UNICODE( ".vor" );
    rEntry.MakeShortName(aFName, FSYS_KIND_NONE);
}
#endif

//------------------------------------------------------------------------
#if 0
//dv!
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
                GetFull().GetFull(),(STREAM_READ | STREAM_SHARE_DENYWRITE), FALSE, 0 );
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
#endif
//========================================================================
//========================================================================
//========================================================================

String SfxDocumentTemplates::GetFullRegionName
(
    USHORT nIdx                     // Index des Bereiches
)   const

/*  [Beschreibung]

    Liefert den logischen Namen eines Bereiches Plus seinem  Pfad


    [R"uckgabewert]                 Referenz auf diesen Namen

*/

{
    DBG_ASSERT( pImp, "not initialized" );

    // First: find the RegionData for the index
    String aName;

    RegionData_Impl *pData1 = pImp->GetRegion( nIdx );

    if ( pData1 )
        aName = pData1->GetRealTitle();

    /*  Second: check if there is another region with the same name.
        If there is one, append the path to the region name so it is
        distinguishable from the other ones
    */

    OUString         aCompareMe( aName );
    USHORT           nCount = pImp->GetRegionCount();
    USHORT           nIndex = 0;
    RegionData_Impl *pData2;

    for( nIndex = 0; nIndex < nCount; nIndex++ )
    {
        pData2 = pImp->GetRegion( nIndex );

        if ( ( nIndex != nIdx ) &&
             ( pData2->GetRealTitle() == aCompareMe ) )
        {
            INetURLObject aURLObj( pData2->GetTargetURL() );
            aURLObj.removeSegment();
            String aParent = aURLObj.getExternalURL();

            //dv! we should truncate the Parent string ( to a lenght of 10? )

            aName += DEFINE_CONST_UNICODE( " (" );
            aName += aParent;
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
    static String maTmpString;

    RegionData_Impl *pData = pImp->GetRegion( nIdx );

    if ( pData )
        maTmpString = pData->GetRealTitle();
    else
        maTmpString.Erase();

    return maTmpString;
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
    sal_Bool    bFound;
    USHORT      nIndex = pImp->GetRegionPos( rRegion, bFound );

    if ( bFound )
        return nIndex;
    else
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
    USHORT nCount = 0;

    if ( pImp )
        nCount = pImp->GetRegionCount();

    return nCount;
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
    DBG_ASSERT( pImp, "not initialized" );

    RegionData_Impl *pData = pImp->GetRegion( rName );
    USHORT           nCount = 0;

    if ( pData )
        nCount = pData->GetCount();

    return nCount;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::IsRegionLoaded( USHORT nIdx ) const
{
    RegionData_Impl *pData = pImp->GetRegion( nIdx );

    if ( pData )
        return TRUE;
    else
        return FALSE;
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
    DBG_ASSERT( pImp, "not initialized" );

    RegionData_Impl *pData = pImp->GetRegion( nRegion );
    USHORT           nCount = 0;

    if ( pData )
        nCount = pData->GetCount();

    return nCount;
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
    DBG_ASSERT( pImp, "not initialized" );

    static String maTmpString;

    EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( pRegion )
        pEntry = pRegion->GetEntry( nIdx );

    if ( pEntry )
        maTmpString = pEntry->GetTitle();
    else
        maTmpString.Erase();

    return maTmpString;
}

//------------------------------------------------------------------------

String SfxDocumentTemplates::GetFileName
(
    USHORT nRegion,     //  Index des Bereiches, in dem der Eintrag liegt
    USHORT nIdx         //  Index des Eintrags
)   const

/*  [Beschreibung]

    Liefert den Dateinamen eines Eintrags eines Bereiches

    [R"uckgabewert]                 Dateiname des Eintrags

*/
{
    DBG_ASSERT( pImp, "not initialized" );

    EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( pRegion )
        pEntry = pRegion->GetEntry( nIdx );

    if ( pEntry )
    {
        INetURLObject aURLObj( pEntry->GetTargetURL() );
        return aURLObj.GetName();
    }
    else
        return String();
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
    DBG_ASSERT( pImp, "not initialized" );

    EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( pRegion )
        pEntry = pRegion->GetEntry( nIdx );

    if ( pEntry )
        return pEntry->GetTargetURL();
    else
        return String();
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
    DBG_ASSERT( pImp, "not initialized" );

    EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( pRegion )
        pEntry = pRegion->GetEntry( rLongName );

    if ( pEntry )
        return pEntry->GetTargetURL();
    else if ( pRegion )
    {
        INetURLObject aURLObj( pRegion->GetTargetURL() );
        aURLObj.insertName( rLongName );
        return aURLObj.getExternalURL();
    }
    else
        return String();
}

//------------------------------------------------------------------------

String SfxDocumentTemplates::GetDefaultTemplatePath
(
    const String& rLongName
)

/*  [Beschreibung]

    Liefert den Standardpfad zu Dokumentvorlagen


    [R"uckgabewert]

    String                  Standardpfad zu Dokumentvorlagen

*/
{
    DBG_ASSERT( pImp, "not initialized" );
    DBG_ASSERT( aDirs.GetTokenCount( cDelim ), "Keine Bereiche" );

    String           aPath( aDirs.GetToken( 0, cDelim ) );
    EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( aPath );

    if ( pRegion )
        pEntry = pRegion->GetEntry( rLongName );

    if ( pEntry )
        return pEntry->GetTargetURL();
    else if ( pRegion )
    {
        INetURLObject aURLObj( pRegion->GetTargetURL() );
        aURLObj.insertName( rLongName );
        return aURLObj.getExternalURL();
    }
    else
        return String();

/* dv! missing: create the directory, if it doesn't exists


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
*/
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::SaveDir
(
//  SfxTemplateDir& rDir        //  das zu speichernde Directory
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
    DBG_ASSERT( pImp, "not initialized" );

    EntryData_Impl  *pEntry;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    // do nothing if there is no region with that index
    if ( !pRegion )
        return;

    pEntry = pRegion->GetEntry( rLongName );

    // do nothing if there is already an entry with that name
    if ( pEntry )
        return;

/* dv! if the filename is only a filename without a path, use the following
    code to create a full path

    INetURLObject aURLObj( pRegion->GetTargetURL() );
    aURLObj.insertName( rFileName );
    OUString aFullFileName = aURLObj.getExternalURL();
*/
    Content aContent;
    pRegion->AddEntry( aContent, rLongName, rFileName );
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
    /* to perform a copy or move, we need to send a transfer command to
       the destination folder with the URL of the source as parameter.
       ( If the destination content doesn't support the transfer command,
       we could try a copy ( and delete ) instead. )
       We need two transfers ( one for the real template and one for its
       representation in the hierarchy )
       ...
    */

    // Don't copy or move any folders
    if( nSourceIdx == USHRT_MAX )
        return FALSE ;

    if ( nSourceRegion == nTargetRegion )
    {
        DBG_ERRORFILE( "Don't know, what to do!" );
        return FALSE;
#if 0
    // Verschieben einer Vorlage innerhalb eines Bereiches
    // --> nur Verwaltungsdaten aktualisieren
    if ( bMove && nTargetRegion == nSourceRegion )
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
#endif
    }

    RegionData_Impl *pSourceRgn = pImp->GetRegion( nSourceRegion );
    if ( !pSourceRgn )
        return FALSE;

    EntryData_Impl *pSource = pSourceRgn->GetEntry( nSourceIdx );
    if ( !pSource )
        return FALSE;

    RegionData_Impl *pTargetRgn = pImp->GetRegion( nTargetRegion );
    if ( !pTargetRgn )
        return FALSE;

    EntryData_Impl *pTarget = pTargetRgn->GetEntry( nTargetIdx );

    Reference< XCommandEnvironment > aCmdEnv;
    Content aTarget;
    Content aHierTarget;

    OUString aTitle;

    if ( pTarget )
        aTitle = pTarget->GetTitle();
    else
        aTitle = pSource->GetTitle();

    try
    {
        aTarget = Content( pTargetRgn->GetTargetURL(), aCmdEnv );
        aHierTarget = Content( pTargetRgn->GetHierarchyURL(), aCmdEnv );

        TransferInfo aTransferInfo;
        aTransferInfo.MoveData = bMove;
        aTransferInfo.SourceURL = pSource->GetTargetURL();
        aTransferInfo.NewTitle = aTitle;
        aTransferInfo.NameClash = NameClash::OVERWRITE;

        Any aArg = makeAny( aTransferInfo );
        OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( COMMAND_TRANSFER ) );

        aTarget.executeCommand( aCmd, aArg );

        aTransferInfo.SourceURL = pSource->GetHierarchyURL();
        aArg = makeAny( aTransferInfo );
        aHierTarget.executeCommand( aCmd, aArg );
    }
    catch ( ContentCreationException& )
    { return FALSE; }
    catch ( CommandAbortedException& )
    { return FALSE; }
    catch ( ... )
    { return FALSE; }

    // update data structures ...
    if ( bMove )
    {
        pSourceRgn->DeleteEntry( nSourceIdx );
    }

    OUString aNewTargetURL = pTargetRgn->GetTargetURL();
    aNewTargetURL += OUString( '/' );
    aNewTargetURL += aTitle;
    pTargetRgn->AddEntry( aTarget, aTitle, aNewTargetURL );

    //dv! wenn aktuell das File geoeffnet ist,
    // muss es hinterher wieder geoeffnet werden
    return TRUE;
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
    return CopyOrMove( nTargetRegion, nTargetIdx,
                       nSourceRegion, nSourceIdx, TRUE );
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::Copy
(
    USHORT nTargetRegion,       //  Index des Zielbereiches
    USHORT nTargetIdx,          //  Index Zielposition
    USHORT nSourceRegion,       //  Index des Quellbereiches
    USHORT nSourceIdx           /*  Index der zu kopierenden / zu verschiebenden
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
    return CopyOrMove( nTargetRegion, nTargetIdx,
                       nSourceRegion, nSourceIdx, FALSE );
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
    return FALSE;
#if 0   //dv!
    DBG_ASSERT( pDirs, "not initialized" );
    SfxTemplateDirEntry *pSourceDirectory = (*pDirs)[nRegion];
    SfxTemplateDir &rSourceDir = pSourceDirectory->GetContent();
    SfxTemplateDirEntry *pSourceEntry = rSourceDir[nIdx];
    DirEntry aSource(pSourceEntry->GetFull());
    DirEntry aDest(rName);
    FSysError eErr = aSource.CopyTo(aDest, FSYS_ACTION_COPYFILE);
    return FSYS_ERR_OK == eErr;
#endif
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
    return FALSE;
#if 0   //dv!
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
#endif
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
    /* delete the template or folder in the hierarchy and in the
       template folder by sending a delete command to the content.
       Then remove the data from the lists
    */

    sal_Bool bRet = sal_False;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( !pRegion )
        return bRet;

    OUString aHierURL;
    OUString aTargetURL;

    if ( nIdx == USHRT_MAX )
    {
        aTargetURL = pRegion->GetTargetURL();
        aHierURL = pRegion->GetHierarchyURL();
    }
    else
    {
        EntryData_Impl *pEntry = pRegion->GetEntry( nIdx );
        if ( !pEntry )
            return bRet;
        aTargetURL = pEntry->GetTargetURL();
        aHierURL = pEntry->GetHierarchyURL();
    }

    Reference< XCommandEnvironment > aCmdEnv;

    Content aHierCont;
    Content aFolderCont;

    try
    {
        aHierCont = Content( aHierURL, aCmdEnv );
        aFolderCont = Content( aTargetURL, aCmdEnv );
    }
    catch ( ContentCreationException& )
    {
        DBG_ERRORFILE( "Template or Region doesn't exist!" );
        return bRet;
    }
    catch ( ... )
    {
        DBG_ERRORFILE( "Any Other Exception!" );
        return bRet;
    }

    try
    {
        OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( COMMAND_DELETE ) );
        Any aArg = makeAny( sal_Bool( sal_True ) );

        aHierCont.executeCommand( aCmd, aArg );
        aFolderCont.executeCommand( aCmd, aArg );

        bRet = sal_True;
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "Template or Region doesn't exist!" );
        return bRet;
    }

    if ( bRet )
    {
        if ( nIdx == USHRT_MAX )
            pImp->DeleteRegion( nRegion );
        else
            pRegion->DeleteEntry( nIdx );
    }

    return bRet;
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
    // create a new entry in hierarchy and another entry in
    // the (first?) template direktory, then update
    // RegionData_Impl list

    Content     aTemplateRoot;
    Content     aTemplateDir;
    sal_Bool    bNew;

    if ( !pImp->GetTemplateRoot( aTemplateRoot, bNew ) )
        return FALSE;

    if ( !pImp->GetTemplateDir( 0, aTemplateDir ) )
        return FALSE;

    OUString aTitle( rText );
    OUString aTargetURL = aDirs.GetToken( 0, C_DELIM );
    aTargetURL += OUString( '/' );
    aTargetURL += aTitle;

    if ( pImp->InsertNewRegionToHierarchy( aTemplateRoot, aTitle,
                                           aTargetURL, 0 ) )
    {
        DBG_ERRORFILE( "Entry already in hierarchy" );
        return FALSE;
    }

    if ( pImp->InsertNewRegionToFolder( aTemplateDir, aTitle ) )
    {
        DBG_ERRORFILE( "Entry already in folder" );
        return FALSE;
    }

    return TRUE;
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
    // find the data entry and rename the entry in the hierarchy as
    // well as the corresponding folder or file

    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );
    EntryData_Impl *pEntry = NULL;

    if ( !pRegion )
        return FALSE;

    Content aHierCont;
    Content aFolderCont;

    OUString aTitle( rName );
    OUString aRealTitle( rName );
    OUString aFolderURL = pRegion->GetTargetURL();
    OUString aHierURL( RTL_CONSTASCII_USTRINGPARAM( TEMPLATE_ROOT_URL ) );
    aHierURL += OUString( '/' );
    aHierURL += pRegion->GetTitle();

    sal_Bool bSetRealTitle( sal_False );

    if ( nIdx == USHRT_MAX )
    {
        sal_Int32 nStart = pRegion->GetRealTitle().getLength();
        aTitle += pRegion->GetTitle().copy( nStart );
        bSetRealTitle = sal_True;
    }
    else
    {
        EntryData_Impl *pEntry = pRegion->GetEntry( nIdx );
        if ( !pEntry )
            return FALSE;
        aFolderURL = pEntry->GetTargetURL();
        aHierURL += OUString( '/' );
        aHierURL += pEntry->GetTitle();
    }

    Reference< XCommandEnvironment > aCmdEnv;
    OUString aTitleStr( OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) ) );

    // Create a hierarchy Content with the old title and
    // rename it
    try
    {
        aHierCont = Content( aHierURL, aCmdEnv );
        aHierCont.setPropertyValue( aTitleStr, makeAny( aTitle ) );
        if ( bSetRealTitle )
        {
            OUString aPropName( OUString( RTL_CONSTASCII_USTRINGPARAM( REAL_NAME ) ) );
            aHierCont.setPropertyValue( aPropName, makeAny( aRealTitle ) );
        }
        aHierURL = aHierCont.get()->getIdentifier()->getContentIdentifier();
    }
    catch( ContentCreationException& )
    {
        DBG_ERRORFILE( "Hierarchy object doesn't exist?" );
        return FALSE;
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
        return FALSE;
    }

    // Create a folder Content with the old title and
    // rename it
    try
    {
        aFolderCont = Content( aFolderURL, aCmdEnv );
        aFolderCont.setPropertyValue( aTitleStr, makeAny( aRealTitle ) );
        aFolderURL = aFolderCont.get()->getIdentifier()->getContentIdentifier();
    }
    catch( ContentCreationException& )
    {
        DBG_ERRORFILE( "Folder object doesn't exist?" );
        return FALSE;
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
        return FALSE;
    }

    // Update the internal data structures
    if ( pEntry )
    {
        pEntry->SetTitle( aRealTitle );
        pEntry->SetTargetURL( aFolderURL );
        pEntry->SetHierarchyURL( aHierURL );
    }
    else
    {
        pRegion->SetTitle( aTitle );
        pRegion->SetRealTitle( aRealTitle );
        pRegion->SetTargetURL( aFolderURL );
        pRegion->SetHierarchyURL( aHierURL );
    }

    return TRUE;
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
    return FALSE;
#if 0   //dv!

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
#endif
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
    return NULL;
#if 0   //dv!
    DBG_ASSERT( pDirs, "not initialized" );
    return (*pDirs)[nRegion]->GetContent()[nIdx]->CreateObjectShell();
#endif
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
#if 0
    DBG_ASSERT( pDirs, "not initialized" );
    return (*pDirs)[nRegion]->GetContent()[nIdx]->DeleteObjectShell();
#endif
    return TRUE;
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
    const EntryData_Impl* pEntry = NULL;
    const USHORT nCount = GetRegionCount();
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
            RegionData_Impl *pRegion = pImp->GetRegion( i );
            if ( pRegion )
                pEntry = pRegion->GetEntry( rName );

            if ( pEntry )
                rPath = pEntry->GetTargetURL();
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
    INetURLObject aFullPath;

    aFullPath.SetSmartProtocol( INET_PROT_FILE );
    aFullPath.SetURL( rPath );
    aFullPath.CutLastName();

    OUString aPathTo = aFullPath.GetFull();

    RegionData_Impl *pData = NULL;
    EntryData_Impl  *pEntry = NULL;
    sal_Bool         bFound = sal_False;

    USHORT nCount = GetRegionCount();

    for ( USHORT i=0; !bFound && (i<nCount); i++ )
    {
        pData = pImp->GetRegion( i );
        if ( pData->GetTargetURL() == aPathTo )
        {
            USHORT nChildCount = pData->GetCount();
            OUString aPath( rPath );

            for ( USHORT j=0; !bFound && (j<nChildCount); j++ )
            {
                pEntry = pData->GetEntry( j );
                if ( pEntry->GetTargetURL() == aPath )
                {
                    bFound = sal_True;
                }
            }
        }
    }

    if ( bFound )
    {
        rRegion = pData->GetRealTitle();
        rName = pEntry->GetTitle();
    }

    return bFound;
}

//------------------------------------------------------------------------

SfxDocumentTemplates::SfxDocumentTemplates()

/*  [Beschreibung]

    Konstruktor
*/

:   pImp(0)
{
    aDirs = SvtPathOptions().GetTemplatePath();
    cDelim = ';'; // absichtlich hart verdrahtet
}

//-------------------------------------------------------------------------

void SfxDocumentTemplates::Construct()

//  verz"ogerter Aufbau der Verwaltungsdaten

{
    if ( !pImp )
        pImp = new SfxDocTemplate_Impl;

    pImp->Construct( aDirs );

#if 0   //dv!
    // schon von jemandem anders konstruiert?
    if ( pDirs )
        return;

    pDirs = new SfxTemplateDir;
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
        const CntStringItem& rItem = (const CntStringItem&) xRegion->Get( WID_TITLE );

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
#endif  //dv!
}

//------------------------------------------------------------------------

SfxDocumentTemplates::~SfxDocumentTemplates()

/*  [Beschreibung]

    Destruktor
    Freigeben der Verwaltungsdaten
*/

{
#if 0   //dv!
    DBG_ASSERT( pDirs, "not initialized" );
    delete pDirs;
    delete pDirCount;
#endif  //dv!
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


// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
EntryData_Impl::EntryData_Impl( const OUString& rTitle )
{
    maTitle     = rTitle;
    mbChecked   = sal_False;
}

// -----------------------------------------------------------------------
void EntryData_Impl::SetTargetURL( const OUString& rTargetURL )
{
    maTargetURL = rTargetURL;
}

// -----------------------------------------------------------------------
int EntryData_Impl::Compare( const OUString& rTitle ) const
{
    return maTitle.compareTo( rTitle );
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
RegionData_Impl::RegionData_Impl( const OUString& rTitle,
                                  const OUString& rRealTitle )
{
    maTitle         = rTitle;
    maRealTitle     = rRealTitle;
    mbChecked       = sal_False;
}

// -----------------------------------------------------------------------
RegionData_Impl::~RegionData_Impl()
{
    EntryData_Impl *pData = maEntries.First();

    while ( pData )
    {
        delete pData;
        pData = maEntries.Next();
    }
}

// -----------------------------------------------------------------------
void RegionData_Impl::SetTargetURL( const OUString& rTargetURL )
{
    if ( ! maTargetContent.get().is() )
    {
        try
        {
            Reference< XCommandEnvironment > aCmdEnv;
            maTargetContent = Content( rTargetURL, aCmdEnv );
        }
        catch( ContentCreationException& )
        {}
        catch( ... )
        { DBG_ERRORFILE( "Any other exception" ); }
    }

    maTargetURL = rTargetURL;

    if ( ! maTitle.getLength() )
    {
        INetURLObject aURLObject( rTargetURL );
        maTitle = aURLObject.getName();
    }
}

// -----------------------------------------------------------------------
void RegionData_Impl::SetParentURL( const OUString& rParentURL )
{
    maParentURL = rParentURL;
}

// -----------------------------------------------------------------------
long RegionData_Impl::GetEntryPos( const OUString& rTitle,
                                   sal_Bool& rFound ) const
{
    // use binary search to find the correct position
    // in the maEntries list

    int     nCompVal = 1;
    long    nStart = 0;
    long    nEnd = maEntries.Count() - 1;
    long    nMid;

    EntryData_Impl* pMid;

    rFound = sal_False;

    while ( nCompVal && ( nStart <= nEnd ) )
    {
        nMid = ( nEnd - nStart ) / 2 + nStart;
        pMid = maEntries.GetObject( nMid );

        nCompVal = pMid->Compare( rTitle );

        if ( nCompVal < 0 )     // pMid < pData
            nStart = nMid + 1;
        else
            nEnd = nMid - 1;
    }

    if ( nCompVal == 0 )
    {
        rFound = sal_True;
    }
    else
    {
        if ( nCompVal < 0 )     // pMid < pData
            nMid++;
    }

    return nMid;
}

// -----------------------------------------------------------------------
void RegionData_Impl::AddEntry( Content& rParentFolder,
                                const OUString& rTitle,
                                const OUString& rTargetURL )
{
    Content aLink;
    Reference< XCommandEnvironment > aCmdEnv;
    OUString aLinkURL( RTL_CONSTASCII_USTRINGPARAM( TEMPLATE_ROOT_URL ) );
    aLinkURL += OUString( '/' ) + maTitle + OUString( '/' ) + rTitle;

    try
    {
        aLink = Content( aLinkURL, aCmdEnv );
    }
    catch( ContentCreationException& )
    {
        Sequence< OUString > aNames(3);
        OUString* pNames = aNames.getArray();
        pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
        pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );
        pNames[2] = OUString( RTL_CONSTASCII_USTRINGPARAM( TARGET_URL ) );

        Sequence< Any > aValues(3);
        Any* pValues = aValues.getArray();
        pValues[0] = makeAny( rTitle );
        pValues[1] = makeAny( sal_Bool( sal_False ) );
        pValues[2] = makeAny( rTargetURL );

        OUString aType( RTL_CONSTASCII_USTRINGPARAM( TYPE_LINK ) );

        if ( ! rParentFolder.get().is() )
        {
            try
            {
                rParentFolder = Content( rTargetURL, aCmdEnv );
            }
            catch( ContentCreationException& )
            {
                return;
            }
            catch( ... )
            {
                DBG_ERRORFILE( "Any other exception" );
                return;
            }
        }

        try
        {
            rParentFolder.insertNewContent( aType, aNames, aValues, aLink );
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "Any other exception" );
        }
    }

    EntryData_Impl* pEntry = new EntryData_Impl( rTitle );
    pEntry->SetTargetURL( rTargetURL );
    pEntry->SetHierarchyURL( aLinkURL );

    sal_Bool    bFound = sal_False;
    long        nPos = GetEntryPos( pEntry->GetTitle(), bFound );

    if ( !bFound )
        maEntries.Insert( pEntry, nPos );
}

// -----------------------------------------------------------------------
USHORT RegionData_Impl::GetCount() const
{
    return maEntries.Count();
}

// -----------------------------------------------------------------------
EntryData_Impl* RegionData_Impl::GetEntry( const OUString& rName ) const
{
    sal_Bool    bFound = sal_False;
    long        nPos = GetEntryPos( rName, bFound );

    if ( bFound )
        return maEntries.GetObject( nPos );
    else
        return NULL;
}

// -----------------------------------------------------------------------
EntryData_Impl* RegionData_Impl::GetEntry( USHORT nIndex ) const
{
    return maEntries.GetObject( nIndex );
}

// -----------------------------------------------------------------------
void RegionData_Impl::DeleteEntry( USHORT nIndex )
{
    EntryData_Impl *pEntry = maEntries.GetObject( nIndex );

    if ( pEntry )
    {
        delete pEntry;
        maEntries.Remove( (ULONG) nIndex );
    }
}

// -----------------------------------------------------------------------
int RegionData_Impl::Compare( RegionData_Impl* pCompare,
                              sal_Bool* pbNamesAreEqual ) const
{
    int nCompare = maRealTitle.compareTo( pCompare->maRealTitle );

    if ( nCompare == 0 )
    {
        if ( pbNamesAreEqual )
            *pbNamesAreEqual = sal_True;

        nCompare = maTargetURL.compareTo( pCompare->maTargetURL );
    }

    return nCompare;
}


// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

SfxDocTemplate_Impl::SfxDocTemplate_Impl()
{
    mnRefCount = 0;
    mbConstructed = sal_False;
}

// -----------------------------------------------------------------------
SfxDocTemplate_Impl::~SfxDocTemplate_Impl()
{
    RegionData_Impl *pRegData = maRegions.First();

    while ( pRegData )
    {
        delete pRegData;
        pRegData = maRegions.Next();
    }
}

// -----------------------------------------------------------------------
RegionData_Impl* SfxDocTemplate_Impl::GetRegion( const OUString& rName )
    const
{
    USHORT nCount = maRegions.Count();
    RegionData_Impl *pData;

    for ( USHORT i=0; i<nCount; i++ )
    {
        pData = maRegions.GetObject( i );

        if ( pData->GetRealTitle() == rName )
            return pData;
    }

    return NULL;
}

// -----------------------------------------------------------------------
RegionData_Impl* SfxDocTemplate_Impl::GetRegionByPath(
                                            const String& rName ) const
{
    USHORT           nCount = maRegions.Count();
    OUString         aCompare( rName );
    RegionData_Impl *pData;

    for ( USHORT i=0; i<nCount; i++ )
    {
        pData = maRegions.GetObject( i );

        if ( pData->GetTargetURL() == aCompare )
            return pData;
    }

    return NULL;
}

// -----------------------------------------------------------------------
RegionData_Impl* SfxDocTemplate_Impl::GetRegion( USHORT nIndex ) const
{
    return maRegions.GetObject( nIndex );
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::DeleteRegion( USHORT nIndex )
{
    RegionData_Impl* pRegion = maRegions.GetObject( nIndex );

    if ( pRegion )
    {
        delete pRegion;
        maRegions.Remove( (ULONG) nIndex );
    }
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::AddRegion( const OUString& rTitle,
                                     const OUString& rRealTitle,
                                     const OUString& rTargetURL,
                                     Content& rContent )
{
    RegionData_Impl* pRegion;
    pRegion = new RegionData_Impl( rTitle, rRealTitle );
    pRegion->SetHierarchyContent( rContent );
    pRegion->SetTargetURL( rTargetURL );
    pRegion->SetHierarchyURL( rContent.get()->getIdentifier()->getContentIdentifier() );

    InsertOrMarkRegion( pRegion );

    // now get the content of the region
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(2);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );
    pProps[1] = OUString::createFromAscii( TARGET_URL );

    try
    {
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = rContent.createCursor( aProps, eInclude );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "createCursor: CommandAbortedException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "createCursor: Any other exception" );
    }

    if ( xResultSet.is() )
    {
        Reference< XCommandEnvironment > aCmdEnv;
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetDir( xRow->getString( 2 ) );

                pRegion->AddEntry( rContent, aTitle, aTargetDir );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
        }
    }
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::CreateFromHierarchy( Content &rTemplRoot )
{
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(4);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );
    pProps[1] = OUString::createFromAscii( TARGET_DIR_URL );
    pProps[2] = OUString::createFromAscii( PARENT_DIR_URL );
    pProps[3] = OUString::createFromAscii( REAL_NAME );

    try
    {
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        xResultSet = rTemplRoot.createCursor( aProps, eInclude );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "createCursor: CommandAbortedException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "createCursor: Any other exception" );
    }

    if ( xResultSet.is() )
    {
        Reference< XCommandEnvironment > aCmdEnv;
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetDir( xRow->getString( 2 ) );
                OUString aParentDir( xRow->getString( 3 ) );
                OUString aRealTitle( xRow->getString( 4 ) );

                OUString aId = xContentAccess->queryContentIdentifierString();
                Content  aContent = Content( aId, aCmdEnv );

                AddRegion( aTitle, aRealTitle, aTargetDir, aContent );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
        }
    }
}

// ------------------------------------------------------------------------
void SfxDocTemplate_Impl::Construct( const String& rDirs )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbConstructed )
        return;
    else
        mbConstructed = sal_True;

    maDirs = rDirs;

    Content     aTemplRoot;
    sal_Bool    bNewRoot;

    if ( ! GetTemplateRoot( aTemplRoot, bNewRoot ) )
    {
        DBG_ERRORFILE( "CreateTemplateHier(): Could not find/create template root" );
        return;
    }

    if ( bNewRoot )
    {
        USHORT      i;
        USHORT      nCount = rDirs.GetTokenCount( C_DELIM );

        for ( i=0; i<nCount; i++ )
        {
            Content aTemplDir;

            if ( GetTemplateDir( i, aTemplDir ) )
                GetFolders( aTemplRoot, aTemplDir, i );
        }
    }
    else
        CreateFromHierarchy( aTemplRoot );
}

// -----------------------------------------------------------------------

sal_Bool SfxDocTemplate_Impl::GetTemplateRoot( Content &rTemplRoot,
                                               sal_Bool &rNew ) const
{
    Reference < XCommandEnvironment > aCmdEnv;
    OUString    aTemplRootURL( RTL_CONSTASCII_USTRINGPARAM( TEMPLATE_ROOT_URL ) );
    sal_Bool    bRet = sal_False;

    rNew = sal_False;

    try
    {
        rTemplRoot = Content( aTemplRootURL, aCmdEnv );
        bRet = sal_True;
    }
    catch( ContentCreationException& )
    {

        OUString aHierRootURL( RTL_CONSTASCII_USTRINGPARAM( HIERARCHIE_ROOT_URL ) );

        Sequence< OUString > aNames(2);
        OUString* pNames = aNames.getArray();
        pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
        pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

        Sequence< Any > aValues(2);
        Any* pValues = aValues.getArray();
        pValues[0] = makeAny( OUString( RTL_CONSTASCII_USTRINGPARAM( TEMPLATE_DIR_NAME ) ) );
        pValues[1] = makeAny( sal_Bool( sal_True ) );

        try
        {
            Content aCnt( aHierRootURL, aCmdEnv );
            OUString aType( RTL_CONSTASCII_USTRINGPARAM( TYPE_FOLDER ) );
            bRet = aCnt.insertNewContent( aType, aNames, aValues, rTemplRoot );
            rNew = sal_True;
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "Any other exception" );
        }
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxDocTemplate_Impl::GetTemplateDir( USHORT nIndex,
                                              Content &rTemplateDir ) const
{
    Reference< XCommandEnvironment > aCmdEnv;
    INetURLObject aURL;

    sal_Bool    bRet = sal_True;
    String      aDir = maDirs.GetToken( nIndex, C_DELIM );

    aURL.SetSmartProtocol( INET_PROT_FILE );
    aURL.SetURL( aDir );

    try
    {
        rTemplateDir = Content( aURL.GetMainURL(), aCmdEnv );
    }
    catch( ContentCreationException& )
    {
        DBG_ERRORFILE( "GetTemplateDir(): Template directory doesn't exist" );
        bRet = sal_False;
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
        bRet = sal_False;
    }

    return bRet;
}

// -----------------------------------------------------------------------

void SfxDocTemplate_Impl::GetFolders( Content& rRoot,
                                      Content& rFolder,
                                      ULONG nIndex )
{
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(1);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );

    try
    {
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        xResultSet = rFolder.createCursor( aProps, eInclude );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "createCursor: CommandAbortedException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "createCursor: Any other exception" );
    }

    if ( xResultSet.is() )
    {
        Reference< XCommandEnvironment > aCmdEnv;
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        OUString aFolderURL = rFolder.get()->getIdentifier()->getContentIdentifier();
        OUString aRootURL = rRoot.get()->getIdentifier()->getContentIdentifier();
        aRootURL += OUString( '/' );
        OUString aIndex( '.' );
        aIndex += OUString::valueOf( (sal_Int32) nIndex );

        Content aFolder;
        Sequence< OUString > aNames(2);
        OUString* pNames = aNames.getArray();
        pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
        pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

        Sequence< OUString > aAdditionalProps(3);
        pNames = aAdditionalProps.getArray();
        pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
        pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( PARENT_DIR_URL ) );
        pNames[2] = OUString( RTL_CONSTASCII_USTRINGPARAM( REAL_NAME ) );

        Sequence< Any > aValues(2);
        Any* pValues = aValues.getArray();

        OUString aType = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPE_FOLDER ) );

        try
        {
            while ( xResultSet->next() )
            {
                sal_Bool    bWasInList;
                OUString    aTitle( xRow->getString(1) );

                if ( aTitle.compareToAscii( "wizard" ) == 0 )
                    continue;
                else if ( aTitle.compareToAscii( "internal" ) == 0 )
                    continue;

                OUString aURLTitle = aTitle + aIndex;
                OUString aNewFolderURL = aRootURL + aURLTitle;
                OUString aId = xContentAccess->queryContentIdentifierString();

                try
                {
                    aFolder = Content( aNewFolderURL, aCmdEnv );
                }
                catch( ContentCreationException& )
                {
                    pValues[0] = makeAny( aURLTitle );
                    pValues[1] = makeAny( sal_Bool( sal_True ) );

                    try
                    {
                        rRoot.insertNewContent( aType, aNames, aValues, aFolder );
                        Reference< XPropertySetInfo > xPropSet = aFolder.getProperties();
                        if ( xPropSet.is() )
                        {
                            pNames = aAdditionalProps.getArray();

                            if ( ! xPropSet->hasPropertyByName( pNames[0] ) )
                            {
                                Reference< XPropertyContainer > xFolderProp( aFolder.get(), UNO_QUERY );
                                if ( xFolderProp.is() )
                                {
                                    try
                                    {
                                        xFolderProp->addProperty( pNames[0], PropertyAttribute::MAYBEVOID,
                                                                  makeAny( aId ) );
                                        xFolderProp->addProperty( pNames[1], PropertyAttribute::MAYBEVOID,
                                                                  makeAny( aId ) );
                                        xFolderProp->addProperty( pNames[2], PropertyAttribute::MAYBEVOID,
                                                                  makeAny( aTitle ) );
                                    }
                                    catch( PropertyExistException& ) {}
                                    catch( IllegalTypeException& ) { DBG_ERRORFILE( "IllegalTypeException" ); }
                                    catch( IllegalArgumentException& ) { DBG_ERRORFILE( "IllegalArgumentException" ); }
                                }
                            }

                            Sequence< Any > aPropValues(3);
                            Any* pPropValues = aPropValues.getArray();

                            pPropValues[0] = makeAny( aId );
                            pPropValues[1] = makeAny( aFolderURL );
                            pPropValues[2] = makeAny( aTitle );

                            aFolder.setPropertyValues( aAdditionalProps, aPropValues );
                        }
                    }
                    catch( CommandAbortedException& )
                    {
                        DBG_ERRORFILE( "CommandAbortedException" );
                    }
                    catch( ... )
                    {
                        DBG_ERRORFILE( "Any other exception" );
                    }
                }

                RegionData_Impl *pRegion = new RegionData_Impl( aURLTitle, aTitle );
                pRegion->SetTargetURL( aId );
                pRegion->SetParentURL( aFolderURL );
                pRegion->SetHierarchyURL( aNewFolderURL );

                bWasInList = InsertOrMarkRegion( pRegion );

                Content aSubFolder( xContentAccess->queryContent(), aCmdEnv );
                GetTemplates( aSubFolder, aFolder, pRegion, aNewFolderURL );

                if ( bWasInList )
                {
                    delete pRegion;
                }
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
        }
    }
}

// -----------------------------------------------------------------------

void SfxDocTemplate_Impl::GetTemplates( Content& rTargetFolder,
                                        Content& rParentFolder,
                                        RegionData_Impl* pRegion,
                                        const OUString& rParentURL )
{
    Reference< XResultSet > xResultSet;
    Sequence< OUString >    aProps(1);
    OUString* pProps = aProps.getArray();

    pProps[0] = OUString::createFromAscii( TITLE );

    try
    {
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = rTargetFolder.createCursor( aProps, eInclude );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "createCursor: CommandAbortedException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "createCursor: Any other exception" );
    }

    if ( xResultSet.is() )
    {
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString(1) );
                OUString aId = xContentAccess->queryContentIdentifierString();

                pRegion->AddEntry( rParentFolder, aTitle, aId );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
        }
    }
}


// -----------------------------------------------------------------------

long SfxDocTemplate_Impl::GetRegionPos( const OUString& rTitle,
                                        sal_Bool& rFound ) const
{
    int     nCompVal = 1;
    long    nStart = 0;
    long    nEnd = maRegions.Count() - 1;
    long    nMid;

    RegionData_Impl* pMid;

    while ( nCompVal && ( nStart <= nEnd ) )
    {
        nMid = ( nEnd - nStart ) / 2 + nStart;
        pMid = maRegions.GetObject( nMid );

        nCompVal = pMid->Compare( rTitle );

        if ( nCompVal < 0 )     // pMid < pData
            nStart = nMid + 1;
        else
            nEnd = nMid - 1;
    }

    if ( nCompVal == 0 )
        rFound = TRUE;
    else
    {
        if ( nCompVal < 0 )     // pMid < pData
            nMid++;

        rFound = sal_False;
    }

    return nMid;
}

// -----------------------------------------------------------------------

sal_Bool SfxDocTemplate_Impl::InsertOrMarkRegion( RegionData_Impl *pData )
{
    ::osl::MutexGuard   aGuard( maMutex );

    int nCompVal = 1;
    long    nStart = 0;
    long    nEnd = maRegions.Count() - 1;
    long    nMid;

    RegionData_Impl* pMid;
    sal_Bool         bFound;
    sal_Bool         bSameName = sal_False;

    while ( nCompVal && ( nStart <= nEnd ) )
    {
        nMid = ( nEnd - nStart ) / 2 + nStart;
        pMid = maRegions.GetObject( nMid );

        nCompVal = pMid->Compare( pData, &bSameName );

        if ( nCompVal < 0 )     // pMid < pData
            nStart = nMid + 1;
        else
            nEnd = nMid - 1;
    }

    if ( nCompVal == 0 )
    {
        bFound = TRUE;
        pMid->SetChecked( sal_True );
    }
    else
    {
        if ( nCompVal < 0 )     // pMid < pData
            nMid++;

        maRegions.Insert( pData, nMid );
        bFound = sal_False;
    }

    return bFound;
}

// -----------------------------------------------------------------------

sal_Bool SfxDocTemplate_Impl::InsertNewRegionToHierarchy(
                                        Content &rRoot,
                                        const OUString &rTitle,
                                        const OUString &rTargetURL,
                                        USHORT nIndex )
{
    Reference< XCommandEnvironment > aCmdEnv;

    OUString aFolderURL = maDirs.GetToken( nIndex, C_DELIM );

    OUString aRootURL = rRoot.get()->getIdentifier()->getContentIdentifier();
    aRootURL += OUString( '/' );

    OUString aIndex( '.' );
    aIndex += OUString::valueOf( (sal_Int32) nIndex );

    Sequence< OUString > aNames(2);
    Sequence< Any > aValues(2);

    OUString* pNames = aNames.getArray();
    pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
    pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

    Sequence< OUString > aAdditionalProps(3);
    pNames = aAdditionalProps.getArray();
    pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
    pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( PARENT_DIR_URL ) );
    pNames[2] = OUString( RTL_CONSTASCII_USTRINGPARAM( REAL_NAME ) );

    OUString aType = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPE_FOLDER ) );

    sal_Bool    bExists = sal_False;

    OUString aURLTitle = rTitle + aIndex;
    OUString aNewFolderURL = aRootURL + aURLTitle;

    Content aFolder;

    try
    {
        aFolder = Content( aNewFolderURL, aCmdEnv );
        bExists = sal_True;
    }
    catch( ContentCreationException& )
    {
        Any* pValues = aValues.getArray();
        pValues[0] = makeAny( aURLTitle );
        pValues[1] = makeAny( sal_Bool( sal_True ) );

        try
        {
            rRoot.insertNewContent( aType, aNames, aValues, aFolder );
            Reference< XPropertySetInfo > xPropSet = aFolder.getProperties();
            if ( xPropSet.is() )
            {
                pNames = aAdditionalProps.getArray();

                if ( ! xPropSet->hasPropertyByName( pNames[0] ) )
                {
                    Reference< XPropertyContainer > xFolderProp( aFolder.get(), UNO_QUERY );
                    if ( xFolderProp.is() )
                    {
                        try
                        {
                            xFolderProp->addProperty( pNames[0], PropertyAttribute::MAYBEVOID,
                                                      makeAny( rTargetURL ) );
                            xFolderProp->addProperty( pNames[1], PropertyAttribute::MAYBEVOID,
                                                      makeAny( aFolderURL ) );
                            xFolderProp->addProperty( pNames[2], PropertyAttribute::MAYBEVOID,
                                                      makeAny( rTitle ) );
                        }
                        catch( PropertyExistException& ) {}
                        catch( IllegalTypeException& ) { DBG_ERRORFILE( "IllegalTypeException" ); }
                        catch( IllegalArgumentException& ) { DBG_ERRORFILE( "IllegalArgumentException" ); }
                    }
                }

                Sequence< Any > aPropValues(3);
                Any* pPropValues = aPropValues.getArray();

                pPropValues[0] = makeAny( rTargetURL );
                pPropValues[1] = makeAny( aFolderURL );
                pPropValues[2] = makeAny( rTitle );

                aFolder.setPropertyValues( aAdditionalProps, aPropValues );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "Any other exception" );
        }
    }

    if ( !bExists )
    {
        RegionData_Impl *pRegion = new RegionData_Impl( aURLTitle, rTitle );
        pRegion->SetTargetURL( rTargetURL );
        pRegion->SetParentURL( aFolderURL );
        pRegion->SetHierarchyURL( aNewFolderURL );

        bExists = InsertOrMarkRegion( pRegion );

        if ( bExists )
        {
            delete pRegion;
        }
    }

    return bExists;
}

// -----------------------------------------------------------------------

sal_Bool SfxDocTemplate_Impl::InsertNewRegionToFolder(
                                        Content &rRoot,
                                        const OUString &rTitle )
{
    Reference< XCommandEnvironment > aCmdEnv;

    Sequence< OUString > aNames(2);
    Sequence< Any > aValues(2);

    OUString* pNames = aNames.getArray();
    pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
    pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

    Any* pValues = aValues.getArray();
    pValues[0] = makeAny( rTitle );
    pValues[1] = makeAny( sal_Bool( sal_True ) );

    OUString aType = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPE_FSYS_FOLDER ) );

    sal_Bool bExists = sal_False;

    Content aFolder;

    try
    {
        bExists = ! rRoot.insertNewContent( aType, aNames, aValues, aFolder );
    }
    catch( CommandAbortedException& )
    {
        bExists = sal_True;
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ... )
    {
        bExists = sal_True;
        DBG_ERRORFILE( "Any other exception" );
    }

    return bExists;
}
