/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <string>
#include <hash_map>
#include <unicode/regex.h>

using namespace std;

namespace transex3
{

struct eqstr
{
    bool operator()( const string s1 , const string s2) const
    {
        return s1.compare( s2 ) == 0;
    }
};

typedef std::hash_map< string , string > stringmap;
typedef std::hash_map< string, stringmap* > INImap;

class INIreader
{
    private:
        UErrorCode section_status;
        UErrorCode parameter_status;
        RegexMatcher* section_match;
        RegexMatcher* parameter_match;

    public:
        INIreader(): section_status   ( U_ZERO_ERROR ) ,
                     parameter_status ( U_ZERO_ERROR )
        {
                     section_match   = new RegexMatcher   ( "^\\s*\\[([a-zA-Z0-9]*)\\].*" , 0 , section_status );
                     parameter_match = new RegexMatcher   ( "^\\s*([a-zA-Z0-9]*)\\s*=\\s*([a-zA-Z0-9 ]*).*" , 0 , parameter_status ) ;
        }
        ~INIreader()
        {
            delete section_match;
            delete parameter_match;
        }
        // open "filename", fill hash_map with sections / paramaters
        bool read( INImap& myMap , string& filename );

    private:
        bool is_section( string& line , string& section_str );
        bool is_parameter( string& line , string& parameter_key , string& parameter_value );
        inline void check_status( UErrorCode status );
        inline void toStlString ( const UnicodeString& str, string& stl_str );
        inline void trim( string& str );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
