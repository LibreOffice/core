/**
  Copyright 2005 Sun Microsystems, Inc.
*/

#ifndef INCLUDED_RTFINPUTSOURCE_HXX
#define INCLUDED_RTFINPUTSOURCE_HXX

namespace writerfilter { namespace rtftok {

class RTFInputSource
{
public:
  virtual int read(void *buffer, int maxLen) = 0;
};

} }  /* end namespace writerfilter::rtftok */


#endif /*  INCLUDED_RTFINPUTSOURCE_HXX */
