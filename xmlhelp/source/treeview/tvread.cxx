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

#include <string.h>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <tvread.hxx>
#include <expat.h>
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/deployment/thePackageManagerFactory.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrl.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/pathoptions.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <memory>
#include <utility>

namespace treeview {

    class TVDom
    {
        friend class TVChildTarget;
        friend class TVRead;

    public:

        explicit TVDom( TVDom* arent = nullptr )
            : kind( Kind::other ),
              parent( arent ),
              children( 0 )
        {
        }

        TVDom* newChild()
        {
            children.emplace_back( new TVDom( this ) );
            return children.back().get();
        }

        void newChild(std::unique_ptr<TVDom> p)
        {
            children.emplace_back(std::move(p));
            children.back()->parent = this;
        }

        TVDom* getParent() const
        {
            if( parent )
                return parent;
            else
                return const_cast<TVDom*>(this);    // I am my own parent, if I am the root
        }

        enum class Kind {
            tree_node,
            tree_leaf,
            other
        };

        bool isLeaf() const { return kind == TVDom::Kind::tree_leaf; }
        void setKind( Kind ind ) { kind = ind; }

        void setApplication( const char* appl )
        {
            application = OUString( appl,
                                         strlen( appl ),
                                         RTL_TEXTENCODING_UTF8 );
        }

        void setTitle( const char* itle )
        {
            title += OUString( itle,
                                    strlen( itle ),
                                    RTL_TEXTENCODING_UTF8 );
        }

        void setTitle( const XML_Char* itle,int len )
        {
            title += OUString( itle,
                                    len,
                                    RTL_TEXTENCODING_UTF8 );
        }

        void setId( const char* d )
        {
            id = OUString( d,
                                strlen( d ),
                                RTL_TEXTENCODING_UTF8 );
        }

        void setAnchor( const char* nchor )
        {
            anchor = OUString( nchor,
                                    strlen( nchor ),
                                    RTL_TEXTENCODING_UTF8 );
        }

        OUString const & getTargetURL()
        {
            if( targetURL.isEmpty() )
            {
                targetURL = "vnd.sun.star.help://" + id;
            }

            return targetURL;
        }

    private:

        Kind   kind;
        OUString  application;
        OUString  title;
        OUString  id;
        OUString  anchor;
        OUString  targetURL;

        TVDom *parent;
        std::vector< std::unique_ptr<TVDom> > children;
    };

}

using namespace treeview;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::configuration;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::deployment;

constexpr OUStringLiteral prodName = u"%PRODUCTNAME";
constexpr OUStringLiteral vendName = u"%VENDORNAME";
constexpr OUStringLiteral vendVersion = u"%VENDORVERSION";
constexpr OUStringLiteral vendShort = u"%VENDORSHORT";
constexpr OUStringLiteral prodVersion = u"%PRODUCTVERSION";

ConfigData::ConfigData()
{
}

void ConfigData::replaceName( OUString& oustring ) const
{
    sal_Int32 idx = -1,k = 0,off;
    bool cap = false;
    OUStringBuffer aStrBuf( 0 );

    while( ( idx = oustring.indexOf( '%', ++idx ) ) != -1 )
    {
        if( oustring.indexOf( prodName,idx ) == idx )
            off = PRODUCTNAME;
        else if( oustring.indexOf( prodVersion,idx ) == idx )
            off = PRODUCTVERSION;
        else if( oustring.indexOf( vendName,idx ) == idx )
            off = VENDORNAME;
        else if( oustring.indexOf( vendVersion,idx ) == idx )
            off = VENDORVERSION;
        else if( oustring.indexOf( vendShort,idx ) == idx )
            off = VENDORSHORT;
        else
            off = -1;

        if( off != -1 )
        {
            if( ! cap )
            {
                cap = true;
                aStrBuf.ensureCapacity( 256 );
            }

            aStrBuf.append( &oustring.getStr()[k],idx - k );
            aStrBuf.append( m_vReplacement[off] );
            k = idx + m_vAdd[off];
        }
    }

    if( cap )
    {
        if( k < oustring.getLength() )
            aStrBuf.append( &oustring.getStr()[k],oustring.getLength()-k );
        oustring = aStrBuf.makeStringAndClear();
    }
}

// TVRead

TVRead::TVRead( const ConfigData& configData,TVDom* tvDom )
{
    if( ! tvDom )
        return;

    Title = tvDom->title;
    configData.replaceName( Title );
    if( tvDom->isLeaf() )
    {
        TargetURL = tvDom->getTargetURL() + configData.appendix;
        if( !tvDom->anchor.isEmpty() )
            TargetURL += "#" + tvDom->anchor;
    }
    else
        Children = new TVChildTarget( configData,tvDom );
}

TVRead::~TVRead()
{
}

// XNameAccess

Any SAL_CALL
TVRead::getByName( const OUString& aName )
{
    bool found( true );
    Any aAny;
    if( aName == "Title" )
        aAny <<= Title;
    else if( aName == "TargetURL" )
        aAny <<= TargetURL;
    else if( aName == "Children" )
    {
        cppu::OWeakObject* p = Children.get();
        aAny <<= Reference< XInterface >( p );
    }
    else
        found = false;

    if( found )
        return aAny;

    throw NoSuchElementException();
}

Sequence< OUString > SAL_CALL
TVRead::getElementNames( )
{
    return { u"Title"_ustr, u"TargetURL"_ustr, u"Children"_ustr };
}

sal_Bool SAL_CALL
TVRead::hasByName( const OUString& aName )
{
    if( aName == "Title"     ||
        aName == "TargetURL" ||
        aName == "Children" )
        return true;

    return false;
}

// XHierarchicalNameAccess

Any SAL_CALL
TVRead::getByHierarchicalName( const OUString& aName )
{
    OUString aRest;
    if( aName.startsWith("Children/", &aRest) )
        return Children->getByHierarchicalName( aRest );

    return getByName( aName );
}

sal_Bool SAL_CALL
TVRead::hasByHierarchicalName( const OUString& aName )
{
    OUString aRest;
    if( aName.startsWith("Children/", &aRest) )
        return Children->hasByHierarchicalName( aRest );

    return hasByName( aName );
}

/**************************************************************************/
/*                                                                        */
/*                      TVChildTarget                                     */
/*                                                                        */
/**************************************************************************/

extern "C" {

static void start_handler(void *userData,
                   const XML_Char *name,
                   const XML_Char **atts)
{
    TVDom::Kind kind;

    if( strcmp( name,"help_section" ) == 0  ||
        strcmp( name,"node" ) == 0 )
        kind = TVDom::Kind::tree_node;
    else if( strcmp( name,"topic" ) == 0 )
        kind = TVDom::Kind::tree_leaf;
    else
        return;

    TVDom **tvDom = static_cast< TVDom** >( userData );
    TVDom  *p;
    p = *tvDom;

    *tvDom = p->newChild();
    p = *tvDom;

    p->setKind( kind );
    while( *atts )
    {
        if( strcmp( *atts,"application" ) == 0 )
            p->setApplication( *(atts+1) );
        else if( strcmp( *atts,"title" ) == 0 )
            p->setTitle( *(atts+1) );
        else if( strcmp( *atts,"id" ) == 0 )
            p->setId( *(atts+1) );
        else if( strcmp( *atts,"anchor" ) == 0 )
            p->setAnchor( *(atts+1) );

        atts+=2;
    }
}

static void end_handler(void *userData,
                 SAL_UNUSED_PARAMETER const XML_Char * )
{
    TVDom **tvDom = static_cast< TVDom** >( userData );
    *tvDom = (*tvDom)->getParent();
}

static void data_handler( void *userData,
                   const XML_Char *s,
                   int len)
{
    TVDom **tvDom = static_cast< TVDom** >( userData );
    if( (*tvDom)->isLeaf() )
        (*tvDom)->setTitle( s,len );
}

}

TVChildTarget::TVChildTarget( const ConfigData& configData,TVDom* tvDom )
{
    Elements.resize( tvDom->children.size() );
    for( size_t i = 0; i < Elements.size(); ++i )
        Elements[i] = new TVRead( configData,tvDom->children[i].get() );
}

TVChildTarget::TVChildTarget()
{
    ConfigData configData = init();

    if( configData.locale.isEmpty() || configData.system.isEmpty() )
        return;

    sal_uInt64  ret,len = 0;
    int j = configData.vFileURL.size();

    TVDom tvDom;
    TVDom* pTVDom = &tvDom;

    while( j )
    {
        len = configData.vFileLen[--j];
        std::unique_ptr<char[]> s(new char[ int(len) ]);  // the buffer to hold the installed files
        osl::File aFile( configData.vFileURL[j] );
        (void)aFile.open( osl_File_OpenFlag_Read );
        aFile.read( s.get(),len,ret );
        aFile.close();

        XML_Parser parser = XML_ParserCreate( nullptr );
        XML_SetElementHandler( parser,
                               start_handler,
                               end_handler );
        XML_SetCharacterDataHandler( parser,
                                     data_handler);
        XML_SetUserData( parser,&pTVDom ); // does not return this

        XML_Status const parsed = XML_Parse(parser, s.get(), int(len), j==0);
        SAL_WARN_IF(XML_STATUS_ERROR == parsed, "xmlhelp",
                "TVChildTarget::TVChildTarget(): Tree file parsing failed");

        XML_ParserFree( parser );

        Check(pTVDom);
    }
    // now TVDom holds the relevant information

    Elements.resize( tvDom.children.size() );
    for( size_t i = 0; i < Elements.size(); ++i )
        Elements[i] = new TVRead( configData,tvDom.children[i].get() );
}

TVChildTarget::~TVChildTarget()
{
}

void TVChildTarget::Check(TVDom* tvDom)
{
    if (tvDom->children.empty())
    {
        return;
    }

    unsigned i = 0;
    bool h = false;

    while((i<tvDom->children.size()-1) && (!h))
    {
        if (((tvDom->children[i])->application == (tvDom->children[tvDom->children.size()-1])->application) &&
            ((tvDom->children[i])->id == (tvDom->children[tvDom->children.size()-1])->id))
        {
            TVDom* p = tvDom->children.back().get();

            for(auto & k : p->children)
            {
                std::unique_ptr<TVDom> tmp(SearchAndInsert(std::move(k), tvDom->children[i].get()));
                if (tmp)
                {
                    tvDom->children[i]->newChild(std::move(tmp));
                }
            }

            tvDom->children.pop_back();
            h = true;
        }
        ++i;
    }
}

std::unique_ptr<TVDom>
TVChildTarget::SearchAndInsert(std::unique_ptr<TVDom> p, TVDom* tvDom)
{
    if (p->isLeaf()) return p;

    bool h = false;
    sal_Int32 max = 0;

    std::vector< std::unique_ptr<TVDom> >::iterator max_It, i;
    max_It = tvDom->children.begin();

    sal_Int32 c_int;
    sal_Int32 p_int = p->id.toInt32();

    for(i = tvDom->children.begin(); i!=tvDom->children.end(); ++i)
        if (!((*i)->isLeaf()) &&
            ((*i)->id.getLength() == p->id.getLength()) &&
            (p->id.replaceAt((*i)->parent->id.getLength(), p->id.getLength()-(*i)->parent->id.getLength(), u"") == (*i)->parent->id))      //prefix check
        {
            h = true;
            c_int = (*i)->id.toInt32();

            if (p_int==c_int)
            {
                (*(tvDom->children.insert(i+1, std::move(p))))->parent = tvDom;
                return nullptr;
            }
            else if(c_int>max && c_int < p_int)
            {
                max = c_int;
                max_It = i+1;
            }
        }
    if (h)
    {
        (*(tvDom->children.insert(max_It, std::move(p))))->parent = tvDom;
        return nullptr;
    }
    else
    {
        for (auto& child : tvDom->children)
        {
            p = SearchAndInsert(std::move(p), child.get());
            if (p == nullptr)
                break;
        }
        return p;
    }
}

Any SAL_CALL
TVChildTarget::getByName( const OUString& aName )
{
    std::u16string_view num( aName.subView( 2, aName.getLength()-4 ) );
    sal_Int32 idx = o3tl::toInt32(num) - 1;
    if( idx < 0 || Elements.size() <= o3tl::make_unsigned( idx ) )
        throw NoSuchElementException();

    cppu::OWeakObject* p = Elements[idx].get();
    return Any( Reference< XInterface >( p ) );
}

Sequence< OUString > SAL_CALL
TVChildTarget::getElementNames( )
{
    Sequence< OUString > seq( Elements.size() );
    auto seqRange = asNonConstRange(seq);
    for( size_t i = 0; i < Elements.size(); ++i )
        seqRange[i] = OUString::number( 1+i );

    return seq;
}

sal_Bool SAL_CALL
TVChildTarget::hasByName( const OUString& aName )
{
    std::u16string_view num( aName.subView( 2, aName.getLength()-4 ) );
    sal_Int32 idx = o3tl::toInt32(num) - 1;
    if( idx < 0 || Elements.size() <= o3tl::make_unsigned( idx ) )
        return false;

    return true;
}

// XHierarchicalNameAccess

Any SAL_CALL
TVChildTarget::getByHierarchicalName( const OUString& aName )
{
    sal_Int32 idx;

    if( ( idx = aName.indexOf( '/' ) ) != -1 )
    {
        std::u16string_view num( aName.subView( 2, idx-4 ) );
        sal_Int32 pref = o3tl::toInt32(num) - 1;

        if( pref < 0 || Elements.size() <= o3tl::make_unsigned( pref ) )
            throw NoSuchElementException();

        return Elements[pref]->getByHierarchicalName( aName.copy( 1 + idx ) );
    }
    else
        return getByName( aName );
}

sal_Bool SAL_CALL
TVChildTarget::hasByHierarchicalName( const OUString& aName )
{
    sal_Int32 idx;

    if( ( idx = aName.indexOf( '/' ) ) != -1 )
    {
        std::u16string_view num( aName.subView( 2, idx-4 ) );
        sal_Int32 pref = o3tl::toInt32(num) - 1;
        if( pref < 0 || Elements.size() <= o3tl::make_unsigned( pref ) )
            return false;

        return Elements[pref]->hasByHierarchicalName( aName.copy( 1 + idx ) );
    }
    else
        return hasByName( aName );
}

ConfigData TVChildTarget::init()
{
    ConfigData configData;

    /**********************************************************************/
    /*                       reading Office.Common                        */
    /**********************************************************************/

    configData.system = officecfg::Office::Common::Help::System::get();
    bool showBasic = officecfg::Office::Common::Help::ShowBasic::get();
    OUString instPath = officecfg::Office::Common::Path::Current::Help::get();
    if( instPath.isEmpty() )
      // try to determine path from default
      instPath = "$(instpath)/help";

    // replace anything like $(instpath);
    subst( instPath );

    /**********************************************************************/
    /*                       reading setup                                */
    /**********************************************************************/

    OUString setupversion = officecfg::Setup::Product::ooSetupVersion::get();
    OUString setupextension = officecfg::Setup::Product::ooSetupExtension::get();

    configData.m_vReplacement[0] = utl::ConfigManager::getProductName();
    configData.m_vReplacement[1] = setupversion + " " + setupextension; // productVersion
    // m_vReplacement[2...4] (vendorName/-Version/-Short) are empty strings

    configData.locale = officecfg::Setup::L10N::ooLocale::get();

    // Determine fileurl from url and locale
    OUString url;
    osl::FileBase::RC errFile = osl::FileBase::getFileURLFromSystemPath( instPath,url );
    if( errFile != osl::FileBase::E_None ) return configData;
    if( !url.endsWith("/") )
        url += "/";
    OUString ret;
    sal_Int32 idx;
    osl::DirectoryItem aDirItem;
    if( osl::FileBase::E_None == osl::DirectoryItem::get( url + configData.locale, aDirItem ) )
        ret = configData.locale;
    else if( ( ( idx = configData.locale.indexOf( '-' ) ) != -1 ||
               ( idx = configData.locale.indexOf( '_' ) ) != -1 ) &&
             osl::FileBase::E_None == osl::DirectoryItem::get( url + configData.locale.subView( 0,idx ),
                                                               aDirItem ) )
        ret = configData.locale.copy( 0,idx );
    else
    {
        configData.locale= "en-US";
        ret = "en";
    }
    url += ret;

    // first of all, try do determine whether there are any *.tree files present

    // Start with extensions to set them at the end of the list
    TreeFileIterator aTreeIt(configData.locale);
    OUString aTreeFile;
    sal_Int32 nFileSize;
    for (;;)
    {
        aTreeFile = aTreeIt.nextTreeFile( nFileSize );
        if( aTreeFile.isEmpty() )
            break;
        configData.vFileLen.push_back( nFileSize );
        configData.vFileURL.push_back( aTreeFile );
    }

    osl::Directory aDirectory( url );
    osl::FileStatus aFileStatus(
            osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_FileURL );
    if( osl::Directory::E_None == aDirectory.open() )
    {
        OUString aFileUrl, aFileName;
        while( aDirectory.getNextItem( aDirItem ) == osl::FileBase::E_None &&
               aDirItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None &&
               aFileStatus.isValid( osl_FileStatus_Mask_FileURL ) &&
               aFileStatus.isValid( osl_FileStatus_Mask_FileName ) )
        {
            aFileUrl = aFileStatus.getFileURL();
            aFileName = aFileStatus.getFileName();
            int idx_ = aFileName.lastIndexOf( '.' );
            if( idx_ == -1 )
              continue;

            const sal_Unicode* str = aFileName.getStr();

            if( aFileName.getLength() == idx_ + 5                   &&
                ( str[idx_ + 1] == 't' || str[idx_ + 1] == 'T' )    &&
                ( str[idx_ + 2] == 'r' || str[idx_ + 2] == 'R' )    &&
                ( str[idx_ + 3] == 'e' || str[idx_ + 3] == 'E' )    &&
                ( str[idx_ + 4] == 'e' || str[idx_ + 4] == 'E' ) )
            {
                OUString baseName = aFileName.copy(0,idx_).toAsciiLowerCase();
                if(! showBasic && baseName == "sbasic" )
                  continue;
                osl::File aFile( aFileUrl );
                if( osl::FileBase::E_None == aFile.open( osl_File_OpenFlag_Read ) )
                {
                    // use the file size, not aFileStatus size, in case the
                    // tree file is a symlink
                    sal_uInt64 nSize;
                    aFile.getSize( nSize );
                    configData.vFileLen.push_back( nSize );
                    configData.vFileURL.push_back( aFileUrl );
                    aFile.close();
                }
            }
        }
        aDirectory.close();
    }

    configData.m_vAdd[0] = 12;
    configData.m_vAdd[1] = 15;
    configData.m_vAdd[2] = 11;
    configData.m_vAdd[3] = 14;
    configData.m_vAdd[4] = 12;

    configData.appendix =
        "?Language=" +
        configData.locale +
        "&System=" +
        configData.system +
        "&UseDB=no";

    return configData;
}

void TVChildTarget::subst( OUString& instpath )
{
    SvtPathOptions aOptions;
    instpath = aOptions.SubstituteVariable( instpath );
}


const char aHelpMediaType[] = "application/vnd.sun.star.help";

TreeFileIterator::TreeFileIterator( OUString aLanguage )
        : m_eState( IteratorState::UserExtensions )
        , m_aLanguage(std::move( aLanguage ))
{
    m_xContext = ::comphelper::getProcessComponentContext();
    if( !m_xContext.is() )
    {
        throw RuntimeException( u"TreeFileIterator::TreeFileIterator(), no XComponentContext"_ustr );
    }

    m_xSFA = ucb::SimpleFileAccess::create(m_xContext);

    m_bUserPackagesLoaded = false;
    m_bSharedPackagesLoaded = false;
    m_bBundledPackagesLoaded = false;
    m_iUserPackage = 0;
    m_iSharedPackage = 0;
    m_iBundledPackage = 0;
}

Reference< deployment::XPackage > TreeFileIterator::implGetHelpPackageFromPackage
    ( const Reference< deployment::XPackage >& xPackage, Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    o_xParentPackageBundle.clear();

    Reference< deployment::XPackage > xHelpPackage;
    if( !xPackage.is() )
        return xHelpPackage;

    // Check if parent package is registered
    beans::Optional< beans::Ambiguous<sal_Bool> > option( xPackage->isRegistered
        ( Reference<task::XAbortChannel>(), Reference<ucb::XCommandEnvironment>() ) );
    bool bRegistered = false;
    if( option.IsPresent )
    {
        beans::Ambiguous<sal_Bool> const & reg = option.Value;
        if( !reg.IsAmbiguous && reg.Value )
            bRegistered = true;
    }
    if( !bRegistered )
        return xHelpPackage;

    if( xPackage->isBundle() )
    {
        const Sequence< Reference< deployment::XPackage > > aPkgSeq = xPackage->getBundle
            ( Reference<task::XAbortChannel>(), Reference<ucb::XCommandEnvironment>() );
        auto pSubPkg = std::find_if(aPkgSeq.begin(), aPkgSeq.end(),
            [](const Reference< deployment::XPackage >& xSubPkg) {
                const Reference< deployment::XPackageTypeInfo > xPackageTypeInfo = xSubPkg->getPackageType();
                OUString aMediaType = xPackageTypeInfo->getMediaType();
                return aMediaType == aHelpMediaType;
            });
        if (pSubPkg != aPkgSeq.end())
        {
            xHelpPackage = *pSubPkg;
            o_xParentPackageBundle = xPackage;
        }
    }
    else
    {
        const Reference< deployment::XPackageTypeInfo > xPackageTypeInfo = xPackage->getPackageType();
        OUString aMediaType = xPackageTypeInfo->getMediaType();
        if( aMediaType == aHelpMediaType )
            xHelpPackage = xPackage;
    }

    return xHelpPackage;
}

Reference< deployment::XPackage > TreeFileIterator::implGetNextUserHelpPackage
    ( Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    Reference< deployment::XPackage > xHelpPackage;

    if( !m_bUserPackagesLoaded )
    {
        Reference< XPackageManager > xUserManager =
            thePackageManagerFactory::get( m_xContext )->getPackageManager(u"user"_ustr);
        m_aUserPackagesSeq = xUserManager->getDeployedPackages
            ( Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );

        m_bUserPackagesLoaded = true;
    }

    if( m_iUserPackage == m_aUserPackagesSeq.getLength() )
    {
        m_eState = IteratorState::SharedExtensions;       // Later: SHARED_MODULE
    }
    else
    {
        const Reference< deployment::XPackage >* pUserPackages = m_aUserPackagesSeq.getConstArray();
        Reference< deployment::XPackage > xPackage = pUserPackages[ m_iUserPackage++ ];
        OSL_ENSURE( xPackage.is(), "TreeFileIterator::implGetNextUserHelpPackage(): Invalid package" );
        xHelpPackage = implGetHelpPackageFromPackage( xPackage, o_xParentPackageBundle );
    }

    return xHelpPackage;
}

Reference< deployment::XPackage > TreeFileIterator::implGetNextSharedHelpPackage
    ( Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    Reference< deployment::XPackage > xHelpPackage;

    if( !m_bSharedPackagesLoaded )
    {
        Reference< XPackageManager > xSharedManager =
            thePackageManagerFactory::get( m_xContext )->getPackageManager(u"shared"_ustr);
        m_aSharedPackagesSeq = xSharedManager->getDeployedPackages
            ( Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );

        m_bSharedPackagesLoaded = true;
    }

    if( m_iSharedPackage == m_aSharedPackagesSeq.getLength() )
    {
        m_eState = IteratorState::BundledExtensions;
    }
    else
    {
        const Reference< deployment::XPackage >* pSharedPackages = m_aSharedPackagesSeq.getConstArray();
        Reference< deployment::XPackage > xPackage = pSharedPackages[ m_iSharedPackage++ ];
        OSL_ENSURE( xPackage.is(), "TreeFileIterator::implGetNextSharedHelpPackage(): Invalid package" );
        xHelpPackage = implGetHelpPackageFromPackage( xPackage, o_xParentPackageBundle );
    }

    return xHelpPackage;
}

Reference< deployment::XPackage > TreeFileIterator::implGetNextBundledHelpPackage
    ( Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    Reference< deployment::XPackage > xHelpPackage;

    if( !m_bBundledPackagesLoaded )
    {
        Reference< XPackageManager > xBundledManager =
            thePackageManagerFactory::get( m_xContext )->getPackageManager(u"bundled"_ustr);
        m_aBundledPackagesSeq = xBundledManager->getDeployedPackages
            ( Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );

        m_bBundledPackagesLoaded = true;
    }

    if( m_iBundledPackage == m_aBundledPackagesSeq.getLength() )
    {
        m_eState = IteratorState::EndReached;
    }
    else
    {
        const Reference< deployment::XPackage >* pBundledPackages = m_aBundledPackagesSeq.getConstArray();
        Reference< deployment::XPackage > xPackage = pBundledPackages[ m_iBundledPackage++ ];
        OSL_ENSURE( xPackage.is(), "TreeFileIterator::implGetNextBundledHelpPackage(): Invalid package" );
        xHelpPackage = implGetHelpPackageFromPackage( xPackage, o_xParentPackageBundle );
    }

    return xHelpPackage;
}

static bool isLetter( sal_Unicode c )
{
    return rtl::isAsciiAlpha(c);
}

void TreeFileIterator::implGetLanguageVectorFromPackage( ::std::vector< OUString > &rv,
    const css::uno::Reference< css::deployment::XPackage >& xPackage )
{
    rv.clear();
    OUString aExtensionPath = xPackage->getURL();
    const Sequence< OUString > aEntrySeq = m_xSFA->getFolderContents( aExtensionPath, true );

    for( const OUString& aEntry : aEntrySeq )
    {
        if( m_xSFA->isFolder( aEntry ) )
        {
            sal_Int32 nLastSlash = aEntry.lastIndexOf( '/' );
            if( nLastSlash != -1 )
            {
                OUString aPureEntry = aEntry.copy( nLastSlash + 1 );

                // Check language scheme
                int nLen = aPureEntry.getLength();
                const sal_Unicode* pc = aPureEntry.getStr();
                bool bStartCanBeLanguage = ( nLen >= 2 && isLetter( pc[0] ) && isLetter( pc[1] ) );
                bool bIsLanguage = bStartCanBeLanguage &&
                    ( nLen == 2 || (nLen == 5 && pc[2] == '-' && isLetter( pc[3] ) && isLetter( pc[4] )) );
                if( bIsLanguage )
                    rv.push_back( aPureEntry );
            }
        }
    }
}


OUString TreeFileIterator::nextTreeFile( sal_Int32& rnFileSize )
{
    OUString aRetFile;

    while( aRetFile.isEmpty() && m_eState != IteratorState::EndReached )
    {
        switch( m_eState )
        {
            case IteratorState::UserExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextUserHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetTreeFileFromPackage( rnFileSize, xHelpPackage );
                break;
            }

            case IteratorState::SharedExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextSharedHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetTreeFileFromPackage( rnFileSize, xHelpPackage );
                break;
            }
            case IteratorState::BundledExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextBundledHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetTreeFileFromPackage( rnFileSize, xHelpPackage );
                break;
            }

        case IteratorState::EndReached:
                OSL_FAIL( "DataBaseIterator::nextTreeFile(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return aRetFile;
}

OUString TreeFileIterator::expandURL( const OUString& aURL )
{
    static Reference< util::XMacroExpander > xMacroExpander;
    static Reference< uri::XUriReferenceFactory > xFac;

    std::scoped_lock aGuard( m_aMutex );

    if( !xMacroExpander.is() || !xFac.is() )
    {
        xFac = uri::UriReferenceFactory::create( m_xContext );

        xMacroExpander = util::theMacroExpander::get(m_xContext);
    }

    OUString aRetURL = aURL;
    Reference< uri::XUriReference > uriRef;
    for (;;)
    {
        uriRef = xFac->parse( aRetURL );
        if ( uriRef.is() )
        {
            Reference < uri::XVndSunStarExpandUrl > sxUri( uriRef, UNO_QUERY );
            if( !sxUri.is() )
                break;

            aRetURL = sxUri->expand( xMacroExpander );
        }
    }
    return aRetURL;
}

OUString TreeFileIterator::implGetTreeFileFromPackage
    ( sal_Int32& rnFileSize, const Reference< deployment::XPackage >& xPackage )
{
    OUString aRetFile;
    OUString aLanguage = m_aLanguage;
    for( sal_Int32 iPass = 0 ; iPass < 2 ; ++iPass )
    {
        aRetFile = expandURL( xPackage->getURL() + "/" + aLanguage + "/help.tree" );
        if( iPass == 0 )
        {
            if( m_xSFA->exists( aRetFile ) )
                break;

            ::std::vector< OUString > av;
            implGetLanguageVectorFromPackage( av, xPackage );
            ::std::vector< OUString >::const_iterator pFound = LanguageTag::getFallback( av, m_aLanguage );
            if( pFound != av.end() )
                aLanguage = *pFound;
        }
    }

    rnFileSize = 0;
    if( m_xSFA->exists( aRetFile ) )
        rnFileSize = m_xSFA->getSize( aRetFile );
    else
        aRetFile.clear();

    return aRetFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
