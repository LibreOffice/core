/*************************************************************************
 *
 *  $RCSfile: doctempl.cxx,v $
 *
 *  $Revision: 1.37 $
 *
 *  last change: $Author: dv $ $Date: 2001-04-06 14:12:28 $
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

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif

#ifndef _SV_RESARY_HXX
#include <vcl/resary.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif

#ifndef  _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#include <tools/string.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <ucbhelper/content.hxx>

#include <com/sun/star/uno/Reference.h>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

#ifdef TF_FILTER
#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif
#else
#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADERQUERY_HPP_
#include <com/sun/star/frame/XFrameLoaderQuery.hpp>
#endif
#endif

#ifndef  _COM_SUN_STAR_FRAME_XDOCUMENTTEMPLATES_HPP_
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSIST_HPP_
#include <com/sun/star/io/XPersist.hpp>
#endif

#ifndef  _COM_SUN_STAR_LANG_XLOCALIZABLE_HPP_
#include <com/sun/star/lang/XLocalizable.hpp>
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
using namespace ::com::sun::star::frame;
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
#include "sfxresid.hxx"
#include "doc.hrc"
#include "fcontnr.hxx"

//========================================================================

// #define DONT_USE_HIERARCHY

#define TITLE                   "Title"
#define IS_FOLDER               "IsFolder"
#define PROPERTY_TYPE           "TypeDescription"
#define TARGET_URL              "TargetURL"
#define TYPE_FOLDER             "application/vnd.sun.star.hier-folder"
#define TYPE_LINK               "application/vnd.sun.star.hier-link"
#define TYPE_FSYS_FOLDER        "application/vnd.sun.staroffice.fsys-folder"

#define TARGET_DIR_URL          "TargetDirURL"
#define COMMAND_DELETE          "delete"
#define COMMAND_TRANSFER        "transfer"

#define STANDARD_FOLDER         "standard"

#define SERVICENAME_TYPEDETECTION       "com.sun.star.document.TypeDetection"
#define TYPEDETECTION_PARAMETER         "FileName"
#define SERVICENAME_OLD_TYPEDETECTION   "com.sun.star.frame.FrameLoaderFactory"
#define PARAMETER_OLD_TYPEDETECTION     "DeepDetection"
#define SERVICENAME_DOCINFO             "com.sun.star.document.DocumentProperties"
#define SERVICENAME_DOCTEMPLATES        "com.sun.star.frame.DocumentTemplates"

//========================================================================

class RegionData_Impl;

namespace DocTempl {

class EntryData_Impl
{
    RegionData_Impl*    mpParent;
    SfxObjectShellLock  mxObjShell;
    SvStorageRef        mxStor;
    OUString            maTitle;
    OUString            maOwnURL;
    OUString            maTargetURL;
    sal_Bool            mbIsOwner   : 1;
    sal_Bool            mbDidConvert: 1;

private:
    RegionData_Impl*    GetParent() const { return mpParent; }

public:
                        EntryData_Impl( RegionData_Impl* pParent,
                                        const OUString& rTitle );

    const OUString&     GetTitle() const { return maTitle; }
    const OUString&     GetTargetURL();
    const OUString&     GetHierarchyURL();

    void                SetTitle( const OUString& rTitle ) { maTitle = rTitle; }
    void                SetTargetURL( const OUString& rURL ) { maTargetURL = rURL; }
    void                SetHierarchyURL( const OUString& rURL) { maOwnURL = rURL; }

    int                 Compare( const OUString& rTitle ) const;

    SfxObjectShellRef   CreateObjectShell();
    BOOL                DeleteObjectShell();
};

};

using namespace ::DocTempl;

DECLARE_LIST( EntryList_Impl, EntryData_Impl* );

// ------------------------------------------------------------------------

class RegionData_Impl
{
    const SfxDocTemplate_Impl*  mpParent;
    EntryList_Impl              maEntries;
    OUString                    maTitle;
    OUString                    maOwnURL;
    OUString                    maTargetURL;

private:
    long                        GetEntryPos( const OUString& rTitle,
                                             sal_Bool& rFound ) const;
    const SfxDocTemplate_Impl*  GetParent() const { return mpParent; }

public:
                        RegionData_Impl( const SfxDocTemplate_Impl* pParent,
                                         const OUString& rTitle );
                        ~RegionData_Impl();

    void                SetTargetURL( const OUString& rURL ) { maTargetURL = rURL; }
    void                SetHierarchyURL( const OUString& rURL) { maOwnURL = rURL; }

    EntryData_Impl*     GetEntry( ULONG nIndex ) const;
    EntryData_Impl*     GetEntry( const OUString& rName ) const;
    EntryData_Impl*     GetByTargetURL( const OUString& rName ) const;

    const OUString&     GetTitle() const { return maTitle; }
    const OUString&     GetTargetURL();
    const OUString&     GetHierarchyURL();

    ULONG               GetCount() const;

    void                SetTitle( const OUString& rTitle ) { maTitle = rTitle; }

    void                AddEntry( const OUString& rTitle,
                                  const OUString& rTargetURL,
                                  USHORT *pPos = NULL );
    void                DeleteEntry( ULONG nIndex );

    int                 Compare( const OUString& rTitle ) const
                            { return maTitle.compareTo( rTitle ); }
    int                 Compare( RegionData_Impl* pCompareWith ) const;
};

DECLARE_LIST( RegionList_Impl, RegionData_Impl* );

// ------------------------------------------------------------------------

class SfxDocTemplate_Impl : public SvRefBase
{
    Reference< XPersist >               mxInfo;
    Reference< XDocumentTemplates >     mxTemplates;

    ::osl::Mutex        maMutex;
    OUString            maRootURL;
    OUString            maStandardGroup;
    RegionList_Impl     maRegions;
    sal_Bool            mbConstructed;

private:
    void                Clear();

public:
                        SfxDocTemplate_Impl();
                        ~SfxDocTemplate_Impl();

    sal_Bool            Construct();
    void                CreateFromHierarchy( Content &rTemplRoot );
    void                AddRegion( const OUString& rTitle,
                                   Content& rContent );

    void                Rescan();

    void                DeleteRegion( ULONG nIndex );

    ULONG               GetRegionCount() const
                            { return maRegions.Count(); }
    RegionData_Impl*    GetRegion( const OUString& rName ) const;
    RegionData_Impl*    GetRegion( ULONG nIndex ) const;
    void                GetTemplates( Content& rTargetFolder,
                                      Content& rParentFolder,
                                      RegionData_Impl* pRegion );

    long                GetRegionPos( const OUString& rTitle,
                                      sal_Bool& rFound ) const;

    sal_Bool            DoUpdate();

    void                GetTitleFromURL( const OUString& rURL, OUString& aTitle );
    sal_Bool            InsertRegion( RegionData_Impl *pData, ULONG nPos = LIST_APPEND );
    OUString            GetRootURL() const { return maRootURL; }

    Reference< XDocumentTemplates >     getDocTemplates() { return mxTemplates; }
};

// ------------------------------------------------------------------------

#ifndef SFX_DECL_DOCTEMPLATES_DEFINED
#define SFX_DECL_DOCTEMPLATES_DEFINED
SV_DECL_REF(SfxDocTemplate_Impl)
#endif

SV_IMPL_REF(SfxDocTemplate_Impl)

// ------------------------------------------------------------------------

SfxDocTemplate_Impl *gpTemplateData = 0;

// -----------------------------------------------------------------------

static sal_Bool getTextProperty_Impl( Content& rContent,
                                      const OUString& rPropName,
                                      OUString& rPropValue );

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
    // First: find the RegionData for the index
    String aName;

    if ( pImp->Construct() )
    {
        RegionData_Impl *pData1 = pImp->GetRegion( nIdx );

        if ( pData1 )
            aName = pData1->GetTitle();

        // --**-- here was some code which appended the path to the
        //      group if there was more than one with the same name.
        //      this should not happen anymore
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

    if ( pImp->Construct() )
    {
        RegionData_Impl *pData = pImp->GetRegion( nIdx );

        if ( pData )
            maTmpString = pData->GetTitle();
        else
            maTmpString.Erase();
    }
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
    if ( !pImp->Construct() )
        return USHRT_MAX;

    sal_Bool    bFound;
    ULONG       nIndex = pImp->GetRegionPos( rRegion, bFound );

    if ( bFound )
        return (USHORT) nIndex;
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
    if ( !pImp->Construct() )
        return 0;

    ULONG nCount = pImp->GetRegionCount();

    return (USHORT) nCount;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::IsRegionLoaded( USHORT nIdx ) const
{
    if ( !pImp->Construct() )
        return FALSE;

    RegionData_Impl *pData = pImp->GetRegion( nIdx );

    if ( pData )
        return TRUE;
    else
        return FALSE;
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
    if ( !pImp->Construct() )
        return 0;

    RegionData_Impl *pData = pImp->GetRegion( rName );
    ULONG            nCount = 0;

    if ( pData )
        nCount = pData->GetCount();

    return (USHORT) nCount;
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
    if ( !pImp->Construct() )
        return 0;

    RegionData_Impl *pData = pImp->GetRegion( nRegion );
    ULONG            nCount = 0;

    if ( pData )
        nCount = pData->GetCount();

    return (USHORT) nCount;
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
    static String maTmpString;

    if ( pImp->Construct() )
    {
        EntryData_Impl  *pEntry = NULL;
        RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

        if ( pRegion )
            pEntry = pRegion->GetEntry( nIdx );

        if ( pEntry )
            maTmpString = pEntry->GetTitle();
        else
            maTmpString.Erase();
    }
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
    if ( !pImp->Construct() )
        return String();

    EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( pRegion )
        pEntry = pRegion->GetEntry( nIdx );

    if ( pEntry )
    {
        INetURLObject aURLObj( pEntry->GetTargetURL() );
        return aURLObj.getName( INetURLObject::LAST_SEGMENT, true,
                                INetURLObject::DECODE_WITH_CHARSET );
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
    if ( !pImp->Construct() )
        return String();

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
    if ( !pImp->Construct() )
        return String();

    EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( pRegion )
        pEntry = pRegion->GetEntry( rLongName );

    if ( pEntry )
        return pEntry->GetTargetURL();
    else if ( pRegion )
    {
        // --**-- extension handling will become more complicated, because
        //          every new document type will have it's own extension
        //          e.g.: .stw or .stc instead of .vor
        INetURLObject aURLObj( pRegion->GetTargetURL() );
        aURLObj.insertName( rLongName, false,
                     INetURLObject::LAST_SEGMENT, true,
                     INetURLObject::ENCODE_ALL );

        OUString aExtension = aURLObj.getExtension();

        if ( ! aExtension.len() )
            aURLObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM( "vor" ) ) );

        return aURLObj.GetMainURL();
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
    if ( ! pImp->Construct() )
        return String();

    // the first region in the list should always be the standard
    // group
    // --**-- perhaps we have to create it ???
    RegionData_Impl *pRegion = pImp->GetRegion( 0L );
    EntryData_Impl  *pEntry = NULL;

    if ( pRegion )
        pEntry = pRegion->GetEntry( rLongName );

    if ( pEntry )
        return pEntry->GetTargetURL();
    else if ( pRegion )
    {
        INetURLObject aURLObj( pRegion->GetTargetURL() );
        aURLObj.insertName( rLongName, false,
                     INetURLObject::LAST_SEGMENT, true,
                     INetURLObject::ENCODE_ALL );

        OUString aExtension = aURLObj.getExtension();

        if ( ! aExtension.len() )
            aURLObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM( "vor" ) ) );

        return aURLObj.GetMainURL();
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
    if ( ! pImp->Construct() )
        return;

    EntryData_Impl  *pEntry;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    // do nothing if there is no region with that index
    if ( !pRegion )
        return;

    pEntry = pRegion->GetEntry( rLongName );

    // do nothing if there is already an entry with that name
    if ( pEntry )
        return;

    Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();

    if ( xTemplates->addTemplate( pRegion->GetTitle(), rLongName, rFileName ) )
        pRegion->AddEntry( rLongName, rFileName );
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

    if ( !pImp->Construct() )
        return FALSE;

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

    OUString aTitle;

    if ( pTarget )
        aTitle = pTarget->GetTitle();
    else
        aTitle = pSource->GetTitle();

    Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();

    if ( xTemplates->addTemplate( pTargetRgn->GetTitle(),
                                  aTitle,
                                  pSource->GetTargetURL() ) )
    {
        if ( bMove )
        {
            pSourceRgn->DeleteEntry( nSourceIdx );
            // --**-- delete the original file
        }

        INetURLObject aNewTarget( pTargetRgn->GetTargetURL() );

        aNewTarget.insertName( aTitle, false,
                     INetURLObject::LAST_SEGMENT, true,
                     INetURLObject::ENCODE_ALL );

        pTargetRgn->AddEntry( aTitle, aNewTarget.GetMainURL() );

        return sal_True;
    }

    // --**-- wenn aktuell das File geoeffnet ist,
    // muss es hinterher wieder geoeffnet werden

    return sal_False;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::Move
(
    USHORT nTargetRegion,       //  Index des Zielbereiches
    USHORT nTargetIdx,          //  Index Zielposition
    USHORT nSourceRegion,       //  Index des Quellbereiches
    USHORT nSourceIdx           /*  Index der zu kopierenden / zu verschiebenden
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
    if ( ! pImp->Construct() )
        return FALSE;

    RegionData_Impl *pSourceRgn = pImp->GetRegion( nRegion );
    if ( !pSourceRgn )
        return FALSE;

    EntryData_Impl *pSource = pSourceRgn->GetEntry( nIdx );
    if ( !pSource )
        return FALSE;

    INetURLObject aTargetURL( rName );

    OUString aTitle( aTargetURL.getName( INetURLObject::LAST_SEGMENT, true,
                                         INetURLObject::DECODE_WITH_CHARSET ) );
    aTargetURL.CutName();

    OUString aParentURL = aTargetURL.GetMainURL();

    Reference< XCommandEnvironment > aCmdEnv;
    Content aTarget;

    try
    {
        aTarget = Content( aParentURL, aCmdEnv );

        TransferInfo aTransferInfo;
        aTransferInfo.MoveData = FALSE;
        aTransferInfo.SourceURL = pSource->GetTargetURL();
        aTransferInfo.NewTitle = aTitle;
        aTransferInfo.NameClash = NameClash::OVERWRITE;

        Any aArg = makeAny( aTransferInfo );
        OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( COMMAND_TRANSFER ) );

        aTarget.executeCommand( aCmd, aArg );
    }
    catch ( ContentCreationException& )
    { return FALSE; }
    catch ( CommandAbortedException& )
    { return FALSE; }
    catch ( Exception& )
    { return FALSE; }

    return TRUE;
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

    BOOL                            TRUE
                                    Aktion konnte ausgef"uhrt werden

                                    FALSE
                                    Aktion konnte nicht ausgef"uhrt werden

    [Querverweise]

    <SfxDocumentTemplates::CopyTo(USHORT,USHORT,const String&)>
*/

{
    if ( ! pImp->Construct() )
        return FALSE;

    RegionData_Impl *pTargetRgn = pImp->GetRegion( nRegion );

    if ( !pTargetRgn )
        return FALSE;

    OUString aTitle;

    pImp->GetTitleFromURL( rName, aTitle );

    Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();

    if ( xTemplates->addTemplate( pTargetRgn->GetTitle(),
                                  aTitle, rName ) )
    {
        if ( nIdx == USHRT_MAX )
            nIdx = 0;
        else
            nIdx += 1;
        pTargetRgn->AddEntry( aTitle, rName, &nIdx );
        rName = aTitle;
        return TRUE;
    }

    return FALSE;
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
    if ( ! pImp->Construct() )
        return FALSE;

    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( !pRegion )
        return FALSE;

    sal_Bool bRet;
    Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();

    if ( nIdx == USHRT_MAX )
    {
        bRet = xTemplates->removeGroup( pRegion->GetTitle() );
        if ( bRet )
            pImp->DeleteRegion( nRegion );
    }
    else
    {
        EntryData_Impl *pEntry = pRegion->GetEntry( nIdx );

        if ( !pEntry )
            return FALSE;

        bRet = xTemplates->removeTemplate( pRegion->GetTitle(),
                                           pEntry->GetTitle() );
        if( bRet )
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
    if ( ! pImp->Construct() )
        return sal_False;

    RegionData_Impl *pRegion = pImp->GetRegion( rText );

    if ( pRegion )
        return sal_False;

    Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();

    if ( xTemplates->addGroup( rText ) )
    {
        RegionData_Impl* pRegion = new RegionData_Impl( pImp, rText );

        if ( ! pImp->InsertRegion( pRegion, nRegion ) )
        {
            delete pRegion;
            return sal_False;
        }
        return sal_True;
    }

    return sal_False;
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
    if ( ! pImp->Construct() )
        return FALSE;

    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );
    EntryData_Impl *pEntry = NULL;

    if ( !pRegion )
        return FALSE;

    Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();
    OUString aEmpty;

    if ( nIdx == USHRT_MAX )
    {
        if ( pRegion->GetTitle() == OUString( rName ) )
            return sal_True;

        // we have to rename a region
        if ( xTemplates->renameGroup( pRegion->GetTitle(), rName ) )
        {
            pRegion->SetTitle( rName );
            pRegion->SetTargetURL( aEmpty );
            pRegion->SetHierarchyURL( aEmpty );
            return sal_True;
        }
    }
    else
    {
        pEntry = pRegion->GetEntry( nIdx );

        if ( !pEntry )
            return FALSE;

        if ( pEntry->GetTitle() == OUString( rName ) )
            return TRUE;

        if ( xTemplates->renameTemplate( pRegion->GetTitle(),
                                         pEntry->GetTitle(),
                                         rName ) )
        {
            pEntry->SetTitle( rName );
            pEntry->SetTargetURL( aEmpty );
            pEntry->SetHierarchyURL( aEmpty );
            return sal_True;
        }
    }

    return sal_False;
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
    if ( ! pImp->Construct() )
        return FALSE;

    pImp->Rescan();

    return sal_True;
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
    if ( !pImp->Construct() )
        return NULL;

    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );
    EntryData_Impl *pEntry = NULL;

    if ( pRegion )
        pEntry = pRegion->GetEntry( nIdx );

    if ( pEntry )
        return pEntry->CreateObjectShell();
    else
        return NULL;
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
    if ( ! pImp->Construct() )
        return TRUE;

    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );
    EntryData_Impl *pEntry = NULL;

    if ( pRegion )
        pEntry = pRegion->GetEntry( nIdx );

    if ( pEntry )
        return pEntry->DeleteObjectShell();
    else
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
    // We don't search for empty names!
    if ( ! rName.Len() )
        return FALSE;

    if ( ! pImp->Construct() )
        return FALSE;

    EntryData_Impl* pEntry = NULL;
    const USHORT nCount = GetRegionCount();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        RegionData_Impl *pRegion = pImp->GetRegion( i );

        if( pRegion &&
            ( !rRegion.Len() || ( rRegion == String( pRegion->GetTitle() ) ) ) )
        {
            pEntry = pRegion->GetEntry( rName );

            if ( pEntry )
            {
                rPath = pEntry->GetTargetURL();
                break;
            }
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
    if ( ! pImp->Construct() )
        return FALSE;

    INetURLObject aFullPath;

    aFullPath.SetSmartProtocol( INET_PROT_FILE );
    aFullPath.SetURL( rPath );
    aFullPath.CutLastName();

    OUString aPathTo = aFullPath.GetMainURL();

    RegionData_Impl *pData = NULL;
    EntryData_Impl  *pEntry = NULL;
    sal_Bool         bFound = sal_False;

    ULONG nCount = GetRegionCount();

    for ( ULONG i=0; !bFound && (i<nCount); i++ )
    {
        pData = pImp->GetRegion( i );
        if ( pData->GetTargetURL() == aPathTo )
        {
            ULONG nChildCount = pData->GetCount();
            OUString aPath( rPath );

            for ( ULONG j=0; !bFound && (j<nChildCount); j++ )
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
        rRegion = pData->GetTitle();
        rName = pEntry->GetTitle();
    }

    return bFound;
}

//------------------------------------------------------------------------

SfxDocumentTemplates::SfxDocumentTemplates()

/*  [Beschreibung]

    Konstruktor
*/
{
    if ( !gpTemplateData )
        gpTemplateData = new SfxDocTemplate_Impl;

    pImp = gpTemplateData;
}

//-------------------------------------------------------------------------

void SfxDocumentTemplates::Construct()

//  verz"ogerter Aufbau der Verwaltungsdaten

{
//  pImp->Construct();
}

//------------------------------------------------------------------------

SfxDocumentTemplates::~SfxDocumentTemplates()

/*  [Beschreibung]

    Destruktor
    Freigeben der Verwaltungsdaten
*/

{
    pImp = NULL;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
EntryData_Impl::EntryData_Impl( RegionData_Impl* pParent,
                                const OUString& rTitle )
{
    mpParent    = pParent;
    maTitle     = rTitle;
    mbIsOwner   = sal_False;
    mbDidConvert= sal_False;
}

// -----------------------------------------------------------------------
int EntryData_Impl::Compare( const OUString& rTitle ) const
{
    return maTitle.compareTo( rTitle );
}

//------------------------------------------------------------------------
SfxObjectShellRef EntryData_Impl::CreateObjectShell()
{
    if( ! mxObjShell.Is() )
    {
        mbIsOwner = FALSE;
        BOOL bDum = FALSE;
        SfxApplication *pSfxApp = SFX_APP();
        String          aTargetURL = GetTargetURL();

        mxObjShell = pSfxApp->DocAlreadyLoaded( aTargetURL, TRUE, bDum );

        if( ! mxObjShell.Is() )
        {
            mbIsOwner = TRUE;
            SfxMedium *pMed=new SfxMedium(
                aTargetURL,(STREAM_READ | STREAM_SHARE_DENYWRITE),  FALSE, 0 );
            const SfxFilter* pFilter = NULL;
            if( pSfxApp->GetFilterMatcher().GuessFilter(
                *pMed, &pFilter, SFX_FILTER_TEMPLATE, 0 ) ||
                pFilter && !pFilter->IsOwnFormat() ||
                pFilter && !pFilter->UsesStorage() )
            {
                SfxErrorContext aEc( ERRCTX_SFX_LOADTEMPLATE,
                                     aTargetURL );
                delete pMed;
                mbDidConvert=TRUE;
                ULONG lErr;
                if ( mxObjShell.Is() )
                    if(lErr=(pSfxApp->LoadTemplate(
                        mxObjShell,aTargetURL)!=ERRCODE_NONE))
                        ErrorHandler::HandleError(lErr);

            }
            else
            {
                const SfxObjectFactory &rFactory =
                    ((SfxFactoryFilterContainer*)pFilter->GetFilterContainer())
                    ->GetFactory();
                delete pMed;
                mbDidConvert=FALSE;
                mxStor = new SvStorage(
                    aTargetURL,
                    STREAM_READWRITE | STREAM_NOCREATE |
                    STREAM_SHARE_DENYALL, STORAGE_TRANSACTED);
                if ( pFilter )
                    mxStor->SetVersion( pFilter->GetVersion() );
                if ( SVSTREAM_OK == mxStor->GetError() )
                {
                    mxObjShell = (SfxObjectShell *)
                        rFactory.CreateObject(SFX_CREATE_MODE_ORGANIZER);
                    if ( mxObjShell.Is() )
                    {
                        mxObjShell->DoInitNew(0);
                        if(!mxObjShell->LoadFrom( mxStor ))
                            mxObjShell.Clear();
                        else
                        {
                            mxObjShell->DoHandsOff();
                            mxObjShell->DoSaveCompleted( mxStor );
                        }
                    }
                }
            }
        }
    }

    return (SfxObjectShellRef)(SfxObjectShell*) mxObjShell;
}

//------------------------------------------------------------------------
BOOL EntryData_Impl::DeleteObjectShell()
{
    BOOL bRet = TRUE;

    if ( mxObjShell.Is() )
    {
        if( mxObjShell->IsModified() )
        {
            //Hier speichern wir auch, falls die Vorlage in Bearbeitung ist...
            bRet = FALSE;
            if ( mbIsOwner )
                if( mbDidConvert )
                {
                    bRet=mxObjShell->PreDoSaveAs_Impl(
                        GetTargetURL(),
                        mxObjShell->GetFactory().GetFilter(0)->GetName(),0 );
                }
                else
                {
                    if( mxObjShell->Save() )
                        bRet = mxStor->Commit();
                    else
                        bRet = FALSE;
                }
        }
        if( bRet )
        {
            mxObjShell.Clear();
            mxStor.Clear();
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------
const OUString& EntryData_Impl::GetHierarchyURL()
{
    if ( !maOwnURL.getLength() )
    {
        INetURLObject aTemplateObj( GetParent()->GetHierarchyURL() );

        aTemplateObj.insertName( GetTitle(), false,
                     INetURLObject::LAST_SEGMENT, true,
                     INetURLObject::ENCODE_ALL );

        maOwnURL = aTemplateObj.GetMainURL();
        DBG_ASSERT( maOwnURL.getLength(), "GetHierarchyURL(): Could not create URL!" );
    }

    return maOwnURL;
}

// -----------------------------------------------------------------------
const OUString& EntryData_Impl::GetTargetURL()
{
    if ( !maTargetURL.getLength() )
    {
        Reference< XCommandEnvironment > aCmdEnv;
        Content aRegion;

        if ( Content::create( GetHierarchyURL(), aCmdEnv, aRegion ) )
        {
            OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( TARGET_URL ) );

            getTextProperty_Impl( aRegion, aPropName, maTargetURL );
        }
        else
        {
            DBG_ERRORFILE( "GetTargetURL(): Could not create hierarchy content!" );
        }
    }

    return maTargetURL;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
RegionData_Impl::RegionData_Impl( const SfxDocTemplate_Impl* pParent,
                                  const OUString& rTitle )
{
    maTitle     = rTitle;
    mpParent    = pParent;
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
long RegionData_Impl::GetEntryPos( const OUString& rTitle,
                                   sal_Bool& rFound ) const
{
#if 1   // Don't use binary search today
    ULONG i;
    ULONG nCount = maEntries.Count();

    for ( i=0; i<nCount; i++ )
    {
        EntryData_Impl *pData = maEntries.GetObject( i );

        if ( pData->Compare( rTitle ) == 0 )
        {
            rFound = sal_True;
            return i;
        }
    }

    rFound = sal_False;
    return i;

#else
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
#endif
}

// -----------------------------------------------------------------------
void RegionData_Impl::AddEntry( const OUString& rTitle,
                                const OUString& rTargetURL,
                                USHORT *pPos )
{
    INetURLObject aLinkObj( GetHierarchyURL() );
    aLinkObj.insertName( rTitle, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    OUString aLinkURL = aLinkObj.GetMainURL();

    EntryData_Impl *pEntry;
    sal_Bool        bFound = sal_False;
    long            nPos = GetEntryPos( rTitle, bFound );

    if ( bFound )
    {
        pEntry = maEntries.GetObject( nPos );
    }
    else
    {
        if ( pPos )
            nPos = *pPos;

        pEntry = new EntryData_Impl( this, rTitle );
        pEntry->SetTargetURL( rTargetURL );
        pEntry->SetHierarchyURL( aLinkURL );
        maEntries.Insert( pEntry, nPos );
    }
}

// -----------------------------------------------------------------------
ULONG RegionData_Impl::GetCount() const
{
    return maEntries.Count();
}

// -----------------------------------------------------------------------
const OUString& RegionData_Impl::GetHierarchyURL()
{
    if ( !maOwnURL.getLength() )
    {
        INetURLObject aRegionObj( GetParent()->GetRootURL() );

        aRegionObj.insertName( GetTitle(), false,
                     INetURLObject::LAST_SEGMENT, true,
                     INetURLObject::ENCODE_ALL );

        maOwnURL = aRegionObj.GetMainURL();
        DBG_ASSERT( maOwnURL.getLength(), "GetHierarchyURL(): Could not create URL!" );
    }

    return maOwnURL;
}

// -----------------------------------------------------------------------
const OUString& RegionData_Impl::GetTargetURL()
{
    if ( !maTargetURL.getLength() )
    {
        Reference< XCommandEnvironment > aCmdEnv;
        Content aRegion;

        if ( Content::create( GetHierarchyURL(), aCmdEnv, aRegion ) )
        {
            OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );

            getTextProperty_Impl( aRegion, aPropName, maTargetURL );
        }
        else
        {
            DBG_ERRORFILE( "GetTargetURL(): Could not create hierarchy content!" );
        }
    }

    return maTargetURL;
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
EntryData_Impl* RegionData_Impl::GetByTargetURL( const OUString& rName ) const
{
    EntryData_Impl *pEntry;

    ULONG nCount = maEntries.Count();

    for ( ULONG i=0; i<nCount; i++ )
    {
        pEntry = maEntries.GetObject( i );
        if ( pEntry && ( pEntry->GetTargetURL() == rName ) )
            return pEntry;
    }

    return NULL;
}

// -----------------------------------------------------------------------
EntryData_Impl* RegionData_Impl::GetEntry( ULONG nIndex ) const
{
    return maEntries.GetObject( nIndex );
}

// -----------------------------------------------------------------------
void RegionData_Impl::DeleteEntry( ULONG nIndex )
{
    EntryData_Impl *pEntry = maEntries.GetObject( nIndex );

    if ( pEntry )
    {
        delete pEntry;
        maEntries.Remove( (ULONG) nIndex );
    }
}

// -----------------------------------------------------------------------
int RegionData_Impl::Compare( RegionData_Impl* pCompare ) const
{
    int nCompare = maTitle.compareTo( pCompare->maTitle );

    return nCompare;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

SfxDocTemplate_Impl::SfxDocTemplate_Impl()
{
    mbConstructed = sal_False;
}

// -----------------------------------------------------------------------
SfxDocTemplate_Impl::~SfxDocTemplate_Impl()
{
    Clear();

    gpTemplateData = NULL;
}

// -----------------------------------------------------------------------
RegionData_Impl* SfxDocTemplate_Impl::GetRegion( ULONG nIndex ) const
{
    return maRegions.GetObject( nIndex );
}

// -----------------------------------------------------------------------
RegionData_Impl* SfxDocTemplate_Impl::GetRegion( const OUString& rName )
    const
{
    ULONG nCount = maRegions.Count();
    RegionData_Impl *pData;

    for ( ULONG i=0; i<nCount; i++ )
    {
        pData = maRegions.GetObject( i );

        if ( pData->GetTitle() == rName )
            return pData;
    }

    return NULL;
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::DeleteRegion( ULONG nIndex )
{
    RegionData_Impl* pRegion = maRegions.GetObject( nIndex );

    if ( pRegion )
    {
        delete pRegion;
        maRegions.Remove( (ULONG) nIndex );
    }
}

// -----------------------------------------------------------------------
/*  AddRegion adds a Region to the RegionList
*/
void SfxDocTemplate_Impl::AddRegion( const OUString& rTitle,
                                     Content& rContent )
{
    RegionData_Impl* pRegion;
    pRegion = new RegionData_Impl( this, rTitle );

    if ( ! InsertRegion( pRegion ) )
    {
        delete pRegion;
        return;
    }

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
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetDir( xRow->getString( 2 ) );

                pRegion->AddEntry( aTitle, aTargetDir );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch ( Exception& ) {}
    }
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::CreateFromHierarchy( Content &rTemplRoot )
{
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(1);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );

    try
    {
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        xResultSet = rTemplRoot.createCursor( aProps, eInclude );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "createCursor: CommandAbortedException" );
    }
    catch ( Exception& ) {}

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

                OUString aId = xContentAccess->queryContentIdentifierString();
                Content  aContent = Content( aId, aCmdEnv );

                AddRegion( aTitle, aContent );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch ( Exception& ) {}
    }
}

// ------------------------------------------------------------------------
sal_Bool SfxDocTemplate_Impl::Construct()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbConstructed )
        return sal_True;

    Reference< XMultiServiceFactory >   xFactory;
    xFactory = ::comphelper::getProcessServiceFactory();

    OUString aService( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_DOCINFO ) );
    Reference< XPersist > xInfo( xFactory->createInstance( aService ), UNO_QUERY );
    mxInfo = xInfo;

    aService = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_DOCTEMPLATES ) );
    Reference< XDocumentTemplates > xTemplates( xFactory->createInstance( aService ), UNO_QUERY );

    if ( xTemplates.is() )
        mxTemplates = xTemplates;
    else
        return sal_False;

    AllSettings aSettings;
    Locale      aLocale = aSettings.GetLocale();

    Reference< XLocalizable > xLocalizable( xTemplates, UNO_QUERY );

    xLocalizable->setLocale( aLocale );

    Reference < XContent > aRootContent = xTemplates->getContent();
    Reference < XCommandEnvironment > aCmdEnv;

    if ( ! aRootContent.is() )
        return sal_False;

    mbConstructed = sal_True;
    maRootURL = aRootContent->getIdentifier()->getContentIdentifier();

    ResStringArray  aLongNames( SfxResId( TEMPLATE_LONG_NAMES_ARY ) );

    if ( aLongNames.Count() )
        maStandardGroup = aLongNames.GetString( 0 );

    Content aTemplRoot( aRootContent, aCmdEnv );
    CreateFromHierarchy( aTemplRoot );

    return sal_True;
}


// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::GetTemplates( Content& rTargetFolder,
                                        Content& rParentFolder,
                                        RegionData_Impl* pRegion )
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
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString(1) );

                if ( aTitle.compareToAscii( "sfx.tlx" ) == 0 )
                    continue;

                OUString aId = xContentAccess->queryContentIdentifierString();

                EntryData_Impl* pEntry = pRegion->GetByTargetURL( aId );

                if ( ! pEntry )
                {
                    OUString aFullTitle;
                    GetTitleFromURL( aId, aFullTitle );

                    if ( aFullTitle.len() )
                        aTitle = aFullTitle;

                    pRegion->AddEntry( aTitle, aId );
                }
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch ( Exception& ) {}
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
sal_Bool SfxDocTemplate_Impl::InsertRegion( RegionData_Impl *pNew,
                                            ULONG nPos )
{
    ::osl::MutexGuard   aGuard( maMutex );
    RegionData_Impl    *pData = maRegions.First();

    while ( pData && ( pData->Compare( pNew ) != 0 ) )
        pData = maRegions.Next();

    if ( ! pData )
    {
        // compare with the name of the standard group here to insert it
        // first

        if ( pNew->GetTitle() == maStandardGroup )
            maRegions.Insert( pNew, (ULONG) 0 );
        else
            maRegions.Insert( pNew, nPos );
    }

    return ( pData == NULL );
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::Rescan()
{
    Reference< XDocumentTemplates > xTemplates = getDocTemplates();
    xTemplates->update();

    Clear();

    Reference < XContent > aRootContent = xTemplates->getContent();
    Reference < XCommandEnvironment > aCmdEnv;

    Content aTemplRoot( aRootContent, aCmdEnv );
    CreateFromHierarchy( aTemplRoot );
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::GetTitleFromURL( const OUString& rURL,
                                           OUString& aTitle )
{
    if ( mxInfo.is() )
    {
        try
        {
            mxInfo->read( rURL );

            Reference< XPropertySet > aPropSet( mxInfo, UNO_QUERY );
            if ( aPropSet.is() )
            {
                OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
                Any aValue = aPropSet->getPropertyValue( aPropName );
                aValue >>= aTitle;
            }
        }
        catch ( IOException& ) {}
        catch ( UnknownPropertyException& ) {}
        catch ( Exception& ) {}
    }

    if ( ! aTitle.len() )
    {
        INetURLObject aURL( rURL );
        aURL.CutExtension();
        aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true,
                               INetURLObject::DECODE_WITH_CHARSET );
    }
}


// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::Clear()
{
    RegionData_Impl *pRegData = maRegions.First();

    while ( pRegData )
    {
        delete pRegData;
        pRegData = maRegions.Next();
    }

    maRegions.Clear();
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
sal_Bool getTextProperty_Impl( Content& rContent,
                               const OUString& rPropName,
                               OUString& rPropValue )
{
    sal_Bool bGotProperty = sal_False;

    // Get the property
    try
    {
        Reference< XPropertySetInfo > aPropInfo = rContent.getProperties();

        // check, wether or not the property exists
        if ( !aPropInfo.is() || !aPropInfo->hasPropertyByName( rPropName ) )
        {
            return sal_False;
        }

        // now get the property
        Any aAnyValue;

        aAnyValue = rContent.getPropertyValue( rPropName );
        aAnyValue >>= rPropValue;

        bGotProperty = sal_True;
    }
    catch ( CommandAbortedException& ) {}
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    return bGotProperty;
}

