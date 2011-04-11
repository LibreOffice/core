/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
        for ( size_t i = 0; i < lst->size(); i++ )
            if ( (*lst)[ i ]->GetNodeType() == XML_NODE_TYPE_ELEMENT )
                HandleElement( ( XMLElement* )(*lst)[ i ] );
            else if ( (*lst)[ i ]->GetNodeType() == XML_NODE_TYPE_COMMENT ) {
                XMLChildNodeList::iterator it = lst->begin();
                ::std::advance( it, i );
                lst->erase( it );
                i--;
            }
}

std::vector<XMLAttribute*>
interestingAttributes( XMLAttributeList* lst )
{
    std::vector<XMLAttribute*> interesting;
    if ( lst )
        for ( size_t i = 0; i < lst->size(); i++ )
            if ( (*lst)[ i ]->Equals( STRING( "id" ) ) )
                interesting.insert( interesting.begin(), (*lst)[ i ] );
            else if ( ! BSTRING( *(*lst)[ i ]).CompareTo( "_", 1 ) )
                interesting.push_back( (*lst)[ i ] );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
