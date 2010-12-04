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

// guw - A wrapper program to execute windows programs with parameters that
// contain cygwin (POSIX) style pathnames.

// Todo: Add a -? switch to guw to issue a help page.

#include <string>
#include <list>
#include <vector>

#include <iostream>
#include <sstream>
#include <fstream>

#include <cstddef>
#include <cerrno>

#include <sys/cygwin.h>
#include <windows.h>
#include <regex.h>


using std::string;
using std::list;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::size_t;

void init_ignorepara(string command);
bool is_ignorepara(const string &para);
int winFormat(string &para);
void do_atcommand(string &para);
void myCygpath(string &path, int shortname = 1 );
void replace_cyg_env( void );
void Fatal( const string text );

int match2s(const string argument, const char *pattern, string &sub1, string &sub2);
void rep_subn_cyg(string &argument);
void rep_subn( string &argument, const char *pattern, int subexp, const char repl);
void rep_char( string &argument, const char from, const char to);

bool debug = false;
bool debug_light = false;

// The commands are treated case insensitive, the parameters
// are case sensitive.
const string ignorepara[] = { "echo /TEST",
                              "cl -clr: -Z",
                              "climaker StarOffice/OpenOffice",
                              "csc -target:",
                              "g++ -DLOCAL_RULE_LANGS -DUPD -DMINOR"
                                " -DBUILD_ID -DSC_INFO_OSVERSION",
                              "gcc -DUDATA_SO_SUFFIX -DSTATIC_O"
                                " -DPACKAGE -DU_MAKE",
                              "lib /OUT: -out: -def: -machine:",
                              "link /BASE: /COMMENT: /DEBUG: /DLL /ENTRY:"
                                " /MACHINE: /MAP /NODEFAULTLIB /OPT: /RELEASE"
                                " /STACK: /SUBSYSTEM: -NODEFAULTLIB:"
                                " -def: delayload: -implib: -map: -out:",
                              "rc -D",
                              "regcomp -env: vnd.sun.star.expand:"
                                " vnd.openoffice.pymodule: file:",
                              "regmerge /UCR",
                              "rsc -DOOO_" };

vector<string> ignorepara_vec;

// environment variables that are "winformatted" when -env is given
const string transformvars[] = { "SOLAR_VERSION",
                                 "SOLARVERSION",
                                 "SOLARVER",
                                 "SRC_ROOT",
                                 "LOCALINI",
                                 "GLOBALINI",
                                 "SOLARENV",
                                 "STAR_INSTPATH",
                                 "STAR_SOLARPATH",
                                 "STAR_PACKMISC",
                                 "STAR_SOLARENVPATH",
                                 "STAR_INITROOT",
                                 "STAR_STANDLST",
                                 "CLASSPATH",
                                 "JAVA_HOME" };


int main(int argc, char **argv) {

  // initialize arglist with arguments
  list<string> arglist(argv, argv + argc);

  // Drop the first (filename) argument
  arglist.pop_front();

  // iterarot over cmdline elements
  list<string>::iterator ele = arglist.begin();

  // Allowed switch values
  bool conv_cyg_arg = false;

  // Look for switches to guw
  // Supported: -env
  //            -dbg
  //            -ldbg
  while ( !arglist.empty()
          && ele!=arglist.end()
          && (ele->find("-", 0) == 0) ) {
    if (ele->find("-env", 0) == 0) {
      if ( conv_cyg_arg )
        Fatal("-env used twice!");

      conv_cyg_arg = true;
      ele = arglist.erase(ele);
      continue;
    }
    else if (ele->find("-dbg", 0) == 0) {

      debug = true;
      ele = arglist.erase(ele);
      continue;
    }
    else if (ele->find("-ldbg", 0) == 0) {

      debug_light = true;
      ele = arglist.erase(ele);
      continue;
    }
    else {
      // Ignore this switch
      ++ele;
    }
  }

  // The next entry must be the program
  string command;
  if ( !arglist.empty() ) {
    command.assign(*arglist.begin());
    arglist.pop_front();
  }
  else
    Fatal("guw needs at least one parameter.");

  if ( debug )
    cerr << "Command: " << command << "\n" << endl;
  // Initialize parameter exception list (for this command)
  init_ignorepara(command);


  // Do something if -env was found
  if ( conv_cyg_arg )
    replace_cyg_env();


  // loop and and DOSify the parameters
  if ( debug )
    cerr << "Transform the parameter\n" << endl;

  ele=arglist.begin();
  while ( ele != arglist.end() ) {

    if ((*ele)[0] == '@')
      do_atcommand(*ele);
    else if (!is_ignorepara(*ele)) {
      if ( debug ) {
        cerr << "----------------" << endl;
        cerr << "Process parameter: " << *ele << endl;
      }
      winFormat(*ele);
      if ( debug )
        cerr << "Transformed to: " << *ele << "\n" << endl;
    }

    ++ele;
  }

  // create the argv[] for execvp(argv[0], argv);
  ele=arglist.begin();

  // const char *nargv[arglist.size()+2]; // or ..
  char *nargv[arglist.size()+2];

  // nargv[0] = command.c_str(); // or ..
  nargv[0] = new char[command.length()+1];
  // strcpy(nargv[0], command.c_str());
  command.copy(nargv[0], command.length());
  nargv[0][command.length()] = 0;

  if ( debug )
    cerr << "----------------\n" << endl;
  if ( debug || debug_light )
    cerr << "Execute: " << nargv[0];

  int count = 1, sLen;
  while ( ele != arglist.end() ) {
    // nargv[count] = ele->c_str(); // or ..
    sLen = ele->length();
    nargv[count] = new char[sLen+1];
    //    strcpy(nargv[count], ele->c_str());
    ele->copy(nargv[count], sLen);
    nargv[count][sLen] = 0;

    if ( debug || debug_light )
      cerr << " " << nargv[count];

    ++count;
    ++ele;
  }
  // last nargv[] must be NULL
  nargv[count] = NULL;
  if ( debug || debug_light )
    cerr << endl;

  // Unfortunately the prototype of execvp does not like const char*,
  // actually not const char* nargv[] coming from .c_str(). So either
  // we copy everything into newly allocated variables or we force it
  // with a cast. const_cast<char * const *>()
  // execvp(nargv[0], const_cast<char * const *>(nargv) );
  if ( execvp(nargv[0], nargv ) < 0 ) {
    perror("Execvp error.  Aborting.");
    exit(1);       
  }

  // Omit the deleting of the dynamically allocated nargv[] elements
  // here as this part will never be reached.

  return 0;
}

// Initialize exception list from global var ignorepara[]
void init_ignorepara(string fullcommand) {
  const size_t kplen = sizeof(ignorepara)/sizeof(string *);
  string shortcommand, cmd, para, sub2;

  // First lowercase everything
  for(size_t i=0;i<fullcommand.length();i++)
    fullcommand[i] = tolower(fullcommand[i]);

  // Remove a potential .exe
  size_t slen = fullcommand.length();

  // for slen == 3 this would yield string::npos otherwise
  if ( slen > 4 && fullcommand.rfind(".exe") == slen - 4 )
    fullcommand.erase(slen-4);

  // get the program name - Only one subexpression
  if (!match2s(fullcommand, "([[:alnum:]_~. +-]+)$",
               shortcommand, sub2)) {
    Fatal("No basename found in: " + fullcommand);
  }

  for (size_t i=0; i != kplen; ++i) {
    std::istringstream line(ignorepara[i]);
    line >> cmd;
    if (shortcommand == cmd)
      while (line >> para) {
        ignorepara_vec.push_back(para);
      }
  }
  return ;
}

// Check if command/parameter is in exception list.
bool is_ignorepara(const string &para) {

  for( vector<string>::iterator it = ignorepara_vec.begin();
       it != ignorepara_vec.end(); it++ ) {
    if ( para.find(*it) != string::npos ) {
      if ( debug )
        cerr << "Found execption para: " << para << endl;

      return true;
    }
  }

  return false;
}

// Reformat para to DOSish format
int winFormat(string &para) {
  string su1, su2;

  // Instead of ([/[:alnum:]_~. +-]+)   use   ((/?[[:alnum:]_~. +-]+)+)

  // find [-][-]X<something>=<path>, sometimes with quotes or "/" at the end
  if (match2s(para, "^(-?-?[[:alpha:]][[:alnum:]_.-]*=)[\'\"]?((/?[[:alnum:]_~. +-]+)+)[\'\"]?$",
              su1, su2)) {

    myCygpath(su2);
    para.assign(su1 + su2);
    if ( debug )
      cerr << "   WinFormat - ([-][-]<something>=<path>)\n"
           << "      " << para << endl;

  }
  // find -X<something>:<path>, sometimes with quotes or "/" at the end
  else if (match2s(para, "^(-[[:alpha:]][[:alnum:]_.]*:)[\'\"]?((/?[[:alnum:]_~. +-]+)+)[\'\"]?$",
                   su1, su2)) {

    myCygpath(su2);
    para.assign(su1 + su2);
    if ( debug )
      cerr << "   WinFormat - (-<something>:<path>)\n"
           << "      " << para << endl;

  }
  // find -X<something>:<NO-path>, and prevents translating of these.
  else if (match2s(para, "^(-[[:alpha:]][[:alnum:]_]*:)(.*)$",
                   su1, su2)) {

    // myCygpath(su2);
    // para.assign(su1 + su2);
    if ( debug )
      cerr << "   WinFormat - (-<something>:<NO-path>)\n"
           << "      " << para << endl;

  }
  // See iz35982 for the reason for the special treatment of this switch.
  // This regex evaluates <something>:///<path>, sometimes with
  // quotes or "/" at the end
  else if (match2s(para, "^([[:alpha:]][[:alnum:]_]*:)[\'\"]?///((/?[[:alnum:]_~. +-]+)+)[\'\"]?$",
                   su1, su2)) {

    myCygpath(su2);
    para.assign(su1 + "///" + su2);
    // Replace \ to /
    rep_char( para, '\\', '/');

    if ( debug )
      cerr << "   WinFormat - (<something>:///<path>)\n"
           << "      " << para << endl;

  }
  // find -X<absolute path>, sometimes with quotes or "/" at the end
  else if (match2s(para, "^(-[[:alpha:]])[\'\"]?((/[[:alnum:]_~. +-]+)+)[\'\"]?$",
                   su1, su2)) {

    myCygpath(su2);
    para.assign(su1 + su2);
    if ( debug )
      cerr << "   WinFormat - (-X<absolute path>)\n"
           << "      " << para << endl;

  }
  // find -FX<path> (MSVC switches for output naming), sometimes with quotes
  // or "/" at the end
  else if (match2s(para, "^(-F[ARdemopr])[\'\"]?(/[/[:alnum:]_~. +-]+)[\'\"]?$",
                   su1, su2)) {

    myCygpath(su2);
    para.assign(su1 + su2);
    if ( debug )
      cerr << "   WinFormat - (compiler naming (-FX<absolute path>) path)\n"
           << "      " << para << endl;

  }
  else{
    // No parameter found, assume a path

    // replace the colon in drives with 0x1F"
    // (Unused ascii US - unit separator)
    rep_subn( para, "(^|[;,])[[:alpha:]](:)", 2, 0x1F);

    // Replace remaining : to ;
    rep_char( para, ':', ';');

    // Replace back US to ':';
    rep_char( para, 0x1F, ':');

    /* Search for posix path ;entry; (The regex accepts valid paths with at
     * least one /) and replace with DOS path, accept quotes.
     * since iz28717 we also accept ',' as path seperator. */
    rep_subn_cyg(para);

    if ( debug )
      cerr << "   WinFormat - full path\n"
           << "      " << para << endl;

  }

  // Sanity check for -X<abspath>
  if (match2s(para, "^(-[[:alpha:]])[\'\"]?((/[[:alnum:]_~. +-]+)+)",
              su1, su2)) {
    Fatal("Not converted -X/... type switch in :" + para);
  }
  // Sanity check for [-]X<something>(:|=)<abspath> case
  if (match2s(para, "^(-?[[:alpha:]][[:alnum:]_.]+[=:])[\'\"]?((/[[:alnum:]_~. +-]+)+)",
              su1, su2)) {
    Fatal("Not processed [-]X<something>(=|:)/... in :" + para);
  }


  return 1;
}

// Reformat para to DOSish format
void do_atcommand(string &para) {
  string at, filename, token;

  // Workaround, iz28717, keep number of @'s.
  match2s(para, "^(@+)(.*)",at ,filename);
  if ( debug ) {
    cerr << "----------------" << endl;
    cerr << "Process @-file" << endl;
    cerr << "   :" << at << ": before filename :" << filename << ":" << endl;
  }

  // Read at file into memory
  std::ifstream atin(filename.c_str());
  list<string> newtoken;
  while (atin >> token) {
    // Read / transform tokens
    if ( debug )
      cerr << "@ token :" << token << ":" << endl;
    if (!is_ignorepara(token))
      winFormat(token);

    newtoken.push_back(token);
  }
  atin.close();

  // Write token tokens bak to file
  if ( debug  || debug_light )
    cerr << "New @-file parameter:" << endl;

  // for debugging ..
  // filename += ".bak";

  std::ofstream atout(filename.c_str());
  list<string>::iterator tok = newtoken.begin();
  while ( tok != newtoken.end() ) {
    if ( debug || debug_light )
      cerr << ( tok != newtoken.begin() ? " " : "" ) << *tok ;

    atout << ( tok != newtoken.begin() ? " " : "" ) << *tok ;
    ++tok;
  }
  // We want a dos file
  atout << '\r' << endl;
  atout.close();

  // Transform the filename
  winFormat(filename);
  para = at + filename;
  if ( debug || debug_light ) {
    cerr << "\nNew @-file name: " << para << "\n" << endl;
  }
}

void myCygpath(string &path, int shortname /* =1 */ )
{
  static char convpath[MAX_PATH];
  static char buf[MAX_PATH];
  int err;

  // Only use cygwin_conv_to_win32_path() on absolute paths as it errors
  // out if its path doen't exist. Unfortunatelt there are a lot of not
  // existing relative pathes used as parameters during an OOo build.
  if( path.find("/", 0) == 0) {
    err = cygwin_conv_to_win32_path( path.c_str(), convpath );
  }
  else {
    rep_char( path, '/', '\\');
    // see below, we copy convpath back to path, that's stupid
    path.copy( convpath, path.length());
    convpath[path.length()] = 0;
    err = 0;
  }

  if (err)
    Fatal("converting: " + path + " - " + strerror(errno) );

  // Only convert to short dos names when space is present
  if (shortname && (path.find(" ", 0) != string::npos) ) {
    DWORD len = GetShortPathName (convpath, buf, MAX_PATH);
    if (!len) {
      Fatal("cannot create short name of " + string(convpath) );
    }

    path.assign(buf);
  }
  else
    path.assign(convpath);

}

void replace_cyg_env( void ) {
  // Transform certain environment variables
  if ( debug )
    cerr << "Transforming some environment variables" << endl;

  const size_t nvars = sizeof(transformvars)/sizeof(string *);

  char *currvar;
  string newvar;

  for (size_t i=0; i != nvars; ++i) {
    if ( currvar = getenv(transformvars[i].c_str() ) ) {
      // Only transform existent vars
      if ( debug )
        cerr << "Transform variable: " << transformvars[i] << "="
             << currvar << endl;
      newvar.assign(currvar);
      winFormat(newvar);
      if( setenv(transformvars[i].c_str(), newvar.c_str(), 1) )
        Fatal("setenv failed on " + transformvars[i] + "=" + newvar +
              " with error: " + strerror(errno));
      if ( debug )
        cerr << "To: " << transformvars[i] << "="
             << newvar << "\n" << endl;
    }
  }
}


void Fatal( const string text ) {
  // End with error
  cerr << "Error: " << text << endl;
  exit(1);
}


int
match2s(const string argument, const char *pattern, string &sub1, string &sub2)
{
  int status;
  regex_t re;

  const int maxsub = 3;  // Only 3 needed, 4 is for debug
  regmatch_t match[maxsub];

  if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
    Fatal("regcomp had a problem.");      /* report error */
  }
  status = regexec(&re, argument.c_str(), maxsub, match, 0);
  regfree(&re);

  if (status == REG_NOMATCH) {
    return(0);      /* no match */
  } else if (status == 0) {
    string tstr(argument.c_str() + match[0].rm_so,
                match[0].rm_eo - match[0].rm_so);
    // cout << "Match: " << tstr << endl;

    sub1.assign(argument.c_str() + match[1].rm_so, match[1].rm_eo - match[1].rm_so);
    // cout << "Match1: " << sub1 << endl;

    sub2.assign(argument.c_str() + match[2].rm_so, match[2].rm_eo - match[2].rm_so);
    // cout << "Match2: " << sub2 << endl;

    return(1);      /* match found */
  } else {
    Fatal("regexec had a problem.");
  }

  // Not reached.
  return(1);
}


// Replace path entry with myCygpath() version
void rep_subn_cyg(string &argument)
{
  // accept ["']<entry>["']:["']<entry>["']:... to make the
  // $(WRAPCMD) echo 1 ICON $(EMQ)"$(APP1ICON)$(EMQ)"
  // work in ?tg_app.mk.
  // FIXME: Better would be to use a DOSified $(APP1ICON) there and remove
  // the special " treatment here.
  const char *pattern = "(^|[;,])[\'\"]?([[:alnum:]_~. +-]*(/[[:alnum:]_~. +-]+)+/?)[\'\"]?([;,]|$)";
  const int subexp = 2;

  int status, pos=0;
  regex_t re;

  string repstr;
  string::size_type oLen, nLen;

  const int maxsub = subexp+1;  // One more than the maximal subexpression
  regmatch_t match[maxsub];

  if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
    Fatal("regcomp had a problem.");      /* report error */
  }
  status = regexec (&re, argument.c_str() + pos, maxsub, match, 0);
  while (status == 0) {  /* While matches found. */
    // Classical assert()
    if (match[subexp].rm_eo == -1) {
      Fatal("Nonexisting subexpression specified!");
    }

    oLen = match[subexp].rm_eo - match[subexp].rm_so;
    repstr.assign(argument.c_str() + pos + match[subexp].rm_so, oLen);

    // Do not replace with shortpaths
    myCygpath(repstr, 0);
    nLen = repstr.length();

    // replace 
    argument.replace( pos + match[subexp].rm_so, oLen, repstr );

    /* Substring found between match[0].rm_so and match[0].rm_eo. */
    /* This call to regexec() finds the next match. */

    pos += match[0].rm_eo + nLen - oLen ;

    // Either the last match did end in ';' or we are at the end of para.
    // REG_NOTBOL is not used because we skip over the ';' by using pos.
    status = regexec (&re, argument.c_str() + pos, maxsub, match, 0);
  }

  if (status != REG_NOMATCH) {
    Fatal("regexec had a problem.");
  }

  regfree(&re);
}

// Replace all occurrences of subexpression number "subexp" with "repl"
void rep_subn( string &argument, const char *pattern, int subexp, const char repl)
{
  int status, pos=0;
  regex_t re;

  const int maxsub = subexp+1;  // One more than the maximal subexpression
  regmatch_t match[maxsub];

  if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
    Fatal("regcomp had a problem.");      /* report error */
  }
  status = regexec (&re, argument.c_str() + pos, maxsub, match, 0);
  while (status == 0) {  /* While matches found. */
    // Classical assert()
    if (match[subexp].rm_eo == -1) {
      Fatal("Nonexisting subexpression specified!");
    }

    argument[pos + match[subexp].rm_so] = repl;

    /* Substring found between match[0].rm_so and match[0].rm_eo. */
    /* This call to regexec() finds the next match. */
    pos += match[0].rm_eo;
    status = regexec (&re, argument.c_str() + pos, maxsub, match, REG_NOTBOL);
  }

  if (status != REG_NOMATCH) {
    Fatal("regexec had a problem.");
  }

  regfree(&re);
}

// Replace all char1 with char2
void rep_char( string &argument, const char from, const char to)
{
  string::size_type loc = 0;

  while ( (loc = argument.find( from, loc )) != string::npos ) {
    argument[loc] = to;
  }
}
