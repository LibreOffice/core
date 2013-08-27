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


#include <limits.h>
#include <com/sun/star/uno/Any.h>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/resary.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XPersist.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/AnyCompareFactory.hpp>
#include <com/sun/star/ucb/XAnyCompare.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include "sfxurlrelocator.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::document;
using namespace ::rtl;
using namespace ::ucbhelper;


#include <sfx2/doctempl.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include "sfxtypes.hxx"
#include <sfx2/app.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/templatelocnames.hrc>
#include "doc.hrc"
#include <sfx2/fcontnr.hxx>
#include <svtools/templatefoldercache.hxx>

#include <comphelper/storagehelper.hxx>
#include <unotools/ucbhelper.hxx>

#include <vector>
using ::std::vector;
using ::std::advance;

//========================================================================

#define TITLE                   "Title"
#define TARGET_URL              "TargetURL"

#define COMMAND_TRANSFER        "transfer"

//========================================================================

class RegionData_Impl;

namespace DocTempl {

class DocTempl_EntryData_Impl
{
    RegionData_Impl*    mpParent;

    // the following member must be SfxObjectShellLock since it controlls that SfxObjectShell lifetime by design
    // and users of this class expect it to be so.
    SfxObjectShellLock  mxObjShell;

    OUString            maTitle;
    OUString            maOwnURL;
    OUString            maTargetURL;
    sal_Bool            mbIsOwner   : 1;
    sal_Bool            mbDidConvert: 1;

private:
    RegionData_Impl*    GetParent() const { return mpParent; }

public:
                        DocTempl_EntryData_Impl( RegionData_Impl* pParent,
                                        const OUString& rTitle );

    const OUString&     GetTitle() const { return maTitle; }
    const OUString&     GetTargetURL();
    const OUString&     GetHierarchyURL();

    void                SetTitle( const OUString& rTitle ) { maTitle = rTitle; }
    void                SetTargetURL( const OUString& rURL ) { maTargetURL = rURL; }
    void                SetHierarchyURL( const OUString& rURL) { maOwnURL = rURL; }

    int                 Compare( const OUString& rTitle ) const;
};

}

using namespace ::DocTempl;

// ------------------------------------------------------------------------

class RegionData_Impl
{
    const SfxDocTemplate_Impl*  mpParent;
    vector< DocTempl_EntryData_Impl* > maEntries;
    OUString                    maTitle;
    OUString                    maOwnURL;
    OUString                    maTargetURL;

private:
    size_t                      GetEntryPos( const OUString& rTitle,
                                             sal_Bool& rFound ) const;
    const SfxDocTemplate_Impl*  GetParent() const { return mpParent; }

public:
                        RegionData_Impl( const SfxDocTemplate_Impl* pParent,
                                         const OUString& rTitle );
                        ~RegionData_Impl();

    void                SetTargetURL( const OUString& rURL ) { maTargetURL = rURL; }
    void                SetHierarchyURL( const OUString& rURL) { maOwnURL = rURL; }

    DocTempl_EntryData_Impl*     GetEntry( size_t nIndex ) const;
    DocTempl_EntryData_Impl*     GetEntry( const OUString& rName ) const;

    const OUString&     GetTitle() const { return maTitle; }
    const OUString&     GetHierarchyURL();

    size_t              GetCount() const;

    void                SetTitle( const OUString& rTitle ) { maTitle = rTitle; }

    void                AddEntry( const OUString& rTitle,
                                  const OUString& rTargetURL,
                                  size_t *pPos = NULL );
    void                DeleteEntry( size_t nIndex );

    int                 Compare( const OUString& rTitle ) const
                            { return maTitle.compareTo( rTitle ); }
    int                 Compare( RegionData_Impl* pCompareWith ) const;
};

typedef vector< RegionData_Impl* > RegionList_Impl;

// ------------------------------------------------------------------------

class SfxDocTemplate_Impl : public SvRefBase
{
    uno::Reference< XPersist >               mxInfo;
    uno::Reference< XDocumentTemplates >     mxTemplates;

    ::osl::Mutex        maMutex;
    OUString            maRootURL;
    OUString            maStandardGroup;
    RegionList_Impl     maRegions;
    sal_Bool            mbConstructed;

    uno::Reference< XAnyCompareFactory > m_rCompareFactory;

    // the following member is intended to prevent clearing of the global data when it is in use
    // TODO/LATER: it still does not make the implementation complete thread-safe
    sal_Int32           mnLockCounter;

private:
    void                Clear();

public:
                        SfxDocTemplate_Impl();
                        ~SfxDocTemplate_Impl();

    void                IncrementLock();
    void                DecrementLock();

    sal_Bool            Construct( );
    void                CreateFromHierarchy( Content &rTemplRoot );
    void                ReInitFromComponent();
    void                AddRegion( const OUString& rTitle,
                                   Content& rContent );

    void                Rescan();

    void                DeleteRegion( size_t nIndex );

    size_t              GetRegionCount() const
                            { return maRegions.size(); }
    RegionData_Impl*    GetRegion( const OUString& rName ) const;
    RegionData_Impl*    GetRegion( size_t nIndex ) const;

    sal_Bool            GetTitleFromURL( const OUString& rURL, OUString& aTitle );
    sal_Bool            InsertRegion( RegionData_Impl *pData, size_t nPos = size_t(-1) );
    OUString            GetRootURL() const { return maRootURL; }

    uno::Reference< XDocumentTemplates >     getDocTemplates() { return mxTemplates; }
};

// ------------------------------------------------------------------------

class DocTemplLocker_Impl
{
    SfxDocTemplate_Impl& m_aDocTempl;
public:
    DocTemplLocker_Impl( SfxDocTemplate_Impl& aDocTempl )
    : m_aDocTempl( aDocTempl )
    {
        m_aDocTempl.IncrementLock();
    }

    ~DocTemplLocker_Impl()
    {
        m_aDocTempl.DecrementLock();
    }
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

OUString SfxDocumentTemplates::GetFullRegionName
(
    sal_uInt16 nIdx                     // Region Index
)   const

/*  [Description]

    Returns the logical name of a region and its path

    [Return value]                 Reference to the Region name

*/

{
    // First: find the RegionData for the index
    OUString aName;

    DocTemplLocker_Impl aLocker( *pImp );

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

const OUString& SfxDocumentTemplates::GetRegionName
(
    sal_uInt16 nIdx                 // Region Index
)   const

/*  [Description]

    Returns the logical name of a region

    [Return value]

    const String&                   Reference to the Region name

*/
{
    static OUString maTmpString;

    DocTemplLocker_Impl aLocker( *pImp );

    if ( pImp->Construct() )
    {
        RegionData_Impl *pData = pImp->GetRegion( nIdx );

        if ( pData )
            maTmpString = pData->GetTitle();
        else
            maTmpString = "";
    }
    else
        maTmpString = "";

    return maTmpString;
}

//------------------------------------------------------------------------

sal_uInt16 SfxDocumentTemplates::GetRegionCount() const

/*  [Description]

    Returns the number of Regions

    [Return value]

    sal_uInt16                  Number of Regions
*/
{
    DocTemplLocker_Impl aLocker( *pImp );

    if ( !pImp->Construct() )
        return 0;

    sal_uIntPtr nCount = pImp->GetRegionCount();

    return (sal_uInt16) nCount;
}

//------------------------------------------------------------------------

sal_uInt16 SfxDocumentTemplates::GetCount
(
    sal_uInt16 nRegion              /* Region index whose number is
                                   to be determined */

)   const

/*  [Description]

    Number of entries in Region

    [Return value]                 Number of entries
*/

{
    DocTemplLocker_Impl aLocker( *pImp );

    if ( !pImp->Construct() )
        return 0;

    RegionData_Impl *pData = pImp->GetRegion( nRegion );
    sal_uIntPtr            nCount = 0;

    if ( pData )
        nCount = pData->GetCount();

    return (sal_uInt16) nCount;
}

//------------------------------------------------------------------------

const OUString& SfxDocumentTemplates::GetName
(
    sal_uInt16 nRegion,     //  Region Index, in which the entry lies
    sal_uInt16 nIdx         //  Index of the entry
)   const

/*  [Description]

    Returns the logical name of an entry in Region

    [Return value]

    const String&           Entry Name
*/

{
    DocTemplLocker_Impl aLocker( *pImp );

    static OUString maTmpString;

    if ( pImp->Construct() )
    {
        DocTempl_EntryData_Impl  *pEntry = NULL;
        RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

        if ( pRegion )
            pEntry = pRegion->GetEntry( nIdx );

        if ( pEntry )
            maTmpString = pEntry->GetTitle();
        else
            maTmpString = "";
    }
    else
        maTmpString = "";

    return maTmpString;
}

//------------------------------------------------------------------------

OUString SfxDocumentTemplates::GetPath
(
    sal_uInt16  nRegion,    //  Region Index, in which the entry lies
    sal_uInt16  nIdx        //  Index of the entry
)   const

/*  [Description]

    Returns the file name with full path to the file assigned to an entry

    [Return value]

    String                  File name with full path
*/
{
    DocTemplLocker_Impl aLocker( *pImp );

    if ( !pImp->Construct() )
        return OUString();

    DocTempl_EntryData_Impl  *pEntry = NULL;
    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( pRegion )
        pEntry = pRegion->GetEntry( nIdx );

    if ( pEntry )
        return pEntry->GetTargetURL();
    else
        return OUString();
}

//------------------------------------------------------------------------

OUString SfxDocumentTemplates::GetTemplateTargetURLFromComponent( const OUString& aGroupName,
                                                                         const OUString& aTitle )
{
    DocTemplLocker_Impl aLocker( *pImp );

    INetURLObject aTemplateObj( pImp->GetRootURL() );

    aTemplateObj.insertName( aGroupName, false,
                        INetURLObject::LAST_SEGMENT, true,
                        INetURLObject::ENCODE_ALL );

    aTemplateObj.insertName( aTitle, false,
                        INetURLObject::LAST_SEGMENT, true,
                        INetURLObject::ENCODE_ALL );


    OUString aResult;
    Content aTemplate;
    uno::Reference< XCommandEnvironment > aCmdEnv;
    if ( Content::create( aTemplateObj.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
    {
        OUString aPropName( TARGET_URL  );
        getTextProperty_Impl( aTemplate, aPropName, aResult );
        aResult = SvtPathOptions().SubstituteVariable( aResult );
    }

    return aResult;
}

//------------------------------------------------------------------------

/** Convert a resource string - a template name - to its localised pair if it exists.
    @param nSourceResIds
        Resource ID where the list of original en-US template names begin.
    @param nDestResIds
        Resource ID where the list of localised template names begin.
    @param nCount
        The number of names that have been localised.
    @param rString
        Name to be translated.
    @return
        The localised pair of rString or rString if the former does not exist.
*/
OUString SfxDocumentTemplates::ConvertResourceString (
    int nSourceResIds, int nDestResIds, int nCount, const OUString& rString )
{
    for( int i = 0; i < nCount; ++i )
    {
        if( rString == SFX2_RESSTR(nSourceResIds + i))
            return SFX2_RESSTR(nDestResIds + i);
    }
    return rString;
}

//------------------------------------------------------------------------

sal_Bool SfxDocumentTemplates::CopyOrMove
(
    sal_uInt16  nTargetRegion,      //  Target Region Index
    sal_uInt16  nTargetIdx,         //  Target position Index
    sal_uInt16  nSourceRegion,      //  Source Region Index
    sal_uInt16  nSourceIdx,         /*  Index to be copied / to moved template */
    sal_Bool    bMove           //  Copy / Move
)

/*  [Description]

    Copy or move a document template

    [Return value]

    sal_Bool            sal_True,   Action could be performed
                        sal_False,  Action could not be performed

    [Cross-references]

    <SfxDocumentTemplates::Move(sal_uInt16,sal_uInt16,sal_uInt16,sal_uInt16)>
    <SfxDocumentTemplates::Copy(sal_uInt16,sal_uInt16,sal_uInt16,sal_uInt16)>
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

    DocTemplLocker_Impl aLocker( *pImp );

    if ( !pImp->Construct() )
        return sal_False;

    // Don't copy or move any folders
    if( nSourceIdx == USHRT_MAX )
        return sal_False ;

    if ( nSourceRegion == nTargetRegion )
    {
        SAL_WARN( "sfx.doc", "Don't know, what to do!" );
        return sal_False;
    }

    RegionData_Impl *pSourceRgn = pImp->GetRegion( nSourceRegion );
    if ( !pSourceRgn )
        return sal_False;

    DocTempl_EntryData_Impl *pSource = pSourceRgn->GetEntry( nSourceIdx );
    if ( !pSource )
        return sal_False;

    RegionData_Impl *pTargetRgn = pImp->GetRegion( nTargetRegion );
    if ( !pTargetRgn )
        return sal_False;

    OUString aTitle = pSource->GetTitle();

    uno::Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();

    if ( xTemplates->addTemplate( pTargetRgn->GetTitle(),
                                  aTitle,
                                  pSource->GetTargetURL() ) )
    {

        INetURLObject aSourceObj( pSource->GetTargetURL() );

        OUString aNewTargetURL = GetTemplateTargetURLFromComponent( pTargetRgn->GetTitle(), aTitle );
        if ( aNewTargetURL.isEmpty() )
            return sal_False;

        if ( bMove )
        {
            // --**-- delete the original file
            sal_Bool bDeleted = xTemplates->removeTemplate( pSourceRgn->GetTitle(),
                                                            pSource->GetTitle() );
            if ( bDeleted )
                pSourceRgn->DeleteEntry( nSourceIdx );
            else
            {
                if ( xTemplates->removeTemplate( pTargetRgn->GetTitle(), aTitle ) )
                    return sal_False; // will trigger tetry with copy instead of move

                // if it is not possible to remove just created template ( must be possible! )
                // it is better to report success here, since at least the copy has succeeded
                // TODO/LATER: solve it more gracefully in future
            }
        }

        // todo: fix SfxDocumentTemplates to handle size_t instead of sal_uInt16
        size_t temp_nTargetIdx = nTargetIdx;
        pTargetRgn->AddEntry( aTitle, aNewTargetURL, &temp_nTargetIdx );

        return sal_True;
    }

    // --**-- if the current file is opened,
    // it must be re-opened afterwards.

    return sal_False;
}

//------------------------------------------------------------------------

sal_Bool SfxDocumentTemplates::Move
(
    sal_uInt16 nTargetRegion,       //  Target Region Index
    sal_uInt16 nTargetIdx,          //  Target position Index
    sal_uInt16 nSourceRegion,       //  Source Region Index
    sal_uInt16 nSourceIdx           /*  Index to be copied / to moved template */
)

/*  [Description]

    Moving a template

    [Return value]

    sal_Bool            sal_True,   Action could be performed
                        sal_False,  Action could not be performed

    [Cross-references]

    <SfxDocumentTemplates::CopyOrMove(sal_uInt16,sal_uInt16,sal_uInt16,sal_uInt16,sal_Bool)>
*/
{
    DocTemplLocker_Impl aLocker( *pImp );

    return CopyOrMove( nTargetRegion, nTargetIdx,
                       nSourceRegion, nSourceIdx, sal_True );
}

//------------------------------------------------------------------------

sal_Bool SfxDocumentTemplates::Copy
(
    sal_uInt16 nTargetRegion,       //  Target Region Index
    sal_uInt16 nTargetIdx,          //  Target position Index
    sal_uInt16 nSourceRegion,       //  Source Region Index
    sal_uInt16 nSourceIdx           /*  Index to be copied / to moved template */
)

/*  [Description]

    Copying a template

    [Return value]

    sal_Bool            sal_True,   Action could be performed
                        sal_False,  Action could not be performed

    [Cross-references]

    <SfxDocumentTemplates::CopyOrMove(sal_uInt16,sal_uInt16,sal_uInt16,sal_uInt16,sal_Bool)>
*/

{
    DocTemplLocker_Impl aLocker( *pImp );

    return CopyOrMove( nTargetRegion, nTargetIdx,
                       nSourceRegion, nSourceIdx, sal_False );
}

//------------------------------------------------------------------------

sal_Bool SfxDocumentTemplates::CopyTo
(
    sal_uInt16          nRegion,    //  Region of the template to be exported
    sal_uInt16          nIdx,       //  Index of the template to be exported
    const OUString&     rName       /*  File name under which the template is to
                                    be created */
)   const

/*  [Description]

    Exporting a template into the file system

    [Return value]

    sal_Bool            sal_True,   Action could be performed
                        sal_False,  Action could not be performed

    [Cross-references]

    <SfxDocumentTemplates::CopyFrom(sal_uInt16,sal_uInt16,String&)>
*/

{
    DocTemplLocker_Impl aLocker( *pImp );

    if ( ! pImp->Construct() )
        return sal_False;

    RegionData_Impl *pSourceRgn = pImp->GetRegion( nRegion );
    if ( !pSourceRgn )
        return sal_False;

    DocTempl_EntryData_Impl *pSource = pSourceRgn->GetEntry( nIdx );
    if ( !pSource )
        return sal_False;

    INetURLObject aTargetURL( rName );

    OUString aTitle( aTargetURL.getName( INetURLObject::LAST_SEGMENT, true,
                                         INetURLObject::DECODE_WITH_CHARSET ) );
    aTargetURL.removeSegment();

    OUString aParentURL = aTargetURL.GetMainURL( INetURLObject::NO_DECODE );

    uno::Reference< XCommandEnvironment > aCmdEnv;
    Content aTarget;

    try
    {
        aTarget = Content( aParentURL, aCmdEnv, comphelper::getProcessComponentContext() );

        TransferInfo aTransferInfo;
        aTransferInfo.MoveData = sal_False;
        aTransferInfo.SourceURL = pSource->GetTargetURL();
        aTransferInfo.NewTitle = aTitle;
        aTransferInfo.NameClash = NameClash::OVERWRITE;

        Any aArg = makeAny( aTransferInfo );
        OUString aCmd( COMMAND_TRANSFER  );

        aTarget.executeCommand( aCmd, aArg );
    }
    catch ( ContentCreationException& )
    { return sal_False; }
    catch ( Exception& )
    { return sal_False; }

    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool SfxDocumentTemplates::CopyFrom
(
    sal_uInt16      nRegion,        /*  Region in which the template is to be
                                    imported */
    sal_uInt16      nIdx,           //  Index of the new template in this Region
    OUString&       rName           /*  File name of the template to be imported
                                    as an out parameter of the (automatically
                                    generated from the file name) logical name
                                    of the template */
)

/*  [Description]

    Import a template from the file system

    [Return value]                 Sucess (sal_True) or serfpTargetDirectory->GetContent());

    sal_Bool            sal_True,   Action could be performed
                        sal_False,  Action could not be performed

    [Cross-references]

    <SfxDocumentTemplates::CopyTo(sal_uInt16,sal_uInt16,const String&)>
*/

{
    DocTemplLocker_Impl aLocker( *pImp );

    if ( ! pImp->Construct() )
        return sal_False;

    RegionData_Impl *pTargetRgn = pImp->GetRegion( nRegion );

    if ( !pTargetRgn )
        return sal_False;

    uno::Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();
    if ( !xTemplates.is() )
        return sal_False;

    OUString aTitle;
    sal_Bool bTemplateAdded = sal_False;

    if( pImp->GetTitleFromURL( rName, aTitle ) )
    {
        bTemplateAdded = xTemplates->addTemplate( pTargetRgn->GetTitle(), aTitle, rName );
    }
    else
    {
        uno::Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );;

        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString("Hidden");
        aArgs[0].Value <<= sal_True;

        INetURLObject   aTemplURL( rName );
        uno::Reference< XDocumentPropertiesSupplier > xDocPropsSupplier;
        uno::Reference< XStorable > xStorable;
        try
        {
            xStorable = uno::Reference< XStorable >(
                xDesktop->loadComponentFromURL( aTemplURL.GetMainURL(INetURLObject::NO_DECODE),
                                                OUString("_blank"),
                                                0,
                                                aArgs ),
                UNO_QUERY );

            xDocPropsSupplier = uno::Reference< XDocumentPropertiesSupplier >(
                xStorable, UNO_QUERY );
        }
        catch( Exception& )
        {
        }

        if( xStorable.is() )
        {
            // get Title from XDocumentPropertiesSupplier
            if( xDocPropsSupplier.is() )
            {
                uno::Reference< XDocumentProperties > xDocProps
                    = xDocPropsSupplier->getDocumentProperties();
                if (xDocProps.is() ) {
                    aTitle = xDocProps->getTitle();
                }
            }

            if( aTitle.isEmpty() )
            {
                INetURLObject aURL( aTemplURL );
                aURL.CutExtension();
                aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true,
                                        INetURLObject::DECODE_WITH_CHARSET );
            }

            // write a template using XStorable interface
            bTemplateAdded = xTemplates->storeTemplate( pTargetRgn->GetTitle(), aTitle, xStorable );
        }
    }


    if( bTemplateAdded )
    {
        INetURLObject aTemplObj( pTargetRgn->GetHierarchyURL() );
        aTemplObj.insertName( aTitle, false,
                              INetURLObject::LAST_SEGMENT, true,
                              INetURLObject::ENCODE_ALL );
        OUString aTemplURL = aTemplObj.GetMainURL( INetURLObject::NO_DECODE );

        uno::Reference< XCommandEnvironment > aCmdEnv;
        Content aTemplCont;

        if( Content::create( aTemplURL, aCmdEnv, comphelper::getProcessComponentContext(), aTemplCont ) )
        {
            OUString aTemplName;
            OUString aPropName( TARGET_URL  );

            if( getTextProperty_Impl( aTemplCont, aPropName, aTemplName ) )
            {
                if ( nIdx == USHRT_MAX )
                    nIdx = 0;
                else
                    nIdx += 1;

                // todo: fix SfxDocumentTemplates to handle size_t instead of sal_uInt16
                size_t temp_nIdx = nIdx;
                pTargetRgn->AddEntry( aTitle, aTemplName, &temp_nIdx );
                rName = aTitle;
                return sal_True;
            }
            else
            {
                DBG_ASSERT( sal_False, "CopyFrom(): The content should contain target URL!" );
            }
        }
        else
        {
            DBG_ASSERT( sal_False, "CopyFrom(): The content just was created!" );
        }
    }

    return sal_False;
}

//------------------------------------------------------------------------

sal_Bool SfxDocumentTemplates::Delete
(
    sal_uInt16 nRegion,             //  Region Index
    sal_uInt16 nIdx                 /*  Index of the entry or USHRT_MAX,
                                    if a directory is meant. */
)

/*  [Description]

    Deleting an entry or a directory

    [Return value]

    sal_Bool            sal_True,   Action could be performed
                        sal_False,  Action could not be performed

    [Cross-references]

    <SfxDocumentTemplates::InsertDir(const String&,sal_uInt16)>
    <SfxDocumentTemplates::KillDir(SfxTemplateDir&)>
*/

{
    DocTemplLocker_Impl aLocker( *pImp );

    /* delete the template or folder in the hierarchy and in the
       template folder by sending a delete command to the content.
       Then remove the data from the lists
    */
    if ( ! pImp->Construct() )
        return sal_False;

    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( !pRegion )
        return sal_False;

    sal_Bool bRet;
    uno::Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();

    if ( nIdx == USHRT_MAX )
    {
        bRet = xTemplates->removeGroup( pRegion->GetTitle() );
        if ( bRet )
            pImp->DeleteRegion( nRegion );
    }
    else
    {
        DocTempl_EntryData_Impl *pEntry = pRegion->GetEntry( nIdx );

        if ( !pEntry )
            return sal_False;

        bRet = xTemplates->removeTemplate( pRegion->GetTitle(),
                                           pEntry->GetTitle() );
        if( bRet )
            pRegion->DeleteEntry( nIdx );
    }

    return bRet;
}

//------------------------------------------------------------------------

sal_Bool SfxDocumentTemplates::InsertDir
(
    const OUString&     rText,      //  the logical name of the new Region
    sal_uInt16          nRegion     //  Region Index
)

/*  [Description]

    Insert an index

    [Return value]

    sal_Bool            sal_True,   Action could be performed
                        sal_False,  Action could not be performed

    [Cross-references]

    <SfxDocumentTemplates::KillDir(SfxTemplateDir&)>
*/
{
    DocTemplLocker_Impl aLocker( *pImp );

    if ( ! pImp->Construct() )
        return sal_False;

    RegionData_Impl *pRegion = pImp->GetRegion( rText );

    if ( pRegion )
        return sal_False;

    uno::Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();

    if ( xTemplates->addGroup( rText ) )
    {
        RegionData_Impl* pNewRegion = new RegionData_Impl( pImp, rText );

        if ( ! pImp->InsertRegion( pNewRegion, nRegion ) )
        {
            delete pNewRegion;
            return sal_False;
        }
        return sal_True;
    }

    return sal_False;
}

sal_Bool SfxDocumentTemplates::InsertTemplate(sal_uInt16 nSourceRegion, sal_uInt16 nIdx, const OUString &rName, const OUString &rPath)
{
    DocTemplLocker_Impl aLocker( *pImp );

    if ( ! pImp->Construct() )
        return sal_False;

    RegionData_Impl *pRegion = pImp->GetRegion( nSourceRegion );

    if ( !pRegion )
        return sal_False;

    size_t pos = nIdx;
    pRegion->AddEntry( rName, rPath, &pos );

    return sal_True;
}

sal_Bool SfxDocumentTemplates::SetName( const OUString& rName, sal_uInt16 nRegion, sal_uInt16 nIdx )

{
    DocTemplLocker_Impl aLocker( *pImp );

    if ( ! pImp->Construct() )
        return sal_False;

    RegionData_Impl *pRegion = pImp->GetRegion( nRegion );

    if ( !pRegion )
        return sal_False;

    uno::Reference< XDocumentTemplates > xTemplates = pImp->getDocTemplates();
    OUString aEmpty;

    if ( nIdx == USHRT_MAX )
    {
        if ( pRegion->GetTitle() == rName )
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
        DocTempl_EntryData_Impl *pEntry = pRegion->GetEntry( nIdx );

        if ( !pEntry )
            return sal_False;

        if ( pEntry->GetTitle() == rName )
            return sal_True;

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

sal_Bool SfxDocumentTemplates::GetFull
(
    const OUString &rRegion,      // Region Name
    const OUString &rName,        // Template Name
    OUString &rPath               // Out: Path + File name
)

/*  [Description]

    Returns Path + File name of the template specified by rRegion and rName.

    [Return value]

    sal_Bool            sal_True,   Action could be performed
                        sal_False,  Action could not be performed

    [Cross-references]

    <SfxDocumentTemplates::GetLogicNames(const String&,String&,String&)>
*/

{
    DocTemplLocker_Impl aLocker( *pImp );

    // We don't search for empty names!
    if ( rName.isEmpty() )
        return sal_False;

    if ( ! pImp->Construct() )
        return sal_False;

    DocTempl_EntryData_Impl* pEntry = NULL;
    const sal_uInt16 nCount = GetRegionCount();

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        RegionData_Impl *pRegion = pImp->GetRegion( i );

        if( pRegion &&
            ( rRegion.isEmpty() || ( rRegion == pRegion->GetTitle() ) ) )
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

sal_Bool SfxDocumentTemplates::GetLogicNames
(
    const OUString &rPath,            // Full Path to the template
    OUString &rRegion,                // Out: Region name
    OUString &rName                   // Out: Template name
) const

/*  [Description]

    Returns and logical path name to the template specified by rPath

    [Return value]

    sal_Bool            sal_True,   Action could be performed
                        sal_False,  Action could not be performed

    [Cross-references]

    <SfxDocumentTemplates::GetFull(const String&,const String&,DirEntry&)>
*/

{
    DocTemplLocker_Impl aLocker( *pImp );

    if ( ! pImp->Construct() )
        return sal_False;

    INetURLObject aFullPath;

    aFullPath.SetSmartProtocol( INET_PROT_FILE );
    aFullPath.SetURL( rPath );
    OUString aPath( aFullPath.GetMainURL( INetURLObject::NO_DECODE ) );

    RegionData_Impl *pData = NULL;
    DocTempl_EntryData_Impl  *pEntry = NULL;
    sal_Bool         bFound = sal_False;

    sal_uIntPtr nCount = GetRegionCount();

    for ( sal_uIntPtr i=0; !bFound && (i<nCount); i++ )
    {
        pData = pImp->GetRegion( i );
        if ( pData )
        {
            sal_uIntPtr nChildCount = pData->GetCount();

            for ( sal_uIntPtr j=0; !bFound && (j<nChildCount); j++ )
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

/*  [Description]

    Constructor
*/
{
    if ( !gpTemplateData )
        gpTemplateData = new SfxDocTemplate_Impl;

    pImp = gpTemplateData;
}

//-------------------------------------------------------------------------

void SfxDocumentTemplates::Construct()

//  Delayed build-up of administrative data

{
}

//------------------------------------------------------------------------

SfxDocumentTemplates::~SfxDocumentTemplates()

/*  [Description]

    Destructor
    Release of administrative data
*/

{
    pImp = NULL;
}

void SfxDocumentTemplates::Update( sal_Bool _bSmart )
{
    if  (   !_bSmart                                                // don't be smart
        ||  ::svt::TemplateFolderCache( sal_True ).needsUpdate()    // update is really necessary
        )
    {
        if ( pImp->Construct() )
            pImp->Rescan();
    }
}

void SfxDocumentTemplates::ReInitFromComponent()
{
    pImp->ReInitFromComponent();
}

// -----------------------------------------------------------------------
DocTempl_EntryData_Impl::DocTempl_EntryData_Impl( RegionData_Impl* pParent,
                                const OUString& rTitle )
{
    mpParent    = pParent;
    maTitle     = SfxDocumentTemplates::ConvertResourceString(
                  STR_TEMPLATE_NAME1_DEF, STR_TEMPLATE_NAME1, NUM_TEMPLATE_NAMES, rTitle );
    mbIsOwner   = sal_False;
    mbDidConvert= sal_False;
}

// -----------------------------------------------------------------------
int DocTempl_EntryData_Impl::Compare( const OUString& rTitle ) const
{
    return maTitle.compareTo( rTitle );
}

// -----------------------------------------------------------------------
const OUString& DocTempl_EntryData_Impl::GetHierarchyURL()
{
    if ( maOwnURL.isEmpty() )
    {
        INetURLObject aTemplateObj( GetParent()->GetHierarchyURL() );

        aTemplateObj.insertName( GetTitle(), false,
                     INetURLObject::LAST_SEGMENT, true,
                     INetURLObject::ENCODE_ALL );

        maOwnURL = aTemplateObj.GetMainURL( INetURLObject::NO_DECODE );
        DBG_ASSERT( !maOwnURL.isEmpty(), "GetHierarchyURL(): Could not create URL!" );
    }

    return maOwnURL;
}

// -----------------------------------------------------------------------
const OUString& DocTempl_EntryData_Impl::GetTargetURL()
{
    if ( maTargetURL.isEmpty() )
    {
        uno::Reference< XCommandEnvironment > aCmdEnv;
        Content aRegion;

        if ( Content::create( GetHierarchyURL(), aCmdEnv, comphelper::getProcessComponentContext(), aRegion ) )
        {
            OUString aPropName( TARGET_URL  );

            getTextProperty_Impl( aRegion, aPropName, maTargetURL );
        }
        else
        {
            SAL_WARN( "sfx.doc", "GetTargetURL(): Could not create hierarchy content!" );
        }
    }

    return maTargetURL;
}

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
    for ( size_t i = 0, n = maEntries.size(); i < n; ++i )
        delete maEntries[ i ];
    maEntries.clear();
}

// -----------------------------------------------------------------------
size_t RegionData_Impl::GetEntryPos( const OUString& rTitle, sal_Bool& rFound ) const
{
#if 1   // Don't use binary search today
    size_t i;
    size_t nCount = maEntries.size();

    for ( i=0; i<nCount; i++ )
    {
        DocTempl_EntryData_Impl *pData = maEntries[ i ];

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
    size_t  nStart = 0;
    size_t  nEnd = maEntries.size() - 1;
    size_t  nMid;

    DocTempl_EntryData_Impl* pMid;

    rFound = sal_False;

    while ( nCompVal && ( nStart <= nEnd ) )
    {
        nMid = ( nEnd - nStart ) / 2 + nStart;
        pMid = maEntries[ nMid ];

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
                                size_t *pPos )
{
    INetURLObject aLinkObj( GetHierarchyURL() );
    aLinkObj.insertName( rTitle, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    OUString aLinkURL = aLinkObj.GetMainURL( INetURLObject::NO_DECODE );

    DocTempl_EntryData_Impl* pEntry;
    sal_Bool        bFound = sal_False;
    size_t          nPos = GetEntryPos( rTitle, bFound );

    if ( bFound )
    {
        pEntry = maEntries[ nPos ];
    }
    else
    {
        if ( pPos )
            nPos = *pPos;

        pEntry = new DocTempl_EntryData_Impl( this, rTitle );
        pEntry->SetTargetURL( rTargetURL );
        pEntry->SetHierarchyURL( aLinkURL );
        if ( nPos < maEntries.size() ) {
            vector< DocTempl_EntryData_Impl* >::iterator it = maEntries.begin();
            advance( it, nPos );
            maEntries.insert( it, pEntry );
        }
        else
            maEntries.push_back( pEntry );
    }
}

// -----------------------------------------------------------------------
size_t RegionData_Impl::GetCount() const
{
    return maEntries.size();
}

// -----------------------------------------------------------------------
const OUString& RegionData_Impl::GetHierarchyURL()
{
    if ( maOwnURL.isEmpty() )
    {
        INetURLObject aRegionObj( GetParent()->GetRootURL() );

        aRegionObj.insertName( GetTitle(), false,
                     INetURLObject::LAST_SEGMENT, true,
                     INetURLObject::ENCODE_ALL );

        maOwnURL = aRegionObj.GetMainURL( INetURLObject::NO_DECODE );
        DBG_ASSERT( !maOwnURL.isEmpty(), "GetHierarchyURL(): Could not create URL!" );
    }

    return maOwnURL;
}

// -----------------------------------------------------------------------
DocTempl_EntryData_Impl* RegionData_Impl::GetEntry( const OUString& rName ) const
{
    sal_Bool    bFound = sal_False;
    long        nPos = GetEntryPos( rName, bFound );

    if ( bFound )
        return maEntries[ nPos ];
    else
        return NULL;
}

// -----------------------------------------------------------------------
DocTempl_EntryData_Impl* RegionData_Impl::GetEntry( size_t nIndex ) const
{
    if ( nIndex < maEntries.size() )
        return maEntries[ nIndex ];
    return NULL;
}

// -----------------------------------------------------------------------
void RegionData_Impl::DeleteEntry( size_t nIndex )
{
    if ( nIndex < maEntries.size() )
    {
        delete maEntries[ nIndex ];
        vector< DocTempl_EntryData_Impl*>::iterator it = maEntries.begin();
        advance( it, nIndex );
        maEntries.erase( it );
    }
}

// -----------------------------------------------------------------------
int RegionData_Impl::Compare( RegionData_Impl* pCompare ) const
{
    int nCompare = maTitle.compareTo( pCompare->maTitle );

    return nCompare;
}

// -----------------------------------------------------------------------

SfxDocTemplate_Impl::SfxDocTemplate_Impl()
: mbConstructed( sal_False )
, mnLockCounter( 0 )
{
}

// -----------------------------------------------------------------------
SfxDocTemplate_Impl::~SfxDocTemplate_Impl()
{
    Clear();

    gpTemplateData = NULL;
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::IncrementLock()
{
    ::osl::MutexGuard aGuard( maMutex );
    mnLockCounter++;
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::DecrementLock()
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mnLockCounter )
        mnLockCounter--;
}

// -----------------------------------------------------------------------
RegionData_Impl* SfxDocTemplate_Impl::GetRegion( size_t nIndex ) const
{
    if ( nIndex < maRegions.size() )
        return maRegions[ nIndex ];
    return NULL;
}

// -----------------------------------------------------------------------
RegionData_Impl* SfxDocTemplate_Impl::GetRegion( const OUString& rName )
    const
{
    for ( size_t i = 0, n = maRegions.size(); i < n; ++i )
    {
        RegionData_Impl* pData = maRegions[ i ];
        if( pData->GetTitle() == rName )
            return pData;
    }
    return NULL;
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::DeleteRegion( size_t nIndex )
{
    if ( nIndex < maRegions.size() )
    {
        delete maRegions[ nIndex ];
        RegionList_Impl::iterator it = maRegions.begin();
        advance( it, nIndex );
        maRegions.erase( it );
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
    uno::Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(2);
    aProps[0] = OUString(TITLE );
    aProps[1] = OUString(TARGET_URL );

    try
    {
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        Sequence< NumberedSortingInfo >     aSortingInfo(1);
        aSortingInfo.getArray()->ColumnIndex = 1;
        aSortingInfo.getArray()->Ascending = sal_True;
        xResultSet = rContent.createSortedCursor( aProps, aSortingInfo, m_rCompareFactory, eInclude );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetDir( xRow->getString( 2 ) );

                pRegion->AddEntry( aTitle, aTargetDir );
            }
        }
        catch ( Exception& ) {}
    }
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::CreateFromHierarchy( Content &rTemplRoot )
{
    uno::Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(1);
    aProps[0] = OUString(TITLE );

    try
    {
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        Sequence< NumberedSortingInfo >     aSortingInfo(1);
        aSortingInfo.getArray()->ColumnIndex = 1;
        aSortingInfo.getArray()->Ascending = sal_True;
        xResultSet = rTemplRoot.createSortedCursor( aProps, aSortingInfo, m_rCompareFactory, eInclude );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        uno::Reference< XCommandEnvironment > aCmdEnv;
        uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString( 1 ) );

                OUString aId = xContentAccess->queryContentIdentifierString();
                Content  aContent = Content( aId, aCmdEnv, comphelper::getProcessComponentContext() );

                AddRegion( aTitle, aContent );
            }
        }
        catch ( Exception& ) {}
    }
}

// ------------------------------------------------------------------------
sal_Bool SfxDocTemplate_Impl::Construct( )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbConstructed )
        return sal_True;

    uno::Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    uno::Reference< XPersist > xInfo( document::DocumentProperties::create(xContext), UNO_QUERY );
    mxInfo = xInfo;

    mxTemplates = frame::DocumentTemplates::create(xContext);

    uno::Reference< XLocalizable > xLocalizable( mxTemplates, UNO_QUERY );

    m_rCompareFactory = AnyCompareFactory::createWithLocale(xContext, xLocalizable->getLocale());

    uno::Reference < XContent > aRootContent = mxTemplates->getContent();
    uno::Reference < XCommandEnvironment > aCmdEnv;

    if ( ! aRootContent.is() )
        return sal_False;

    mbConstructed = sal_True;
    maRootURL = aRootContent->getIdentifier()->getContentIdentifier();

    ResStringArray  aLongNames( SfxResId( TEMPLATE_LONG_NAMES_ARY ) );

    if ( aLongNames.Count() )
        maStandardGroup = aLongNames.GetString( 0 );

    Content aTemplRoot( aRootContent, aCmdEnv, xContext );
    CreateFromHierarchy( aTemplRoot );

    return sal_True;
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::ReInitFromComponent()
{
    uno::Reference< XDocumentTemplates > xTemplates = getDocTemplates();
    if ( xTemplates.is() )
    {
        uno::Reference < XContent > aRootContent = xTemplates->getContent();
        uno::Reference < XCommandEnvironment > aCmdEnv;
        Content aTemplRoot( aRootContent, aCmdEnv, comphelper::getProcessComponentContext() );
        Clear();
        CreateFromHierarchy( aTemplRoot );
    }
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTemplate_Impl::InsertRegion( RegionData_Impl *pNew, size_t nPos )
{
    ::osl::MutexGuard   aGuard( maMutex );

    // return false (not inserted) if the entry already exists
    for ( size_t i = 0, n = maRegions.size(); i < n; ++i )
        if ( maRegions[ i ]->Compare( pNew ) == 0 )
            return sal_False;

    size_t newPos = nPos;
    if ( pNew->GetTitle() == maStandardGroup )
        newPos = 0;

    if ( newPos < maRegions.size() )
    {
        RegionList_Impl::iterator it = maRegions.begin();
        advance( it, newPos );
        maRegions.insert( it, pNew );
    }
    else
        maRegions.push_back( pNew );

    return sal_True;
}

// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::Rescan()
{
    Clear();

    try
    {
        uno::Reference< XDocumentTemplates > xTemplates = getDocTemplates();
        DBG_ASSERT( xTemplates.is(), "SfxDocTemplate_Impl::Rescan:invalid template instance!" );
        if ( xTemplates.is() )
        {
            xTemplates->update();

            uno::Reference < XContent > aRootContent = xTemplates->getContent();
            uno::Reference < XCommandEnvironment > aCmdEnv;

            Content aTemplRoot( aRootContent, aCmdEnv, comphelper::getProcessComponentContext() );
            CreateFromHierarchy( aTemplRoot );
        }
    }
    catch( const Exception& )
    {
        SAL_WARN( "sfx.doc", "SfxDocTemplate_Impl::Rescan: caught an exception while doing the update!" );
    }
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTemplate_Impl::GetTitleFromURL( const OUString& rURL,
                                           OUString& aTitle )
{
    if ( mxInfo.is() )
    {
        try
        {
            mxInfo->read( rURL );
        }
        catch ( Exception& )
        {
            // the document is not a StarOffice document
            return sal_False;
        }


        try
        {
            uno::Reference< XPropertySet > aPropSet( mxInfo, UNO_QUERY );
            if ( aPropSet.is() )
            {
                OUString aPropName( TITLE  );
                Any aValue = aPropSet->getPropertyValue( aPropName );
                aValue >>= aTitle;
            }
        }
        catch ( IOException& ) {}
        catch ( UnknownPropertyException& ) {}
        catch ( Exception& ) {}
    }

    if ( aTitle.isEmpty() )
    {
        INetURLObject aURL( rURL );
        aURL.CutExtension();
        aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true,
                               INetURLObject::DECODE_WITH_CHARSET );
    }

    return sal_True;
}


// -----------------------------------------------------------------------
void SfxDocTemplate_Impl::Clear()
{
    ::osl::MutexGuard   aGuard( maMutex );
    if ( mnLockCounter )
        return;

    for ( size_t i = 0, n = maRegions.size(); i < n; ++i )
        delete maRegions[ i ];
    maRegions.clear();
}

// -----------------------------------------------------------------------
sal_Bool getTextProperty_Impl( Content& rContent,
                               const OUString& rPropName,
                               OUString& rPropValue )
{
    sal_Bool bGotProperty = sal_False;

    // Get the property
    try
    {
        uno::Reference< XPropertySetInfo > aPropInfo = rContent.getProperties();

        // check, whether or not the property exists
        if ( !aPropInfo.is() || !aPropInfo->hasPropertyByName( rPropName ) )
        {
            return sal_False;
        }

        // now get the property
        Any aAnyValue;

        aAnyValue = rContent.getPropertyValue( rPropName );
        aAnyValue >>= rPropValue;

        if ( SfxURLRelocator_Impl::propertyCanContainOfficeDir( rPropName ) )
        {
            SfxURLRelocator_Impl aRelocImpl( ::comphelper::getProcessComponentContext() );
            aRelocImpl.makeAbsoluteURL( rPropValue );
        }

        bGotProperty = sal_True;
    }
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    return bGotProperty;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
