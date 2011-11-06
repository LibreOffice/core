/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "layoutparse.hxx"

#define STRING( str ) String( str, RTL_TEXTENCODING_UTF8 )
#define BSTRING( str ) ByteString( str, RTL_TEXTENCODING_UTF8 )

LayoutXMLFile::LayoutXMLFile( bool mergeMode )
    : XMLFile()
    , mMergeMode( mergeMode )
{
}

void
LayoutXMLFile::SearchL10NElements( XMLParentNode* pCur, int )
{
    if ( !pCur )
        pCur = this;

    /* Recurse int children, SearchL10NElements does not do that for us.  */
    if ( XMLChildNodeList* lst = pCur->GetChildList() )
        for ( sal_uLong i = 0; i < lst->Count(); i++ )
            if ( lst->GetObject( i )->GetNodeType() == XML_NODE_TYPE_ELEMENT )
                HandleElement( ( XMLElement* )lst->GetObject( i ) );
            else if ( lst->GetObject( i )->GetNodeType() == XML_NODE_TYPE_COMMENT )
                lst->Remove( i-- );
}

std::vector<XMLAttribute*>
interestingAttributes( XMLAttributeList* lst )
{
    std::vector<XMLAttribute*> interesting;
    if ( lst )
        for ( sal_uLong i = 0; i < lst->Count(); i++ )
            if ( lst->GetObject( i )->Equals( STRING( "id" ) ) )
                interesting.insert( interesting.begin(), lst->GetObject( i ) );
            else if ( ! BSTRING( *lst->GetObject( i ) ).CompareTo( "_", 1 ) )
                interesting.push_back( lst->GetObject( i ) );
    return interesting;
}

void
LayoutXMLFile::HandleElement( XMLElement* element )
{
    std::vector<XMLAttribute*> interesting = interestingAttributes( element->GetAttributeList() );

    if ( interesting.size() )
    {
        std::vector<XMLAttribute*>::iterator i = interesting.begin();

        ByteString id = BSTRING( (*i++)->GetValue() );

        if ( mMergeMode )
            InsertL10NElement( id, element );
        else
            for ( ; i != interesting.end(); ++i )
            {
                ByteString attributeId = id;
                ByteString value = BSTRING( ( *i )->GetValue() );
                XMLElement *e = new XMLElement( *element );
                e->RemoveAndDeleteAllChilds();
                /* Copy translatable text to CONTENT. */
                //new XMLData( STRING( ( *i )->GetValue() ), e, true );
                new XMLData( STRING( value ), e, true );
                attributeId += BSTRING ( **i );
                InsertL10NElement( attributeId, e );
            }
    }

    SearchL10NElements( (XMLParentNode*) element );
}

void LayoutXMLFile::InsertL10NElement( ByteString const& id, XMLElement* element )
{
    ByteString const language = "en-US";
    LangHashMap* languageMap = 0;
    XMLHashMap::iterator pos = XMLStrings->find( id );
    if ( pos != XMLStrings->end() )
    {
        languageMap = pos->second;
        fprintf( stderr, "error:%s:duplicate translation found, id=%s\n",
                 id.GetBuffer(), BSTRING( sFileName ).GetBuffer() );
        exit( 1 );
    }
    else
    {
        languageMap = new LangHashMap();
        XMLStrings->insert( XMLHashMap::value_type( id , languageMap ) );
        order.push_back( id );
    }
    (*languageMap)[ language ] = element;
}

sal_Bool LayoutXMLFile::Write( ByteString &aFilename )
{

    if ( aFilename.Len() )
    {
        ofstream aFStream( aFilename.GetBuffer() , ios::out | ios::trunc );
        if ( !aFStream )
            fprintf( stderr, "ERROR: cannot open file:%s\n", aFilename.GetBuffer() );
        else
        {
            XMLFile::Write( aFStream );
            aFStream.close();
            return true;
        }
    }
    return false;
}
