/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <unicode/regex.h>
#include <unicode/unistr.h>
#include <string>
#include <fstream>
#include <iostream>
#include "inireader.hxx"

using namespace std;
namespace transex3
{

bool INIreader::read( INImap& myMap , string& filename )
{
    ifstream aFStream( filename.c_str() );
    if( aFStream && aFStream.is_open())
    {
        string line;
        string section;
        string param_key;
        string param_value;
        stringmap* myvalues = 0;

        while( std::getline( aFStream , line ) )
        {
            trim( line );
            if( line.empty() ){
            }
            else if( is_section( line , section ) )
            {
                //cerr << "[" << section << "]\n";
                myvalues = new stringmap();
                myMap[ section ] = myvalues ;
            }
            else if ( is_parameter( line , param_key , param_value ) )
            {
                //cerr << "" << param_key << " = " << param_value << "\n";
                if( myvalues )
                {
                    (*myvalues)[ param_key ] = param_value ;
                }
                else
                {
                    cerr << "ERROR: The INI file " << filename << " appears to be broken ... parameters without a section?!?\n";
                    if( aFStream.is_open() ) aFStream.close();
                    return false;
                }
            }
        }

        if( aFStream.is_open() )
            aFStream.close();

        return true;
    }
    else
    {
        cerr << "ERROR: Can't open file '" << filename << "'\n";
    }
    return false;
}

bool INIreader::is_section( string& line , string& section_str )
{
    // Error in regex ?
    check_status( section_status );
    UnicodeString target( line.c_str() , line.length() );

    section_match->reset( target );
    check_status( section_status );

    if( section_match->find() )
    {
        check_status( section_status );
        UnicodeString result(  section_match->group( 1 , section_status) );
        check_status( section_status );
        toStlString( result , section_str );

        return true;
    }
    return false;
}

bool INIreader::is_parameter( string& line , string& parameter_key , string& parameter_value )
{
    // Error in regex ?
    check_status( parameter_status );
    UnicodeString target( line.c_str() , line.length() );

    parameter_match->reset( target );
    check_status( parameter_status );

    if( parameter_match->find() )
    {
        check_status( parameter_status );

        UnicodeString result1(  parameter_match->group( 1 , parameter_status) );
        check_status( parameter_status );
        toStlString( result1 , parameter_key );
        UnicodeString result2(  parameter_match->group( 2 , parameter_status) );
        check_status( parameter_status );
        toStlString( result2 , parameter_value );

        return true;
    }
    return false;
}

void INIreader::check_status( UErrorCode status )
{
    if( U_FAILURE( status) )
    {
        cerr << "Error in or while using regex: " << u_errorName( status ) << "\n";
        exit(-1);
    }
}

void INIreader::toStlString( const UnicodeString& str , string& stl_str)
{
         // convert to string
        char* buffer = new char[ str.length()*3 ];
        str.extract( 0 , str.length() , buffer );
        stl_str = string( buffer );
        delete [] buffer;
}

void INIreader::trim( string& str )
{
    string str1 = str.substr( 0 , str.find_last_not_of(' ') + 1 );
    str = str1.empty() ? str1 : str1.substr( str1.find_first_not_of(' ') );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
