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
 */

/*************************************************************************
 *
 *  $RCSfile: parseAFM.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pl $ $Date: 2002-07-20 15:21:19 $
 *
 ************************************************************************/

/* parseAFM.c
 *
 * This file is used in conjuction with the parseAFM.h header file.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/file.h>
#include <math.h>
#include <parseAFM.hxx>
#include <psprint/strhelper.hxx>

#define lineterm EOL    /* line terminating character */
#define normalEOF 1 /* return code from parsing routines used only */
/* in this module */
#define Space "space"   /* used in string comparison to look for the width */
/* of the space character to init the widths array */
#define False "false"   /* used in string comparison to check the value of */
/* boolean keys (e.g. IsFixedPitch)  */

#define MATCH(A,B)      (strncmp((A),(B), MAX_NAME) == 0)

namespace psp {

/*************************** GLOBALS ***********************/

static char *ident = NULL; /* storage buffer for keywords */


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

/* keywords for the system:
 * This a table of all of the current strings that are vaild AFM keys.
 * Each entry can be referenced by the appropriate parseKey value (an
 * enumerated data type defined above). If you add a new keyword here,
 * a corresponding parseKey MUST be added to the enumerated data type
 * defined above, AND it MUST be added in the same position as the
 * string is in this table.
 *
 * IMPORTANT: since the sorting algorithm is a binary search, the keywords
 * must be placed in lexicographical order. And, NULL should remain at the
 * end.
 */

static char    *keyStrings[] = {
    "Ascender", "Ascent", "B", "C", "CC", "CH", "CapHeight", "CharWidth", "CharacterSet" ,"Characters", "Comment",
    "Descender", "Descent", "Em", "EncodingScheme", "EndCharMetrics", "EndComposites", "EndDirection",
    "EndFontMetrics", "EndKernData", "EndKernPairs", "EndTrackKern",
    "FamilyName", "FontBBox", "FontName", "FullName", "IsBaseFont", "IsFixedPitch",
    "ItalicAngle", "KP", "KPX", "L", "MappingScheme", "MetricsSets", "N",
    "Notice", "PCC", "StartCharMetrics", "StartComposites", "StartDirection",
    "StartFontMetrics", "StartKernData", "StartKernPairs",
    "StartTrackKern", "StdHW", "StdVW", "TrackKern", "UnderlinePosition",
    "UnderlineThickness", "V", "Version", "W", "W0X", "WX", "Weight", "XHeight",
    NULL};

/*************************** PARSING ROUTINES **************/

/*************************** token *************************/

/*  A "AFM File Conventions" tokenizer. That means that it will
 *  return the next token delimited by white space.  See also
 *  the `linetoken' routine, which does a similar thing but
 *  reads all tokens until the next end-of-line.
 */

static char *token( FILE* stream )
{
    int ch, idx;

    /* skip over white space */
    while ((ch = fgetc(stream)) == ' ' || ch == lineterm || ch == '\r' ||
           ch == ',' || ch == '\t' || ch == ';');

    idx = 0;
    while (ch != EOF && ch != ' ' && ch != lineterm && ch != '\r'
           && ch != '\t' && ch != ':' && ch != ';')
    {
        ident[idx++] = ch;
        ch = fgetc(stream);
    } /* while */

    if (ch == EOF && idx < 1) return ((char *)NULL);
    if (idx >= 1 && ch != ':' ) ungetc(ch, stream);
    if (idx < 1 ) ident[idx++] = ch;    /* single-character token */
    ident[idx] = 0;

    return(ident);  /* returns pointer to the token */

} /* token */


/*************************** linetoken *************************/

/*  "linetoken" will get read all tokens until the EOL character from
 *  the given stream.  This is used to get any arguments that can be
 *  more than one word (like Comment lines and FullName).
 */

static char *linetoken( FILE* stream )
{
    int ch, idx;

    while ((ch = fgetc(stream)) == ' ' || ch == '\t' );

    idx = 0;
    while (ch != EOF && ch != lineterm && ch != '\r')
    {
        ident[idx++] = ch;
        ch = fgetc(stream);
    } /* while */

    ungetc(ch, stream);
    ident[idx] = 0;

    return(ident);  /* returns pointer to the token */

} /* linetoken */


/*************************** recognize *************************/

/*  This function tries to match a string to a known list of
 *  valid AFM entries (check the keyStrings array above).
 *  "ident" contains everything from white space through the
 *  next space, tab, or ":" character.
 *
 *  The algorithm is a standard Knuth binary search.
 */

static enum parseKey recognize( register char* ident)
{
    int lower = 0, upper = (int) NOPE, midpoint, cmpvalue;
    bool found = false;

    while ((upper >= lower) && !found)
    {
        midpoint = (lower + upper)/2;
        if (keyStrings[midpoint] == NULL) break;
        cmpvalue = strncmp(ident, keyStrings[midpoint], MAX_NAME);
        if (cmpvalue == 0) found = true;
        else if (cmpvalue < 0) upper = midpoint - 1;
        else lower = midpoint + 1;
    } /* while */

    if (found) return (enum parseKey) midpoint;
    else return NOPE;

} /* recognize */


/************************* parseGlobals *****************************/

/*  This function is called by "parseFile". It will parse the AFM File
 *  up to the "StartCharMetrics" keyword, which essentially marks the
 *  end of the Global Font Information and the beginning of the character
 *  metrics information.
 *
 *  If the caller of "parseFile" specified that it wanted the Global
 *  Font Information (as defined by the "AFM File Specification"
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

static int parseGlobals( FILE* fp, register GlobalFontInfo* gfi )
{
    bool cont = true, save = (gfi != NULL);
    int error = ok;
    register char *keyword;
    int direction = -1;

    while (cont)
    {
        keyword = token(fp);

        if (keyword == NULL)
            /* Have reached an early and unexpected EOF. */
            /* Set flag and stop parsing */
        {
            error = earlyEOF;
            break;   /* get out of loop */
        }
        if (!save)
            /* get tokens until the end of the Global Font info section */
            /* without saving any of the data */
            switch (recognize(keyword))
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
            switch(recognize(keyword))
            {
                case STARTFONTMETRICS:
                    keyword = token(fp);
                    gfi->afmVersion = strdup( keyword );
                    break;
                case COMMENT:
                    keyword = linetoken(fp);
                    break;
                case FONTNAME:
                    keyword = token(fp);
                    gfi->fontName = strdup( keyword );
                    break;
                case ENCODINGSCHEME:
                    keyword = token(fp);
                    gfi->encodingScheme = strdup( keyword );
                    break;
                case FULLNAME:
                    keyword = linetoken(fp);
                    gfi->fullName = strdup( keyword );
                    break;
                case FAMILYNAME:
                    keyword = linetoken(fp);
                    gfi->familyName = strdup( keyword );
                    break;
                case WEIGHT:
                    keyword = token(fp);
                    gfi->weight = strdup( keyword );
                    break;
                case ITALICANGLE:
                    keyword = token(fp);
                    gfi->italicAngle = StringToDouble( keyword );
                    break;
                case ISFIXEDPITCH:
                    keyword = token(fp);
                    if (MATCH(keyword, False))
                        gfi->isFixedPitch = 0;
                    else
                        gfi->isFixedPitch = 1;
                    break;
                case UNDERLINEPOSITION:
                    keyword = token(fp);
                    gfi->underlinePosition = atoi(keyword);
                    break;
                case UNDERLINETHICKNESS:
                    keyword = token(fp);
                    gfi->underlineThickness = atoi(keyword);
                    break;
                case VERSION:
                    keyword = token(fp);
                    gfi->version = strdup( keyword );
                    break;
                case NOTICE:
                    keyword = linetoken(fp);
                    gfi->notice = strdup( keyword );
                    break;
                case FONTBBOX:
                    keyword = token(fp);
                    gfi->fontBBox.llx = atoi(keyword);
                    keyword = token(fp);
                    gfi->fontBBox.lly = atoi(keyword);
                    keyword = token(fp);
                    gfi->fontBBox.urx = atoi(keyword);
                    keyword = token(fp);
                    gfi->fontBBox.ury = atoi(keyword);
                    break;
                case CAPHEIGHT:
                    keyword = token(fp);
                    gfi->capHeight = atoi(keyword);
                    break;
                case XHEIGHT:
                    keyword = token(fp);
                    gfi->xHeight = atoi(keyword);
                    break;
                case DESCENT:
                    keyword = token(fp);
                    gfi->descender = -atoi(keyword);
                    break;
                case DESCENDER:
                    keyword = token(fp);
                    gfi->descender = atoi(keyword);
                    break;
                case ASCENT:
                case ASCENDER:
                    keyword = token(fp);
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
                    keyword = token(fp);
                    break;
                case STARTDIRECTION:
                    keyword = token(fp);
                    direction = atoi(keyword);
                    break; /* ignore this for now */
                case ENDDIRECTION:
                    break; /* ignore this for now */
                case MAPPINGSCHEME:
                    keyword = token(fp);
                    break; /* ignore     this for now */
                case CHARACTERS:
                    keyword = token(fp);
                    break; /* ignore this for now */
                case ISBASEFONT:
                    keyword = token(fp);
                    break; /* ignore this for now */
                case CHARACTERSET:
                    keyword=token(fp); //ignore
                    break;
                case STDHW:
                    keyword=token(fp); //ignore
                    break;
                case STDVW:
                    keyword=token(fp); //ignore
                    break;
                case CHARWIDTH:
                    keyword = token(fp);
                    if (direction == 0)
                        gfi->charwidth = atoi(keyword);
                    keyword = token(fp);
                    /* ignore y-width for now */
                    break;
                case METRICSSETS:
                    keyword = token(fp);
                    break; /* ignore this for now */
                case NOPE:
                default:
                    error = parseError;
                    break;
            } /* switch */
    } /* while */

    return(error);

} /* parseGlobals */



/************************* initializeArray ************************/

/*  Unmapped character codes are (at Adobe Systems) assigned the
 *  width of the space character (if one exists) else they get the
 *  value of 250 ems. This function initializes all entries in the
 *  char widths array to have this value. Then any mapped character
 *  codes will be replaced with the width of the appropriate character
 *  when parsing the character metric section.

 *  This function parses the Character Metrics Section looking
 *  for a space character (by comparing character names). If found,
 *  the width of the space character will be used to initialize the
 *  values in the array of character widths.
 *
 *  Before returning, the position of the read/write pointer of the
 *  file is reset to be where it was upon entering this function.
 */

static int initializeArray( FILE* fp, register int* cwi)
{
    bool cont = true, found = false;
    long opos = ftell(fp);
    int code = 0, width = 0, i = 0, error = 0;
    register char *keyword;

    while (cont)
    {
        keyword = token(fp);
        if (keyword == NULL)
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        switch(recognize(keyword))
        {
            case COMMENT:
                keyword = linetoken(fp);
                break;
            case CODE:
                code = atoi(token(fp));
                break;
            case CODEHEX:
                sscanf(token(fp),"<%x>", &code);
                break;
            case XWIDTH:
                width = atoi(token(fp));
                break;
            case X0WIDTH:
                (void) token(fp);
                break;
            case CHARNAME:
                keyword = token(fp);
                if (MATCH(keyword, Space))
                {
                    cont = false;
                    found = true;
                }
                break;
            case ENDCHARMETRICS:
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

    if (!found)
        width = 250;

    for (i = 0; i < 256; ++i)
        cwi[i] = width;

    fseek(fp, opos, 0);

    return(error);

} /* initializeArray */


/************************* parseCharWidths **************************/

/*  This function is called by "parseFile". It will parse the AFM File
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

static int parseCharWidths( FILE* fp, register int* cwi)
{
    bool cont = true, save = (cwi != NULL);
    int pos = 0, error = ok;
    register char *keyword;

    while (cont)
    {
        keyword = token(fp);
        /* Have reached an early and unexpected EOF. */
        /* Set flag and stop parsing */
        if (keyword == NULL)
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        if (!save)
            /* get tokens until the end of the Char Metrics section without */
            /* saving any of the data*/
            switch (recognize(keyword))
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
            switch(recognize(keyword))
            {
                case COMMENT:
                    keyword = linetoken(fp);
                    break;
                case CODE:
                    keyword = token(fp);
                    pos = atoi(keyword);
                    break;
                case XYWIDTH:
                    /* PROBLEM: Should be no Y-WIDTH when doing "quick & dirty" */
                    keyword = token(fp); keyword = token(fp); /* eat values */
                    error = parseError;
                    break;
                case CODEHEX:
                    keyword = token(fp);
                    sscanf(keyword, "<%x>", &pos);
                    break;
                case X0WIDTH:
                    (void) token(fp);
                    break;
                case XWIDTH:
                    keyword = token(fp);
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
                    keyword = token(fp);
                    break;
                case CHARBBOX:
                    keyword = token(fp); keyword = token(fp);
                    keyword = token(fp); keyword = token(fp);
                    break;
                case LIGATURE:
                    keyword = token(fp); keyword = token(fp);
                    break;
                case VVECTOR:
                    keyword = token(fp);
                    keyword = token(fp);
                    break;
                case NOPE:
                default:
                    error = parseError;
                    break;
            } /* switch */
    } /* while */

    return(error);

} /* parseCharWidths */


/*
 * number of char metrics is almost allways inaccurate, so be gentle and try to
 * adapt our internal storage by adjusting the allocated list
 */

static int
reallocFontMetrics( void **pp_fontmetrics, int *p_oldcount, unsigned int n_newcount,
                    unsigned int n_size )
{
    char *p_tmpmetrics = NULL;

    if ((pp_fontmetrics == NULL) || (*pp_fontmetrics == NULL))
        return storageProblem;

    if (*p_oldcount == n_newcount)
        return ok;

    p_tmpmetrics = (char*)realloc(*pp_fontmetrics, n_newcount * n_size);
    if (p_tmpmetrics == NULL)
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

static int parseCharMetrics( FILE* fp, register FontInfo* fi)
{
    bool cont = true, firstTime = true;
    int error = ok, count = 0;
    register CharMetricInfo *temp = fi->cmi;
    register char *keyword;

    while (cont)
    {
        keyword = token(fp);
        if (keyword == NULL)
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        switch(recognize(keyword))
        {
            case COMMENT:
                keyword = linetoken(fp);
                break;
            case CODE:
                if (!(count < fi->numOfChars))
                {
                    reallocFontMetrics( (void**)&(fi->cmi),
                                        &(fi->numOfChars), enlargeCount(fi->numOfChars),
                                        sizeof(CharMetricInfo) );
                    temp = &(fi->cmi[ count - 1 ]);
                }
                if (count < fi->numOfChars)
                {
                    if (firstTime) firstTime = false;
                    else temp++;
                    temp->code = atoi(token(fp));
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
                    reallocFontMetrics( (void**)&(fi->cmi),
                                        &(fi->numOfChars), enlargeCount(fi->numOfChars),
                                        sizeof(CharMetricInfo) );
                    temp = &(fi->cmi[ count - 1 ]);
                }
                if (count < fi->numOfChars) {
                    if (firstTime)
                        firstTime = false;
                    else
                        temp++;
                    sscanf(token(fp),"<%x>", &temp->code);
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
                temp->wx = atoi(token(fp));
                temp->wy = atoi(token(fp));
                break;
            case X0WIDTH:
                temp->wx = atoi(token(fp));
                break;
            case XWIDTH:
                temp->wx = atoi(token(fp));
                break;
            case CHARNAME:
                keyword = token(fp);
                temp->name = (char *) malloc(strlen(keyword) + 1);
                strcpy(temp->name, keyword);
                break;
            case CHARBBOX:
                temp->charBBox.llx = atoi(token(fp));
                temp->charBBox.lly = atoi(token(fp));
                temp->charBBox.urx = atoi(token(fp));
                temp->charBBox.ury = atoi(token(fp));
                break;
            case LIGATURE: {
                Ligature **tail = &(temp->ligs);
                Ligature *node = *tail;

                if (*tail != NULL)
                {
                    while (node->next != NULL)
                        node = node->next;
                    tail = &(node->next);
                }

                *tail = (Ligature *) calloc(1, sizeof(Ligature));
                keyword = token(fp);
                (*tail)->succ = (char *) malloc(strlen(keyword) + 1);
                strcpy((*tail)->succ, keyword);
                keyword = token(fp);
                (*tail)->lig = (char *) malloc(strlen(keyword) + 1);
                strcpy((*tail)->lig, keyword);
                break; }
            case ENDCHARMETRICS:
                cont = false;;
                break;
            case ENDFONTMETRICS:
                cont = false;
                error = normalEOF;
                break;
            case VVECTOR:
                keyword = token(fp);
                keyword = token(fp);
                break;
            case NOPE:
            default:
                error = parseError;
                break;
        } /* switch */
    } /* while */

    if ((error == ok) && (count != fi->numOfChars))
        error = reallocFontMetrics( (void**)&(fi->cmi), &(fi->numOfChars),
                                    count, sizeof(CharMetricInfo) );

    if ((error == ok) && (count != fi->numOfChars))
        error = parseError;

    return(error);

} /* parseCharMetrics */



/************************* parseTrackKernData ***********************/

/*  This function is called by "parseFile". It will parse the AFM File
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

static int parseTrackKernData( FILE* fp, register FontInfo* fi)
{
    bool cont = true, save = (fi->tkd != NULL);
    int pos = 0, error = ok, tcount = 0;
    register char *keyword;

    while (cont)
    {
        keyword = token(fp);

        if (keyword == NULL)
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        if (!save)
            /* get tokens until the end of the Track Kerning Data */
            /* section without saving any of the data */
            switch(recognize(keyword))
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
            switch(recognize(keyword))
            {
                case COMMENT:
                    keyword = linetoken(fp);
                    break;
                case TRACKKERN:
                    if (!(tcount < fi->numOfTracks))
                    {
                        reallocFontMetrics( (void**)&(fi->tkd), &(fi->numOfTracks),
                                            enlargeCount(fi->numOfTracks), sizeof(TrackKernData) );
                    }

                    if (tcount < fi->numOfTracks)
                    {
                        keyword = token(fp);
                        fi->tkd[pos].degree = atoi(keyword);
                        keyword = token(fp);
                        fi->tkd[pos].minPtSize = StringToDouble(keyword);
                        keyword = token(fp);
                        fi->tkd[pos].minKernAmt = StringToDouble(keyword);
                        keyword = token(fp);
                        fi->tkd[pos].maxPtSize = StringToDouble(keyword);
                        keyword = token(fp);
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
        error = reallocFontMetrics( (void**)&(fi->tkd), &(fi->numOfTracks),
                                    tcount, sizeof(TrackKernData) );

    if (error == ok && tcount != fi->numOfTracks)
        error = parseError;

    return(error);

} /* parseTrackKernData */


/************************* parsePairKernData ************************/

/*  This function is called by "parseFile". It will parse the AFM File
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

static int parsePairKernData( FILE* fp, register FontInfo* fi)
{
    bool cont = true, save = (fi->pkd != NULL);
    int pos = 0, error = ok, pcount = 0;
    register char *keyword;

    while (cont)
    {
        keyword = token(fp);

        if (keyword == NULL)
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        if (!save)
            /* get tokens until the end of the Pair Kerning Data */
            /* section without saving any of the data */
            switch(recognize(keyword))
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
            switch(recognize(keyword))
            {
                case COMMENT:
                    keyword = linetoken(fp);
                    break;
                case KERNPAIR:
                    if (!(pcount < fi->numOfPairs))
                    {
                        reallocFontMetrics( (void**)&(fi->pkd), &(fi->numOfPairs),
                                            enlargeCount(fi->numOfPairs), sizeof(PairKernData) );
                    }
                    if (pcount < fi->numOfPairs)
                    {
                        keyword = token(fp);
                        fi->pkd[pos].name1 = strdup( keyword );
                        keyword = token(fp);
                        fi->pkd[pos].name2 = strdup( keyword );
                        keyword = token(fp);
                        fi->pkd[pos].xamt = atoi(keyword);
                        keyword = token(fp);
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
                        reallocFontMetrics( (void**)&(fi->pkd), &(fi->numOfPairs),
                                            enlargeCount(fi->numOfPairs), sizeof(PairKernData) );
                    }
                    if (pcount < fi->numOfPairs)
                    {
                        keyword = token(fp);
                        fi->pkd[pos].name1 = strdup( keyword );
                        keyword = token(fp);
                        fi->pkd[pos].name2 = strdup( keyword );
                        keyword = token(fp);
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
        error = reallocFontMetrics( (void**)&(fi->pkd), &(fi->numOfPairs),
                                    pcount, sizeof(PairKernData) );

    if (error == ok && pcount != fi->numOfPairs)
        error = parseError;

    return(error);

} /* parsePairKernData */


/************************* parseCompCharData **************************/

/*  This function is called by "parseFile". It will parse the AFM File
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

static int parseCompCharData( FILE* fp, register FontInfo* fi)
{
    bool cont = true, firstTime = true, save = (fi->ccd != NULL);
    int pos = 0, j = 0, error = ok, ccount = 0, pcount = 0;
    register char *keyword;

    while (cont)
    {
        keyword = token(fp);
        if (keyword == NULL)
            /* Have reached an early and unexpected EOF. */
            /* Set flag and stop parsing */
        {
            error = earlyEOF;
            break; /* get out of loop */
        }
        if (ccount > fi->numOfComps)
        {
            reallocFontMetrics( (void**)&(fi->ccd), &(fi->numOfComps),
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
            switch(recognize(keyword))
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
                    keyword = linetoken(fp);
                    break;
                default:
                    break;
            } /* switch */
        else
            /* otherwise parse entire Composite Character info section, */
            /* saving the data */
            switch(recognize(keyword))
            {
                case COMMENT:
                    keyword = linetoken(fp);
                    break;
                case COMPCHAR:
                    if (!(ccount < fi->numOfComps))
                    {
                        reallocFontMetrics( (void**)&(fi->ccd), &(fi->numOfComps),
                                            enlargeCount(fi->numOfComps), sizeof(CompCharData) );
                    }
                    if (ccount < fi->numOfComps)
                    {
                        keyword = token(fp);
                        if (pcount != fi->ccd[pos].numOfPieces)
                            error = parseError;
                        pcount = 0;
                        if (firstTime) firstTime = false;
                        else pos++;
                        fi->ccd[pos].ccName = strdup( keyword );
                        keyword = token(fp);
                        fi->ccd[pos].numOfPieces = atoi(keyword);
                        fi->ccd[pos].pieces = (Pcc *)
                            calloc(fi->ccd[pos].numOfPieces, sizeof(Pcc));
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
                        keyword = token(fp);
                        fi->ccd[pos].pieces[j].pccName = strdup( keyword );
                        keyword = token(fp);
                        fi->ccd[pos].pieces[j].deltax = atoi(keyword);
                        keyword = token(fp);
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
        reallocFontMetrics( (void**)&(fi->ccd), &(fi->numOfComps),
                            ccount, sizeof(CompCharData) );

    if (error == ok && ccount != fi->numOfComps)
        error = parseError;

    return(error);

} /* parseCompCharData */




/*************************** 'PUBLIC' FUNCTION ********************/


/*************************** parseFile *****************************/

/*  parseFile is the only 'public' procedure available. It is called
 *  from an application wishing to get information from an AFM file.
 *  The caller of this function is responsible for locating and opening
 *  an AFM file and handling all errors associated with that task.
 *
 *  parseFile expects 3 parameters: a vaild file pointer, a pointer
 *  to a (FontInfo *) variable (for which storage will be allocated and
 *  the data requested filled in), and a mask specifying which
 *  data from the AFM File should be saved in the FontInfo structure.
 *
 *  The file will be parsed and the requested data will be stored in
 *  a record of type FontInfo (refer to ParseAFM.h).
 *
 *  parseFile returns an error code as defined in parseAFM.h.
 *
 *  The position of the read/write pointer associated with the file
 *  pointer upon return of this function is undefined.
 */

int parseFile ( FILE* fp, FontInfo** fi, FLAGS flags)
{

    int code = ok;  /* return code from each of the parsing routines */
    int error = ok; /* used as the return code from this function */

    register char *keyword; /* used to store a token */


    /* storage data for the global variable ident */
    ident = (char *) calloc(MAX_NAME, sizeof(char));
    if (ident == NULL) {error = storageProblem; return(error);}

    (*fi) = (FontInfo *) calloc(1, sizeof(FontInfo));
    if ((*fi) == NULL) {error = storageProblem; return(error);}

    if (flags & P_G)
    {
        (*fi)->gfi = (GlobalFontInfo *) calloc(1, sizeof(GlobalFontInfo));
        if ((*fi)->gfi == NULL) {error = storageProblem; return(error);}
    }

    /* The AFM File begins with Global Font Information. This section */
    /* will be parsed whether or not information should be saved. */
    code = parseGlobals(fp, (*fi)->gfi);

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
        (*fi)->numOfChars = atoi(token(fp));
        if (flags & (P_M ^ P_W))
        {
            (*fi)->cmi = (CharMetricInfo *)
                calloc((*fi)->numOfChars, sizeof(CharMetricInfo));
            if ((*fi)->cmi == NULL) {error = storageProblem; return(error);}
            code = parseCharMetrics(fp, *fi);
        }
        else
        {
            if (flags & P_W)
            {
                (*fi)->cwi = (int *) calloc(256, sizeof(int));
                if ((*fi)->cwi == NULL)
                {
                    error = storageProblem;
                    return(error);
                }
            }
            /* parse section regardless */
            code = parseCharWidths(fp, (*fi)->cwi);
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
        keyword = token(fp);
        if (keyword == NULL)
            /* Have reached an early and unexpected EOF. */
            /* Set flag and stop parsing */
        {
            code = earlyEOF;
            break; /* get out of loop */
        }
        switch(recognize(keyword))
        {
            case STARTKERNDATA:
                break;
            case ENDKERNDATA:
                break;
            case STARTTRACKKERN:
                keyword = token(fp);
                if (flags & P_T)
                {
                    (*fi)->numOfTracks = atoi(keyword);
                    (*fi)->tkd = (TrackKernData *)
                        calloc((*fi)->numOfTracks, sizeof(TrackKernData));
                    if ((*fi)->tkd == NULL)
                    {
                        error = storageProblem;
                        return(error);
                    }
                } /* if */
                code = parseTrackKernData(fp, *fi);
                break;
            case STARTKERNPAIRS:
                keyword = token(fp);
                if (flags & P_P)
                {
                    (*fi)->numOfPairs = atoi(keyword);
                    (*fi)->pkd = (PairKernData *)
                        calloc((*fi)->numOfPairs, sizeof(PairKernData));
                    if ((*fi)->pkd == NULL)
                    {
                        error = storageProblem;
                        return(error);
                    }
                } /* if */
                code = parsePairKernData(fp, *fi);
                break;
            case STARTCOMPOSITES:
                keyword = token(fp);
                if (flags & P_C)
                {
                    (*fi)->numOfComps = atoi(keyword);
                    (*fi)->ccd = (CompCharData *)
                        calloc((*fi)->numOfComps, sizeof(CompCharData));
                    if ((*fi)->ccd == NULL)
                    {
                        error = storageProblem;
                        return(error);
                    }
                } /* if */
                code = parseCompCharData(fp, *fi);
                break;
            case ENDFONTMETRICS:
                code = normalEOF;
                break;
            case COMMENT:
                linetoken(fp);
                break;
            case NOPE:
            default:
                code = parseError;
                break;
        } /* switch */

        if ((error != earlyEOF) && (code < 0)) error = code;

    } /* while */

    if ((error != earlyEOF) && (code < 0)) error = code;

    if (ident != NULL) { free(ident); ident = NULL; }

    return(error);

} /* parseFile */

void
freeFontInfo (FontInfo *fi)
{
    int i, j;

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
        for (int i = 0; i < fi->numOfPairs; i++)
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
            for (j = 0; j < fi->ccd[i].numOfPieces; j++)
                free (fi->ccd[i].pieces[j].pccName);

            free (fi->ccd[i].pieces);
        }
        free (fi->ccd);
    }

    free (fi);
}

} // namspace
