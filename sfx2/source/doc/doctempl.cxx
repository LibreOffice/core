/*************************************************************************
 *
 *  $RCSfile: doctempl.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: dv $ $Date: 2001-02-16 12:10:55 $
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
#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif
#ifndef _SV_RESARY_HXX
#include <vcl/resary.hxx>
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

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSIST_HPP_
#include <com/sun/star/io/XPersist.hpp>
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

#define DONT_USE_HIERARCHY

#define HIERARCHIE_ROOT_URL     "vnd.sun.star.hier:/"
#define TEMPLATE_ROOT_URL       "vnd.sun.star.hier:/templates"
#define TEMPLATE_DIR_NAME       "templates"
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

#define C_DELIM                 ';'

class WaitWindow_Impl : public WorkWindow
{
    Rectangle   _aRect;
    USHORT      _nTextStyle;
    String      _aText;

    public:
                     WaitWindow_Impl();
                    ~WaitWindow_Impl();
    virtual void     Paint( const Rectangle& rRect );
};

#define X_OFFSET 15
#define Y_OFFSET 15

//========================================================================

class EntryData_Impl
{
    SfxObjectShellLock  mxObjShell;
    SvStorageRef        mxStor;
    OUString            maTitle;
    OUString            maOwnURL;
    OUString            maTargetURL;
    sal_Bool            mbInUse     : 1;
    sal_Bool            mbNew       : 1;
    sal_Bool            mbIsOwner   : 1;
    sal_Bool            mbDidConvert: 1;

public:
                        EntryData_Impl( const OUString& rTitle );

    const OUString&     GetTitle() const { return maTitle; }
    const OUString&     GetTargetURL() const { return maTargetURL; }
    const OUString&     GetHierarchyURL() const { return maOwnURL; }

    void                SetTitle( const OUString& rTitle ) { maTitle = rTitle; }
    void                SetTargetURL( const OUString& rTargetURL );
    void                SetHierarchyURL( const OUString& rURL) { maOwnURL = rURL; }

    int                 Compare( const OUString& rTitle ) const;
    void                SetType( const OUString& rType );

    void                ResetFlags() { mbInUse = sal_False; mbNew = sal_False; }
    void                RemoveEntry();
    void                SetInUse( sal_Bool bInUse ) { mbInUse = bInUse; }
    sal_Bool            IsInUse() const { return mbInUse; }
    sal_Bool            IsNew() const { return mbNew; }


    SfxObjectShellRef   CreateObjectShell();
    BOOL                DeleteObjectShell();
};

DECLARE_LIST( EntryList_Impl, EntryData_Impl* );

// ------------------------------------------------------------------------

class RegionData_Impl
{
    EntryList_Impl      maEntries;
    OUString            maTitle;
    OUString            maOwnURL;
    OUString            maTargetURL;
    Content             maTargetContent;
    Content             maHierarchyContent;
    sal_Bool            mbInUse     : 1;
    sal_Bool            mbNew       : 1;

private:
    long                GetEntryPos( const OUString& rTitle,
                                     sal_Bool& rFound ) const;

public:
                        RegionData_Impl( const OUString& rTitle );
                        ~RegionData_Impl();

    EntryData_Impl*     GetEntry( ULONG nIndex ) const;
    EntryData_Impl*     GetEntry( const OUString& rName ) const;
    EntryData_Impl*     GetByTargetURL( const OUString& rName ) const;

    const OUString&     GetTitle() const { return maTitle; }
    const OUString&     GetTargetURL() const { return maTargetURL; }
    const OUString&     GetHierarchyURL() const { return maOwnURL; }

    ULONG               GetCount() const;

    void                SetTitle( const OUString& rTitle ) { maTitle = rTitle; }
    void                SetTargetURL( const OUString& rTargetURL );
    void                SetHierarchyURL( const OUString& rURL) { maOwnURL = rURL; }

    void                SetTargetContent( const Content& rTarget ) { maTargetContent = rTarget; }
    void                SetHierarchyContent( const Content& rContent ) { maHierarchyContent = rContent; }

    EntryData_Impl*     AddEntry( Content& rParentFolder,
                                  const OUString& rTitle,
                                  const OUString& rTargetURL,
                                  USHORT *pPos = NULL );
    void                DeleteEntry( ULONG nIndex );

    int                 Compare( const OUString& rTitle ) const
                            { return maTitle.compareTo( rTitle ); }
    int                 Compare( RegionData_Impl* pCompareWith ) const;

    void                ResetFlags();
    void                RemoveUnused();
    void                RemoveRegion();
    void                SetInUse( sal_Bool bInUse ) { mbInUse = bInUse; }
    sal_Bool            IsInUse() const { return mbInUse; }
};

DECLARE_LIST( RegionList_Impl, RegionData_Impl* );

struct NamePair_Impl
{
    OUString maShortName;
    OUString maLongName;
};

DECLARE_LIST( NameList_Impl, NamePair_Impl* );

// ------------------------------------------------------------------------
class SfxDocTemplate_Impl : public SvRefBase
{
    Reference< XMultiServiceFactory >   mxFactory;
    Reference< XPersist >               mxInfo;
    ::osl::Mutex        maMutex;
    String              maDirs;
    RegionList_Impl     maRegions;
    NameList_Impl       maNames;
    sal_Bool            mbConstructed   : 1;
    sal_Bool            mbUpdating      : 1;

private:
    sal_Bool            InsertOrMarkRegion( RegionData_Impl *pData );
    void                ReadFolderList();
    OUString            GetLongName( const OUString& rShortName );

public:
                        SfxDocTemplate_Impl();
                        ~SfxDocTemplate_Impl();

    void                Construct( const String& rDirURLs );
    sal_Bool            Rescan( sal_Bool bNow );
    void                CreateFromHierarchy( Content &rTemplRoot );

    void                AddRegion( const OUString& rTitle,
                                   const OUString& rTargetURL,
                                   Content& rContent );
    void                DeleteRegion( ULONG nIndex );

    ULONG               GetRegionCount() const
                            { return maRegions.Count(); }
    RegionData_Impl*    GetRegion( const OUString& rName ) const;
    RegionData_Impl*    GetRegion( ULONG nIndex ) const;
    RegionData_Impl*    GetRegionByPath( const String& rPath ) const;
    void                GetFolders( Content& rRoot, Content& rFolder );
    void                GetTemplates( Content& rTargetFolder,
                                      Content& rParentFolder,
                                      RegionData_Impl* pRegion );

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

    void                GetTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType );

    void                AddToStandard( Content& rRoot,
                                       Content& rFolder );
    sal_Bool            DoUpdate();
};

SfxDocTemplate_Impl *gpTemplateData = 0;
String  gaEmptyString;

#ifndef SFX_DECL_DOCTEMPLATES_DEFINED
#define SFX_DECL_DOCTEMPLATES_DEFINED
SV_DECL_REF(SfxDocTemplate_Impl)
#endif

SV_IMPL_REF(SfxDocTemplate_Impl)


//------------------------------------------------------------------------
class Updater_Impl : public ::vos::OThread
{
private:
    SfxDocTemplate_ImplRef  maDocTemplates;

public:
                            Updater_Impl( SfxDocTemplate_Impl* pTemplates );
                            ~Updater_Impl();

    virtual void SAL_CALL   run();
    virtual void SAL_CALL   onTerminated();
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
    pImp->Construct( aDirs );

    // First: find the RegionData for the index
    String aName;

    RegionData_Impl *pData1 = pImp->GetRegion( nIdx );

    if ( pData1 )
        aName = pData1->GetTitle();

    /*  Second: check if there is another region with the same name.
        If there is one, append the path to the region name so it is
        distinguishable from the other ones
    */

    OUString         aCompareMe( aName );
    ULONG            nCount = pImp->GetRegionCount();
    ULONG            nIndex = 0;
    RegionData_Impl *pData2;

    for( nIndex = 0; nIndex < nCount; nIndex++ )
    {
        pData2 = pImp->GetRegion( nIndex );

        if ( ( nIndex != nIdx ) &&
             ( pData2->GetTitle() == aCompareMe ) )
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

    pImp->Construct( aDirs );

    RegionData_Impl *pData = pImp->GetRegion( nIdx );

    if ( pData )
        maTmpString = pData->GetTitle();
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
    pImp->Construct( aDirs );

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
    pImp->Construct( aDirs );

    ULONG nCount = pImp->GetRegionCount();

    return (USHORT) nCount;
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
    pImp->Construct( aDirs );

    RegionData_Impl *pData = pImp->GetRegion( rName );
    ULONG            nCount = 0;

    if ( pData )
        nCount = pData->GetCount();

    return (USHORT) nCount;
}

//------------------------------------------------------------------------

BOOL SfxDocumentTemplates::IsRegionLoaded( USHORT nIdx ) const
{
    pImp->Construct( aDirs );

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
    pImp->Construct( aDirs );

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
    pImp->Construct( aDirs );

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
    pImp->Construct( aDirs );

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
    pImp->Construct( aDirs );

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
    pImp->Construct( aDirs );

    EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

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
    pImp->Construct( aDirs );

    DBG_ASSERT( aDirs.GetTokenCount( cDelim ), "Keine Bereiche" );

    USHORT  nCount = aDirs.GetTokenCount(cDelim);
    String           aPath( aDirs.GetToken( nCount-1, cDelim ) );
    EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( aPath );

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
    pImp->Construct( aDirs );

    EntryData_Impl  *pEntry;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    // do nothing if there is no region with that index
    if ( !pRegion )
        return;

    pEntry = pRegion->GetEntry( rLongName );

    // do nothing if there is already an entry with that name
    if ( pEntry )
        return;

    Content aContent;

    pEntry = pRegion->AddEntry( aContent, rLongName, rFileName );

    if ( pEntry )
    {
        OUString aType;
        OUString aName;

        pImp->GetTitleFromURL( rFileName, aName, aType );
        pEntry->SetType( aType );
    }
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

    pImp->Construct( aDirs );

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

        TransferInfo aTransferInfo;
        aTransferInfo.MoveData = bMove;
        aTransferInfo.SourceURL = pSource->GetTargetURL();
        aTransferInfo.NewTitle = aTitle;
        aTransferInfo.NameClash = NameClash::OVERWRITE;

        Any aArg = makeAny( aTransferInfo );
        OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( COMMAND_TRANSFER ) );

        aTarget.executeCommand( aCmd, aArg );

#ifndef DONT_USE_HIERARCHY
        aHierTarget = Content( pTargetRgn->GetHierarchyURL(), aCmdEnv );
        aTransferInfo.SourceURL = pSource->GetHierarchyURL();
        aArg = makeAny( aTransferInfo );
        aHierTarget.executeCommand( aCmd, aArg );
#endif
    }
    catch ( ContentCreationException& )
    { return FALSE; }
    catch ( CommandAbortedException& )
    { return FALSE; }
    catch ( Exception& )
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
    pImp->Construct( aDirs );

    RegionData_Impl *pSourceRgn = pImp->GetRegion( nRegion );
    if ( !pSourceRgn )
        return FALSE;

    EntryData_Impl *pSource = pSourceRgn->GetEntry( nIdx );
    if ( !pSource )
        return FALSE;

    INetURLObject aTargetURL( rName );

    OUString aTitle( aTargetURL.GetName() );
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
    pImp->Construct( aDirs );

    RegionData_Impl *pTargetRgn = pImp->GetRegion( nRegion );

    if ( !pTargetRgn )
        return FALSE;

    INetURLObject aURL( rName );
    OUString aTitle( aURL.GetName() );

    Reference< XCommandEnvironment > aCmdEnv;
    Content aTarget;

    try
    {
        aTarget = Content( pTargetRgn->GetTargetURL(), aCmdEnv );

        TransferInfo aTransferInfo;
        aTransferInfo.MoveData = FALSE;
        aTransferInfo.SourceURL = rName;
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

    // update data structures ...
    INetURLObject aTmp( pTargetRgn->GetTargetURL() );
    aTmp.insertName( aTitle, false,
                     INetURLObject::LAST_SEGMENT, true,
                     INetURLObject::ENCODE_ALL );

    try
    {
        if ( nIdx == USHRT_MAX )
            nIdx = 0;
        else
            nIdx += 1;

        OUString aType;
        EntryData_Impl *pEntry;

        pImp->GetTitleFromURL( rName, aTitle, aType );
#ifndef DONT_USE_HIERARCHY
        aTarget = Content( pTargetRgn->GetHierarchyURL(), aCmdEnv );
#endif
        pEntry = pTargetRgn->AddEntry( aTarget, aTitle,
                                       aTmp.GetMainURL(),
                                       &nIdx );
        if ( pEntry )
        {
            pEntry->SetType( aType );
        }

        return TRUE;
    }
    catch ( ContentCreationException& ) {}
    catch ( CommandAbortedException& ) {}
    catch ( Exception& ) {}

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
    pImp->Construct( aDirs );

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
#ifndef DONT_USE_HIERARCHY
        aHierCont = Content( aHierURL, aCmdEnv );
#endif
        aFolderCont = Content( aTargetURL, aCmdEnv );
    }
    catch ( ContentCreationException& )
    {
        DBG_ERRORFILE( "Template or Region doesn't exist!" );
        return bRet;
    }
    catch ( Exception& )
    { return bRet; }

    try
    {
        OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( COMMAND_DELETE ) );
        Any aArg = makeAny( sal_Bool( sal_True ) );

#ifndef DONT_USE_HIERARCHY
        aHierCont.executeCommand( aCmd, aArg );
#endif
        aFolderCont.executeCommand( aCmd, aArg );

        bRet = sal_True;
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "Template or Region doesn't exist!" );
        return bRet;
    }
    catch ( Exception& )
    { return bRet; }

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

    pImp->Construct( aDirs );

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

    pImp->Construct( aDirs );

    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );
    EntryData_Impl *pEntry = NULL;

    if ( !pRegion )
        return FALSE;

    Content aHierCont;
    Content aFolderCont;

    OUString aTitle( rName );
    OUString aFolderTitle;
    OUString aFolderURL;
    OUString aHierURL;
    OUString aTargetStr;
    OUString aTitleStr( OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) ) );

    if ( nIdx == USHRT_MAX )
    {
        aFolderURL = pRegion->GetTargetURL();
        aHierURL = pRegion->GetHierarchyURL();
        aFolderTitle = aTitle;
        aTargetStr = OUString( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
    }
    else
    {
        pEntry = pRegion->GetEntry( nIdx );

        if ( !pEntry )
            return FALSE;

        aFolderURL = pEntry->GetTargetURL();
        aHierURL = pEntry->GetHierarchyURL();

        if ( nIdx != USHRT_MAX )
        {
            INetURLObject aURL( aFolderURL );
            aURL.setName( aTitle );

            OUString aExtension = aURL.getExtension();

            if ( ! aExtension.len() )
                aURL.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM( "vor" ) ) );

            aFolderTitle = aURL.getName();
        }

        aTargetStr = OUString( RTL_CONSTASCII_USTRINGPARAM( TARGET_URL ) );
    }

    Reference< XCommandEnvironment > aCmdEnv;

    // Create a folder Content with the old title and
    // rename it
    try
    {
        aFolderCont = Content( aFolderURL, aCmdEnv );
        aFolderCont.setPropertyValue( aTitleStr, makeAny( aFolderTitle ) );
        aFolderURL = aFolderCont.get()->getIdentifier()->getContentIdentifier();
    }
    catch( ContentCreationException& )
    {
        DBG_ERRORFILE( "Folder object doesn't exist?" );
        return FALSE;
    }
    catch ( Exception& )
    { return FALSE; }

#ifndef DONT_USE_HIERARCHY

    // Create a hierarchy Content with the old title and
    // rename it, then set the new target URL
    try
    {
        aHierCont = Content( aHierURL, aCmdEnv );
        aHierCont.setPropertyValue( aTitleStr, makeAny( aTitle ) );
        aHierCont.setPropertyValue( aTargetStr, makeAny( aFolderURL ) );
        aHierURL = aHierCont.get()->getIdentifier()->getContentIdentifier();
    }
    catch( ContentCreationException& )
    {
        DBG_ERRORFILE( "Hierarchy object doesn't exist?" );
        return FALSE;
    }
    catch ( Exception& )
    { return FALSE; }
#endif

    // Update the internal data structures
    if ( pEntry )
    {

        pEntry->SetTitle( aTitle );
        pEntry->SetTargetURL( aFolderURL );
        pEntry->SetHierarchyURL( aHierURL );
    }
    else
    {
        pRegion->SetTitle( aTitle );
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
    pImp->Construct( aDirs );

    return pImp->Rescan( sal_True );

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
    pImp->Construct( aDirs );

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
    pImp->Construct( aDirs );

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
    pImp->Construct( aDirs );

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
        if( !rRegion.Len() || ( aName == rRegion ) )
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
    pImp->Construct( aDirs );

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
    aDirs = SvtPathOptions().GetTemplatePath();
    cDelim = ';'; // absichtlich hart verdrahtet

    if ( !gpTemplateData )
        gpTemplateData = new SfxDocTemplate_Impl;

    pImp = gpTemplateData;
}

//-------------------------------------------------------------------------

void SfxDocumentTemplates::Construct()

//  verz"ogerter Aufbau der Verwaltungsdaten

{
//  pImp->Construct( aDirs );
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
EntryData_Impl::EntryData_Impl( const OUString& rTitle )
{
    maTitle     = rTitle;
    mbInUse     = sal_True;
    mbNew       = sal_True;
    mbIsOwner   = sal_False;
    mbDidConvert= sal_False;
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
void EntryData_Impl::SetType( const OUString& rType )
{
#ifndef DONT_USE_HIERARCHY

    if ( ! rType.len() )
        return;

    Content aContent;
    try
    {
        Reference< XCommandEnvironment > aCmdEnv;
        aContent = Content( maOwnURL, aCmdEnv );

        Reference< XPropertySetInfo > xPropSet = aContent.getProperties();
        if ( xPropSet.is() )
        {
            OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( PROPERTY_TYPE ) );

            if ( ! xPropSet->hasPropertyByName( aPropName ) )
            {
                Reference< XPropertyContainer > xProperties( aContent.get(), UNO_QUERY );
                if ( xProperties.is() )
                {
                    try
                    {
                        xProperties->addProperty( aPropName, PropertyAttribute::MAYBEVOID,
                                                  makeAny( rType ) );
                    }
                    catch( PropertyExistException& ) {}
                    catch( IllegalTypeException& ) { DBG_ERRORFILE( "IllegalTypeException" ); }
                    catch( IllegalArgumentException& ) { DBG_ERRORFILE( "IllegalArgumentException" ); }
                }
            }

            aContent.setPropertyValue( aPropName, makeAny( rType ) );
        }
    }
    catch ( CommandAbortedException& ) {}
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}
#endif
}

// -----------------------------------------------------------------------
void EntryData_Impl::RemoveEntry()
{
    Content aEntry;

    try
    {
        Reference< XCommandEnvironment > aCmdEnv;
        aEntry = Content( maOwnURL, aCmdEnv );

        OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( COMMAND_DELETE ) );
        Any aArg = makeAny( sal_Bool( sal_True ) );

        aEntry.executeCommand( aCmd, aArg );
    }
    catch ( CommandAbortedException& ) {}
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}
}

//------------------------------------------------------------------------
SfxObjectShellRef EntryData_Impl::CreateObjectShell()
{
    if( ! mxObjShell.Is() )
    {
        mbIsOwner = FALSE;
        BOOL bDum = FALSE;
        SfxApplication *pSfxApp = SFX_APP();
        mxObjShell = pSfxApp->DocAlreadyLoaded( maTargetURL, TRUE, bDum );

        if( ! mxObjShell.Is() )
        {
            mbIsOwner = TRUE;
            SfxMedium *pMed=new SfxMedium(
                maTargetURL,(STREAM_READ | STREAM_SHARE_DENYWRITE), FALSE, 0 );
            const SfxFilter* pFilter = NULL;
            if( pSfxApp->GetFilterMatcher().GuessFilter(
                *pMed, &pFilter, SFX_FILTER_TEMPLATE, 0 ) ||
                pFilter && !pFilter->IsOwnFormat() ||
                pFilter && !pFilter->UsesStorage() )
            {
                SfxErrorContext aEc( ERRCTX_SFX_LOADTEMPLATE,
                                     maTargetURL );
                delete pMed;
                mbDidConvert=TRUE;
                ULONG lErr;
                if ( mxObjShell.Is() )
                    if(lErr=(pSfxApp->LoadTemplate(
                        mxObjShell,maTargetURL)!=ERRCODE_NONE))
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
                    maTargetURL,
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
                        maTargetURL,
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
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
RegionData_Impl::RegionData_Impl( const OUString& rTitle )
{
    maTitle     = rTitle;
    mbInUse     = sal_True;
    mbNew       = sal_True;
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
        catch( ContentCreationException& ) {}
        catch ( Exception& ) {}
    }

    maTargetURL = rTargetURL;

    if ( ! maTitle.getLength() )
    {
        INetURLObject aURLObject( rTargetURL );
        maTitle = aURLObject.getName();
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
EntryData_Impl* RegionData_Impl::AddEntry( Content& rParentFolder,
                                           const OUString& rTitle,
                                           const OUString& rTargetURL,
                                           USHORT *pPos )
{
    INetURLObject aLinkObj( GetHierarchyURL() );
    aLinkObj.insertName( rTitle, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    OUString aLinkURL = aLinkObj.GetMainURL();

#ifndef DONT_USE_HIERARCHY

    Content aLink;
    Reference< XCommandEnvironment > aCmdEnv;

    if ( ! Content::create( aLinkURL, aCmdEnv, aLink ) )
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
                return NULL;
            }
        }

        try
        {
            rParentFolder.insertNewContent( aType, aNames, aValues, aLink );
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "CommandAbortedException" );
            return NULL;
        }
    }
#endif

    EntryData_Impl *pEntry;
    sal_Bool        bFound = sal_False;
    long            nPos = GetEntryPos( rTitle, bFound );

    if ( bFound )
    {
        pEntry = maEntries.GetObject( nPos );
        pEntry->SetInUse( sal_True );
    }
    else
    {
        if ( pPos )
            nPos = *pPos;

        pEntry = new EntryData_Impl( rTitle );
        pEntry->SetTargetURL( rTargetURL );
        pEntry->SetHierarchyURL( aLinkURL );
        maEntries.Insert( pEntry, nPos );
    }

    return pEntry;
}

// -----------------------------------------------------------------------
ULONG RegionData_Impl::GetCount() const
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
void RegionData_Impl::ResetFlags()
{
    mbInUse = sal_False;
    mbNew   = sal_False;

    EntryData_Impl *pData = maEntries.First();
    while ( pData )
    {
        pData->ResetFlags();
        pData = maEntries.Next();
    }
}

// -----------------------------------------------------------------------
void RegionData_Impl::RemoveUnused()
{
    ULONG           nCount = maEntries.Count();
    EntryData_Impl *pData;

    while ( nCount )
    {
        pData = maEntries.GetObject( --nCount );
        if ( !pData->IsInUse() )
        {
            pData->RemoveEntry();
            maEntries.Remove( nCount );
            delete pData;
        }
    }
}

// -----------------------------------------------------------------------
void RegionData_Impl::RemoveRegion()
{
    Content aRegion;

    try
    {
        Reference< XCommandEnvironment > aCmdEnv;
        aRegion = Content( maOwnURL, aCmdEnv );

        OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( COMMAND_DELETE ) );
        Any aArg = makeAny( sal_Bool( sal_True ) );

        aRegion.executeCommand( aCmd, aArg );
    }
    catch ( CommandAbortedException& ) {}
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

SfxDocTemplate_Impl::SfxDocTemplate_Impl()
{
    mbConstructed = sal_False;
    mbUpdating = sal_False;
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

    gpTemplateData = NULL;
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
RegionData_Impl* SfxDocTemplate_Impl::GetRegionByPath(
                                            const String& rName ) const
{
    ULONG            nCount = maRegions.Count();
    OUString         aCompare( rName );
    RegionData_Impl *pData;

    for ( ULONG i=0; i<nCount; i++ )
    {
        pData = maRegions.GetObject( i );

        if ( pData->GetTargetURL() == aCompare )
            return pData;
    }

    return NULL;
}

// -----------------------------------------------------------------------
RegionData_Impl* SfxDocTemplate_Impl::GetRegion( ULONG nIndex ) const
{
    return maRegions.GetObject( nIndex );
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
                                     const OUString& rTargetURL,
                                     Content& rContent )
{
    RegionData_Impl* pRegion;
    pRegion = new RegionData_Impl( rTitle );
    pRegion->SetHierarchyContent( rContent );
    pRegion->SetTargetURL( rTargetURL );
#ifndef DONT_USE_HIERARCHY
    pRegion->SetHierarchyURL( rContent.get()->getIdentifier()->getContentIdentifier() );
#endif

    if ( InsertOrMarkRegion( pRegion ) )
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
        catch ( Exception& ) {}
    }
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::CreateFromHierarchy( Content &rTemplRoot )
{
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(2);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );
    pProps[1] = OUString::createFromAscii( TARGET_DIR_URL );

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
                OUString aTargetDir( xRow->getString( 2 ) );

                OUString aId = xContentAccess->queryContentIdentifierString();
                Content  aContent = Content( aId, aCmdEnv );

                AddRegion( aTitle, aTargetDir, aContent );
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
void SfxDocTemplate_Impl::Construct( const String& rDirs )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbConstructed )
        return;

    mxFactory = ::comphelper::getProcessServiceFactory();

    OUString aService( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_DOCINFO ) );
    Reference< XPersist > xInfo( mxFactory->createInstance( aService ), UNO_QUERY );
    mxInfo = xInfo;

    Content     aTemplRoot;
    sal_Bool    bNewRoot;

    maDirs = rDirs;

    if ( ! GetTemplateRoot( aTemplRoot, bNewRoot ) )
    {
        DBG_ERRORFILE( "CreateTemplateHier(): Could not find/create template root" );
        return;
    }

    mbConstructed = sal_True;

    ReadFolderList();

    if ( bNewRoot )
    {
#ifndef DONT_USE_HIERARCHY
        WaitWindow_Impl aWindow;
#endif
        Rescan( sal_True );
    }
    else
    {
        CreateFromHierarchy( aTemplRoot );
        Rescan( sal_False );
    }
}

// -----------------------------------------------------------------------

sal_Bool SfxDocTemplate_Impl::GetTemplateRoot( Content &rTemplRoot,
                                               sal_Bool &rNew ) const
{
#ifndef DONT_USE_HIERARCHY
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
    }
    catch ( Exception& ) {}

    return bRet;
#else
    rNew = sal_True;
    return sal_True;
#endif
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
    catch ( Exception& )
    { return FALSE; }

    return bRet;
}

// -----------------------------------------------------------------------

void SfxDocTemplate_Impl::GetFolders( Content& rRoot,
                                      Content& rFolder )
{
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(1);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );

    AddToStandard( rRoot, rFolder );

    try
    {
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        xResultSet = rFolder.createCursor( aProps, eInclude );
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

        OUString aFolderURL = rFolder.get()->getIdentifier()->getContentIdentifier();

#ifndef DONT_USE_HIERARCHY
        OUString aRootURL = rRoot.get()->getIdentifier()->getContentIdentifier();
        aRootURL += OUString( '/' );
#endif

        Content aFolder;
        Sequence< OUString > aNames(2);
        OUString* pNames = aNames.getArray();
        pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
        pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

        OUString aAdditionalProp( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );

        Sequence< Any > aValues(2);
        Any* pValues = aValues.getArray();

        OUString aType = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPE_FOLDER ) );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString(1) );
                OUString aId = xContentAccess->queryContentIdentifierString();

                if ( aTitle.compareToAscii( "wizard" ) == 0 )
                    continue;
                else if ( aTitle.compareToAscii( "internal" ) == 0 )
                    continue;

                aTitle = GetLongName( aTitle );

#ifndef DONT_USE_HIERARCHY

                INetURLObject aNewFolderObj( aRootURL );
                aNewFolderObj.insertName( aTitle, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );

                OUString aNewFolderURL = aNewFolderObj.GetMainURL();

                if ( ! Content::create( aNewFolderURL, aCmdEnv, aFolder ) )
                {
                    pValues[0] = makeAny( aTitle );
                    pValues[1] = makeAny( sal_Bool( sal_True ) );

                    try
                    {
                        rRoot.insertNewContent( aType, aNames, aValues, aFolder );
                        Reference< XPropertySetInfo > xPropSet = aFolder.getProperties();
                        if ( xPropSet.is() )
                        {
                            if ( ! xPropSet->hasPropertyByName( aAdditionalProp ) )
                            {
                                Reference< XPropertyContainer > xFolderProp( aFolder.get(), UNO_QUERY );
                                if ( xFolderProp.is() )
                                {
                                    try
                                    {
                                        xFolderProp->addProperty( aAdditionalProp,
                                                                  PropertyAttribute::MAYBEVOID,
                                                                  makeAny( aId ) );
                                    }
                                    catch( PropertyExistException& ) {}
                                    catch( IllegalTypeException& ) { DBG_ERRORFILE( "IllegalTypeException" ); }
                                    catch( IllegalArgumentException& ) { DBG_ERRORFILE( "IllegalArgumentException" ); }
                                }
                            }

                            aFolder.setPropertyValue( aAdditionalProp, makeAny( aId ) );
                        }
                    }
                    catch( CommandAbortedException& )
                    {
                        DBG_ERRORFILE( "CommandAbortedException" );
                    }
                }

#endif

                RegionData_Impl *pRegion = GetRegion( aTitle );

                if ( !pRegion )
                {
                    pRegion = new RegionData_Impl( aTitle );
                    pRegion->SetTargetURL( aId );
#ifndef DONT_USE_HIERARCHY
                    pRegion->SetHierarchyURL( aNewFolderURL );
#endif
                }

                InsertOrMarkRegion( pRegion );

                Content aSubFolder( xContentAccess->queryContent(), aCmdEnv );
                GetTemplates( aSubFolder, aFolder, pRegion );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "GetFolders::next(): CommandAbortedException" );
        }
        catch ( Exception& ) {}
    }
}

// -----------------------------------------------------------------------

void SfxDocTemplate_Impl::AddToStandard( Content& rRoot,
                                         Content& rFolder )
{
    OUString aNewFolderURL;
    OUString aTitle = GetLongName( OUString( RTL_CONSTASCII_USTRINGPARAM( STANDARD_FOLDER ) ) );
    OUString aFolderURL = rFolder.get()->getIdentifier()->getContentIdentifier();
    Content  aFolder;

#ifndef DONT_USE_HIERARCHY

    OUString aRootURL = rRoot.get()->getIdentifier()->getContentIdentifier();

    INetURLObject aNewFolderObj( aRootURL );
    aNewFolderObj.insertName( aTitle, false,
          INetURLObject::LAST_SEGMENT, true,
          INetURLObject::ENCODE_ALL );

    aNewFolderURL = aNewFolderObj.GetMainURL();

    Reference< XCommandEnvironment > aCmdEnv;

    if ( ! Content::create( aNewFolderURL, aCmdEnv, aFolder ) )
    {
        OUString aType = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPE_FOLDER ) );

        Sequence< OUString > aNames(2);
        OUString* pNames = aNames.getArray();
        pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
        pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

        Sequence< Any > aValues(2);
        Any* pValues = aValues.getArray();

        pValues[0] = makeAny( aTitle );
        pValues[1] = makeAny( sal_Bool( sal_True ) );

        try
        {
            rRoot.insertNewContent( aType, aNames, aValues, aFolder );
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "CommandAbortedException" );
            return;
        }
        catch( Exception& ) { return; }
    }

#endif

    // Always set the target URL, because the last one should win!

    OUString aAdditionalProp( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );

    try
    {
        Reference< XPropertySetInfo > xPropSet = aFolder.getProperties();
        if ( xPropSet.is() )
        {
            if ( ! xPropSet->hasPropertyByName( aAdditionalProp ) )
            {
                Reference< XPropertyContainer > xFolderProp( aFolder.get(), UNO_QUERY );
                if ( xFolderProp.is() )
                {
                    try
                    {
                        xFolderProp->addProperty( aAdditionalProp,
                                                  PropertyAttribute::MAYBEVOID,
                                                  makeAny( aFolderURL ) );
                    }
                    catch( PropertyExistException& ) {}
                    catch( IllegalTypeException& ) { DBG_ERRORFILE( "IllegalTypeException" ); }
                    catch( IllegalArgumentException& ) { DBG_ERRORFILE( "IllegalArgumentException" ); }
                }
            }
            aFolder.setPropertyValue( aAdditionalProp, makeAny( aFolderURL ) );
        }
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( Exception& ) {}

    RegionData_Impl *pRegion = GetRegion( aTitle );

    if ( !pRegion )
    {
        pRegion = new RegionData_Impl( aTitle );
        pRegion->SetHierarchyURL( aNewFolderURL );
        InsertOrMarkRegion( pRegion );
    }
    else
    {
        pRegion->SetInUse( sal_True );
    }

    OUString aOldTarget = pRegion->GetTargetURL();

    if ( !aOldTarget.len() )
        pRegion->SetTargetURL( aFolderURL );

    GetTemplates( rFolder, aFolder, pRegion );
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

                if ( pEntry )
                    pEntry->SetInUse( sal_True );
                else
                {
                    OUString aFullTitle;
                    OUString aType;
                    GetTitleFromURL( aId, aFullTitle, aType );

                    if ( aFullTitle.len() )
                        aTitle = aFullTitle;

                    pEntry = pRegion->AddEntry( rParentFolder, aTitle, aId );

                    if ( pEntry && pEntry->IsNew() )
                    {
                        pEntry->SetType( aType );
                    }
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

sal_Bool SfxDocTemplate_Impl::InsertOrMarkRegion( RegionData_Impl *pNew )
{
    ::osl::MutexGuard   aGuard( maMutex );
    RegionData_Impl    *pData = maRegions.First();
    sal_Bool            bFound;
    sal_Bool            bSameName = sal_False;

    while ( pData && ( pData->Compare( pNew ) != 0 ) )
        pData = maRegions.Next();

    if ( pData )
    {
        bFound = sal_True;
        pData->SetInUse( sal_True );
    }
    else
    {
        bFound = sal_False;
        pNew->SetInUse( sal_True );

        NamePair_Impl   *pPair = maNames.First();

        if ( pNew->GetTitle() == pPair->maLongName )
            maRegions.Insert( pNew, (ULONG) 0 );
        else
            maRegions.Insert( pNew, LIST_APPEND );
    }

    return bFound;

    // don't use binary search here, the region list isn't sorted
    // at least, not in the moment
#if 0

    int     nCompVal = 1;
    long    nStart = 0;
    long    nEnd = maRegions.Count() - 1;
    long    nMid;

    RegionData_Impl* pMid;

    while ( nCompVal && ( nStart <= nEnd ) )
    {
        nMid = ( nEnd - nStart ) / 2 + nStart;
        pMid = maRegions.GetObject( nMid );

        nCompVal = pMid->Compare( pNew /*, &bSameName */ );

        if ( nCompVal < 0 )     // pMid < pNew
            nStart = nMid + 1;
        else
            nEnd = nMid - 1;
    }

    if ( nCompVal == 0 )
    {
        bFound = TRUE;
        pMid->SetInUse( sal_True );
    }
    else
    {
        if ( nCompVal < 0 )     // pMid < pNew
            nMid++;

        maRegions.Insert( pNew, nMid );
        bFound = sal_False;
    }

    return bFound;
#endif
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

    Sequence< OUString > aNames(2);
    Sequence< Any > aValues(2);

    OUString* pNames = aNames.getArray();
    pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
    pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

    OUString aAdditionalProp( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
    OUString aType( RTL_CONSTASCII_USTRINGPARAM( TYPE_FOLDER ) );

    sal_Bool bExists = sal_False;

    //dv!  hier muss ein URLObject benutzt werden
    OUString aNewFolderURL = aRootURL + rTitle;

#ifndef DONT_USE_HIERARCHY

    Content aFolder;

    try
    {
        aFolder = Content( aNewFolderURL, aCmdEnv );
        bExists = sal_True;
    }
    catch( ContentCreationException& )
    {
        Any* pValues = aValues.getArray();
        pValues[0] = makeAny( rTitle );
        pValues[1] = makeAny( sal_Bool( sal_True ) );

        try
        {
            rRoot.insertNewContent( aType, aNames, aValues, aFolder );
            Reference< XPropertySetInfo > xPropSet = aFolder.getProperties();
            if ( xPropSet.is() )
            {
                if ( ! xPropSet->hasPropertyByName( aAdditionalProp ) )
                {
                    Reference< XPropertyContainer > xFolderProp( aFolder.get(), UNO_QUERY );
                    if ( xFolderProp.is() )
                    {
                        try
                        {
                            xFolderProp->addProperty( aAdditionalProp,
                                                      PropertyAttribute::MAYBEVOID,
                                                      makeAny( rTargetURL ) );
                        }
                        catch( PropertyExistException& ) {}
                        catch( IllegalTypeException& ) { DBG_ERRORFILE( "IllegalTypeException" ); }
                        catch( IllegalArgumentException& ) { DBG_ERRORFILE( "IllegalArgumentException" ); }
                    }
                }

                aFolder.setPropertyValue( aAdditionalProp, makeAny( rTargetURL ) );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "CommandAbortedException" );
        }
    }
    catch ( Exception& )
    { return FALSE; }

#endif

    if ( !bExists )
    {
        RegionData_Impl *pRegion = new RegionData_Impl( rTitle );
        pRegion->SetTargetURL( rTargetURL );
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

#ifndef DONT_USE_HIERARCHY

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
    catch ( Exception& )
    { return FALSE; }

#endif

    return bExists;
}

// -----------------------------------------------------------------------
#if 0 // not needed any longer, because GetTitleFromURL gets the type, too
      // reactivate, if the type isn't good enough ( no deep type detection )
OUString SfxDocTemplate_Impl::GetTypeFromURL( const OUString& rURL )
{
#ifdef TF_FILTER
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_TYPEDETECTION ) );
    Reference< XTypeDetection > xTypeDetection( mxFactory->createInstance( aService ), UNO_QUERY );
    OUString                    aTypeName;

    if( xTypeDetection.is() == sal_True )
    {
        // Build a new media descriptor.
        // He will "walking" during all member of our type detection and loading process as an InOut-parameter!
        // Every "user" can add or remove information about given document.

        Sequence< PropertyValue > aValues(1);
        PropertyValue* pValues = aValues.getArray();

        pValues->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPEDETECTION_PARAMETER ) );
        pValues->Value = makeAny( rURL );

        // Try to get right type name for given descriptor.
        // (Allow a deep detection by calling with "sal_True" - If no input stream already exist - every member of our detection process cann add him to the descriptor!)
        aTypeName = xTypeDetection->queryTypeByDescriptor( aValues, sal_True );
    }
    return aTypeName;
#else
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_OLD_TYPEDETECTION ) );
    Reference< XFrameLoaderQuery >  xTypeDetection( mxFactory->createInstance( aService ), UNO_QUERY );
    OUString                        aTypeName;

    if( xTypeDetection.is() == sal_True )
     {
         Sequence< PropertyValue > aValues(1);
         PropertyValue* pValues = aValues.getArray();

        pValues->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( PARAMETER_OLD_TYPEDETECTION ) );
        pValues->Value = makeAny( sal_Bool( sal_True ) );

        aTypeName = xTypeDetection->searchFilter( rURL, aValues );
    }
    return aTypeName;
#endif
}
#endif

// -----------------------------------------------------------------------

void SfxDocTemplate_Impl::GetTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType )
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

                aPropName = OUString( RTL_CONSTASCII_USTRINGPARAM( "MIMEType" ) );
                aValue = aPropSet->getPropertyValue( aPropName );
                aValue >>= aType;
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
        aTitle = aURL.GetName();
    }
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::ReadFolderList()
{
    ResStringArray  aShortNames( SfxResId( TEMPLATE_SHORT_NAMES_ARY ) );
    ResStringArray  aLongNames( SfxResId( TEMPLATE_LONG_NAMES_ARY ) );

    NamePair_Impl*  pPair;


    USHORT nCount = Min( aShortNames.Count(), aLongNames.Count() );

    for ( USHORT i=0; i<nCount; i++ )
    {
        pPair = new NamePair_Impl;
        pPair->maShortName  = aShortNames.GetString( i );
        pPair->maLongName   = aLongNames.GetString( i );

        maNames.Insert( pPair, LIST_APPEND );
    }
}

// -----------------------------------------------------------------------
OUString SfxDocTemplate_Impl::GetLongName( const OUString& rShortName )
{
    OUString         aRet;
    NamePair_Impl   *pPair = maNames.First();

    while ( pPair )
    {
        if ( pPair->maShortName == rShortName )
        {
            aRet = pPair->maLongName;
            break;
        }
        else
            pPair = maNames.Next();
    }

    if ( !aRet.len() )
        aRet = rShortName;

    return aRet;
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTemplate_Impl::Rescan( sal_Bool bNow )
{
    sal_Bool         bRet = sal_True;

    if ( bNow )
    {
        bRet = DoUpdate();
    }
    else
    {
        Updater_Impl *pThread = new Updater_Impl( this );
        pThread->create();
    }

    return bRet;
}

//------------------------------------------------------------------------
sal_Bool SfxDocTemplate_Impl::DoUpdate()
{
    sal_Bool bRet = sal_True;

    // First: reset the flags of all regions and their children
    RegionData_Impl *pRegion = maRegions.First();

    while ( pRegion )
    {
        pRegion->ResetFlags();
        pRegion = maRegions.Next();
    }

    // Now scan all the folder in the template search path and mark
    // them

    Content     aTemplRoot;
    sal_Bool    bNewRoot;

    if ( ! GetTemplateRoot( aTemplRoot, bNewRoot ) )
        return sal_False;

    ULONG i = maDirs.GetTokenCount( C_DELIM );

    while( i )
    {
        Content aTemplDir;

        i -= 1;

        if ( GetTemplateDir( (USHORT) i, aTemplDir ) )
            GetFolders( aTemplRoot, aTemplDir );
    }

    // Now remove all non existing objects

    ULONG nCount = GetRegionCount();

    while ( nCount )
    {
        pRegion = maRegions.GetObject( --nCount );

        if ( pRegion->IsInUse() )
            pRegion->RemoveUnused();
        else
        {
            pRegion->RemoveRegion();
            maRegions.Remove( nCount );
            delete pRegion;
        }
    }

    return bRet;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

Updater_Impl::Updater_Impl( SfxDocTemplate_Impl* pTemplates )
{
    maDocTemplates = pTemplates;
}

//------------------------------------------------------------------------
Updater_Impl::~Updater_Impl()
{
    maDocTemplates = NULL;
}

//------------------------------------------------------------------------
void SAL_CALL Updater_Impl::run()
{
    maDocTemplates->DoUpdate();
}

//------------------------------------------------------------------------
void SAL_CALL Updater_Impl::onTerminated()
{
    delete this;
}

WaitWindow_Impl::WaitWindow_Impl()
    : WorkWindow( NULL, WB_BORDER | WB_3DLOOK )
{
    Rectangle aRect = Rectangle( 0, 0, 300, 30000 );
    _nTextStyle = TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_WORDBREAK | TEXT_DRAW_MULTILINE;
    _aText = String( SfxResId( RID_CNT_STR_WAITING ) );
    _aRect = GetTextRect( aRect, _aText, _nTextStyle );
    aRect = _aRect;
    aRect.Right() += 2*X_OFFSET;
    aRect.Bottom() += 2*Y_OFFSET;
    _aRect.SetPos( Point( X_OFFSET, Y_OFFSET ) );
    SetOutputSizePixel( aRect.GetSize() );
    Show();
    Update();
    Flush();
}

WaitWindow_Impl::~WaitWindow_Impl()
{
    Hide();
}

void WaitWindow_Impl::Paint( const Rectangle& rRect )
{
    DrawText( _aRect, _aText, _nTextStyle );
}

