/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * (C) 1988, 1989, 1990 by Adobe Systems Incorporated. All rights reserved.
 *
 * This file may be freely copied and redistributed as long as:
 *   1) This entire notice continues to be included in the file,
 *   2) If the file has been modified in any way, a notice of such
 *      modification is conspicuously indicated.
 *
 * PostScript, Display PostScript, and Adobe are registered trademarks of
 * Adobe Systems Incorporated.
 *
 * ************************************************************************
 * THE INFORMATION BELOW IS FURNISHED AS IS, IS SUBJECT TO CHANGE WITHOUT
 * NOTICE, AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ADOBE SYSTEMS
 * INCORPORATED. ADOBE SYSTEMS INCORPORATED ASSUMES NO RESPONSIBILITY OR
 * LIABILITY FOR ANY ERRORS OR INACCURACIES, MAKES NO WARRANTY OF ANY
 * KIND (EXPRESS, IMPLIED OR STATUTORY) WITH RESPECT TO THIS INFORMATION,
 * AND EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR PARTICULAR PURPOSES AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
 * ************************************************************************
 */

/*
 * Changes made for OpenOffice.org
 *
 *  10/24/2000 pl       - changed code to compile with c++-compilers
 *                      - added namespace to avoid symbol clashes
 *                      - replaced BOOL by bool
 *                      - added function to free space allocated by parseFile
 *  10/26/2000 pl       - added additional keys
 *                      - added ability to parse slightly broken files
 *                      - added charwidth member to GlobalFontInfo
 *  04/26/2001 pl       - added OpenOffice header
 *  10/19/2005 pl       - performance increase:
 *                         - fread file in one pass
 *                         - replace file io by buffer access
 *  10/20/2005 pl       - performance increase:
 *                         - use one table lookup in token() routine
 *                           instead of many conditions
 *                         - return token length in token() routine
 *                         - use hash lookup instead of binary search
 *                           in recognize() routine
 */

/* parseAFM.c
 *
 * This file is used in conjunction with the parseAFM.h header file.
 * This file contains several procedures that are used to parse AFM
 * files. It is intended to work with an application program that needs
 * font metric information. The program can be used as is by making a
 * procedure call to "parseFile" (passing in the expected parameters)
 * and having it fill in a data structure with the data from the
 * AFM file, or an application developer may wish to customize this
 * code.
 *
 * There is also a file, parseAFMclient.c, that is a sample application
 * showing how to call the "parseFile" procedure and how to use the data
 * after "parseFile" has returned.
 *
 * Please read the comments in parseAFM.h and parseAFMclient.c.
 *
 * History:
 *  original: DSM  Thu Oct 20 17:39:59 PDT 1988
 *  modified: DSM  Mon Jul  3 14:17:50 PDT 1989
 *    - added 'storageProblem' return code
 *    - fixed bug of not allocating extra byte for string duplication
 *    - fixed typos
 *  modified: DSM  Tue Apr  3 11:18:34 PDT 1990
 *    - added free(ident) at end of parseFile routine
 *  modified: DSM  Tue Jun 19 10:16:29 PDT 1990
 *    - changed (width == 250) to (width = 250) in initializeArray
 */

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "parseAFM.hxx"
#include "vcl/strhelper.hxx"

#include "rtl/alloc.h"

#define lineterm EOL    /* line terminating character */
#define normalEOF 1 /* return code from parsing routines used only */
/* in this module */
#define False "false"   /* used in string comparison to check the value of */
/* boolean keys (e.g. IsFixedPitch)  */

#define MATCH(A,B)      (strncmp((A),(B), MAX_NAME) == 0)

namespace psp {

class FileInputStream
{
    char*         m_pMemory;
    unsigned int        m_nPos;
    unsigned int        m_nLen;
    public:
    explicit FileInputStream( const char* pFilename );
    ~FileInputStream();

    int getChar() { return (m_nPos < m_nLen) ? int(m_pMemory[m_nPos++]) : -1; }
    void ungetChar()
    {
        if( m_nPos > 0 )
            m_nPos--;
    }
};

FileInputStream::FileInputStream(const char* pFilename)
    : m_pMemory(nullptr)
    , m_nPos(0)
    , m_nLen(0)
{
    FILE* fp = fopen( pFilename, "r" );
    if( fp )
    {
        struct stat aStat;
        if (!fstat(fileno(fp), &aStat) && S_ISREG(aStat.st_mode) && aStat.st_size > 0)
        {
            m_pMemory = static_cast<char*>(rtl_allocateMemory( aStat.st_size ));
            m_nLen = (unsigned int)fread( m_pMemory, 1, aStat.st_size, fp );
        }
        fclose( fp );
    }
}

FileInputStream::~FileInputStream()
{
    rtl_freeMemory( m_pMemory );
}

/*************************** GLOBALS ***********************/
/* "shorts" for fast case statement
 * The values of each of these enumerated items correspond to an entry in the
 * table of strings defined below. Therefore, if you add a new string as
 * new keyword into the keyStrings table, you must also add a corresponding
 * parseKey AND it MUST be in the same position!
 *
 * IMPORTANT: since the sorting algorithm is a binary search, the strings of
 * keywords must be placed in lexicographical order, below. [Therefore, the
 * enumerated items are not necessarily in lexicographical order, depending
 * on the name chosen. BUT, they must be placed in the same position as the
 * corresponding key string.] The NOPE shall remain in the last position,
 * since it does not correspond to any key string, and it is used in the
 * "recognize" procedure to calculate how many possible keys there are.
 */

// some metrics have Ascent, Descent instead Ascender, Descender or Em
// which is not allowed per afm spcification, but let us handle
// this gently
enum parseKey {
    ASCENDER, ASCENT, CHARBBOX, CODE, COMPCHAR, CODEHEX, CAPHEIGHT, CHARWIDTH, CHARACTERSET, CHARACTERS, COMMENT,
    DESCENDER, DESCENT, EM, ENCODINGSCHEME, ENDCHARMETRICS, ENDCOMPOSITES, ENDDIRECTION,
    ENDFONTMETRICS, ENDKERNDATA, ENDKERNPAIRS, ENDTRACKKERN,
    FAMILYNAME, FONTBBOX, FONTNAME, FULLNAME, ISBASEFONT, ISFIXEDPITCH,
    ITALICANGLE, KERNPAIR, KERNPAIRXAMT, LIGATURE, MAPPINGSCHEME, METRICSSETS, CHARNAME,
    NOTICE, COMPCHARPIECE, STARTCHARMETRICS, STARTCOMPOSITES, STARTDIRECTION,
    STARTFONTMETRICS, STARTKERNDATA, STARTKERNPAIRS,
    STARTTRACKKERN, STDHW, STDVW, TRACKKERN, UNDERLINEPOSITION,
    UNDERLINETHICKNESS, VVECTOR, VERSION, XYWIDTH, X0WIDTH, XWIDTH, WEIGHT, XHEIGHT,
    NOPE
};

/*************************** PARSING ROUTINES **************/

/*************************** token *************************/

/*  A "AFM file Conventions" tokenizer. That means that it will
 *  return the next token delimited by white space.  See also
 *  the `linetoken' routine, which does a similar thing but
 *  reads all tokens until the next end-of-line.
 */

// token white space is ' ', '\n', '\r', ',', '\t', ';'
static const bool is_white_Array[ 256 ] =
{   false, false, false, false, false, false, false, false, // 0-7
    false,  true,  true, false, false,  true, false, false, // 8-15
    false, false, false, false, false, false, false, false, // 16-23
    false, false, false, false, false, false, false, false, // 24-31
     true, false, false, false, false, false, false, false, // 32-39
    false, false, false, false,  true, false, false, false, // 40-47
    false, false, false, false, false, false, false, false, // 48-55
    false, false, false,  true, false, false, false, false, // 56-63

    false, false, false, false, false, false, false, false, // 64 -
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, // 127

    false, false, false, false, false, false, false, false, // 128 -
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, // 191

    false, false, false, false, false, false, false, false, // 192 -
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, // 255
};
// token delimiters are ' ', '\n', '\r', '\t', ':', ';'
static const bool is_delimiter_Array[ 256 ] =
{   false, false, false, false, false, false, false, false, // 0-7
    false,  true,  true, false, false,  true, false, false, // 8-15
    false, false, false, false, false, false, false, false, // 16-23
    false, false, false, false, false, false, false, false, // 24-31
     true, false, false, false, false, false, false, false, // 32-39
    false, false, false, false, false, false, false, false, // 40-47
    false, false, false, false, false, false, false, false, // 48-55
    false, false,  true,  true, false, false, false, false, // 56-63

    false, false, false, false, false, false, false, false, // 64 -
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, // 127

    false, false, false, false, false, false, false, false, // 128 -
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, // 191

    false, false, false, false, false, false, false, false, // 192 -
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, // 255
};
static char *token( FileInputStream* stream, int& rLen )
{
    static char ident[MAX_NAME]; /* storage buffer for keywords */

    int ch, idx;

    /* skip over white space */
    // relies on EOF = -1
    while( is_white_Array[ (ch = stream->getChar()) & 255 ] )
        ;

    idx = 0;
    while( ch != -1 && ! is_delimiter_Array[ ch & 255 ] && idx < MAX_NAME-1 )
    {
        ident[idx++] = ch;
        ch = stream->getChar();
    }

    if (ch == -1 && idx < 1) return (nullptr);
    if (idx >= 1 && ch != ':' && ch != -1) stream->ungetChar();
    if (idx < 1 ) ident[idx++] = ch;    /* single-character token */
    ident[idx] = 0;
    rLen = idx;

    return ident;  /* returns pointer to the token */

} /* token */

/*************************** linetoken *************************/

/*  "linetoken" will get read all tokens until the EOL character from
 *  the given stream.  This is used to get any arguments that can be
 *  more than one word (like Comment lines and FullName).
 */

static char *linetoken( FileInputStream* stream )
{
    static char ident[MAX_NAME]; /* storage buffer for keywords */
    int ch, idx;

    while ((ch = stream->getChar()) == ' ' || ch == '\t' ) ;

    idx = 0;
    while (ch != -1 && ch != lineterm && ch != '\r' && idx < MAX_NAME-1 )
    {
        ident[idx++] = ch;
        ch = stream->getChar();
    } /* while */

    stream->ungetChar();
    ident[idx] = 0;

    return ident;  /* returns pointer to the token */

} /* linetoken */

/*************************** recognize *************************/

/*  This function tries to match a string to a known list of
 *  valid AFM entries (check the keyStrings array above).
 *  "ident" contains everything from white space through the
 *  next space, tab, or ":" character.
 *
 *  The algorithm is a standard Knuth binary search.
 */
#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-register"
#endif
#endif
#include "afm_hash.hpp"
#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic pop
#endif
#endif

static inline enum parseKey recognize( char* ident, int len)
{
    const hash_entry* pEntry = AfmKeywordHash::in_word_set( ident, len );
    return pEntry ? pEntry->eKey : NOPE;

} /* recognize */

/************************* parseGlobals *****************************/

/*  This function is called by "parseFile". It will parse the AFM file
 *  up to the "StartCharMetrics" keyword, which essentially marks the
 *  end of the Global Font Information and the beginning of the character
 *  metrics information.
 *
 *  If the caller of "parseFile" specified that it wanted the Global
 *  Font Information (as defined by the "AFM file Specification"
 *  document), then that information will be stored in the returned
 *  data structure.
 *
 *  Any Global Font Information entries that are not found in a
 *  given file, will have the usual default initialization value
 *  for its type (i.e. entries of type int will be 0, etc).
 *
 *  This function returns an error code specifying whether there was
 *  a premature EOF or a parsing error. This return value is used by
 *  parseFile to determine if there is more file to parse.
 */

static int parseGlobals( FileInputStream* fp, GlobalFontInfo* gfi )
{
    bool cont = true, save = (gfi != nullptr);
    int error = ok;
    int direction = -1;
    int tokenlen;

    while (cont)
    {
        char *keyword = token(fp, tokenlen);

        if (keyword == nullptr)
            /* Have reached an early and unexpected EOF. */
            /* Set flag and stop parsing */
        {
            error = earlyEOF;
            break;   /* get out of loop */
        }
        if (!save)
            /* get tokens until the end of the Global Font info section */
            /* without saving any of the data */
            switch (recognize(keyword, tokenlen))
            {
                case STARTCHARMETRICS:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                default:
                    break;
            } /* switch */
        else
            /* otherwise parse entire global font info section, */
            /* saving the data */
            switch(recognize(keyword, tokenlen))
            {
                case STARTFONTMETRICS:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->afmVersion = strdup( keyword );
                    break;
                case COMMENT:
                    linetoken(fp);
                    break;
                case FONTNAME:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->fontName = strdup( keyword );
                    break;
                case ENCODINGSCHEME:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->encodingScheme = strdup( keyword );
                    break;
                case FULLNAME:
                    if ((keyword = linetoken(fp)) != nullptr)
                        gfi->fullName = strdup( keyword );
                    break;
                case FAMILYNAME:
                    if ((keyword = linetoken(fp)) != nullptr)
                        gfi->familyName = strdup( keyword );
                    break;
                case WEIGHT:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->weight = strdup( keyword );
                    break;
                case ITALICANGLE:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->italicAngle = StringToDouble( keyword );
                    break;
                case ISFIXEDPITCH:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                    {
                        if (MATCH(keyword, False))
                            gfi->isFixedPitch = false;
                        else
                            gfi->isFixedPitch = true;
                    }
                    break;
                case UNDERLINEPOSITION:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->underlinePosition = atoi(keyword);
                    break;
                case UNDERLINETHICKNESS:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->underlineThickness = atoi(keyword);
                    break;
                case VERSION:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->version = strdup( keyword );
                    break;
                case NOTICE:
                    if ((keyword = linetoken(fp)) != nullptr)
                        gfi->notice = strdup( keyword );
                    break;
                case FONTBBOX:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->fontBBox.llx = atoi(keyword);
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->fontBBox.lly = atoi(keyword);
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->fontBBox.urx = atoi(keyword);
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->fontBBox.ury = atoi(keyword);
                    break;
                case CAPHEIGHT:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->capHeight = atoi(keyword);
                    break;
                case XHEIGHT:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->xHeight = atoi(keyword);
                    break;
                case DESCENT:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->descender = -atoi(keyword);
                    break;
                case DESCENDER:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->descender = atoi(keyword);
                    break;
                case ASCENT:
                case ASCENDER:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        gfi->ascender = atoi(keyword);
                    break;
                case STARTCHARMETRICS:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                case EM:
                    // skip one token
                    token(fp,tokenlen);
                    break;
                case STARTDIRECTION:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        direction = atoi(keyword);
                    break; /* ignore this for now */
                case ENDDIRECTION:
                    break; /* ignore this for now */
                case MAPPINGSCHEME:
                    token(fp,tokenlen);
                    break; /* ignore     this for now */
                case CHARACTERS:
                    token(fp,tokenlen);
                    break; /* ignore this for now */
                case ISBASEFONT:
                    token(fp,tokenlen);
                    break; /* ignore this for now */
                case CHARACTERSET:
                    token(fp,tokenlen); //ignore
                    break;
                case STDHW:
                    token(fp,tokenlen); //ignore
                    break;
                case STDVW:
                    token(fp,tokenlen); //ignore
                    break;
                case CHARWIDTH:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                    {
                        if (direction == 0)
                            gfi->charwidth = atoi(keyword);
                    }
                    token(fp,tokenlen);
                    /* ignore y-width for now */
                    break;
                case METRICSSETS:
                    token(fp,tokenlen); /*eat token*/
                    break; /* ignore this for now */
                case NOPE:
                default:
                    error = parseError;
                    break;
            } /* switch */
    } /* while */

    return error;

} /* parseGlobals */

/************************* parseCharWidths **************************/

/*  This function is called by "parseFile". It will parse the AFM file
 *  up to the "EndCharMetrics" keyword. It will save the character
 *  width info (as opposed to all of the character metric information)
 *  if requested by the caller of parseFile. Otherwise, it will just
 *  parse through the section without saving any information.
 *
 *  If data is to be saved, parseCharWidths is passed in a pointer
 *  to an array of widths that has already been initialized by the
 *  standard value for unmapped character codes. This function parses
 *  the Character Metrics section only storing the width information
 *  for the encoded characters into the array using the character code
 *  as the index into that array.
 *
 *  This function returns an error code specifying whether there was
 *  a premature EOF or a parsing error. This return value is used by
 *  parseFile to determine if there is more file to parse.
 */

static int parseCharWidths( FileInputStream* fp, int* cwi)
{
    bool cont = true, save = (cwi != nullptr);
    int pos = 0, error = ok, tokenlen;

    while (cont)
    {
        char *keyword = token(fp,tokenlen);
        /* Have reached an early and unexpected EOF. */
        /* Set flag and stop parsing */
        if (keyword == nullptr)
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        if (!save)
            /* get tokens until the end of the Char Metrics section without */
            /* saving any of the data*/
            switch (recognize(keyword,tokenlen))
            {
                case ENDCHARMETRICS:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                default:
                    break;
            } /* switch */
        else
            /* otherwise parse entire char metrics section, saving */
            /* only the char x-width info */
            switch(recognize(keyword,tokenlen))
            {
                case COMMENT:
                    linetoken(fp); /*eat token*/
                    break;
                case CODE:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        pos = atoi(keyword);
                    break;
                case XYWIDTH:
                    /* PROBLEM: Should be no Y-WIDTH when doing "quick & dirty" */
                    token(fp,tokenlen); token(fp,tokenlen); /* eat values */
                    error = parseError;
                    break;
                case CODEHEX:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        sscanf(keyword, "<%x>", &pos);
                    break;
                case X0WIDTH:
                    (void) token(fp,tokenlen);
                    break;
                case XWIDTH:
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        if (pos >= 0) /* ignore unmapped chars */
                            cwi[pos] = atoi(keyword);
                    break;
                case ENDCHARMETRICS:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                case CHARNAME:  /* eat values (so doesn't cause parseError) */
                    token(fp,tokenlen);
                    break;
                case CHARBBOX:
                    token(fp,tokenlen); token(fp,tokenlen);
                    token(fp,tokenlen); token(fp,tokenlen);
                    break;
                case LIGATURE:
                    token(fp,tokenlen); token(fp,tokenlen);
                    break;
                case VVECTOR:
                    token(fp,tokenlen); /*eat token*/
                    token(fp,tokenlen); /*eat token*/
                    break;
                case NOPE:
                default:
                    error = parseError;
                    break;
            } /* switch */
    } /* while */

    return error;

} /* parseCharWidths */

/*
 * number of char metrics is almost always inaccurate, so be gentle and try to
 * adapt our internal storage by adjusting the allocated list
 */

static int
reallocFontMetrics( void **pp_fontmetrics, int *p_oldcount, int n_newcount, unsigned int n_size )
{
    char *p_tmpmetrics = nullptr;

    if ((pp_fontmetrics == nullptr) || (*pp_fontmetrics == nullptr))
        return storageProblem;

    if (*p_oldcount == n_newcount)
        return ok;

    p_tmpmetrics = static_cast<char*>(realloc(*pp_fontmetrics, n_newcount * n_size));
    if (p_tmpmetrics == nullptr)
        return storageProblem;

    if ( n_newcount > *p_oldcount )
    {
        char *p_inimetrics = p_tmpmetrics + n_size * *p_oldcount;
        int   n_inimetrics = n_size * (n_newcount - *p_oldcount);
        memset( p_inimetrics, 0, n_inimetrics );
    }

    *pp_fontmetrics = p_tmpmetrics;
    *p_oldcount    = n_newcount;

    return ok;
}

static unsigned int
enlargeCount( unsigned int n_oldcount )
{
    unsigned int n_newcount = n_oldcount + n_oldcount / 5;
    if (n_oldcount == n_newcount )
        n_newcount = n_oldcount + 5;

    return n_newcount;
}

/************************* parseCharMetrics ************************/

/*  This function is called by parseFile if the caller of parseFile
 *  requested that all character metric information be saved
 *  (as opposed to only the character width information).
 *
 *  parseCharMetrics is passed in a pointer to an array of records
 *  to hold information on a per character basis. This function
 *  parses the Character Metrics section storing all character
 *  metric information for the ALL characters (mapped and unmapped)
 *  into the array.
 *
 *  This function returns an error code specifying whether there was
 *  a premature EOF or a parsing error. This return value is used by
 *  parseFile to determine if there is more file to parse.
 */

static int parseCharMetrics( FileInputStream* fp, FontInfo* fi)
{
    bool cont = true, firstTime = true;
    int error = ok, count = 0, tokenlen;
    CharMetricInfo *temp = fi->cmi;

    while (cont)
    {
        char *keyword = token(fp,tokenlen);
        if (keyword == nullptr)
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        switch(recognize(keyword,tokenlen))
        {
            case COMMENT:
                linetoken(fp); /*eat token*/
                break;
            case CODE:
                if (!(count < fi->numOfChars))
                {
                    reallocFontMetrics( reinterpret_cast<void**>(&fi->cmi),
                                        &(fi->numOfChars), enlargeCount(fi->numOfChars),
                                        sizeof(CharMetricInfo) );
                    temp = &(fi->cmi[ count - 1 ]);
                }
                if (count < fi->numOfChars)
                {
                    if (firstTime) firstTime = false;
                    else temp++;
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        temp->code = atoi(keyword);
                    if (fi->gfi && fi->gfi->charwidth)
                        temp->wx = fi->gfi->charwidth;
                    count++;
                }
                else
                {
                    error = parseError;
                    cont = false;
                }
                break;
            case CODEHEX:
                if (!(count < fi->numOfChars ))
                {
                    reallocFontMetrics( reinterpret_cast<void**>(&fi->cmi),
                                        &(fi->numOfChars), enlargeCount(fi->numOfChars),
                                        sizeof(CharMetricInfo) );
                    temp = &(fi->cmi[ count - 1 ]);
                }
                if (count < fi->numOfChars) {
                    if (firstTime)
                        firstTime = false;
                    else
                        temp++;
                    if ((keyword = token(fp,tokenlen)) != nullptr)
                        sscanf(keyword,"<%x>", &temp->code);
                    if (fi->gfi && fi->gfi->charwidth)
                        temp->wx = fi->gfi->charwidth;
                    count++;
                }
                else {
                    error = parseError;
                    cont = false;
                }
                break;
            case XYWIDTH:
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    temp->wx = atoi(keyword);
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    temp->wy = atoi(keyword);
                break;
            case X0WIDTH:
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    temp->wx = atoi(keyword);
                break;
            case XWIDTH:
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    temp->wx = atoi(keyword);
                break;
            case CHARNAME:
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    temp->name = strdup(keyword);
                break;
            case CHARBBOX:
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    temp->charBBox.llx = atoi(keyword);
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    temp->charBBox.lly = atoi(keyword);
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    temp->charBBox.urx = atoi(keyword);
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    temp->charBBox.ury = atoi(keyword);
                break;
            case LIGATURE: {
                Ligature **tail = &(temp->ligs);
                Ligature *node = *tail;

                if (*tail != nullptr)
                {
                    while (node->next != nullptr)
                        node = node->next;
                    tail = &(node->next);
                }

                *tail = static_cast<Ligature *>(calloc(1, sizeof(Ligature)));
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    (*tail)->succ = strdup(keyword);
                if ((keyword = token(fp,tokenlen)) != nullptr)
                    (*tail)->lig = strdup(keyword);
                break; }
            case ENDCHARMETRICS:
                cont = false;
                break;
            case ENDFONTMETRICS:
                cont = false;
                error = normalEOF;
                break;
            case VVECTOR:
                token(fp,tokenlen); /*eat token*/
                token(fp,tokenlen); /*eat token*/
                break;
            case NOPE:
            default:
                error = parseError;
                break;
        } /* switch */
    } /* while */

    if ((error == ok) && (count != fi->numOfChars))
        error = reallocFontMetrics( reinterpret_cast<void**>(&fi->cmi), &(fi->numOfChars),
                                    count, sizeof(CharMetricInfo) );

    if ((error == ok) && (count != fi->numOfChars))
        error = parseError;

    return error;

} /* parseCharMetrics */

/************************* parseTrackKernData ***********************/

/*  This function is called by "parseFile". It will parse the AFM file
 *  up to the "EndTrackKern" or "EndKernData" keywords. It will save the
 *  track kerning data if requested by the caller of parseFile.
 *
 *  parseTrackKernData is passed in a pointer to the FontInfo record.
 *  If data is to be saved, the FontInfo record will already contain
 *  a valid pointer to storage for the track kerning data.
 *
 *  This function returns an error code specifying whether there was
 *  a premature EOF or a parsing error. This return value is used by
 *  parseFile to determine if there is more file to parse.
 */

static int parseTrackKernData( FileInputStream* fp, FontInfo* fi)
{
    bool cont = true, save = (fi->tkd != nullptr);
    int pos = 0, error = ok, tcount = 0, tokenlen;

    while (cont)
    {
        char *keyword = token(fp,tokenlen);

        if (keyword == nullptr)
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        if (!save)
            /* get tokens until the end of the Track Kerning Data */
            /* section without saving any of the data */
            switch(recognize(keyword,tokenlen))
            {
                case ENDTRACKKERN:
                case ENDKERNDATA:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                default:
                    break;
            } /* switch */
        else
            /* otherwise parse entire Track Kerning Data section, */
            /* saving the data */
            switch(recognize(keyword,tokenlen))
            {
                case COMMENT:
                    linetoken(fp); /*eat token*/
                    break;
                case TRACKKERN:
                    if (!(tcount < fi->numOfTracks))
                    {
                        reallocFontMetrics( reinterpret_cast<void**>(&fi->tkd), &(fi->numOfTracks),
                                            enlargeCount(fi->numOfTracks), sizeof(TrackKernData) );
                    }

                    if (tcount < fi->numOfTracks)
                    {
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->tkd[pos].degree = atoi(keyword);
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->tkd[pos].minPtSize = StringToDouble(keyword);
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->tkd[pos].minKernAmt = StringToDouble(keyword);
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->tkd[pos].maxPtSize = StringToDouble(keyword);
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->tkd[pos++].maxKernAmt = StringToDouble(keyword);
                        tcount++;
                    }
                    else
                    {
                        error = parseError;
                        cont = false;
                    }
                    break;
                case ENDTRACKKERN:
                case ENDKERNDATA:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                case NOPE:
                default:
                    error = parseError;
                    break;
            } /* switch */
    } /* while */

    if (error == ok && tcount != fi->numOfTracks)
        error = reallocFontMetrics( reinterpret_cast<void**>(&fi->tkd), &(fi->numOfTracks),
                                    tcount, sizeof(TrackKernData) );

    if (error == ok && tcount != fi->numOfTracks)
        error = parseError;

    return error;

} /* parseTrackKernData */

/************************* parsePairKernData ************************/

/*  This function is called by "parseFile". It will parse the AFM file
 *  up to the "EndKernPairs" or "EndKernData" keywords. It will save
 *  the pair kerning data if requested by the caller of parseFile.
 *
 *  parsePairKernData is passed in a pointer to the FontInfo record.
 *  If data is to be saved, the FontInfo record will already contain
 *  a valid pointer to storage for the pair kerning data.
 *
 *  This function returns an error code specifying whether there was
 *  a premature EOF or a parsing error. This return value is used by
 *  parseFile to determine if there is more file to parse.
 */

static int parsePairKernData( FileInputStream* fp, FontInfo* fi)
{
    bool cont = true, save = (fi->pkd != nullptr);
    int pos = 0, error = ok, pcount = 0, tokenlen;

    while (cont)
    {
        char *keyword = token(fp,tokenlen);

        if (keyword == nullptr)
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        if (!save)
            /* get tokens until the end of the Pair Kerning Data */
            /* section without saving any of the data */
            switch(recognize(keyword,tokenlen))
            {
                case ENDKERNPAIRS:
                case ENDKERNDATA:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                default:
                    break;
            } /* switch */
        else
            /* otherwise parse entire Pair Kerning Data section, */
            /* saving the data */
            switch(recognize(keyword,tokenlen))
            {
                case COMMENT:
                    linetoken(fp); /*eat token*/
                    break;
                case KERNPAIR:
                    if (!(pcount < fi->numOfPairs))
                    {
                        reallocFontMetrics( reinterpret_cast<void**>(&fi->pkd), &(fi->numOfPairs),
                                            enlargeCount(fi->numOfPairs), sizeof(PairKernData) );
                    }
                    if (pcount < fi->numOfPairs)
                    {
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->pkd[pos].name1 = strdup( keyword );
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->pkd[pos].name2 = strdup( keyword );
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->pkd[pos].xamt = atoi(keyword);
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->pkd[pos++].yamt = atoi(keyword);
                        pcount++;
                    }
                    else
                    {
                        error = parseError;
                        cont = false;
                    }
                    break;
                case KERNPAIRXAMT:
                    if (!(pcount < fi->numOfPairs))
                    {
                        reallocFontMetrics( reinterpret_cast<void**>(&fi->pkd), &(fi->numOfPairs),
                                            enlargeCount(fi->numOfPairs), sizeof(PairKernData) );
                    }
                    if (pcount < fi->numOfPairs)
                    {
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->pkd[pos].name1 = strdup( keyword );
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->pkd[pos].name2 = strdup( keyword );
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->pkd[pos++].xamt = atoi(keyword);
                        pcount++;
                    }
                    else
                    {
                        error = parseError;
                        cont = false;
                    }
                    break;
                case ENDKERNPAIRS:
                case ENDKERNDATA:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                case NOPE:
                default:
                    error = parseError;
                    break;
            } /* switch */
    } /* while */

    if ((error == ok) && (pcount != fi->numOfPairs))
        error = reallocFontMetrics( reinterpret_cast<void**>(&fi->pkd), &(fi->numOfPairs),
                                    pcount, sizeof(PairKernData) );

    if (error == ok && pcount != fi->numOfPairs)
        error = parseError;

    return error;

} /* parsePairKernData */

/************************* parseCompCharData **************************/

/*  This function is called by "parseFile". It will parse the AFM file
 *  up to the "EndComposites" keyword. It will save the composite
 *  character data if requested by the caller of parseFile.
 *
 *  parseCompCharData is passed in a pointer to the FontInfo record, and
 *  a boolean representing if the data should be saved.
 *
 *  This function will create the appropriate amount of storage for
 *  the composite character data and store a pointer to the storage
 *  in the FontInfo record.
 *
 *  This function returns an error code specifying whether there was
 *  a premature EOF or a parsing error. This return value is used by
 *  parseFile to determine if there is more file to parse.
 */

static int parseCompCharData( FileInputStream* fp, FontInfo* fi)
{
    bool cont = true, firstTime = true, save = (fi->ccd != nullptr);
    int pos = 0, j = 0, error = ok, ccount = 0, pcount = 0, tokenlen;

    while (cont)
    {
        char *keyword = token(fp,tokenlen);
        if (keyword == nullptr)
            /* Have reached an early and unexpected EOF. */
            /* Set flag and stop parsing */
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        if (ccount > fi->numOfComps)
        {
            reallocFontMetrics( reinterpret_cast<void**>(&fi->ccd), &(fi->numOfComps),
                                enlargeCount(fi->numOfComps), sizeof(CompCharData) );
        }
        if (ccount > fi->numOfComps)
        {
            error = parseError;
            break; /* get out of loop */
        }
        if (!save)
            /* get tokens until the end of the Composite Character info */
            /* section without saving any of the data */
            switch(recognize(keyword,tokenlen))
            {
                case ENDCOMPOSITES:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                case COMMENT:
                case COMPCHAR:
                    linetoken(fp);
                    break;
                default:
                    break;
            } /* switch */
        else
            /* otherwise parse entire Composite Character info section, */
            /* saving the data */
            switch(recognize(keyword,tokenlen))
            {
                case COMMENT:
                    linetoken(fp); /*eat token*/
                    break;
                case COMPCHAR:
                    if (!(ccount < fi->numOfComps))
                    {
                        reallocFontMetrics( reinterpret_cast<void**>(&fi->ccd), &(fi->numOfComps),
                                            enlargeCount(fi->numOfComps), sizeof(CompCharData) );
                    }
                    if (ccount < fi->numOfComps)
                    {
                        keyword = token(fp,tokenlen);
                        if (pcount != fi->ccd[pos].numOfPieces)
                            error = parseError;
                        pcount = 0;
                        if (firstTime) firstTime = false;
                        else pos++;
                        fi->ccd[pos].ccName = strdup( keyword );
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->ccd[pos].numOfPieces = atoi(keyword);
                        fi->ccd[pos].pieces = static_cast<Pcc *>(
                            calloc(fi->ccd[pos].numOfPieces, sizeof(Pcc)));
                        j = 0;
                        ccount++;
                    }
                    else
                    {
                        error = parseError;
                        cont = false;
                    }
                    break;
                case COMPCHARPIECE:
                    if (pcount < fi->ccd[pos].numOfPieces)
                    {
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->ccd[pos].pieces[j].pccName = strdup( keyword );
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->ccd[pos].pieces[j].deltax = atoi(keyword);
                        if ((keyword = token(fp,tokenlen)) != nullptr)
                            fi->ccd[pos].pieces[j++].deltay = atoi(keyword);
                        pcount++;
                    }
                    else
                        error = parseError;
                    break;
                case ENDCOMPOSITES:
                    cont = false;
                    break;
                case ENDFONTMETRICS:
                    cont = false;
                    error = normalEOF;
                    break;
                case NOPE:
                default:
                    error = parseError;
                    break;
            } /* switch */
    } /* while */

    if (error == ok && ccount != fi->numOfComps)
        reallocFontMetrics( reinterpret_cast<void**>(&fi->ccd), &(fi->numOfComps),
                            ccount, sizeof(CompCharData) );

    if (error == ok && ccount != fi->numOfComps)
        error = parseError;

    return error;

} /* parseCompCharData */

/*************************** 'PUBLIC' FUNCTION ********************/

/*************************** parseFile *****************************/

/*  parseFile is the only 'public' procedure available. It is called
 *  from an application wishing to get information from an AFM file.
 *  The caller of this function is responsible for locating and opening
 *  an AFM file and handling all errors associated with that task.
 *
 *  parseFile expects 3 parameters: a filename pointer, a pointer
 *  to a (FontInfo *) variable (for which storage will be allocated and
 *  the data requested filled in), and a mask specifying which
 *  data from the AFM file should be saved in the FontInfo structure.
 *
 *  The file will be parsed and the requested data will be stored in
 *  a record of type FontInfo (refer to ParseAFM.h).
 *
 *  parseFile returns an error code as defined in parseAFM.h.
 *
 *  The position of the read/write pointer associated with the file
 *  pointer upon return of this function is undefined.
 */

int parseFile( const char* pFilename, FontInfo** fi, FLAGS flags)
{
    FileInputStream aFile( pFilename );

    int code = ok;  /* return code from each of the parsing routines */
    int error = ok; /* used as the return code from this function */
    int tokenlen;

    char *keyword; /* used to store a token */

    (*fi) = static_cast<FontInfo *>(calloc(1, sizeof(FontInfo)));
    if ((*fi) == nullptr) { error = storageProblem; return error; }

    if (flags & P_G)
    {
        (*fi)->gfi = static_cast<GlobalFontInfo *>(calloc(1, sizeof(GlobalFontInfo)));
        if ((*fi)->gfi == nullptr) { error = storageProblem; return error; }
    }

    /* The AFM file begins with Global Font Information. This section */
    /* will be parsed whether or not information should be saved. */
    code = parseGlobals(&aFile, (*fi)->gfi);

    if (code < 0) error = code;

    /* The Global Font Information is followed by the Character Metrics */
    /* section. Which procedure is used to parse this section depends on */
    /* how much information should be saved. If all of the metrics info */
    /* is wanted, parseCharMetrics is called. If only the character widths */
    /* is wanted, parseCharWidths is called. parseCharWidths will also */
    /* be called in the case that no character data is to be saved, just */
    /* to parse through the section. */

    if ((code != normalEOF) && (code != earlyEOF))
    {
        if ((keyword = token(&aFile,tokenlen)) != nullptr)
            (*fi)->numOfChars = atoi(keyword);
        if (flags & (P_M ^ P_W))
        {
            (*fi)->cmi = static_cast<CharMetricInfo *>(
                calloc((*fi)->numOfChars, sizeof(CharMetricInfo)));
            if ((*fi)->cmi == nullptr) { error = storageProblem; return error; }
            code = parseCharMetrics(&aFile, *fi);
        }
        else
        {
            if (flags & P_W)
            {
                (*fi)->cwi = static_cast<int *>(calloc(256, sizeof(int)));
                if ((*fi)->cwi == nullptr)
                {
                    error = storageProblem;
                    return error;
                }
            }
            /* parse section regardless */
            code = parseCharWidths(&aFile, (*fi)->cwi);
        } /* else */
    } /* if */

    if ((error != earlyEOF) && (code < 0)) error = code;

    /* The remaining sections of the AFM are optional. This code will */
    /* look at the next keyword in the file to determine what section */
    /* is next, and then allocate the appropriate amount of storage */
    /* for the data (if the data is to be saved) and call the */
    /* appropriate parsing routine to parse the section. */

    while ((code != normalEOF) && (code != earlyEOF))
    {
        keyword = token(&aFile,tokenlen);
        if (keyword == nullptr)
            /* Have reached an early and unexpected EOF. */
            /* Set flag and stop parsing */
        {
            code = earlyEOF;
            break; /* get out of loop */
        }
        switch(recognize(keyword,tokenlen))
        {
            case STARTKERNDATA:
                break;
            case ENDKERNDATA:
                break;
            case STARTTRACKKERN:
                keyword = token(&aFile,tokenlen);
                if ((flags & P_T) && keyword)
                {
                    (*fi)->numOfTracks = atoi(keyword);
                    (*fi)->tkd = static_cast<TrackKernData *>(
                        calloc((*fi)->numOfTracks, sizeof(TrackKernData)));
                    if ((*fi)->tkd == nullptr)
                    {
                        error = storageProblem;
                        return error;
                    }
                } /* if */
                code = parseTrackKernData(&aFile, *fi);
                break;
            case STARTKERNPAIRS:
                keyword = token(&aFile,tokenlen);
                if ((flags & P_P) && keyword)
                {
                    (*fi)->numOfPairs = atoi(keyword);
                    (*fi)->pkd = static_cast<PairKernData *>(
                        calloc((*fi)->numOfPairs, sizeof(PairKernData)));
                    if ((*fi)->pkd == nullptr)
                    {
                        error = storageProblem;
                        return error;
                    }
                } /* if */
                code = parsePairKernData(&aFile, *fi);
                break;
            case STARTCOMPOSITES:
                keyword = token(&aFile,tokenlen);
                if ((flags & P_C) && keyword)
                {
                    (*fi)->numOfComps = atoi(keyword);
                    (*fi)->ccd = static_cast<CompCharData *>(
                        calloc((*fi)->numOfComps, sizeof(CompCharData)));
                    if ((*fi)->ccd == nullptr)
                    {
                        error = storageProblem;
                        return error;
                    }
                } /* if */
                code = parseCompCharData(&aFile, *fi);
                break;
            case ENDFONTMETRICS:
                code = normalEOF;
                break;
            case COMMENT:
                linetoken(&aFile);
                break;
            case NOPE:
            default:
                code = parseError;
                break;
        } /* switch */

        if ((error != earlyEOF) && (code < 0)) error = code;

    } /* while */

    if ((error != earlyEOF) && (code < 0)) error = code;

    return error;

} /* parseFile */

void
freeFontInfo (FontInfo *fi)
{
    int i;

    if (fi->gfi)
    {
        free (fi->gfi->afmVersion);
        free (fi->gfi->fontName);
        free (fi->gfi->fullName);
        free (fi->gfi->familyName);
        free (fi->gfi->weight);
        free (fi->gfi->version);
        free (fi->gfi->notice);
        free (fi->gfi->encodingScheme);
        free (fi->gfi);
    }

    free (fi->cwi);

    if (fi->cmi)
    {
        for (i = 0; i < fi->numOfChars; i++)
        {
            Ligature *ligs;
            free (fi->cmi[i].name);
            ligs = fi->cmi[i].ligs;
            while (ligs)
            {
                Ligature *tmp;
                tmp = ligs;
                ligs = ligs->next;
                free (tmp->succ);
                free (tmp->lig);
                free (tmp);
            }
        }
        free (fi->cmi);
    }

    free (fi->tkd);

    if (fi->pkd)
    {
        for ( i = 0; i < fi->numOfPairs; i++)
        {
            free (fi->pkd[i].name1);
            free (fi->pkd[i].name2);
        }
        free (fi->pkd);
    }

    if (fi->ccd)
    {
        for (i = 0; i < fi->numOfComps; i++)
        {
            free (fi->ccd[i].ccName);
            int j;
            for (j = 0; j < fi->ccd[i].numOfPieces; j++)
                free (fi->ccd[i].pieces[j].pccName);

            free (fi->ccd[i].pieces);
        }
        free (fi->ccd);
    }

    free (fi);
}

} // namspace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
