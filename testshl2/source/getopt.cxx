/*************************************************************************
#*
#*    $RCSfile: getopt.cxx,v $
#*
#*    class classname(s)
#*
#*    Description       perl-like commandline processor
#*                     (see description in headerfile)
#*
#*    Creation Date     Stefan Zimmermann  09/15/2000
#*    last change       $Author: vg $ $Date: 2003-10-06 13:35:48 $
#*    $Revision: 1.3 $
#*
#*    Copyright 2000 Sun Microsystems, Inc. All Rights Reserved.
#*
#*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/testshl2/source/getopt.cxx,v 1.3 2003-10-06 13:35:48 vg Exp $

      Source Code Control System - Update

*************************************************************************/
#ifndef __QADEV_REGSCAN_GETOPT_HXX__
#include "getopt.hxx"
#endif

#include "filehelper.hxx"

//----------------------------------------------------------------------------
// OptDsc
//----------------------------------------------------------------------------
//> createOptDsc
void OptDsc::createOptDsc( const rtl::OString& opt ) {

    // analyze type of option
     // in case of flag indicate and set name and hint
    if ( ( opt.indexOf("=") == -1 ) && ( opt.indexOf(":") == -1 )  ) {
        m_type = OT_BOOL;

        // extract options dokumentation if any
        sal_Int32 index = opt.indexOf(",");
        if ( index == -1 ) {
            m_name = opt;
        }
        else {
            m_name = opt.copy( 0, index );
            m_hint = ( opt.copy( index + 1 ) ).trim();
        }
        return;
    }

    vector < rtl::OString > optLine;

    // ':' indicates that option has optional parameter(s)
    if ( opt.indexOf(":") != -1 ) {
        m_type |= OT_OPTIONAL;
        // create optionline tokenvector
        split( opt, ":,", optLine );
    }
    else {
        // create optionline tokenvector
        split( opt, "=,", optLine );
    }

    // extract name of option
    m_name = optLine[0];

    // parameter(s) of string type
    if ( optLine[1].indexOf("s") != -1 ) {
        m_type |= OT_STRING;
        m_legend += "arg";
    }
    // parameter(s) of numeric type
    if ( optLine[1].indexOf("n") != -1 ) {
        m_type |= OT_NUMBER;
        m_legend += "arg";
    }
    // multiple parameters allowed
    if ( optLine[1].indexOf("@") != -1 ) {
        m_type |= OT_MULTI;
        m_legend += "#1,[arg#n]";
    }
    else {
        m_type |= OT_SINGLE;
    }

    // extract options dokumentation if any
    m_hint = optLine[optLine.size()-1].trim();

} ///< createOptDsc

//> split
void OptDsc::split( const rtl::OString& opt, const rtl::OString& cSet,
                                        vector< rtl::OString >& optLine ) {

    const sal_Int32 cSetLen = cSet.getLength();
    const sal_Char* pcSet   = cSet.getStr();
    sal_Int32 index = 0;
    sal_Int32 oldIndex = 0;

    sal_Int32 i;
       for ( i = 0; i < cSetLen; i++ ) {
           index = opt.indexOf( pcSet[i] );
        if( index != -1 ) {
               optLine.push_back( opt.copy( oldIndex, index - oldIndex ) );
               oldIndex = index + 1;
        }
       }
       optLine.push_back( opt.copy( oldIndex ) );
} ///< split

//----------------------------------------------------------------------------
// GetOpt
//----------------------------------------------------------------------------
//> ~GetOpt
GetOpt::~GetOpt() {
    vector< OptDsc* >::iterator iter = m_optionset.begin();
    while ( iter != m_optionset.end() ) {
        delete (*iter);
        (*iter) = 0;
        iter++;
    }
    m_optionset.clear();
} ///< ~GetOpt

/**
 * private
 * initialization of GetOpt class means to fill the vector members
 * representing the commandline and optionset
 * @param char* cmdLine[] = comandline
 * @param char* optSet[]  = optionset
 *
 * @return void
 */
//> initialize
void GetOpt::initialize( char* cmdLine[], char* optSet[] ) {

    while ( *cmdLine ) {
        m_cmdline.push_back( rtl::OString( *cmdLine ) );
        *cmdLine++;
    }
    // insert an empty OString, to mark the end.
    m_cmdline.push_back(rtl::OString());

    while ( *optSet && ( rtl::OString( optSet[0] ).indexOf("-") == 0 ) ) {
        m_optionset.push_back( new OptDsc( *optSet ) );
        *optSet++;
    }

} ///< initialize

/**
 * public
 * returns a pointer to an object of type optiondescription (OptDsc)
 *
 * @param rtl::OString& opt = name of option
 *
 * @return OptDsc* = pointer to requested optiondescription
 *                   or NULL if not found
 */
//> getOptDsc
OptDsc* GetOpt::getOptDsc( rtl::OString& opt ) {

    vector< OptDsc* >::iterator iter = m_optionset.begin();
    while ( iter != m_optionset.end() ) {
        if ( (*iter)->getName() == opt ) {
            return (*iter);
        }
        iter++;
    }
    return NULL;

} ///< getOptDsc

/**
 * public
 * check if option is already present in optionhash
 * @param const rtl::OString& opt = name of option
 * @return sal_Bool
 */
//> hasOpt
sal_Bool GetOpt::hasOpt( const rtl::OString& opt ) const {

    if ( m_opthash.find( opt ) != m_opthash.end() ) {
        return sal_True ;
    }
    return sal_False ;

} ///< hasOpt

/**
 * private
 * handles the initialized comandline vector
 * and fill the optionhash with evaluated options
 * @param  none
 * @return void
 */
//> createCmdLineOptions
void GetOpt::createCmdLineOptions() {

    // get iterator of comandline vector
    vector< rtl::OString >::iterator iter = m_cmdline.begin();

    int nSize = m_cmdline.size();

    // extract first comandlineparameter as program name
    m_prgname = (*iter);
    iter++;

    // process the whole vector
    while ( iter != m_cmdline.end() ) {
        // extract following comandline parameter(s) as program parameter(s)
        // int nIdxOfMinus = (*iter).indexOf("-");
        if ( (*iter).indexOf("-") != 0 )    /* start without '-' */
        {
            if ((*iter).getLength() > 0 )     /* is not empty */
            {
                m_param.push_back(*iter);
            }
            iter++;
            continue;
        }
        // option occured
        if ( (*iter).indexOf("-") == 0 )
        {
            // ignore invalid options
            if ( ! evaluateOpt( iter ) )
            {
                iter++;
                // check if wrong option has got a parameter
                // and skip that, too
                if( (iter + 1) != m_cmdline.end() )
                {
                    if ( (*(iter + 1)).indexOf("-") != 0 )
                    {
                        iter++;
                    }
                }
                continue;
            }
            rtl::OString opt( (*iter) );
            vector< rtl::OString > optValues;

            // option is no flag
            if ( ! getOptDsc( opt )->isFlag() ) {
                // but has optional parameters
                if ( getOptDsc( opt )->isOptional() ) {
                    // no parameters present
                    if ( ! hasParam( iter ) ) {
                        m_opthash[ opt ] = optValues;
                        optValues.clear();
                        iter++;
                        continue;
                    }
                }
                iter++;
                // more than one option parameters occured
                if ( (*iter).indexOf( "," )  != -1  ) {
                    tokenize( (*iter), "," , optValues );
                }
                else  {
                    optValues.push_back( (*iter) );
                }
            }
            // create key/value pair in optionhash and clear value vector
            m_opthash[ opt ] = optValues;
            optValues.clear();
        }
        iter++;
    }
} ///< createCmdLineOptions

/**
 * public
 * check if option has parameter(s)
 * @param vector< rtl::OString >::iterator iter = iterator of
 *                                                  comandline vector
 *
 * @return sal_Bool
 */
//> hasParam
sal_Bool GetOpt::hasParam( vector< rtl::OString >::iterator iter ) {
    if ( iter+1 == m_cmdline.end() ) {
        return sal_False;
    }
    if ( (*(iter+1)).indexOf("-") == 0 ) {
        return sal_False;
    }
    if ( (*(iter+1)) == "" ) {
        return sal_False;
    }

    return sal_True;
} ///< hasParam

/**
 * public
 * option evaluation in general means to verify if the option occur is
 * a member of optionset say an admitted option, if so does it appear with
 * the right or tolerable usage
 *
 * @param vector< rtl::OString >::iterator iter = iterator of
 *                                                  comandline vector
 *
 * @return sal_Bool
 */
//> evaluateOpt
sal_Bool GetOpt::evaluateOpt( vector< rtl::OString >::iterator iter ) {

    // option is no member of optionset
    if ( ! exist( (*iter) ) ) {
        cout << "Unknown option " << (*iter).getStr()
                << " occurred !" << endl;
        return sal_False;
    }

    // option is a flag
    if ( getOptDsc( (*iter) )->isFlag() ) {
        return sal_True;
    }

    // parameter not optional
    if ( ! getOptDsc( (*iter) )->isOptional() ) {

        // verify that next vectoritem is present and no option
        if ( ( *( iter + 1 ) ).getLength() &&
                          ( ( *( iter + 1 ) ).indexOf( "-" ) != 0 ) )  {

            // if we are waiting for one single parameter
            if ( getOptDsc( *iter )->isSingle() ) {
                // but find multiple parameters
                if( ( *( iter + 1 ) ).indexOf(",") != -1 ) {
                    cout << "Wrong use of option " << (*iter).getStr()
                            << " too many parameters !" << endl;
                    return sal_False;
                }
                return sal_True;
            }
            return sal_True;
        }

        cout << "Wrong use of option " << (*iter).getStr()
                << " parameter missing !" << endl;
        return sal_False;
    }
    // parameter optional
    if ( getOptDsc( *iter )->isSingle() ) {

        if ( hasParam( iter ) ) {
            if( ( *( iter + 1 ) ).indexOf(",") != -1 ) {
                cout << "Wrong use of option " << (*iter).getStr()
                        << " too many parameters !" << endl;
                return sal_False;
            }
        }
    }
    return sal_True;

} ///< evaluateOpt


//> createOpt
void GetOpt::createOpt( rtl::OString& optDscStr ) {
    m_optionset.push_back( new OptDsc( optDscStr ) );
} ///< createOpt

/**
 * public
 * conditional addition of an option to optionhash
 * overriding options, already present in optionhash, is not permitted
 *
 * @param rtl::OString& optStr = optionstring which is to break in a
 *                                 key/value pair and to add to optionhash
 *
 * @return void
 */
//> addOpt
void GetOpt::addOpt( rtl::OString& optStr, sal_Bool eval ) {

    vector< rtl::OString > optTok;
    tokenize( optStr, "=", optTok );

    // prevent override of commandline options
    // by options from ini file
    if( hasOpt( optTok[0] ) ) {
        return;
    }

    // evaluate rigth usage of option
    if( eval ) {
        if ( ! evaluateOpt( optTok.begin() ) ) {
            return;
        }
    }
    vector< rtl::OString > optValues;

    if ( optTok.size() > 1 ) {
        rtl::OString oValStr( optTok[1] );
        // found a variable
        if ( oValStr.indexOf(",")  == -1  ) {
            optValues.push_back( oValStr );
        }
        else  {
            tokenize( oValStr, ",", optValues );
        }
    }
    m_opthash[ optTok[0] ] = optValues;
    optValues.clear();
    return;
} ///< addOpt

/**
 * public
 * verify the existance of an option in optionset
 * @param rtl::OString& opt = option name
 * @return sal_Bool
 */
//> exist
sal_Bool GetOpt::exist( rtl::OString& opt ) {
    if ( getOptDsc( opt ) ) {
        return sal_True;
    }
    return sal_False;
} ///< exist

/**
 * public
 * verify the existance of variables inside options
 * @param none
 * @return sal_Bool
 */
//> hasVars
sal_Bool GetOpt::hasVars() {
    if ( m_varvec.size() ) {
        return sal_True;
    }
    return sal_False;
} ///< hasVars


/**
 * public
 * proceeds a buffer representing the content of an ini file and adds the
 * options to optionhash. The optionstrings in the file are allowed to contain
 * variables indicated by delimiters described with varDelim
 *
 * @param rtl::OString iOpts = raw filecontent
 * @param const rtl::OString& varDelim = delimiter indicating a variable
 *
 * @return void
 */
//> str2Opt
void GetOpt::str2Opt( rtl::OString iOpts ) {

    // tokenize filecontent by '\n' to create a vector of lines
    vector< rtl::OString > iniLines;
    tokenize( iOpts, "\n", iniLines );

    sal_uInt32 tCnt = iniLines.size();

    // process all lines
    sal_uInt32 i;
    for ( i = 1; i < tCnt; i++ ) {
        rtl::OString optLine( iniLines[i] );
        // ignore comments
        if ( ! ( optLine.indexOf("#") == 0 ) ) {
            // filter valid options after trim
            if ( ( optLine.indexOf("-") == 0 ) ) {
                // line contains a variable
                if ( ( optLine.indexOf( m_vardelim ) != -1 ) ) {
                    // push to var vector for later process
                    m_varvec.push_back( optLine );
                    continue;
                }
                addOpt( optLine );
            }
        }
    }
} ///< str2opt

void GetOpt::replVars() {

    // process vector of lines containing variables
    vector< rtl::OString >::iterator iter = m_varvec.begin();
    while ( iter != m_varvec.end() ) {
        sal_uInt32 index = 0;
        while ( ( index = (*iter).indexOf( m_vardelim ) ) != -1 ) {
            vector< rtl::OString > varLineTok;
            rtl::OString varKey( "-" );
            tokenize( *iter, m_vardelim, varLineTok );
            varKey += varLineTok[1];
            vector< rtl::OString > keyValues = getOptVec( varKey );

            if ( keyValues.size() > 1 ) {
                rtl::OString rplStr;

                vector< rtl::OString >::iterator kvi = keyValues.begin();
                while ( kvi != keyValues.end() ) {
                    rplStr += (*kvi);
                    kvi++;
                    if ( kvi != keyValues.end() ) {
                        rplStr += ",";
                    }
                }
                (*iter ) = (*iter).replaceAt(
                                    index, varKey.getLength()+1, rplStr );
            }
            else  {
                if( *(keyValues[0])) {
                (*iter) = (*iter).replaceAt(
                                index, varKey.getLength()+1, keyValues[0] );
                }
            }
        }
        addOpt( (*iter) );
        iter++;
    }

}

/**
 * public
 * displays a formatted usagescreen
 * @param  none
 * @return void
 */
//> showUsage
void GetOpt::showUsage() {

    sFormat frm;
    frm.fCol = getMaxNameLength() + 2;
    frm.sCol = frm.fCol + getMaxLegendLength() + 2 ;
    frm.len = 79;

    vector< rtl::OString > nameVec;
    vector< rtl::OString > paramVec;

    tokenize( getName(), "/\\", nameVec );
    if ( m_param.empty() ) {
        if ( hasOpt( "-db" ) ) {
            tokenize( getOpt( "-db" ), "/\\", paramVec );
        }
        else  {
            paramVec.push_back( rtl::OString( "not available" ) );
        }
    }
    else  {
        tokenize( getFirstParam(), "/\\", paramVec );
    }

    cout << "\n\n\n\n\nUsage: prgname param [options]\n\nPRGNAME = [path]";

    if ( nameVec.end() ) {
        cout << (*(nameVec.end()-1)).getStr();
    }
    cout << "\nPARAM   = [path]";
    if ( paramVec.end() ) {
        cout << (*(paramVec.end()-1)).getStr() << endl;
    }

    cout << "\nOPTIONS = [" << flush;

    vector< OptDsc* >::iterator iter =  m_optionset.begin();

    while ( iter != m_optionset.end() ) {
        cout << (*iter)->getName().getStr() << "," << flush;
        iter++;
    }
    cout    << "]\n\nOPTIONS:\n" << flush;

    iter = m_optionset.begin();

    while ( iter != m_optionset.end() ) {
        cout <<  optDsc2Str( *iter, frm ).getStr() << endl << flush;

        iter++;
    }

} ///< showUsage

/**
 * public
 * displays the actual option/parameter status
 * @param none
 * @return void
 */
//> printStatus
void GetOpt::printStatus( void ) {

    sal_uInt32 maxlen = getMaxNameLength();
    optHashMap::iterator iter = m_opthash.begin();
    cout << endl;

    while ( iter != m_opthash.end() ) {
        rtl::OString option( (*iter).first );

        cout.setf(ios::left);
        cout.width( maxlen+1 );
        cout << option.getStr() << "= ";

        if ( ! getOptDsc( option )->isFlag() ) {
            if ( ! getOptVec( option ).empty() ) {
                sal_uInt32 j;
                for ( j = 0; j < (*iter).second.size(); j++ ) {
                    cout << (( (*iter).second )[j]).getStr() << " ";
                }
                cout << endl;
            }
            else  {
                cout << "FALSE\n";
            }
        }
        else {
            cout << "TRUE\n";
        }
        iter++;
    }
}

/**
 * private
 * converts an object of type optiondescription (OptDsc) to a formatted
 * displayable string for usagescreen needs
 * @param OptDsc* optDsc = pointer to option description
 * @param sFormat frm    = format structur
 *
 * @return const rtl::OString = formatted string for display purposes
 */
//> optDsc2Str >>> to be replaced by intelliget algorythm <<<
const rtl::OString GetOpt::optDsc2Str( OptDsc* optDsc , sFormat frm ) {

    sal_Char* buf = new sal_Char[ frm.len + 1 ];
    sal_Char* pBuf = buf;

    sal_uInt32 i;
    for ( i = 0; i < frm.len; i++ ) {
        *pBuf++ = ' ';
    }
    *pBuf = '\0';

    rtl::OStringBuffer strBuf( buf );
    rtl::OString oStr = strBuf.makeStringAndClear();

    oStr = oStr.replaceAt( 0, optDsc->getName().getLength(),
                                                        optDsc->getName() );
    if ( optDsc->isOptional() ) {
        oStr = oStr.replaceAt( frm.fCol-1, 1, "[" );
    }
    oStr = oStr.replaceAt( frm.fCol, optDsc->getLegend().getLength(),
                                                        optDsc->getLegend() );
    if ( optDsc->isOptional() ) {
        oStr = oStr.replaceAt( frm.fCol + optDsc->getLegend().getLength() ,
                                                                    1, "]" );
    }
    if ( ( frm.sCol + optDsc->getHint().getLength() ) >= frm.len ) {

        oStr = oStr.replaceAt( frm.sCol, frm.len - frm.sCol,
                                                        optDsc->getHint() );
    }
    else  {
        oStr = oStr.replaceAt( frm.sCol,
                        optDsc->getHint().getLength(),  optDsc->getHint() );
    }

    delete [] buf;

    return oStr;

} ///< optDsc2Str

/**
 * private
 * returns the maximum length of all optionnames for format purposes
 * @param none
 * @return sal_uInt32 length of longest optionname
 */
//> getMaxNameLength
sal_uInt32 GetOpt::getMaxNameLength() {

    sal_Int32 len = 0;
    vector< OptDsc* >::iterator iter =  m_optionset.begin();

    while ( iter != m_optionset.end() ) {
        if( len < (*iter)->getName().getLength() ){
            len = (*iter)->getName().getLength();
        }
        iter++;
    }
    return len;
} ///< getMaxNameLength

/**
 * private
 * returns the maximum length of all option legends for format purposes
 * @param none
 * @return sal_uInt32 length of longest optionlegend
 */
//> getMaxLegendLength
sal_uInt32 GetOpt::getMaxLegendLength() {

    sal_Int32 len = 0;
    vector< OptDsc* >::iterator iter =  m_optionset.begin();

    while ( iter != m_optionset.end() ) {
        if( len < (*iter)->getLegend().getLength() ){
            len = (*iter)->getLegend().getLength();
        }
        iter++;
    }
    return len;

} ///< getMaxLegendLength

/**
 * public
 * reads the filecontent and pass it to str2opt to add valid options
 * to optionhash
 * @param rtl::OString iniPth = full qualified filename
 * @return ::osl::FileBase::RC = to indicate errors
 */
//> getIniOptions
::osl::FileBase::RC GetOpt::getIniOptions( rtl::OString iniPth ) {

    ::osl::FileStatus fState( FileStatusMask_All );
    ::osl::DirectoryItem dItem;
    rtl::OUString nrmPath( FileHelper::convertPath( iniPth ) );

    ::osl::DirectoryItem::get( nrmPath, dItem );
    dItem.getFileStatus( fState );
    rtl::OUString fName( fState.getFileURL() );
    ::osl::File iniFile( fName );

    const sal_uInt32 filesize = (sal_uInt32)fState.getFileSize();

    ::osl::FileBase::RC ret;
    sal_uInt64 bytesread;

    if ( ( ret = iniFile.open( OpenFlag_Read ) ) != ::osl::FileBase::E_None )  {
        return ret;
    }
     char* buf = new char[ filesize + 1 ];
    ret = iniFile.read( buf, filesize, bytesread );
    buf[ filesize ] = '\0';

    str2Opt( buf );
    delete [] buf;
    return ret;

} ///< getIniOptions

/**
 * private
 * tokenize a string in dependance of a character set and stores the tokens
 * to a token vector
 * @param const rtl::OString& opt = optionstring to tokenize
 * @param const rtl::OString& cSet = characterset of delimiters
 * @param vector< rtl::OString >& optLine = vector of tokens
 * @param sal_Bool strip = indicates if CR,LF and TAB should be stripped off
 *                         the token
 * @return void
 */
//> tokenize
void GetOpt::tokenize( const rtl::OString& opt, const rtl::OString& cSet,
                       vector< rtl::OString >& optLine, sal_Bool strip ) {

    const sal_Char* pText;                      // pointer f. text,
    const sal_Char* pcSet;                      // charset and
    vector< const sal_Char* > delimVec;         // vector of delimiters

    // parametercheck for opt ...
    if( ! opt.getLength() ) {
        return;
    }
    // ... and charSet
    if( ! cSet.getLength() ) {
        return;
    }
    // pointer to begin of textinstance
    pText = opt.getStr();

    // text
    while( *pText ) {
        // charset-pointer to begin of charset
        pcSet = cSet.getStr();
        // charset
        while( *pcSet ) {
            // delimiter found
            if( ( ( *pText == *pcSet ) ) && ( pText != opt ) ) {
                delimVec.push_back( pText );
                // increment counter
            }
            pcSet++;
        }
        // increment pointer
        pText++;
    }

    // save endpointer
    delimVec.push_back( opt + opt.getLength() );

    sal_Char* pToken;                           // ptr to token chars
    const sal_Char* pBegin;                     // ptr to begin of current,
    const sal_Char* pEnd=opt;                   // and begin of prev. token
    sal_uInt32 i=0;

    while ( pEnd < delimVec[delimVec.size()-1] ) {

        pBegin = pEnd;
        if( pBegin > opt.getStr() ) {
            pBegin += 1;
        }
        pEnd = delimVec[i];
        // initialize size
        sal_uInt32 nSize =  pEnd - pBegin;
        // allocate memory

        // allocate memory for token
        sal_Char* cToken = new sal_Char[ nSize + 1 ];

        // get address of allocated memory
        pToken = cToken;

        // copy token from text
        sal_uInt32 j;
        for ( j = 0; j < nSize ; ++j ) {
            *pToken++ = *pBegin++;
        }
        // append string end
        *pToken = '\0';

        rtl::OString oTok(cToken);

        if( strip ) {
            // strip off CR,LF and TAB
            oTok = oTok.replace( 0x0a, 0x20 );
            oTok = oTok.replace( 0x0d, 0x20 );
            oTok = oTok.replace( 0x09, 0x20 );
            oTok = oTok.trim();
        }
        if( oTok != "" ) {
            // push to vector
            optLine.push_back( oTok );
        }

        // free memory where cToken points to
        delete [] cToken;
        i++;
    }
    return ;
} ///< tokenize

// -----------------------------------------------------------------------------
rtl::OString& GetOpt::getOpt( const rtl::OString& opt )
{
    if (m_opthash.find( opt ) != m_opthash.end())
    {
        if (m_opthash[opt].begin())
        {
            return *( m_opthash[opt].begin() );
        }
        rtl::OString aStr;
        aStr = "GetOpt( ";
        aStr += opt;
        aStr += " ): Value not found.";
            throw ValueNotFoundException(aStr.getStr());
    }
    else
        throw ValueNotFoundException(opt);
}


// -----------------------------------------------------------------------------

Exception::Exception()
        : m_sAsciiMessage()
{
}
//---------------------------------------------------------------------
Exception::Exception(char const* sAsciiMessage)
        : m_sAsciiMessage(sAsciiMessage)
{
}
//---------------------------------------------------------------------
Exception::Exception(rtl::OString const& sAsciiMessage)
        : m_sAsciiMessage(sAsciiMessage)
{
}
//---------------------------------------------------------------------

rtl::OUString Exception::message() const
{
    return rtl::OStringToOUString( m_sAsciiMessage, RTL_TEXTENCODING_ASCII_US );
}
//---------------------------------------------------------------------
char const* Exception::what() const
{
    return m_sAsciiMessage.getLength() ? m_sAsciiMessage.getStr() : "FAILURE in REGSCAN: No description available";
}

// -----------------------------------------------------------------------------
static const char c_sValueNotFoundException[] = "GetOpt: Value not Found Exception: ";
//---------------------------------------------------------------------
ValueNotFoundException::ValueNotFoundException()
        : Exception( rtl::OString(RTL_CONSTASCII_STRINGPARAM(c_sValueNotFoundException)) )
{
}
//---------------------------------------------------------------------

ValueNotFoundException::ValueNotFoundException(char const* sException)
        : Exception( rtl::OString(RTL_CONSTASCII_STRINGPARAM(c_sValueNotFoundException)) += sException)
{
}



