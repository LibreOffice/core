/*************************************************************************
 *
 *  $RCSfile: doctemplates.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dv $ $Date: 2001-03-09 14:50:08 $
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


#include "doctemplates.hxx"

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef  _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SV_RESARY_HXX
#include <vcl/resary.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef  _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif

#ifndef  _COM_SUN_STAR_IO_XPERSIST_HPP_
#include <com/sun/star/io/XPersist.hpp>
#endif

#ifndef  _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef  _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif

#ifndef  _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif

#include "sfxresid.hxx"
#include "doc.hrc"

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#define TEMPLATE_SERVICE_NAME               "com.sun.star.frame.DocumentTemplates"
#define TEMPLATE_IMPLEMENTATION_NAME        "com.sun.star.comp.sfx2.DocumentTemplates"

#define HIERARCHIE_ROOT_URL     "vnd.sun.star.hier:/"
#define TEMPLATE_ROOT_URL       "vnd.sun.star.hier:/templates"
#define TEMPLATE_DIR_NAME       "templates"
#define TITLE                   "Title"
#define IS_FOLDER               "IsFolder"
#define TARGET_URL              "TargetURL"
#define TYPE_FOLDER             "application/vnd.sun.star.hier-folder"
#define TYPE_LINK               "application/vnd.sun.star.hier-link"
#define TYPE_FSYS_FOLDER        "application/vnd.sun.staroffice.fsys-folder"

#define PROPERTY_DIRLIST        "DirectoryList"
#define PROPERTY_TYPE           "TypeDescription"

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

//-----------------------------------------------------------------------------

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;

using namespace rtl;
using namespace ucb;

//-----------------------------------------------------------------------------

struct NamePair_Impl
{
    OUString maShortName;
    OUString maLongName;
};

DECLARE_LIST( NameList_Impl, NamePair_Impl* );

//-----------------------------------------------------------------------------

class SfxDocTplService_Impl
{
    Reference< XMultiServiceFactory >   mxFactory;

    Reference< XPersist >       mxInfo;
    Sequence< OUString >        maTemplateDirs;
    NameList_Impl               maNames;
    Locale                      maLocale;
    Content                     maRootContent;
    sal_Bool                    mbIsInitialized : 1;
    sal_Bool                    mbLocaleSet     : 1;


    void                        init_Impl();
    void                        getDefaultLocale();
    void                        getDirList();
    void                        readFolderList();
    OUString                    getLongName( const OUString& rShortName );
    void                        getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType );

    void                        getFolders( Content& rRoot,
                                            Content& rFolder );
    void                        getTemplates( Content& rTargetFolder,
                                              Content& rParentFolder );
    void                        addEntry( Content& rParentFolder,
                                          const OUString& rTitle,
                                          const OUString& rTargetURL );
    void                        addToStandard( Content& rRoot,
                                               Content& rFolder );
public:
                                 SfxDocTplService_Impl( Reference< XMultiServiceFactory > xFactory );
                                ~SfxDocTplService_Impl();

    void                        init() { if ( !mbIsInitialized ) init_Impl(); }
    Content                     getContent();

    void                        setLocale( const LOCALE & rLocale );
    Locale                      getLocale();

    void                        update();
};


//-----------------------------------------------------------------------------
// private SfxDocTplService_Impl
//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::init_Impl()
{
    if ( !mbLocaleSet )
        getDefaultLocale();

    // convert locale to string
    OUString aLang = maLocale.Language;
    aLang += OUString( '-' );
    aLang += maLocale.Country;

    // set maRootContent to the root of the templates hierarchy. Create the
    // entry if necessary

    Reference < XCommandEnvironment > aCmdEnv;
    OUString aRootURL( RTL_CONSTASCII_USTRINGPARAM( TEMPLATE_ROOT_URL ) );
    aRootURL += OUString( '/' );
    aRootURL += aLang;

    if ( Content::create( aRootURL, aCmdEnv, maRootContent ) )
    {
        mbIsInitialized = sal_True;
    }
    else
    {
        try
        {
            Sequence< OUString > aNames(2);
            OUString* pNames = aNames.getArray();
            pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
            pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

            Sequence< Any > aValues(2);
            Any* pValues = aValues.getArray();
            pValues[1] = makeAny( sal_Bool( sal_True ) );

            OUString aType( RTL_CONSTASCII_USTRINGPARAM( TYPE_FOLDER ) );
            OUString aLangRootURL( RTL_CONSTASCII_USTRINGPARAM( HIERARCHIE_ROOT_URL ) );
            aLangRootURL += OUString( RTL_CONSTASCII_USTRINGPARAM( TEMPLATE_DIR_NAME ) );

            Content aLangRoot;

            if ( ! Content::create( aLangRootURL, aCmdEnv, aLangRoot ) )
            {
                OUString aHierRootURL( RTL_CONSTASCII_USTRINGPARAM( HIERARCHIE_ROOT_URL ) );
                Content aHierRoot( aHierRootURL, aCmdEnv );

                pValues[0] = makeAny( OUString( RTL_CONSTASCII_USTRINGPARAM( TEMPLATE_DIR_NAME ) ) );
                aHierRoot.insertNewContent( aType, aNames, aValues, aLangRoot );
            }

            pValues[0] = makeAny( aLang );

            aLangRoot.insertNewContent( aType, aNames, aValues, maRootContent );
            mbIsInitialized = sal_True;
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "Init_Impl(): Could not create hierarchy entry" );
        }
        catch( RuntimeException& )
        {
            DBG_ERRORFILE( "Init_Impl(): got runtime exception" );
        }
    }

    if ( mbIsInitialized )
    {
        OUString aService( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_DOCINFO ) );
        Reference< XPersist > xInfo( mxFactory->createInstance( aService ), UNO_QUERY );
        mxInfo = xInfo;

        getDirList();
        readFolderList();
        update();
    }
}
//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::getDefaultLocale()
{
    AllSettings aSettings;

    maLocale    = aSettings.GetLocale();
    mbLocaleSet = sal_True;
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::readFolderList()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
OUString SfxDocTplService_Impl::getLongName( const OUString& rShortName )
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

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::getDirList()
{
    OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( PROPERTY_DIRLIST ) );
    sal_Bool bHasProperty = sal_False;
    Any      aValue;

    // Get the template dir list
    try
    {
        Reference< XPropertySetInfo > aPropSet = maRootContent.getProperties();

        if ( aPropSet.is() && aPropSet->hasPropertyByName( aPropName ) )
        {
            aValue = maRootContent.getPropertyValue( aPropName );
            aValue >>= maTemplateDirs;
            bHasProperty = sal_True;
        }
    }
    catch ( CommandAbortedException& ) {}
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    if ( bHasProperty )
        return;

    INetURLObject   aURL;
    String          aDirs = SvtPathOptions().GetTemplatePath();
    USHORT          nCount = aDirs.GetTokenCount( C_DELIM );

    maTemplateDirs = Sequence< OUString >( nCount );

    OUString* pDirs = maTemplateDirs.getArray();

    for ( USHORT i=0; i<nCount; i++ )
    {
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetURL( aDirs.GetToken( i, C_DELIM ) );
        pDirs[i] = aURL.GetMainURL();
    }

    // Store the template dir list
    try
    {
        aValue <<= maTemplateDirs;
        Reference< XPropertyContainer > xProperties( maRootContent.get(), UNO_QUERY );
        if ( xProperties.is() )
        {
            try
            {
                xProperties->addProperty( aPropName, PropertyAttribute::MAYBEVOID, aValue );
            }
            catch( PropertyExistException& ) {}
            catch( IllegalTypeException& ) { DBG_ERRORFILE( "IllegalTypeException" ); }
            catch( IllegalArgumentException& ) { DBG_ERRORFILE( "IllegalArgumentException" ); }
        }

        maRootContent.setPropertyValue( aPropName, aValue );
    }
    catch ( CommandAbortedException& ) {}
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}
}

// -----------------------------------------------------------------------

void SfxDocTplService_Impl::getFolders( Content& rRoot,
                                        Content& rFolder )
{
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(1);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );

    addToStandard( rRoot, rFolder );

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
        OUString aRootURL = rRoot.get()->getIdentifier()->getContentIdentifier();
        aRootURL += OUString( '/' );

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

                aTitle = getLongName( aTitle );

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

                Content aSubFolder( xContentAccess->queryContent(), aCmdEnv );
                getTemplates( aSubFolder, aFolder );
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

void SfxDocTplService_Impl::getTemplates( Content& rTargetFolder,
                                          Content& rParentFolder )
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

                OUString aFullTitle;
                OUString aType;
                getTitleFromURL( aId, aFullTitle, aType );

                if ( aFullTitle.len() )
                    aTitle = aFullTitle;

                addEntry( rParentFolder, aTitle, aId );
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

void SfxDocTplService_Impl::getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType )
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
void SfxDocTplService_Impl::addEntry( Content& rParentFolder,
                                      const OUString& rTitle,
                                      const OUString& rTargetURL )
{
    INetURLObject aLinkObj( rParentFolder.getURL() );
    aLinkObj.insertName( rTitle, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    OUString aLinkURL = aLinkObj.GetMainURL();

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

        try
        {
            rParentFolder.insertNewContent( aType, aNames, aValues, aLink );
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "CommandAbortedException" );
        }
    }
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::addToStandard( Content& rRoot,
                                           Content& rFolder )
{
    OUString aNewFolderURL;
    OUString aTitle = getLongName( OUString( RTL_CONSTASCII_USTRINGPARAM( STANDARD_FOLDER ) ) );
    OUString aFolderURL = rFolder.get()->getIdentifier()->getContentIdentifier();
    Content  aFolder;

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

    getTemplates( rFolder, aFolder );
}

//-----------------------------------------------------------------------------
// public SfxDocTplService_Impl
//-----------------------------------------------------------------------------

SfxDocTplService_Impl::SfxDocTplService_Impl( Reference< XMultiServiceFactory > xFactory )
{
    mxFactory       = xFactory;
    mbIsInitialized = sal_False;
    mbLocaleSet     = sal_False;
}

//-----------------------------------------------------------------------------
SfxDocTplService_Impl::~SfxDocTplService_Impl()
{
}

//-----------------------------------------------------------------------------
Locale SfxDocTplService_Impl::getLocale()
{
    if ( !mbLocaleSet )
        getDefaultLocale();

    return maLocale;
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::setLocale( const Locale &rLocale )
{
    if ( mbLocaleSet &&
         ( maLocale.Language != rLocale.Language ) &&
         ( maLocale.Country != rLocale.Country ) )
        mbIsInitialized = sal_False;

    maLocale    = rLocale;
    mbLocaleSet = sal_True;
}

//-----------------------------------------------------------------------------
Content SfxDocTplService_Impl::getContent()
{
    init();

    return maRootContent;
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::update()
{
    init();

    Reference< XCommandEnvironment > aCmdEnv;

    sal_Int32   nCount = maTemplateDirs.getLength();
    OUString*   pDirs = maTemplateDirs.getArray();
    Content     aDirContent;

    while ( nCount )
    {
        nCount--;
        if ( Content::create( pDirs[ nCount ], aCmdEnv, aDirContent ) )
            getFolders( maRootContent, aDirContent );
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

SFX_IMPL_XSERVICEINFO( SfxDocTplService, TEMPLATE_SERVICE_NAME, TEMPLATE_IMPLEMENTATION_NAME )
SFX_IMPL_SINGLEFACTORY( SfxDocTplService )

//-----------------------------------------------------------------------------
SfxDocTplService::SfxDocTplService( const Reference< XMultiServiceFactory >& xFactory )
{
    pImp = new SfxDocTplService_Impl( xFactory );
}

//-----------------------------------------------------------------------------

SfxDocTplService::~SfxDocTplService()
{
    delete pImp;
}

//-----------------------------------------------------------------------------
//--- XLocalizable ---
//-----------------------------------------------------------------------------

LOCALE SAL_CALL SfxDocTplService::getLocale()
    throw( RUNTIMEEXCEPTION )
{
    return pImp->getLocale();
}

//-----------------------------------------------------------------------------

void SAL_CALL SfxDocTplService::setLocale( const LOCALE & rLocale )
    throw( RUNTIMEEXCEPTION )
{
    pImp->setLocale( rLocale );
}

//-----------------------------------------------------------------------------
//--- XDocumentTemplates ---
//-----------------------------------------------------------------------------
Reference< XCONTENT > SAL_CALL SfxDocTplService::getContent()
    throw( RUNTIMEEXCEPTION )
{
    return pImp->getContent().get();
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::storeTemplate( const ::rtl::OUString& GroupName,
                                                   const ::rtl::OUString& TemplateName,
                                                   const Reference< XSTORABLE >& Storable )
    throw( RUNTIMEEXCEPTION )
{
    return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::addTemplate( const ::rtl::OUString& GroupName,
                                                 const ::rtl::OUString& TemplateName,
                                                 const ::rtl::OUString& SourceURL )
    throw( RUNTIMEEXCEPTION )
{
    return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::removeTemplate( const ::rtl::OUString& GroupName,
                                                    const ::rtl::OUString& TemplateName )
    throw( RUNTIMEEXCEPTION )
{
    return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::addGroup( const ::rtl::OUString& GroupName )
    throw( RUNTIMEEXCEPTION )
{
    return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::removeGroup( const ::rtl::OUString& GroupName )
    throw( RUNTIMEEXCEPTION )
{
    return sal_False;
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxDocTplService::update()
    throw( RUNTIMEEXCEPTION )
{
    pImp->update();
}

