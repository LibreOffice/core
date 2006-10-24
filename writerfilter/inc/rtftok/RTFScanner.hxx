/**
  Copyright 2005 Sun Microsystems, Inc.
*/

#ifndef INCLUDED_RTFSCANNER_HXX
#define INCLUDED_RTFSCANNER_HXX

#include <WriterFilterDllApi.hxx>
#include <vector>
#include <rtftok/RTFInputSource.hxx>
#include <rtftok/RTFScannerHandler.hxx>

namespace writerfilter { namespace rtftok {

class WRITERFILTER_DLLPUBLIC RTFScanner {
public:
        RTFScanner(RTFScannerHandler &eventHandler_) : eventHandler(eventHandler_) {};
        virtual ~RTFScanner()   { }

    const char* YYText()    { return yytext; }
    int YYLeng()        { return yyleng; }

    virtual int yylex() = 0;

    int lineno() const      { return yylineno; }

protected:
    char* yytext;
    int yyleng;
    int yylineno;       // only maintained if you use %option yylineno
     int yy_flex_debug; // only has effect with -d or "%option debug"
        RTFScannerHandler &eventHandler;


public:
  static writerfilter::rtftok::RTFScanner* createRTFScanner(writerfilter::rtftok::RTFInputSource& inputSource, writerfilter::rtftok::RTFScannerHandler &eventHandler);
};

} } /* end namespace writerfilter::rtftok */


#endif /* INCLUDED_RTFSCANNER_HXX */
