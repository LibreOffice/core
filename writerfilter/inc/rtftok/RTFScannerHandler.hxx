/**
  Copyright 2005 Sun Microsystems, Inc.
  */

#ifndef INCLUDED_RTFSCANNERHANDLER_HXX
#define INCLUDED_RTFSCANNERHANDLER_HXX

namespace writerfilter { namespace rtftok {

class RTFScannerHandler
{
public:
    virtual void dest(char*token, char*value)=0;
    virtual void ctrl(char*token, char*value)=0;
    virtual void lbrace(void)=0;
    virtual void rbrace(void)=0;
    virtual void addSpaces(int count)=0;
    virtual void addBinData(unsigned char data)=0;
    virtual void addChar(char ch) =0;
    virtual void addCharU(sal_Unicode ch) =0;
    virtual void addHexChar(char* hexch) =0;

};

} } /* end namespace writerfilter::rtftok */

#endif /* INCLUDED_RTFSCANNERHANDLER_HXX */
