/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: getopt.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:20:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/*************************************************************************
#*    class GetOpt
#*
#*
#*    Implementation    getopt.cxx
#*
#*    Description:  class to reflect the commandline options. In addition
#*                  it provides functinality to get options from an inifile
#*                  ( getIniOptions()), evaluate, test and get options
#*                  ( hasOpt(), getOpt() ) and display an usage screen
#*                  formated in dependance of the given optionset, as well
#*                  as display a status about activ options
#*
#*
#* further descriptions:
#*
#* the constructor is called with two parameters:
#* char* argv[] and char* optionSet[],
#* the commandline and the available set of options.
#* Valid optionstrings have to start with a minus '-', otherwise it will
#* be ignored. Optionstrings consist of one or two arguments divided by a
#* comma separator, where the second (optional) argument is a short
#* description of the option, called hint. It will be automatically
#* formated and displayed in the usage screen if available.
#* The first argument of the optionstring can consist of two sections,
#* the name and the optional necessity and format.
#* The name of the option will be the starting minus followed by a string
#* like "-src".
#* One section options will be treated as flag (see '-v' option in example).
#* The second section consist of '=' or ':' which defines the necessity,
#* in combination with a character 's' for string or 'n' for numeric and
#* an optional '@' standing for 'vector of'.
#* '=' means this option needs an argument, ':' means can take an argument
#* followed by 's' is, as mentioned above, a single string paramenter,
#* followed by a 's' in combination with '@' means this argument consists
#* of one string or a vector of strings separated by commas
#*
#* an example for an optionset:
#*
#*  static char* optionSet[] = {
#*                  "-src=s,    release or version of sources",
#*                  "-ini=s,    ini file with additional options",
#*                  "-db=s,     path of type description database",
#*                  "-inPth=s,  source path of component descriptions",
#*                  "-outPth=s, destination path for testcases",
#*                  "-logPth=s, destination path for logging",
#*                  "-mdl=s@,   name(s) of module(s) to generate",
#*                  "-v,        enable verbose screen messages",
#*                  "-dbg,      enable debug messages",
#*                  "-h:s,      display help or help on option",
#*                  "-help:s,   see -h",
#*                  NULL
#*                  };
#*
#*    As seen a GetOpt class object will be created via:
#*         GetOpt  myOptions( argv, optionSet );
#*
*************************************************************************/
#ifndef __QADEV_REGSCAN_GETOPT_HXX__
#define __QADEV_REGSCAN_GETOPT_HXX__

#ifndef _OSL_FILE_HXX_
#include    <osl/file.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include    <sal/types.h>
#endif

#ifndef _RTL_STRBUF_HXX_
#include    <rtl/strbuf.hxx>
#endif


#ifndef _RTL_STRING_HXX_
#include    <rtl/string.hxx>
#endif

// #ifndef __QADEV_REGSCAN_UTIL_HXX__
// #include "inc/util.hxx"
// #endif


#ifdef SOLARIS
#include <sys/time.h>
#endif

#include <iostream>
#include <hash_map>
// #include "rsexception.hxx"

// using namespace std;

struct gstr {
    sal_Bool operator()( const rtl::OString& oStr1,
                                    const rtl::OString& oStr2 ) const {
        return( oStr1 == oStr2 );
    }
};

struct ghstr
{
    sal_uInt32 operator()( const rtl::OString& str ) const {
        return str.hashCode();
    }
};

struct frmt {
    sal_uInt32 fCol;
    sal_uInt32 sCol;
    sal_uInt32 len;
};

typedef std::hash_map< rtl::OString, std::vector< rtl::OString >, ghstr, gstr >
                                                                optHashMap;

typedef frmt sFormat;
typedef sal_uInt16 optType;

#define OT_INVALID          0x0000
#define OT_BOOL             0x0001
#define OT_STRING           0x0002
#define OT_NUMBER           0x0004
#define OT_SINGLE           0x0010
#define OT_MULTI            0x0020
#define OT_OPTIONAL         0x0100

//: Option
class OptDsc {

    rtl::OString    m_name;
    rtl::OString    m_hint;
    rtl::OString    m_legend;
    optType         m_type;

    /**
     * analysis of optionstring and creation of option description class
     * @param const rtl::OString& opt = optionstring to analyse
     * @return void
     */
    void createOptDsc( const rtl::OString& opt );

    /**
     * splitting of an optionstring and creation of a tokenvector
     * in dependance of a charset
     *
     * @param const rtl::OString& opt  = optionstring to split
     * @param const rtl::OString& cSet = delimiter charset
     * @param vector< rtl::OString >& optLine = tokenvector to fill
     *
     * @return void
     */
    void split( const rtl::OString& opt, const rtl::OString& charSet,
                    std::vector< rtl::OString >& optLine );

    /**
     * private default c'tor, copy c'tor and assignment operator
     * to get compiler errors on dumb effords
     */
    OptDsc();
    OptDsc( OptDsc& );
    OptDsc& operator = ( OptDsc& );

public:

    //> c'tor
    OptDsc( const rtl::OString& opt )
            : m_name(),
              m_hint(),
              m_legend(),
              m_type( 0 ) {

        createOptDsc( opt );
    } ///< c'tor

    //> d'tor
    ~OptDsc(){}

    /**
     * returns the name of this option
     * @return rtl::OString&
     */
    inline rtl::OString& getName() { return m_name; }
    /**
     * returns the hint of this option
     * @return rtl::OString&
     */
    inline rtl::OString& getHint() { return m_hint; }
    /**
     * returns the legend of this option
     * @return rtl::OString&
     */
    inline rtl::OString& getLegend() { return m_legend; }

    /**
     * returns the type of this option
     * @return optType
     */
    inline optType getType() { return m_type; }


    inline sal_Bool isFlag() {
        return( ( m_type &~ 0xfffe ) == OT_BOOL ) ? sal_True : sal_False;
    }
    inline sal_Bool isString() {
        return( ( m_type &~ 0xfffd ) == OT_STRING ) ? sal_True : sal_False;
    }
    inline sal_Bool isNumber() {
        return( ( m_type &~ 0xfffb ) == OT_NUMBER ) ? sal_True : sal_False;
    }
    inline sal_Bool isSingle() {
        return( ( m_type &~ 0xffef ) == OT_SINGLE ) ? sal_True : sal_False;
    }
    inline sal_Bool isMulti() {
        return( ( m_type &~ 0xffdf ) == OT_MULTI ) ? sal_True : sal_False;
    }
    inline sal_Bool isOptional() {
        return( ( m_type &~ 0xfeff ) == OT_OPTIONAL ) ? sal_True : sal_False;
    }

}; ///:~ Option

class Exception
{
    rtl::OString m_sAsciiMessage;
public:
    Exception();
    virtual ~Exception(){}

    Exception(char const* sAsciiMessage);
    Exception(rtl::OString const& sAsciiMessage);

    virtual rtl::OUString message() const;
    virtual char const* what() const;
};

class ValueNotFoundException : public Exception
{
public:
    ValueNotFoundException();
    ValueNotFoundException(char const* sExit);
};

//: GetOpt
class GetOpt {

    rtl::OString                m_prgname;
    rtl::OString                m_vardelim;
    std::vector< rtl::OString >     m_cmdline;
    std::vector< rtl::OString >     m_param;
    std::vector< rtl::OString > m_varvec;
    std::vector< OptDsc* >          m_optionset;
      optHashMap                    m_opthash;

    //> private methods
    void initialize( char* cmdLine[], char const * optSet[] );
    void createCmdLineOptions();
    sal_uInt32 getMaxNameLength();
    sal_uInt32 getMaxLegendLength();
    const rtl::OString optDsc2Str( OptDsc* optDsc , sFormat frm );
    void tokenize(  const rtl::OString& opt, const rtl::OString& charSet,
                std::vector< rtl::OString >& optLine, sal_Bool strip = sal_True );
    ///< private methods

    GetOpt();
    GetOpt( GetOpt& );
    GetOpt& operator = ( GetOpt& );

public:

    //> c'tor
    GetOpt( char* cmdLine[], char const * optSet[], rtl::OString varDelim =
                                                        rtl::OString( "$" ) )
            : m_vardelim( varDelim ) {

        initialize( cmdLine, optSet );
        createCmdLineOptions();
    } ///< c'tor

    //> d'tor
    ~GetOpt();
    ///< d'tor

    //> inline methods
    inline std::vector< rtl::OString >* getCmdLine() { return &m_cmdline; }
    inline optHashMap* getOptions() { return ( &m_opthash ); }
    inline rtl::OString& getName() { return m_prgname; }
    inline rtl::OString& getFirstParam() { return *(m_param.begin()); }
    inline std::vector< rtl::OString >& getParams() { return m_param; }
    rtl::OString& getOpt( const rtl::OString& opt );
    void rmvOpt( rtl::OString& opt ) {
        m_opthash.erase( opt );
    }
    inline std::vector< rtl::OString >& getOptVec( const rtl::OString& opt ) {
        return m_opthash[ opt ];
    }
    ///< inline methods


    sal_Bool exist( rtl::OString& opt );
    sal_Bool hasParam( std::vector< rtl::OString >::iterator iter );
    sal_Bool hasVars( void );
    sal_Bool evaluateOpt( std::vector< rtl::OString >::iterator iter );
    OptDsc* getOptDsc( rtl::OString& opt );
    sal_Bool hasOpt( const rtl::OString& opt ) const;
    ::osl::FileBase::RC getIniOptions( rtl::OString iniPth );
    void createOpt( rtl::OString& optdsc );
    void str2Opt( rtl::OString iOpts );
    void addOpt( rtl::OString& opt, sal_Bool evaluate = sal_True );
    void replVars( void );
    void showUsage( void );
    void printStatus( void );

}; ///:~ GetOpt
#endif
