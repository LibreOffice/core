#include <com/sun/star/xml/sax/SAXException.hpp>
#include <transex3/vosapp.hxx>

#include "export.hxx"
#include "layoutparse.hxx"
#include "helpmerge.hxx"

// Convert a rtl::OUString to a byte string.
#define OUSTRING_CSTR( str ) \
    rtl::OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr()

#define STRING( str ) String( str, RTL_TEXTENCODING_UTF8 )
#define BSTRING( str ) ByteString( str, RTL_TEXTENCODING_UTF8 )

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


class TranslateLayout : public Application
{
    ByteString mGid1;
    ByteString mLanguage;
    ByteString mLocalize;
    ByteString mOutput;
    ByteString mProject;
    ByteString mRoot;
    bool mMergeMode;
    std::vector< ByteString > mLanguages;
    std::list< ByteString > mFiles;

public:
    TranslateLayout();
    ~TranslateLayout();
    ByteString GetCommandLineParam( int i );
    ByteString GetOptionArgument( int const i );
    void ExceptionalMain();
    void Main();
    void Merge();
    void MergeLanguage( ByteString const& language );
    void ParseCommandLine();
    void CreateSDF();
};

static void usage()
{
    fprintf( stderr, "Usage: tralay [OPTION]... XML-FILE\n" );
    fprintf( stderr, "\nOptions:\n" );
    fprintf( stderr, "  -h,--help                  show this help\n" );
    fprintf( stderr, "  -l,--language=LANG         process this language\n" );
    fprintf( stderr, "  -m,--merge=DATABASE.SDF    translation database\n" );
    fprintf( stderr, "\nExamples:\n" );
    fprintf( stderr, "  tralay -l en-US -o localize.sdf zoom.xml   # Extract\n" );
    fprintf( stderr, "  tralay -m localize.sdf -l de -l nl -o out zoom.xml  # Merge/translate\n" );
    exit( 2 );
}

ByteString TranslateLayout::GetCommandLineParam( int i )
{
    return ByteString( OUSTRING_CSTR( Application::GetCommandLineParam( i ) ) );
}

ByteString TranslateLayout::GetOptionArgument( int const i )
{
    if ( i >= GetCommandLineParamCount() )
        usage();
    ByteString arg = GetCommandLineParam( i );
    if ( !arg.CompareTo( "-", 1 ) )
    {
        fprintf( stderr, "Option needs an argument: %s, found: %s\n",
                 GetCommandLineParam( i - 1 ).GetBuffer(),
                 arg.GetBuffer() );
        usage();
    }
    return arg;
 }

void TranslateLayout::ParseCommandLine()
{
    for ( int i = 0; i < GetCommandLineParamCount(); i++ )
    {
        ByteString aParam = GetCommandLineParam( i );
        if ( aParam.Equals( "-h" ) || aParam.Equals( "--help" ) )
            usage();
        else if ( aParam.Equals( "-l" ) || aParam.Equals( "--language" ) )
            mLanguages.push_back ( GetOptionArgument( ++i ) );
        else if ( aParam.Equals( "-m" ) || aParam.Equals( "--merge" ) )
        {
            mMergeMode = true;
            mLocalize = GetOptionArgument( ++i );
        }
        else if ( aParam.Equals( "-o" ) || aParam.Equals( "--output" ) )
            mOutput = GetOptionArgument( ++i );
        else if ( !aParam.CompareTo( "-", 1 ) )
        {
            fprintf( stderr, "error: No such option: %s\n", aParam.GetBuffer() );
            usage();
        }
        else
            mFiles.push_back( aParam );
    }
    if ( !mFiles.size() )
    {
        fprintf( stderr, "error: No XML-FILE found\n" );
        usage();
    }
}

static XMLAttribute*
findAttribute( XMLAttributeList* lst, String const& name )
{
    for ( ULONG i = 0; i < lst->Count(); i++ )
        if ( lst->GetObject( i )->Equals( name ) )
            return lst->GetObject( i );
    return 0;
}

static XMLAttribute*
translateAttribute( XMLAttributeList* lst,
                    String const& name, String const& translation )
{
    if ( XMLAttribute* a = findAttribute( lst, name ) )
        return lst->Replace ( new XMLAttribute( name.Copy( 1 ), translation ), a );
    return 0;
}

static void
translateElement( XMLElement* element, ByteString const& lang,
                  ResData* resData, MergeDataFile& mergeData )
{
    XMLAttributeList* attributes = element->GetAttributeList();
    std::vector<XMLAttribute*> interesting = interestingAttributes( attributes );
    ByteString id = BSTRING( interesting[0]->GetValue() );
    for ( std::vector<XMLAttribute*>::iterator i = ++interesting.begin();
          i != interesting.end(); ++i )
    {
        ByteString attributeId = id;
        attributeId += BSTRING ( **i );
        resData->sGId = attributeId;
        resData->sId = element->GetOldref();

        if ( PFormEntrys* entry = mergeData.GetPFormEntrys( resData ) )
        {
            ByteString translation;
            entry->GetText( translation, STRING_TYP_TEXT, lang, true );
//            ByteString original = removeContent( element );
            if ( !translation.Len() )
#if 0
                translation = original;
#else
                translation = BSTRING( ( *i )->GetValue() );
#endif
            delete translateAttribute( attributes, **i , STRING( translation ) );
        }
    }
}

static bool is_dir( ByteString const& name )
{
    if (DIR* dir = opendir( name.GetBuffer() ) )
    {
        closedir( dir );
        return true;
    }
    return false;
}

static void make_directory( ByteString const& name )
{
#ifdef WNT
    _mkdir( name.GetBuffer() );
#else
    mkdir( name.GetBuffer() , S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif
}

void TranslateLayout::MergeLanguage( ByteString const& language )
{
    ByteString xmlFile = mFiles.front();

    MergeDataFile mergeData( mLocalize, xmlFile,
                             FALSE, RTL_TEXTENCODING_MS_1252 );

    SimpleXMLParser aParser;
    LayoutXMLFile* layoutXml = new LayoutXMLFile( mMergeMode );
    if ( !aParser.Execute( STRING( xmlFile ), layoutXml ) )
    {
        fprintf(stderr, "error: parsing: %s\n", xmlFile.GetBuffer() );
        return;
    }

    layoutXml->Extract();

    ResData resData( "", "", "" );
    resData.sResTyp = mProject; /* mGid1 ?? */
    resData.sFilename = xmlFile;

    XMLHashMap* xmlStrings = layoutXml->GetStrings();
    for ( XMLHashMap::iterator i = xmlStrings->begin(); i != xmlStrings->end();
          ++i )
    {
        if ( LangHashMap* languageMap = i->second )
            if ( XMLElement* element = ( *languageMap )[ "en-US" ] )
                translateElement( element, language, &resData, mergeData );
    }

    ByteString outFile = "/dev/stdout";
    if ( mOutput.Len() )
    {
        outFile = mOutput;
        if ( is_dir( outFile ) )
        {
            ByteString outDir = mOutput;
            outDir.Append( "/" ).Append( language );
            if ( !is_dir( outDir ) )
                make_directory( outDir );
            outFile = outDir;
            outFile.Append( "/" ).Append( xmlFile );
        }
    }
    layoutXml->Write( outFile );
    delete layoutXml;
}

void TranslateLayout::Merge()
{
    if ( mLanguages.size() )
        for ( std::vector<ByteString>::iterator i = mLanguages.begin();
              i != mLanguages.end(); ++i)
            MergeLanguage( *i );
    else
        MergeLanguage( mLanguage );
}

void TranslateLayout::CreateSDF()
{
    ByteString xmlFile = mFiles.front();
    ByteString sdf = "/dev/stdout";
    if ( mOutput.Len() )
        sdf = mOutput;
    Export::SetLanguages( mLanguages );
    HelpParser::CreateSDF( sdf, mProject, mRoot, xmlFile,
                           new LayoutXMLFile( mMergeMode ), mGid1 );
}

void TranslateLayout::ExceptionalMain()
{
    ParseCommandLine();
    if ( mLanguages.size() )
        mLanguage = mLanguages.front();
    if ( mMergeMode )
        Merge();
    else
        CreateSDF();
}

void TranslateLayout::Main()
{
    try
    {
        ExceptionalMain();
    }
    catch ( xml::sax::SAXException& rExc )
    {
        OString aStr( OUStringToOString( rExc.Message,
                                         RTL_TEXTENCODING_ASCII_US ) );
        uno::Exception exc;
        if (rExc.WrappedException >>= exc)
        {
            aStr += OString( " >>> " );
            aStr += OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US );
        }
        fprintf( stderr, "error: parsing: '%s'\n", aStr.getStr() );
        OSL_ENSURE( 0, aStr.getStr() );
    }
    catch ( uno::Exception& rExc )
    {
        OString aStr( OUStringToOString( rExc.Message,
                                         RTL_TEXTENCODING_ASCII_US ) );
        fprintf( stderr, "error: UNO: '%s'\n", aStr.getStr() );
        OSL_ENSURE( 0, aStr.getStr() );
    }
}

TranslateLayout::TranslateLayout()
    : Application()
    , mGid1( "layout" )
    , mLanguage( "en-US" )
    , mLocalize( "localize.sdf" )
    , mOutput()
    , mProject( "layout" )
    , mRoot( "/root" )
    , mMergeMode( false )
    , mLanguages()
    , mFiles()
{
}

TranslateLayout::~TranslateLayout()
{
}

SAL_IMPLEMENT_MAIN()
{
    TranslateLayout t;
    t.Main();
    return 0;
}
