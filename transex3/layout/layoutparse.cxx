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
        for ( ULONG i = 0; i < lst->Count(); i++ )
            if ( lst->GetObject( i )->GetNodeType() == XML_NODE_TYPE_ELEMENT )
                HandleElement( ( XMLElement* )lst->GetObject( i ) );
}

std::vector<XMLAttribute*>
interestingAttributes( XMLAttributeList* lst )
{
    std::vector<XMLAttribute*> interesting;
    if ( lst )
        for ( ULONG i = 0; i < lst->Count(); i++ )
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
        ByteString id = BSTRING( interesting[0]->GetValue() );

        if ( mMergeMode )
            InsertL10NElement( id, element );
        else
            for ( std::vector<XMLAttribute*>::iterator i = ++interesting.begin();
                  i != interesting.end(); ++i )
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

BOOL LayoutXMLFile::Write( ByteString &aFilename )
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
