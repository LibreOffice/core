/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "slidehack.hxx"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <curl/curl.h>

using boost::property_tree::ptree;

namespace SlideHack {
namespace {

class GroupImpl : public Group {
public:
    GroupImpl()
    {
    }
    virtual ~GroupImpl()
    {
    }

    virtual GroupMetaPtr getMetaData()
    {
        // what is this doing here ?
    }

    virtual void fetchData( bool bThumbnail, bool bODPStream )
    {
        // queue maDataComplete signal at idle
    }

    virtual void getVersions()
    {
        // queue maVersions signal at idle
    }
};

static size_t read_function( void* data, size_t item_size, size_t num_members, void* user_data )
{
    if( num_members )
    {
        std::string* pBuffer=(std::string*)user_data;
        pBuffer->append( (const char*)data, item_size*num_members );
    }
    return item_size * num_members;
}

static boost::shared_ptr<ptree> read_data( CURL* pCurl, const char* url)
{
    std::string buffer;
    curl_easy_setopt( pCurl, CURLOPT_NOPROGRESS, 1 );
    curl_easy_setopt( pCurl, CURLOPT_WRITEFUNCTION, read_function );
    curl_easy_setopt( pCurl, CURLOPT_WRITEDATA, &buffer );
    curl_easy_setopt( pCurl, CURLOPT_URL, url );
    curl_easy_setopt( pCurl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt( pCurl, CURLOPT_MAXREDIRS, 100);
    curl_easy_setopt( pCurl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt( pCurl, CURLOPT_SSL_VERIFYHOST, 0);
#if OSL_DEBUG_LEVEL > 2
    curl_easy_setopt( pCurl, CURLOPT_VERBOSE, 0);
#endif

    if( !curl_easy_perform( pCurl ) )
    {
        boost::shared_ptr<ptree> res(new boost::property_tree::ptree);
        std::istringstream stream(buffer);
        boost::property_tree::read_json(stream, *res);

        return res;
    }

    return boost::shared_ptr<ptree>();
}

class StoreImpl : public Store {
    std::vector<OString> m_userList;
    std::vector<OString> m_tagList;
    CURL* m_pCurl;

public:
    StoreImpl() :
        m_pCurl(NULL)
    {
        curl_global_init( CURL_GLOBAL_ALL );
        m_pCurl = curl_easy_init( );

        boost::shared_ptr<ptree> users = read_data( m_pCurl, "https://localhost:8080/api/users/" );
        for( ptree::const_iterator i=users->begin(); i != users->end(); ++i )
            m_userList.push_back( i->first.c_str() );

        boost::shared_ptr<ptree> tags = read_data( m_pCurl, "https://localhost:8080/api/tags/" );
        for( ptree::const_iterator i=tags->begin(); i != tags->end(); ++i )
            m_tagList.push_back( i->first.c_str() );
    }

    ~StoreImpl()
    {
        if ( NULL != m_pCurl )
            curl_easy_cleanup( m_pCurl );
    }

    virtual sal_uInt32 search( OUString aSearchEntry )
    {
        // queue maSearchCompleted at idle ...
        return 0;
    }
    virtual void  cancelSearch( sal_uInt32 nHandle ) { (void) nHandle; }

    /// used to create a group handle from a stored slide, so we can
    /// check for updated versions etc.
    virtual GroupPtr lookupGroup( OriginPtr pOrigin )
    {
        return GroupPtr();
    }
    virtual GroupPtr createGroup( const OUString &rName,
                                  const OUString &rTitle,
                                  const OUString &rKeywords,
                                  SlideHack::OriginDetails::UpdateType eType,
                                  const std::vector< SdPage * > &rPages )
    {
        GroupPtr pGroup( new GroupImpl() );
        // FIXME: really need something stored on SdPages for
        // the rendering ...
        return pGroup;
    }
};

} // end anonymous namespace

StorePtr Store::getStore()
{
    return StorePtr( new StoreImpl() );
}

} // end SlideHack namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
